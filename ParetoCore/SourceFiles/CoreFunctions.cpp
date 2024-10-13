#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <omp.h>
#include <conio.h>
#include <process.h>  


#define QKEY 81
#define strcmpCaseInsenstive strcmpi

#define CLIENTFILES "clientFiles\\%s"
#define CLIENTFILES_STL "clientFiles\\%s_STL.%.3f.stl"

#else
int MB_OK = 0;
void MessageBoxA(int a, char *b, char *c, int d)
{
	;//dummy call for compilation
}
#define strcmpCaseInsenstive strcasecmp

#define CLIENTFILES "clientFiles/%s"
#define CLIENTFILES_STL "clientFiles/%s_STL.%.3f.stl"

#endif




#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#include "AFElasticityGPUDefines.h"
#include "CommonGUI.h"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "Timer.h"
#include <ctime>
#include "taubinSmooth.h"
#include <algorithm>    // std::sort
#include "MeshContraction.hpp"
#include "Simplify.h"

#if (COMPILE_MODE == BATCH_MODE) 
#define Message(a,b,c,d) (std::wcout<<b<<endl)

#else
#define Message(a,b,c,d) MessageBoxA(a,b,c,d)
#endif


#if (COMPILE_MODE != BATCH_MODE) 
#include <GL/glui.h>
#endif
#pragma warning(disable:4996) // strcpy
#pragma warning(disable:4305) // double to float

int BCType[MAX_BC_SET][STL_MAX_GROUPS][3];
double BCValue[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
double ElasticityAcceleration[MAX_BC_SET][4];// Linear acceleration (x,y,z) and //radians per second (omega) results in radial acceleration



int ThermalBCType[MAX_BC_SET][STL_MAX_GROUPS];
double ThermalBCValue[MAX_BC_SET][STL_MAX_GROUPS];// could be T (Dirichlet), q (flux), h (convection) or epsi (radiation) value
double ThermalBCArea[MAX_BC_SET][STL_MAX_GROUPS];// keep track of all the areas
double ThermalBCRef[MAX_BC_SET][STL_MAX_GROUPS];// for convection and radiation

double ConstraintData[MAX_BC_SET][MAX_CONSTRAINTS][6]; //Deng: (upper/lower, hard/soft, limit value,x,y,z) // last three are optional
int ConstraintType[MAX_BC_SET][MAX_CONSTRAINTS];
int NumberOfConstraints[MAX_BC_SET] = { 0,0,0,0,0,0,0,0,0,0 };

char * materialNamesStr[] = { "AlloySteel", "Ti6Al4V","ABSPlastic","Al1060","CastIronGr25","AMS5862","Other" };

struct LiveVariables liveVar;
char userDefinedTitle[256];

ParetoApplication topopt;
ParetoApplication topoptTemp;
GPUData h_gpuData;
Voxelizer vox;
Voxelizer voxInitialDesign;
Timer globalTimer;
Timer timer;

int replaceTitle = 1;
enum PARETO_GUI_STATE guiState = GROUND;

bool GetMACaddress(char str[]);
int GetAllMACaddresses(char str[12][14]);
unsigned long hashString(char *str, int tmp);
void solveStatic();
bool verifyDirichletConditions();
bool verifyNeumannConditions();
extern bool saveIsoSurface(char *fileName, double *offset, bool snapToGeom, double desiredFractionOfIsoTriangles, bool fromMeshDensity, double pseudodensityCutoff = 0.5);

#if (COMPILE_MODE == BATCH_MODE)// dummy function definitions
void pareto_myGlutDisplay() { ; }
void setMainWindow() { ; }
void setPlotRoutine(int) { ; }
void setRotate(double *) { ; }
void setTranslate(double *) { ; }
void setZoom(double) { ; }
void pareto_setViewRoutine(int) { ; }
void SaveAsTIFF(const char *fileName) { ; }
void SaveAsBMP(const char *fileName) { ; }
void alert(char *msg) { printf(msg, "Pareto"); };
#else // just declare
void pareto_myGlutDisplay();
void setMainWindow();
void setPlotRoutine(int);
void setRotate(double *);
void setTranslate(double *);
void setZoom(double);
void pareto_setViewRoutine(int);
void SaveAsTIFF(const char *fileName);
void alert(char *msg) { Message(NULL, msg, "Pareto", MB_OK); };
void plotDataString(char str[]);
#endif



using namespace Simplify;
/**********************************************************************/
void printEstimatedTime(double estimatedTimeForCompletion, char *str)
{
	double hours, minutes, seconds;

	hours = floor(estimatedTimeForCompletion / 3600);
	estimatedTimeForCompletion -= hours * 3600;
	minutes = floor(estimatedTimeForCompletion / 60);
	estimatedTimeForCompletion -= minutes * 60;
	seconds = (estimatedTimeForCompletion);
	sprintf(str, "Estimated time: %.0f hr(s), %.0f min(s), %.0f sec(s)", hours, minutes, seconds);
}
/************************************************************/
int find_word(FILE *filein, char *word)
{
	char str[256];
	fpos_t pos;
	fgetpos(filein, &pos);
	while (fscanf(filein, "%s", str) != EOF) {
		if (strcmp(str, word) == 0)
			return 1;
	}
	// failed, return back to original position
	fsetpos(filein, &pos);
	return 0;
}
/************************************************************/
void printAvailableSystemMemory()
{
#ifdef _WIN32
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	printf("Available CPU memory (MB): %I64d \n", status.ullAvailPhys / 1024 / 1024);
#endif
}
/************************************************************/
int getAvailableSystemMemory()
{
#ifdef _WIN32
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	return (status.ullAvailPhys / 1024 / 1024);
#else
	return 0;
#endif
}


/************************************************************/
void resetLiveVariables()
{
	int i, j, bcSet;

	liveVar.showTriangles = 0;
	liveVar.plotAxis = 1;
	liveVar.DOSWindowCheck = 0;
	liveVar.DOSWindowOpen = false;

	liveVar.selectedPart = -1;
	liveVar.currentBCSet = 0;
	liveVar.nBCSet = 1;
	liveVar.ParetoBCType = PARETO_BC_FIXED;
	liveVar.assemblyMode = 0;

	liveVar.faceType = FACE_COARSE_CYL;
	liveVar.cutoffAngle = 15; ///degrees to find recursive mark neighboring triangles
	liveVar.normalToSurface = 0;

	liveVar.showConcaveEdges = 0;
	liveVar.XForce = 0;
	liveVar.YForce = 0;
	liveVar.ZForce = 0;
	liveVar.Pressure = 0;
	liveVar.Torque = 0;
	liveVar.XAcceleration = 0;
	liveVar.YAcceleration = 0;
	liveVar.ZAcceleration = 0;
	liveVar.radiansPerSecZAxis = 0;

	liveVar.CurrentElasticityBCLabel = 0;

	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		liveVar.activeBCSets[bcSet] = true;// all bc sets are active
		for (i = 0; i < STL_MAX_GROUPS; i++) {//default all BC are free Neumann
			for (j = 0; j < 3; j++) {
				BCType[bcSet][i][j] = ELASTICITY_NEUMANN;
				BCValue[bcSet][i][j] = 0;
			}
		}
		for (i = 0; i < 4; i++)
			ElasticityAcceleration[bcSet][i] = 0.0;// 4th component is radial acceleration

	}
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < STL_MAX_GROUPS; i++) {//default all BC are free Neumann
			ThermalBCArea[bcSet][i] = 0;
			ThermalBCType[bcSet][i] = THERMAL_NEUMANN;
			ThermalBCValue[bcSet][i] = 0;
			ThermalBCRef[bcSet][i] = 0;
		}
	}

	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < MAX_CONSTRAINTS; i++) {
			for (j = 0; j < 6; j++) {// (upper/lower,hard/soft/limit value,x,y,z
				ConstraintData[bcSet][i][j] = 0;
			}
		}
	}
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < MAX_CONSTRAINTS; i++) {
			ConstraintType[bcSet][i] = -1;
		}
	}
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		NumberOfConstraints[bcSet] = 0;
	}

	liveVar.ThermalBCType = PARETO_BC_TEMP;
	liveVar.ThermalBCValue = 23; // C
	liveVar.ThermalBCReference = 23;// for convection and radiation

	liveVar.nFacesSelected = 0;

	liveVar.nElemsToMesh = 10000;
	liveVar.meshQuality = 1;
	liveVar.conformingMesh = 0;
	liveVar.maxAspectRatio = 1.0;

	liveVar.useNonLinearThermal = 0;

	liveVar.partialvoxel = 0;
	liveVar.useBubbleFunctions = 1;
	liveVar.elemSignatureTolPower = 8;
	liveVar.xSym = 0;
	liveVar.ySym = 0;
	liveVar.zSym = 0;
	liveVar.forceRemesh = 1;
	liveVar.densityCutoff = 0.50;// fraction of nodes in element that must be inside to include the element
	liveVar.displayVolFraction = 1.0;
	liveVar.plotPrimaryMesh = 1;
	strcpy(liveVar.prjFileName, "");
	strcpy(liveVar.voxFileName, "");
	liveVar.boundingBox = 0;
	liveVar.plotContours = 0;
	liveVar.usePreconditioner = 0;
	liveVar.nodeBasedKU = 1;
	liveVar.workSpaceMethod = DEFLATION_RIGID;
	liveVar.nWorkSpaceGroups = 64;
	liveVar.cgResidualExponent = 9;
	liveVar.cgMaxIter = 20000;
	liveVar.eigRelAccuracyExponent = 2;

	liveVar.displayGeom = DISPLAY_GEOM_INITIALSTL;
	liveVar.displayField = DISPLAY_FIELD_NONE;
	liveVar.displayMode = DISPLAY_GEOM;
	liveVar.displayEdges = 0;
	liveVar.displayQuality = 6;
	liveVar.useEigenStatic = 0;
	liveVar.nEigenModes = 5;
	liveVar.eigenValues = NULL;
	liveVar.eigenModes = NULL;
	liveVar.EigenMethod = 0;
	liveVar.displayEigenMode = 1;
	liveVar.geomTransparent = 0;
	liveVar.plotAnimation = 0;
	liveVar.plotElasticityBC = 1;
	liveVar.plotThermalBC = 0;
	liveVar.plotSymmetryPlanes = 0;
	liveVar.plotOverGeom = 1;
	liveVar.plotTitle = 1;
	liveVar.tiffDPI = 300;
	liveVar.text3DFontType = 1;// text3D
	liveVar.plotSupportStructures = 0;
	liveVar.maxTimeAllowed = 360000.0;//hundred hours for topopt, default
	liveVar.estimatedTimeForTopOpt = 0;
	liveVar.cuttingPlane = NO_CUTTINGPLANE;

	liveVar.timeForOneFEA = -1;// this will be computed when FEA is executed.
#if (COMPILE_MODE == BATCH_MODE)
	liveVar.maxTimeAllowed = 99.0;// seconds
#endif

	liveVar.nSmoothingIterations = 2;
	liveVar.plotNormals = 0;
	liveVar.plotNonDesignGeometry = 1;
	liveVar.plotElementsWithDensityGreaterThan = 0.5;

	/// contour
	liveVar.smootheningMethod = 1;// 0 for non-area preserving, 1 for area preserving
	liveVar.contourFeatureTolerance = 4;
	liveVar.contourSimplificationTolerance = 2;
	liveVar.contourSmoothening = 2;
	liveVar.contourOffset = 2;

	//transient
	liveVar.newmarkGamma = 0.5;
	liveVar.newmarkBeta = 0.25;
	liveVar.dampingM = 0;
	liveVar.dampingK = 5e-4;
	liveVar.initialTemperature = 23;
	liveVar.transientTolerance = 12;// 10^(-transientTolerance)
	liveVar.transientDeltaTime = 0.0001;
	liveVar.transientTotalTime = 0.005;
	strcpy(liveVar.transientTrackPtStr, "1.0 1.0 1.0");


	//optimization
	liveVar.useAllLoads = 1;
	liveVar.desiredVolFraction = 0.5;
	liveVar.stepSize = 0.025;

	liveVar.maxDisplacementAllowed = 1e20;
	liveVar.topoptObjective = COMPLIANCE;

	liveVar.snapToGeomRelativeTol = 1.05;
	liveVar.drawDirection = 0;//0 is nothing, 1 is x, 2 is y and 3 is z
	liveVar.imposedBuildDirection = 0;//0 is nothing, +/-1 is x, +/- 2 is y and +/- 3 is z
	liveVar.throughCutDirection = 0;//0 is nothing, 1 is x, 2 is y and 3 is z
	liveVar.cyclicSymmetry = 0;
	liveVar.imposedOverHangAngle = 45;// degrees
	liveVar.modelScale = 1.0;
	liveVar.keepDirichlet = 0;
	liveVar.stressSafetyFactor = 1.0;

	liveVar.isTopOptComplete = false;
	liveVar.saveIntermediate = 0;
	liveVar.invertedIsoApproach = 0;
	strcpy(liveVar.stlFileName, "");
	liveVar.saveImagesForReport = 0;

	//lattice
	liveVar.latticeType = 0;
	liveVar.latticeUnitCellSizeInMM = 5;//mm
	liveVar.latticeResolution = 10;// how many voxel elements per side of lattice unit cell
	liveVar.latticeFillRatio = 0.5;
	liveVar.latticeSurfaceRetainType = LATTICE_SURFACE_CRITICAL;
	liveVar.latticeSmoothening = 10;
	liveVar.latticeSaveSTL = 0;
}
/************************************************************/
void oneTimeInitialization()
{
	liveVar.paretoUnits = UNITS_METER;
	liveVar.material = ALLOY_STEEL;
	liveVar.E = 2.1e11;
	liveVar.nu = 0.28;
	liveVar.Y = 500e6;//Young's modulus
	liveVar.rho = 7700.0;
	liveVar.alpha = 6e-6;
	liveVar.K = 16;//conductivity
	liveVar.specificHeat = 452;
	liveVar.useGPU = 0;
	liveVar.assemblyMode = 0;
	liveVar.nSTLFilesLoaded = 0;
	strcpy(liveVar.userName, "noName");

	liveVar.stlVerbose = 0;
	liveVar.voxVerbose = 0;
	liveVar.hexmeshVerbose = 0;
	liveVar.deflationVerbose = 0;
	liveVar.topoptVerbose = 0;

}
/************************************************************/
void cpuID(unsigned i, unsigned regs[4]) {
#ifdef _WIN32
	__cpuid((int *)regs, (int)i);

#else
	asm volatile
		("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
			: "a" (i), "c" (0));
	// ECX is set to zero for CPUID function 4
#endif
}
/************************************************************/
void setOMPThreads()
{
	int nProcessors = omp_get_max_threads();
	printf("nProcessors: %d\n", nProcessors);

	// Get vendor
	unsigned regs[4];
	char vendor[12];
	cpuID(0, regs);
	((unsigned *)vendor)[0] = regs[1]; // EBX
	((unsigned *)vendor)[1] = regs[3]; // EDX
	((unsigned *)vendor)[2] = regs[2]; // ECX
	string cpuVendor = string(vendor, 12);

	// Get CPU features
	cpuID(1, regs);
	unsigned cpuFeatures = regs[3]; // EDX

	// Logical core count per CPU
	cpuID(1, regs);
	unsigned logical, cores;

	if (cpuVendor == "GenuineIntel") {
		// Get DCP cache info
		cpuID(4, regs);
		printf("Intel: ");
		logical = nProcessors;
		cores = logical / 2;

	}
	else if (cpuVendor == "AuthenticAMD") {
		// Get NC: Number of CPU cores - 1
		cpuID(0x80000008, regs);
		printf("AMD: ");
		logical = nProcessors;
		cores = logical;
	}


	// Detect hyper-threads  
	bool hyperThreads = cpuFeatures & (1 << 28) && cores < logical;

	printf("hyper-threads: %s\n", (hyperThreads ? "true" : "false"));
	omp_set_dynamic(1);
	omp_set_num_threads((int)cores);

	printf("logical cpus: %d; ", logical);
	printf("cpu cores: %d\n", cores);

}

/************************************************************/
void initializeLiveVariables()
{
	// initialize  GUI live variables

	liveVar.sciartHomeDirectory = (char *)malloc(512 * sizeof(char));

	/*
	char * pPath;

	pPath = getenv ("SCIARTHOME");

	if (pPath==NULL) {
		strcpy(liveVar.sciartHomeDirectory,".\\");
	}
	else
		strcpy(liveVar.sciartHomeDirectory,pPath);

#ifdef _WIN32
	if (_chdir(liveVar.sciartHomeDirectory) == -1) {
		printf("%s does not exist!\n", liveVar.sciartHomeDirectory);
		strcpy(liveVar.sciartHomeDirectory,".\\");
	}
#endif
	*/
	strcpy(liveVar.sciartHomeDirectory, ".\\");//always current directory
	printf("Home Directory: %s \n", liveVar.sciartHomeDirectory);
	liveVar.projectDirectory = (char *)malloc(512 * sizeof(char));
	liveVar.reportFolder = (char *)malloc(512 * sizeof(char));

	liveVar.userMessage = (char *)malloc(512 * sizeof(char));
	strcpy(liveVar.userMessage, "Ready to load geometry or project ...");

	printf("---------------------------------------------\n");
	printf("Initializing version %.2f\n", PARETO_VERSION);

	//setOMPThreads();
	vox.setMaxVoxels(MAX_VOXELS);

	liveVar.transientTrackPtStr = (char *)malloc(512 * sizeof(char));
	strcpy(liveVar.transientTrackPtStr, "1.0 1.0 1.0");

	for (int i = 0; i < MAX_STL_PARTS; i++) {
		liveVar.geomFileName[i] = (char *)malloc(512 * sizeof(char));
		strcpy(liveVar.geomFileName[i], "");
	}

	liveVar.modelName = (char *)malloc(512 * sizeof(char));
	strcpy(liveVar.modelName, "");
	liveVar.voxFileName = (char *)malloc(512 * sizeof(char));
	strcpy(liveVar.voxFileName, "");
	liveVar.prjFileName = (char *)malloc(512 * sizeof(char));
	strcpy(liveVar.prjFileName, "");
	liveVar.stlFileName = (char *)malloc(512 * sizeof(char));


#ifdef COMPILE_GPUCODE
	liveVar.useGPU = 1;
	if (isThereValidCudaDevice() == 1) {
		liveVar.useGPU = 1;
		printf("Using GPU device for all FEA \n");
	}
	else
		printf("There is no valid NVidia GPU device; using CPU for FEA\n");
#endif
	resetLiveVariables();
	printf("Max mesh size allowed is: %ld \n", vox.getMaxVoxels());
	printf("---------------------------------------------\n");
}
/**********************************************************************/
void safeFree(void *ptr)
{
	if (ptr) free(ptr);
}
/************************************************************/
bool checkForFiniteForce()
{
	int i, bcSet, faceID;

	bcSet = liveVar.currentBCSet;
	for (faceID = 0; faceID < vox.getNumTriGroups(); faceID++) {
		for (i = 0; i < 3; i++) {
			if ((BCType[bcSet][faceID][i] == ELASTICITY_NEUMANN) && (fabs(BCValue[bcSet][faceID][i]) > 0))
				return true;// at least one non-zero force
			if ((BCType[bcSet][faceID][i] == ELASTICITY_TORQUE) && (fabs(BCValue[bcSet][faceID][i]) > 0))
				return true;// at least one non-zero force
			if ((BCType[bcSet][faceID][i] == ELASTICITY_PRESSURE) && (fabs(BCValue[bcSet][faceID][3]) > 0))
				return true;// at least one non-zero force
		}
	}
	for (i = 0; i < 4; i++)
		if (fabs(ElasticityAcceleration[bcSet][i]) > 0)
			return true;// body force present

	return false;
}
/**********************************************************************/
void  freeEigenModesMemory()
{
	if (liveVar.eigenModes != NULL) {
		for (int i = 0; i < MAX_EIGEN_MODES; i++)
			safeFree(liveVar.eigenModes[i]);
		safeFree(liveVar.eigenModes);
		safeFree(liveVar.eigenValues);
	}
}

/************************************************************/
bool topOptCallBackFunctionInverted(double isoValueConverged)
{
	char tempSTLFile[256], tempStr[256], objFile[256];
	indexed_mesh mesh;
	double meshSize, volFraction;
	meshSize = topopt.getMeshElementSize();

	strcpy(objFile, liveVar.modelName);
	strcat(objFile, "_baseModel.obj");

	vox.saveAsObj(objFile);

	volFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	strcpy(tempSTLFile, liveVar.modelName);
	sprintf(tempStr, "_%.3f.stl", volFraction);
	strcat(tempSTLFile, tempStr);

	topopt.createAndDumpIsoSurfaceFromTSfield(tempSTLFile, true);
	mesh.load_from_binary_stereo_lithography_file(tempSTLFile);
	remove(tempSTLFile);

	strcpy(objFile, liveVar.modelName);
	strcat(objFile, tempStr);
	mesh.save_to_obj_file(objFile);

	/*mesh.load_from_binary_stereo_lithography_file(tempFile);
	for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
	mesh.getVertex(i,xyz);
	vox.snapPointToGeometry(xyz,1.2*meshSize);//find the new  xyz location after snapping to geometry
	mesh.setVertex(i,xyz);//make the change only if the vertex normals have not changed much
	}
	mesh.expand_surface(1.5*meshSize);
	mesh.taubin_smooth(0.6307, -0.6734,5);
	// save as obj
	mesh.save_to_obj_file("InverseIso.obj");
	// construct boolean difference
	system(".\\MeshToolsTestApp.exe .\\BaseModel.obj .\\InverseIso.obj .\\Difference.stl 2 .\\MeshInfoReport.txt");


	if (liveVar.snapIsoToGeom) {
	mesh.taubin_smooth(0.6307, -0.6734,5);
	for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
	mesh.getVertex(i,xyz);
	vox.snapPointToGeometry(xyz,0.75*meshSize);//find the new  xyz location after snapping to geometry
	mesh.setVertex(i,xyz);//make the change only if the vertex normals have not changed much
	}
	mesh.taubin_smooth(0.6307, -0.6734,15);
	}
	else
	mesh.taubin_smooth(0.6307, -0.6734,15);

	if (1) {// extended off file
	mesh.save_to_extended_off_file(offFile);
	topopt.readExtendedOffFile(liveVar.currentIsoCounterToPlot,offFile);
	remove(offFile);
	}
	else {// standard binary
	mesh.save_to_binary_stereo_lithography_file(tempFile);
	topopt.readIsoBinary(liveVar.currentIsoCounterToPlot,tempFile);
	remove(tempFile);
	}
	*/
	setPlotRoutine(0);


	return true;
}
/************************************************************/
bool latticeDesignCallBackFunction(double param)
{
	cout << "Percent complete: " << 100 * param << endl;
	return true;

}
/************************************************************/
bool topOptCallBackFunction(bool saveIsoSurface)
{
	char stlFile[256], offFile[512], tempStr[512], offFileTemp[512];
	indexed_mesh mesh;
	double xyz[3];
	double meshSize, volFrac, newEstimatedTimeForCompletion;
	int isoSurfaceExtractionMethod, nSmootheningSteps;
	Timer tempTimer;

	if (saveIsoSurface) {
		meshSize = topopt.getMeshElementSize();
		volFrac = topopt.getCurrentVolumeFractionOfOptimizedParts();
		newEstimatedTimeForCompletion = liveVar.estimatedTimeForTopOpt*volFrac;
		printEstimatedTime(newEstimatedTimeForCompletion, tempStr);
		sprintf(liveVar.userMessage, tempStr);

		if (topopt.getNumElems() >= 100000)
			isoSurfaceExtractionMethod = 0;//uses the mesh surface directly, not smooth, but connected, for large number of elements, lack of smoothness is not as bad
		else
			isoSurfaceExtractionMethod = 1;// uses the topopt isosurface, smooth, but may not be connected

		isoSurfaceExtractionMethod = 0;

		//printf("\n>>Creating isosurface at vol. fraction of %.3f \n",volFrac);
		strcpy(stlFile, liveVar.modelName);
		sprintf(tempStr, ".%.3f.stl", volFrac);
		strcat(stlFile, tempStr);
		strcpy(offFile, liveVar.modelName);
		sprintf(tempStr, ".%.3f.off", volFrac);
		strcat(offFile, tempStr);

		strcpy(offFileTemp, offFile);
		strcat(offFileTemp, ".txt");
		nSmootheningSteps = 15;
		if (isoSurfaceExtractionMethod == 0) {// mesh based
			topopt.createAndDumpIsoSurfaceFromMesh(stlFile);// not smooth, but connected	
		}
		else {
			topopt.createAndDumpIsoSurfaceFromTSfield(stlFile);// smooth, but may not be connected
		}
		mesh.load_from_binary_stereo_lithography_file(stlFile);
		remove(stlFile);

		mesh.taubin_smooth(0.6307, -0.6734, nSmootheningSteps);
		for (int i = 0; i < mesh.getNumberOfVertices(); i++) {
			mesh.getVertex(i, xyz);
			vox.snapPointToGeometry(xyz, liveVar.snapToGeomRelativeTol*meshSize);//find the new  xyz location after snapping to geometry
			mesh.setVertex(i, xyz);//make the change only if the vertex normals have not changed much
		}
		
		mesh.taubin_smooth(0.6307, -0.6734, nSmootheningSteps);
#if (COMPILE_MODE != BATCH_MODE)
		if (liveVar.saveIntermediate) {
			printf("\n>>Saving isosurface at vol. fraction of %.3f \n", volFrac);
			mesh.save_to_binary_stereo_lithography_file(stlFile);// useful if there is a software crash
		}
#endif
		//cout << "\nSaving off file " << topopt.getCurrentIsoCounter() << endl;
		bool saveAsOff;
		saveAsOff = true;
		if (saveAsOff) {
			mesh.save_to_extended_off_file(offFile);
			topopt.readExtendedOffFile(offFile);
			remove(offFile);
		}
		else {
			mesh.save_to_binary_stereo_lithography_file(stlFile);
			topopt.readIsoBinary(stlFile);
			remove(stlFile);
		}
		
		liveVar.displayVolFraction = volFrac;
	}// end of save isosurface
	setPlotRoutine(0);

#if (COMPILE_MODE == BATCH_MODE)

	double actualFrac, time, isoCutoff;
	int isoCounter;
	char tempFile[256];
	//save isosurface at intermediate topologies

	isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction, &actualFrac, &isoCutoff, &time);
	sprintf(stlFile, CLIENTFILES_STL, liveVar.userName, actualFrac);

	mesh.save_to_binary_stereo_lithography_file(stlFile);
	sprintf(tempFile, CLIENTFILES, liveVar.userName);
	strcat(tempFile, "_currentVolFraction.txt");
	FILE *fptr = fopen(tempFile, "w");
	if (fptr != NULL) {
		fprintf(fptr, "%.3f", actualFrac);
		fclose(fptr);
	}

	// check for stop by user file
	sprintf(tempFile, CLIENTFILES, liveVar.userName);
	strcat(tempFile, "_stop.txt");
	fptr = fopen(tempFile, "r");
	if (fptr != NULL) {
		printf("\nOptimization stopped by user \n");
		fclose(fptr);
		return false;// file found
	}
#else
	// For non-batchmode we can stop using key strokes
	if (GetAsyncKeyState(VK_F8)) {
		guiState = TOPOPT_SUCCESFUL;
		alert("Topopt stopped by user\n");
		return false;
	}
#endif

	return true;
}

/************************************************************/
void allocateMemoryForEigenModes()
{
	freeEigenModesMemory();
	liveVar.eigenModes = (double **)malloc(MAX_EIGEN_MODES * sizeof(double *));
	liveVar.eigenValues = (double *)malloc(MAX_EIGEN_MODES * sizeof(double));
	for (int i = 0; i < MAX_EIGEN_MODES; i++)
		liveVar.eigenModes[i] = (double *)malloc(3 * topopt.getNumNodes() * sizeof(double));

}
/************************************************************/
bool callBackFunctionForSTLGeomRead(double fractionCompleted)
{
	return true;// user can quit
}
/************************************************************/
int loadGeom(bool assembly)
{
	char *pch;
	int success;
	double STLVolume, STLArea, maxLengthTypical, minLengthTypical, xyzMin[3], xyzMax[3];
	double maxLengthInMeters, minLengthInMeters;

	resetLiveVariables();
	topopt.removeAllMaterials();
	topopt.InitializeAllAFElasticityVariables();
	topopt.InitializeTopOpt(1);// to clear up TS memory
	//if (liveVar.usingGUI) glui->sync_live();
	if ((liveVar.nSTLFilesLoaded == 0) || (!assembly))
		vox.deleteCurrentModel();
	printf("---------------------------------------------\n");
	printf("Loading file %s \n", liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
	if (assembly) printf("... in assembly mode \n");
	success = 0;


	success = vox.readSTLGeom(liveVar.geomFileName[liveVar.nSTLFilesLoaded], liveVar.useGPU, assembly, callBackFunctionForSTLGeomRead);

	if (success != 1) {
		printf("STL loading unsuccesful\n");
		printf("---------------------------------------------\n");
		if (liveVar.usingGUI)
			Message(NULL, "Invalid .stl file, please repair and reload", "Pareto", MB_OK);

		strcpy(liveVar.userMessage, "Geometry loading unsuccessul");
		return 0;
	}
	vox.resetGroups();
	vox.resetHighlight(); // unselect all highlighted triangles
	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	printf("#Triangles = %d; #Nodes = %d \n", vox.getNumTriangles(), vox.getNumTriPoints());
	printf("STL volume = %e; STL Area = %e \n", STLVolume, STLArea);

	liveVar.modelScale = vox.getModelScale();


	maxLengthTypical = 5;// meters
	minLengthTypical = 0.001;// meters
	vox.getBoundingBox(xyzMin, xyzMax);
	vox.getMinAndMaxLengths(&minLengthInMeters, &maxLengthInMeters);

	if (liveVar.paretoUnits == UNITS_METER) {
		maxLengthInMeters *= 1.0;
		minLengthInMeters *= 1.0;
		if (maxLengthInMeters > maxLengthTypical) {
			if (maxLengthInMeters / 39.36 < maxLengthTypical)
				alert("The part is large in meter units; suggested units: inches.");
			else if (maxLengthInMeters / 1000.0 < maxLengthTypical)
				alert("The part is large in meter units; suggested units: mm.");
			else
				alert("Please check the units and model; the part appears to be too large in meters");
		}
		else if (minLengthInMeters < minLengthTypical)
			alert("Please check the units and model; the part appears to be too small in meters");// the part is too small in meters, no other likely units
	}
	else if (liveVar.paretoUnits == UNITS_INCH) {
		maxLengthInMeters *= 0.0254;
		minLengthInMeters *= 0.0254;
		if (maxLengthInMeters > maxLengthTypical) {
			if (maxLengthInMeters*39.36 / 1000.0 < maxLengthTypical)
				alert("The part is large in inch units; suggested units: mm.");
			else
				alert("Please check the units; the part appears to be too large in inches");
		}
		if (minLengthInMeters < minLengthTypical) {
			if (minLengthInMeters*39.36 > minLengthTypical)
				alert("The part is small in inch units; suggested units: meters.");
			else
				alert("Please check the units; the part appears to be too small in inches");// the part is too small in meters, no other likely units
		}
	}
	else if (liveVar.paretoUnits == UNITS_MM) {
		maxLengthInMeters *= 0.001;
		minLengthInMeters *= 0.001;
		if (maxLengthInMeters > maxLengthTypical)
			alert("Please check the units and model; the part appears to be too large in mm.");// the part is too small in meters, no other likely units
		if (minLengthInMeters < minLengthTypical) {
			if (minLengthInMeters*39.36 > minLengthTypical)
				alert("The part is small in mm units; suggested units: inches.");
			else if (minLengthInMeters * 1000 > minLengthTypical)
				alert("The part is small in mm units; suggested units: meters.");
			else
				alert("Please check the units and model; the part appears to be too small in mm");// the part is too small in meters, no other likely units
		}
	}

	printf("--------------------------------------------------\n");
	printf("CoreFunctions: Model length = %.5f\n", liveVar.modelScale);
	printf("CoreFunctions: Model length = %.5f\n", liveVar.modelScale);


	int nElementsRecommended = vox.getRecommendedNumberOfElements();
	nElementsRecommended = MAX(nElementsRecommended, 10000);
	printf("CoreFunctions: Recommended elements = %d\n", nElementsRecommended);


	liveVar.nElemsToMesh = nElementsRecommended;

	liveVar.meshQuality = MESH_VERY_COARSE;
	if (liveVar.nElemsToMesh > 10000)
		liveVar.meshQuality = MESH_COARSE;
	if (liveVar.nElemsToMesh > 25000)
		liveVar.meshQuality = MESH_MEDIUM;
	if (liveVar.nElemsToMesh > 50000)
		liveVar.meshQuality = MESH_FINE;
	if (liveVar.nElemsToMesh > 100000)
		liveVar.meshQuality = MESH_VERY_FINE;

	int nDeflationGroups;
	nDeflationGroups = NEARESTINT(MAX(nElementsRecommended / 750.0, 64));
	liveVar.nWorkSpaceGroups = nDeflationGroups;

	//modelName ... strip all path data and extension
	strcpy(liveVar.modelName, liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
	strcpy(liveVar.projectDirectory, liveVar.modelName);

	while (pch = strstr(liveVar.modelName, "\\")) // strip \ (backslash)
		strcpy(liveVar.modelName, pch + 1);

	// modelName now contains the project name with extension
	// remove the project name from directory
	pch = strstr(liveVar.projectDirectory, liveVar.modelName);
	strncpy(pch, "\0", 1);// strip the name of project
	printf("Project directory: %s\n", liveVar.projectDirectory);

	if (assembly)
		liveVar.nSTLFilesLoaded++;
	else
		liveVar.nSTLFilesLoaded = 1;

	if (vox.getNumParts() > 1) liveVar.nodeBasedKU = 0;

	pch = strstr(liveVar.modelName, ".STL");
	if (pch) strncpy(pch, "\0", 1);// strip .STL extension
	pch = strstr(liveVar.modelName, ".stl");
	if (pch) strncpy(pch, "\0", 1);// strip .stl extension
	//Set plotting routine mesh

	if (liveVar.usingGUI) {
		pareto_setViewRoutine(VIEW_XYPLUS);
		pareto_setViewRoutine(VIEW_ISO);// helps zoom to model
		guiState = GEOM_SUCCESFUL;
		liveVar.displayMode = DISPLAY_GEOM;
		setPlotRoutine(1);
	}
	strcpy(liveVar.voxFileName, liveVar.modelName);
	strcat(liveVar.voxFileName, ".vox");
	strcpy(liveVar.stlFileName, liveVar.modelName);
	strcat(liveVar.stlFileName, ".stl");
	strcpy(liveVar.prjFileName, liveVar.modelName);
	strcat(liveVar.prjFileName, "Project.prj");

	// For topology otpimization, set the maximum allowable values
	liveVar.maxDisplacementAllowed = 1e-2*liveVar.modelScale;

	printf("---------------------------------------------\n");
	strcpy(liveVar.userMessage, "Ready to apply material ...");

	return 1;
}
/************************************************************/
bool updateGeom(bool assembly)
{

	// This is used to load a new geometry file, while carrying over boundary conditions
	char *pch;
	int success, bcSet, i, j;
	double STLVolume, STLArea, volumeInCm3;
	int BCTypeTemp[MAX_BC_SET][STL_MAX_GROUPS][3];
	double BCValueTemp[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
	int ThermalBCTypeTemp[MAX_BC_SET][STL_MAX_GROUPS];
	double ThermalBCAreaTemp[MAX_BC_SET][STL_MAX_GROUPS];
	double ThermalBCValueTemp[MAX_BC_SET][STL_MAX_GROUPS];
	double ThermalBCRefTemp[MAX_BC_SET][STL_MAX_GROUPS];// for convection and radiation

	if (guiState < BC_SUCCESFUL) {
		printf("BC must be first applied before updating geom \n");
		return false;
	}

	voxInitialDesign.copySTLGeometry(vox);//copy the current geom into voxInitialDesign

	// save all the BC
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < STL_MAX_GROUPS; i++) {//default all BC are free Neumann
			for (j = 0; j < 3; j++) {
				BCTypeTemp[bcSet][i][j] = BCType[bcSet][i][j];
				BCValueTemp[bcSet][i][j] = BCValue[bcSet][i][j];
			}
		}
	}
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < STL_MAX_GROUPS; i++) {//default all BC are free Neumann
			ThermalBCTypeTemp[bcSet][i] = ThermalBCType[bcSet][i];
			ThermalBCAreaTemp[bcSet][i] = ThermalBCArea[bcSet][i];
			ThermalBCValueTemp[bcSet][i] = ThermalBCValue[bcSet][i];
			ThermalBCRefTemp[bcSet][i] = ThermalBCRef[bcSet][i];
		}
	}

	resetLiveVariables();

	// copy back all the BC
	for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
		for (i = 0; i < STL_MAX_GROUPS; i++) {
			for (j = 0; j < 3; j++) {
				BCType[bcSet][i][j] = BCTypeTemp[bcSet][i][j];
				BCValue[bcSet][i][j] = BCValueTemp[bcSet][i][j];
			}
		}
	}
	if ((liveVar.nSTLFilesLoaded == 0) || (!assembly))
		vox.deleteCurrentModel();
	printf("---------------------------------------------\n");
	printf("Loading file %s \n", liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
	if (assembly) printf("... in assembly mode \n");
	success = 0;

	success = vox.readSTLGeom(liveVar.geomFileName[liveVar.nSTLFilesLoaded], liveVar.useGPU, assembly, callBackFunctionForSTLGeomRead);
	if (success != 1) {
		printf("STL loading unsuccessful\n");
		printf("---------------------------------------------\n");
		if (liveVar.usingGUI)
			Message(NULL, "Invalid .stl file, please repair and reload", "Pareto", MB_OK);
		return false;
	}
	vox.resetGroups();
	vox.resetHighlight(); // unselect all highlighted triangles
	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	printf("#Triangles = %d; #Nodes = %d \n", vox.getNumTriangles(), vox.getNumTriPoints());
	printf("STL volume = %e; STL Area = %e \n", STLVolume, STLArea);

	if (liveVar.paretoUnits == UNITS_METER)
		volumeInCm3 = STLVolume*1000000.0;

	else if (liveVar.paretoUnits == UNITS_MM)
		volumeInCm3 = STLVolume*0.001;

	else if (liveVar.paretoUnits == UNITS_INCH)
		volumeInCm3 = STLVolume*2.54*2.54*2.54;

	printf("--------------------------------------------------\n");
	printf("Model volume (cm^3) = %.5f\n", volumeInCm3);
	printf("--------------------------------------------------\n");
	if ((volumeInCm3 < 0.1) && (liveVar.usingGUI))
		Message(NULL, "Model appears to be too SMALL.\nPlease check units & reload", "Pareto", MB_OK);

	if ((volumeInCm3 > 500) && (liveVar.usingGUI))
		Message(NULL, "Model appears to be too LARGE.\nPlease check units & reload", "Pareto", MB_OK);

	int nElementsRecommended = vox.getRecommendedNumberOfElements();
	nElementsRecommended = MAX(nElementsRecommended, 4000);
	//printf("Recommended minimum size = %d\n",nElementsRecommended);

	//modelName ... strip all path data and extension
	liveVar.nElemsToMesh = nElementsRecommended;
	strcpy(liveVar.modelName, liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
	strcpy(liveVar.projectDirectory, liveVar.modelName);

	while (pch = strstr(liveVar.modelName, "\\")) // strip \ (backslash)
		strcpy(liveVar.modelName, pch + 1);

	// modelName now contains the project name with extension
	// remove the project name from directory
	pch = strstr(liveVar.projectDirectory, liveVar.modelName);
	strncpy(pch, "\0", 1);// strip the name of project
	printf("Project directory: %s\n", liveVar.projectDirectory);

	if (assembly)
		liveVar.nSTLFilesLoaded++;
	else
		liveVar.nSTLFilesLoaded = 1;

	pch = strstr(liveVar.modelName, ".STL");
	if (pch) strncpy(pch, "\0", 1);// strip .STL extension
	pch = strstr(liveVar.modelName, ".stl");
	if (pch) strncpy(pch, "\0", 1);// strip .stl extension

	// Now find closest triangles to copy over groups
	vox.transferClosestTriGroups(voxInitialDesign);
	//Set plotting routine mesh

	liveVar.modelScale = topopt.getModelScale();
	guiState = BC_SUCCESFUL;
	if (liveVar.usingGUI) {
		pareto_setViewRoutine(VIEW_XYPLUS);
		pareto_setViewRoutine(VIEW_ISO);// helps zoom to model
		liveVar.displayMode = DISPLAY_GEOM;
		setPlotRoutine(1);
	}
	strcpy(liveVar.voxFileName, liveVar.modelName);
	strcat(liveVar.voxFileName, ".vox");
	strcpy(liveVar.stlFileName, liveVar.modelName);
	strcat(liveVar.stlFileName, ".stl");
	//strcpy(liveVar.prjFileName,liveVar.modelName);
	//strcat(liveVar.prjFileName,".prj");

	// For topology otpimization, set the maximum allowable values
	liveVar.maxDisplacementAllowed = 1e-2*liveVar.modelScale;

	printf("---------------------------------------------\n");
	return true;
}
/************************************************************/
int loadDMatrices(char fileName[])
{
	int m, i, numDMatrices, matId;
	double D[36];

	printf("---------------------------------------------\n");

	FILE *filein = fopen(fileName, "r");
	if (filein == 0) {
		printf("Could not find %s \n", fileName);
		return 0;
	}
	printf("Reading Dmatrix file %s \n", fileName);
	fscanf(filein, "%d", &numDMatrices);
	printf("Num of DMatrices = %d \n", numDMatrices);
	for (m = 0; m < numDMatrices; m++) {
		fscanf(filein, "%d", &matId);
		for (i = 0; i < 36; i++)
			fscanf(filein, "%lf", &(D[i]));
		if (!topopt.setDMatrix(matId, D))
			return 0;
	}
	fclose(filein);
	printf("---------------------------------------------\n");
	liveVar.nodeBasedKU = 0;// node based KU not allowed anymore
	topopt.setTStoAnalytical(false); // cannot use analytical TS anymore
	return 1;
}
/************************************************************/
void changeTopOptBC()
{
	topopt.resetBC();
	int bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);// sets h_f
	}
	topopt.setElasticityAcceleration(ElasticityAcceleration[bcSet]);// add body force to h_f
}
//--------------------------------------------------------------------------
void eliminateEmptyGroups(void)
{
	bool groupUsed[STL_MAX_GROUPS];
	int mapping[STL_MAX_GROUPS];

	printf("Eliminating empty groups\n");

	for (int g = 0; g < STL_MAX_GROUPS; g++) {
		groupUsed[g] = false;
		mapping[g] = -1;
	}

	groupUsed[0] = true;// always needed
	for (int tri = 0; tri < vox.getNumTriangles(); tri++)
		groupUsed[vox.getTriGroup(tri)] = true;

	int counter = 0;
	for (int g = 0; g < STL_MAX_GROUPS; g++) {
		if (groupUsed[g]) {
			mapping[g] = counter;
			counter++;
		}
	}

	int oldGroup, newGroup;
	int newMaxGroupNumber = 0;
	for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
		oldGroup = vox.getTriGroup(tri);
		newGroup = mapping[oldGroup];
		vox.setTriGroup(tri, newGroup);
		newMaxGroupNumber = MAX(newMaxGroupNumber, newGroup);
	}
	vox.setNumTriGroups(newMaxGroupNumber + 1);
	for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++) {
		for (int oldGroup = 0; oldGroup < STL_MAX_GROUPS; oldGroup++) {
			newGroup = mapping[oldGroup];
			if (newGroup == -1) continue;
			for (int j = 0; j < 3; j++)
				BCType[bcSet][newGroup][j] = BCType[bcSet][oldGroup][j];

			for (int j = 0; j < 6; j++)
				BCValue[bcSet][newGroup][j] = BCValue[bcSet][oldGroup][j];


			ThermalBCType[bcSet][newGroup] = ThermalBCType[bcSet][oldGroup];
			ThermalBCArea[bcSet][newGroup] = ThermalBCArea[bcSet][oldGroup];
			ThermalBCValue[bcSet][newGroup] = ThermalBCValue[bcSet][oldGroup];
			ThermalBCRef[bcSet][newGroup] = ThermalBCRef[bcSet][oldGroup];
		}
	}
}
/************************************************************/
bool hasMaterialBeenApplied(void)
{
	char tempStr[256];
	for (int part = 0; part < vox.getNumParts(); part++) {
		printf("part %d material: %d \n", part, topopt.getMaterialIDofPart(part));
		if (topopt.getMaterialIDofPart(part) < 0) {
			sprintf(tempStr, "Please apply material to part %d", part);
			printf("%s\n", tempStr);
			strcpy(liveVar.userMessage, tempStr);
			Message(NULL, tempStr, "Pareto", MB_OK);
			return false;
		}
	}
	return true;
}
/************************************************************/
void applyElasticityBC()
{
	int i, j, bcSet, currentGroupNum;
	int oldGroup;
	double dir[3], nrm, area, averageNormal[3], cylinderAxis[3], axisPoint[3], cylinderRadius, triNormal[3], cutoff;
	bcSet = liveVar.currentBCSet;
	liveVar.nBCSet = MAX(liveVar.nBCSet, bcSet + 1);

	if (!hasMaterialBeenApplied()) return;

	currentGroupNum = vox.getNumTriGroups();// 0 is default for all
	// Need to make sure that BC associated with a different BCSet are not lost in the
	// reassignment of group numbers
	// Walk thru all highlighted triangles. If a previous group exists copy that BC to the
	// new group

	bool anyTriangleSelected;


	// for multiple load set, if a force is applied on a set of triangles  for one load set
	// then the same set of triangles must be chosen for other load sets
	liveVar.plotElasticityBC = 1;
	liveVar.plotThermalBC = 0;
	anyTriangleSelected = false;
	for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
		if (vox.isTriangleHighlighted(tri)) {
			anyTriangleSelected = true;
			oldGroup = vox.getTriGroup(tri);
			for (i = 0; i < liveVar.nBCSet; i++) {
				for (j = 0; j < 3; j++)
					BCType[i][currentGroupNum][j] = BCType[i][oldGroup][j];
				for (j = 0; j < 6; j++)
					BCValue[i][currentGroupNum][j] = BCValue[i][oldGroup][j];

				ThermalBCType[i][currentGroupNum] = ThermalBCType[i][oldGroup];
				ThermalBCArea[i][currentGroupNum] = ThermalBCArea[i][oldGroup];
				ThermalBCValue[i][currentGroupNum] = ThermalBCValue[i][oldGroup];
				ThermalBCRef[i][currentGroupNum] = ThermalBCRef[i][oldGroup];
			}
		}
	}

	if (!anyTriangleSelected) {
		printf("Please select a triangle to apply boundary condition\n");
		return;
	}
	SURFACE_TYPE surfaceType = vox.assignHighlightedTrianglesToGroup(currentGroupNum, averageNormal, &area,
		cylinderAxis, axisPoint, &cylinderRadius);

	//printf("Current group number is %d \n",currentGroupNum);
	// now set the BC matrix
	//printf("Type of surface selected: %d \n",surfaceType);
	for (j = 0; j < 3; j++) {//default all BC are free Neumann
		BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
		BCValue[bcSet][currentGroupNum][j] = 0;
	}
	switch (liveVar.ParetoBCType) {
	case PARETO_BC_CLEAR:
		for (j = 0; j < 3; j++) {
			BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
			BCValue[bcSet][currentGroupNum][j] = 0;
		}
		break;
	case PARETO_BC_FIXED:
		for (j = 0; j < 3; j++) {
			BCType[bcSet][currentGroupNum][j] = ELASTICITY_DIRICHLET;
			BCValue[bcSet][currentGroupNum][j] = 0;
		}
		// copy the Dirichlet to all other BC sets
		// We cannot allow different Dirichlet for different loads
		for (i = 1; i < MAX_BC_SET; i++) {
			for (j = 0; j < 3; j++)
				BCType[i][currentGroupNum][j] = ELASTICITY_DIRICHLET;
			for (j = 0; j < 6; j++)
				BCValue[i][currentGroupNum][j] = 0;
		}
		break;
	case PARETO_BC_XFIXED:
		BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
		BCValue[bcSet][currentGroupNum][0] = 0;
		// copy the Dirichlet to all other BC sets
		// We cannot allow different Dirichlet for different loads
		for (i = 1; i < MAX_BC_SET; i++) {
			BCType[i][currentGroupNum][0] = ELASTICITY_DIRICHLET;
			BCValue[i][currentGroupNum][0] = 0;
		}
		break;
	case PARETO_BC_YFIXED:
		BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
		BCValue[bcSet][currentGroupNum][1] = 0;
		// copy the Dirichlet to all other BC sets
		// We cannot allow different Dirichlet for different loads
		for (i = 1; i < MAX_BC_SET; i++) {
			BCType[i][currentGroupNum][1] = ELASTICITY_DIRICHLET;
			BCValue[i][currentGroupNum][1] = 0;
		}
		break;
	case PARETO_BC_ZFIXED:
		BCType[bcSet][currentGroupNum][2] = ELASTICITY_DIRICHLET;
		BCValue[bcSet][currentGroupNum][2] = 0;
		// copy the Dirichlet to all other BC sets
		// We cannot allow different Dirichlet for different loads
		for (i = 1; i < MAX_BC_SET; i++) {
			BCType[i][currentGroupNum][2] = ELASTICITY_DIRICHLET;
			BCValue[i][currentGroupNum][2] = 0;
		}
		break;
	case PARETO_BC_FORCE:
		for (j = 0; j < 3; j++)
			BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
		BCValue[bcSet][currentGroupNum][0] = liveVar.XForce;
		BCValue[bcSet][currentGroupNum][1] = liveVar.YForce;
		BCValue[bcSet][currentGroupNum][2] = liveVar.ZForce;
		break;
	case PARETO_BC_NORMAL_FORCE:
		if ((surfaceType != PLANAR)) {
			if (liveVar.usingGUI)
				Message(NULL, "Currently, normal force can only be applied on planar faces\n", "Pareto", MB_OK);
			else
				printf("Currently, normal force can only be applied on planar faces\n");
			break;
		}
		dir[0] = -averageNormal[0];
		dir[1] = -averageNormal[1];
		dir[2] = -averageNormal[2];

		// store the values as follows
		// dx dy dz pressure area 0 

		nrm = NORM3D(dir) + 1e-12;
		for (j = 0; j < 3; j++) {
			dir[j] /= nrm;
			BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
			BCValue[bcSet][currentGroupNum][j] = liveVar.NormalForce*dir[j];
		}

		break;
	case PARETO_BC_PRESSURE:
		if (surfaceType == PLANAR) {
			dir[0] = -averageNormal[0];
			dir[1] = -averageNormal[1];
			dir[2] = -averageNormal[2];
			// store the values as follows
			// direction (dx dy dz) pressure area 0 
			nrm = NORM3D(dir) + 1e-12;
			for (j = 0; j < 3; j++) {
				dir[j] /= nrm;
				BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;
				BCValue[bcSet][currentGroupNum][j] = dir[j];
			}
			BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
			BCValue[bcSet][currentGroupNum][4] = area;
			BCValue[bcSet][currentGroupNum][5] = -1;
		}
		else if (surfaceType == CYLINDER) {
			nrm = NORM3D(cylinderAxis);
			dir[0] = cylinderAxis[0] / nrm;
			dir[1] = cylinderAxis[1] / nrm;
			dir[2] = cylinderAxis[2] / nrm;
			if ((abs(dir[0]) < 0.99) && (abs(dir[1]) < 0.99) && (abs(dir[2]) < 0.99)) {
				if (liveVar.usingGUI)
					Message(NULL, "Pressure BC: the cylinder axis must be either x, y or z axis\n", "Pareto", MB_OK);
				break;
			}
			for (j = 0; j < 3; j++)
				BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;

			for (j = 0; j < 3; j++)
				BCValue[bcSet][currentGroupNum][j] = axisPoint[j];// additional information needed for cylinder

			BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
			BCValue[bcSet][currentGroupNum][4] = area;
			if (dir[0] >= 0.99)
				BCValue[bcSet][currentGroupNum][5] = 1;
			else if (dir[1] >= 0.99)
				BCValue[bcSet][currentGroupNum][5] = 2;
			else
				BCValue[bcSet][currentGroupNum][5] = 3;

		}
		else {
			// store the values as follows
			// direction (0 0 0) pressure area 0 
			// the direction will be determined inside AFElasticity based on mesh normal
			for (j = 0; j < 3; j++) {
				BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;
				BCValue[bcSet][currentGroupNum][j] = 0;
			}
			BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
			BCValue[bcSet][currentGroupNum][4] = area;
			BCValue[bcSet][currentGroupNum][5] = 0;
		}
		break;
	case PARETO_BC_RETAIN://  retain this surface during optimization
		for (j = 0; j < 3; j++) {
			BCType[bcSet][currentGroupNum][j] = ELASTICITY_RETAIN;
			BCValue[bcSet][currentGroupNum][j] = 0;
		}
		// if the Load Set is 0, copy the Retain to all other BC sets
		// We cannot allow different Dirichlet
		if (bcSet == 0) {
			for (i = 1; i < MAX_BC_SET; i++) {
				for (j = 0; j < 3; j++) {
					BCType[i][currentGroupNum][j] = ELASTICITY_RETAIN;
					BCValue[i][currentGroupNum][j] = 0;
				}
			}
		}
		break;
	case PARETO_BC_SLIDING:
		if ((surfaceType != CYLINDER) && (surfaceType != PLANAR)) {
			if (liveVar.usingGUI)
				Message(NULL, "Currently, sliding can only be applied on cylinders and planar surfaces \n", "Pareto", MB_OK);
			break;
		}
		if (surfaceType == PLANAR) {
			dir[0] = -averageNormal[0];
			dir[1] = -averageNormal[1];
			dir[2] = -averageNormal[2];
			nrm = NORM3D(dir) + 1e-12;
			for (j = 0; j < 3; j++)
				dir[j] /= nrm;

			if (fabs(dir[0]) > 0.99) {
				BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
				BCValue[bcSet][currentGroupNum][0] = 0;

				BCType[bcSet][currentGroupNum][1] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][1] = 0;

				BCType[bcSet][currentGroupNum][2] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][2] = 0;
			}
			else if (fabs(dir[1]) > 0.99) {
				BCType[bcSet][currentGroupNum][0] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][0] = 0;

				BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
				BCValue[bcSet][currentGroupNum][1] = 0;

				BCType[bcSet][currentGroupNum][2] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][2] = 0;

			}
			else if (fabs(dir[2]) > 0.99) {
				BCType[bcSet][currentGroupNum][0] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][0] = 0;

				BCType[bcSet][currentGroupNum][1] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][1] = 0;

				BCType[bcSet][currentGroupNum][2] = ELASTICITY_DIRICHLET;
				BCValue[bcSet][currentGroupNum][2] = 0;

			}
			else {
				Message(NULL, "Currently, sliding can only be applied on planar surfaces with normal along x, y, or z \n", "Pareto", MB_OK);
				break;
			}

		}
		else if (surfaceType == CYLINDER) {
			nrm = NORM3D(cylinderAxis);
			dir[0] = cylinderAxis[0] / nrm;
			dir[1] = cylinderAxis[1] / nrm;
			dir[2] = cylinderAxis[2] / nrm;
			if ((abs(dir[0]) < 0.99) && (abs(dir[1]) < 0.99) && (abs(dir[2]) < 0.99)) {
				if (liveVar.usingGUI)
					Message(NULL, "Sliding BC: the cylinder axis must be either x, y or z axis\n", "Pareto", MB_OK);
				break;
			}
			vox.incrementTriGroups();// we will need three groups
			vox.incrementTriGroups();
			// We are now ready to apply sliding
			// The basic idea is as follows. Suppose the axis direction is x axis, then we constrain along y
			// the triangles normal to y axis, and we constrain along z the triangles normal to z

			// we will need 3 groups

			// The first two groups are Dirichlet along a particular direction
			// the third group are the triangles whose normals are not oriented along axis direction
			// these are retained
			for (i = 0; i < 3; i++) {
				BCType[bcSet][currentGroupNum][i] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum][i] = 0;

				BCType[bcSet][currentGroupNum + 1][i] = ELASTICITY_NEUMANN;
				BCValue[bcSet][currentGroupNum + 1][i] = 0;

				BCType[bcSet][currentGroupNum + 2][i] = ELASTICITY_RETAIN;
				BCValue[bcSet][currentGroupNum + 2][i] = 0;
			}

			// for the first two groups, decide which direction you are going to restrain
			if (abs(dir[0]) >= 0.99) {// axis is along x
				BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
				BCType[bcSet][currentGroupNum + 1][2] = ELASTICITY_DIRICHLET;
			}
			else if (abs(dir[1]) >= 0.99) {// axis is along y
				BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
				BCType[bcSet][currentGroupNum + 1][2] = ELASTICITY_DIRICHLET;
			}
			else if (abs(dir[2]) >= 0.99) {// axis is along z
				BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
				BCType[bcSet][currentGroupNum + 1][1] = ELASTICITY_DIRICHLET;
			}

			cutoff = cos(35 * 3.1415 / 180);// this is an arbitrary cutoff
			for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
				if (!vox.isTriangleHighlighted(tri))	continue;
				vox.getNormalOfTriangle(tri, triNormal);
				nrm = NORM3D(triNormal);
				triNormal[0] = triNormal[0] / nrm;
				triNormal[1] = triNormal[1] / nrm;
				triNormal[2] = triNormal[2] / nrm;

				if (abs(dir[0]) >= 0.99) {// axis is along x
					if (abs(triNormal[1]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum);
					else if (abs(triNormal[2]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum + 1);
					else
						vox.setTriGroup(tri, currentGroupNum + 2);// set to retain
				}
				else if (abs(dir[1]) >= 0.99) {// axis is along y
					if (abs(triNormal[0]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum);
					else if (abs(triNormal[2]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum + 1);
					else
						vox.setTriGroup(tri, currentGroupNum + 2);
				}
				else if (abs(dir[2]) >= 0.99) {// axis is along z
					if (abs(triNormal[0]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum);
					else if (abs(triNormal[1]) >= cutoff)
						vox.setTriGroup(tri, currentGroupNum + 1);
					else
						vox.setTriGroup(tri, currentGroupNum + 2);
				}
			}
		}
		break;
	case PARETO_BC_TORQUE:
		if ((surfaceType != PLANAR) && (surfaceType != CYLINDER)) {
			if (liveVar.usingGUI)
				Message(NULL, "Currently, torque can only be applied on planes or cylinders \n", "Pareto", MB_OK);
			break;
		}
		if (surfaceType == PLANAR) {
			nrm = NORM3D(averageNormal);
			dir[0] = averageNormal[0] / nrm;
			dir[1] = averageNormal[1] / nrm;
			dir[2] = averageNormal[2] / nrm;
			nrm = NORM3D(dir) + 1e-12;
			for (j = 0; j < 3; j++) {
				dir[j] /= nrm;
				BCType[bcSet][currentGroupNum][j] = ELASTICITY_TORQUE;
				BCValue[bcSet][currentGroupNum][j] = (liveVar.Torque)*dir[j];
				BCValue[bcSet][currentGroupNum][3 + j] = axisPoint[j];// additional information needed for torque
			}
		}
		else if (surfaceType == CYLINDER) {
			nrm = NORM3D(cylinderAxis);
			dir[0] = cylinderAxis[0] / nrm;
			dir[1] = cylinderAxis[1] / nrm;
			dir[2] = cylinderAxis[2] / nrm;
			nrm = NORM3D(dir) + 1e-12;
			for (j = 0; j < 3; j++) {
				dir[j] /= nrm;
				BCType[bcSet][currentGroupNum][j] = ELASTICITY_TORQUE;
				BCValue[bcSet][currentGroupNum][j] = (liveVar.Torque)*dir[j];
				BCValue[bcSet][currentGroupNum][3 + j] = axisPoint[j];// additional information needed for torque
			}
		}
		break;
	}
	printf("Elasticity boundary condition applied on face group %d \n", currentGroupNum);
	printf("---------------------------------------------\n");

	//eliminateEmptyGroups();
	guiState = BC_SUCCESFUL;
	liveVar.CurrentElasticityBCLabel = vox.getNumTriGroups();
	vox.resetHighlight();

}
/************************************************************/
void applyThermalBC()
{
	int i, bcSet, currentGroupNum;
	int oldGroup;
	double area, averageNormal[3], cylinderAxis[3], axisPoint[3], cylinderRadius;

	for (int part = 0; part < vox.getNumParts(); part++) {
		if (topopt.getMaterialIDofPart(part) < 0) {
			printf("Please apply material to part %d\n", part);
			Message(NULL, "Please apply material to all parts\n", "Pareto", MB_OK);
			return;
		}
	}

	bcSet = liveVar.currentBCSet;
	liveVar.nBCSet = MAX(liveVar.nBCSet, bcSet + 1);

	currentGroupNum = vox.getNumTriGroups();// 0 is default for all
	// Need to make sure that BC associated with a different BCSet are not lost in the
	// reassignment of group numbers
	// Walk thru all highlighted triangles. If a previous group exists copy that BC to the
	// new group

	liveVar.plotElasticityBC = 0;
	liveVar.plotThermalBC = 1;
	bool anyTriangleSelected = false;
	for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
		if (vox.isTriangleHighlighted(tri)) {
			anyTriangleSelected = true;
			oldGroup = vox.getTriGroup(tri);
			for (i = 0; i < liveVar.nBCSet; i++) {
				for (int j = 0; j < 3; j++)
					BCType[i][currentGroupNum][j] = BCType[i][oldGroup][j];
				for (int j = 0; j < 6; j++)
					BCValue[i][currentGroupNum][j] = BCValue[i][oldGroup][j];

				ThermalBCType[i][currentGroupNum] = ThermalBCType[i][oldGroup];
				ThermalBCArea[i][currentGroupNum] = ThermalBCArea[i][oldGroup];
				ThermalBCValue[i][currentGroupNum] = ThermalBCValue[i][oldGroup];
				ThermalBCRef[i][currentGroupNum] = ThermalBCRef[i][oldGroup];
			}
		}
	}
	if (!anyTriangleSelected) {
		printf("Please select a triangle to apply boundary condition\n");
		return;
	}
	SURFACE_TYPE surfaceType = vox.assignHighlightedTrianglesToGroup(currentGroupNum, averageNormal, &area,
		cylinderAxis, axisPoint, &cylinderRadius);

	// now set the BC matrix
	printf("---------------------------------------------\n");

	if (fabs(area) == 0) {
		printf("No surface was selected\n");
		printf("---------------------------------------------\n");
	}

	ThermalBCArea[bcSet][currentGroupNum] = area;
	ThermalBCType[bcSet][currentGroupNum] = THERMAL_NEUMANN;
	ThermalBCValue[bcSet][currentGroupNum] = 0;
	ThermalBCRef[bcSet][currentGroupNum] = 0;

	switch (liveVar.ThermalBCType) {
	case PARETO_BC_TEMP:
		ThermalBCType[bcSet][currentGroupNum] = THERMAL_DIRICHLET;
		ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValue;

		// if the Load Set is 0, copy the Dirichlet to all other BC sets
		// We cannot allow different Dirichlet
		if (bcSet == 0) {
			for (i = 1; i < MAX_BC_SET; i++) {
				ThermalBCType[i][currentGroupNum] = THERMAL_DIRICHLET;
				ThermalBCValue[i][currentGroupNum] = liveVar.ThermalBCValue;
			}
		}
		break;
	case PARETO_BC_HEAT:
		ThermalBCType[bcSet][currentGroupNum] = THERMAL_NEUMANN;
		ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValue;
		break;
	case PARETO_BC_CONVECTION:
		ThermalBCType[bcSet][currentGroupNum] = THERMAL_CONVECTION;
		ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValue;
		ThermalBCRef[bcSet][currentGroupNum] = liveVar.ThermalBCReference;
		break;
	case PARETO_BC_RADIATION:
		ThermalBCType[bcSet][currentGroupNum] = THERMAL_RADIATION;
		ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValue;
		ThermalBCRef[bcSet][currentGroupNum] = liveVar.ThermalBCReference;
		break;
	}
	printf("Thermal boundary condition applied on face group %d\n", currentGroupNum);
	printf("---------------------------------------------\n");
	guiState = BC_SUCCESFUL;
	liveVar.CurrentThermalBCLabel = currentGroupNum;
	vox.resetHighlight();
}

/************************************************************/
void savePRJ()
// Save the current project
{
	char *pch, materialName[256];
	int bcSet, tri, group, counter;
	double materialProperties[7];

	printf("---------------------------------------------\n");
	printf("Saving project file %s \n", liveVar.prjFileName);
	FILE *fptr = fopen(liveVar.prjFileName, "w");

	if (fptr == NULL) {
		printf("Saving project unsuccesful\n");
		printf("---------------------------------------------\n");
		return;
	}
	fprintf(fptr, "#KEY VERSION\n");
	fprintf(fptr, "%.2f\n\n", PARETO_VERSION);

	// Units should come before reading geometry

	fprintf(fptr, "#KEY UNITS\n");
	fprintf(fptr, "%d \n\n", liveVar.paretoUnits);


	fprintf(fptr, "#KEY ASSEMBLY\n");
	fprintf(fptr, "%d\n\n", liveVar.assemblyMode);// if we want to load more than one stl file

	for (int stlFile = 0; stlFile < liveVar.nSTLFilesLoaded; stlFile++) {
		pch = liveVar.geomFileName[stlFile];
		while (strstr(pch, "\\")) {// Get rid of all directory information, etc
			pch = strstr(pch, "\\");
			pch = pch + 1;
		}
		fprintf(fptr, "#KEY GEOMETRY\n");
		fprintf(fptr, "%s\n\n", pch);
	}
	// we may have one stl file with multiple parts
	// each part may have  a different material
	for (int part = 0; part < topopt.getNumberOfParts(); part++) {
		// print the corresponding material
		fprintf(fptr, "#KEY PART\n");
		fprintf(fptr, "%d\n\n", part);

		topopt.getMaterialPropertiesOfPart(part, materialProperties, materialName);

		fprintf(fptr, "#KEY MATERIALNAME\n");
		fprintf(fptr, "%s\n\n", materialName);

		fprintf(fptr, "#KEY E\n");
		fprintf(fptr, "%e \n\n", materialProperties[0]);

		fprintf(fptr, "#KEY nu\n");
		fprintf(fptr, "%0.3f \n\n", materialProperties[1]);

		fprintf(fptr, "#KEY Y\n");
		fprintf(fptr, "%e\n\n", materialProperties[2]);

		fprintf(fptr, "#KEY rho\n");
		fprintf(fptr, "%e\n\n", materialProperties[3]);

		fprintf(fptr, "#KEY K\n");
		fprintf(fptr, "%e\n\n", materialProperties[4]);

		fprintf(fptr, "#KEY alpha\n");
		fprintf(fptr, "%e\n\n", materialProperties[5]);

		fprintf(fptr, "#KEY Cp\n");
		fprintf(fptr, "%e\n\n", materialProperties[6]);


		fprintf(fptr, "#KEY MATERIAL_END\n");
		fprintf(fptr, "%d \n\n", part);

		fprintf(fptr, "#KEY NONDESIGN\n");
		fprintf(fptr, "%d \n\n", (topopt.shouldPartBeOptimized(part) ? 0 : 1));

	}
	fprintf(fptr, "#KEY TRIGROUP\n");
	counter = 0;
	for (tri = 0; tri < vox.getNumTriangles(); tri++) {
		group = vox.getTriGroup(tri);
		if (group) {
			fprintf(fptr, "%d %d ", tri, group);
			counter++;
			if (counter % 10 == 0) fprintf(fptr, "\n");// a new line
		}
	}
	fprintf(fptr, "\n-1 0 \n \n");// done
	int activeGroups[STL_MAX_GROUPS];

	vox.getActiveGroups(activeGroups);
	if (guiState >= BC_SUCCESFUL) {
		for (bcSet = 0; bcSet < liveVar.nBCSet; bcSet++) {
			fprintf(fptr, "#KEY BCSET \n");
			fprintf(fptr, "%d \n", bcSet);
			for (int j = 0; j < vox.getNumTriGroups(); j++) {
				if (!activeGroups[j]) continue;
				fprintf(fptr, "#KEY BC\n");

				fprintf(fptr, "%d %d %d %d %.3e %.3e %.3e %.3e %.3e %.3e\n", j,
					BCType[bcSet][j][0], BCType[bcSet][j][1], BCType[bcSet][j][2],
					BCValue[bcSet][j][0], BCValue[bcSet][j][1], BCValue[bcSet][j][2],
					BCValue[bcSet][j][3], BCValue[bcSet][j][4], BCValue[bcSet][j][5]);

			}
			fprintf(fptr, "\n");
			fprintf(fptr, "#KEY BCSET \n");
			fprintf(fptr, "%d \n", bcSet);
			fprintf(fptr, "#KEY ELASTICITY_ACCELERATION\n");
			fprintf(fptr, "%.3e %.3e %.3e %.3e \n", ElasticityAcceleration[bcSet][0], ElasticityAcceleration[bcSet][1], ElasticityAcceleration[bcSet][2], ElasticityAcceleration[bcSet][2]);
			fprintf(fptr, "\n");
			for (int j = 0; j < vox.getNumTriGroups(); j++) {
				if (!activeGroups[j]) continue;
				fprintf(fptr, "#KEY THERMALBC\n");
				fprintf(fptr, "%d %d %.3e %.3e %.3e\n", j, ThermalBCType[bcSet][j], ThermalBCArea[bcSet][j], ThermalBCValue[bcSet][j], ThermalBCRef[bcSet][j]);
			}

			fprintf(fptr, "\n");
		}
	}
	fprintf(fptr, "#KEY SYMMETRY\n");
	fprintf(fptr, "%d %d %d \n\n", liveVar.xSym, liveVar.ySym, liveVar.zSym);

	fprintf(fptr, "#KEY MESH\n");
	fprintf(fptr, "%d \n\n", liveVar.nElemsToMesh);

#ifdef COMPILE_GPUCODE
	if (liveVar.useGPU == 1) {
		fprintf(fptr, "#KEY USEGPU\n");
		fprintf(fptr, "%d\n\n", liveVar.useGPU);
	}
#endif

	fprintf(fptr, "#KEY NODEBASED\n");
	fprintf(fptr, "%d \n\n", liveVar.nodeBasedKU);


	fprintf(fptr, "#KEY TIMESTEP\n");
	fprintf(fptr, "%0.5e %.5e\n\n", liveVar.transientDeltaTime, liveVar.transientTotalTime);

	fprintf(fptr, "#KEY INITIALTEMP\n");
	fprintf(fptr, "%0.5e\n\n", liveVar.initialTemperature);


	/// Manuf constraints
	fprintf(fptr, "#KEY DRAWDIR\n");
	fprintf(fptr, "%d\n\n", liveVar.drawDirection);

	fprintf(fptr, "#KEY THROUGHCUTDIR\n");
	fprintf(fptr, "%d\n\n", liveVar.throughCutDirection);

	fprintf(fptr, "#KEY CYCLICSYM\n");
	fprintf(fptr, "%d\n\n", liveVar.cyclicSymmetry);

	fprintf(fptr, "#KEY SMOOTHING\n");
	fprintf(fptr, "%d\n\n", liveVar.nSmoothingIterations);


	fprintf(fptr, "#KEY KEEPDIR\n");
	fprintf(fptr, "%d\n\n", liveVar.keepDirichlet);


	//TopOpt
	fprintf(fptr, "#KEY OBJECTIVE\n");
	fprintf(fptr, "%d\n\n", liveVar.topoptObjective);

	fprintf(fptr, "#KEY VOLFRAC\n");
	fprintf(fptr, "%.3f\n\n", liveVar.desiredVolFraction);

	fprintf(fptr, "#KEY STEPSIZE\n");
	fprintf(fptr, "%.4f\n\n", liveVar.stepSize);


	fprintf(fptr, "#KEY MAXDISPLACEMENT\n");
	fprintf(fptr, "%.2e\n\n", liveVar.maxDisplacementAllowed);

	fprintf(fptr, "#KEY STRESSSAFETYFACTOR\n");
	fprintf(fptr, "%.3f\n\n", liveVar.stressSafetyFactor);


	fclose(fptr);
	printf("---------------------------------------------\n");
}
/************************************************************/
bool meshingCallBack(double fractionCompleted)
{
	//printf("%.2f \n",fractionCompleted);
	return true;
}
/************************************************************/
void createVoxelMesh(char voxFile[])
{
	bool success;
	char str[256];

	if (vox.getMaxVoxels() < liveVar.nElemsToMesh) {
		sprintf(str, "The number of elements is limited to %d \n", vox.getMaxVoxels());
		strcat(str, "Contact support@sciartsoft.com for a full version\n");
		printf("%s ", str);
		if (liveVar.usingGUI)
			Message(NULL, str, "Pareto", MB_OK);
	}

	if (liveVar.nElemsToMesh < vox.getRecommendedNumberOfElements()) {
		sprintf(str, "The minimum number of elements recommended is %d  \n", vox.getRecommendedNumberOfElements());
		printf("%s ", str);
		if (liveVar.usingGUI)
			Message(NULL, str, "Pareto", MB_OK);
	}

	timer.start();
	printf("CoreFunctions: Meshing STL: N = %d \n", liveVar.nElemsToMesh);
	int symmetryIndex = 1 * liveVar.xSym + 2 * liveVar.ySym + 4 * liveVar.zSym;

	success = vox.voxelize(liveVar.nElemsToMesh, liveVar.maxAspectRatio, symmetryIndex, meshingCallBack);
	if (!success) {
		printf("Could not mesh\n");
		printf("---------------------------------------------\n");
		guiState = BC_SUCCESFUL;
		return;
	}
	vox.dumpVoxelDataBinary(voxFile);
	sprintf(str, "Meshing completed; time = %.3f (secs)", timer.getElapsedTimeInSec());
	printf(str);
	printf("\n---------------------------------------------\n");
	strcpy(liveVar.userMessage, str);

}
/************************************************************/
bool checkMeshingError()
{
	double STLVolume, STLArea, err, voxelVolume;
	char str[256];
	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	voxelVolume = vox.getVoxelVolume();
	err = 100 * fabs(voxelVolume - STLVolume) / STLVolume;
	printf("Mesher: STL Volume = %0.2e \n", STLVolume);
	printf("Mesher: Mesh Volume = %0.2e \n", voxelVolume);
	printf("Mesher: PercentVolErr = %0.3f \n", err);

	if (err > 50.0) {
		sprintf(str, "Error\nMesh is very poor.\nVolume percent error = %.2f. \nTry increasing mesh size.\n", err);
		printf(str);
		if (liveVar.usingGUI) Message(NULL, str, "Pareto", MB_OK);
		printf("---------------------------------------------\n");
		return false;
	}
	else if (err > 5.0) {
		sprintf(str, "Warning: Mesh is poor.\nVolume percent error = %.2f \nTry increasing mesh size.\n", err);
		printf(str);
		printf("---------------------------------------------\n");
		//if (liveVar.usingGUI) Message(NULL,str,"Pareto",MB_OK);
	}
	return true;
}

/************************************************************/
void transferBoundaryConditionsToConformingMesh()
{
	int triGroup;
	double x[3], y[3], z[3], pt1[3], pt2[3], pt3[3], meshPt[3], ptClosest[3], stu[3], dist;

	int bcSet = liveVar.currentBCSet;

	// boundary condition transfer from geometry to mesh
	for (int node = 0; node < topopt.getNumNodes(); node++) {
		topopt.getNodeXYZ(node, meshPt);
		for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
			triGroup = vox.getTriGroup(tri);
			if (triGroup == 0)// no boundary conditions
				continue;// don't check distance to these triangles
			vox.getTriPts(tri, x, y, z);//get triangles points
			//walk thru all nodes of mesh

			pt1[0] = x[0]; pt1[1] = y[0]; pt1[2] = z[0];
			pt2[0] = x[1]; pt2[1] = y[1]; pt2[2] = z[1];
			pt3[0] = x[2]; pt3[1] = y[2]; pt3[2] = z[2];
			dist = vox.distanceOfPointFromTriangle3D(meshPt, pt1, pt2, pt3, ptClosest, stu);
			if (dist < 0.0001*liveVar.modelScale) {
				topopt.assignNodeLabel(node, triGroup);
				if ((BCType[bcSet][triGroup][0] * BCType[bcSet][triGroup][1] * BCType[bcSet][triGroup][2] == 0)) // if Dirichlet, stop
					break;
			}
		}
	}
	topopt.resetBC();
	//skip the first face group since this corresponds to zero Neumann
	for (int faceGroup = 1; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);

}
///*****************************************************
void generateQuadMesh()
{
#ifdef _WIN32
	/*STARTUPINFO si = {};
	si.cb = sizeof si;
	PROCESS_INFORMATION pi = {};
	char fullPath[256];

	strcpy(fullPath,liveVar.sciartHomeDirectory);
	strcat(fullPath,"\\QuadMeshing\\generateQuadMesh.bat");
	printf("Quad mesh path %s \n",fullPath);
	const TCHAR* target = _T(fullPath);

	//const TCHAR* target = _T("C:\\Users\\suresh\\Desktop\\suresh\\Software\\Matlab\\STOLab\\mesh2d\\gen4uMeshGenerator\\generateQuadMesh.bat");
	if ( !CreateProcess(target, 0, 0, FALSE, 0, 0, 0, 0, &si, &pi) )
	printf( "CreateProcess failed \n");
	else {
	printf("Waiting on process for 5 seconds..\n");
	WaitForSingleObject(pi.hProcess, 5 * 1000);
	if ( PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0) ) // Good
	printf("Request to terminate process has been sent!\n");

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	}*/
#else
	printf("Could not execute on non-Window machine\n");
#endif
}
/************************************************************/
bool createSkeleton()
{
	JMeshContraction meshContraction;
	Point3I point3I;
	Point3D point3D;
	vector<Point3I> triangles;
	vector<Point3D> pCoord;
	int savedCounter, nodes[3], nNodes, nTriangles;
	double actualVolumeFraction, isoCutoff, timeInSecs, xyz[3];

	savedCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction, &actualVolumeFraction, &isoCutoff, &timeInSecs);
	if (savedCounter < 0) 	return false;

	nTriangles = topopt.getNumIsoTriangles(savedCounter);
	nNodes = topopt.getNumIsoNodes(savedCounter);
	pCoord.reserve(nNodes);
	triangles.reserve(nTriangles);

	triangles.clear();
	pCoord.clear();
	for (int tri = 0; tri < nTriangles; tri++) {
		topopt.getIsoTriNodes(savedCounter, tri, nodes);
		for (int i = 0; i < 3; i++)
			point3I[i] = nodes[i];

		triangles.push_back(point3I);
	}
	for (int node = 0; node < nNodes; node++) {
		topopt.getIsoNodeXYZ(savedCounter, node, xyz);
		for (int i = 0; i < 3; i++)
			point3D[i] = xyz[i];
		pCoord.push_back(point3D);
	}

	meshContraction.setMesh(triangles, pCoord);
	meshContraction.solve(2);

	for (int node = 0; node < nNodes; node++) {
		meshContraction.getNodePt(node, point3D);
		for (int i = 0; i < 3; i++)
			xyz[i] = point3D[i];

		topopt.setIsoNodeXYZ(savedCounter, node, xyz);
	}


	return true;
}
/************************************************************/
void createLatticeStructureMesh()
{
	char latticeSpecs[1024];
	double latticeUnitCellSizeInModelUnits, materialProperties[7], area, boundingBox[3][2], geometryVolume;
	bool success;
	char *voxFile = "temp.vox";
	char *offFile = "temp.off";
	char stlFile[256];
	indexed_mesh mesh;


	// apply dummy material
	materialProperties[0] = liveVar.E;
	materialProperties[1] = liveVar.nu;
	materialProperties[2] = liveVar.Y;
	materialProperties[3] = liveVar.rho;
	materialProperties[4] = liveVar.K;
	materialProperties[5] = liveVar.alpha;
	materialProperties[6] = liveVar.specificHeat;
	for (int part = 0; part < vox.getNumParts(); part++)
		topoptTemp.setIsotropicMaterialPropertiesOfPart(part, materialProperties, materialNamesStr[liveVar.material]);

	if (guiState == TOPOPT_SUCCESFUL) {// save the optimized mesh in a temporary object
		topopt.dumpVoxelFileBinary(voxFile);
		topoptTemp.readVoxelFileBinary(voxFile);// store for later
		remove(voxFile);
	}

	strcpy(stlFile, liveVar.modelName);
	strcat(stlFile, "_lattice.stl");

	vox.getBoundingBox(boundingBox);
	geometryVolume = vox.getVolumeOfSTLGeom(&area);

	switch (liveVar.paretoUnits) {
	case UNITS_METER:
		latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM / 1000.0;
		break;
	case UNITS_MM:
		latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM;
		break;
	case UNITS_INCH:
		latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM / 25.4;
		break;
	}

	// set all parameters
	topopt.setLatticeType((LATTICE_TYPE)liveVar.latticeType);
	topopt.setLatticeSurfaceRetainType((LATTICE_SURFACE_RETAIN)liveVar.latticeSurfaceRetainType);
	topopt.setLatticeResolution(liveVar.latticeResolution);
	topopt.setLatticeFillRatio(liveVar.latticeFillRatio);
	topopt.setLatticeUnitCellSizeInModelUnits(latticeUnitCellSizeInModelUnits);
	topopt.setLatticeBoundingBox(geometryVolume, boundingBox);

	topopt.getLatticeSpecsAsString(latticeSpecs);// check to see if the numbers are acceptable
#ifdef _WIN32
	int result = MessageBoxA(NULL, latticeSpecs, "Lattice Specs", MB_YESNO);
	if (result == IDNO)	return;
#endif

	printf("CoreFunction: Attempting to create a lattice structure mesh... \n");
	timer.start();

	int symmetryIndex = 1 * liveVar.xSym + 2 * liveVar.ySym + 4 * liveVar.zSym;
	int numberOfLatticeVoxElems = topopt.getLatticeNumberOfVoxelElementsNeeded();

	success = vox.voxelize(numberOfLatticeVoxElems, liveVar.maxAspectRatio, symmetryIndex, meshingCallBack);
	if (!success) {
		printf("CoreFunction: Could not mesh... \n");
		guiState = BC_SUCCESFUL;
		return;
	}
	vox.dumpVoxelDataBinary(voxFile);// this is the full voxel

	for (int part = 0; part < vox.getNumParts(); part++)
		topopt.setIsotropicMaterialPropertiesOfPart(part, materialProperties, materialNamesStr[liveVar.material]);

	if (!topopt.readVoxelFileBinary(voxFile))
		return;

	remove(voxFile);
	topopt.resetBC();//elasticity
	int bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);

	printf("CoreFunction: Attempting to create a lattice structure mesh... \n");
	// Main code
	topopt.computeLatticeStructure(false, latticeDesignCallBackFunction);

	if (guiState == TOPOPT_SUCCESFUL) {// use the the optimized mesh to kill outside elements
		double elemCenter[3];
		double *pseudoDensity = (double *)malloc(topopt.getNumElems() * sizeof(double));
		topopt.getPseudoDensity(pseudoDensity);// the full lattice
		for (int k = 0; k < topopt.getNumElems(); k++) {
			topopt.getElemCenter(k, elemCenter);
			if (topoptTemp.findElemWithCenter(elemCenter[0], elemCenter[1], elemCenter[2], 2) == -1)
				pseudoDensity[k] = 0;
		}
		topopt.setPseudoDensity(pseudoDensity);
	}

	// creating the isosurface from mesh
	topopt.dumpIsoSurfaceFromMesh(stlFile);
	mesh.load_from_binary_stereo_lithography_file(stlFile);
	remove(stlFile);
	mesh.taubin_smooth(0.6307, -0.6734, liveVar.latticeSmoothening);
	topopt.resetIsoCounter();
	mesh.save_to_extended_off_file(offFile);
	topopt.readExtendedOffFile(offFile);
	remove(offFile);

	if (liveVar.latticeSaveSTL) // save smoothed stl file
		mesh.save_to_binary_stereo_lithography_file(stlFile);

	liveVar.modelScale = topopt.getModelScale();
	topopt.createElementMatricesTemplates();// we will need this FEA

	guiState = TOPOPT_SUCCESFUL;
	printf("nDOF = %d; time taken = %0.3f (s)\n", 3 * topopt.getNumNodes(), timer.getElapsedTimeInSec());

	printf("Volume fraction %.2f \n", topopt.getCurrentVolumeFractionOfOptimizedParts());
	printf("---------------------------------------------\n");
	liveVar.isTopOptComplete = true;// in order to plot isosurface

	liveVar.forceRemesh = 0;
	liveVar.nWorkSpaceGroups = MIN(MAX_WORKSPACE_GROUPS, MAX(64, NEARESTINT(topopt.getNumElems() / 750.0)));

}
/************************************************************/
void createTopOptMesh()
{
	char voxFile[256];
	int bcSet;

	Timer localTimer;

	printf("CoreFunction: Attempting to create a voxel mesh... \n");
	strcpy(voxFile, "temp.vxb");
	createVoxelMesh(voxFile);
	if (!checkMeshingError()) {
		remove(voxFile); //delete file
		return;
	}
	localTimer.start();
	if (!topopt.readVoxelFileBinary(voxFile)) {
		return;
	}
	remove(voxFile);
	printf("CoreFunction: Mesh read: %.3f (secs) \n", localTimer.getElapsedTimeInSec());
	double meshVolume = topopt.getMeshVolume();
	topopt.resetBC();//elasticity
	topopt.allocateMemory_thermal();

	topopt.resetBC_thermal(); //thermal BC

	bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);

	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodes_thermal(faceGroup, ThermalBCType[bcSet][faceGroup], ThermalBCArea[bcSet][faceGroup], ThermalBCValue[bcSet][faceGroup], ThermalBCRef[bcSet][faceGroup]);

	topopt.setElasticityAcceleration(ElasticityAcceleration[bcSet]);

	liveVar.plotOverGeom = 1;
	if (!topopt.isMeshConnected()) {
		printf("Error: Mesh is disconnected.\nTry increasing mesh size. \n");
		printf("---------------------------------------------\n");
		if (liveVar.usingGUI)
			Message(NULL, "Error: Mesh is disconnected.\nTry increasing mesh size. \n", "Pareto", MB_OK);
		return;
	}

	guiState = HEX_SUCCESFUL;
	topopt.resetIsoCounter();

	liveVar.modelScale = topopt.getModelScale();

	topopt.createElementMatricesTemplates();
	guiState = HEX_SUCCESFUL;
	printf("nDOF = %d; time taken = %0.3f (s)\n", 3 * topopt.getNumNodes(), timer.getElapsedTimeInSec());
	printf("---------------------------------------------\n");

	topopt.releaseEigen();
	//topopt.initializeContours();//get rid of old contours
	topopt.InitializeTopOpt(1);// get rid of isosurface triangles


	// create tessellation for display
	printf("CoreFunctions: Creating tessellation model  ... \n");
	char *tessellationFile = "tessellation.stl";
	vox.dumpSTLTesselation(tessellationFile, 20, 0.010*liveVar.modelScale);
	topopt.readTessellationFile(tessellationFile);//read into AFElasticity; will be used later in SolveElasticity
	remove(tessellationFile); //delete file
}
/************************************************************/
int readVoxel(char *voxelFileName)
{
	int bcSet;
	timer.start();
	printf("---------------------------------------------\n");
	topopt.readVoxelFile(voxelFileName);
	liveVar.modelScale = vox.getModelScale();


	printf("E = %e, nu = %0.3f, rho = %e \n", liveVar.E, liveVar.nu, liveVar.rho);
	topopt.createElementMatricesTemplates();

	guiState = HEX_SUCCESFUL;
	setPlotRoutine(1);
	printf("nDOF = %d; time taken = %0.3f (s)\n", 3 * topopt.getNumNodes(), timer.getElapsedTimeInSec());

	topopt.resetBC();
	bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup <= topopt.getMaxNodeLabel(); faceGroup++) {
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);
	}

	printf("---------------------------------------------\n");
	return 1;
}
/************************************************************/
void initializeTopOptVariables()
{
	int nNodes, useGPU, mode;

	printf("---------------------------------------------\n");
	nNodes = topopt.getNumNodes();
	if (guiState < HEX_SUCCESFUL) {
		printf(" First create hex \n");
		printf("---------------------------------------------\n");
		return;
	}

	if (liveVar.useGPU) {
#ifdef COMPILE_GPUCODE
		if (isThereValidCudaDevice() == 1) {
			printf("Ready to optimize topology on GPU ... \n");
			useGPU = 1;
			topopt.enableGPU();
		}
		else {
			printf("There is no valid CUDA device supporting double precision.\n");
			printf("---------------------------------------------\n");
			return;
		}
#endif
	}
	else {
		printf("Ready to optimize topology on CPU ... \n");
		topopt.disableGPU();
		useGPU = 0;
	}

	if (liveVar.keepDirichlet) mode = 3;
	else mode = 1;

	if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
	else topopt.disableNodalSpMV();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();


	topopt.setNumSmoothingOperations(liveVar.nSmoothingIterations);
	topopt.InitializeTopOpt(mode);
	topopt.setTopoptMode(TOPOPT_MODE_ELASTIC);

	// For batch mode, remove all files the report progress and stop Pareto (from previous runs)

#if (COMPILE_MODE == BATCH_MODE)
	char tempFile[256];

	sprintf(tempFile, CLIENTFILES, liveVar.userName);
	strcat(tempFile, "_currentVolFraction.txt");
	remove(tempFile);
	// check for stop by user file
	sprintf(tempFile, CLIENTFILES, liveVar.userName);
	strcat(tempFile, "_stop.txt");
	remove(tempFile);
#endif
}
/************************************************************/
bool verifySymmetryConditions()
{
	bool symmetry[3];

	vox.getSTLSymmetry(symmetry);

	if ((liveVar.xSym == 1) && (!symmetry[0])) {
		printf("Geometry is not symmetric in X, but X symmetry is imposed. Please verify. \n");
		return false;
	}
	if ((liveVar.ySym == 1) && (!symmetry[1])) {
		printf("Geometry is not symmetric in Y, but Ysymmetry is imposed. Please verify. \n");
		return false;
	}
	if ((liveVar.zSym == 1) && (!symmetry[2])) {
		printf("Geometry is not symmetric in Z, but Z symmetry is imposed. Please verify. \n");
		return false;
	}

	return true;
}
/************************************************************/
bool verifyDirichletConditions()
{
	int type[] = { 1,1,1 };
	int bcSet;

	bool groupUsed[STL_MAX_GROUPS];
	for (int g = 0; g < STL_MAX_GROUPS; g++)
		groupUsed[g] = false;

	groupUsed[0] = true;// always needed
	for (int tri = 0; tri < vox.getNumTriangles(); tri++)
		groupUsed[vox.getTriGroup(tri)] = true;


	//Make sure all three directions are fixed over the geometry
	bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
		if (!groupUsed[faceGroup]) continue;
		type[0] *= BCType[bcSet][faceGroup][0];
		type[1] *= BCType[bcSet][faceGroup][1];
		type[2] *= BCType[bcSet][faceGroup][2];
	}
	if (type[0] != 0)
		Message(NULL, "Warning: Geometry is not fixed along x direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n", "Pareto", MB_OK);

	if (type[1] != 0)
		Message(NULL, "Warning: Geometry is not fixed along y direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n", "Pareto", MB_OK);

	if (type[2] != 0)
		Message(NULL, "Warning: Geometry is not fixed along z direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n", "Pareto", MB_OK);


	if ((type[0] != 0) || (type[1] != 0) || (type[2] != 0))
		return false;
	else
		return true;
}
/************************************************************/
bool verifyNeumannConditions()
{
	int type[] = { 1,1,1 };
	int bcSet;

	//Make sure all three directions are fixed over the geometry
	bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
		for (int j = 0; j < 3; j++)
			if (fabs(BCValue[bcSet][faceGroup][j]) > 0)
				return true;
	}
	return false;
}
/************************************************************/
void printReactionForces()
{
	double *reactionForces = (double *)malloc(3 * topopt.getNumNodes() * sizeof(double));
	topopt.getReactionForces(reactionForces);
	double fx, fy, fz;
	fx = 0; fy = 0; fz = 0;
	for (int node = 0; node < topopt.getNumNodes(); node++) {
		if (topopt.isNodeFree(node)) {
			fx += reactionForces[3 * node];
			fy += reactionForces[3 * node + 1];
			fz += reactionForces[3 * node + 2];

		}
	}
	printf("Reaction Forces: (%f %f %f) \n", fx, fy, fz);
	free(reactionForces);
	printf("---------------------------------------------\n");
}

/************************************************************/
void recomputeBoundaryConditions()
{
	// Use the gradient information to recompute boundary conditions
	double uGrad[3], vGrad[3], wGrad[3], x, y, z, pt[3], ptOnBoundary[3], dVec[3], nodeBC[3];
	int i, node, BCType[3], triGroup;


	printf("---------------------------------------------\n");
	printf("Recomputing boundary conditions from gradient \n");
	for (i = 0; i < 3; i++) {
		uGrad[i] = 0;
		vGrad[i] = 0;
		wGrad[i] = 0;
	}
	for (i = 0; i < 3; i++)
		BCType[i] = ELASTICITY_DIRICHLET;

	triGroup = 1; // assume fixed on this triangle group
	for (node = 0; node < topopt.getNumNodes(); node++) {
		if (!topopt.isNodeOnBoundary(node)) continue;
		if (topopt.isNodeFree(node)) continue;

		topopt.getNodeXYZ(node, pt);
		vox.distanceOfPointToSTLTriGroup(pt, triGroup, ptOnBoundary);
		x = ptOnBoundary[0]; y = ptOnBoundary[1]; z = ptOnBoundary[2];

		topopt.computeFieldGradientsAtNode(node, uGrad, vGrad, wGrad);

		for (i = 0; i < 3; i++)
			dVec[i] = pt[i] - ptOnBoundary[i];

		nodeBC[0] = (uGrad[0] * dVec[0] + uGrad[1] * dVec[1] + uGrad[2] * dVec[2]);
		nodeBC[1] = (vGrad[0] * dVec[0] + vGrad[1] * dVec[1] + vGrad[2] * dVec[2]);
		nodeBC[2] = (wGrad[0] * dVec[0] + wGrad[1] * dVec[1] + wGrad[2] * dVec[2]);

		topopt.setBCOnSingleNode(node, BCType, nodeBC);
	}
}

/************************************************************/
void trimHexMeshAgainstBoundingBox(double xyzMin[3], double xyzMax[3])
{
	// Assume a Hexmesh (voxel) mesh of an allowable space has been construced
	// Now, given an initial design (that is a subset of the design space)
	// set all elements outside the initial design to void
	double center[3];

	if (guiState < HEX_SUCCESFUL) {
		printf("Please mesh first \n");
		printf("---------------------------------------------\n");
		return;
	}
	//  remove all elements from allowable space that are outside bounding box 
	double *pseudoDensity = (double *)malloc(topopt.getNumElems() * sizeof(double));

	for (int elem = 0; elem < topopt.getNumElems(); elem++) {
		pseudoDensity[elem] = 1.0;
		topopt.getElemCenter(elem, center);
		for (int i = 0; i < 3; i++)
			if ((center[i] < xyzMin[i]) || (center[i] > xyzMax[i]))
				pseudoDensity[elem] = 0.0;
	}
	topopt.setPseudoDensity(pseudoDensity);
	free(pseudoDensity);
}
/************************************************************/
void trimHexMeshAgainstInitialDesign(char initialDesign[])
{
	// Assume a Hexmesh (voxel) mesh of an allowable space has been construced
	// Now, given an initial design (that is a subset of the design space)
	// set all elements outside the initial design to void
	int success, voxelElem;
	double center[3], offset[3], offset0[3];

	if (guiState < HEX_SUCCESFUL) {
		printf("Please mesh first \n");
		printf("---------------------------------------------\n");
		return;
	}
	printf("Attempting to open %s \n", initialDesign);
	voxInitialDesign.deleteCurrentModel();
	success = voxInitialDesign.readSTLGeom(initialDesign, 0, false, callBackFunctionForSTLGeomRead);

	if (success != 1) {
		printf("STL loading unsuccesful\n");
		printf("---------------------------------------------\n");
		return;
	}

	int symmetryIndex = 1 * liveVar.xSym + 2 * liveVar.ySym + 4 * liveVar.zSym;
	success = voxInitialDesign.voxelize(liveVar.nElemsToMesh, liveVar.maxAspectRatio, 0, meshingCallBack);

	if (!success) {
		printf("Could not voxelize\n");
		printf("---------------------------------------------\n");
		guiState = BC_SUCCESFUL;
		return;
	}
	// Now remove all elements from allowable space that are outside initiail design
	double distance, voxSize[3], xietamu[3];
	double *pseudoDensity = (double *)malloc(topopt.getNumElems() * sizeof(double));

	voxInitialDesign.getVoxelSize(voxSize);
	voxInitialDesign.getSTLGeomCenter(offset);
	vox.getSTLGeomCenter(offset0);

	for (int elem = 0; elem < topopt.getNumElems(); elem++) {
		pseudoDensity[elem] = 1.0;
		topopt.getElemCenter(elem, center);
		for (int i = 0; i < 3; i++) center[i] += (offset0[i] - offset[i]);
		distance = voxInitialDesign.closestVoxelElementToPoint(center[0], center[1], center[2], &voxelElem, xietamu);
		if (distance > 0.7*voxSize[0]) {
			//printf("%d: %e %e %e %e\n",elem,center[0],center[1],center[2],distance/voxSize[0]);
			pseudoDensity[elem] = 0.0;
		}
	}
	topopt.setPseudoDensity(pseudoDensity);
	free(pseudoDensity);
}

/************************************************************/
void deformMeshToBoundary()
{
	int node, nNodes, triClosest;
	double pt[3], ptSave[3], dist, eta;


	printf("---------------------------------------------\n");
	if (guiState < HEX_SUCCESFUL)
		createTopOptMesh();

	eta = 0;
	topopt.laplaceSmoothVoxelMesh(eta);

	nNodes = topopt.getNumNodes();
	for (node = 0; node < nNodes; node++) {
		if (!topopt.isNodeOnBoundary(node)) continue;
		topopt.getNodeXYZ(node, pt);
		for (int i = 0; i < 3; i++)
			ptSave[i] = pt[i];

		dist = vox.distanceOfPointToSTLGeom(ptSave, pt, &triClosest);
		for (int i = 0; i < 3; i++)
			ptSave[i] += eta*(pt[i] - ptSave[i]);
		topopt.setMeshNodeLocation(node, ptSave);
	}
	topopt.discardTemplates();
	topopt.findTemplates();
	topopt.computeKeOfTemplates();
	topopt.disableDeflation();

	//topopt.enableDeflation();
	printf("---------------------------------------------\n");
	liveVar.nodeBasedKU = 0;
	liveVar.workSpaceMethod = 0;
	guiState = HEX_SUCCESFUL;
	solveStatic();

}

/************************************************************/
bool thermalFEACallBackFunction(double fractionCompleted)
{

#if (COMPILE_MODE != BATCH_MODE)
	if (GetAsyncKeyState(VK_F8)) {
		guiState = STATICFEA_SUCCESFUL;
		strcpy(liveVar.userMessage, "FEA stopped by user\n");
		return false;
	}

	topopt.sampleParetoSensorNumber(0);
	guiState = THERMALFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_TEMPERATURE;
	liveVar.plotAxis = 0;
	//topopt.pushThermalSolutionToTessellation();
	//setPlotRoutine(0);
#endif

	return true;
}
/************************************************************/
bool staticFEACallBackFunction(double fractionCompleted)
{
#if (COMPILE_MODE != BATCH_MODE)
	if (GetAsyncKeyState(VK_F8)) {
		guiState = STATICFEA_SUCCESFUL;
		strcpy(liveVar.userMessage, "FEA stopped by user\n");
		return false;
	}

	sprintf(liveVar.userMessage, "Solving FEA: %.0f%%; keep F8 pressed to quit!", 100 * fractionCompleted);

	//setPlotRoutine(0);
#endif

	return true;
}
/************************************************************/
void solveToolPathThermoStatic()
{
	int nNodes;

	printf("---------------------------------------------\n");
	nNodes = topopt.getNumNodes();
	if (guiState < HEX_SUCCESFUL) return;
	printf("Starting ThermoStatic FEA\n");
	timer.start();
	topopt.setCGMaxIter(20000);
	topopt.setCGRelativeResError(pow(10.0, -1.0*liveVar.cgResidualExponent));

	topopt.disableGPU();
	if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
	else topopt.disableNodalSpMV();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();



	topopt.solveThermalProblem();
	double TimeStepSize, PowerInput, radius;
	float TotalTimeSteps;

	TimeStepSize = 0.3;// time step
	TotalTimeSteps = 300;
	radius = 0.004;

	if (liveVar.useNonLinearThermal)
		PowerInput = 450; //watts
	else
		PowerInput = 450; //watts

	topopt.initializeParetoSensor(0, TEMPERATURE, 23, "TimeStep", "Temp (C)", "Sensor");

	double sensorLocation[3];
	sensorLocation[0] = 0.14;
	sensorLocation[1] = 0.05;
	sensorLocation[2] = 0.01;
	topopt.setParetoSensorLocation(0, sensorLocation);

	//topopt.simulateToolPath(TimeStepSize, PowerInput,radius, TotalTimeSteps,(liveVar.useNonLinearThermal ==1),0,thermalFEACallBackFunction);


	printf("Time taken = %0.3f (s)\n", timer.getElapsedTimeInSec());
	guiState = THERMALFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_TEMPERATURE;
	liveVar.plotAxis = 0;
	printf("Pushing thermal solution to tessellation ... \n");
	topopt.pushThermalSolutionToTessellation();
	printf("---------------------------------------------\n");

	setPlotRoutine(1);
	printf("---------------------------------------------\n");
}
/************************************************************/
void solveThermoStatic()
{
	int nNodes;
	double initialTemperature = 23;//C for elastic calculations. 
	bool solveElastic = false;
	printf("---------------------------------------------\n");
	nNodes = topopt.getNumNodes();
	if (guiState < HEX_SUCCESFUL) return;
	printf("Starting ThermoStatic FEA\n");
	timer.start();
	topopt.setCGMaxIter(20000);
	topopt.setCGRelativeResError(pow(10.0, -1.0*liveVar.cgResidualExponent));

	topopt.disableGPU();
	if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
	else topopt.disableNodalSpMV();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();
	topopt.setThermoElasticReferenceTemperature(initialTemperature);//
	if (liveVar.useNonLinearThermal) {
		float k[3], c[3], TLower, TUpper, alpha[3];
		k[0] = 16.0f;
		k[1] = 0.012f;
		k[2] = 0.0f;
		c[0] = c[1] = c[2] = 0.0f;
		alpha[0] = 6e-4;
		alpha[1] = alpha[2] = 0;
		TLower = 23.0f; // C
		TUpper = 1200.0f; // C
		topopt.setNonLinearThermalExpansionCoeffs(alpha);
		topopt.setNonLinearThermalCoefficients(k, c, TLower, TUpper);
		topopt.solveNLStaticThermalProblem(solveElastic, thermalFEACallBackFunction);
	}
	else {
		topopt.solveThermalProblem();
	}

	printf("Time taken = %0.3f (s)\n", timer.getElapsedTimeInSec());

	if (solveElastic) {
		guiState = STATICFEA_SUCCESFUL;
		liveVar.displayMode = DISPLAY_INITIAL_STRESS;
	}
	else
	{
		guiState = THERMALFEA_SUCCESFUL;
		liveVar.displayMode = DISPLAY_TEMPERATURE;
	}
	liveVar.plotAxis = 0;
	printf("Pushing thermal solution to tessellation ... \n");
	topopt.pushThermalSolutionToTessellation();
	printf("---------------------------------------------\n");

	setPlotRoutine(1);
	printf("---------------------------------------------\n");
}
/************************************************************/
void solveThermoTransient()
{
	int nNodes;
	double initialTemperature, simulationTime, timeStepSize;
	bool solveElastic = true;
	int isPhaseChange = 0;
	printf("---------------------------------------------\n");
	nNodes = topopt.getNumNodes();
	if (guiState < HEX_SUCCESFUL) return;
	printf("Starting Thermo Transient FEA\n");
	timer.start();
	topopt.setCGMaxIter(20000);
	topopt.setCGRelativeResError(pow(10.0, -1.0*liveVar.cgResidualExponent));

	topopt.disableGPU();
	if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
	else topopt.disableNodalSpMV();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();

	initialTemperature = 23;
	simulationTime = 1000.0;
	timeStepSize = 5.0;
	topopt.setThermoElasticReferenceTemperature(initialTemperature);//
	if (liveVar.useNonLinearThermal) {
		float k[3], c[3], alpha[3], TLower, TUpper;
		k[0] = 16.0f;
		k[1] = 0.012f;
		k[2] = 0.0f;
		c[0] = 1000;
		c[1] = c[2] = 0.0f;
		alpha[0] = 6e-4;
		alpha[1] = alpha[2] = 0;
		TLower = 23.0f; // C
		TUpper = 1200.0f; // C
		topopt.setNonLinearThermalExpansionCoeffs(alpha);
		topopt.setNonLinearThermalCoefficients(k, c, TLower, TUpper);
		topopt.solveNLStaticThermalProblem(solveElastic, thermalFEACallBackFunction);

		double L = 1e5;
		double Tms = 280;
		double Tml = 320;
		topopt.InitializeNonLinearPhaseChangeData(isPhaseChange, L, Tms, Tml);
		topopt.simulateNonLinearTransient(initialTemperature, simulationTime, timeStepSize, solveElastic, thermalFEACallBackFunction);
	}
	else {
		topopt.solveTransientThermoElasticProblem(initialTemperature, timeStepSize, simulationTime, solveElastic, thermalFEACallBackFunction);
	}

	printf("Time taken = %0.3f (s)\n", timer.getElapsedTimeInSec());
	guiState = THERMALFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_TEMPERATURE;
	liveVar.plotAxis = 0;
	printf("Pushing thermal solution to tessellation ... \n");
	topopt.pushThermalSolutionToTessellation();
	printf("---------------------------------------------\n");
	if (solveElastic) {
		guiState = STATICFEA_SUCCESFUL;
		liveVar.displayMode = DISPLAY_INITIAL_STRESS;
		topopt.pushInitialStaticSolutionToTessellation();
	}
	setPlotRoutine(1);
	printf("---------------------------------------------\n");
}
/************************************************************/
void solveThermoElastic()
{
	//compute the thermal forces and then solve elastic
	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();
	int nNodes = topopt.getNumNodes();
	double *h_fSave = (double *)malloc(3 * nNodes * sizeof(double));
	topopt.setThermoElasticReferenceTemperature(liveVar.initialTemperature);// where is there no stress or strain
	topopt.getForceVector(h_fSave);
	topopt.addThermalForce();
	solveStatic();
	topopt.setForceVector(h_fSave);
	free(h_fSave);
}
/************************************************************/
void solveStatic()
{
	int nNodes;

	if (!verifyDirichletConditions()) {
		if (liveVar.usingGUI)
			Message(NULL, "Warning: Geometry is not fixed in all three directions.\nThe displacements may be incorrect, and FEA will take longer.\n ", "Pareto", MB_OK);
	}

	if (topopt.getForceNorm() == 0) {
		printf("Error: Zero force detected\n");
		printf("---------------------------------------------\n");

		liveVar.displayMode = DISPLAY_GEOM;
		if (liveVar.usingGUI) {
			Message(NULL, "Error: Zero force detected\n", "Pareto", MB_OK);
			setPlotRoutine(1);
			pareto_myGlutDisplay();
		}
		return;
	}
	if (!verifySymmetryConditions()) {
		printf("Error: Incorrect symmetry being imposed. Please verify. \n");
		if (liveVar.usingGUI)
			Message(NULL, "Error: Incorrect symmetry being imposed. Please verify. \n", "Pareto", MB_OK);
		return;
	}

	printf("---------------------------------------------\n");

	nNodes = topopt.getNumNodes();
	if (guiState < HEX_SUCCESFUL)
		return;


	if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
	else topopt.disableNodalSpMV();

	topopt.setNodeAssignmentMethod(0);// grid method

	liveVar.displayMode = DISPLAY_GEOM;
	setPlotRoutine(0);
	printf("Starting Static FEA\n");
	strcpy(liveVar.userMessage, "Starting Static FEA");
	topopt.setCGMaxIter(20000);
	topopt.setCGRelativeResError(pow(10.0, -1.0*liveVar.cgResidualExponent));


#ifdef COMPILE_GPUCODE
	if (liveVar.useGPU) {
		topopt.enableGPU();
	}
	else
#endif
		topopt.disableGPU();

	if (liveVar.usePreconditioner)
		topopt.enablePreconditioner();
	else
		topopt.disablePreconditioner();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();


	double timeForOneFEA;

	timer.start();

	topopt.solveElasticityStatic();
	topopt.postprocess();
	timeForOneFEA = timer.getElapsedTimeInSec();
	printf("#CG Iterations = %d\n", topopt.getLastCGIteration());
	printf("Time taken = %.3f (sec) \n", timer.getElapsedTimeInSec());


	printf("Max Displacement: %0.2e \n", topopt.getDeformationMax());
	printf("Max Von Mises: %0.2e \n", topopt.getMaxVonMisesStress());
	printf("Time taken = %.3f (sec) \n", timer.getElapsedTimeInSec());

	if ((guiState >= TOPOPT_SUCCESFUL) && (!liveVar.forceRemesh)) {
		printf("Pushing stress results to iso\n");
		topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface
		guiState = TOPOPT_SUCCESFUL;
		liveVar.displayMode = DISPLAY_FINAL_STRESS;
	}
	else {
		printf("Pushing elasticity solution to tessellation ... \n");
		topopt.pushInitialStaticSolutionToTessellation();
		guiState = STATICFEA_SUCCESFUL;
		liveVar.displayMode = DISPLAY_INITIAL_STRESS;
	}

	printf("---------------------------------------------\n");

	liveVar.plotAxis = 1;
	//Set plotting routine mesh

	setPlotRoutine(0);
	strcpy(liveVar.userMessage, "Static FEA solved");
}
/************************************************************/
void solveModal()
{
	int nNodes;
	double eigRelativeAccuracy;
	nNodes = topopt.getNumNodes();

	if (!verifyDirichletConditions()) {
		printf("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n");
		if (liveVar.usingGUI)
			Message(NULL, "Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n", "Pareto", MB_OK);
	}

	if (guiState < HEX_SUCCESFUL)
		return;

#ifdef COMPILE_GPUCODE
	if (liveVar.useGPU) {
		topopt.enableGPU();
	}
	else
#endif
		topopt.disableGPU();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();

	timer.start();
	topopt.initializeSolution();
	allocateMemoryForEigenModes();
	printf("---------------------------------------------\n");
	printf("Computing Eigenmodes \n");
	eigRelativeAccuracy = pow(10.0, -liveVar.eigRelAccuracyExponent);

	bool success = topopt.solveElasticityModal(liveVar.nEigenModes, liveVar.eigenValues, liveVar.eigenModes, eigRelativeAccuracy, liveVar.maxTimeAllowed);

	if (!success)
		printf("Error \n");
	printf("---------------------------------------------\n");
	for (int i = 0; i < liveVar.nEigenModes; i++)
		printf("Eigenmode %d: %.4f (Hz) \n", i + 1, liveVar.eigenValues[i]);
	printf("---------------------------------------------\n");
	printf("Time taken = %0.3f (s)\n", timer.getElapsedTimeInSec());
	printf("---------------------------------------------\n");

	topopt.populateEigen(liveVar.nEigenModes, liveVar.eigenValues, liveVar.eigenModes);
	topopt.pushInitialStaticSolutionToTessellation();
	printf("---------------------------------------------\n");

	guiState = MODALFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_MODAL;
	setPlotRoutine(1);

}
/************************************************************/
void solveBuckling()
{
	int nNodes;
	nNodes = topopt.getNumNodes();

	if (!verifyDirichletConditions()) {
		printf("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n");
		if (liveVar.usingGUI)
			Message(NULL, "Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n", "Pareto", MB_OK);
	}

	if (guiState < HEX_SUCCESFUL)
		return;

#ifdef COMPILE_GPUCODE
	if (liveVar.useGPU) {
		topopt.enableGPU();
	}
	else
#endif
		topopt.disableGPU();

	liveVar.nEigenModes = 1;
	//glui->sync_live();

	if (liveVar.workSpaceMethod != DEFLATION_NONE)
		topopt.enableDeflation(liveVar.workSpaceMethod, liveVar.nWorkSpaceGroups);
	else
		topopt.disableDeflation();

	timer.start();
	allocateMemoryForEigenModes();
	printf("---------------------------------------------\n");
	printf("Computing the first buckling modes \n");
	topopt.solveBucklingModal(liveVar.nEigenModes, liveVar.eigenValues, liveVar.eigenModes);

	printf("---------------------------------------------\n");
	for (int i = 0; i < liveVar.nEigenModes; i++)
		printf("Buckling constant %d: %.4f  \n", i + 1, liveVar.eigenValues[i]);
	printf("---------------------------------------------\n");
	printf("Time taken = %0.3f (s)\n", timer.getElapsedTimeInSec());
	printf("---------------------------------------------\n");

	guiState = MODALFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_MODAL;
	setPlotRoutine(1);

	topopt.populateEigen(liveVar.nEigenModes, liveVar.eigenValues, liveVar.eigenModes);
}
/**********************************************************************/
void getMaxDeformationAndStressOverMesh(double *maxDeformation, double *maxStress)
{
	int whichNode;
	*maxDeformation = topopt.getDeformationMax();
	*maxStress = topopt.getMaxVonMisesStress(&whichNode);
}
/************************************************************/
void setTopOptConstraints(void)
{
	int symmetryIndex = 1 * liveVar.xSym + 2 * liveVar.ySym + 4 * liveVar.zSym;
	topopt.setAxiSymmetry(liveVar.cyclicSymmetry);
	topopt.setNumSmoothingOperations(liveVar.nSmoothingIterations);
	topopt.setStressSafetyFactor(liveVar.stressSafetyFactor);
	topopt.setSymmetryIndex(symmetryIndex);
	topopt.setDrawDirection(liveVar.drawDirection);
	topopt.setThroughCutDirection(liveVar.throughCutDirection);
}
/************************************************************/
void SIMPOptimize()
{
	double offset[] = { 0,0,0 };
	topopt.setSIMPTargetVolumeFraction(0.5);
	topopt.setSIMPMinDensity(0.05);// lowering affects CG Convergence and increases affects accuracy
	guiState = STATICFEA_SUCCESFUL;
	liveVar.displayMode = DISPLAY_PSEUDODENSITY_MESH;
	topopt.solveSIMP(dummyCallBack);
	topopt.printPseudoDensityRange();

	saveIsoSurface("isosurface.stl", offset, false, 1.0, true);
	printf("Finished saving .stl \n");
	//topopt.readIsoBinary( "isosurface.stl");
	guiState = STATICFEA_SUCCESFUL;
	liveVar.plotAxis = 0;
	//liveVar.plotOverGeom = 1;
	//liveVar.displayMode = DISPLAY_FINAL_STL;
	liveVar.geomTransparent = 0;
	setPlotRoutine(0);
}
/************************************************************/
void singleLoadOptimize()
{
	int nFEA;
	double isoValueConverged, convergedVolumeFrac, volDecrement, D0, S0, volDecrementFinal;
	double targetVolumeFraction, nFEAEstimated, currentVolFraction, estimatedTimeForCompletion;
	double actualTime, maxDeformation, maxStress;
	char resultFile[1000], tempStr[256];


	liveVar.isTopOptComplete = false;
	initializeTopOptVariables();
	if (!verifyDirichletConditions())
		printf("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n");


	if (liveVar.timeForOneFEA <= 0) {
		timer.stop();
		timer.start();
		topopt.solveElasticityStatic();
		topopt.postprocess();
		liveVar.timeForOneFEA = timer.getElapsedTimeInSec();
	}

	D0 = topopt.getDeformationMax();
	S0 = topopt.getMaxVonMisesStress();

	nFEA = 1;
	replaceTitle = 0; // don't replace title;
	sprintf(userDefinedTitle, "Vol. = 1; D/D0 = 1; S/S0 = 1, nFEA = %d; T = 0(s)", nFEA);

	topopt.setMaxDisplacement(liveVar.maxDisplacementAllowed);

	printf("Initial Max Displacement: %e\n", D0);
	printf("  Allowed Displacement: %e\n", liveVar.maxDisplacementAllowed);
	printf("Initial Max Stress: %e\n", S0);
	printf("  Yield Stress: %e\n", liveVar.Y);


	targetVolumeFraction = liveVar.desiredVolFraction;
	currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	volDecrement = liveVar.stepSize;

	printf("-----------------------------------------------------------\n");
	if (liveVar.topoptObjective == (TOPOPT_QUANTITY)VOLUME) {
		liveVar.topoptObjective = (TOPOPT_QUANTITY)COMPLIANCE;// default
		nFEAEstimated = 3 * (currentVolFraction - targetVolumeFraction) / volDecrement;
	}
	else if (liveVar.topoptObjective == (TOPOPT_QUANTITY)COMPLIANCE) {
		printf("Objective: Maximize Stiffness\n");
		nFEAEstimated = 3 * (currentVolFraction - targetVolumeFraction) / volDecrement;
	}
	else if (liveVar.topoptObjective == (TOPOPT_QUANTITY)VONMISES) {
		printf("Objective: Maximize Strength\n");
		nFEAEstimated = 6 * (currentVolFraction - targetVolumeFraction) / volDecrement;
	}
	else if (liveVar.topoptObjective == (TOPOPT_QUANTITY)EIGENVALUE) {
		printf("Objective: Maximize EigenValue\n");
		nFEAEstimated = 6 * (currentVolFraction - targetVolumeFraction) / volDecrement;
	}
	else {
		printf("CommonGUI: Objective not known\n");
		return;
	}
	nFEAEstimated = MAX(1, nFEAEstimated);
	topopt.setNumSmoothingOperations(liveVar.nSmoothingIterations);
	topopt.setQuantityOfInterest((TOPOPT_QUANTITY)liveVar.topoptObjective);
	estimatedTimeForCompletion = 1.5*nFEAEstimated*liveVar.timeForOneFEA;
	liveVar.estimatedTimeForTopOpt = estimatedTimeForCompletion;
	printf("-----------------------------------------------------------\n");
	printf("Time for 1 FEA: %.3f (secs)\n", liveVar.timeForOneFEA);
	printf("Estimated # of FEAs: %.0f \n", nFEAEstimated);
	printEstimatedTime(estimatedTimeForCompletion, tempStr);
	printf("%s \n", tempStr);
	strcpy(liveVar.userMessage, tempStr);
	guiState = TOPOPT_SUCCESFUL;


	liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	if (liveVar.usingGUI) {
		liveVar.displayMode = DISPLAY_FINAL_STL;
		setPlotRoutine(1);
		setMainWindow();
	}
	volDecrementFinal = volDecrement;
	timer.stop();
	topopt.initializeSolution();

	globalTimer.start();
	nFEA = 0;

	setTopOptConstraints();
	convergedVolumeFrac = topopt.TraceParetoCurve(targetVolumeFraction, volDecrement, &nFEA, &isoValueConverged, topOptCallBackFunction,
		&volDecrementFinal, liveVar.maxTimeAllowed);

	actualTime = globalTimer.getElapsedTimeInSec();

	globalTimer.stop();
	maxDeformation = topopt.getDeformationMax();
	maxStress = topopt.getMaxVonMisesStress();
	printf("---------------------------------------------\n");
	printf("Final volume fraction: %.2f\n", topopt.getCurrentVolumeFractionOfOptimizedParts());
	printf("Final Max Displacement: %e\n", maxDeformation);
	printf("Final Max Stress: %e\n", maxStress);
	sprintf(userDefinedTitle, "Vol. = %0.3f; D/D0 = %.3f; S/S0 = %0.3f", convergedVolumeFrac, maxDeformation / D0, maxStress / S0);
	printf("%s \n", userDefinedTitle);

	liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	strcpy(liveVar.userMessage, "Optimization completed");


	printf("-----------------------------------------------------------\n");
	printf("Actual # of FEAs: %d \n", nFEA);

	char finalResultMessage[1000];
	sprintf(finalResultMessage, "-----------------------------------------------------------\n");
	sprintf(tempStr, "Final volume fraction = %.2f", convergedVolumeFrac);
	strcat(finalResultMessage, tempStr);
	sprintf(tempStr, "\nNumber of FEAs: %d \n", nFEA);
	strcat(finalResultMessage, tempStr);
	printEstimatedTime(actualTime, tempStr);
	printf("%s \n", tempStr);

	strcat(finalResultMessage, tempStr);
	strcat(finalResultMessage, "\n");

	topopt.getLastTopoptErrorMessage(tempStr);
	strcat(finalResultMessage, tempStr);
	strcat(finalResultMessage, "\n-----------------------------------------------------------\n");
	printf("%s \n", finalResultMessage);
	//Message(NULL,finalResultMessage,"Pareto",MB_OK | MB_APPLMODAL );


	liveVar.plotAxis = 0;
	strcpy(resultFile, liveVar.projectDirectory);
	strcat(resultFile, liveVar.modelName);
	strcat(resultFile, "_finalIsoBinary.stl");
	printf("Saving STL to %s \n", resultFile);
	topopt.dumpIsoBinary(topopt.getCurrentIsoCounter(), resultFile);
	printf("Pushing stress results to iso\n");
	topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface
	printf("-----------------------------------------------------------\n");
	liveVar.plotAxis = 0;
	liveVar.displayMode = DISPLAY_FINAL_STL;
	guiState = TOPOPT_SUCCESFUL;
	setPlotRoutine(1);
	liveVar.isTopOptComplete = true;


}
/************************************************************/
void retainSurfacesForMultiload(void)
{
	double bcValue;
	// in multi-load problems, we must also retain nodes where forces are applied on the loads, else they will be deleted
	for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++) {
		for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
			bcValue = 0;
			for (int i = 0; i < 3; i++)
				bcValue += fabs(BCValue[bcSet][faceGroup][i]);
			if ((BCType[bcSet][faceGroup][0] == ELASTICITY_NEUMANN) && (bcValue > 0)) {// this is a force face
				topopt.retainNodesWithLabel(faceGroup);
			}
		}
	}
}
/************************************************************/
void multiLoadOptimize()
{
	int bcSet, i, faceGroup, activeGroups[STL_MAX_GROUPS], nFEA;
	double D0, S0;
	bool dirichlet1, dirichlet2;
	double targetVolumeFraction, volDecrement, nFEAEstimated, currentVolFraction, estimatedTimeForCompletion;
	double hours, minutes, seconds;


	liveVar.isTopOptComplete = false;
	if (!verifyDirichletConditions())
		printf("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. \n");

	vox.getActiveGroups(activeGroups);
	// Check if all BCTypes are same
	for (bcSet = 1; bcSet < liveVar.nBCSet; bcSet++) {
		for (faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
			if (!activeGroups[faceGroup]) continue;
			for (i = 0; i < 3; i++) {
				dirichlet1 = (BCType[0][faceGroup][i] == 0);
				dirichlet2 = (BCType[bcSet][faceGroup][i] == 0);
				if ((dirichlet1 && !dirichlet2) || (!dirichlet1 && dirichlet2)) {
					printf("Error in multiload boundary conditions \n");
					printf("All bcSets must share the same dirichlet conditions\n");
					printf("---------------------------------------------\n");
					return;
				}
			}
		}
	}

	retainSurfacesForMultiload();
	initializeTopOptVariables();

	topopt.initializeSolution();

	topopt.resetIsoCounter();
	if (liveVar.timeForOneFEA <= 0) {
		timer.start();
		topopt.solveElasticityStatic();
		topopt.postprocess();
		liveVar.timeForOneFEA = timer.getElapsedTimeInSec();
	}
	getMaxDeformationAndStressOverMesh(&D0, &S0);
	replaceTitle = 0; // don't replace title;

	guiState = TOPOPT_SUCCESFUL;

	printf("Initial Max Displacement: %e\n", D0);
	printf("Allowed Displacement: %e\n\n", liveVar.maxDisplacementAllowed);
	printf("Initial Max Stress: %e\n", S0);
	printf("Allowed Stress: %e\n", liveVar.Y);


	liveVar.displayMode = DISPLAY_FINAL_STL;
	liveVar.currentBCSet = 0;
	setPlotRoutine(1);

	volDecrement = liveVar.stepSize;
	targetVolumeFraction = liveVar.desiredVolFraction;
	currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	printf("-----------------------------------------------------------\n");
	printf("Active BC Sets: ");
	for (i = 0; i < liveVar.nBCSet; i++)
		if (liveVar.activeBCSets[i])
			printf("%d ", i);
	printf("\n");


	nFEAEstimated = 6 * liveVar.nBCSet*(currentVolFraction - targetVolumeFraction) / volDecrement;
	nFEAEstimated = MAX(1, nFEAEstimated);


	estimatedTimeForCompletion = nFEAEstimated*liveVar.timeForOneFEA;
	liveVar.estimatedTimeForTopOpt = estimatedTimeForCompletion;
	hours = floor(estimatedTimeForCompletion / 3600);
	estimatedTimeForCompletion -= hours * 3600;
	minutes = floor(estimatedTimeForCompletion / 60);
	estimatedTimeForCompletion -= minutes * 60;
	seconds = (estimatedTimeForCompletion);
	printf("-----------------------------------------------------------\n");
	printf("Estimated # of FEAs: %.0f \n", nFEAEstimated);
	printf("Estimated time: %.0f hour(s), %.0f minute(s), %.0f second(s)\n", hours, minutes, seconds);
	printf("-----------------------------------------------------------\n");


	double isoValueConverged, convergedVolumeFrac, volDecrementFinal;
	printf("Multiload method: Augmented Lagrangian Method \n");


	targetVolumeFraction = liveVar.desiredVolFraction;
	currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	volDecrement = liveVar.stepSize;
	guiState = TOPOPT_SUCCESFUL;
	liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

	liveVar.displayMode = DISPLAY_FINAL_STL;
	setPlotRoutine(1);

	volDecrementFinal = volDecrement;
	topopt.initializeSolution();

	printf("maxDispAllowed: %.2e, maxStressAllowed = %.2e\n\n", liveVar.maxDisplacementAllowed, liveVar.Y);

	topopt.setMaxDisplacement(liveVar.maxDisplacementAllowed);

	strcpy(liveVar.userMessage, "Preparing for multiload optimization");
	globalTimer.stop();
	globalTimer.start();
	nFEA = 0;

	for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++) {
		if (NumberOfConstraints[bcSet] == 0) {// no explicit constraints imposed for this bcSet. Apply standard displacement and stress constraints
			ConstraintType[bcSet][0] = 1;// compliance
			ConstraintData[bcSet][0][0] = 1;// it is an upper limit
			ConstraintData[bcSet][0][1] = 1;// hard constraint
			ConstraintData[bcSet][0][2] = 1;// not used
			NumberOfConstraints[bcSet] = 1;

			/*
			ConstraintType[bcSet][1] = 2;// stress
			ConstraintData[bcSet][1][0] = 1;// it is an upper limit
			ConstraintData[bcSet][1][1] = 1;// hard constraint
			ConstraintData[bcSet][1][2] = 1;// not used
			NumberOfConstraints[bcSet] =2;
			*/

			if (NumberOfConstraints[bcSet] > MAX_CONSTRAINTS) {
				printf("Number of constraints exceeds allowable\n");
				return;
			}
		}
	}

	char tempStr[256];
	sprintf(tempStr, "Topopt; estimated time: %.0f hr(s), %.0f min(s), %.0f sec(s)", hours, minutes, seconds);
	strcpy(liveVar.userMessage, tempStr);

	liveVar.topoptObjective = (TOPOPT_QUANTITY)VOLUME;// for multi objective problems, always minimize volume

	setTopOptConstraints();

	convergedVolumeFrac = topopt.TraceParetoCurve_MLMC(targetVolumeFraction, volDecrement, &nFEA,
		&isoValueConverged, topOptCallBackFunction, &volDecrementFinal,
		ConstraintType, NumberOfConstraints, ConstraintData, liveVar.topoptObjective, liveVar.nBCSet, BCType, BCValue,
		vox.getNumTriGroups(), liveVar.activeBCSets);

	double actualTime, tempDouble;
	actualTime = globalTimer.getElapsedTimeInSec();

	tempDouble = actualTime;
	globalTimer.stop();

	hours = floor(tempDouble / 3600);
	tempDouble -= hours * 3600;
	minutes = floor(tempDouble / 60);
	tempDouble -= minutes * 60;
	seconds = (tempDouble);
	printf("-----------------------------------------------------------\n");
	printf("Actual # of FEAs: %d \n", nFEA);
	printf("Actual time: %.0f hour(s), %.0f minute(s), %.2f second(s)\n", hours, minutes, seconds);
	printf("-----------------------------------------------------------\n");

	printf("---------------------------------------------\n");
	printf("Final volume fraction: %.2f\n", topopt.getCurrentVolumeFractionOfOptimizedParts());



	liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
	if (liveVar.usingGUI) pareto_myGlutDisplay();

	char resultFile[256];
	strcpy(resultFile, liveVar.projectDirectory);
	strcat(resultFile, liveVar.modelName);
	strcat(resultFile, "_finalIsoBinary.stl");
	printf("Saving STL to %s \n", resultFile);
	topopt.dumpIsoBinary(topopt.getCurrentIsoCounter(), resultFile);
	printf("-----------------------------------------------------------\n");

	printf("Pushing stress results to iso\n");
	topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface
	printf("-----------------------------------------------------------\n");

	char finalResultMessage[1000];
	sprintf(finalResultMessage, "Optimization completed.\nFinal volume fraction = %.2f\n", convergedVolumeFrac);
	sprintf(tempStr, "Number of FEAs: %d \n", nFEA);
	strcat(finalResultMessage, tempStr);
	printEstimatedTime(actualTime, tempStr);
	strcat(finalResultMessage, tempStr);
	strcat(finalResultMessage, "\n");
	printf("%s \n", tempStr);

	topopt.getLastTopoptErrorMessage(tempStr);
	strcat(finalResultMessage, tempStr);
	strcat(finalResultMessage, "\n----------------------------------------------\n");
	printf("%s\n", finalResultMessage);
	//Message(NULL,finalResultMessage,"Pareto",MB_OK | MB_APPLMODAL );


	guiState = TOPOPT_SUCCESFUL;
	liveVar.plotAxis = 0;
	liveVar.displayMode = DISPLAY_FINAL_STL;
	setPlotRoutine(1);

	liveVar.isTopOptComplete = true;

}
/**************************************************************/
void multiLoadOptimizeThread(void* Param)
{
	multiLoadOptimize();
}
/**************************************************************/
void singleLoadOptimizeThread(void* Param)
{
	singleLoadOptimize();
}
/************************************************************/
int loadAbaqusInp(char *abaqusInpFile)
// Load a Abaqus input file
{
	int success;
	timer.start();

	//resetLiveVariables();
	printf("---------------------------------------------\n");
	printf("Loading file %s \n", abaqusInpFile);
	success = topopt.readAbaqusInpFile(abaqusInpFile);

	if (success != 1) {
		printf("Abaqus .inp loading unsuccesful\n");
		printf("---------------------------------------------\n");
		return 0;
	}

	double meshVolume = topopt.getMeshVolume();
	char str[256];
	double STLVolume, STLArea;

	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	double err = 100 * fabs(meshVolume - STLVolume) / STLVolume;
	printf("loadAbaqusInp: PercentVolErr = %0.3f \n", err);
	if (err > 10.0) {
		sprintf(str, "Warning: Volume percent error = %.2f \nMesh is not consistent with geom\n", err);
		printf(str);
		if (liveVar.usingGUI)
			Message(NULL, str, "Pareto", MB_OK);
	}
	liveVar.modelScale = topopt.getModelScale();

	int triGroup;
	double x[3], y[3], z[3], pt1[3], pt2[3], pt3[3], meshPt[3], ptClosest[3], stu[3], dist;

	int bcSet = liveVar.currentBCSet;

	// boundary condition transfer from geometry to mesh
	for (int node = 0; node < topopt.getNumNodes(); node++) {
		topopt.getNodeXYZ(node, meshPt);
		for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
			triGroup = vox.getTriGroup(tri);
			if (triGroup == 0)// no boundary conditions
				continue;// don't check distance to these triangles
			vox.getTriPts(tri, x, y, z);//get triangles points
			//walk thru all nodes of mesh

			pt1[0] = x[0]; pt1[1] = y[0]; pt1[2] = z[0];
			pt2[0] = x[1]; pt2[1] = y[1]; pt2[2] = z[1];
			pt3[0] = x[2]; pt3[1] = y[2]; pt3[2] = z[2];
			dist = vox.distanceOfPointFromTriangle3D(meshPt, pt1, pt2, pt3, ptClosest, stu);
			if (dist < 0.01*liveVar.modelScale) {
				topopt.assignNodeLabel(node, triGroup);
				if ((BCType[bcSet][triGroup][0] * BCType[bcSet][triGroup][1] * BCType[bcSet][triGroup][2] == 0)) // if Dirichlet, stop
					break;
			}
		}
	}
	topopt.resetBC();

	//skip the first face group since this corresponds to zero Neumann
	for (int faceGroup = 1; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);

	printf("---------------------------------------------\n");
	guiState = HEX_SUCCESFUL;
	liveVar.plotOverGeom = 0;// don't plot over geom, instead plot over mesh
	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	setPlotRoutine(1);
	liveVar.nodeBasedKU = 0;// should not use
	printf("nDOF = %d; time taken = %0.3f (s)\n", 3 * topopt.getNumNodes(), timer.getElapsedTimeInSec());
	printf("---------------------------------------------\n");
	return 1;
}
/************************************************************/
int loadParetoMesh(char *paretoMeshFile)
// Load a Pareto mesh input file
{
	int success;
	timer.start();

	//resetLiveVariables();
	printf("---------------------------------------------\n");
	printf("Loading file %s \n", paretoMeshFile);

	success = topopt.readParetoMeshFile(paretoMeshFile); //.msh from windows call

	if (success != 1) {
		printf("Pareto .msh loading unsuccesful\n");
		printf("---------------------------------------------\n");
		return 0;
	}

	double meshVolume = topopt.getMeshVolume();
	char str[256];
	double STLVolume, STLArea;

	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	double err = 100 * fabs(meshVolume - STLVolume) / STLVolume;
	printf(" stlVol = %.2e, meshVolume =  %.2e, percentVolErr = %0.3f \n", STLVolume, meshVolume, err);
	if (err > 10.0) {
		sprintf(str, "Warning: Volume percent error = %.2f \nMesh is not consistent with geom\n", err);
		printf(str);
		if (liveVar.usingGUI)
			Message(NULL, str, "Pareto", MB_OK);
	}
	liveVar.modelScale = topopt.getModelScale();

	int triGroup;
	double x[3], y[3], z[3], pt1[3], pt2[3], pt3[3], meshPt[3], ptClosest[3], stu[3], dist;

	int bcSet = liveVar.currentBCSet;

	// boundary condition transfer from geometry to mesh
	for (int node = 0; node < topopt.getNumNodes(); node++) {
		topopt.getNodeXYZ(node, meshPt);
		for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
			triGroup = vox.getTriGroup(tri);
			if (triGroup == 0)// no boundary conditions
				continue;// don't check distance to these triangles
			vox.getTriPts(tri, x, y, z);//get triangles points
			//walk thru all nodes of mesh

			pt1[0] = x[0]; pt1[1] = y[0]; pt1[2] = z[0];
			pt2[0] = x[1]; pt2[1] = y[1]; pt2[2] = z[1];
			pt3[0] = x[2]; pt3[1] = y[2]; pt3[2] = z[2];
			dist = vox.distanceOfPointFromTriangle3D(meshPt, pt1, pt2, pt3, ptClosest, stu);
			if (dist < 0.01*liveVar.modelScale) {
				topopt.assignNodeLabel(node, triGroup);
				if ((BCType[bcSet][triGroup][0] * BCType[bcSet][triGroup][1] * BCType[bcSet][triGroup][2] == 0)) // if Dirichlet, stop
					break;
			}
		}
	}
	topopt.resetBC();

	//skip the first face group since this corresponds to zero Neumann
	for (int faceGroup = 1; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, BCType[bcSet][faceGroup], BCValue[bcSet][faceGroup]);

	printf("---------------------------------------------\n");
	guiState = HEX_SUCCESFUL;
	liveVar.plotOverGeom = 0;// don't plot over geom, instead plot over mesh
	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	setPlotRoutine(1);

	liveVar.nodeBasedKU = 0;// should not use
	printf("nDOF = %d; time taken = %0.3f (s)\n", 3 * topopt.getNumNodes(), timer.getElapsedTimeInSec());
	printf("---------------------------------------------\n");
	return 1;
}
/************************************************************/
void cleanUp()
{

	for (int i = 0; i < MAX_PARTS; i++)
		safeFree(liveVar.geomFileName[i]);
	safeFree(liveVar.stlFileName);
	freeEigenModesMemory();
}
/************************************************************/
void loadPRJ(char *extension)
// Load a  project
{
	int  bcSet, tri, group, tempInt, assemblyMode;
	char str[256], *pch, tempStr[256];
	char fullPathName[256];
	double temp, projectVersion;
	char xyzFile[256], materialName[256];
	char outputFile[256];
	double xietamu[3], x, y, z, vonmises, delta;
	int nPoints, voxelElem, ptNumber, voxelElemPrev, currentPartId;
	double pt[3], uvw[3], stresses[6], materialProperties[7];
	char fullPathProject[512];


	strcpy(fullPathProject, ".");

#if (COMPILE_MODE != BATCH_MODE) 
	char* cwd = _getcwd(0, 0);
	printf("Current directory is %s \n", cwd);
	strcpy(fullPathProject, cwd);
#endif

	bool loadProjectVerbose = false;

	oneTimeInitialization();
	liveVar.assemblyMode = 0;
	printf("Loading project file %s \n", liveVar.prjFileName);
	strcpy(liveVar.modelName, liveVar.prjFileName);
	// also copy  to the directory string
	strcpy(liveVar.projectDirectory, liveVar.prjFileName);
#ifdef _WIN32
	while (pch = strstr(liveVar.modelName, "\\")) // strip \ (backslash)
		strcpy(liveVar.modelName, pch + 1);
#else
	while (pch = strstr(liveVar.modelName, "/")) // strip / (forwardslash)
		strcpy(liveVar.modelName, pch + 1);
#endif
	// modelName now contains the project name with extension
	// remove the project name from directory
	pch = strstr(liveVar.projectDirectory, liveVar.modelName);
	strncpy(pch, "\0", 1);// strip the name of project
	printf("Project directory: %s\n", liveVar.projectDirectory);

	// go back to modelName and remove .prj extension
	pch = strstr(liveVar.modelName, extension);
	strncpy(pch, "\0", 1);// strip .prj extension
	sprintf(userDefinedTitle, liveVar.modelName);

	strcat(fullPathProject, "\\");
	strcat(fullPathProject, liveVar.prjFileName);
	printf("---------------------------------------------\n");
	FILE *fptr = fopen(liveVar.prjFileName, "r");

	if (fptr == NULL) {
		printf("Project file %s not found\n", liveVar.prjFileName);
		printf("---------------------------------------------\n");
		return;
	}
	resetLiveVariables();// Use all default values
	bcSet = 0;
	bool version_found;
	version_found = false;
	vox.deleteCurrentModel();
	topopt.removeAllMaterials();
	topopt.InitializeTopOpt(1);// to clear up memory
	currentPartId = 0;
	strcpy(materialName, "UnknownMaterial");
	while (find_word(fptr, "#KEY")) {
		fscanf(fptr, "%s", str);
		if (COMPILE_MODE != BATCH_MODE)
			if (loadProjectVerbose) printf("Project: Processing key %s\n", str);
		if (strcmp(str, "VERSION") == 0) {
			version_found = true;
			fscanf(fptr, "%lf", &(projectVersion));
			if (loadProjectVerbose) printf("Project version is %.2f \n", projectVersion);
			if ((liveVar.usingGUI) && (projectVersion < PARETO_VERSION)) {
				Message(NULL, "Project is is an older version. \nPlease save project  \n", "Pareto", MB_OK);
			}
		}
		else if (strcmp(str, "USERNAME") == 0) {
			fscanf(fptr, "%s", &(liveVar.userName));
		}
		else if (strcmp(str, "ASSEMBLY") == 0) {
			fscanf(fptr, "%d", &(assemblyMode));
		}
		else if (strcmp(str, "HEXMESH_VERBOSE") == 0) {
			fscanf(fptr, "%d", &(tempInt));
			if (tempInt) topopt.enableHexmeshVerbose();
			else topopt.disableHexmeshVerbose();
		}
		else if (strcmp(str, "TOPOPT_VERBOSE") == 0) {
			fscanf(fptr, "%d", &(tempInt));
			if (tempInt) topopt.enableTSVerbose();
			else topopt.disableTSVerbose();
		}
		else if (strcmp(str, "DEFLATION_VERBOSE") == 0) {
			fscanf(fptr, "%d", &(tempInt));
			if (tempInt) topopt.enableDeflationVerbose();
			else topopt.disableDeflationVerbose();
		}
		else if (strcmp(str, "GEOMETRY") == 0) {
			if (!version_found) {
				printf("Project may be of an older version. \nPlease read manual for possible format changes\nIt is recommended that you recreate project file from GUI\n");
				if (liveVar.usingGUI) Message(NULL, "Project may be of an older version. \nPlease read manual for possible format changes\nIt is recommended that you recreate project file from GUI", "Pareto", MB_OK);
			}
			fscanf(fptr, "%s", tempStr);
			if (strstr(tempStr, ".STL") || strstr(tempStr, ".stl")) {
				printf("Opening file %s in directory %s part = %d \n", tempStr, liveVar.projectDirectory, liveVar.nSTLFilesLoaded);
				strcpy(liveVar.geomFileName[liveVar.nSTLFilesLoaded], liveVar.projectDirectory);
				strcat(liveVar.geomFileName[liveVar.nSTLFilesLoaded], tempStr);
				if (loadGeom(liveVar.assemblyMode == 1) == 0) {
					printf("Load Project: Could not load stl file %s \n", liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
					break;
				}
				liveVar.assemblyMode = assemblyMode;//only update after the first geometry is read.
			}
			else {
				printf(" Geometry file %s does not seem to have .stl extension \n", tempStr);
				printf(" Make sure the file name does not have a space in it \n");
				break;
			}
		}
		else if (strcmp(str, "UPDATE_GEOM") == 0) {// update the geometry and transform the boundary conditions
			fscanf(fptr, "%s", tempStr);
			if (strstr(tempStr, ".STL") || strstr(tempStr, ".stl")) {
				if (loadProjectVerbose) printf("Opening file %s in directory %s part = %d \n", tempStr, liveVar.projectDirectory, liveVar.nSTLFilesLoaded);
				strcpy(liveVar.geomFileName[liveVar.nSTLFilesLoaded], liveVar.projectDirectory);
				strcat(liveVar.geomFileName[liveVar.nSTLFilesLoaded], tempStr);
				if (!updateGeom(false)) {
					printf("Load Project: Could not update stl file %s \n", liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
					break;
				}
			}
			else {
				printf(" Geometry file %s does not seem to have .stl extension \n", tempStr);
				printf(" Make sure the file name does not have a space in it \n");
				break;
			}
		}
		else if (strcmp(str, "PART") == 0) {
			fscanf(fptr, "%d ", &(currentPartId));// used below in Material
		}
		else if (strcmp(str, "MATERIALNAME") == 0) {
			fscanf(fptr, "%s ", materialName);//used below, assumes the order of materialName first
			liveVar.material = 6;//Other by default
			for (int i = 0; i < 6; i++) {
				if (strcmpCaseInsenstive(materialNamesStr[i], materialName) == 0)
					liveVar.material = i;
			}
		}
		else if (strcmp(str, "MATERIAL") == 0) { //older project 
			fscanf(fptr, "%e %e %e %e %e %e", &(liveVar.E), &(liveVar.nu), &(liveVar.rho), &(liveVar.K), &(liveVar.alpha), &(liveVar.specificHeat));
			topopt.setIsotropicMaterialOfPart(currentPartId, liveVar.E, liveVar.nu, liveVar.rho, liveVar.K, liveVar.alpha, liveVar.specificHeat, materialName);
			liveVar.Y = 0.001*liveVar.E;// best guess
		}
		else if (strcmp(str, "E") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[0]));
			liveVar.E = materialProperties[0];//this will get overwritten for multiple bodies
		}
		else if (strcmp(str, "nu") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[1]));
			liveVar.nu = materialProperties[1];//this will get overwritten for multiple bodies
		}
		else if (strcmp(str, "Y") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[2]));
			liveVar.Y = materialProperties[2];//this will get overwritten for multiple bodies
		}
		else if (strcmp(str, "rho") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[3]));
			liveVar.rho = materialProperties[3];
		}
		else if (strcmp(str, "K") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[4]));
			liveVar.K = materialProperties[4];
		}
		else if (strcmp(str, "alpha") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[5]));
			liveVar.alpha = materialProperties[5];
		}
		else if (strcmp(str, "Cp") == 0) {
			fscanf(fptr, "%le ", &(materialProperties[6]));
			liveVar.specificHeat = materialProperties[6];
		}
		else if (strcmp(str, "MATERIAL_END") == 0) {
			topopt.setIsotropicMaterialPropertiesOfPart(currentPartId, materialProperties, materialName);
		}
		else if (strcmp(str, "NONDESIGN") == 0) {
			fscanf(fptr, "%d ", &(tempInt));
			if (tempInt == 1) topopt.setPartToNonDesign(currentPartId);
		}
		else if (strcmp(str, "UNITS") == 0)
			fscanf(fptr, "%d ", &(liveVar.paretoUnits));
		else if (strcmp(str, "TRIGROUP") == 0) {
			while (1) {
				fscanf(fptr, "%d %d", &tri, &group);
				if (tri == -1)
					break;
				vox.setTriGroup(tri, group);
			}
		}
		else if (strcmp(str, "BCSET") == 0) {
			fscanf(fptr, "%d ", &(liveVar.currentBCSet));
			bcSet = liveVar.currentBCSet;
			liveVar.nBCSet = MAX(liveVar.nBCSet, bcSet + 1);
			if (liveVar.nBCSet >= MAX_BC_SET - 1) {
				printf("Number of boundary conditions exceeds allowed\n");
				break;
			}
		}
		else if (strcmp(str, "BC") == 0) {
			fscanf(fptr, "%d ", &group);
			fscanf(fptr, "%d %d %d %lf %lf %lf %lf %lf %lf", &(BCType[bcSet][group][0]), &(BCType[bcSet][group][1]), &(BCType[bcSet][group][2]),
				&(BCValue[bcSet][group][0]), &(BCValue[bcSet][group][1]), &(BCValue[bcSet][group][2]),
				&(BCValue[bcSet][group][3]), &(BCValue[bcSet][group][4]), &(BCValue[bcSet][group][5]));
			guiState = BC_SUCCESFUL;
			// copy the Dirichlet to all other BC sets
			// We cannot allow different Dirichlet for different loads
			// if one BCset has zero dirichlet then so should all bcsets
			for (int i = 0; i < 3; i++) {
				if (BCType[bcSet][group][i] == ELASTICITY_DIRICHLET)
					for (int tempBCSet = 0; tempBCSet < MAX_BC_SET; tempBCSet++)
						BCType[tempBCSet][group][i] = ELASTICITY_DIRICHLET;
			}
		}
		else if (strcmp(str, "THERMALBC") == 0) {
			fscanf(fptr, "%d ", &group);
			fscanf(fptr, "%d %lf %lf %lf", &(ThermalBCType[bcSet][group]), &(ThermalBCArea[bcSet][group]), &(ThermalBCValue[bcSet][group]), &(ThermalBCRef[bcSet][group]));
			guiState = BC_SUCCESFUL;
		}
		else if (strcmp(str, "ELASTICITY_ACCELERATION") == 0) {
			fscanf(fptr, "%f %f %f", &(liveVar.XAcceleration), &(liveVar.YAcceleration), &(liveVar.ZAcceleration));
			ElasticityAcceleration[liveVar.currentBCSet][0] = liveVar.XAcceleration;
			ElasticityAcceleration[liveVar.currentBCSet][1] = liveVar.YAcceleration;
			ElasticityAcceleration[liveVar.currentBCSet][2] = liveVar.ZAcceleration;
			if (PARETO_VERSION >= 2017.02) {
				fscanf(fptr, "%f ", &(liveVar.radiansPerSecZAxis));
				ElasticityAcceleration[liveVar.currentBCSet][3] = liveVar.radiansPerSecZAxis;
			}
			guiState = BC_SUCCESFUL;
		}
		else if (strcmp(str, "SYMMETRY") == 0) {
			fscanf(fptr, "%d %d %d", &(liveVar.xSym), &(liveVar.ySym), &(liveVar.zSym));
		}
		else if (strcmp(str, "MESH") == 0) {
			if (guiState >= BC_SUCCESFUL) {
				// can be an Abaqus input file, Pareto mesh file, or a vox file, or a meshing directive
				fscanf(fptr, "%s", tempStr);
				if (strstr(tempStr, ".inp") || strstr(tempStr, ".INP")) {// abaqus file
					if (loadProjectVerbose) printf("Loading Abaqus inp file  %s\n", tempStr);
					if (loadAbaqusInp(tempStr) == 0) {
						printf("Load Project: Could not load inp file %s \n", tempStr);
						break;
					}
					guiState = HEX_SUCCESFUL;
					topopt.discardTemplates();
					topopt.createElementMatricesTemplates();
					topopt.disableNodalSpMV();

					solveStatic();/// solve right away
					liveVar.displayMode = DISPLAY_INITIAL_DELTA;
					guiState = STATICFEA_SUCCESFUL;
				}
				else if (strstr(tempStr, ".msh") || strstr(tempStr, ".MSH") || strstr(tempStr, ".g")) {// Pareto mesh file (windows) or Exodus file (Linux)
					if (loadProjectVerbose) printf("Loading Pareto mesh inp file  %s\n", tempStr);
					if (loadParetoMesh(tempStr) == 0) {
						printf("Load Project: Could not load mesh file %s \n", tempStr);
						break;
					}
					guiState = HEX_SUCCESFUL;
					topopt.discardTemplates();
					topopt.createElementMatricesTemplates();
					topopt.disableNodalSpMV();
					solveStatic();/// solve right away
					liveVar.displayMode = DISPLAY_INITIAL_DELTA;
					guiState = STATICFEA_SUCCESFUL;
				}

				else if (strstr(tempStr, ".vox")) {
					if (loadProjectVerbose) printf("Loading voxel file  %s\n", tempStr);
					if (readVoxel(tempStr) == 0) {
						printf("Load Project: Could not load vox file %s \n", tempStr);
						break;
					}
					liveVar.plotOverGeom = 0;//plot everything over mesh
				}
				else {
					int oldNMesh = liveVar.nElemsToMesh;
					sscanf(tempStr, "%d ", &(liveVar.nElemsToMesh));
					liveVar.meshQuality = MESH_VERY_COARSE;
					if (liveVar.nElemsToMesh > 10000)
						liveVar.meshQuality = MESH_COARSE;
					if (liveVar.nElemsToMesh > 25000)
						liveVar.meshQuality = MESH_MEDIUM;
					if (liveVar.nElemsToMesh > 50000)
						liveVar.meshQuality = MESH_FINE;
					if (liveVar.nElemsToMesh > 100000)
						liveVar.meshQuality = MESH_VERY_FINE;
					guiState = BC_SUCCESFUL;// need to remesh if this is encountered

					liveVar.nWorkSpaceGroups = (int)MIN(MAX_WORKSPACE_GROUPS, MAX(64, liveVar.nElemsToMesh / 750.0));
				}
			}
			else {
				printf("Could not create mesh; please set boundary conditions \n");
			}
		}
		else if (strcmp(str, "USEGPU") == 0) {
			fscanf(fptr, "%d", &(liveVar.useGPU));
		}
		else if (strcmp(str, "MESHASPECT") == 0) {
			fscanf(fptr, "%lf", &(temp));
			liveVar.maxAspectRatio = MIN(temp, 10.0);
			liveVar.maxAspectRatio = MAX(liveVar.maxAspectRatio, 1.0);
		}
		else if (strcmp(str, "NODEBASED") == 0) {
			fscanf(fptr, "%d", &(liveVar.nodeBasedKU));
		}
		else if (strcmp(str, "INITIALTEMP") == 0) {
			fscanf(fptr, "%f ", &(liveVar.initialTemperature));
		}
		else if (strcmp(str, "TIMESTEP") == 0) {
			fscanf(fptr, "%f %f", &(liveVar.transientDeltaTime), &(liveVar.transientTotalTime));
		}
		else if (strcmp(str, "VOLFRAC") == 0) {
			fscanf(fptr, "%lf", &temp);
			liveVar.desiredVolFraction = (float)temp;
		}
		else if (strcmp(str, "STEPSIZE") == 0) {
			fscanf(fptr, "%lf", &(temp));
			liveVar.stepSize = temp;
		}

		else if (strcmp(str, "DRAWDIR") == 0) {
			fscanf(fptr, "%d", &(liveVar.drawDirection));
		}
		else if (strcmp(str, "THROUGHCUTDIR") == 0) {
			fscanf(fptr, "%d", &(liveVar.throughCutDirection));
		}
		else if (strcmp(str, "CYCLICSYM") == 0) {
			fscanf(fptr, "%d", &(liveVar.cyclicSymmetry));
		}
		else if (strcmp(str, "SMOOTHING") == 0) {
			fscanf(fptr, "%d", &(liveVar.nSmoothingIterations));
		}
		else if (strcmp(str, "KEEPDIR") == 0) {
			fscanf(fptr, "%d", &(liveVar.keepDirichlet));
		}
		else if (strcmp(str, "COMMENT") == 0) {
			fgets(str, 255, fptr);// remainder of line
			fgets(str, 255, fptr);
			printf("Comment: %s \n", str);
		}
		else if (strcmp(str, "DEFLATION") == 0) {// enable/disable
			fscanf(fptr, "%d %d", &(liveVar.workSpaceMethod), &(liveVar.nWorkSpaceGroups));
			if (liveVar.nWorkSpaceGroups <= 0)
				liveVar.nWorkSpaceGroups = (int)MIN(MAX_WORKSPACE_GROUPS, MAX(64, liveVar.nElemsToMesh / 750.0));
		}
		else if (strcmp(str, "MAXTIME") == 0) {// time allowed
			fscanf(fptr, "%lf", &(temp));
			liveVar.maxTimeAllowed = (float)temp;
			//printf("Max time allowed is %f \n",liveVar.maxTimeAllowed);
		}
		else if (strcmp(str, "SOLVETHERMAL") == 0) {
			if (guiState < HEX_SUCCESFUL) {
				createTopOptMesh();
			}
			if (guiState >= HEX_SUCCESFUL) {
				fscanf(fptr, "%d", &(liveVar.currentBCSet));
				changeTopOptBC();
				solveThermoStatic();
			}
			else {
				printf("Could not solve thermal problem; please create mesh \n");
			}
		}
		else if (strcmp(str, "SOLVESTATIC") == 0) {
			if (guiState < HEX_SUCCESFUL) {
				createTopOptMesh();
			}
			if (guiState >= HEX_SUCCESFUL) {
				fscanf(fptr, "%d", &(liveVar.currentBCSet));
				changeTopOptBC();

				if (!checkForFiniteForce()) {
					printf("Warning: No force has been applied on the geometry\n");
					printf("---------------------------------------------\n");
				}
				solveStatic();
			}
			else {
				printf("Could not solve static problem; please create mesh \n");
			}
		}
		else if (strcmp(str, "SOLVEMODAL") == 0) {
			if (guiState < HEX_SUCCESFUL) {
				createTopOptMesh();
			}
			if (guiState >= HEX_SUCCESFUL) {
				fscanf(fptr, "%d %d", &(liveVar.currentBCSet), &(liveVar.nEigenModes));
				changeTopOptBC();
				solveModal();
			}
			else {
				printf("Could not solve modal problem; please create mesh \n");
			}
		}
		else if (strcmp(str, "TRIM") == 0) {
			fscanf(fptr, "%s", tempStr);
			if (strstr(tempStr, ".STL") || strstr(tempStr, ".stl")) {
				strcpy(fullPathName, liveVar.projectDirectory);
				strcat(fullPathName, tempStr);
				trimHexMeshAgainstInitialDesign(fullPathName);
			}
			else {
				printf(" Geometry file %s does not seem to have .stl extension \n", liveVar.geomFileName[liveVar.nSTLFilesLoaded]);
				break;
			}
		}
		else if (strcmp(str, "TRIMBOX") == 0) {
			double xyzMin[3], xyzMax[3];
			fscanf(fptr, "%lf %lf %lf %lf %lf %lf", &(xyzMin[0]), &(xyzMin[1]), &(xyzMin[2]), &(xyzMax[0]), &(xyzMax[1]), &(xyzMax[2]));
			trimHexMeshAgainstBoundingBox(xyzMin, xyzMax);

		}
		else if (strcmp(str, "CONSTRAINT") == 0) {
			fscanf(fptr, "%d ", &(ConstraintType[bcSet][NumberOfConstraints[bcSet]]));
			if (ConstraintType[bcSet][NumberOfConstraints[bcSet]] == DISPLACEMENT) { //Deng
				fscanf(fptr, "%lf %lf %lf %lf %lf %lf", &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][0]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][1]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][2]),
					&(ConstraintData[bcSet][NumberOfConstraints[bcSet]][3]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][4]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][5]));
			}
			else {
				fscanf(fptr, "%lf %lf %lf", &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][0]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][1]), &(ConstraintData[bcSet][NumberOfConstraints[bcSet]][2]));
			}

			NumberOfConstraints[bcSet]++;
			guiState = BC_SUCCESFUL;
		}
		else if (strcmp(str, "DUMP") == 0) {
			fscanf(fptr, "%d", &tempInt);
			double offset[] = { 0,0,0 };
			double tempLoc[3], locationDelta[3], locationStress[3], deltaMax, vonMisesMax;
			int	nodeDeltaMax, nodeStressMax;

			if (guiState >= THERMALFEA_SUCCESFUL) {
				if (tempInt == DUMP_BASIC) {// Basic result: nNodes, max delta info and max stress info
					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_deltaSigma.txt");
					FILE *fptr = fopen(tempStr, "w");
					if (fptr == NULL) {
						printf("Could not open file %s \n", tempStr);
						if (fptr == NULL) continue;
					}

					deltaMax = topopt.getDeformationMax(&nodeDeltaMax);
					vonMisesMax = topopt.getMaxVonMisesStress(&nodeStressMax);
					topopt.getNodeXYZ(nodeDeltaMax, tempLoc);
					vox.distanceOfPointToSTL(tempLoc, locationDelta);
					topopt.getNodeXYZ(nodeStressMax, tempLoc);
					vox.distanceOfPointToSTL(tempLoc, locationStress);
					fprintf(fptr, "%d\n", topopt.getNumElems());
					fprintf(fptr, "%.3e %.3e %.3e %.5e\n", locationDelta[0] + offset[0], locationDelta[1] + offset[1], locationDelta[2] + offset[2], deltaMax);
					fprintf(fptr, "%.3e %.3e %.3e %.5e\n", locationStress[0] + offset[0], locationStress[1] + offset[1], locationStress[2] + offset[2], vonMisesMax);
					fprintf(fptr, "%.3f \n", topopt.getCurrentVolumeFractionOfOptimizedParts());
					fclose(fptr);
				}
				else if (tempInt == DUMP_TEMP) {// Temperature at all  points requested 
					sprintf(xyzFile, CLIENTFILES, liveVar.userName);
					strcat(xyzFile, "_XYZ.txt");

					sprintf(outputFile, CLIENTFILES, liveVar.userName);
					strcat(outputFile, "_TEMP.txt");

					FILE *xyzFilePtr = fopen(xyzFile, "r");
					if (xyzFilePtr == NULL) {
						printf("Could not open file %s \n", xyzFile);
						continue;
					}
					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						fclose(xyzFilePtr);
						continue;
					}
					fscanf(xyzFilePtr, "%d", &nPoints);
					fprintf(outputFilePtr, "%d\n", nPoints);

					voxelElemPrev = 0;
					double pt[3];
					for (int i = 0; i < nPoints; i++) {
						fscanf(xyzFilePtr, "%lf %lf %lf ", &x, &y, &z);
						pt[0] = x - offset[0];
						pt[1] = y - offset[1];
						pt[2] = z - offset[2];
						topopt.getTemperatureAt(pt, &temp);
						fprintf(outputFilePtr, "%.2e  \n", temp);
					}
					fclose(xyzFilePtr);
					fclose(outputFilePtr);
				}
				else if (tempInt == DUMP_UVW) {// u v w and stress at all  points requested 
					sprintf(xyzFile, CLIENTFILES, liveVar.userName);
					strcat(xyzFile, "_XYZ.txt");

					sprintf(outputFile, CLIENTFILES, liveVar.userName);
					strcat(outputFile, "_UVWSTRESS.txt");

					FILE *xyzFilePtr = fopen(xyzFile, "r");
					if (xyzFilePtr == NULL) {
						printf("Could not open file %s \n", xyzFile);
						continue;
					}

					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						fclose(xyzFilePtr);
						continue;
					}
					fscanf(xyzFilePtr, "%d", &nPoints);
					fprintf(outputFilePtr, "%d\n", nPoints);

					voxelElemPrev = 0;

					for (int i = 0; i < nPoints; i++) {
						fscanf(xyzFilePtr, "%lf %lf %lf ", &x, &y, &z);
						pt[0] = x - offset[0];
						pt[1] = y - offset[1];
						pt[2] = z - offset[2];
						if (topopt.getElasticitySolutionAt(pt, uvw, stresses, &vonmises))
							fprintf(outputFilePtr, "%.2e %.2e %.2e %.2e \n", uvw[0], uvw[1], uvw[2], vonmises);
						else // use previous value, assuming point is close by
							fprintf(outputFilePtr, "%.2e %.2e %.2e %.2e  \n", uvw[0], uvw[1], uvw[2], vonmises);//previous value
					}
					fclose(xyzFilePtr);
					fclose(outputFilePtr);

				}
				else if (tempInt == DUMP_UVW_TEMP) {// u v w and stress, and temperature at all  points requested 
					sprintf(xyzFile, CLIENTFILES, liveVar.userName);
					strcat(xyzFile, "_XYZ.txt");
					sprintf(outputFile, CLIENTFILES, liveVar.userName);
					strcat(outputFile, "_UVWSTRESS.txt");
					FILE *xyzFilePtr = fopen(xyzFile, "r");
					if (xyzFilePtr == NULL) {
						printf("Could not open file %s \n", xyzFile);
						continue;
					}
					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						fclose(xyzFilePtr);
						continue;
					}
					fscanf(xyzFilePtr, "%d", &nPoints);
					fprintf(outputFilePtr, "%d\n", nPoints);

					voxelElemPrev = 0;
					for (int i = 0; i < nPoints; i++) {
						fscanf(xyzFilePtr, "%lf %lf %lf ", &x, &y, &z);
						pt[0] = x - offset[0];
						pt[1] = y - offset[1];
						pt[2] = z - offset[2];
						topopt.getElasticitySolutionAt(pt, uvw, stresses, &vonmises);
						topopt.getTemperatureAt(pt, &temp);
						fprintf(outputFilePtr, "%.2e %.2e %.2e %.2e %.2e \n", uvw[0], uvw[1], uvw[2], vonmises, temp);
					}
					fclose(xyzFilePtr);
					fclose(outputFilePtr);

				}
				else if (tempInt == DUMP_VOXELMESH) {
					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_voxelMesh.txt");
					vox.dumpVoxelData(tempStr);
				}
				else if (tempInt == DUMP_ABAQUS_RPT) {// u v w at all  mesh nodes in ABAQUS .rpt format
					if (loadProjectVerbose) printf("Dumping .rpt file\n");
					sprintf(outputFile, "%s", liveVar.userName);
					strcat(outputFile, ".rpt");

					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						continue;
					}
					fprintf(outputFilePtr, "**************************************** \n");
					fprintf(outputFilePtr, "Generated by Pareto \n\n\n");
					fprintf(outputFilePtr, " Label          @Loc 1          @Loc 1          @Loc 1 \n");
					fprintf(outputFilePtr, "-----------------------------------------------------------------\n");

					for (int i = 0; i < topopt.getNumNodes(); i++) {
						topopt.getNodeDeformation(i, &uvw[0], &uvw[1], &uvw[2], &vonmises);
						fprintf(outputFilePtr, "%d %10e %10e %10e \n", i + 1, uvw[0], uvw[1], uvw[2]);//note that node number starts from 1
					}
					fclose(outputFilePtr);
				}
				else if (tempInt == DUMP_BND) {// identify nodes that are constrained and nodes on which forces are applied
					//#DIRICHLET
					//nNodes
					//n1 n2 n3 n4  ...
					//#NEUMANN
					//nNodes
					//N1 N2 N3 N4 ...
					int nDirichletNodes, nNeumannNodes;
					sprintf(outputFile, "%s", liveVar.userName);
					strcat(outputFile, ".bnd");

					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						continue;
					}
					nDirichletNodes = 0;
					for (int i = 0; i < topopt.getNumNodes(); i++) {
						if (topopt.isNodeFree(i)) continue;
						nDirichletNodes++;
					}
					fprintf(outputFilePtr, "%d \n", nDirichletNodes);

					for (int i = 0; i < topopt.getNumNodes(); i++) {
						if (topopt.isNodeFree(i)) continue;
						fprintf(outputFilePtr, "%d ", i);
						nDirichletNodes++;
					}
					fprintf(outputFilePtr, "\n");

					double force[3], magnitude;
					nNeumannNodes = 0;
					for (int i = 0; i < topopt.getNumNodes(); i++) {
						topopt.getNeumannForce(i, force);
						magnitude = NORM3D(force);
						if (magnitude > 0) {
							nNeumannNodes++;
						}
					}
					fprintf(outputFilePtr, "%d \n", nNeumannNodes);
					for (int i = 0; i < topopt.getNumNodes(); i++) {
						topopt.getNeumannForce(i, force);
						magnitude = NORM3D(force);
						if (magnitude > 0) {
							fprintf(outputFilePtr, "%d ", i);
							nNeumannNodes++;
						}
					}

					fprintf(outputFilePtr, "\n");
					fclose(outputFilePtr);
				}
				else if (tempInt == DUMP_MESH) {//  dump mesh file
					sprintf(outputFile, "%s", liveVar.userName);
					strcat(outputFile, ".msh");
					topopt.dumpParetoMesh(outputFile);
				}
			}
			if (guiState >= MODALFEA_SUCCESFUL) {
				if (tempInt == DUMP_MODAL) {// Modal fea eigen values and modes requested
					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_eigenValues.txt");
					FILE *fptrTemp = fopen(tempStr, "w");
					if (fptrTemp == NULL) {
						printf("Could not open file %s \n", tempStr);
						continue;
					}
					fprintf(fptrTemp, "%d \n", liveVar.nEigenModes);
					for (int i = 0; i < liveVar.nEigenModes; i++) {
						fprintf(fptrTemp, "%e \n", sqrt(liveVar.eigenValues[i]) / (2 * 3.1415));
					}
					fclose(fptrTemp);

					sprintf(xyzFile, CLIENTFILES, liveVar.userName);
					strcat(xyzFile, "_XYZ.txt");

					FILE *xyzFilePtr = fopen(xyzFile, "r");
					if (xyzFilePtr == NULL) {
						printf("Could not open file %s \n", xyzFile);
						continue;
					}

					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_eigenModes.txt");

					FILE *outputFilePtr = fopen(tempStr, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", tempStr);
						fclose(outputFilePtr);
						continue;
					}
					// Find the maximum deflection for scaling
					double deformationMax[MAX_EIGEN_MODES];
					for (int i = 0; i < liveVar.nEigenModes; i++) {
						deformationMax[i] = 0;
						for (int node = 0; node < topopt.getNumNodes(); node++) {
							uvw[0] = liveVar.eigenModes[i][3 * node];
							uvw[1] = liveVar.eigenModes[i][3 * node + 1];
							uvw[2] = liveVar.eigenModes[i][3 * node + 2];
							delta = sqrt(uvw[0] * uvw[0] + uvw[1] * uvw[1] + uvw[2] * uvw[2]);
							deformationMax[i] = MAX(deformationMax[i], delta);
						}
						fprintf(outputFilePtr, "%d %lf\n", i, deformationMax[i]);
					}

					fscanf(xyzFilePtr, "%d", &nPoints);
					fprintf(outputFilePtr, "%d\n", nPoints);
					int nodes[8];
					double xi, eta, mu, N[8], uNodes[8], vNodes[8], wNodes[8];
					for (int i = 0; i < nPoints; i++) {
						fscanf(xyzFilePtr, "%d %lf %lf %lf ", &ptNumber, &x, &y, &z);
						x -= offset[0];
						y -= offset[1];
						z -= offset[2];
						//vox.closestVoxelNodeToPoint(x,y,z,&voxelNode);
						vox.closestVoxelElementToPoint(x, y, z, &(voxelElem), xietamu);
						if (voxelElem >= 0) {
							topopt.getElemNodes(voxelElem, nodes);
							xi = MAX(-1, MIN(1, xietamu[0]));
							eta = MAX(-1, MIN(1, xietamu[1]));
							mu = MAX(-1, MIN(1, xietamu[2]));
							topopt.computeN(xi, eta, mu, N);
						}
						// else use the previous set of nodes, xi eta and mu

						for (int mode = 0; mode < liveVar.nEigenModes; mode++) {
							// gather 
							for (int m = 0; m < 8; m++) {
								uNodes[m] = liveVar.eigenModes[mode][3 * nodes[m]];
								vNodes[m] = liveVar.eigenModes[mode][3 * nodes[m] + 1];
								wNodes[m] = liveVar.eigenModes[mode][3 * nodes[m] + 2];
							}
							uvw[0] = uvw[1] = uvw[2] = 0;
							for (int m = 0; m < 8; m++) {
								uvw[0] += N[m] * uNodes[m];
								uvw[1] += N[m] * vNodes[m];
								uvw[2] += N[m] * wNodes[m];
							}
							fprintf(outputFilePtr, "%.3f %.3f %.3f \n", uvw[0], uvw[1], uvw[2]);
						}
					}
					fclose(xyzFilePtr);
					fclose(outputFilePtr);
				}
			}

			if (guiState >= TOPOPT_SUCCESFUL) {
				if (tempInt == DUMP_TOPOPT_ISO) {// Intermediate topologies requested
					char resultFile[256], tempStr[256];
					double offset[] = { 0,0,0 };
					int nSavedVolFractions;
					double volFractions[MAX_ISO_SAVES], maxDisp[MAX_ISO_SAVES], maxStress[MAX_ISO_SAVES], maxCompliance[MAX_ISO_SAVES], timeTaken[MAX_ISO_SAVES];

					// Save the last error message
					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_topoptError.txt");
					FILE *fptr = fopen(tempStr, "w");

					if (fptr == NULL) {
						printf("Could not open file %s \n", tempStr);
						continue;
					}
					char errorMessage[1024];
					topopt.getLastTopoptErrorMessage(errorMessage);
					fprintf(fptr, "%s\n", errorMessage);
					fclose(fptr);

					// Save all the displacements and stresses at intermediate topologies
					nSavedVolFractions = topopt.getCurrentIsoCounter() + 1;
					sprintf(tempStr, CLIENTFILES, liveVar.userName);
					strcat(tempStr, "_topoptResults.txt");
					fptr = fopen(tempStr, "w");
					if (fptr == NULL) {
						printf("Could not open file %s \n", tempStr);
						continue;
					}
					topopt.getResultsAtAllVolFracs(volFractions, maxDisp, maxStress, maxCompliance, timeTaken);
					fprintf(fptr, "%d \n", nSavedVolFractions);
					for (int i = 0; i < nSavedVolFractions; i++) {
						fprintf(fptr, "%.3f %e %e %.2f \n", volFractions[i], maxDisp[i], maxStress[i], timeTaken[i]);
					}
					fclose(fptr);
					double actualFrac, time, isoCutoff;
					int isoCounter;
					//save isosurface at intermediate topologies
					for (int i = 0; i < nSavedVolFractions; i++) {
						strcpy(resultFile, liveVar.projectDirectory);
						strcat(resultFile, liveVar.modelName);
						sprintf(tempStr, ".%.3f", volFractions[i]);
						strcat(resultFile, tempStr);
						strcat(resultFile, ".stl");

						isoCounter = topopt.getClosestCounterAtVolFraction(volFractions[i], &actualFrac, &isoCutoff, &time);
						topopt.dumpIsoBinary(isoCounter, resultFile);
					}
				}
				if (tempInt == DUMP_ISO_UVW) {//  stress at the center of each triangle of the final isosurface
					int isoCounter;
					double xTri[3], yTri[3], zTri[3], triNormal[3];

					isoCounter = topopt.getCurrentIsoCounter();
					sprintf(outputFile, CLIENTFILES, liveVar.userName);
					strcat(outputFile, "_ISO_STRESS.txt");

					FILE *outputFilePtr = fopen(outputFile, "w");
					if (outputFilePtr == NULL) {
						printf("Could not open file %s \n", outputFile);
						continue;
					}
					nPoints = topopt.getNumIsoTriangles(isoCounter);//  stress at the center of each triangle
					double vertexNormals0[3], vertexNormals1[3], vertexNormals2[3];
					for (int tri = 0; tri < topopt.getNumIsoTriangles(isoCounter); tri++) {
						topopt.getIsoTriPts(isoCounter, tri, xTri, yTri, zTri, triNormal, vertexNormals0, vertexNormals1, vertexNormals2);
						pt[0] = (xTri[0] + xTri[1] + xTri[2]) / 3;
						pt[1] = (yTri[0] + yTri[1] + yTri[2]) / 3;
						pt[2] = (zTri[0] + zTri[1] + zTri[2]) / 3;
						if (topopt.getElasticitySolutionAt(pt, uvw, stresses, &vonmises))
							fprintf(outputFilePtr, "%.2e \n", vonmises);
						else // use previous value, assuming point is close by
							fprintf(outputFilePtr, "%.2e  \n", vonmises);//previous value
					}
					fclose(outputFilePtr);
				}
			}
		}
		else if (strcmp(str, "OBJECTIVE") == 0) {
			fscanf(fptr, "%d", &(liveVar.topoptObjective));
		}
		else if (strcmp(str, "MAXDISPLACEMENT") == 0) {
			fscanf(fptr, "%lf", &temp);
			liveVar.maxDisplacementAllowed = temp;
		}
		else if (strcmp(str, "STRESSSAFETYFACTOR") == 0) {
			fscanf(fptr, "%lf", &temp);
			liveVar.stressSafetyFactor = temp;
		}
		else if (strcmp(str, "TOPOPT") == 0) {
			char str[256];
			int nActiveBCSets = 0;
			fgets(str, 255, fptr);// rest of line
			fgets(str, 255, fptr);// next line
			char* token = strtok(str, " ");
			for (int i = 0; i < liveVar.nBCSet; i++)
				liveVar.activeBCSets[i] = false;
			while ((token) && (token[0] != '\n')) {
				int bcset = atoi(token);
				if (bcset > liveVar.nBCSet - 1)
					printf("Invalid BCSet: %d \n", bcset);
				else {
					nActiveBCSets++;
					liveVar.activeBCSets[bcset] = true;// this is an active BC Set
					liveVar.currentBCSet = bcset;

				}
				token = strtok(NULL, " ");
			}
			if ((guiState < HEX_SUCCESFUL))
				createTopOptMesh();
			if (guiState >= HEX_SUCCESFUL) {
				if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)) {
					multiLoadOptimize();
				}
				else {
					changeTopOptBC();
					singleLoadOptimize();
				}
			}
		}
		else if (strcmp(str, "SAVE_TIFF") == 0) {
			char str[256];
			fgets(str, 255, fptr);// rest of line
			fscanf(fptr, "%s", str);// next line, file name
			SaveAsTIFF(str);
		}
		else {
			printf("Unknown keyword %s \n", str);
			continue;
		}
	}

	fclose(fptr);
	strcpy(liveVar.userMessage, "Ready for FEA or optimization ...");
	printf("---------------------------------------------\n");

}
