#include "CoreFunctions.hpp"
#include "text3D.hpp"
#include "ParetoApplication.h"
#include "isosurface.h"
#include "Simplify.h"
#include <vector>

//#include "tetgen.h"

using namespace Simplify;

#ifdef _WIN32
//#define PMP
#endif

#ifdef PMP
#include "pmp\SurfaceMesh.h"
#include "pmp\algorithms\SurfaceSmoothing.h"
#include "pmp\algorithms\SurfaceRemeshing.h"
#include "pmp\algorithms\SurfaceFeatures.h"
using namespace pmp;
#endif

extern LiveVariables liveVar;// liveVariables
extern ParetoApplication topopt;
extern ParetoApplication topoptTemp;
extern Voxelizer vox;
extern Voxelizer voxTemp;


#define GUI_BUILD_DATE 1.23

extern bool getYesNoResponse(const char format[],...);
extern void writeWarningToGUIWindow(const char format[],...);
extern void writeMessageToGUIWindow(const char format[],...);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);
extern void updateProgressBar(const char title[], int percentComplete);
extern void resetProgressBar(const char title[]);
extern void setStatusMessage(const char str[]);
extern void grabImage(const char *fileName);
/************************************************************/
bool SIMPCallBackCoreFunctions(float fractionCompleted)
{
    return true;
}
/************************************************************/
bool dummyCallBackCoreFunctions(double fractionCompleted)
{
    return true;
}
/************************************************************/
bool meshingCallBackCoreFunctions(double fractionCompleted)
{
    updateProgressBar("Meshing Progress", (int) 100*fractionCompleted);
    return true;
}
/************************************************************/
bool frequencyResponseCallBackCoreFunctions(double fractionCompleted)
{
    updateProgressBar("Freq. Response Progress", (int) 100*fractionCompleted);

    return true;
}
/************************************************************/
bool STlReadCallBackCoreFunctions(double fractionCompleted)
{
    updateProgressBar("STL percent read",100*fractionCompleted);
    return true;
}
/************************************************************/
bool FEACallBackCoreFunctions(double fractionCompleted)
{
    updateProgressBar("FEA Progress", (int) 100*fractionCompleted);
    return true;
}
/**********************************************************************/
void printEstimatedTime(double estimatedTimeForCompletion,char *str)
{
    double hours, minutes, seconds;

    hours = floor(estimatedTimeForCompletion/3600);
    estimatedTimeForCompletion -= hours*3600;
    minutes = floor(estimatedTimeForCompletion/60);
    estimatedTimeForCompletion -= minutes*60;
    seconds =(estimatedTimeForCompletion);
    sprintf(str,"Estimated time: %.0f hr(s), %.0f min(s), %.0f sec(s)",hours,minutes,seconds);
}
/**********************************************************************/
void printMessage(const char *str)
{
    writeMessageToGUIWindow(str);
}
/**********************************************************************/
void alert(const char *str)
{
    writeWarningToGUIWindow(str);
}
/************************************************************/
bool elastoPlasticCallBack(double fractionCompleted)
{
    writeMessageToGUIWindow("Performing Elasto-plastic analyis");
    updateProgressBar("Elasto-plastic in progress .. ", (int) 100*fractionCompleted);
    return true;
}
/************************************************************/
bool dummyCallBackTopOpt(bool saveIsoSurface)
{
    return true;
}
/************************************************************/
bool topOptMultiMaterialCallBackFunction(bool saveIsoSurface)
{
    char tempStr[1024],topoptMsg[1024];
    string stlFile,offFile,stlFileRaw;
    indexed_mesh mesh;
    double xyz[3];
    double meshSize,costFrac,newEstimatedTimeForCompletion;
    int nSmootheningSteps;

    costFrac = topopt.getCurrentCostFraction_MultiMaterial();
    topopt.getLastTopoptErrorMessage(topoptMsg);

    if (saveIsoSurface){
        meshSize = topopt.getMeshElementSize();

        newEstimatedTimeForCompletion = liveVar.estimatedTimeForTopOpt*costFrac;
        printEstimatedTime(newEstimatedTimeForCompletion,tempStr);

        // we will save an stl file for each material, smoothen and then append
        for (int matl = 0; matl < topopt.getNumMaterials(); matl++) {
            sprintf(tempStr,".%d.%.3f.stl",matl,costFrac);
            stlFile  = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);
            offFile = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);

            topopt.createAndDumpIsoSurfaceFromMesh_MultiMaterial(stlFile.c_str(),matl);// not smooth, but connected

            mesh.load_from_binary_stereo_lithography_file(stlFile.c_str());
            remove(stlFile.c_str());

            mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);
            if (liveVar.snapToGeom) {
                for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
                    mesh.getVertex(i,xyz);
                    vox.snapPointToGeometry(xyz,liveVar.snapToGeomRelativeTol*meshSize);//find the new  xyz location after snapping to geometry
                    mesh.setVertex(i,xyz);//make the change only if the vertex normals have not changed much
                }
            }
            mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);

            mesh.save_to_extended_off_file(offFile.c_str());
            if (matl == 0)
                topopt.readExtendedOffFile(offFile.c_str());
            else
                topopt.readAndAppendFromExtendedOffFile(offFile.c_str());
            remove(offFile.c_str());

        }
        // do this after removing other files.
        if (liveVar.saveIntermediate)
            mesh.save_to_binary_stereo_lithography_file(stlFile.c_str());// useful if there is a software crash

        topopt.getLastTopoptErrorMessage(topoptMsg);
        writeMessageToGUIWindow("\nCurrent cost fraction: %.3f %s",costFrac, topoptMsg);
        updateProgressBar("TopOpt in progress ...", (int) 100*(1-costFrac)/(1-liveVar.desiredVolFraction));
    }

    if (liveVar.stopOptimization){
        liveVar.stopOptimization = false;
        return false;
    }
    return true;
}
/************************************************************/
bool multiscaleCallBackFunction(float percentCompleted)
{
    string stlFile;
    char tempStr[256];

    sprintf(tempStr,".multiscale.stl");
    stlFile  = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);
    topopt.createAndDumpIsoSurfaceFromPseudoDensity(stlFile.c_str(),0.5);

    voxTemp.readSTLGeom(stlFile.c_str());
    writeMessageToGUIWindow("Vol.: %.3f; compliance: %.4g; ",topopt.getCurrentVolumeFraction(), topopt.computeCompliance());

#ifdef QT_GUI_LIB
    updateProgressBar("TopOpt in progress ...", (int) percentCompleted);
 #endif
    if (liveVar.stopOptimization){
        liveVar.stopOptimization = false;
        return false;
    }
    return true;
}
/************************************************************/
bool topOptCallBackFunctionSIMP(float percentCompleted)
{
    string stlFile;
    char tempStr[256];

    sprintf(tempStr,".SIMP.stl");
    stlFile  = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);
    topopt.createAndDumpIsoSurfaceFromPseudoDensity(stlFile.c_str(),0.5);

    voxTemp.readSTLGeom(stlFile.c_str(),0,false,NULL,false);
    writeMessageToGUIWindow("Vol.: %.3f; compliance: %.4g; ",topopt.getCurrentVolumeFraction(), topopt.computeCompliance());

#ifdef QT_GUI_LIB
    updateProgressBar("TopOpt in progress ...", (int) percentCompleted);
 #endif
    if (liveVar.stopOptimization){
        liveVar.stopOptimization = false;
        return false;
    }
    return true;
}
/************************************************************/
bool topOptCallBackFunction(bool saveIsoSurface)
{
    char tempStr[512],topoptMsg[512];
    string stlFile,offFile;
    indexed_mesh mesh;
    double xyz[3];
    double meshSize,volFrac,newEstimatedTimeForCompletion;
    int nSmootheningSteps;

    volFrac = topopt.getCurrentVolumeFractionOfOptimizedParts();
    nSmootheningSteps = 20;

    if (saveIsoSurface){
        meshSize = topopt.getMeshElementSize();

        newEstimatedTimeForCompletion = liveVar.estimatedTimeForTopOpt*volFrac;
        printEstimatedTime(newEstimatedTimeForCompletion,tempStr);

        sprintf(tempStr,".%.3f.stl",volFrac);
        stlFile  = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);
        sprintf(tempStr,".%.3f.off",volFrac);
        offFile = liveVar.projectDirectory  + liveVar.modelName + string(tempStr);

        if (!liveVar.useTSFieldForIsoSurfaceExtraction) {// voxel mesh based
            topopt.createAndDumpIsoSurfaceFromMesh(stlFile.c_str());// not smooth, but connected
        }
        else{
            topopt.createAndDumpIsoSurfaceFromTSfield(stlFile.c_str());// smooth, but may not be connected
        }

        mesh.load_from_binary_stereo_lithography_file(stlFile.c_str());
        remove(stlFile.c_str());

        mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);
        if (liveVar.snapToGeom) {
            for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
                mesh.getVertex(i,xyz);
                vox.snapPointToGeometry(xyz,liveVar.snapToGeomRelativeTol*meshSize);//find the new  xyz location after snapping to geometry
                mesh.setVertex(i,xyz);
            }

        }

        mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);
        mesh.save_to_extended_off_file(offFile.c_str());
        topopt.readExtendedOffFile(offFile.c_str());
        remove(offFile.c_str());
        if (liveVar.saveIntermediate)
            mesh.save_to_binary_stereo_lithography_file(stlFile.c_str());// useful if there is a software crash

        liveVar.displayVolFraction = volFrac;
        topopt.getLastTopoptErrorMessage(topoptMsg);
        writeMessageToGUIWindow("Current vol fraction: %.3f. %s",volFrac, topoptMsg);
        updateProgressBar("TopOpt in progress ...", (int) 100*(1-volFrac)/(1-liveVar.desiredVolFraction));

    }
#ifdef QT_GUI_LIB
    updateProgressBar("TopOpt in progress ...", (int) 100*(1-volFrac)/(1-liveVar.desiredVolFraction));
 #endif


    if (liveVar.stopOptimization){
        liveVar.stopOptimization = false;
        return false;
    }
    return true;
}
/************************************************************/
bool latticeDesignCallBackFunction(double param)
{
    updateProgressBar("Lattice Design Progress", (int) 100*param);
    if (liveVar.stopOptimization){
        liveVar.stopOptimization = false;
        return false;
    }
    return true;
}
/**********************************************************************/
void CoreFunctions::safeFree(void *ptr)
{
    if (ptr) free(ptr);
}
/************************************************************/
void CoreFunctions::resetLiveVariablesTopOptConstraints()
{
    // all constraints are inactive by default
    liveVar.topoptConstraints.isExtrudeActive = false;
    liveVar.topoptConstraints.isAMBuildActive = false;
    liveVar.topoptConstraints.isDrawDirectionActive = false;
    liveVar.topoptConstraints.isCyclicSymmetryActive = false;
    liveVar.topoptConstraints.isMaxDisplacementActive = false;
    liveVar.topoptConstraints.isxGridActive = false;
    liveVar.topoptConstraints.isyGridActive = false;
    liveVar.topoptConstraints.iszGridActive = false;
    liveVar.topoptConstraints.isMinFeatureSizeActive = false;
    liveVar.topoptConstraints.isMaxFeatureSizeActive = false;
    liveVar.topoptConstraints.isStressSafetyActive = true;
    liveVar.topoptConstraints.isMinEigenActive = false;
    liveVar.topoptConstraints.isTemperatureLimitActive = false;
    liveVar.topoptConstraints.xSymmetry = false;
    liveVar.topoptConstraints.ySymmetry = false;
    liveVar.topoptConstraints.zSymmetry = false;
    liveVar.topoptConstraints.keepDirichlet = false;


    // typical values when active
    liveVar.topoptConstraints.drawDirection = 1;
    liveVar.topoptConstraints.extrudeDirection = 1;
    liveVar.topoptConstraints.AMBuildDirection = 0;
    liveVar.topoptConstraints.xGridPattern = 1;
    liveVar.topoptConstraints.yGridPattern = 1;
    liveVar.topoptConstraints.zGridPattern = 1;
    liveVar.topoptConstraints.cyclicSymmetry = 4;
    liveVar.topoptConstraints.stressSafetyFactor = 1;
    liveVar.topoptConstraints.maxDisplacementAllowed = 0.1;
    liveVar.topoptConstraints.minFeatureSize = 0.0001;
    liveVar.topoptConstraints.maxFeatureSize = 0.1;
    liveVar.topoptConstraints.minEigenValueAllowedHz = 1000;
    liveVar.topoptConstraints.maxTemperatureAllowed = 2000;
    liveVar.topoptConstraints.keepTopologyConnected = true;
}

/************************************************************/
void CoreFunctions::resetLiveVariables()
{
    int i, j, bcSet;
    liveVar.displayEdges = false;
    liveVar.plotAxis = true;
    liveVar.DOSWindowCheck = 0;
    liveVar.DOSWindowOpen = false;
    liveVar.logFilePtr = NULL;


    liveVar.MATERIAL_APPLIED = false;
    liveVar.GEOMETRY_UPLOADED = false;
    liveVar.BC_STRUCTURAL_APPLIED = false;
    liveVar.ACCELERATION_APPLIED = false;
    liveVar.BC_THERMAL_APPLIED = false;
    liveVar.MESH_APPLIED = false;
    liveVar.STATIC_FEA_SOLVED = false;
    liveVar.THERMAL_FEA_SOLVED = false;
    liveVar.TRANSIENT_FEA_SOLVED = false;
    liveVar.CONSTRAINTS_APPLIED = false;
    liveVar.TOPOPT_SOLVED = false;
    liveVar.LATTICE_DESIGNED = false;
    liveVar.GCODE_LOADED  = false;
    liveVar.whichLoadSetElasticityFEASolved = -1;

    liveVar.selectedPart = -1;
    liveVar.currentBCSet = 0;
    liveVar.nBCSet = 0;
    liveVar.ParetoBCType = PARETO_BC_FIXED;
    liveVar.assemblyMode = false;

    liveVar.deformationScaled = 1.0;
    liveVar.isDeformationScaled = true;
    liveVar.nonDesignCheck = false;
    liveVar.faceType = FACE_COARSE_CYL;
    liveVar.cutoffAngle = 15; ///degrees to find recursive mark neighboring triangles
    liveVar.normalToSurface = 0;

    liveVar.xCuttingPlanePercent = 0;
    liveVar.yCuttingPlanePercent = 0;
    liveVar.zCuttingPlanePercent = 0;

    liveVar.numberOfMaterialIndicesStored = 0;

    liveVar.showConcaveEdges = false;
    liveVar.XForce = 0;
    liveVar.YForce = 0;
    liveVar.ZForce = 0;
    liveVar.Pressure = 0;
    liveVar.Torque = 0;
    liveVar.XAcceleration =0;
    liveVar.YAcceleration =0;
    liveVar.ZAcceleration =0;
    liveVar.radiansPerSecZAxis =0;

    liveVar.CurrentElasticityBCLabel  = 0;

    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        liveVar.activeBCSets[bcSet] = true;// all bc sets are active
        for ( i =0; i <STL_MAX_GROUPS; i++){//default all BC are free Neumann
            for (j =0; j < 3;j++){
                liveVar.BCType[bcSet][i][j] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][i][j] = 0;
            }
        }
        for (i = 0; i < 4; i++)
            liveVar.ElasticityAcceleration[bcSet][i] = 0.0;// 4th component is radial acceleration

    }
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <STL_MAX_GROUPS; i++){//default all BC are free Neumann
            liveVar.ThermalBCArea[bcSet][i] = 0;
            liveVar.ThermalBCType[bcSet][i] = THERMAL_NEUMANN;
            liveVar.ThermalBCValue[bcSet][i] = 0;
            liveVar.ThermalBCRef[bcSet][i] = 0;
        }
    }

    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <MAX_CONSTRAINTS; i++){
            for (j =0; j < 6;j++){// (upper/lower,hard/soft/limit value,x,y,z
                liveVar.ConstraintData[bcSet][i][j] = 0;
            }
        }
    }
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <MAX_CONSTRAINTS; i++){
            liveVar.ConstraintType[bcSet][i] = -1;
        }
    }
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        liveVar.NumberOfConstraints[bcSet]=0;
    }

    liveVar.ThermalBCTypeGUI = PARETO_BC_TEMP;
    liveVar.ThermalBCValueGUI = 23; // C
    liveVar.ThermalBCReferenceGUI = 23;// for convection and radiation

    liveVar.nFacesSelected = 0;


    liveVar.nElemsToMesh = 10000;
    liveVar.meshQuality = 1;
    liveVar.conformingMesh = 0;
    liveVar.maxAspectRatio = 1.0;

    liveVar.zeroStrainTemperature = 300; // Kelvin for thermo-elasticity
    liveVar.useNonLinearThermal = 0;


    liveVar.selectedPart = -1;
    liveVar.partsToHide.clear();

    liveVar.partialvoxel = 0;
    liveVar.useBubbleFunctions = 1;
    liveVar.elemSignatureTolPower = 8;


    liveVar.forceRemesh = 1;
    liveVar.densityCutoff = 0.50;// fraction of nodes in element that must be inside to include the element
    liveVar.displayVolFraction = 1.0;



    //liveVar.prjFileName = string("");
    liveVar.voxFileName = string("");
    liveVar.plotBoundingBox = 0;
    liveVar.usePreconditioner = 1;
    liveVar.directSolver = false;
    liveVar.nodeBasedKU = 0;
    liveVar.workSpaceMethod = DEFLATION_RIGID;
    liveVar.nWorkSpaceGroups = 64;
    liveVar.cgResidualExponent = 9;
    liveVar.cgMaxIter = 20000;
    liveVar.eigRelAccuracyExponent = 2;

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayEdges = false;
    liveVar.displayQuality = 6;
    liveVar.nEigenModes = 1;
    liveVar.eigenValues = NULL;
    liveVar.eigenModes = NULL;
    liveVar.EigenMethod = 0;
    liveVar.displayEigenMode = true;
    liveVar.geomTransparent = false;
    liveVar.plotAnimation = false;
    liveVar.plotElasticityBC = true;
    liveVar.plotThermalBC = false;
    liveVar.plotTopOptConstraints = true;
    liveVar.plotSummaryResults = true;
    liveVar.tiffDPI = 300;

    liveVar.plotSupportStructures = 0;
    liveVar.maxTimeAllowed = 360000.0;//hundred hours for topopt, default
    liveVar.estimatedTimeForTopOpt = 0;
    liveVar.cuttingPlane = NO_CUTTINGPLANE;
    liveVar.animationVariable = 0;
    liveVar.staticAnimationScaling = 0;
    liveVar.modalAnimationScaling = 0;

    liveVar.timeForOneFEA = -1;// this will be computed when FEA is executed.

    liveVar.plotNormals = false;
    liveVar.plotNonDesignGeometry = true;
    liveVar.plotElementsWithDensityGreaterThan = 0.5;

    liveVar.nElastoPlasticForceSteps = 10;

    //optimization
    liveVar.useAllLoads = 1;
    liveVar.desiredVolFraction = 0.5;
    liveVar.convergedVolFraction = 1.0;
    liveVar.stepSize = 0.025;
    liveVar.stopOptimization = false;
	liveVar.estimatedTimeForTopOpt = 0;

    liveVar.topoptObjective = COMPLIANCE;

    resetLiveVariablesTopOptConstraints();


    liveVar.structuralDynamics.alphaM = 0.0;
    liveVar.structuralDynamics.betaK = 0.0005;
    liveVar.structuralDynamics.NewmarkGamma = 0.5;
    liveVar.structuralDynamics.NewmarkBeta = 0.25;
    liveVar.structuralDynamics.totalTime = 1;
    liveVar.structuralDynamics.timeStep = 0.005;
    liveVar.structuralDynamics.trackPoint[0] = 0.0;
    liveVar.structuralDynamics.trackPoint[1] = 0.0;
    liveVar.structuralDynamics.trackPoint[2] = 0.0;
    liveVar.structuralDynamics.trackDOF = 0;

    liveVar.nonlinear.nIterations = 100;
    liveVar.nonlinear.nSteps = 10;
    liveVar.nonlinear.tolerance = 1e-7;


    liveVar.imposedOverHangAngle = 35;// degrees
    liveVar.modelScale = 1.0;

    liveVar.snapToGeom = true;
    liveVar.snapToGeomRelativeTol =	0.9; //snap if the CAD surface is within this relative value with respect to mesh size

    liveVar.isTopOptComplete = false;
    liveVar.saveIntermediate = false;
    liveVar.stlFileName = string("");
    liveVar.useTSFieldForIsoSurfaceExtraction = true;

    liveVar.autoDesignTopologyError = 1.0; // for discarding small components and for checking symmetry between components
    liveVar.autoDesignSurfaceError = 1.0; // for contour approximation
    liveVar.autoDesignContourOffset = 4; // for offseting contours to include corners
    liveVar.autoDesignSmootheningIterations = 5;// for smoothening contours

    //lattice
    liveVar.latticeType = 0;
    liveVar.latticeUnitCellSizeInMM = 5;//mm
    liveVar.latticeResolution = 10;// how many voxel elements per side of lattice unit cell
    liveVar.latticeFillRatio = 0.5;
    liveVar.latticeSurfaceRetainType = LATTICE_SURFACE_CRITICAL;
    liveVar.latticeSmoothening = 10;
    liveVar.latticeSaveSTL = 0;
    char str[256];
    sprintf(str,"PARETO GUI BUILD DATE: %.2f",GUI_BUILD_DATE);
    setStatusMessage(str);
	liveVar.statusMessage = string(str);
    topopt.setLogFilePtr((FILE *)NULL);
}
/************************************************************/
int CoreFunctions::find_word(FILE *filein,const char *word)
{
    char str[256];
    fpos_t pos;
    fgetpos(filein, &pos);
    while(fscanf(filein, "%s", str) !=EOF) {
        if (strcmp(str,word) == 0)
            return 1;
    }
    // failed, return back to original position
    fsetpos(filein,&pos);
    return 0;
}
/************************************************************/
int CoreFunctions::getRecommendedDeflationGroups(int nElems)
{
    if (liveVar.workSpaceMethod == DEFLATION_TRANSLATION)
        return (int) MIN(MAX_WORKSPACE_GROUPS,MAX(MIN_WORKSPACE_GROUPS,nElems/1000.0));
    else if (liveVar.workSpaceMethod == DEFLATION_RIGID)
        return (int) MIN(MAX_WORKSPACE_GROUPS,MAX(MIN_WORKSPACE_GROUPS,nElems/500.0));
    else if (liveVar.workSpaceMethod == DEFLATION_STRAIN)
        return (int) MIN(MAX_WORKSPACE_GROUPS,MAX(MIN_WORKSPACE_GROUPS,nElems/250.0));
}
/************************************************************/
void CoreFunctions::oneTimeInitialization()
{
    liveVar.UNITS_APPLIED = true;// to enable drag and drop
    liveVar.paretoUnits = UNITS_METER;
    liveVar.temperatureUnits = UNITS_TEMPERATURE_KELVIN;
    liveVar.angleUnits = UNITS_ANGLE_DEGREE;
    //liveVar.material = ALLOY_STEEL;
    liveVar.E = 2.1e11;
    liveVar.nu = 0.28;
    liveVar.Y = 500e6;//Young's modulus
    liveVar.rho = 7700.0;
    liveVar.alpha = 6e-6;
    liveVar.K = 16;//conductivity
    liveVar.specificHeat = 452;
    liveVar.price = 2.0;

    liveVar.useGPU = 0;
    liveVar.assemblyMode = false;
    liveVar.nSTLFilesLoaded = 0;
    liveVar.userName = string("noName");

    liveVar.stlVerbose = false;
    liveVar.voxVerbose = false;
    liveVar.hexmeshVerbose = false;
    liveVar.plasticityVerbose = false;
    liveVar.deflationVerbose = false;
    liveVar.topoptVerbose = false;
    liveVar.showNativeCADGeometry = false;

}
/************************************************************/
void  CoreFunctions::initializeLiveVariables()
{

    vox.setMaxVoxels(MAX_VOXELS);

#ifdef COMPILE_GPUCODE
    liveVar.useGPU = 1;
    if (isThereValidCudaDevice() ==1) {
        liveVar.useGPU = 1;
        writeMessageToGUIWindow("Using GPU device for all FEA \n");
    }
    else
        writeMessageToGUIWindow("There is no valid NVidia GPU device; using CPU for FEA\n");
#endif
    resetLiveVariables();
    writeMessageToGUIWindow("Max number of elements allowed: %ld ",vox.getMaxVoxels());
}
/**********************************************************************/
void  CoreFunctions::freeEigenModesMemory(void)
{
    if (liveVar.eigenModes != NULL) {
        for (int i =0; i < MAX_EIGEN_MODES; i++)
            safeFree(liveVar.eigenModes[i]);
        safeFree(liveVar.eigenModes);
        safeFree(liveVar.eigenValues);
    }
}
/************************************************************/
bool CoreFunctions::checkForFiniteForce(void)
{
    int i,bcSet,faceID;

    bcSet = liveVar.currentBCSet;
    for (faceID = 0; faceID < vox.getNumTriGroups();faceID++) {
        for (i =0; i < 3; i++) {
            if ( (liveVar.BCType[bcSet][faceID][i] == ELASTICITY_NEUMANN) && (fabs(liveVar.BCValue[bcSet][faceID][i]) > 0))
                return true;// at least one non-zero force
            if ( (liveVar.BCType[bcSet][faceID][i] == ELASTICITY_TORQUE) && (fabs(liveVar.BCValue[bcSet][faceID][i]) > 0))
                return true;// at least one non-zero force
            if ( (liveVar.BCType[bcSet][faceID][i] == ELASTICITY_PRESSURE) && (fabs(liveVar.BCValue[bcSet][faceID][3]) > 0))
                return true;// at least one non-zero force
            if ( (liveVar.BCType[bcSet][faceID][i] == ELASTICITY_BEARING_LOAD) && (fabs(liveVar.BCValue[bcSet][faceID][i]) > 0))
                return true;// at least one non-zero force
            if ( (liveVar.BCType[bcSet][faceID][i] == ELASTICITY_REMOTE_LOAD) && (fabs(liveVar.BCValue[bcSet][faceID][i]) > 0))
                return true;// at least one non-zero force
        }
    }
    for (i =0; i < 4; i++)
        if (fabs(liveVar.ElasticityAcceleration[bcSet][i]) > 0)
            return true;// body force present

    return false;
}
/**********************************************************************/
bool CoreFunctions::distanceFunctionIsosurface(void)
{
    string voxFile;

    voxFile = liveVar.projectDirectory + string("temp.vxb");
    writeMessageToGUIWindow("CoreFunction: Attempting to create a voxel mesh; temp file %s ",voxFile.c_str());
    createVoxelMesh(voxFile.c_str());

    if (!checkMeshingError()){
        writeMessageToGUIWindow("CoreFunction: Error in creating mesh ");
        remove(voxFile.c_str()); //delete file
        return false;
    }

    if (!topopt.readVoxelFileBinary(voxFile.c_str())){
        writeMessageToGUIWindow("CoreFunction: Error in reading mesh ");
        return false;
    }
    remove(voxFile.c_str());

    double nodePt[3],closestPt[3],dist,*distFunction,normal[3],vec[3];
    int nNodes,closestTri;

    nNodes = topopt.getNumNodes() ;
    cout << "#Nodes = " << nNodes << endl;
    distFunction = (double *)malloc(nNodes*sizeof(double));
    for (int node = 0; node < nNodes; node++){
        topopt.getNodeXYZ(node,nodePt);
        dist = vox.distanceOfPointToSTL(nodePt,closestPt,&closestTri);
        //to determine sign, check normal
        for (int i =0; i < 3; i++)
            vec[i] = closestPt[i] - nodePt[i];
        vox.getNormalOfTriangle(closestTri,normal);
        if (DOTPROD3D(normal,vec) < 0)
            dist = -dist;

        distFunction[node] = dist;
    }

    // Used  both for plotting isosurfaces (if fptr is NULL) and for saving isosurfaces (not NULL)
    int elem,ntri;
    double x[8],y[8],z[8],p0[3],p1[3],p2[3],p01[3],p02[3],nrm,elemSize;
    int nodes[8];
    GRIDCELL grid;
    TRIANGLE triangles[10];
    float fbuf[12];
    unsigned short att = 0;
    unsigned int nTriangles;

    string isoSurfaceFile;

    isoSurfaceFile = liveVar.projectDirectory + string("isoSurface.stl");

    FILE *fptr = fopen(isoSurfaceFile.c_str(),"wb");
    if (fptr == NULL) {
        printf("Unable to open STL file: %s \n",isoSurfaceFile.c_str());
        return false;
    }
    char header[80];
    memset(header,'0', 80*sizeof( char));// dummy header
    fwrite(header,80,1, fptr);
    nTriangles = 0; // dummy, will overwrite later on
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);

    nTriangles = 0;
    double isoCutoff;
    elemSize = topopt.getMeshElementSize();
    isoCutoff = elemSize*0.1;
    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);
        for (int m = 0;m < 8;m++) {
            grid.p[m].x = x[m];
            grid.p[m].y = y[m];
            grid.p[m].z = z[m];
            grid.val[m] = distFunction[nodes[m]];
        }
        ntri = PolygoniseCube(grid,isoCutoff,triangles);

        for (int m = 0; m < ntri;m++) {
            p0[0] = triangles[m].p[0].x;
            p0[1] = triangles[m].p[0].y;
            p0[2] = triangles[m].p[0].z;

            p1[0] = triangles[m].p[1].x;
            p1[1] = triangles[m].p[1].y;
            p1[2] = triangles[m].p[1].z;

            p2[0] = triangles[m].p[2].x;
            p2[1] = triangles[m].p[2].y;
            p2[2] = triangles[m].p[2].z;

            nTriangles++;
            SUBTRACT3D(p01,p1,p0);
            SUBTRACT3D(p02,p2,p0);
            CROSS3D(normal,p01,p02);


            nrm = NORM3D(normal);

            fbuf[0] = (float) (normal[0]/nrm) ;
            fbuf[1] = (float) (normal[1]/nrm) ;
            fbuf[2] = (float) (normal[2]/nrm) ;

            fbuf[3]  = (float) (p0[0]);
            fbuf[4]  = (float) (p0[1]);
            fbuf[5]  = (float) (p0[2]);

            fbuf[6]  = (float) (p1[0]);
            fbuf[7]  = (float) (p1[1]);
            fbuf[8]  = (float) (p1[2]);

            fbuf[9]   = (float) (p2[0]);
            fbuf[10]  = (float) (p2[1]);
            fbuf[11]  = (float) (p2[2]);

            fwrite(fbuf, 12*sizeof(float), 1, fptr);
            fwrite(&att,sizeof(unsigned short),1,fptr);

        }
    }
    fseek(fptr,80,SEEK_SET);
    // write the number of triangles
    fwrite(&nTriangles, sizeof(unsigned int), 1, fptr);
    fclose(fptr);

    free(distFunction);

    setStatusMessage("isoGenerate done");
}
/**********************************************************************/
bool CoreFunctions::loadGeom(bool assembly)
{
    int success;
    double STLVolume,STLArea,maxLengthTypical,minLengthTypical,xyzMin[3],xyzMax[3];
    double maxLengthInMeters,minLengthInMeters;

    setStatusMessage("Loading Geometry");
    resetLiveVariables();
    topopt.removeAllMaterials();
    topopt.InitializeAllAFElasticityVariables();
    topopt.InitializeTopOpt(1);// to clear up TS memory

    vox.deleteCurrentModel();
    writeMessageToGUIWindow("---------------------------------------------");
    writeMessageToGUIWindow("Loading file %s ",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
    if (assembly) writeMessageToGUIWindow("... in assembly mode \n");

    // make sure file exists
    FILE *filein;
    filein = fopen(liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str(),"r");
    if (filein == 0) {
        writeWarningToGUIWindow("Could not find %s",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
        return false;
    }
    fclose(filein);

    success = vox.readSTLGeom(liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str(),liveVar.useGPU,assembly,STlReadCallBackCoreFunctions);

    resetProgressBar("Model read!");

    if (success !=1){
        writeWarningToGUIWindow("Geometry loading unsuccessful ... the model may contain non-manifold edges");
        //return false;
    }
    vox.resetGroups();
    vox.resetHighlight(); // unselect all highlighted triangles
    STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
    writeMessageToGUIWindow("#Triangles = %d; #Nodes = %d ",vox.getNumTriangles(),vox.getNumTriPoints());
    writeMessageToGUIWindow("STL volume = %e; STL Area = %e ",STLVolume,STLArea);

    liveVar.modelScale = vox.getModelScale();

    maxLengthTypical = 5;// meters
    minLengthTypical = 0.001;// meters
    vox.getBoundingBox(xyzMin,xyzMax);
    vox.getMinAndMaxLengths(&minLengthInMeters,&maxLengthInMeters);

    /*
    if (liveVar.paretoUnits == UNITS_METER){
        maxLengthInMeters *= 1.0;
        minLengthInMeters *= 1.0;
        if  (maxLengthInMeters > maxLengthTypical){
            if (maxLengthInMeters/39.36 < maxLengthTypical)
                alert("The part is large in meter units; suggested units: inches.");
            else if (maxLengthInMeters/1000.0 < maxLengthTypical)
                alert("The part is large in meter units; suggested units: mm.");
            else
                alert("Please check the units and model; the part appears to be too large in meters");
        }
        else if  (minLengthInMeters < minLengthTypical)
            alert("Please check the units and model; the part appears to be too small in meters");// the part is too small in meters, no other likely units
    }
    else if (liveVar.paretoUnits == UNITS_INCH){
        maxLengthInMeters *= 0.0254;
        minLengthInMeters *= 0.0254;
        if  (maxLengthInMeters > maxLengthTypical) {
            if (maxLengthInMeters*39.36/1000.0 < maxLengthTypical)
                alert("The part is large in inch units; suggested units: mm.");
            else
                alert("Please check the units; the part appears to be too large in inches");
        }
        if  (minLengthInMeters < minLengthTypical){
            if (minLengthInMeters*39.36 > minLengthTypical)
                alert("The part is small in inch units; suggested units: meters.");
            else
                alert("Please check the units; the part appears to be too small in inches");// the part is too small in meters, no other likely units
        }
    }
    else if (liveVar.paretoUnits == UNITS_MM){
        maxLengthInMeters *= 0.001;
        minLengthInMeters *= 0.001;
        if  (maxLengthInMeters > maxLengthTypical)
            alert("Please check the units and model; the part appears to be too large in mm.");// the part is too small in meters, no other likely units
        if  (minLengthInMeters < minLengthTypical){
            if (minLengthInMeters*39.36 > minLengthTypical)
                alert("The part is small in mm units; suggested units: inches.");
            else if (minLengthInMeters*1000 > minLengthTypical)
                alert("The part is small in mm units; suggested units: meters.");
            else
                alert("Please check the units and model; the part appears to be too small in mm");// the part is too small in meters, no other likely units
        }
    }
    */
    writeMessageToGUIWindow("CoreFunctions: Model length = %.5f",liveVar.modelScale);
    int nElementsRecommended = vox.getRecommendedNumberOfElements();
    nElementsRecommended = MAX(nElementsRecommended,10000);
    writeMessageToGUIWindow("CoreFunctions: nElementsRecommended = %d",nElementsRecommended);


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

    //int nDeflationGroups;
    //nDeflationGroups =  getRecommendedDeflationGroups(liveVar.nElemsToMesh);
    //liveVar.nWorkSpaceGroups = nDeflationGroups;


    liveVar.modelName  = liveVar.geomFileName[liveVar.nSTLFilesLoaded];
    liveVar.projectDirectory = liveVar.geomFileName[liveVar.nSTLFilesLoaded];

    //modelName ... strip all path data and extension
    size_t pos = liveVar.modelName.rfind('/');
    liveVar.modelName = (liveVar.modelName).substr(pos+1);

    // string .STL or .stl
    liveVar.modelName = (liveVar.modelName).substr(0,liveVar.modelName.length()-4);// strip .stl extension
    writeMessageToGUIWindow("Model name: %s",liveVar.modelName.c_str());


    pos = liveVar.projectDirectory.rfind('/');
    liveVar.projectDirectory = (liveVar.projectDirectory).substr(0,pos+1);
    writeMessageToGUIWindow("Project directory: %s",liveVar.projectDirectory.c_str());
    string tempFileName = liveVar.projectDirectory + string("Pareto.log");


    liveVar.logFilePtr = fopen(tempFileName.c_str(),"w");
    writeMessageToGUIWindow("Model name: %s",liveVar.modelName.c_str());
    writeMessageToGUIWindow("Project directory: %s",liveVar.projectDirectory.c_str());
    writeMessageToGUIWindow("Pareto Version %.2f",PARETO_VERSION);
    writeMessageToGUIWindow("Pareto Build Date %.2f",PARETO_BUILD_DATE);
    writeMessageToGUIWindow("GUI Build Date %.2f",GUI_BUILD_DATE);
    topopt.setLogFilePtr(liveVar.logFilePtr);
    vox.setLogFilePtr(liveVar.logFilePtr);

    tempFileName = liveVar.projectDirectory + string("charSet.bin");
    if (!t3dInit(tempFileName.c_str())) {
        writeWarningToGUIWindow("Could not initialize text3D");
        return false;
    }

    if (assembly)
        liveVar.nSTLFilesLoaded++;
    else
        liveVar.nSTLFilesLoaded = 1;

    if (vox.getNumParts() > 1) liveVar.nodeBasedKU = 0;


    //Set plotting routine mesh

    liveVar.voxFileName = liveVar.modelName + string(".vox");
    liveVar.stlFileName = liveVar.modelName + string(".stl");
    //liveVar.prjFileName = liveVar.prjFileName + string("Project.prj");


    // For topology otpimization, set the maximum allowable values
    liveVar.topoptConstraints.maxDisplacementAllowed =0.1*liveVar.modelScale;

    liveVar.topoptConstraints.minFeatureSize = -1;
    liveVar.topoptConstraints.maxFeatureSize = -1;

    liveVar.GEOMETRY_UPLOADED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    setStatusMessage("Geometry Loaded");
    return true;
}

/************************************************************/
void CoreFunctions::allocateMemoryForEigenModes(void)
{
    freeEigenModesMemory();
    liveVar.eigenModes = (double **)malloc(MAX_EIGEN_MODES*sizeof(double *));
    liveVar.eigenValues = (double *)malloc(MAX_EIGEN_MODES*sizeof(double));
    for (int i =0; i < MAX_EIGEN_MODES; i++)
        liveVar.eigenModes[i] = (double *)malloc(3*topopt.getNumNodes()*sizeof(double));

}
/************************************************************/
bool CoreFunctions::updateGeom(bool assembly)
{

    // This is used to load a new geometry file, while carrying over boundary conditions
    int success,bcSet,i,j;
    double STLVolume,STLArea,volumeInCm3;
    int BCTypeTemp[MAX_BC_SET][STL_MAX_GROUPS][3];
    double BCValueTemp[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
    int ThermalBCTypeTemp[MAX_BC_SET][STL_MAX_GROUPS];
    double ThermalBCAreaTemp[MAX_BC_SET][STL_MAX_GROUPS];
    double ThermalBCValueTemp[MAX_BC_SET][STL_MAX_GROUPS];
    double ThermalBCRefTemp[MAX_BC_SET][STL_MAX_GROUPS];// for convection and radiation

    voxTemp.copySTLGeometry(vox);//copy the current geom into voxTemp

    // save all the BC
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <STL_MAX_GROUPS; i++){//default all BC are free Neumann
            for (j =0; j < 3;j++){
                BCTypeTemp[bcSet][i][j] = liveVar.BCType[bcSet][i][j] ;
                BCValueTemp[bcSet][i][j] = liveVar.BCValue[bcSet][i][j];
            }
        }
    }
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <STL_MAX_GROUPS; i++){//default all BC are free Neumann
            ThermalBCTypeTemp[bcSet][i] = liveVar.ThermalBCType[bcSet][i];
            ThermalBCAreaTemp[bcSet][i] = liveVar.ThermalBCArea[bcSet][i];
            ThermalBCValueTemp[bcSet][i] = liveVar.ThermalBCValue[bcSet][i];
            ThermalBCRefTemp[bcSet][i] = liveVar.ThermalBCRef[bcSet][i];
        }
    }

    //resetLiveVariables();

    // copy back all the BC
    for (bcSet = 0; bcSet < MAX_BC_SET; bcSet++) {
        for ( i =0; i <STL_MAX_GROUPS; i++){
            for (j =0; j < 3;j++){
                liveVar.BCType[bcSet][i][j] = BCTypeTemp[bcSet][i][j] ;
                liveVar.BCValue[bcSet][i][j] = BCValueTemp[bcSet][i][j];
            }
        }
    }
    if ((liveVar.nSTLFilesLoaded == 0) || (!assembly))
        vox.deleteCurrentModel();


    writeMessageToGUIWindow("Loading file %s ",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
    if (assembly) writeMessageToGUIWindow("... in assembly mode ");
    success = 0;


    success = vox.readSTLGeom(liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str(),liveVar.useGPU,assembly,dummyCallBackCoreFunctions);
    if (success !=1){
        writeWarningToGUIWindow("STL loading unsuccessful");
        return false;
    }
    vox.resetGroups();
    vox.resetHighlight(); // unselect all highlighted triangles
    STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
    writeMessageToGUIWindow("#Triangles = %d; #Nodes = %d ",vox.getNumTriangles(),vox.getNumTriPoints());
    writeMessageToGUIWindow("STL volume = %e; STL Area = %e ",STLVolume,STLArea);

    if (liveVar.paretoUnits == UNITS_METER)
        volumeInCm3 = STLVolume*1000000.0;

    else if (liveVar.paretoUnits == UNITS_MM)
        volumeInCm3 = STLVolume*0.001;

    else if (liveVar.paretoUnits == UNITS_INCH)
        volumeInCm3 = STLVolume*2.54*2.54*2.54;

    writeMessageToGUIWindow("--------------------------------------------------");
    writeMessageToGUIWindow("Model volume (cm^3) = %.5f",volumeInCm3);
    writeMessageToGUIWindow("--------------------------------------------------");
    if ((volumeInCm3 < 0.1) && (liveVar.usingGUI))
        alert("Model appears to be too SMALL.\nPlease check units & reload");

    if ((volumeInCm3 > 500) && (liveVar.usingGUI))
        alert("Model appears to be too LARGE.\nPlease check units & reload");

    int nElementsRecommended = vox.getRecommendedNumberOfElements();
    nElementsRecommended = MAX(nElementsRecommended,4000);
    liveVar.nElemsToMesh = nElementsRecommended;
    liveVar.modelName = liveVar.geomFileName[liveVar.nSTLFilesLoaded];

    //modelName ... strip all path data and extension
    size_t pos = liveVar.modelName.rfind('/');
    liveVar.modelName = (liveVar.modelName).substr(pos+1);

    if (assembly)
        liveVar.nSTLFilesLoaded++;
    else
        liveVar.nSTLFilesLoaded = 1;


    // Now find closest triangles to copy over groups
    vox.transferClosestTriGroups(voxTemp);

    return true;
}
/************************************************************/
bool CoreFunctions::loadTemporaryGeom(bool assembly)
{
    // This is used to load a new geometry file, while carrying over boundary conditions
    bool success;

    writeMessageToGUIWindow("Loading file %s ",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
    if (assembly) writeMessageToGUIWindow("... in assembly mode ");

    success = voxTemp.readSTLGeom(liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str(),liveVar.useGPU,assembly,dummyCallBackCoreFunctions);
    if (!success){
        writeWarningToGUIWindow("STL loading unsuccessful");
        return false;
    }
    voxTemp.resetGroups();
    voxTemp.resetHighlight(); // unselect all highlighted triangles

    return true;
}
/************************************************************/
void CoreFunctions::changeTopOptBC(void)
{
    topopt.resetBC();
    topopt.allocateMemory_thermal();
    topopt.resetBC_thermal();
    int bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
        topopt.setBCOnNodesWithLabel(faceGroup,liveVar.BCType[bcSet][faceGroup],liveVar.BCValue[bcSet][faceGroup]);// sets h_f
    }

    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodes_thermal(faceGroup,liveVar.ThermalBCType[bcSet][faceGroup],liveVar.ThermalBCArea[bcSet][faceGroup],liveVar.ThermalBCValue[bcSet][faceGroup],liveVar.ThermalBCRef[bcSet][faceGroup]);


    double acc[4];

    for (int i = 0; i < 4; i++)
        acc[i] = liveVar.ElasticityAcceleration[bcSet][i];
    topopt.setElasticityAcceleration(acc);// add body force to h_f
}
/************************************************************/
/*void CoreFunctions::changeMaterial(void)
{
    double EScale = 1.0,rhoScale = 1.0,alphaScale =1.0,KScale = 1.0,priceScale = 1.0,E,nu,Y,rho,alpha,K,price;



    // By default all materials properties are in m,kg,s
    if (liveVar.paretoUnits == UNITS_MM) {// convert into mm,gm,s
        EScale = 1e-6;//N per mm^2
        rhoScale = 1e-6;
        alphaScale = 1.0;
        KScale = 1.0;
        priceScale = 0.001;
    }
    else if (liveVar.paretoUnits == UNITS_INCH) {// convert into in,lb,s
        EScale = 0.2248/(39.36*39.36);//0.0001450377*386.07;
        rhoScale = 3.612729e-005;
        alphaScale = 1.0;
        KScale = 1.0;
        priceScale = 0.454;
    }
    if (liveVar.material == ALLOY_STEEL) {
        E = 2.1e11*EScale;
        nu = 0.28;
        Y = 500e6*EScale;
        rho = 7700*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 16*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == ABS_PLASTIC) {
        E = 2.e9*EScale;
        nu = 0.39;
        Y = 42e6*EScale;
        rho = 1020*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 0.2*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == Ti6Al4V) {
        E = 11.12e10*EScale;
        nu = 0.33;
        Y = 903.213e6*EScale;
        rho = 4326*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 1.01*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == AL_1060) {
        E = 6.9e10*EScale;
        nu = 0.33;
        Y = 110e6*EScale;
        rho = 2700*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 205*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == CASTIRON_GR25) {
        E = 91e9*EScale;
        nu = 0.26;
        Y = 110e6*EScale;
        rho = 7200*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 46*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == AMS5862) {
        E = 2e11*EScale;
        nu = 0.27;
        Y = 1000e6*EScale;
        rho = 7833*rhoScale;
        alpha = 6e-6*alphaScale;
        K = 46*KScale;
        price = 2*priceScale;
    }
    else if (liveVar.material == OTHER_MATL) {
        E = 1;
        nu = 0.33;
        Y = 1.0;
        rho = 1;
        alpha = 1;
        K = 1;
        price = 1;
    }
    liveVar.E = E;
    liveVar.nu = nu;
    liveVar.Y = Y;
    liveVar.rho = rho;
    liveVar.K = K;
    liveVar.alpha = alpha;
    liveVar.specificHeat = 1.0;
    liveVar.price = price;
}
*/
//--------------------------------------------------------------------------
void CoreFunctions::eliminateEmptyGroups(void)
{
    bool groupUsed[STL_MAX_GROUPS];
    int mapping[STL_MAX_GROUPS];

    writeMessageToGUIWindow("Eliminating empty groups\n");

    for (int g = 0; g < STL_MAX_GROUPS; g++){
        groupUsed[g] = false;
        mapping[g] = -1;
    }

    groupUsed[0] = true;// always needed
    for (int tri = 0; tri < vox.getNumTriangles(); tri++)
        groupUsed[vox.getTriGroup(tri)] = true;

    int counter =0;
    for (int g = 0; g < STL_MAX_GROUPS; g++){
        if (groupUsed[g]){
            mapping[g] = counter;
            counter++;
        }
    }

    int oldGroup,newGroup;
    int newMaxGroupNumber = 0;
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        oldGroup = vox.getTriGroup(tri);
        newGroup = mapping[oldGroup];
        vox.setTriGroup(tri,newGroup);
        newMaxGroupNumber = MAX(newMaxGroupNumber,newGroup);
    }
    vox.setNumTriGroups(newMaxGroupNumber+1);
    for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++){
        for (int oldGroup = 0; oldGroup < STL_MAX_GROUPS; oldGroup++){
            newGroup = mapping[oldGroup];
            if (newGroup == -1) continue;
            for (int j = 0; j < 3; j++)
                liveVar.BCType[bcSet][newGroup][j] = liveVar.BCType[bcSet][oldGroup][j];

            for (int j = 0; j < 6; j++)
                liveVar.BCValue[bcSet][newGroup][j] = liveVar.BCValue[bcSet][oldGroup][j];


            liveVar.ThermalBCType[bcSet][newGroup] = liveVar.ThermalBCType[bcSet][oldGroup];
            liveVar.ThermalBCArea[bcSet][newGroup] = liveVar.ThermalBCArea[bcSet][oldGroup];
            liveVar.ThermalBCValue[bcSet][newGroup] = liveVar.ThermalBCValue[bcSet][oldGroup];
            liveVar.ThermalBCRef[bcSet][newGroup] = liveVar.ThermalBCRef[bcSet][oldGroup];
        }
    }
}
/************************************************************/
bool CoreFunctions::hasMaterialBeenApplied(void)
{
    char tempStr[256];
    for (int part = 0; part < vox.getNumParts(); part++) {
        writeMessageToGUIWindow("part %d material: %d ",part,topopt.getMaterialIDofPart(part));
        if (topopt.getMaterialIDofPart(part) < 0) {
            sprintf(tempStr,"Please apply material to part %d", part);
            writeWarningToGUIWindow("%s\n",tempStr);
            return false;
        }
    }
    return true;
}
/************************************************************/
void CoreFunctions::applyMaterial(void)
{
    double materialProperties[7];

    materialProperties[0] = liveVar.E;
    materialProperties[1] = liveVar.nu;
    materialProperties[2] = liveVar.Y;
    materialProperties[3] = liveVar.rho;
    materialProperties[4] = liveVar.K;
    materialProperties[5] = liveVar.alpha;
    materialProperties[6] = liveVar.specificHeat;

    if (liveVar.paretoUnits == UNITS_MM){
        // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 1000;
    }
    if (liveVar.paretoUnits == UNITS_INCH){
        // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 386.22;
    }
    for (int i = 0; i < 7; i++){
        if (materialProperties[i] < 0){
            alert("Material properties must all be positive\n");
            return;
        }
    }
    if (vox.getNumParts() == 1){// only one part exists
        liveVar.selectedPart = 0;
        topopt.setIsotropicMaterialPropertiesOfPart(liveVar.selectedPart,materialProperties,liveVar.materialName.c_str(),liveVar.price);
        if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(liveVar.selectedPart);
        else topopt.setPartToBeOptimized(liveVar.selectedPart);
        writeMessageToGUIWindow("Material applied to part %d ",liveVar.selectedPart);
    }
    else if (liveVar.selectedPart != -1) {// one of many parts selected
        topopt.setIsotropicMaterialPropertiesOfPart(liveVar.selectedPart,materialProperties,liveVar.materialName.c_str(),liveVar.price);
        if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(liveVar.selectedPart);
        else topopt.setPartToBeOptimized(liveVar.selectedPart);
        writeMessageToGUIWindow("Material applied to part %d ",liveVar.selectedPart);
    }
    else if (liveVar.selectedPart == -1){// no part selected among many parts
        // const int result = MessageBoxA(NULL, "Apply to all parts?", "Pareto",  MB_YESNO);
        int result = 0;
        switch (result){
        case 0:
            for (int part = 0; part < vox.getNumParts(); part++) {
                topopt.setIsotropicMaterialPropertiesOfPart(part,materialProperties,liveVar.materialName.c_str(),liveVar.price);
                if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(part);
                else topopt.setPartToBeOptimized(part);
            }
            writeMessageToGUIWindow("Material applied to all parts");
            break;
        case 1:
            writeWarningToGUIWindow("First select part before applying material ");
            break;
        }
    }


    vox.resetHighlight();
    bool materialSelectionDone;;
    materialSelectionDone = true;
    for (int part = 0; part < vox.getNumParts(); part++) {
        if (topopt.getMaterialIDofPart(part) < 0)
            materialSelectionDone = false;
    }
    if (materialSelectionDone){
        bool atLeastOnePartToBeOptimized = false;
        for (int part = 0; part < vox.getNumParts(); part++) {
            if (topopt.shouldPartBeOptimized(part)){
                atLeastOnePartToBeOptimized = true;
                break;
            }
        }
        if (!atLeastOnePartToBeOptimized){
            writeWarningToGUIWindow("All parts are non-design (cannot be optimized)! Please modify one or more parts.");
            return;
        }
    }
    if (materialSelectionDone)
        liveVar.MATERIAL_APPLIED = true;

    liveVar.STATIC_FEA_SOLVED = false;
    liveVar.TOPOPT_SOLVED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    setStatusMessage("Material Applied");

}
/************************************************************/
void CoreFunctions::applyElasticityBC(void)
{
    int i,j,bcSet,currentGroupNum;
    int oldGroup;
    double dir[3],nrm,area,averageNormal[3],cylinderAxis[3],axisPoint[3],cylinderRadius,triNormal[3],cutoff;
    bcSet = liveVar.currentBCSet;
    liveVar.nBCSet = MAX(liveVar.nBCSet, bcSet+1);

    if (!liveVar.GEOMETRY_UPLOADED) return;
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
    anyTriangleSelected= false;
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        if (vox.isTriangleHighlighted(tri)) {
            anyTriangleSelected= true;
            oldGroup = vox.getTriGroup(tri);
            for (i = 0; i < liveVar.nBCSet;i++) {
                for ( j =0; j < 3; j++)
                    liveVar.BCType[i][currentGroupNum][j] = liveVar.BCType[i][oldGroup][j];
                for ( j =0; j < 6; j++)
                    liveVar.BCValue[i][currentGroupNum][j] = liveVar.BCValue[i][oldGroup][j];

                liveVar.ThermalBCType[i][currentGroupNum] = liveVar.ThermalBCType[i][oldGroup];
                liveVar.ThermalBCArea[i][currentGroupNum] = liveVar.ThermalBCArea[i][oldGroup];
                liveVar.ThermalBCValue[i][currentGroupNum] = liveVar.ThermalBCValue[i][oldGroup];
                liveVar.ThermalBCRef[i][currentGroupNum] = liveVar.ThermalBCRef[i][oldGroup];
            }
        }
    }

    if (!anyTriangleSelected) {
        writeMessageToGUIWindow("Please select a triangle to apply boundary condition");
        return;
    }
    if (liveVar.ParetoBCType == PARETO_BC_CLEAR){
        liveVar.STATIC_FEA_SOLVED = false;
        liveVar.TOPOPT_SOLVED = false;
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.CurrentElasticityBCLabel =  vox.getNumTriGroups();
        vox.assignHighlightedTrianglesToGroup(0,averageNormal,&area, cylinderAxis,axisPoint,&cylinderRadius);
        vox.resetHighlight();
        return;
    }
    SURFACE_TYPE surfaceType = vox.assignHighlightedTrianglesToGroup(currentGroupNum,averageNormal,&area,
                                                                     cylinderAxis,axisPoint,&cylinderRadius);


    cout << "surfaceType " << surfaceType << endl;

    //printf("Current group number is %d \n",currentGroupNum);
    // now set the BC matrix
    //printf("Type of surface selected: %d \n",surfaceType);
    for ( j =0; j < 3; j++) {//default all BC are free Neumann
        liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
        liveVar.BCValue[bcSet][currentGroupNum][j] = 0;
    }
    switch (liveVar.ParetoBCType) {
    case PARETO_BC_CLEAR:
        for (j =0; j < 3; j++) {
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
            liveVar.BCValue[bcSet][currentGroupNum][j] = 0;
        }
        break;
    case PARETO_BC_FIXED:
        for (j =0; j < 3; j++) {
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_DIRICHLET;
            liveVar.BCValue[bcSet][currentGroupNum][j] = 0;
        }
        liveVar.BCValue[bcSet][currentGroupNum][0] = liveVar.XDisp;
        liveVar.BCValue[bcSet][currentGroupNum][1] = liveVar.YDisp;
        liveVar.BCValue[bcSet][currentGroupNum][2] = liveVar.ZDisp;
        if (!ALLOW_INDEPENDENT_RESTRAINTS){
            // copy the Dirichlet to all other BC sets
            // We cannot allow different Dirichlet for different loads
            for (i = 1; i < MAX_BC_SET;i++) {
                for (j =0; j < 3; j++)
                    liveVar.BCType[i][currentGroupNum][j] = ELASTICITY_DIRICHLET;
                for (j =0; j < 6; j++)
                    liveVar.BCValue[i][currentGroupNum][j] = 0;
            }
        }
        break;
    case PARETO_BC_XFIXED:
        liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
        liveVar.BCValue[bcSet][currentGroupNum][0] = liveVar.XDisp;
        // copy the Dirichlet to all other BC sets
        // We cannot allow different Dirichlet for different loads
        for (i = 1; i < MAX_BC_SET;i++) {
            liveVar.BCType[i][currentGroupNum][0] = ELASTICITY_DIRICHLET;
            liveVar.BCValue[i][currentGroupNum][0] = liveVar.XDisp;
        }
        break;
    case PARETO_BC_YFIXED:
        liveVar.BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
        liveVar.BCValue[bcSet][currentGroupNum][1] = liveVar.YDisp;
        // copy the Dirichlet to all other BC sets
        // We cannot allow different Dirichlet for different loads
        for (i = 1; i < MAX_BC_SET;i++) {
            liveVar.BCType[i][currentGroupNum][1] = ELASTICITY_DIRICHLET;
            liveVar.BCValue[i][currentGroupNum][1] = liveVar.YDisp;
        }
        break;
    case PARETO_BC_ZFIXED:
        liveVar.BCType[bcSet][currentGroupNum][2] = ELASTICITY_DIRICHLET;
        liveVar.BCValue[bcSet][currentGroupNum][2] = liveVar.ZDisp;
        // copy the Dirichlet to all other BC sets
        // We cannot allow different Dirichlet for different loads
        for (i = 1; i < MAX_BC_SET;i++) {
            liveVar.BCType[i][currentGroupNum][2] = ELASTICITY_DIRICHLET;
            liveVar.BCValue[i][currentGroupNum][2] = liveVar.ZDisp;
        }
        break;
    case PARETO_BC_FORCE:
        for (j = 0; j < 3; j++)
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
        liveVar.BCValue[bcSet][currentGroupNum][0] = liveVar.XForce;
        liveVar.BCValue[bcSet][currentGroupNum][1] = liveVar.YForce;
        liveVar.BCValue[bcSet][currentGroupNum][2] = liveVar.ZForce;
        break;
    case PARETO_BC_BEARING_LOAD:
        if (surfaceType != CYLINDER) {
            alert("Bearing load can only be applied on cylinders");
            break;
        }
        nrm = NORM3D(cylinderAxis);
        dir[0] = cylinderAxis[0]/nrm;
        dir[1] = cylinderAxis[1]/nrm;
        dir[2] = cylinderAxis[2]/nrm;
        PRINT3D(dir);
        if ((abs(dir[0]) < 0.99) && (abs(dir[1]) < 0.99) && (abs(dir[2]) < 0.99)){
            alert("Bearing load BC: the cylinder axis must be either x, y or z axis");
            break;
        }
        // force must be perpendicular to axis
        double forceDir[3],forceNrm,dtPrd;
        forceNrm = sqrt(liveVar.XForce*liveVar.XForce + liveVar.YForce*liveVar.YForce + liveVar.ZForce*liveVar.ZForce);
        if (forceNrm < 1e-12){
            alert("Bearing load BC: zero force detected");
            break;
        }
        forceDir[0] = liveVar.XForce/forceNrm;
        forceDir[1] = liveVar.YForce/forceNrm;
        forceDir[2] = liveVar.ZForce/forceNrm;
        dtPrd = DOTPROD3D(forceDir,dir);
        if (abs(dtPrd) > TOL){
            alert("Bearing load BC: force must be perpendicular to cylinder axis");
            break;
        }

        for (j = 0; j < 3; j++)
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_BEARING_LOAD;
        liveVar.BCValue[bcSet][currentGroupNum][0] = liveVar.XForce;
        liveVar.BCValue[bcSet][currentGroupNum][1] = liveVar.YForce;
        liveVar.BCValue[bcSet][currentGroupNum][2] = liveVar.ZForce;
        if (abs(dir[0]) > 0.99){
            liveVar.BCValue[bcSet][currentGroupNum][3] = 1;
            liveVar.BCValue[bcSet][currentGroupNum][4] = axisPoint[1];// sufficient to store y and z value
            liveVar.BCValue[bcSet][currentGroupNum][5] = axisPoint[2];// sufficient to store y and z value
        }
        else if (abs(dir[1]) > 0.99){
            liveVar.BCValue[bcSet][currentGroupNum][3] = 2;
            liveVar.BCValue[bcSet][currentGroupNum][4] = axisPoint[0];// sufficient to store x and z value
            liveVar.BCValue[bcSet][currentGroupNum][5] = axisPoint[2];// sufficient to store x and z value
        }
        else{
            liveVar.BCValue[bcSet][currentGroupNum][3] = 3;
            liveVar.BCValue[bcSet][currentGroupNum][4] = axisPoint[0];// sufficient to store x and y value
            liveVar.BCValue[bcSet][currentGroupNum][5] = axisPoint[1];// sufficient to store x and y value
        }
        break;
    case PARETO_BC_NORMAL_FORCE:
        if ((surfaceType != PLANAR) ) {
            if (liveVar.usingGUI)
                alert("Currently, normal force can only be applied on planar faces\n");
            else
                alert("Currently, normal force can only be applied on planar faces\n");
            break;
        }
        dir[0] = -averageNormal[0];
        dir[1] = -averageNormal[1];
        dir[2] = -averageNormal[2];

        // store the values as follows
        // dx dy dz pressure area 0

        nrm = NORM3D(dir)+1e-12;
        for (j = 0; j < 3; j++){
            dir[j] /= nrm;
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_NEUMANN;
            liveVar.BCValue[bcSet][currentGroupNum][j] = liveVar.NormalForce*dir[j];
        }

        break;
    case PARETO_BC_PRESSURE:
        if (surfaceType == PLANAR){
            dir[0] = -averageNormal[0];
            dir[1] = -averageNormal[1];
            dir[2] = -averageNormal[2];
            // store the values as follows
            // direction (dx dy dz) pressure area 0
            nrm = NORM3D(dir)+1e-12;
            for (j = 0; j < 3; j++){
                dir[j] /= nrm;
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;
                liveVar.BCValue[bcSet][currentGroupNum][j] = dir[j];
            }
            liveVar.BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
            liveVar.BCValue[bcSet][currentGroupNum][4] = area;
            liveVar.BCValue[bcSet][currentGroupNum][5] = -1;
        }
        else if (surfaceType == CYLINDER) {
            nrm = NORM3D(cylinderAxis);
            dir[0] = cylinderAxis[0]/nrm;
            dir[1] = cylinderAxis[1]/nrm;
            dir[2] = cylinderAxis[2]/nrm;
            if ((abs(dir[0]) < 0.99) && (abs(dir[1]) < 0.99) && (abs(dir[2]) < 0.99)){
                alert("Pressure BC: the cylinder axis must be either x, y or z axis\n");
                break;
            }
            for (j = 0; j < 3; j++)
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;

            for (j = 0; j < 3; j++)
                liveVar.BCValue[bcSet][currentGroupNum][j] = axisPoint[j];// additional information needed for cylinder

            liveVar.BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
            liveVar.BCValue[bcSet][currentGroupNum][4] = area;
            if (dir[0] >= 0.99)
                liveVar.BCValue[bcSet][currentGroupNum][5] = 1;
            else if (dir[1] >= 0.99)
                liveVar.BCValue[bcSet][currentGroupNum][5] = 2;
            else
                liveVar.BCValue[bcSet][currentGroupNum][5] = 3;

        }
        else {
            // store the values as follows
            // direction (0 0 0) pressure area 0
            // the direction will be determined inside AFElasticity based on mesh normal
            for (j = 0; j < 3; j++){
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_PRESSURE;
                liveVar.BCValue[bcSet][currentGroupNum][j] = 0;
            }
            liveVar.BCValue[bcSet][currentGroupNum][3] = liveVar.Pressure;
            liveVar.BCValue[bcSet][currentGroupNum][4] = area;
            liveVar.BCValue[bcSet][currentGroupNum][5] = 0;
        }
        break;
    case PARETO_BC_RETAIN://  retain this surface during optimization
        for (j = 0; j < 3; j++){
            liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_RETAIN;
            liveVar.BCValue[bcSet][currentGroupNum][j] = 0;
        }
        // if the Load Set is 0, copy the Retain to all other BC sets
        // We cannot allow different Dirichlet
        if (bcSet == 0) {
            for (i = 1; i < MAX_BC_SET;i++) {
                for (j =0; j < 3; j++){
                    liveVar.BCType[i][currentGroupNum][j] = ELASTICITY_RETAIN;
                    liveVar.BCValue[i][currentGroupNum][j] = 0;
                }
            }
        }
        break;
    case PARETO_BC_SLIDING:
        if ( (surfaceType != CYLINDER) && (surfaceType != PLANAR)) {
            alert("Currently, sliding can only be applied on cylinders and planar surfaces \n");
            break;
        }
        if (surfaceType == PLANAR) {
            dir[0] = -averageNormal[0];
            dir[1] = -averageNormal[1];
            dir[2] = -averageNormal[2];
            nrm = NORM3D(dir)+1e-12;
            for (j = 0; j < 3; j++)
                dir[j] /= nrm;

            if (fabs(dir[0]) > 0.99){
                liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
                liveVar.BCValue[bcSet][currentGroupNum][0] = 0;

                liveVar.BCType[bcSet][currentGroupNum][1] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][1] = 0;

                liveVar.BCType[bcSet][currentGroupNum][2] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][2] = 0;
            }
            else if (fabs(dir[1]) > 0.99){
                liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][0] = 0;

                liveVar.BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
                liveVar.BCValue[bcSet][currentGroupNum][1] = 0;

                liveVar.BCType[bcSet][currentGroupNum][2] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][2] = 0;

            }
            else if (fabs(dir[2]) > 0.99){
                liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][0] = 0;

                liveVar.BCType[bcSet][currentGroupNum][1] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][1] = 0;

                liveVar.BCType[bcSet][currentGroupNum][2] = ELASTICITY_DIRICHLET;
                liveVar.BCValue[bcSet][currentGroupNum][2] = 0;

            }
            else {
                alert("Currently, sliding can only be applied on planar surfaces with normal along x, y, or z \n");
                break;
            }

        }
        else if (surfaceType == CYLINDER) {
            nrm = NORM3D(cylinderAxis);
            dir[0] = cylinderAxis[0]/nrm;
            dir[1] = cylinderAxis[1]/nrm;
            dir[2] = cylinderAxis[2]/nrm;
            PRINT3D(dir);
            if ((abs(dir[0]) < 0.99) && (abs(dir[1]) < 0.99) && (abs(dir[2]) < 0.99)){
                alert("Sliding BC: the cylinder axis must be either x, y or z axis\n");
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
            for (i = 0; i < 3; i++){
                liveVar.BCType[bcSet][currentGroupNum][i] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum][i] = 0;

                liveVar.BCType[bcSet][currentGroupNum+1][i] = ELASTICITY_NEUMANN;
                liveVar.BCValue[bcSet][currentGroupNum+1][i] = 0;

                liveVar.BCType[bcSet][currentGroupNum+2][i] = ELASTICITY_RETAIN;
                liveVar.BCValue[bcSet][currentGroupNum+2][i] = 0;
            }

            // for the first two groups, decide which direction you are going to restrain
            if (abs(dir[0]) >= 0.99) {// axis is along x
                liveVar.BCType[bcSet][currentGroupNum][1] = ELASTICITY_DIRICHLET;
                liveVar.BCType[bcSet][currentGroupNum+1][2] = ELASTICITY_DIRICHLET;
            }
            else if (abs(dir[1]) >= 0.99) {// axis is along y
                liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
                liveVar.BCType[bcSet][currentGroupNum+1][2] = ELASTICITY_DIRICHLET;
            }
            else if (abs(dir[2]) >= 0.99) {// axis is along z
                liveVar.BCType[bcSet][currentGroupNum][0] = ELASTICITY_DIRICHLET;
                liveVar.BCType[bcSet][currentGroupNum+1][1] = ELASTICITY_DIRICHLET;
            }

            cutoff = cos(35*3.1415/180);// this is an arbitrary cutoff
            for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
                if (!vox.isTriangleHighlighted(tri))	continue;
                vox.getNormalOfTriangle(tri,triNormal);
                nrm = NORM3D(triNormal);
                triNormal[0] = triNormal[0]/nrm;
                triNormal[1] = triNormal[1]/nrm;
                triNormal[2] = triNormal[2]/nrm;

                if (abs(dir[0]) >= 0.99) {// axis is along x
                    if (abs(triNormal[1]) >= cutoff)
                        vox.setTriGroup(tri,currentGroupNum);
                    else if (abs(triNormal[2]) >= cutoff)
                        vox.setTriGroup(tri,currentGroupNum+1);
                    else
                        vox.setTriGroup(tri,currentGroupNum+2);// set to retain
                }
                else if (abs(dir[1]) >= 0.99) {// axis is along y
                    if (abs(triNormal[0]) >=cutoff)
                        vox.setTriGroup(tri,currentGroupNum);
                    else if (abs(triNormal[2]) >= cutoff)
                        vox.setTriGroup(tri,currentGroupNum+1);
                    else
                        vox.setTriGroup(tri,currentGroupNum+2);
                }
                else if (abs(dir[2]) >= 0.99) {// axis is along z
                    if (abs(triNormal[0]) >= cutoff)
                        vox.setTriGroup(tri,currentGroupNum);
                    else if (abs(triNormal[1]) >= cutoff)
                        vox.setTriGroup(tri,currentGroupNum+1);
                    else
                        vox.setTriGroup(tri,currentGroupNum+2);
                }
            }
        }
        break;
    case PARETO_BC_TORQUE:
        if ( (surfaceType != PLANAR) && (surfaceType != CYLINDER)) {
            //alert("Currently, torque can only be applied on planes or cylinders \n");
            //break;
            axisPoint[0] = axisPoint[1] = axisPoint[2] = 0; // default
        }
        if (surfaceType == PLANAR) {
            nrm = NORM3D(averageNormal);
            dir[0] = averageNormal[0]/nrm;
            dir[1] = averageNormal[1]/nrm;
            dir[2] = averageNormal[2]/nrm;
            nrm = NORM3D(dir)+1e-12;
            for (j = 0; j < 3; j++){
                dir[j] /= nrm;
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_TORQUE;
                liveVar.BCValue[bcSet][currentGroupNum][j] = (liveVar.Torque)*dir[j];
                liveVar.BCValue[bcSet][currentGroupNum][3+j] = axisPoint[j];// additional information needed for torque
            }
        }
        else if (surfaceType == CYLINDER) {
            nrm = NORM3D(cylinderAxis);
            dir[0] = cylinderAxis[0]/nrm;
            dir[1] = cylinderAxis[1]/nrm;
            dir[2] = cylinderAxis[2]/nrm;
            nrm = NORM3D(dir)+1e-12;
            for (j = 0; j < 3; j++){
                dir[j] /= nrm;
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_TORQUE;
                liveVar.BCValue[bcSet][currentGroupNum][j] = (liveVar.Torque)*dir[j];
                liveVar.BCValue[bcSet][currentGroupNum][3+j] = axisPoint[j];// additional information needed for torque
            }
        }
        else{
            axisPoint[0] = axisPoint[1] = axisPoint[2] = 0; // default
            nrm = NORM3D(cylinderAxis);
            dir[0] = cylinderAxis[0]/nrm;
            dir[1] = cylinderAxis[1]/nrm;
            dir[2] = cylinderAxis[2]/nrm;
            nrm = NORM3D(dir)+1e-12;
            for (j = 0; j < 3; j++){
                dir[j] /= nrm;
                liveVar.BCType[bcSet][currentGroupNum][j] = ELASTICITY_TORQUE;
                liveVar.BCValue[bcSet][currentGroupNum][j] = (liveVar.Torque)*dir[j];
                liveVar.BCValue[bcSet][currentGroupNum][3+j] = axisPoint[j];// additional information needed for torque
            }
        }
        break;
    }
    writeMessageToGUIWindow("Elasticity boundary condition applied on face group %d ",currentGroupNum);
    writeMessageToGUIWindow("---------------------------------------------");

    //eliminateEmptyGroups();
    liveVar.BC_STRUCTURAL_APPLIED = true;
    liveVar.STATIC_FEA_SOLVED = false;
    liveVar.TOPOPT_SOLVED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.CurrentElasticityBCLabel =  vox.getNumTriGroups();
    vox.resetHighlight();
}
/************************************************************/
void CoreFunctions::applyThermalBC(void)
{
    int i,bcSet,currentGroupNum;
    int oldGroup;
    double area,averageNormal[3],cylinderAxis[3],axisPoint[3],cylinderRadius;

    for (int part = 0; part < vox.getNumParts(); part++) {
        if (topopt.getMaterialIDofPart(part) < 0) {
            writeWarningToGUIWindow("Please apply material to part %d", part);
            return;
        }
    }

    bcSet = liveVar.currentBCSet;
    liveVar.nBCSet = MAX(liveVar.nBCSet, bcSet+1);

    currentGroupNum = vox.getNumTriGroups();// 0 is default for all
    // Need to make sure that BC associated with a different BCSet are not lost in the
    // reassignment of group numbers
    // Walk thru all highlighted triangles. If a previous group exists copy that BC to the
    // new group

    liveVar.plotElasticityBC = false;
    liveVar.plotThermalBC = true;
    bool anyTriangleSelected = false;
    for (int tri = 0; tri < vox.getNumTriangles(); tri++) {
        if (vox.isTriangleHighlighted(tri)) {
            anyTriangleSelected = true;
            oldGroup = vox.getTriGroup(tri);
            for (i = 0; i < liveVar.nBCSet;i++) {
                for (int j =0; j < 3; j++)
                    liveVar.BCType[i][currentGroupNum][j] = liveVar.BCType[i][oldGroup][j];
                for (int j =0; j < 6; j++)
                    liveVar.BCValue[i][currentGroupNum][j] = liveVar.BCValue[i][oldGroup][j];

                liveVar.ThermalBCType[i][currentGroupNum] = liveVar.ThermalBCType[i][oldGroup];
                liveVar.ThermalBCArea[i][currentGroupNum] = liveVar.ThermalBCArea[i][oldGroup];
                liveVar.ThermalBCValue[i][currentGroupNum] = liveVar.ThermalBCValue[i][oldGroup];
                liveVar.ThermalBCRef[i][currentGroupNum] = liveVar.ThermalBCRef[i][oldGroup];
            }
        }
    }
    if (!anyTriangleSelected) {
        //writeWarningToGUIWindow("Please select a triangle to apply boundary condition");
        return;
    }
    SURFACE_TYPE surfaceType = vox.assignHighlightedTrianglesToGroup(currentGroupNum,averageNormal,&area,
                                                                     cylinderAxis,axisPoint,&cylinderRadius);
    // now set the BC matrix

    if (fabs(area) == 0){
        writeWarningToGUIWindow("No surface was selected");
    }

    liveVar.ThermalBCArea[bcSet][currentGroupNum] = area;
    liveVar.ThermalBCType[bcSet][currentGroupNum] = THERMAL_NEUMANN;
    liveVar.ThermalBCValue[bcSet][currentGroupNum] = 0;
    liveVar.ThermalBCRef[bcSet][currentGroupNum] = 0;

    switch (liveVar.ThermalBCTypeGUI) {
    case PARETO_BC_TEMP:
        liveVar.ThermalBCType[bcSet][currentGroupNum] = THERMAL_DIRICHLET;
        liveVar.ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValueGUI;

        // if the Load Set is 0, copy the Dirichlet to all other BC sets
        // We cannot allow different Dirichlet
        if (bcSet == 0) {
            for (i = 1; i < MAX_BC_SET;i++) {
                liveVar.ThermalBCType[i][currentGroupNum] = THERMAL_DIRICHLET;
                liveVar.ThermalBCValue[i][currentGroupNum] = liveVar.ThermalBCValueGUI;
            }
        }
        break;
    case PARETO_BC_HEAT:
        liveVar.ThermalBCType[bcSet][currentGroupNum] = THERMAL_NEUMANN;
        liveVar.ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValueGUI;
        break;
    case PARETO_BC_CONVECTION:
        liveVar.ThermalBCType[bcSet][currentGroupNum] = THERMAL_CONVECTION;
        liveVar.ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValueGUI;
        liveVar.ThermalBCRef[bcSet][currentGroupNum] = liveVar.ThermalBCReferenceGUI;
        break;
    case PARETO_BC_RADIATION:
        liveVar.ThermalBCType[bcSet][currentGroupNum] = THERMAL_RADIATION;
        liveVar.ThermalBCValue[bcSet][currentGroupNum] = liveVar.ThermalBCValueGUI;
        liveVar.ThermalBCRef[bcSet][currentGroupNum] = liveVar.ThermalBCReferenceGUI;
        break;
    }
    liveVar.BC_THERMAL_APPLIED = true;
    liveVar.CurrentThermalBCLabel = currentGroupNum;
    vox.resetHighlight();

    setStatusMessage("Applied Thermal BC");
}
/************************************************************/
void CoreFunctions::saveSimplifiedIsoSurface(int isoCounter, int simplificationLevel, string stlFile)
{
    int targetNumberOfTriangles,isoTriangles,nSmootheningSteps;
    indexed_mesh mesh;
    double xyz[3];
    char offFile[] = "temp.off";
    double meshSize = topopt.getMeshElementSize();
    isoTriangles = topopt.getNumIsoTriangles(isoCounter);
    switch(simplificationLevel) {
        case 0: targetNumberOfTriangles = isoTriangles*0.1;break;
        case 1: targetNumberOfTriangles = isoTriangles*0.2;break;
        case 2: targetNumberOfTriangles = isoTriangles*0.3;break;
        case 3: targetNumberOfTriangles = isoTriangles*0.6;break;
        case 4: targetNumberOfTriangles = isoTriangles*0.9;break;
    }
    writeMessageToGUIWindow("Target triangles = %d ",targetNumberOfTriangles);
    topopt.dumpIsoOffFormat(isoCounter,offFile);
    Simplify::load_off(offFile);
    remove(offFile);
    Simplify::simplify_mesh(targetNumberOfTriangles);
    Simplify::write_stl_binary(stlFile.c_str());

    mesh.load_from_binary_stereo_lithography_file(stlFile.c_str());
    nSmootheningSteps = 15;
    mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);

    for (int iter = 0; iter < 5; iter ++){
        for (int i = 0;i < mesh.getNumberOfVertices(); i++) {
            mesh.getVertex(i,xyz);
            vox.snapPointToGeometry(xyz,liveVar.snapToGeomRelativeTol*meshSize);//find the new  xyz location after snapping to geometry
            mesh.setVertex(i,xyz);
        }
        mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);
    }
    mesh.save_to_binary_stereo_lithography_file(stlFile.c_str());
}
/************************************************************/
double roundDouble(double x,int nDecimals)
//round up a float type and show one decimal place
{
      x *= pow(10.0,nDecimals);
      x = NEARESTINT(x);
      x /= pow(10.0,nDecimals);
      return x;
}

/************************************************************/
/*
bool CoreFunctions::blendCADandIsoSurface(void)
{
    #ifdef DELAUNAY
    // Algorithm
    // 1. Create pointcloud by selectively sampling from  CAD and topopt isosurface
    // 2. Create a delaunay tetrahedralization of point data
    // 3. Eliminate tets that are outside the current mesh
    // 4. Recover the surface triangulation



    int savedCounter,elem,nCADPts;
    double actualVolumeFraction,isoCutoff, timeInSecs;
    double triNormal[3],meshSize,meshPt[3];
    bool *isElemOnCADBoundary;
    int nDecimals;
    std::vector<Array3D> points,normals;
    Array3D pt,normal;
    vector<Point3d> edgePoints;


    // 1. Generate point cloud on CAD model
    // 2. Select CAD points that are close to active voxel elements
    // 3. Mark the corresponding voxel as CAD-voxel
    // 4. Select isosurface points that are not in CAD-voxel

    isElemOnCADBoundary = (bool *)malloc(topopt.getNumElems()*sizeof(bool));
    memset(isElemOnCADBoundary,false,topopt.getNumElems()*sizeof(bool));

    meshSize = topopt.getMeshElementSize();
    points.clear();
    normals.clear();
    edgePoints.clear();

    nDecimals = 5;
    resetProgressBar("Generating CAD Points");
    vox.generatePointsOnSharpEdges(meshSize,edgePoints);
    for (int e = 0; e < edgePoints.size(); e++){
        meshPt[0] = edgePoints[e].x;
        meshPt[1] = edgePoints[e].y;
        meshPt[2] = edgePoints[e].z;
        elem = topopt.findNonVoidElemClosestTo(meshPt[0],meshPt[1],meshPt[2],1.5);
        if (elem < 0) continue;
        isElemOnCADBoundary[elem] = true;
        vox.getNormalAtPoint(meshPt,triNormal);
        for (int i = 0; i < 3; i++){
            pt[i] = roundDouble(meshPt[i],nDecimals);// important to round the decimals, else Delaunay gets messed up
            normal[i] = roundDouble(triNormal[i],nDecimals);
        }
        points.push_back(pt);// duplicates are eliminated later on
        normals.push_back(normal);
    }

    // generate on surface by walking on boundary nodes of voxel mesh
    for (int node = 0; node < topopt.getNumNodes(); node++){
        if (!topopt.isNodeOnBoundary(node)) continue;
        topopt.getNodeXYZ(node,meshPt);
        vox.snapPointToTriangulation(meshPt,0.9*meshSize);//find the new  xyz location after snapping to geometry
        elem = topopt.findNonVoidElemClosestTo(meshPt[0],meshPt[1],meshPt[2],1.5);
        if (elem < 0) continue;
        isElemOnCADBoundary[elem] = true;
        vox.getNormalAtPoint(meshPt,triNormal);
        for (int i = 0; i < 3; i++){
            pt[i] = roundDouble(meshPt[i],nDecimals);// important to round the decimals, else Delaunay gets messed up
            normal[i] = roundDouble(triNormal[i],nDecimals);
        }
        points.push_back(pt);// duplicates are eliminated later on
        normals.push_back(normal);
    }
    updateProgressBar("Generating CAD points",100.0);
    resetProgressBar("Generating isosurface Points");
    nCADPts= points.size();
    savedCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&actualVolumeFraction,&isoCutoff, &timeInSecs);
    // now cherry pick points from isosurface
    for (int node = 0; node < topopt.getNumIsoNodes(savedCounter); node++){
        topopt.getIsoNodeXYZ(savedCounter,node,meshPt);
        elem = topopt.findNonVoidElemClosestTo(meshPt[0],meshPt[1],meshPt[2],2.25);
        if (elem < 0) continue;
        if (isElemOnCADBoundary[elem]) continue;
        if (!topopt.isElemInInterior(elem)) continue;
        topopt.getIsoNodeNormal(savedCounter,node,triNormal);
        for (int i = 0; i < 3; i++){
            pt[i] = roundDouble(meshPt[i],nDecimals);
            normal[i] = roundDouble(triNormal[i],nDecimals);
        }
        points.push_back(pt);
        normals.push_back(normal);
    }
    updateProgressBar("Generating isosurface points",100.0);

    std::vector<Array3D> uniquePoints,uniqueNormals;
    bool duplicate;
    double d;

    uniquePoints.clear();
    uniqueNormals.clear();
    resetProgressBar("Eliminating duplicate pts");
    for (int i = 0; i < points.size(); i++){
        duplicate = false;
        for (int j = 0; j < uniquePoints.size(); j++){
            d = sqrt(pow(points[i][0] -uniquePoints[j][0],2)+
                     pow(points[i][1] -uniquePoints[j][1],2)+
                     pow(points[i][2] -uniquePoints[j][2],2));
            if (d < meshSize*0.25){
              duplicate = true;
              break;
            }
        }
        if (!duplicate){
            uniquePoints.push_back(points[i]);
            uniqueNormals.push_back(normals[i]);
        }
    }
    updateProgressBar("Eliminating duplicate pts",100.0);

    string pointCloudFile = liveVar.projectDirectory + liveVar.modelName + string("_pointCloud.txt");

    FILE *fptr = fopen(pointCloudFile.c_str(),"w");
    fprintf(fptr,"%d \n",uniquePoints.size());
    for (int i = 0; i < uniquePoints.size(); i++){
         fprintf(fptr,"%g %g %g ",uniquePoints[i][0],uniquePoints[i][1],uniquePoints[i][2]);
         fprintf(fptr,"%f %f %f ",uniqueNormals[i][0],uniqueNormals[i][1],uniqueNormals[i][2]);
         if (i < nCADPts)
             fprintf(fptr,"1 \n");
         else
             fprintf(fptr,"0 \n");
    }
    fprintf(fptr,"%g \n",meshSize);// also dump the mesh size for convenience
    fclose(fptr);

    tetgenio in, out;

    updateProgressBar("Generating tets",0.0);

    // All indices start from 0
    in.firstnumber = 0;
    in.numberofpoints =  uniquePoints.size();
    in.pointlist = new REAL[in.numberofpoints * 3];
    for (int i = 0; i < uniquePoints.size(); i++){
        in.pointlist[3*i] = uniquePoints[i][0];
        in.pointlist[3*i+1] = uniquePoints[i][1];
        in.pointlist[3*i+2] = uniquePoints[i][2];
    }

    in.numberoffacets = 0;
    tetgenbehavior tetgenParams;// all default
    tetrahedralize(&tetgenParams, &in, &out);
    updateProgressBar("Generating tets",100.0);

    string tetGenFiles = liveVar.projectDirectory + liveVar.modelName + string("_tetGen");

    char tetGenFile[256];

    strcpy(tetGenFile,tetGenFiles.c_str());
    out.save_nodes(tetGenFile);
    out.save_faces(tetGenFile);
    out.save_elements(tetGenFile);
    DelaunaySurface delaunaySurf;
    delaunaySurf.setPoints(uniquePoints,uniqueNormals);

    string nodeFile = tetGenFiles + string(".node");
    string eleFile = tetGenFiles + string(".ele");

    delaunaySurf.readTetmesh(nodeFile.c_str(),eleFile.c_str());
    updateProgressBar("Constructing surface",0.0);
    delaunaySurf.extractSurface(meshSize);
    updateProgressBar("Constructing tets",100.0);

    string delaunaySurfFile = liveVar.projectDirectory + liveVar.modelName + string("_blended.off");
    delaunaySurf.saveSurfaceAsOffFile(delaunaySurfFile.c_str());
    voxTemp.deleteCurrentModel();
    voxTemp.readOffFile(delaunaySurfFile.c_str(),0);

    resetProgressBar("Topology cleaned-up");

    points.clear();
    uniquePoints.clear();
    normals.clear();
    uniqueNormals.clear();

    free(isElemOnCADBoundary);

    liveVar.displayGeom = DISPLAY_GEOM_OTHER;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayEdges = true;
#endif
    return true;
}*/
/************************************************************/
bool CoreFunctions::uniformRemeshing(vector<Array3D> &coords, vector<Array3I> &triangles, int nTrianglesDesired, double creaseAngle)
{
 #ifdef PMP
    size_t numNodes = coords.size();
    size_t numFaces = triangles.size();

    auto getLength = [](const Array3D &p0, const Array3D &p1)
    {
        double dx = p1[0] - p0[0];
        double dy = p1[1] - p0[1];
        double dz = p1[2] - p0[2];
        return sqrt(dx*dx + dy*dy + dz*dz);
    };

    auto getArea = [&](const Array3D &p0, const Array3D &p1, const Array3D &p2)
    {
        double a = getLength(p1, p2);
        double b = getLength(p2, p0);
        double c = getLength(p0, p1);
        double s = 0.5*(a+b+c);
        double prod = s*(s-a)*(s-b)*(s-c);
        return sqrt(fabs(prod));
    };

    double surfArea = 0.0;
    for( size_t i = 0; i < numFaces; i++) {
        auto p0 = coords[triangles[i][0]];
        auto p1 = coords[triangles[i][1]];
        auto p2 = coords[triangles[i][2]];
        surfArea += getArea(p0, p1, p2);
    }

    SurfaceMesh mesh;
    vector<pmp::Vertex> nodes( numNodes );

    for( size_t i = 0; i < numNodes; i++)
        nodes[i] = mesh.add_vertex(Point( coords[i][0], coords[i][1], coords[i][2]));

    for( size_t i = 0; i < numFaces; i++) {
        auto v0 = nodes[triangles[i][0]];
        auto v1 = nodes[triangles[i][1]];
        auto v2 = nodes[triangles[i][2]];
        mesh.add_triangle( v0, v1, v2);
    }

    SurfaceFeatures sf(mesh);
    sf.clear();
    sf.detect_angle(creaseAngle);

    SurfaceSmoothing(mesh).initialize();
    SurfaceSmoothing(mesh).explicit_smoothing(2);

    std::vector<double> elen(mesh.n_edges());
    size_t eIndex = 0;
    for (auto eit : mesh.edges()) {
        elen[eIndex++] = distance(mesh.position(mesh.vertex(eit, 0)),
                                  mesh.position(mesh.vertex(eit, 1)));
    }
    std::sort( elen.begin(), elen.end());

    // figure out desired edge-length based on number of triangles desired

    double edgelength;
    if( nTrianglesDesired < 0.0) {
        nTrianglesDesired = 10*numFaces;
        nTrianglesDesired = (nTrianglesDesired > 10000)?10000:nTrianglesDesired;
        nTrianglesDesired = (nTrianglesDesired < numFaces)?numFaces:nTrianglesDesired;
    }
    edgelength = sqrt(4.0*surfArea/nTrianglesDesired/sqrt(3.0));

    cout << "Input #Triangles       : " << numFaces << endl;
    cout << "Input Area    : " << surfArea << endl;
    cout << "Input Min EdgeLength   : " << elen.front() << endl;
    cout << "Input Max EdgeLength   : " << elen.back()  << endl;
    cout << "Target EdgeLength      : " << edgelength  << endl;
    cout << "Expected #Triangles    : " << nTrianglesDesired << endl;

    SurfaceRemeshing(mesh).uniform_remeshing(edgelength);
    auto points = mesh.get_vertex_property<Point>("v:point");
    coords.clear();
    coords.reserve( mesh.n_vertices());
    for (auto vit : mesh.vertices())
    {
        auto p = points[vit];
        coords.push_back( {p[0], p[1], p[2]} );
    }

    triangles.clear();
    triangles.resize( mesh.n_faces() );
    size_t tIndex = 0;
    for (auto fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        int nIndex = 0;
        SurfaceMesh::VertexAroundFaceCirculator fvit = mesh.vertices(*fit), fvend = fvit;
        do
        {
            triangles[tIndex][nIndex] = (*fvit).idx();
            nIndex++;
        } while (++fvit != fvend);
        tIndex++;
    }

    return true;
#else
    writeMessageToParetoLog("CoreFunctions: PMP not defined");
    return false;
#endif
}
/************************************************************/
bool CoreFunctions::projectIsosurfaceMeshOntoCAD(double relativeProjectionDistance, double relaticeRemeshDensity)
{
    /* Algorithm
     * 1. Project all isosurface nodes onto the CAD surface (nearest triangle)
     * 2. Remesh the resulting mesh using pmp
     */

    /* To be done
     * 1. Project nodes onto edges but ensure no bad triangle
     * 2. Straighten out the edges
     * 3. While projecting check against local normal
     * 4. Ensure symmetry
     */
#ifdef PMP
    int nIsosurfaceNodes,nIsosurfaceTri,currentCounter,nodes[3],closestTri;
    double meshPt[3],meshSize;
    FILE *fptr;
    vector<Array3D> IsosurfacePts;
    vector<Array3I> Isosurfacetriangles;

    currentCounter = topopt.getCurrentIsoCounter();
    nIsosurfaceNodes = topopt.getNumIsoNodes(currentCounter);
    nIsosurfaceTri = topopt.getNumIsoTriangles(currentCounter);
    meshSize = topopt.getMeshElementSize();

    string projectedMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_projectedMesh.off");
    fptr = fopen(projectedMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",nIsosurfaceNodes,nIsosurfaceTri);

    resetProgressBar("Projecting surface");

    // Create the KD tree of the original CAD mesh for fast nearest calculation
    SurfaceMesh CADMesh;

    vector<pmp::Vertex> CADPts( vox.getNumTriPoints() );

    liveVar.displayGeom = DISPLAY_GEOM_NONE;// this avoids repeated display update
    liveVar.displayField = DISPLAY_FIELD_NONE;

    for( size_t i = 0; i < vox.getNumTriPoints() ; i++){
        double pt[3];
        vox.getTriNode(i,pt);
        CADPts[i] = CADMesh.add_vertex(Point( pt[0], pt[1], pt[2]));
    }

    for( size_t i = 0; i < vox.getNumTriangles(); i++) {
        int triNodes[3];
        vox.getTriNodes(i,triNodes);
        auto v0 = CADPts[triNodes[0]];
        auto v1 = CADPts[triNodes[1]];
        auto v2 = CADPts[triNodes[2]];
        CADMesh.add_triangle( v0, v1, v2);
    }

    TriangleKdTree kdTree(CADMesh);// for fast projection
    Point pt;
    TriangleKdTree::NearestNeighbor nn;
    for (int node = 0; node < nIsosurfaceNodes; node++){
        topopt.getIsoNodeXYZ(currentCounter,node,meshPt);
        for (int i = 0; i < 3; i++)
            pt[i] = meshPt[i];
        nn = kdTree.nearest(pt);
        if (nn.dist < relativeProjectionDistance*meshSize)
            for (int i = 0; i < 3; i++)
                meshPt[i] = nn.nearest[i];

        fprintf(fptr,"%g %g %g \n",meshPt[0],meshPt[1],meshPt[2]);
        Array3D pt3d;
        for (int i = 0; i < 3; i++)
            pt3d[i] = meshPt[i];
        IsosurfacePts.push_back(pt3d);
    }
    updateProgressBar("Projecting surface",100.0);
    for (int tri = 0; tri < nIsosurfaceTri; tri++){
        topopt.getIsoTriNodes(currentCounter,tri,nodes);
        fprintf(fptr,"3 %d %d %d \n",nodes[0],nodes[1],nodes[2]);
        Array3I nodes3d;
        for (int i = 0; i < 3; i++)
            nodes3d[i] = nodes[i];
        Isosurfacetriangles.push_back(nodes3d);

    }
    fclose(fptr);

    updateProgressBar("Projecting surface",100.0);
    resetProgressBar("Remeshing projected surface");

    uniformRemeshing(IsosurfacePts, Isosurfacetriangles,int (relaticeRemeshDensity*nIsosurfaceTri),30);// uses PMP library
    updateProgressBar("Remeshing projected surface",100.0);

    /*
    // Now project onto edges
    for (int node = 0; node < IsosurfacePts.size(); node++){
        double isoPt[3];
        for (int i = 0; i < 3; i++)
            isoPt[i] = IsosurfacePts[node][i];

        vox.snapPointToEdge(isoPt,0.5*relativeProjectionDistance*meshSize);
        for (int i = 0; i < 3; i++)
            IsosurfacePts[node][i] = isoPt[i];
    }*/


    string projectedIsosurfaceMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_projectedRemesh.off");
    fptr = fopen(projectedIsosurfaceMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",IsosurfacePts.size(),Isosurfacetriangles.size());
    for (int node = 0; node < IsosurfacePts.size(); node++)
        fprintf(fptr,"%g %g %g \n",IsosurfacePts[node][0],IsosurfacePts[node][1],IsosurfacePts[node][2]);

    for (int tri = 0; tri < Isosurfacetriangles.size(); tri++)
        fprintf(fptr,"3 %d %d %d \n",Isosurfacetriangles[tri][0],Isosurfacetriangles[tri][1],Isosurfacetriangles[tri][2]);

    fclose(fptr);
    resetProgressBar("Done");

    voxTemp.deleteCurrentModel();
    voxTemp.readOffFile(projectedIsosurfaceMeshFile.c_str(),0);

    liveVar.displayGeom = DISPLAY_GEOM_OTHER;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayEdges = true;
#endif
    return true;
}
/************************************************************/
bool CoreFunctions::mergeCADandIsosurfaceMesh(double CADControlParam, double IsosurfaceControlParam)
{
    // CADControlParam: higher the value, more CAD triangles get included and less isotriangles
    //IsosurfaceControlParam: higher the value

    // Algorithm
    // 1. Create CAD surface mesh (remeshing)
    // 2. Remove triangles from CAD mesh where material has been removed
    // 3. Remove triangles from isosurface where material has been retaiend
    // 4. Merge/knit the two meshes
    int nIsosurfaceNodes,nIsosurfaceTri;
    double meshPt[3],meshSize;
    int nCADnodes,nCADTriangles,currentCounter,triNodes[3];
    bool *isElemOnCADBoundary;


    cout << " mergeCADandIsosurfaceMesh " << endl;

    //**************************************
    //Allocate memory,etc
    //**************************************
    currentCounter =topopt.getCurrentIsoCounter();
    nIsosurfaceTri = topopt.getNumIsoTriangles(currentCounter);
    meshSize = topopt.getMeshElementSize();
    isElemOnCADBoundary = (bool *)malloc(topopt.getNumElems()*sizeof(bool));
    memset(isElemOnCADBoundary,false,topopt.getNumElems()*sizeof(bool));

    //**************************************
    // Step 1: Remesh CAD geometry
    //**************************************

    vector<Array3D> CADPts;
    vector<Array3I> CADtriangles;
    int nRemeshedTrianglesDesired;
    double creaseAngle;
    nRemeshedTrianglesDesired = nIsosurfaceTri;// match the isosurface mesh
    creaseAngle = 10;
    CADPts.clear();// these are the original CAD pts and triangles
    CADtriangles.clear();

    for (int n = 0; n < vox.getNumTriPoints();n++){
        double pt[3];
        Array3D pt3d;
        vox.getTriNode(n,pt);
        for (int i = 0; i < 3; i++)
            pt3d[i] = pt[i];
        CADPts.push_back(pt3d);
    }

    for (int tri = 0; tri < vox.getNumTriangles();tri++){
        int nodes[3];
        Array3I nodes3d;
        vox.getTriNodes(tri,nodes);
        for (int i = 0; i < 3; i++)
            nodes3d[i] = nodes[i];
        CADtriangles.push_back(nodes3d);
    }
    resetProgressBar("Meshing CAD surface");

    //create a fine mesh of CAD model
    uniformRemeshing(CADPts, CADtriangles,nRemeshedTrianglesDesired);// uses PMP library
    updateProgressBar("Merging CAD surface",100.0);

    resetProgressBar("Merging CAD and Isosurface Mesh");

    //save the CAD mesh .off format
    string remeshedCADMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_CadMesh.off");
    FILE *fptr = fopen(remeshedCADMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",CADPts.size(),CADtriangles.size());
    for (int node = 0; node < CADPts.size(); node++)
        fprintf(fptr,"%g %g %g \n",CADPts[node][0],CADPts[node][1],CADPts[node][2]);

    for (int tri = 0; tri < CADtriangles.size(); tri++)
        fprintf(fptr,"3 %d %d %d \n",CADtriangles[tri][0],CADtriangles[tri][1],CADtriangles[tri][2]);

    fclose(fptr);

    updateProgressBar("Merging CAD and Isosurface Mesh",25.0);

    nCADnodes = CADPts.size();
    nCADTriangles = CADtriangles.size();

    //**************************************
    // Step 2: Remove triangles from CAD mesh where material has been removed
    // and create trimmed CAD triangles
    //**************************************

    std::vector<Array3I> trimmedCADTriangles;
    Array3I cadTri;
    Array3D cadPt;
    double xCenter,yCenter,zCenter;
    int voxElem;

    trimmedCADTriangles.clear();

    double boundingBox[3][2];
    int elems[1000],nElemsInBox;
    for (int tri = 0; tri < nCADTriangles; tri++){
        // for every triangle, check if the center is closed to an active voxel element (means keep)
        xCenter = yCenter = zCenter = 0;
        cadTri = CADtriangles[tri];
        for (int i =0; i < 3; i++){
            cadPt = CADPts[cadTri[i]];
            xCenter += cadPt[0];
            yCenter += cadPt[1];
            zCenter += cadPt[2];
        }
        xCenter /= 3.0;
        yCenter /= 3.0;
        zCenter /= 3.0;

        //create a bounding box around the triangle center
        boundingBox[0][0] = xCenter - CADControlParam*meshSize;
        boundingBox[0][1] = xCenter + CADControlParam*meshSize;
        boundingBox[1][0] = yCenter - CADControlParam*meshSize;
        boundingBox[1][1] = yCenter + CADControlParam*meshSize;
        boundingBox[2][0] = zCenter - CADControlParam*meshSize;
        boundingBox[2][1] = zCenter + CADControlParam*meshSize;
        nElemsInBox = topopt.findElemsInBoundingBox(boundingBox,elems);
        for (int i = 0; i < nElemsInBox; i++){
            if (topopt.doesElemExistsAtTSCounter(elems[i],currentCounter)){
                trimmedCADTriangles.push_back(cadTri);
                //mark this voxel as near CAD boundary ... needed for later
                isElemOnCADBoundary[elems[i]] = true;// need for later
            }
        }
    }

    //**************************************
    // Step 3: Trim and map CAD nodes.
    // Since many nodes are discarded. We have to renumber the nodes, etc
    //**************************************
    std::vector<Array3D> trimmedCADPts;
    std::vector<Array3I> trimmedMappedCADTriangles;

    trimmedCADPts.clear();
    trimmedMappedCADTriangles.clear();
    bool *nodeUsed;
    int *nodeMapping;
    nodeUsed = (bool *)malloc(nCADnodes*sizeof(bool));
    nodeMapping = (int *)malloc(nCADnodes*sizeof(int));
    memset(nodeUsed,0,nCADnodes*sizeof(bool));// none used by default
    memset(nodeMapping,0,nCADnodes*sizeof(int));
    for (int tri = 0; tri < trimmedCADTriangles.size(); tri++){
        for (int i =0; i < 3; i++)
            nodeUsed[trimmedCADTriangles[tri][i]] = true;
    }
    int nUsedCADNodes;
    nUsedCADNodes = 0;
    for (int node = 0; node < nCADnodes; node++){
        nodeMapping[node] = -1;
        if (nodeUsed[node]){
            nodeMapping[node] = nUsedCADNodes;
            nUsedCADNodes++;
        }
    }

    // Save the trimmed CAD file
    string trimmedCADMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_trimmedCadMesh.off");
    fptr = fopen(trimmedCADMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",nUsedCADNodes,trimmedCADTriangles.size());
    for (int node = 0; node < nCADnodes; node++){
        if (nodeUsed[node]){
            trimmedCADPts.push_back(CADPts[node]);
            fprintf(fptr,"%g %g %g \n",CADPts[node][0],CADPts[node][1],CADPts[node][2]);
        }
    }

    for (int tri = 0; tri < trimmedCADTriangles.size(); tri++){
        Array3I mappedNodes;
        for (int i =0; i < 3; i++)
            mappedNodes[i] = nodeMapping[trimmedCADTriangles[tri][i]];

        trimmedMappedCADTriangles.push_back(mappedNodes);
        fprintf(fptr,"3 %d %d %d \n",mappedNodes[0],mappedNodes[1],mappedNodes[2]);
    }
    fclose(fptr);

    free(nodeUsed);
    free(nodeMapping);

    updateProgressBar("Merging CAD and Isosurface Mesh",50.0);

    //**************************************
    // Step 3: Trim Isosurface mesh against current topology
    //**************************************
    std::vector<Array3D> trimmedIsoSurfacePts;
    std::vector<Array3I> trimmedIsoSurfaceTriangles;
    Array3I isosurfaceTri;
    Array3D isosurfacePt;

    bool *triangleUsed;

    nIsosurfaceNodes = topopt.getNumIsoNodes(currentCounter);
    nIsosurfaceTri = topopt.getNumIsoTriangles(currentCounter);
    nodeUsed = (bool *)malloc(nIsosurfaceNodes*sizeof(bool));
    nodeMapping = (int *)malloc(nIsosurfaceNodes*sizeof(int));
    triangleUsed = (bool *)malloc(nIsosurfaceTri*sizeof(bool));
    memset(nodeUsed,false,nIsosurfaceNodes*sizeof(bool));
    memset(nodeMapping,0,nIsosurfaceNodes*sizeof(int));
    memset(triangleUsed,false,nIsosurfaceTri*sizeof(bool));

    string isosurfaceMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_isosurfaceMesh.off");
    fptr = fopen(isosurfaceMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",nIsosurfaceNodes,nIsosurfaceTri);
    for (int node = 0; node < nIsosurfaceNodes; node++){
        topopt.getIsoNodeXYZ(currentCounter,node,meshPt);
        fprintf(fptr,"%g %g %g \n",meshPt[0],meshPt[1],meshPt[2]);
    }

    for (int tri = 0; tri < nIsosurfaceTri; tri++){
        topopt.getIsoTriNodes(currentCounter,tri,triNodes);
        fprintf(fptr,"3 %d %d %d \n",triNodes[0],triNodes[1],triNodes[2]);
    }

    fclose(fptr);
    for (int tri = 0; tri < nIsosurfaceTri; tri++){// isosurface triangles
        topopt.getIsoTriNodes(currentCounter,tri,triNodes);
        // for every triangle, check if the center is closed to an active voxel element
        xCenter = yCenter = zCenter = 0;
        for (int i =0; i < 3; i++){
            topopt.getIsoNodeXYZ(currentCounter,triNodes[i],meshPt);
            xCenter += meshPt[0];
            yCenter += meshPt[1];
            zCenter += meshPt[2];
        }
        xCenter /= 3.0;
        yCenter /= 3.0;
        zCenter /= 3.0;

        voxElem = topopt.findNonVoidElemClosestTo(xCenter,yCenter,zCenter,IsosurfaceControlParam);
        if (voxElem < 0) continue;// no active voxel, so triangle not important
        if (isElemOnCADBoundary[voxElem]) continue;//opposite case of CAD mesh

        triangleUsed[tri] = true;
        for (int i = 0; i < 3; i++)
            nodeUsed[triNodes[i]] = true;
    }
    int nUsedIsosurfaceNodes;
    nUsedIsosurfaceNodes= 0;
    for (int node = 0; node < nIsosurfaceNodes; node++){
         nodeMapping[node] = -1;
         if (nodeUsed[node]){
             nodeMapping[node] = nUsedIsosurfaceNodes;
             nUsedIsosurfaceNodes++;
         }
     }

    trimmedIsoSurfacePts.clear();
    trimmedIsoSurfaceTriangles.clear();
    for (int node = 0; node < nIsosurfaceNodes; node++){
        if (nodeUsed[node]){
            topopt.getIsoNodeXYZ(currentCounter,node,meshPt);
            for (int i = 0; i < 3; i++)
                isosurfacePt[i] = meshPt[i];
            trimmedIsoSurfacePts.push_back(isosurfacePt);
        }
    }

    for (int tri = 0; tri < nIsosurfaceTri; tri++){
        if (triangleUsed[tri]){
            topopt.getIsoTriNodes(currentCounter,tri,triNodes);
            for (int i = 0; i < 3; i++)
                isosurfaceTri[i] = nodeMapping[triNodes[i]];
            trimmedIsoSurfaceTriangles.push_back(isosurfaceTri);
        }
    }

    string trimmedIsosurfaceMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_trimmedIsosurfaceMesh.off");
    fptr = fopen(trimmedIsosurfaceMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",trimmedIsoSurfacePts.size(),trimmedIsoSurfaceTriangles.size());
    for (int node = 0; node < trimmedIsoSurfacePts.size(); node++)
        fprintf(fptr,"%g %g %g \n",trimmedIsoSurfacePts[node][0],trimmedIsoSurfacePts[node][1],trimmedIsoSurfacePts[node][2]);

    for (int tri = 0; tri < trimmedIsoSurfaceTriangles.size(); tri++)
        fprintf(fptr,"3 %d %d %d \n",trimmedIsoSurfaceTriangles[tri][0],trimmedIsoSurfaceTriangles[tri][1],trimmedIsoSurfaceTriangles[tri][2]);

    fclose(fptr);

    updateProgressBar("Merging CAD and Isosurface Mesh",75.0);
    free(isElemOnCADBoundary);

    free(nodeMapping);
    free(nodeUsed);
    free(triangleUsed);

    //**************************************
    // Step 4: Eliminate duplicate pts and union the two meshes
    //**************************************
    bool eliminateDuplicatePts;
    eliminateDuplicatePts = false;

    std::vector<Array3D> combinedPts;
    std::vector<Array3I> combinedTriangles;
    int nTotalPts,nUniquePts;

    bool duplicate;
    double d;

    nTotalPts =  trimmedCADPts.size() + trimmedIsoSurfacePts.size();
    nodeMapping = (int *)malloc(nTotalPts*sizeof(int));

    combinedPts.clear();
    nUniquePts = 0;
    if (!eliminateDuplicatePts){
        for (int node = 0; node < trimmedCADPts.size(); node++){
            nodeMapping[node] = node;
            nUniquePts++;
            combinedPts.push_back(trimmedCADPts[node]);
        }

        for (int node = 0; node < trimmedIsoSurfacePts.size(); node++){
            nodeMapping[node+trimmedCADPts.size()] = nUniquePts;
            nUniquePts++;
            combinedPts.push_back(trimmedIsoSurfacePts[node]);
        }

    }
    else {
        for (int node = 0; node < trimmedCADPts.size(); node++){
            nodeMapping[node] = -1;
            duplicate = false;
            for (int j = 0; j < combinedPts.size(); j++){
                d = sqrt(pow(trimmedCADPts[node][0] -combinedPts[j][0],2)+
                        pow(trimmedCADPts[node][1] -combinedPts[j][1],2)+
                        pow(trimmedCADPts[node][2] -combinedPts[j][2],2));
                if (d < meshSize*0.0){
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate){
                nodeMapping[node] = nUniquePts;
                nUniquePts++;
                combinedPts.push_back(trimmedCADPts[node]);
            }
        }

        for (int node = 0; node < trimmedIsoSurfacePts.size(); node++){
            nodeMapping[node+trimmedCADPts.size()] = -1;
            duplicate = false;
            for (int j = 0; j < combinedPts.size(); j++){
                d = sqrt(pow(trimmedIsoSurfacePts[node][0] -combinedPts[j][0],2)+
                        pow(trimmedIsoSurfacePts[node][1] -combinedPts[j][1],2)+
                        pow(trimmedIsoSurfacePts[node][2] -combinedPts[j][2],2));
                if (d < meshSize*0.0){
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate){
                nodeMapping[node+trimmedCADPts.size()] = nUniquePts;
                nUniquePts++;
                combinedPts.push_back(trimmedIsoSurfacePts[node]);
            }
        }
    }
    cout << "nTotalPts " << nTotalPts << " nUniquePts " << nUniquePts << endl;
    combinedTriangles.clear();
    Array3I combinedTri;
    for (int tri = 0; tri < trimmedMappedCADTriangles.size(); tri++){
        cadTri = trimmedMappedCADTriangles[tri];
        for (int i = 0; i < 3; i++)
            combinedTri[i] = nodeMapping[cadTri[i]];
        combinedTriangles.push_back(combinedTri);
    }
     for (int tri = 0; tri < trimmedIsoSurfaceTriangles.size(); tri++){
         isosurfaceTri = trimmedIsoSurfaceTriangles[tri];
         for (int i = 0; i < 3; i++)
             combinedTri[i] = nodeMapping[trimmedCADPts.size() + isosurfaceTri[i]];
         combinedTriangles.push_back(combinedTri);
     }

    //**************************************
    // Step 4: Save the two trimmed meshes in a single file
    //**************************************
    updateProgressBar("Merging CAD and Isosurface Mesh",100.0);

    // Save the trimmed CAD file
    string combinedMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_combinedMesh.off");
    fptr = fopen(combinedMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",combinedPts.size(),combinedTriangles.size());
    for (int node = 0; node < combinedPts.size(); node++)
        fprintf(fptr,"%g %g %g \n",combinedPts[node][0],combinedPts[node][1],combinedPts[node][2]);

    for (int tri = 0; tri < combinedTriangles.size(); tri++)
        fprintf(fptr,"3 %d %d %d \n",combinedTriangles[tri][0],combinedTriangles[tri][1],combinedTriangles[tri][2]);

    fclose(fptr);

    resetProgressBar("Merging Done");
    return true;

    //**************************************
    // Step 5: Use new code to fill the gaps
    //**************************************

    updateProgressBar("Adding triangles",0.0);


    // Save the blended file
    combinedMeshFile = liveVar.projectDirectory + liveVar.modelName + string("_blendedMesh.off");
    fptr = fopen(combinedMeshFile.c_str(),"w");
    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d %d 0 \n",combinedPts.size(),combinedTriangles.size());
    for (int node = 0; node < combinedPts.size(); node++)
        fprintf(fptr,"%g %g %g \n",combinedPts[node][0],combinedPts[node][1],combinedPts[node][2]);

    for (int tri = 0; tri < combinedTriangles.size(); tri++)
        fprintf(fptr,"3 %d %d %d \n",combinedTriangles[tri][0],combinedTriangles[tri][1],combinedTriangles[tri][2]);

    fclose(fptr);

    resetProgressBar("Blending done");

    voxTemp.deleteCurrentModel();
    voxTemp.readOffFile(combinedMeshFile.c_str(),0);

    liveVar.displayGeom = DISPLAY_GEOM_OTHER;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayEdges = true;

    return true;
}
/************************************************************/
bool CoreFunctions::simplifyNegativeSubtractAndSaveResult(void)
{
#ifdef _WIN32
    indexed_mesh mesh;
    int nSmootheningSteps,nTriangles;

    // Step 1:Create and load the negative component(s)
    string negativeTopologySTLFile  = liveVar.projectDirectory + liveVar.modelName + string("_1_negativeSTL.stl");
    string negativeTopologyOFFFile  = liveVar.projectDirectory + liveVar.modelName + string("_2_negativeOff.off");
    topopt.createAndDumpNegativeIsoSurfaceFromMesh(negativeTopologySTLFile.c_str());
    mesh.load_from_binary_stereo_lithography_file(negativeTopologySTLFile.c_str());
    nSmootheningSteps = 50;
    mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);
    mesh.save_to_extended_off_file(negativeTopologyOFFFile.c_str());
    nTriangles = mesh.getNumberOfTriangles();
    writeMessageToGUIWindow("Original triangles = %d ",nTriangles);

    // Step 2:Simplify the STL triangulation
    int simplificationLevel = 2,targetNumberOfTriangles;
    switch(simplificationLevel) {
        case 0: targetNumberOfTriangles = nTriangles*0.1;break;
        case 1: targetNumberOfTriangles = nTriangles*0.2;break;
        case 2: targetNumberOfTriangles = nTriangles*0.3;break;
        case 3: targetNumberOfTriangles = nTriangles*0.6;break;
        case 4: targetNumberOfTriangles = nTriangles*0.9;break;
    }
    writeMessageToGUIWindow("Target triangles = %d ",targetNumberOfTriangles);
    string negativeTopologySimplifiedSTLFile  = liveVar.projectDirectory + liveVar.modelName + string("_3_negativeSimplifiedSTL.stl");
    Simplify::load_off(negativeTopologyOFFFile.c_str());
    Simplify::simplify_mesh(targetNumberOfTriangles);
    Simplify::write_stl_binary(negativeTopologySimplifiedSTLFile.c_str());

    // Step 3:Smoothened the simplified negative triangulation
    mesh.load_from_binary_stereo_lithography_file(negativeTopologySimplifiedSTLFile.c_str());
    nSmootheningSteps = 50;
    mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);

    //Step 5: Expand the smoothened surface
    double offsetOutward;
    offsetOutward = 0.5*topopt.getMeshElementSize();
    mesh.expand_surface(offsetOutward);
    mesh.taubin_smooth(0.6307, -0.6734,nSmootheningSteps);

    // Step 6: Save the simplified and smoothened
    string negativeTopologySmoothenedOFFFile  = liveVar.projectDirectory + liveVar.modelName + string("_4_negativeSmoothOff.off");
    mesh.save_to_extended_off_file(negativeTopologySmoothenedOFFFile.c_str());

    //Step 7: Save the initial design as Off
    string initialDesignOFFFile  = liveVar.projectDirectory + liveVar.modelName + string("_initialOff.off");
    vox.saveAsOff(initialDesignOFFFile.c_str());

    // Step 8: Perform Boolean
    string finalTopologyOFFFile  = liveVar.projectDirectory + liveVar.modelName + string("_5_finalOff.off");
    if (!vox.Boolean(initialDesignOFFFile.c_str(),negativeTopologySmoothenedOFFFile.c_str(),ivbool3d::M_SUB_AB,finalTopologyOFFFile.c_str())){
        writeMessageToParetoLog("Could not perform Boolean ");
        return false;
    }
#endif
    return true;
}
bool CoreFunctions::createTetmesh(void)
{
    /*
    tetgenio in, out;
    double area,stlVolume,volumeOfEachTet,tetEdgeLength;
    int nElementsDesired;

    stlVolume= vox.getVolumeOfSTLGeom(&area);
    nElementsDesired = 10000;
    volumeOfEachTet = stlVolume/nElementsDesired;
    tetEdgeLength = pow(6*sqrt(2)*volumeOfEachTet,1.0/3.0);
    cout << "tetEdgeLength " << tetEdgeLength << endl;

    updateProgressBar("Generating tets",0.0);

    // All indices start from 0
    in.firstnumber = 0;
    in.numberofpoints =  vox.getNumTriPoints();

    in.pointlist = new REAL[in.numberofpoints * 3];
    double pt[3];
    for (int i = 0; i < vox.getNumTriPoints(); i++){
        vox.getTriNode(i,pt);
        in.pointlist[3*i] = pt[0];
        in.pointlist[3*i+1] = pt[1];
        in.pointlist[3*i+2] = pt[2];
    }
    in.numberoffacets = vox.getNumTriangles();
    in.facetlist = new tetgenio::facet[in.numberoffacets];
    int nodes[3];
    for (int tri = 0; tri < vox.getNumTriangles(); tri++){
        in.facetlist[tri].numberofholes = 0;
        in.facetlist[tri].numberofpolygons = 1;
        in.facetlist[tri].holelist = new REAL[1];
        in.facetlist[tri].polygonlist = new tetgenio::polygon[1];
        in.facetlist[tri].polygonlist[0].numberofvertices = 3;
        in.facetlist[tri].polygonlist[0].vertexlist = new int[3];

        voxTemp.getTriNodes(tri,nodes);
        in.facetlist[tri].polygonlist[0].vertexlist[0] = nodes[0];
        in.facetlist[tri].polygonlist[0].vertexlist[1] = nodes[1];
        in.facetlist[tri].polygonlist[0].vertexlist[2] = nodes[2];
    }


    tetgenbehavior tetgenParams;// all default
    tetrahedralize(&tetgenParams, &in, &out);
    updateProgressBar("Generating tets",100.0);
    string tetGenFiles = liveVar.projectDirectory + liveVar.modelName + string("_tetGen");
    char tetGenFile[256];
    strcpy(tetGenFile,tetGenFiles.c_str());
    out.save_nodes(tetGenFile);
    out.save_faces(tetGenFile);
    out.save_elements(tetGenFile);

    resetProgressBar("Tetmesh complete");

    */
    return true;
}
/************************************************************/
/*
bool CoreFunctions::createTetmesh(void)
{
    tetgenio in, out;
    double area,stlVolume,volumeOfEachTet,tetEdgeLength;
    int nElementsDesired;
    string tessellationFile = liveVar.projectDirectory + liveVar.modelName + string("Tessellation.stl");

    stlVolume= vox.getVolumeOfSTLGeom(&area);
    nElementsDesired = 10000;
    volumeOfEachTet = stlVolume/nElementsDesired;
    tetEdgeLength = pow(6*sqrt(2)*volumeOfEachTet,1.0/3.0);
    cout << "tetEdgeLength " << tetEdgeLength << endl;
    vox.dumpSTLTesselation(tessellationFile.c_str(),5,tetEdgeLength);

    voxTemp.readSTLGeom(tessellationFile.c_str());
    updateProgressBar("Generating tets",0.0);

    // All indices start from 0
    in.firstnumber = 0;
    in.numberofpoints =  voxTemp.getNumTriPoints();
    cout << "#pts = " <<  voxTemp.getNumTriPoints() << endl;
    in.pointlist = new REAL[in.numberofpoints * 3];
    double pt[3];
    for (int i = 0; i < voxTemp.getNumTriPoints(); i++){
        voxTemp.getTriNode(i,pt);
        in.pointlist[3*i] = pt[0];
        in.pointlist[3*i+1] = pt[1];
        in.pointlist[3*i+2] = pt[2];
    }


    in.numberoftrifaces = voxTemp.getNumTriangles();
    int nodes[3];
    in.trifacelist = new int[in.numberoftrifaces * 3];
    for (int i = 0; i < voxTemp.getNumTriangles(); i++){
        voxTemp.getTriNodes(i,nodes);
        in.trifacelist[3*i] = nodes[0];
        in.trifacelist[3*i+1] = nodes[1];
        in.trifacelist[3*i+2] = nodes[2];
    }


    tetgenbehavior tetgenParams;// all default


    tetrahedralize(&tetgenParams, &in, &out);
    updateProgressBar("Generating tets",100.0);

    string tetGenFile = liveVar.projectDirectory + liveVar.modelName + string(".tetmesh");

    FILE *fptr = fopen(tetGenFile.c_str(),"w");
    fprintf(fptr,"%d \n",out.numberofpoints);
    for (int i  =0; i < out.numberofpoints; i++){
        fprintf(fptr,"%lf %lf %lf \n",
                out.pointlist[3*i], out.pointlist[3*i+1],out.pointlist[3*i+2]);
    }


    fprintf(fptr,"%d \n",out.numberoftetrahedra);
    for (int i  =0; i < out.numberoftetrahedra; i++){
        fprintf(fptr,"%d %d %d %d \n",
                out.tetrahedronlist[4*i], out.tetrahedronlist[4*i+1],
                out.tetrahedronlist[4*i+2],out.tetrahedronlist[4*i+3] );
    }

    fclose(fptr);

    string tetGenFiles = liveVar.projectDirectory + liveVar.modelName + string("_tetGen");

    char tetGenFile[256];


    strcpy(tetGenFile,tetGenFiles.c_str());
    out.save_nodes(tetGenFile);
    out.save_faces(tetGenFile);
    out.save_elements(tetGenFile);

    resetProgressBar("Tetmesh complete");
    return true;
}
*/
/************************************************************/
bool CoreFunctions::autoDesign(string finalDesignOffFile)
{
#ifdef _WIN32
    double meshSize;
    string initialOffFile,componentDesignOffFile;

    initialOffFile = liveVar.projectDirectory + liveVar.modelName + string("_initial.off");
    setTopOptConstraints();// just in case this has not been initialized
    vox.saveAsOff(initialOffFile.c_str());// save the initial design
    vox.saveAsOff(finalDesignOffFile.c_str());// save the initial design

    meshSize = topopt.getMeshElementSize();

    if (!liveVar.topoptConstraints.isExtrudeActive){
        writeWarningToGUIWindow("This feature requires extrude topopt constraint");
        return false;
    }

    topopt.generateContourOfMeshComponents(liveVar.autoDesignTopologyError,
                                           liveVar.autoDesignSurfaceError,
                                           liveVar.autoDesignSmootheningIterations,
                                           liveVar.autoDesignContourOffset );

    resetProgressBar("Generating autodesign");
    for (int component = 0; component < topopt.getNumberOfMeshComponents(); component++){//
        updateProgressBar("Autodesign in progress ",100.0*component/topopt.getNumberOfMeshComponents());
        componentDesignOffFile = liveVar.projectDirectory + string("component_") + to_string(component) + string(".off");
        if (!topopt.generateSolidForComponent(component,componentDesignOffFile))
            continue;

        if (!vox.Boolean(finalDesignOffFile.c_str(),componentDesignOffFile.c_str(),ivbool3d::M_SUB_AB,finalDesignOffFile.c_str())){
            writeMessageToGUIWindow( "AutoDesign: Could not perform subtract operation ");
            continue;
        }
        remove(componentDesignOffFile.c_str());
    }
    resetProgressBar("Autodesign complete");
    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = finalDesignOffFile;
    bool successfulLoad = loadTemporaryGeom(liveVar.assemblyMode);


    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_FINAL;
        liveVar.plotElasticityBC = false;
        liveVar.GEOMETRY_UPLOADED = true;
        liveVar.plotAxis = false;
        liveVar.plotTopOptConstraints = false;

        string autodesignSTLFile = liveVar.projectDirectory + liveVar.modelName + string("_autodesign.stl");
        voxTemp.saveAsSTL(autodesignSTLFile.c_str());
        writeMessageToGUIWindow("Autodesign file saved to %s",autodesignSTLFile.c_str());
    }
    if (voxTemp.getNumSharpEdges() == 0){
        writeWarningToGUIWindow("Possible autoDesign failure; try different parameters");
    }
#endif
    return true;
}
/************************************************************/
void CoreFunctions::saveMesh(void)
// Save the current mesh
{
    topopt.dumpParetoMesh(liveVar.meshFileName.c_str());
}

/************************************************************/
void CoreFunctions::savePRJ(bool withResults)
// Save the current project
{
    char materialName[256];
    int bcSet,tri,group,counter;
    double materialProperties[7],materialPrice = 1;

    writeMessageToGUIWindow("Saving project file %s ",liveVar.prjFileName.c_str());
    FILE *fptr = fopen(liveVar.prjFileName.c_str(),"w");

    if (fptr == NULL){
        writeWarningToGUIWindow("Saving project unsuccessful");
        writeMessageToGUIWindow("---------------------------------------------");
        return;
    }
    fprintf(fptr,"#KEY VERSION\n");
    fprintf(fptr,"%.2f\n\n",PARETO_VERSION);

    // Units should come before reading geometry

    fprintf(fptr,"#KEY UNITS\n");
    fprintf(fptr,"%d \n\n",liveVar.paretoUnits);


    fprintf(fptr,"#KEY TEMPERATURE_UNITS\n");
    fprintf(fptr,"%d \n\n",liveVar.temperatureUnits);


    fprintf(fptr,"#KEY ASSEMBLY\n");
    fprintf(fptr,"%d\n\n",liveVar.assemblyMode);// if we want to load more than one stl file

    for (int stlFile =0; stlFile < liveVar.nSTLFilesLoaded; stlFile++) {
        fprintf(fptr,"#KEY GEOMETRY\n");
        fprintf(fptr,"%s.stl\n\n",liveVar.modelName.c_str());
    }
    // we may have one stl file with multiple parts
    // each part may have  a different material
    for (int part =0; part < topopt.getNumberOfParts(); part++) {
        // print the corresponding material
        fprintf(fptr,"#KEY PART\n");
        fprintf(fptr,"%d\n\n",part);

        topopt.getMaterialPropertiesOfPart(part,materialProperties,materialName);
        materialPrice = topopt.getMaterialPriceOfPart(part);

        if (liveVar.paretoUnits == UNITS_MM){// we undo the changes in applyMaterial
            // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
            // but this lead to inconsistency in units; we fix this here!
            materialProperties[3] *= 1000;
        }

        if (liveVar.paretoUnits == UNITS_INCH){//undo
            // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
            // but this lead to inconsistency in units; we fix this here!
            materialProperties[3] *= 386.22;
        }
        fprintf(fptr,"#KEY MATERIALNAME\n");
        fprintf(fptr,"%s\n\n",materialName);

        fprintf(fptr,"#KEY E\n");
        fprintf(fptr,"%e \n\n",materialProperties[0]);

        fprintf(fptr,"#KEY nu\n");
        fprintf(fptr,"%0.3f \n\n",materialProperties[1]);

        fprintf(fptr,"#KEY Y\n");
        fprintf(fptr,"%e\n\n",materialProperties[2]);

        fprintf(fptr,"#KEY rho\n");
        fprintf(fptr,"%e\n\n",materialProperties[3]);

        fprintf(fptr,"#KEY K\n");
        fprintf(fptr,"%e\n\n",materialProperties[4]);

        fprintf(fptr,"#KEY alpha\n");
        fprintf(fptr,"%e\n\n",materialProperties[5]);

        fprintf(fptr,"#KEY Cp\n");
        fprintf(fptr,"%e\n\n",materialProperties[6]);

        fprintf(fptr,"#KEY price\n");
        fprintf(fptr,"%e\n\n",materialPrice);

        fprintf(fptr,"#KEY MATERIAL_END\n");
        fprintf(fptr,"%d \n\n",part);

        fprintf(fptr,"#KEY NONDESIGN\n");
        fprintf(fptr,"%d \n\n",(topopt.shouldPartBeOptimized(part)? 0:1));

    }
    fprintf(fptr,"#KEY TRIGROUP\n");
    counter =0;
    for ( tri = 0; tri < vox.getNumTriangles();tri++) {
        group = vox.getTriGroup(tri);
        if (group)  {
            fprintf(fptr,"%d %d ",tri, group);
            counter++;
            if (counter%10 == 0) fprintf(fptr,"\n");// a new line
        }
    }
    fprintf(fptr,"\n-1 0 \n \n");// done
    int activeGroups[STL_MAX_GROUPS];

    vox.getActiveGroups(activeGroups);
    if ((liveVar.BC_STRUCTURAL_APPLIED) || (liveVar.BC_THERMAL_APPLIED)) {
        for (bcSet = 0; bcSet < liveVar.nBCSet; bcSet++) {
            fprintf(fptr,"#KEY %BCSET \n");
            fprintf(fptr,"%d \n",bcSet);
            for (int j = 0; j < vox.getNumTriGroups(); j++) {
                if (!activeGroups[j]) continue;
                fprintf(fptr,"#KEY BC\n");
                fprintf(fptr,"%d %d %d %d %.3e %.3e %.3e %.3e %.3e %.3e\n",j,
                        liveVar.BCType[bcSet][j][0],liveVar.BCType[bcSet][j][1],liveVar.BCType[bcSet][j][2],
                        liveVar.BCValue[bcSet][j][0],liveVar.BCValue[bcSet][j][1],liveVar.BCValue[bcSet][j][2],
                        liveVar.BCValue[bcSet][j][3],liveVar.BCValue[bcSet][j][4],liveVar.BCValue[bcSet][j][5]);
            }
            fprintf(fptr,"\n");
            if (NORM3D(liveVar.ElasticityAcceleration[bcSet]) + abs(liveVar.ElasticityAcceleration[bcSet][3]) > 0) { // non-zero acceleration
                fprintf(fptr,"#KEY ELASTICITY_ACCELERATION\n");
                fprintf(fptr,"%.3e %.3e %.3e %.3e \n",liveVar.ElasticityAcceleration[bcSet][0],
                        liveVar.ElasticityAcceleration[bcSet][1],liveVar.ElasticityAcceleration[bcSet][2],
                        liveVar.ElasticityAcceleration[bcSet][3]);
                fprintf(fptr,"\n");
            }
            for (int j = 0; j < vox.getNumTriGroups(); j++) {
                if (!activeGroups[j]) continue;
                fprintf(fptr,"#KEY THERMALBC\n");
                fprintf(fptr,"%d %d %.3e %.3e %.3e\n",j,liveVar.ThermalBCType[bcSet][j],
                        liveVar.ThermalBCArea[bcSet][j],liveVar.ThermalBCValue[bcSet][j],liveVar.ThermalBCRef[bcSet][j]);
            }
            fprintf(fptr,"\n");
        }
    }

    // For thermo-elasticity
    fprintf(fptr,"#KEY ZERO_STRAIN_TEMPERATURE\n");
    fprintf(fptr,"%e \n\n",liveVar.zeroStrainTemperature);


    if (liveVar.includeThermalEffects) {
        fprintf(fptr,"#KEY THERMAL_EFFECTS\n");
        fprintf(fptr,"0 \n\n");
    }

    fprintf(fptr,"#KEY MESH\n");
    fprintf(fptr,"%d \n\n",liveVar.nElemsToMesh);


    fprintf(fptr,"#KEY MASS_DAMPING\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.alphaM);

    fprintf(fptr,"#KEY STIFFNESS_DAMPING\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.betaK);

    fprintf(fptr,"#KEY NEWMARK_GAMMA\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.NewmarkGamma);

    fprintf(fptr,"#KEY NEWMARK_BETA\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.NewmarkBeta);

    fprintf(fptr,"#KEY TRANSIENT_TIME\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.totalTime);

    fprintf(fptr,"#KEY TRANSIENT_STEP\n");
    fprintf(fptr,"%e \n\n",liveVar.structuralDynamics.timeStep);

    fprintf(fptr,"#KEY TRACK_POINT\n");
    fprintf(fptr,"%e %e %e  %d \n\n",
            liveVar.structuralDynamics.trackPoint[0],
            liveVar.structuralDynamics.trackPoint[1],
            liveVar.structuralDynamics.trackPoint[2],
            liveVar.structuralDynamics.trackDOF);

    fprintf(fptr,"#KEY NONLINEAR_STEPS\n");
    fprintf(fptr,"%d \n\n",liveVar.nonlinear.nSteps);

    fprintf(fptr,"#KEY NONLINEAR_ITERATIONS\n");
    fprintf(fptr,"%d \n\n",liveVar.nonlinear.nIterations);

    fprintf(fptr,"#KEY NONLINEAR_TOLERANCE\n");
    fprintf(fptr,"%e \n\n",liveVar.nonlinear.tolerance);

#ifdef COMPILE_GPUCODE
    if (liveVar.useGPU ==  1) {
        fprintf(fptr,"#KEY USEGPU\n");
        fprintf(fptr,"%d\n\n",liveVar.useGPU);
    }
#endif

    /// Manuf constraints
    if (liveVar.topoptConstraints.isDrawDirectionActive){
        fprintf(fptr,"#KEY DRAWDIR\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.drawDirection);
    }

    if (liveVar.topoptConstraints.isExtrudeActive){
        fprintf(fptr,"#KEY EXTRUDEDIR\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.extrudeDirection);
    }

    if (liveVar.topoptConstraints.isCyclicSymmetryActive){
        fprintf(fptr,"#KEY CYCLICSYM\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.cyclicSymmetry);
    }

    if (liveVar.topoptConstraints.isxGridActive){
        fprintf(fptr,"#KEY XGRIDPATTERN\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.xGridPattern);
    }

    if (liveVar.topoptConstraints.isyGridActive){
        fprintf(fptr,"#KEY YGRIDPATTERN\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.yGridPattern);
    }

    if (liveVar.topoptConstraints.iszGridActive){
        fprintf(fptr,"#KEY ZGRIDPATTERN\n");
        fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.zGridPattern);
    }

    if (liveVar.topoptConstraints.isMinFeatureSizeActive){
        fprintf(fptr,"#KEY MINFEATURESIZE\n");
        fprintf(fptr,"%e\n\n",liveVar.topoptConstraints.minFeatureSize);
    }

    if (liveVar.topoptConstraints.isMaxFeatureSizeActive){
        fprintf(fptr,"#KEY MAXFEATURESIZE\n");
        fprintf(fptr,"%e\n\n",liveVar.topoptConstraints.maxFeatureSize);
    }

    if (liveVar.topoptConstraints.isMaxDisplacementActive){
        fprintf(fptr,"#KEY MAXDISPLACEMENT\n");
        fprintf(fptr,"%.2e\n\n",liveVar.topoptConstraints.maxDisplacementAllowed);
    }

    if (liveVar.topoptConstraints.isStressSafetyActive){
        fprintf(fptr,"#KEY STRESSSAFETYFACTOR\n");
        fprintf(fptr,"%.3f\n\n",liveVar.topoptConstraints.stressSafetyFactor);
    }
    if (liveVar.topoptConstraints.isMinEigenActive){
        fprintf(fptr,"#KEY MINEIGENVALUE\n");
        fprintf(fptr,"%.2e\n\n",liveVar.topoptConstraints.minEigenValueAllowedHz);
    }
    if (liveVar.topoptConstraints.isTemperatureLimitActive){
        fprintf(fptr,"#KEY MAXTEMPERATURE\n");
        fprintf(fptr,"%.2e\n\n",liveVar.topoptConstraints.maxTemperatureAllowed);
    }

    bool anySymmetry;
    anySymmetry = liveVar.topoptConstraints.xSymmetry || liveVar.topoptConstraints.ySymmetry || liveVar.topoptConstraints.zSymmetry;

    if (anySymmetry){
        fprintf(fptr,"#KEY SYMMETRY\n");
        fprintf(fptr,"%d %d %d \n\n",liveVar.topoptConstraints.xSymmetry,liveVar.topoptConstraints.ySymmetry,liveVar.topoptConstraints.zSymmetry);
    }

    //fprintf(fptr,"#KEY CONNECTEDTOPOLOGY\n");
    //fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.keepTopologyConnected);

    fprintf(fptr,"#KEY KEEPDIR\n");
    fprintf(fptr,"%d\n\n",liveVar.topoptConstraints.keepDirichlet);

    //TopOpt
    fprintf(fptr,"#KEY OBJECTIVE\n");
    fprintf(fptr,"%d\n\n",liveVar.topoptObjective);

    fprintf(fptr,"#KEY VOLFRAC\n");
    fprintf(fptr,"%.3f\n\n",liveVar.desiredVolFraction);

    fprintf(fptr,"#KEY STEPSIZE\n");
    fprintf(fptr,"%.4f\n\n",liveVar.stepSize);

    if ((withResults) && (liveVar.TOPOPT_SOLVED)){
        string meshFileName;
        meshFileName = liveVar.modelName + string(".vxb");
        fprintf(fptr,"#KEY FINALMESH\n");
        fprintf(fptr,"%s\n\n",meshFileName.c_str());
        meshFileName = liveVar.projectDirectory + meshFileName;//in project dir location
        topopt.dumpMeshBinary(meshFileName.c_str());

        string isoSurfaceFileName;
        isoSurfaceFileName = liveVar.modelName + string("finalIso.off");
        fprintf(fptr,"#KEY FINALISO\n");
        fprintf(fptr,"%s\n\n",isoSurfaceFileName.c_str());
        isoSurfaceFileName = liveVar.projectDirectory + isoSurfaceFileName;//in project dir location
        topopt.dumpExtendedOffFile(isoSurfaceFileName.c_str());
    }


    fclose(fptr);
    setStatusMessage("Project Saved");
}
/************************************************************/
void CoreFunctions::createVoxelMesh(const char *voxFile)
{

    bool success;
    char str[256];

    if (vox.getNumTriangles() > 100000){
        bool response;
        sprintf(str,"Model has %d triangles; meshing will be slow",vox.getNumTriangles() );
        writeMessageToGUIWindow(str);
        response = getYesNoResponse("Model has %d triangles; meshing will be slow! Continue?",vox.getNumTriangles() );
        if (!response)
            return;
    }

    setStatusMessage("Creating Mesh");
    if (vox.getMaxVoxels() < liveVar.nElemsToMesh){
        sprintf(str,"The number of elements is limited to %d ",vox.getMaxVoxels());
        strcat(str,"Contact support@sciartsoft.com for a full version");
        writeWarningToGUIWindow("%s ",str);
		return;
    }

    writeMessageToGUIWindow("The minimum number of elements recommended is %d  ",vox.getRecommendedNumberOfElements());
    if (liveVar.nElemsToMesh < vox.getRecommendedNumberOfElements()) {
        writeMessageToGUIWindow("The minimum number of elements recommended is %d  ",vox.getRecommendedNumberOfElements());
    }

    timer.start();
    writeMessageToGUIWindow("CoreFunctions: Meshing STL: N = %d ",liveVar.nElemsToMesh);

    int symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
                        2*liveVar.topoptConstraints.ySymmetry
                    + 4*liveVar.topoptConstraints.zSymmetry;

    success = vox.voxelize(liveVar.nElemsToMesh,liveVar.maxAspectRatio,symmetryIndex,meshingCallBackCoreFunctions);
    if (!success) {
        writeWarningToGUIWindow("Could not mesh\n");
        liveVar.MESH_APPLIED = false;
        return;
    }
    vox.dumpVoxelDataBinary(voxFile);
    sprintf(str,"Meshing completed; time = %.3f (secs)",timer.getElapsedTimeInSec());
    writeMessageToGUIWindow(str);
    writeMessageToGUIWindow("---------------------------------------------");
    liveVar.MESH_APPLIED = true;

    setStatusMessage("Mesh Created");

}
/************************************************************/
bool CoreFunctions::checkMeshingError(void)
{
    double STLVolume,STLArea,err,voxelVolume;
    char str[256];
    STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
    voxelVolume = vox.getVoxelVolume();

    err = 100*fabs(voxelVolume-STLVolume)/STLVolume;
    writeMessageToGUIWindow("Mesher: STL Volume = %0.2e ",STLVolume);
    writeMessageToGUIWindow("Mesher: Mesh Volume = %0.2e ",voxelVolume);
    writeMessageToGUIWindow("Mesher: PercentVolErr = %0.3f ",err);

    if (err > 25.0){
        bool response;
        sprintf(str,"Mesh is very poor; volume percent error = %.2f.",err);
        writeMessageToGUIWindow(str);
        response = getYesNoResponse("Mesh is very poor; volume percent error = %.2f.\nContinue? ",err);
        if (!response){
            resetProgressBar("");
            setStatusMessage("");
            return false;
        }
    }
    else if (err > 5.0){
        sprintf(str,"Warning: Mesh is poor.\nVolume percent error = %.2f \nTry increasing mesh size.",err);
        writeMessageToGUIWindow(str);
    }
    return true;
}
/************************************************************/
bool CoreFunctions::computeZernikeCoefficients(int gridSize, int ZernikeOrder, string outFile)
{
    string voxFile;


    vox.setVoxelGrid(gridSize,gridSize,gridSize);
    vox.setEqualGridSize();
    voxFile = liveVar.projectDirectory + string("temp.vxb");
    writeMessageToGUIWindow("CoreFunction: Attempting to create a voxel mesh; temp file %s ",voxFile.c_str());
    createVoxelMesh(voxFile.c_str());
    if (!checkMeshingError()){
        writeMessageToGUIWindow("CoreFunction: Error in creating mesh ");
        remove(voxFile.c_str()); //delete file
        return false;
    }
    resetProgressBar("Done Creating Mesh");
    remove(voxFile.c_str()); //delete file

    int nsize = gridSize*gridSize*gridSize;

    double* data = new double[nsize];
    int counter;
    counter = 0;
    for (int elx = 0; elx < gridSize; elx++){
        for (int ely = 0; ely < gridSize; ely++){
            for (int elz = 0; elz < gridSize; elz++){
                if (vox.isElemInside(elx,ely,elz)){
                    data[counter] = 1.0;
                }
                else
                    data[counter] = 0.0;
                counter++;
            }
        }
    }
    // compute the zernike descriptors
    //ZernikeDescriptor<double, float> zd (data, gridSize, ZernikeOrder);
    //zd.SaveInvariants(outFile.c_str());
    //ZernikeDescriptor<double, float>::ComplexT3D _grid;
    //zd.Reconstruct(_grid);
    return true;

    vox.unsetVoxelGrid();
    vox.unsetEqualGridSize();
}

/************************************************************/
void CoreFunctions::createLatticeStructureMesh(void)
{
    char latticeSpecs[1024];
    double latticeUnitCellSizeInModelUnits,materialProperties[7],area,boundingBox[3][2],geometryVolume;
    bool success;
    string voxFile,offFile, stlFile;
    indexed_mesh mesh;
    int numberOfLatticeVoxElems;

    // apply dummy material
    materialProperties[0] = liveVar.E;
    materialProperties[1] = liveVar.nu;
    materialProperties[2] = liveVar.Y;
    materialProperties[3] = liveVar.rho;
    materialProperties[4] = liveVar.K;
    materialProperties[5] = liveVar.alpha;
    materialProperties[6] = liveVar.specificHeat;

    if (liveVar.paretoUnits == UNITS_MM){
        // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 1000;
    }
    if (liveVar.paretoUnits == UNITS_INCH){
        // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 386.22;
    }
    for (int part = 0; part < vox.getNumParts(); part++)
        topoptTemp.setIsotropicMaterialPropertiesOfPart(part,materialProperties,liveVar.materialName.c_str());

    voxFile = liveVar.projectDirectory +string("temp.vox");
    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.stl");
    offFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.off");


    vox.getBoundingBox(boundingBox);
    geometryVolume = vox.getVolumeOfSTLGeom(&area);

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM/1000.0;
        break;
    case UNITS_MM:
        latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM;
        break;
    case UNITS_INCH:
        latticeUnitCellSizeInModelUnits = liveVar.latticeUnitCellSizeInMM/25.4;
        break;
    }

    // set all parameters
    topopt.setLatticeType((LATTICE_TYPE) liveVar.latticeType);
    topopt.setLatticeSurfaceRetainType((LATTICE_SURFACE_RETAIN) liveVar.latticeSurfaceRetainType);
    topopt.setLatticeResolution(liveVar.latticeResolution);
    topopt.setLatticeFillRatio(liveVar.latticeFillRatio);
    topopt.setLatticeUnitCellSizeInModelUnits(latticeUnitCellSizeInModelUnits);
    topopt.setLatticeBoundingBox(geometryVolume,boundingBox);

    topopt.getLatticeSpecsAsString(latticeSpecs);// check to see if the numbers are acceptable

    if(!getYesNoResponse(latticeSpecs))
        return;
    timer.start();
    int symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
            2*liveVar.topoptConstraints.ySymmetry
        + 4*liveVar.topoptConstraints.zSymmetry;

    writeMessageToGUIWindow("CoreFunction: Attempting to create mesh... ");
    numberOfLatticeVoxElems = topopt.getLatticeNumberOfVoxelElementsNeeded();

    success = vox.voxelize(numberOfLatticeVoxElems,liveVar.maxAspectRatio,symmetryIndex,dummyCallBackCoreFunctions);
    if (!success) {
        writeWarningToGUIWindow("CoreFunction: Could not mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }
    vox.dumpVoxelDataBinary(voxFile.c_str());

    for (int part = 0; part < vox.getNumParts(); part++)
        topopt.setIsotropicMaterialPropertiesOfPart(part,materialProperties,liveVar.materialName.c_str());

    if (!topopt.readVoxelFileBinary(voxFile.c_str()))
        return;

    remove(voxFile.c_str());
    topopt.resetBC();//elasticity
    int bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodesWithLabel(faceGroup,liveVar.BCType[bcSet][faceGroup],liveVar.BCValue[bcSet][faceGroup]);


    writeMessageToGUIWindow("CoreFunction: Attempting to create a lattice structure ... ");

    //topopt.computeLatticeUnitCellAsOffFile();return;
    // creating the isosurface from mesh
    topopt.dumpIsoSurfaceFromMesh(stlFile.c_str());
    mesh.load_from_binary_stereo_lithography_file(stlFile.c_str());
    remove(stlFile.c_str());
    mesh.taubin_smooth(0.6307, -0.6734,liveVar.latticeSmoothening);
    topopt.resetIsoCounter();
    mesh.save_to_extended_off_file(offFile.c_str());
    topopt.readExtendedOffFile(offFile.c_str());
    remove(offFile.c_str());

    if (liveVar.latticeSaveSTL) // save smoothed stl file
        mesh.save_to_binary_stereo_lithography_file(stlFile.c_str());

    liveVar.modelScale = topopt.getModelScale();
    topopt.createElementMatricesTemplates();// we will need this FEA

    liveVar.TOPOPT_SOLVED = true;
    writeMessageToGUIWindow("nDOF = %d; time taken = %0.3f (s)",3*topopt.getNumNodes(),timer.getElapsedTimeInSec());
    writeMessageToGUIWindow("Volume fraction %.2f \n",topopt.getCurrentVolumeFractionOfOptimizedParts());

    liveVar.forceRemesh = 0;
    liveVar.plotAxis = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotTopOptConstraints = false;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.TOPOPT_SOLVED = true;
    liveVar.LATTICE_DESIGNED = true;
    liveVar.isTopOptComplete = true;
    resetProgressBar("");
    setStatusMessage("Lattice design complete!");
}
/************************************************************/
bool CoreFunctions::generateResultsFromFinalMesh(string finalMeshName)
{
    //Given a final mesh file, generate static fea results at start and final
    int bcSet;

    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("Structural boundary conditions is undefined; please see specify restraints and loads");
        return false;
    }
    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }
    changeTopOptBC();

    timer.start();
    if (!verifySymmetryConditions()){
        writeWarningToGUIWindow("Error: Incorrect symmetry being imposed in FEA. Please verify. ");
        return false;
    }

    // we need to read the voxel file couple of times!!

    // first read Voxel file, and save the final pseudo density
    if (!topopt.readVoxelFileBinary(finalMeshName.c_str() )){
        writeWarningToGUIWindow("Error: Could not read mesh %s ",finalMeshName.c_str());
        return false;
    }
    double *pseudoDensitySave;
    pseudoDensitySave = (double *) malloc(topopt.getNumElems()*sizeof(double));
    topopt.getPseudoDensity(pseudoDensitySave);


    // read Voxel file again, but disregard pseudodensity to ensure proper connectivity of the entire mesh
    if (!topopt.readVoxelFileBinary(finalMeshName.c_str(),true )){
        writeWarningToGUIWindow("Error: Could not read mesh %s ",finalMeshName.c_str());
        return false;
    }

    liveVar.forceRemesh = false;
    liveVar.MESH_APPLIED = true;

    writeMessageToGUIWindow("Mesh read with %d elements",topopt.getNumElems());

    topopt.resetBC();//elasticity
    topopt.allocateMemory_thermal();

    topopt.resetBC_thermal(); //thermal BC

    bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodesWithLabel(faceGroup,liveVar.BCType[bcSet][faceGroup],liveVar.BCValue[bcSet][faceGroup]);

    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodes_thermal(faceGroup,liveVar.ThermalBCType[bcSet][faceGroup],liveVar.ThermalBCArea[bcSet][faceGroup],liveVar.ThermalBCValue[bcSet][faceGroup],liveVar.ThermalBCRef[bcSet][faceGroup]);

    double acc[4];

    for (int i = 0; i < 4; i++)
        acc[i] = liveVar.ElasticityAcceleration[bcSet][i];
    topopt.setElasticityAcceleration(acc);// add body force to h_f

    liveVar.MESH_APPLIED = true;
    if (!topopt.isMeshConnected()){
        writeWarningToGUIWindow("Error: Mesh is disconnected.");
        return false;
    }
    topopt.resetIsoCounter();
    liveVar.modelScale = topopt.getModelScale();

    topopt.createElementMatricesTemplates();

    writeMessageToGUIWindow("nDOF = %d; time taken = %0.3f (s)",3*topopt.getNumNodes(),timer.getElapsedTimeInSec());

    topopt.releaseEigen();
    //topopt.initializeContours();//get rid of old contours
    topopt.InitializeTopOpt(1);// get rid of isosurface triangles

    // create tessellation for display
    string tessellationFile;

    tessellationFile = liveVar.projectDirectory + string("tessellation.stl");
    writeMessageToGUIWindow("CoreFunctions: Creating tessellation file %s ",tessellationFile.c_str());
    vox.dumpSTLTesselation(tessellationFile.c_str(),20,0.010*liveVar.modelScale);
    topopt.readTessellationFile(tessellationFile.c_str());//read into AFElasticity; will be used later in SolveElasticity
    remove(tessellationFile.c_str()); //delete file

    solveStatic();

    liveVar.displayField = DISPLAY_FIELD_NONE;

    // now push the final pseudodensity back
    topopt.setPseudoDensity(pseudoDensitySave);
    free(pseudoDensitySave);


    return true;
}
/************************************************************/
bool CoreFunctions::createTopOptMesh(void)
{
    string voxFile;
    int bcSet;
    Timer localTimer;

    voxFile = liveVar.projectDirectory + string("temp.vxb");
    writeMessageToGUIWindow("CoreFunction: Attempting to create a voxel mesh; temp file %s ",voxFile.c_str());
    createVoxelMesh(voxFile.c_str());
    if (!checkMeshingError()){
        writeMessageToGUIWindow("CoreFunction: Error in creating mesh ");
        remove(voxFile.c_str()); //delete file
        return false;
    }
    setStatusMessage("Reading Mesh");
    localTimer.start();
    if (!topopt.readVoxelFileBinary(voxFile.c_str())){
        return false;
    }
    std::string paretoMeshFile;
    paretoMeshFile =liveVar.projectDirectory + string("temp.mesh");
    //topopt.dumpParetoMesh(paretoMeshFile.c_str());
    remove(voxFile.c_str());
    writeMessageToGUIWindow("CoreFunction: Mesh read: %.3f (secs) ",localTimer.getElapsedTimeInSec());

    //std::string AbaqusMeshFile;
    //AbaqusMeshFile =liveVar.projectDirectory + string("temp.inp");

    //topopt.dumpMeshAsAbaqusInpFile(( char *)AbaqusMeshFile.c_str());
    topopt.resetBC();//elasticity

    //string paretoMesh = liveVar.projectDirectory + liveVar.modelName + string(".mesh");
    //topopt.dumpParetoMesh(paretoMesh.c_str());
    topopt.allocateMemory_thermal();

    topopt.resetBC_thermal(); //thermal BC

    bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodesWithLabel(faceGroup,liveVar.BCType[bcSet][faceGroup],liveVar.BCValue[bcSet][faceGroup]);

    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
        topopt.setBCOnNodes_thermal(faceGroup,liveVar.ThermalBCType[bcSet][faceGroup],liveVar.ThermalBCArea[bcSet][faceGroup],liveVar.ThermalBCValue[bcSet][faceGroup],liveVar.ThermalBCRef[bcSet][faceGroup]);

    double acc[4];

    for (int i = 0; i < 4; i++)
        acc[i] = liveVar.ElasticityAcceleration[bcSet][i];

    topopt.setElasticityAcceleration(acc);// add body force to h_f

    if (!topopt.isMeshConnected()){
        writeWarningToGUIWindow("Error: Mesh is disconnected.\nTry selecting ThinStructure Option");
        resetProgressBar("");
        setStatusMessage("");
        return false;
    }
    topopt.resetIsoCounter();
    liveVar.modelScale = topopt.getModelScale();
    //topopt.forceMaxTemplates();
    topopt.createElementMatricesTemplates();
    liveVar.MESH_APPLIED = true;
    writeMessageToGUIWindow("nDOF = %d; time taken = %0.3f (s)",3*topopt.getNumNodes(),timer.getElapsedTimeInSec());
    writeMessageToGUIWindow("---------------------------------------------");

    topopt.releaseEigen();
    //topopt.initializeContours();//get rid of old contours
    topopt.InitializeTopOpt(1);// get rid of isosurface triangles

    // create tessellation for display
    string tessellationFile;

    tessellationFile = liveVar.projectDirectory + string("tessellation.stl");
    writeMessageToGUIWindow("CoreFunctions: Creating tessellation file %s ",tessellationFile.c_str());
    vox.dumpSTLTesselation(tessellationFile.c_str(),20,0.010*liveVar.modelScale);
    topopt.readTessellationFile(tessellationFile.c_str());//read into AFElasticity; will be used later in SolveElasticity
    remove(tessellationFile.c_str()); //delete file

    setStatusMessage("Mesh Read");
    return true;
}
/************************************************************/
bool CoreFunctions::readMeshAndAllocate(const char *meshFile)
{
	int bcSet;
	Timer localTimer;


	setStatusMessage("Reading Mesh");
    cout << meshFile << endl;
    topopt.enableHexmeshVerbose();


    localTimer.start();
    if (strstr(meshFile,".msh")){
        cout << "Reading  voxel mesh" << endl;
        if (!topopt.readVoxelFileBinary(meshFile)) {
            return false;
        }
    }
    else if (strstr(meshFile,".inp")){
        if (!topopt.readAbaqusInpFile(meshFile)) {
            return false;
        }
        topopt.createElementMatricesTemplates();
        char outputFile[] = "abaqusMesh.msh";
        topopt.dumpMeshBinary(outputFile);
        return true;
    }


	writeMessageToGUIWindow("CoreFunction: Mesh read: %.3f (secs) ", localTimer.getElapsedTimeInSec());

	topopt.resetBC();//elasticity
	topopt.allocateMemory_thermal();

	topopt.resetBC_thermal(); //thermal BC

	bcSet = liveVar.currentBCSet;
	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodesWithLabel(faceGroup, liveVar.BCType[bcSet][faceGroup], liveVar.BCValue[bcSet][faceGroup]);

	for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++)
		topopt.setBCOnNodes_thermal(faceGroup, liveVar.ThermalBCType[bcSet][faceGroup], liveVar.ThermalBCArea[bcSet][faceGroup], liveVar.ThermalBCValue[bcSet][faceGroup], liveVar.ThermalBCRef[bcSet][faceGroup]);

	double acc[4]; 

	for (int i = 0; i < 4; i++)
		acc[i] = liveVar.ElasticityAcceleration[bcSet][i];
	topopt.setElasticityAcceleration(acc);// add body force to h_f

	if (!topopt.isMeshConnected()) {
        writeWarningToGUIWindow("Warning: Mesh is disconnected.");
        int seedElem = 0;
        topopt.removeDisconnectedElementsUsingSeedElement(seedElem);
        //resetProgressBar("");
        //setStatusMessage("");
        //return false;
    }
	topopt.resetIsoCounter();
	liveVar.modelScale = topopt.getModelScale();

	topopt.createElementMatricesTemplates();

    writeMessageToGUIWindow("nDOF = %d", 3 * topopt.getNumNodes());
	writeMessageToGUIWindow("---------------------------------------------");

	topopt.releaseEigen();
	topopt.InitializeTopOpt(1);// get rid of isosurface triangles

							   // create tessellation for display
	

	setStatusMessage("Mesh Read");
    liveVar.MESH_APPLIED = true;
    liveVar.forceRemesh = false;
	return true;
}
/************************************************************/
void CoreFunctions::initializeTopOptVariables(void)
{
    int nNodes,useGPU,mode;

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow(" First create mesh");
        return;
    }
    nNodes = topopt.getNumNodes();
    if (liveVar.useGPU) {
#ifdef COMPILE_GPUCODE
        if (isThereValidCudaDevice() ==1) {
            writeMessageToGUIWindow("Ready to optimize topology on GPU ... ");
            useGPU = 1;
            topopt.enableGPU();
        }
        else {
            writeWarningToGUIWindow("There is no valid CUDA device supporting double precision.");

            return;
        }
#endif
    }
    else {
        writeMessageToGUIWindow("Ready to optimize topology on CPU ... ");
        topopt.disableGPU();
        useGPU = 0;
    }

    if (liveVar.topoptConstraints.keepDirichlet) mode = 3;
    else mode = 1;

    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();


    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();

    topopt.InitializeTopOpt(mode);
    setTopOptConstraints();

    topopt.setTopoptMode(TOPOPT_MODE_ELASTIC);
    topopt.setTStoAnalytical(true);
}
/************************************************************/
bool CoreFunctions::verifySymmetryConditions(void)
{
    bool symmetry[3];

    vox.getSTLSymmetry(symmetry);

    if ((liveVar.topoptConstraints.xSymmetry) && (!symmetry[0])){
        writeWarningToGUIWindow("Geometry is not symmetric in X, but X symmetry is imposed. Please verify. ");
        return false;
    }
    if ((liveVar.topoptConstraints.ySymmetry) && (!symmetry[1])){
        writeWarningToGUIWindow("Geometry is not symmetric in Y, but Ysymmetry is imposed. Please verify. ");
        return false;
    }
    if ((liveVar.topoptConstraints.zSymmetry) && (!symmetry[2])){
        writeWarningToGUIWindow("Geometry is not symmetric in Z, but Z symmetry is imposed. Please verify. ");
        return false;
    }

    return true;
}
/************************************************************/
bool CoreFunctions::verifyDirichletConditions(void)
{
    int type[] = {1,1,1};
    int bcSet;

    bool groupUsed[STL_MAX_GROUPS];
    for (int g = 0; g < STL_MAX_GROUPS; g++)
        groupUsed[g] = false;

    groupUsed[0] = true;// always needed
    for (int tri = 0; tri < vox.getNumTriangles(); tri++)
        groupUsed[vox.getTriGroup(tri)] = true;


    //Make sure all three directions are fixed over the geometry
    bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++){
        if (!groupUsed[faceGroup]) continue;
        type[0] *= liveVar.BCType[bcSet][faceGroup][0];
        type[1] *= liveVar.BCType[bcSet][faceGroup][1];
        type[2] *= liveVar.BCType[bcSet][faceGroup][2];
    }
    if (type[0] !=0 )
        alert("Warning: Geometry is not fixed along x direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n");

    if (type[1] !=0 )
        alert("Warning: Geometry is not fixed along y direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n");

    if (type[2] !=0 )
        alert("Warning: Geometry is not fixed along z direction.\nIt is strongly recommended that a sliding condition be applied to restrain model\n");


    if ( (type[0] != 0) || (type[1] != 0) || (type[2] != 0))
        return false;
    else
        return true;
}
/************************************************************/
bool CoreFunctions::verifyNeumannConditions(void)
{
    int type[] = {1,1,1};
    int bcSet;

    //Make sure all three directions are fixed over the geometry
    bcSet = liveVar.currentBCSet;
    for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++){
        for (int j = 0; j < 3; j++)
            if (fabs(liveVar.BCValue[bcSet][faceGroup][j]) > 0)
                return true;
    }
    return false;
}
/************************************************************/
bool CoreFunctions::solveThermoStatic(void)
{

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.THERMAL_FEA_SOLVED = false;// avoids repeated display
    setStatusMessage("Solving ThermoStatic");
    writeMessageToGUIWindow("Attempting to solve static thermal problem ...");
    if (!liveVar.BC_THERMAL_APPLIED ){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return false;
    }
    changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        createTopOptMesh();

    if (!liveVar.MESH_APPLIED){
        writeWarningToGUIWindow("Mesh was not created");
        return false;
    }
    writeMessageToGUIWindow("Starting ThermoStatic FEA");
    timer.start();
    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));


    if (liveVar.useGPU)
        topopt.enableGPU();
    else
        topopt.disableGPU();

    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    liveVar.nWorkSpaceGroups = getRecommendedDeflationGroups(liveVar.nElemsToMesh);

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();


    topopt.setThermoElasticReferenceTemperature(liveVar.zeroStrainTemperature);//

    topopt.solveThermalProblem();
    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());

    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;

    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Solved thermal ...");
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");

    topopt.pushThermalSolutionToTessellation();
    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = true;
    setStatusMessage("ThermoStatic Solved");
    return true;
}
/************************************************************/
bool CoreFunctions::solveThermoTransient(void)
{
    int nNodes;
    double initialTemperature,simulationTime,timeStepSize;
    bool solveElastic = true;
    int isPhaseChange = 0;
    setStatusMessage("Solving Thermal Transient");

    nNodes = topopt.getNumNodes();
    if (!liveVar.MESH_APPLIED)
        return false;
    writeMessageToGUIWindow("Starting Thermo Transient FEA");
    timer.start();
    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));

    topopt.disableGPU();
    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();

    initialTemperature = 23;
    simulationTime = 1000.0;
    timeStepSize = 5.0;
    topopt.setThermoElasticReferenceTemperature(initialTemperature);//
    if (liveVar.useNonLinearThermal){
        float k[3],c[3],alpha[3],TLower,TUpper;
        k[0] = 16.0f;
        k[1] = 0.012f;
        k[2] = 0.0f;
        c[0] = 1000;
        c[1] = c[2]= 0.0f;
        alpha[0] = 6e-4;
        alpha[1] = alpha[2] = 0;
        TLower = 23.0f; // C
        TUpper = 1200.0f; // C
        topopt.setNonLinearThermalExpansionCoeffs(alpha);
        topopt.setNonLinearThermalCoefficients(k, c);
        topopt.solveNLStaticThermalProblem(solveElastic,dummyCallBackCoreFunctions);

        double L = 1e5;
        double Tms = 280;
        double Tml = 320;
        topopt.InitializeNonLinearPhaseChangeData(isPhaseChange,L,  Tms,  Tml);
        topopt.solveNLTransientThermalProblem(initialTemperature, timeStepSize,simulationTime,solveElastic,dummyCallBackCoreFunctions);
    }
    else {
        topopt.solveTransientThermoElasticProblem( initialTemperature,NULL, timeStepSize, simulationTime,solveElastic,dummyCallBackCoreFunctions);
    }

    writeMessageToGUIWindow("Time taken = %0.3f (s)",timer.getElapsedTimeInSec());
    liveVar.THERMAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_NONE;
    liveVar.displayField = DISPLAY_FIELD_TEMPERATURE;
    liveVar.plotAxis = 0;
    writeMessageToGUIWindow("Pushing thermal solution to tessellation ... ");
    topopt.pushThermalSolutionToTessellation();
    if (solveElastic) {
        liveVar.STATIC_FEA_SOLVED = true;
		liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
        liveVar.displayGeom = DISPLAY_GEOM_NONE;
        liveVar.displayField = DISPLAY_FIELD_STRESS;
        topopt.pushInitialStaticSolutionToTessellation();
    }

    setStatusMessage("Thermal Transient Complete");
    return true;
}
/************************************************************/
bool CoreFunctions::solveThermoElastic(void)
{

    setStatusMessage("Solving Thermo Elastic");

    //compute the thermal forces and then solve elastic
    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();
    int nNodes = topopt.getNumNodes();
    double *h_fSave = (double *)malloc(3*nNodes*sizeof(double));
    topopt.setThermoElasticReferenceTemperature(liveVar.zeroStrainTemperature);// where is there no stress or strain
    topopt.getForceVector(h_fSave);
    topopt.addThermalForce();

    if (!topopt.solveElasticityStatic() ){
        writeWarningToGUIWindow("Could not solve structural FEA; check for disconnected components, or try increasing mesh size");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
         resetProgressBar("");
        return false;
    }
    topopt.postprocess();

    topopt.setForceVector(h_fSave);
    free(h_fSave);

    double timeForOneFEA = timer.getElapsedTimeInSec();
    liveVar.timeForOneFEA = timeForOneFEA;

    writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
    writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
    writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Pushing elasticity solution to tessellation ... ");
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;

    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = true;

    setStatusMessage("Thermo-Elastic Solved");
    return true;
}
/************************************************************/
bool CoreFunctions::singleLoadOptimize_multiscale(void)
{
    // This solves the static problem at two scales and optimizes design at small scale
    //SET the following variables
    int nClusters = liveVar.noOfMicroStructureGroups;
    double volFrac = liveVar.desiredVolFraction;
    int clusteringType = liveVar.typeOfClustering;//2 = density cluster, 3 = density+strain cluster
    int maxIterations = 10;
    cout<<"nClusters = "<<nClusters<<"; volFrac = "<<volFrac<<"; clusteringType = "<<clusteringType<<endl;

    //Declaring other variables
    int elem,cluster, microElem, nElems, mi_nElems, i, j, tempInt2;
    int  dataSize[2];
    int *elemToClusterMapping, *nClusterElems;

    double *allEigenVectors,*criteria, *kCenters, *microPseudoDensity;
    double *equivD,*gradD, *volPerMicroElem, *microSensitivities;
    double tempDouble, totalVolume;
    double materialProperties[7];
    double elemSize[3],mi_elemSize[3],principalVector[3][3],principalStrains[3];

    char materialName[50];

    bool rotation;
    bool *keepMicroElems;

    topopt.resetLinearSolvesCounter();
    changeTopOptBC();
    solveStatic();

    nElems = topopt.getNumElems();

    if(clusteringType == 3)
        rotation = true;
    else
        rotation = false;

    if(rotation){
        allEigenVectors = (double*)malloc(9*nElems*sizeof(double));
        criteria = (double*)malloc(3*nElems*sizeof(double));
    }
    else{
        allEigenVectors = NULL;
        criteria = (double*)malloc(nElems*sizeof(double));
    }
    elemToClusterMapping = (int*)malloc(nElems*sizeof(int));
    nClusterElems = (int*)malloc(nClusters*sizeof(int));
    equivD = (double*)malloc(nClusters*36*sizeof(double));

    //For multiple runs reset number of materials in topopt to 1
    topopt.getMaterialPropertiesOfPart(0,materialProperties,materialName);
    topopt.removeAllMaterials();
    topopt.setIsotropicMaterialPropertiesOfPart(0,materialProperties,materialName);

    //Step0: create element stiffness matrix templates for macromesh
    topopt.generateStiffnessMatrixTemplates();

    //Step1: perform TopOpt for SIMP p = 1 on macro-domain
    double *density = (double*)malloc(nElems*sizeof(double));

    for(elem = 0;elem<nElems;elem++)
        topopt.setMaterialIdOfElem(0,elem);
    topopt.performSIMPpEquals1Optimization(volFrac,density);
    topopt.postprocess(); //To carry out strain computation

    //Criteria computation for clustering
    if(rotation){
        for(elem = 0;elem<nElems;elem++){
            topopt.getPrincipalStrainAndVectorOfElem(elem,principalStrains,principalVector);

            for(i =0;i<3;i++){
                for(j=0;j<3;j++){
                    allEigenVectors[elem*9+i*3+j] = principalVector[i][j];
                }
            }
            if (density[elem]<0.1){
                criteria[elem*3] = -5;  criteria[elem*3+1] = -5;    criteria[elem*3+2] = -5;
            }
            else if (density[elem]>0.9){
                criteria[elem*3] = 5;   criteria[elem*3+1] = 5;     criteria[elem*3+2] = 5;
            }
            else{
                tempInt2++;
                criteria[elem*3] = density[elem];
                criteria[elem*3+1] = pow(principalStrains[1]/principalStrains[0],2);
                criteria[elem*3+2] = pow(principalStrains[2]/principalStrains[0],2);
            }
        }
        dataSize[0] = 3;
        kCenters = (double*)malloc(nElems*3*sizeof(double));
    }
    else if(clusteringType == 2){
        dataSize[0] = 1;
        memcpy(criteria,density,nElems*sizeof(double));
        kCenters = (double*)malloc(nElems*sizeof(double));
    }
    if(clusteringType !=1){ //Density based k-clustering
        dataSize[1] = nElems;
        topopt.kMeans(criteria,dataSize,nClusters,elemToClusterMapping,kCenters,1,false);
        safeFree(kCenters);
        safeFree(criteria);}
    else{ //Density brackets
        for (elem = 0; elem < nElems; elem++){
            elemToClusterMapping[elem] = ceil(density[elem]*nClusters)-1;
        }
    }
    safeFree(density);
    //Restore density and exponent
    for(elem = 0;elem<nElems;elem++)
        topopt.setPseudoDensityOfElem(elem,1);
    topopt.setEexponent(1.0); //Doesn't matter as long as all PseudoDensities are 1

    //Step2: cluster macro-elements and generate material property placeholders
    topopt.disableSimpMode();
    topopt.disableNodalSpMV();// cannot use nodal SpMV
    topopt.enablePreconditioner();
    topopt.getMaterialPropertiesOfPart(0,materialProperties,materialName);
    topopt.addPlaceHolderMaterial(nClusters);  //Setting up material placeholders for microstructure properties
    for (elem = 0; elem < nElems; elem++){
        topopt.setMaterialIdOfElem(elemToClusterMapping[elem],elem); //Assign material Id according to clustering
    }

    //Initialize microstructures
    topopt.getElemSize(elemSize);
    double boundingBox[3][2];
    tempDouble = (elemSize[0]+elemSize[1]+elemSize[2])/3; //Mean size of a macroVoxel is the size of microstructure; Ideal case macroVoxel is a cube
    for(i =0;i<3;i++){
        boundingBox[i][0] = 0;
        boundingBox[i][1] = tempDouble;
    }
    if(topoptTemp.generateCuboidMeshWithMapping(liveVar.numMicroElems,boundingBox)!=1){//Create of microMesh with equal sides and equal number of elements on each side
        writeMessageToGUIWindow("Cuboid mesh with mapping unsuccessful");
        return false;
    }
    mi_nElems = topoptTemp.getNumElems();
    topoptTemp.getElemSize(mi_elemSize);

    //Assign memory
    microPseudoDensity = (double*)malloc(nClusters*mi_nElems*sizeof(double));
    gradD = (double*)malloc(nClusters*36*mi_nElems*sizeof(double));
    volPerMicroElem = (double*)malloc(nClusters*mi_nElems*sizeof(double));
    microSensitivities = (double*)malloc(nClusters*mi_nElems*sizeof(double));
    keepMicroElems = (bool*)malloc(nClusters*mi_nElems*sizeof(bool));

    memset(nClusterElems,0,nClusters*sizeof(int));
    for (int elem = 0; elem < nElems; elem++)
        nClusterElems[elemToClusterMapping[elem]]++;

    //Volume associated with each microElement = volume of a macroElement/(number of elements per microstructure) * (number of that microstructure in the domain)
    for(cluster = 0;cluster<nClusters;cluster++){
        for(microElem = 0;microElem<mi_nElems;microElem++)
            volPerMicroElem[cluster*mi_nElems+microElem] = elemSize[0]*elemSize[1]*elemSize[2]/mi_nElems*nClusterElems[cluster];
    }

    //Initialize microstructure based on average density of each cluster or all clusters may be same; using the latter
    for(cluster = 0;cluster<nClusters;cluster++){
        topoptTemp.perturbCuboidMeshDensityToVolFraction(volFrac,&microPseudoDensity[cluster*mi_nElems]);
    }

    topoptTemp.setIsotropicMaterialPropertiesOfPart(0,materialProperties,materialName);
    topoptTemp.createElementMatricesTemplates(true);

    //initialize some variables to be used during iteration
    topopt.forceMaxTemplates();// every element is a template
    //Freeze some microElements on the boundary to ensure better connectivity
    topoptTemp.freezeEdgesOfMicrostructure(keepMicroElems,nClusters,mi_nElems);
    topoptTemp.initializeForMicrostructure();
    totalVolume = 0;
    for(cluster= 0;cluster<nClusters;cluster++){
        for(microElem = 0;microElem<mi_nElems;microElem++){
            totalVolume += volPerMicroElem[cluster*mi_nElems+microElem];
        }
    }
    topopt.enablePreconditioner();
    cout<<"Starting iterations now ..."<<endl;
    for(int iter = 0;iter <maxIterations;iter++){
        //Get equivalent D matrices for initial microstructures and its sensitivity
        memset(gradD,0,36*nClusters*mi_nElems*sizeof(double));
        memset(equivD,0,36*nClusters*sizeof(double));
        for(cluster = 0;cluster<nClusters;cluster++){
            topoptTemp.setPseudoDensity(&microPseudoDensity[cluster*mi_nElems]);
            topoptTemp.computeMicrostructureEquivalentD(&(equivD[36*cluster]),&(gradD[36*cluster*mi_nElems]));
        }

        //Assign Ke to macroelements: use of Ke templates computed earlier in step 0
        topopt.assignElemStiffMatBasedOnClusters(rotation,equivD,allEigenVectors,elemToClusterMapping);
        topopt.disableDeflation();
        topopt.enableDeflation();

        if (!topopt.solveElasticityStatic() ){
            writeWarningToGUIWindow("Could not solve FEA at MacroLevel; check for disconnected components, or try increasing mesh size");
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            resetProgressBar("");
            return false;
        }
        cout<<"Iter: "<<iter<<", Compliance = "<<topopt.computeCompliance()<<endl;

        memset(microSensitivities,0,nClusters*mi_nElems*sizeof(double));
        topopt.computeMicroElemSensitivity(rotation,microSensitivities,allEigenVectors,elemToClusterMapping,gradD,mi_nElems,nClusters);
        for(cluster = 0;cluster<nClusters;cluster++){
            topoptTemp.smoothenElemField(&microSensitivities[cluster*mi_nElems],2);
        }
        topoptTemp.updateDensitySIMP_general(nClusters*mi_nElems,microPseudoDensity,microSensitivities,volPerMicroElem,keepMicroElems,volFrac,0.2,0.5,0.01);

        //Print Microstructures
        /*
        char filename[50];
        for(cluster = 0;cluster<nClusters;cluster++){
            topoptTemp.setPseudoDensity(&microPseudoDensity[cluster*mi_nElems]);
            sprintf(filename,"StlForCluster%dAtIteration%d.stl",cluster+1,iter+1);
            topoptTemp.createAndDumpIsoSurfaceFromPseudoDensity(filename);
        }
        topoptTemp.printMicrostructures(microPseudoDensity,nClusters,iter);
        */

        //multiscaleCallBackFunction(0.5);
    }

    safeFree(allEigenVectors);
    safeFree(volPerMicroElem);
    safeFree(keepMicroElems);
    safeFree(microPseudoDensity);
    safeFree(microSensitivities);
    safeFree(elemToClusterMapping);
    safeFree(equivD);
    safeFree(gradD);
    safeFree(nClusterElems);
    return true;
}
/************************************************************/
bool CoreFunctions::solveStaticWithMaxTemplates(void)
{
    // This solves the static problem with #templates = #elems
    double Ke0[24][24],Ke[24][24],rnd;

    topopt.enableHexmeshVerbose();
    topopt.enableDeflationVerbose();
    topopt.forceMaxTemplates();// every element is a template
    topopt.disableNodalSpMV();// cannot use nodal SpMV
    topopt.createElementMatricesTemplates(true);// baseline to create memory
    topopt.getKeOfTemplateElem(0,Ke0);

    for (int elem = 0; elem < 0; elem++){
        rnd = 0.9 + 1.0*rand()/RAND_MAX;
        for (int i = 0; i < 24; i++)
            for (int j = i; j < 24; j++)
                Ke[i][j] = rnd*Ke0[i][j];
        for (int i = 0; i < 24; i++)
            for (int j = 0; j < i; j++)
                Ke[i][j] = Ke[j][i];
        topopt.setKeOfTemplateElem(elem,Ke);
    }

    topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    if (!topopt.solveElasticityStatic() ){
        writeWarningToGUIWindow("Could not solve FEA; check for disconnected components, or try increasing mesh size");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        resetProgressBar("");
        return false;
    }

    topopt.postprocess();
    double timeForOneFEA;
    timeForOneFEA = timer.getElapsedTimeInSec();
    liveVar.timeForOneFEA = timeForOneFEA;

    writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
    writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
    writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());


    writeMessageToGUIWindow("Pushing elasticity solution to tessellation ... ");
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;

    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = true;
    setStatusMessage("Static FEA Complete");

    return true;
}
/************************************************************/
bool CoreFunctions::unBond(void)
{
    // if partId of elem is not same same as all of its neighbors, set it to void
    int neighborElems[1000],nNeighbors,partId,partIdNeighbor;
    for (int elem = 0; elem < topopt.getNumElems(); elem++){
        partId = topopt.getPartIdOfElem(elem);
        nNeighbors = topopt.getNeighboringElementsOfElem(elem,neighborElems);
        for (int i =0; i < nNeighbors; i++){
            partIdNeighbor = topopt.getPartIdOfElem(neighborElems[i]);
            if (partId > partIdNeighbor){
                topopt.setPseudoDensityOfElem(elem,0);
                break;
            }
        }

    }
    return true;
}

/************************************************************/
bool CoreFunctions::solveStatic(void)
{
    int nNodes;
    double timeForOneFEA;
    double *h_fSave;

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("Structural boundary conditions is undefined; please see specify restraints and loads");
        return false;
    }
    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }
    changeTopOptBC();

    if (!verifySymmetryConditions()){
        writeWarningToGUIWindow("Error: Incorrect symmetry being imposed in FEA. Please verify. ");
        return false;
    }


    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED ))
        if (!createTopOptMesh())
            return false;

    if (!verifyDirichletConditions()){
        writeWarningToGUIWindow("Warning: Geometry is not fixed in all three directions.\nThe displacements may be incorrect, and FEA will take longer. ");
        //return false;
    }

    if (topopt.getForceNorm() == 0) {
        writeMessageToGUIWindow("Error: Zero force detected; possibly due to a very coarse mesh");
    }

    nNodes = topopt.getNumNodes();

    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    topopt.setNodeAssignmentMethod(0);// grid method

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.plotElasticityBC = false;
    writeMessageToGUIWindow("Starting Static FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));


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


    liveVar.nWorkSpaceGroups = getRecommendedDeflationGroups(liveVar.nElemsToMesh);

    if (liveVar.workSpaceMethod != DEFLATION_NONE){
        cout << "type: " << liveVar.workSpaceMethod << "#: " << liveVar.nWorkSpaceGroups<< endl;
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    }
    else
        topopt.disableDeflation();

    topopt.setThermalEffects(false);
    if (liveVar.includeThermalEffects) {
        topopt.disableDeflation();
        topopt.setThermalEffects(true);// for topopt
        h_fSave = (double *)malloc(3*topopt.getNumNodes()*sizeof(double));// needed below
        topopt.setThermoElasticReferenceTemperature(liveVar.zeroStrainTemperature);// where is there no stress or strain
        topopt.solveThermalProblem();
        writeMessageToGUIWindow("Time taken for Thermal = %0.3f (s)",timer.getElapsedTimeInSec());
        liveVar.THERMAL_FEA_SOLVED = true;
        topopt.getForceVector(h_fSave);// the elasticity force is saved
        topopt.addThermalForce();
    }
    setStatusMessage("Solving Static Structural ");
    timer.start();

    if (liveVar.directSolver){
        if (!topopt.solveElasticityStaticDirect() ){
            writeWarningToGUIWindow("Could not solve FEA; check for disconnected components, or try increasing mesh size");
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            resetProgressBar("");
            return false;
        }
    }
    else {
        if (!topopt.solveElasticityStatic() ){
            writeWarningToGUIWindow("Could not solve FEA; check for disconnected components, or try increasing mesh size");
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            resetProgressBar(" ");
            return false;
        }
    }

    topopt.postprocess();
    if (liveVar.includeThermalEffects) {
        topopt.setForceVector(h_fSave);
        free(h_fSave);
    }
    timeForOneFEA = timer.getElapsedTimeInSec();
    liveVar.timeForOneFEA = timeForOneFEA;

    writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
    writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
    writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());

    if ((liveVar.TOPOPT_SOLVED) && (!liveVar.forceRemesh) ){
        writeMessageToGUIWindow("Pushing stress results to iso");
        topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface
        liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
        liveVar.displayField = DISPLAY_FIELD_STRESS;
    }
    else{
        writeMessageToGUIWindow("Pushing elasticity solution to tessellation ... ");
        topopt.pushInitialStaticSolutionToTessellation();
        liveVar.STATIC_FEA_SOLVED = true;
		liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_STRESS;
    }
    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = true;
    setStatusMessage("Static FEA Complete");
    return true;
}
/************************************************************/
bool CoreFunctions::solveFrequencyResponse(void)
{
    int nNodes;
    double timeForOneFEA;

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("Structural boundary conditions is undefined; please see specify restraints and loads");
        return false;
    }
    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }
    changeTopOptBC();

    if (!verifySymmetryConditions()){
        writeWarningToGUIWindow("Error: Incorrect symmetry being imposed in FEA. Please verify. ");
        return false;
    }


    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED ))
        if (!createTopOptMesh())
            return false;


    if (!verifyDirichletConditions()){
        writeWarningToGUIWindow("Warning: Geometry is not fixed in all three directions.\nThe displacements may be incorrect, and FEA will take longer. ");
        //return false;
    }

    if (topopt.getForceNorm() == 0) {
        writeMessageToGUIWindow("Error: Zero force detected; possibly due to a very coarse mesh");
    }

    nNodes = topopt.getNumNodes();

    if (liveVar.nodeBasedKU) topopt.enableNodalSpMV();
    else topopt.disableNodalSpMV();

    topopt.setNodeAssignmentMethod(0);// grid method

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.plotElasticityBC = false;
    writeMessageToGUIWindow("Starting Static FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));


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

    liveVar.nWorkSpaceGroups = getRecommendedDeflationGroups(liveVar.nElemsToMesh);
    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);



    setStatusMessage("Solving Frequency Structural ");
    timer.start();
    int nFrequencySteps;
    double omega[1000];
    double deltaMax[1000];
    double phase[1000];
    double fStart,fEnd,fStep;// % Hz


    fStart = 100;
    fEnd = 1000;
    fStep = 100;

    topopt.setDampingParams(liveVar.structuralDynamics.alphaM,liveVar.structuralDynamics.betaK);

    nFrequencySteps = ceil((fEnd-fStart)/fStep)+1;

    for (int step =0; step < nFrequencySteps; step++)
        omega[step] = (fStart + step*fStep)*2*3.1415;

    if (1) {
        if (!topopt.frequencyResponseDirect(nFrequencySteps,omega,deltaMax,phase,frequencyResponseCallBackCoreFunctions) ){
            writeWarningToGUIWindow("Could not solve frequency response; check for disconnected components, or try increasing mesh size");
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            resetProgressBar(" ");
            return false;
        }
    }
    else {
        solveStatic();
        liveVar.nEigenModes = 10;
        solveModal();
        if (!topopt.frequencyResponseModalSuperPosition(nFrequencySteps,omega,deltaMax,phase,frequencyResponseCallBackCoreFunctions) ){
            writeWarningToGUIWindow("Could not solve frequency response; check for disconnected components, or try increasing mesh size");
            liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
            resetProgressBar(" ");
            return false;
        }

    }

    topopt.saveFrequencyResponseData(nFrequencySteps,omega,deltaMax,phase);

    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    resetProgressBar("Freq Response complete!");
    liveVar.plotAxis = true;
    setStatusMessage("Freq Response  Complete");
    return true;
}
/************************************************************/
bool CoreFunctions::solveElastoPlastic(void)
{
    double timeForOneFEA;

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    if (!liveVar.STATIC_FEA_SOLVED){
        writeWarningToGUIWindow("Please solve linear static problem first");
        return false;
    }

    topopt.disableNodalSpMV();// don't use nodal SPMV

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.plotElasticityBC = false;
    writeMessageToGUIWindow("Starting Elasto-Plastic FEA");

    topopt.setCGMaxIter(20000);
    topopt.setCGRelativeResError(pow(10.0,-1.0*liveVar.cgResidualExponent));


#ifdef COMPILE_GPUCODE
    if (liveVar.useGPU) {
        topopt.enableGPU();
    }
    else
#endif
        topopt.disableGPU();

    topopt.computeKeOfTemplates(false);// don't use bubble functions!!

    topopt.disableDeflation();

    liveVar.nWorkSpaceGroups = getRecommendedDeflationGroups(liveVar.nElemsToMesh);
    if (liveVar.workSpaceMethod != DEFLATION_NONE)
        topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
    else
        topopt.disableDeflation();

    topopt.solveElasticityStatic();
    setStatusMessage("Solving Elasto-Plastic");
    timer.start();

    int nSteps = liveVar.nElastoPlasticForceSteps;

    if (!topopt.SolveElastoPlasticity(nSteps) ){
        writeWarningToGUIWindow("Could not solve elasto-plasticity");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    }

    timeForOneFEA = timer.getElapsedTimeInSec();
    liveVar.timeForOneFEA = timeForOneFEA;

    writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
    writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

    writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
    writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
    writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());


    writeMessageToGUIWindow("Pushing elasticity solution to tessellation ... ");
    topopt.pushInitialStaticSolutionToTessellation();
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;

    resetProgressBar("Elasto-Plastic complete!");
    liveVar.plotAxis = true;
    setStatusMessage("Elasto-Plastic Complete");
    return true;
}
/************************************************************/
bool CoreFunctions::solveModal(void)
{
    double eigRelativeAccuracy;

	if (!liveVar.STATIC_FEA_SOLVED) {
        writeWarningToGUIWindow("Please solve static structural before solving modal problem.");
		return false;

    }
    timer.start();
    //topopt.initializeSolution();
    allocateMemoryForEigenModes();

    writeMessageToGUIWindow("Computing Eigenmodes ");
    eigRelativeAccuracy = pow(10.0,-liveVar.eigRelAccuracyExponent);

    setStatusMessage("Solving Modal Analysis");
    bool success = topopt.solveElasticityModal(liveVar.nEigenModes,liveVar.eigenValues,liveVar.eigenModes,eigRelativeAccuracy,liveVar.maxTimeAllowed);

	if (!success) {
        writeWarningToGUIWindow("Error in Modal Analysis \n");
		return false;
	}
	 
    for (int i = 0; i < liveVar.nEigenModes; i++)
        writeMessageToGUIWindow("Eigenmode %d: %.4f (Hz)",i+1,liveVar.eigenValues[i]);

    writeMessageToGUIWindow("Time taken = %0.3f (s)\n",timer.getElapsedTimeInSec());

    topopt.populateEigen(liveVar.nEigenModes,liveVar.eigenValues,liveVar.eigenModes);
    writeMessageToGUIWindow("Pushing modal solution to tessellation");
    topopt.pushInitialModalSolutionToTessellation(liveVar.eigenModes[0]);//push first eigenmode

    liveVar.MODAL_FEA_SOLVED = true;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_MODAL;
    setStatusMessage("Modal Analysis Complete");
    return true;
}
/**********************************************************************/
void CoreFunctions::getMaxDeformationAndStressOverMesh(double *maxDeformation,double *maxStress)
{
    int whichNode;
    *maxDeformation = topopt.getDeformationMax();
    *maxStress = topopt.getMaxVonMisesStress(&whichNode);
}
/************************************************************/
void CoreFunctions::setTopOptConstraints(void)
{
    int smoothingIterations,symmetryIndex;

    topopt.resetTopOptConstraints();// remove all constraints

    symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
                        2*liveVar.topoptConstraints.ySymmetry
                    + 4*liveVar.topoptConstraints.zSymmetry;

    topopt.setSymmetryIndex(symmetryIndex);

    if (liveVar.topoptConstraints.isExtrudeActive)
        topopt.setExtrudeDirection(liveVar.topoptConstraints.extrudeDirection);

    if (liveVar.topoptConstraints.isAMBuildActive)
        topopt.setAMBuildDirection(liveVar.topoptConstraints.AMBuildDirection);

    if (liveVar.topoptConstraints.isDrawDirectionActive)
        topopt.setDrawDirection(liveVar.topoptConstraints.drawDirection);

    if (liveVar.topoptConstraints.isCyclicSymmetryActive)
        topopt.setAxiSymmetry(liveVar.topoptConstraints.cyclicSymmetry);

    if (liveVar.topoptConstraints.isxGridActive)
        topopt.setXGridPattern(liveVar.topoptConstraints.xGridPattern);

    if (liveVar.topoptConstraints.isyGridActive)
        topopt.setYGridPattern(liveVar.topoptConstraints.yGridPattern);

    if (liveVar.topoptConstraints.iszGridActive)
        topopt.setZGridPattern(liveVar.topoptConstraints.zGridPattern);

    smoothingIterations = 1;// default
    if (liveVar.topoptConstraints.isMinFeatureSizeActive) {
        double meshSize = topopt.getMeshElementSize();
        smoothingIterations = floor(liveVar.topoptConstraints.minFeatureSize/meshSize);
        if (smoothingIterations < 1){
            writeWarningToGUIWindow("Warning: Min feature size %g is smaller than mesh size %g",liveVar.topoptConstraints.minFeatureSize,meshSize);
            liveVar.topoptConstraints.minFeatureSize = 1.1*meshSize;
        }
        smoothingIterations = MIN(6,MAX(1,smoothingIterations));
    }
    topopt.setNumSmoothingOperations(smoothingIterations);

    if (liveVar.topoptConstraints.isMaxDisplacementActive)
        topopt.setMaxDisplacement(liveVar.topoptConstraints.maxDisplacementAllowed);

    if (liveVar.topoptConstraints.isStressSafetyActive)
        topopt.setStressSafetyFactor(liveVar.topoptConstraints.stressSafetyFactor);

    if (liveVar.topoptConstraints.isMinEigenActive)
        topopt.setEigenConstraint(liveVar.topoptConstraints.minEigenValueAllowedHz);

    if (liveVar.topoptConstraints.isTemperatureLimitActive)
        topopt.setMaxTemperatureLimit(liveVar.topoptConstraints.maxTemperatureAllowed);

    if (liveVar.topoptConstraints.isMaxFeatureSizeActive)
        topopt.setMaxFeatureSize(liveVar.topoptConstraints.maxFeatureSize);

    topopt.setKeepTopologyConnected(liveVar.topoptConstraints.keepTopologyConnected);
}
/************************************************************/
bool CoreFunctions::singleLoadThermalOptimize(void)
{
    int nFEA;
    double isoValueConverged,volDecrement,D0,S0,volDecrementFinal;
    double targetVolumeFraction,nFEAEstimated, currentVolFraction;
    double actualTime, maxDeformation,maxStress;
    char  tempStr[256];
    string resultFile;

    if (!liveVar.BC_THERMAL_APPLIED){
        writeWarningToGUIWindow("Thermal BC is undefined; please see specify BC");
        return false;
    }
    if (!solveThermoStatic()){
        writeWarningToGUIWindow("Could not solve thermal static FEA!");
        return false;
    }

    liveVar.timeForOneFEA = timer.getElapsedTimeInSec();

    liveVar.isTopOptComplete = false;
    initializeTopOptVariables();

    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    targetVolumeFraction =liveVar.desiredVolFraction;
    currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
    volDecrement = liveVar.stepSize;

    setTopOptConstraints();// reset

    topopt.setTopoptMode(TOPOPT_MODE_THERMAL);

    volDecrementFinal = volDecrement;
    setStatusMessage("Optimizing Topology");
    timer.start();
    nFEA = 0;
    liveVar.TOPOPT_SOLVED = true;//for live update

    liveVar.convergedVolFraction = topopt.TraceParetoCurve(targetVolumeFraction,volDecrement,&nFEA,&isoValueConverged,topOptCallBackFunction, &volDecrementFinal,liveVar.maxTimeAllowed);

    actualTime= timer.getElapsedTimeInSec();

    timer.stop();

    if (topopt.getCurrentIsoCounter() == -1){
        writeWarningToGUIWindow("Error: Could not carry out TopOpt, probably due to an incorrect constraint");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.plotElasticityBC = true;
        liveVar.TOPOPT_SOLVED = false;
        return false;
    }
    maxDeformation = topopt.getDeformationMax();
    maxStress = topopt.getMaxVonMisesStress();
    writeMessageToGUIWindow("---------------------------------------------");
    writeMessageToGUIWindow("Final volume fraction: %.2f",topopt.getCurrentVolumeFractionOfOptimizedParts());
    writeMessageToGUIWindow("Final Max Displacement: %e",maxDeformation);
    writeMessageToGUIWindow("Final Max Stress: %e",maxStress);
    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Actual # of FEAs: %d ",nFEA);

    char finalResultMessage[1000];
    sprintf(finalResultMessage,"-----------------------------------------------------------\n");
    sprintf(tempStr,"Final volume fraction = %.2f",liveVar.convergedVolFraction);
    strcat(finalResultMessage,tempStr);
    sprintf(tempStr,"\nNumber of FEAs: %d \n",nFEA);
    strcat(finalResultMessage,tempStr);
    printEstimatedTime(actualTime,tempStr);
    writeMessageToGUIWindow("%s \n",tempStr);

    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n");

    topopt.getLastTopoptErrorMessage(tempStr);
    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n-----------------------------------------------------------");
    writeMessageToGUIWindow(finalResultMessage);

    liveVar.plotAxis = 0;
    resultFile  = liveVar.projectDirectory + liveVar.modelName + string("_finalIsoBinary.stl");
    writeMessageToGUIWindow("Saving STL to %s ",resultFile.c_str());

    topopt.dumpIsoBinary(topopt.getCurrentIsoCounter(),resultFile.c_str());
    //writeMessageToGUIWindow("Pushing stress results to iso");
    //topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface

    writeMessageToGUIWindow("-----------------------------------------------------------");
    liveVar.plotAxis = 0;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.TOPOPT_SOLVED = true;
    liveVar.isTopOptComplete = true;

    resetProgressBar("Topopt complete!");
    setStatusMessage("Topology Optimization Complete");


    writeMessageToGUIWindow("Pushing temperature results to isosurface");
    topopt.pushFinalTemperatureToIsoSurface();
    return true;
}

/************************************************************/
bool CoreFunctions::singleLoadOptimize(void)
{
    int nFEA;
    double isoValueConverged,volDecrement,D0,S0,volDecrementFinal;
    double targetVolumeFraction,nFEAEstimated, currentVolFraction;
    double actualTime, maxDeformation,maxStress;
    char  tempStr[256];
    string resultFile;



    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("BC is undefined; please see specify BC");
        return false;
    }
    if (!verifyDirichletConditions()) {
        writeWarningToGUIWindow("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. ");
        return false;
    }

    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }
    setTopOptConstraints();// we need this here
    changeTopOptBC();


    timer.start();
    if (!solveStatic()){
        writeWarningToGUIWindow("Could not solve static FEA!");
        return false;
    }

    liveVar.timeForOneFEA = timer.getElapsedTimeInSec();
    // first solve eigen, else displacement field is overwritten
    if (topopt.isEigenConstraintActive()){
        writeMessageToGUIWindow("Eigenmode is active");
        timer.start();
        if (!solveModal()){
            writeWarningToGUIWindow("Could not solve modal FEA!");
            return false;
        }
        liveVar.timeForOneModalFEA = timer.getElapsedTimeInSec();
        double firstEigenValueHz = topopt.getFirstEigenValue();
        if (firstEigenValueHz < liveVar.topoptConstraints.minEigenValueAllowedHz){
            writeWarningToGUIWindow("Error: First mode %.2f (Hz) is less than min allowable %.2f (Hz).\nPlease check topopt constraints.",firstEigenValueHz,liveVar.topoptConstraints.minEigenValueAllowedHz);
            return false;
        }
    }

    liveVar.isTopOptComplete = false;
    initializeTopOptVariables();

    if (liveVar.includeThermalEffects){// for thermo-elasticity
        topopt.setTStoAnalytical(false);
        topopt.setTopoptMode(TOPOPT_MODE_THERMO_ELASTIC);
    }

    setTopOptConstraints();// reset

    D0 = topopt.getDeformationMax();
    S0 = topopt.getMaxVonMisesStress();

    nFEA = 1;

    topopt.setMaxDisplacement(liveVar.topoptConstraints.maxDisplacementAllowed);

    writeMessageToGUIWindow("Initial Max Displacement: %e",D0);
    writeMessageToGUIWindow("  Allowed Displacement: %e",liveVar.topoptConstraints.maxDisplacementAllowed);
    writeMessageToGUIWindow("Initial Max Stress: %e",S0);
    writeMessageToGUIWindow("  Yield Stress: %e",liveVar.Y );

    if (D0 > liveVar.topoptConstraints.maxDisplacementAllowed){
        writeWarningToGUIWindow("Error: Initial displacement %.2g exceeds allowable %.2g ",D0,liveVar.topoptConstraints.maxDisplacementAllowed);
        return false;
    }

    int partId;
    if (topopt.isDesignPastYield(&partId,1.0)){
        writeWarningToGUIWindow("Stress in part %d has exceed yield limit",partId);
        return false;
    }

    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    targetVolumeFraction =liveVar.desiredVolFraction;
    currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
    volDecrement = liveVar.stepSize;


    if (liveVar.topoptObjective == (TOPOPT_QUANTITY) VOLUME) {
        liveVar.topoptObjective = (TOPOPT_QUANTITY) COMPLIANCE;// default
        nFEAEstimated = 3*(currentVolFraction-targetVolumeFraction)/volDecrement;
    }
    else if (liveVar.topoptObjective == (TOPOPT_QUANTITY) COMPLIANCE) {
        writeMessageToGUIWindow("Objective: Maximize Stiffness");
        nFEAEstimated = 2*(currentVolFraction-targetVolumeFraction)/volDecrement;
    }
    else if (liveVar.topoptObjective == (TOPOPT_QUANTITY) VONMISES) {
        writeMessageToGUIWindow("Objective: Minimize Strength");
        nFEAEstimated = 4*(currentVolFraction-targetVolumeFraction)/volDecrement;
    }
    else if (liveVar.topoptObjective == (TOPOPT_QUANTITY) EIGENVALUE) {
        writeMessageToGUIWindow("Objective: Maximize Eigenvalue");
        nFEAEstimated = 4*(currentVolFraction-targetVolumeFraction)/volDecrement;
    }
    else {
        writeWarningToGUIWindow("Objective not known\n");
        return false;
    }
    nFEAEstimated = MAX(1,nFEAEstimated);
    topopt.setQuantityOfInterest((TOPOPT_QUANTITY) liveVar.topoptObjective);
	liveVar.estimatedTimeForTopOpt = 1.25*nFEAEstimated*liveVar.timeForOneFEA;
    if (topopt.isEigenConstraintActive()){
        liveVar.estimatedTimeForTopOpt += (nFEAEstimated/2)*liveVar.timeForOneModalFEA;
    }
    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Time for 1 FEA: %.3f (secs)",liveVar.timeForOneFEA);
    writeMessageToGUIWindow("Estimated # of FEAs: %.0f",nFEAEstimated);
    printEstimatedTime(liveVar.estimatedTimeForTopOpt,tempStr);
    writeMessageToGUIWindow("%s ",tempStr);


    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotAxis = false;
    volDecrementFinal = volDecrement;
    timer.stop();
    topopt.initializeSolution();

    setStatusMessage("Optimizing Topology");
    timer.start();
    nFEA = 0;
    liveVar.TOPOPT_SOLVED = true;//for live update

    liveVar.convergedVolFraction = topopt.TraceParetoCurve(targetVolumeFraction,volDecrement,&nFEA,&isoValueConverged,topOptCallBackFunction, &volDecrementFinal,liveVar.maxTimeAllowed);

    actualTime= timer.getElapsedTimeInSec();

    timer.stop();

    if (topopt.getCurrentIsoCounter() == -1){
        writeWarningToGUIWindow("Error: Could not carry out TopOpt, probably due to an incorrect constraint");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.plotElasticityBC = true;
        liveVar.TOPOPT_SOLVED = false;
        return false;
    }
    maxDeformation = topopt.getDeformationMax();
    maxStress = topopt.getMaxVonMisesStress();
    writeMessageToGUIWindow("---------------------------------------------");
    writeMessageToGUIWindow("Final volume fraction: %.2f",topopt.getCurrentVolumeFractionOfOptimizedParts());
    writeMessageToGUIWindow("Final Max Displacement: %e",maxDeformation);
    writeMessageToGUIWindow("Final Max Stress: %e",maxStress);
    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Actual # of FEAs: %d ",nFEA);

    char finalResultMessage[1000];
    sprintf(finalResultMessage,"-----------------------------------------------------------\n");
    sprintf(tempStr,"Final volume fraction = %.2f",liveVar.convergedVolFraction);
    strcat(finalResultMessage,tempStr);
    sprintf(tempStr,"\nNumber of FEAs: %d \n",nFEA);
    strcat(finalResultMessage,tempStr);
    printEstimatedTime(actualTime,tempStr);
    writeMessageToGUIWindow("%s \n",tempStr);

    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n");

    topopt.getLastTopoptErrorMessage(tempStr);
    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n-----------------------------------------------------------");
    writeMessageToGUIWindow(finalResultMessage);

    liveVar.plotAxis = 0;
    resultFile  = liveVar.projectDirectory + liveVar.modelName + string("_finalIsoBinary.stl");
    writeMessageToGUIWindow("Saving STL to %s ",resultFile.c_str());

    topopt.dumpIsoBinary(topopt.getCurrentIsoCounter(),resultFile.c_str());
    writeMessageToGUIWindow("Pushing stress results to iso");
    topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface
    if (liveVar.includeThermalEffects)// for thermo-elasticity
        topopt.pushFinalTemperatureToIsoSurface();


    if (topopt.isEigenConstraintActive()){
        topopt.pushFinalModalSolutionToIsoSurface();// modal solution
    }
    writeMessageToGUIWindow("-----------------------------------------------------------");
    liveVar.plotAxis = 0;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.TOPOPT_SOLVED = true;
    liveVar.isTopOptComplete = true;

    resetProgressBar("Topopt complete!");
    setStatusMessage("Topology Optimization Complete");

    string  voxFile  = liveVar.projectDirectory + liveVar.modelName + string("_mesh.msh");

    topopt.dumpParetoMesh(voxFile.c_str());
    return true;
}
/************************************************************/
bool CoreFunctions::singleLoadOptimizeSIMP(void)
{
    int nFEA;
    double volDecrement,D0,S0,volDecrementFinal;
    double targetVolumeFraction;
    double actualTime, maxDeformation,maxStress;
    char  tempStr[256];
    string resultFile;

    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("BC is undefined; please see specify BC");
        return false;
    }
    if (!verifyDirichletConditions()) {
        writeWarningToGUIWindow("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. ");
        return false;
    }

    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }

    setTopOptConstraints();// we need this here
    changeTopOptBC();

    timer.start();
    if (!solveStatic()){
        writeWarningToGUIWindow("Could not solve static FEA!");
        return false;
    }
    initializeTopOptVariables();
    setTopOptConstraints();// reset

    liveVar.timeForOneFEA = timer.getElapsedTimeInSec();

    liveVar.isTopOptComplete = false;

    D0 = topopt.getDeformationMax();
    S0 = topopt.getMaxVonMisesStress();

    nFEA = 1;

    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    targetVolumeFraction =liveVar.desiredVolFraction;

    topopt.setSIMPTargetVolumeFraction(targetVolumeFraction);

    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    volDecrementFinal = volDecrement;
    timer.stop();
    topopt.initializeSolution();

    setStatusMessage("Optimizing Topology");
    timer.start();
    nFEA = 0;
    liveVar.TOPOPT_SOLVED = true;//for live update
    topopt.solveSIMPPareto(topOptCallBackFunctionSIMP);
    actualTime= timer.getElapsedTimeInSec();
    timer.stop();
    maxDeformation = topopt.getDeformationMax();
    maxStress = topopt.getMaxVonMisesStress();
    writeMessageToGUIWindow("---------------------------------------------");
    writeMessageToGUIWindow("Final volume fraction: %.2f",topopt.getCurrentVolumeFractionOfOptimizedParts());
    writeMessageToGUIWindow("Final Max Displacement: %e",maxDeformation);
    writeMessageToGUIWindow("Final Max Stress: %e",maxStress);
    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Actual # of FEAs: %d ",nFEA);

    char finalResultMessage[1000];
    sprintf(finalResultMessage,"-----------------------------------------------------------\n");
    sprintf(tempStr,"Final volume fraction = %.2f",liveVar.convergedVolFraction);
    strcat(finalResultMessage,tempStr);
    sprintf(tempStr,"\nNumber of FEAs: %d \n",nFEA);
    strcat(finalResultMessage,tempStr);
    printEstimatedTime(actualTime,tempStr);
    writeMessageToGUIWindow("%s \n",tempStr);

    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n");


    liveVar.plotAxis = 0;
    resultFile  = liveVar.projectDirectory + liveVar.modelName + string("_finalIsoBinary.stl");
    writeMessageToGUIWindow("Saving STL to %s ",resultFile.c_str());


    liveVar.plotAxis = 0;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.TOPOPT_SOLVED = true;
    liveVar.isTopOptComplete = true;

    resetProgressBar("Topopt complete!");
    setStatusMessage("Topology Optimization Complete");


    return true; 
}
/************************************************************/
void CoreFunctions::dumpPointCloud(string fileName)
{
    int savedCounter,elem,nCADPts;
    double actualVolumeFraction,isoCutoff, timeInSecs;
    double triNormal[3],meshSize,meshPt[3];
    bool *isElemOnCADBoundary;
    vector<Point3d> points,normals;
    Point3d pt,normal;
    FILE *fptr2;

    // 1. Generate point cloud on CAD model
    // 2. Select CAD points that are close to active voxel elements
    //    Mark the corresponding voxel as CAD-voxel
    // 3. Select isosurface points that are not in CAD-voxel

    meshSize = topopt.getMeshElementSize();
    points.clear();
    normals.clear();

    // generate on surface by walking on boundary nodes of voxel mesh
    isElemOnCADBoundary = (bool *)malloc(topopt.getNumElems()*sizeof(bool));
    memset(isElemOnCADBoundary,false,topopt.getNumElems()*sizeof(bool));
    for (int node = 0; node < topopt.getNumNodes(); node++){
        if (!topopt.isNodeOnBoundary(node)) continue;
        topopt.getNodeXYZ(node,meshPt);
        vox.snapPointToGeometry(meshPt,1.9*meshSize);//find the new  xyz location after snapping to geometry
        elem = topopt.findNonVoidElemClosestTo(meshPt[0],meshPt[1],meshPt[2],1.25);
        if (elem < 0) continue;
        isElemOnCADBoundary[elem] = true;
        vox.getNormalAtPoint(meshPt,triNormal);
        pt.x = meshPt[0];
        pt.y = meshPt[1];
        pt.z = meshPt[2];
        normal.x = triNormal[0];
        normal.y = triNormal[1];
        normal.z = triNormal[2];
        points.push_back(pt);
        normals.push_back(normal);
    }

    nCADPts= points.size();

    savedCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction,&actualVolumeFraction,&isoCutoff, &timeInSecs);
    // now cherry pick points from isosurface
    for (int node = 0; node < topopt.getNumIsoNodes(savedCounter); node++){
        topopt.getIsoNodeXYZ(savedCounter,node,meshPt);
        elem = topopt.findNonVoidElemClosestTo(meshPt[0],meshPt[1],meshPt[2],1.5);
        if (elem < 0) continue;
        if (isElemOnCADBoundary[elem]) continue;
        if (!topopt.isElemInInterior(elem)) continue;
        topopt.getIsoNodeNormal(savedCounter,node,triNormal);

        pt.x = meshPt[0];
        pt.y = meshPt[1];
        pt.z = meshPt[2];
        normal.x = triNormal[0];
        normal.y = triNormal[1];
        normal.z = triNormal[2];
        points.push_back(pt);
        normals.push_back(normal);
    }

    vector<Point3d> uniquePoints,uniqueNormals;
    bool duplicate;
    double d;
    uniquePoints.clear();
    uniqueNormals.clear();
    for (int i = 0; i < points.size(); i++){
        duplicate = false;
        for (int j = 0; j < uniquePoints.size(); j++){
            d = sqrt(pow(points[i].x -uniquePoints[j].x,2)+
                     pow(points[i].y -uniquePoints[j].y,2)+
                     pow(points[i].z -uniquePoints[j].z,2));
            if (d < meshSize*0.9){
              duplicate = true;
              break;
            }
        }
        if (!duplicate){
            uniquePoints.push_back(points[i]);
            uniqueNormals.push_back(normals[i]);
        }
    }

    fptr2 = fopen(fileName.c_str(),"w");
    fprintf(fptr2,"%d \n",uniquePoints.size());
    for (int i = 0; i < uniquePoints.size(); i++){
         fprintf(fptr2,"%g %g %g ",uniquePoints[i].x,uniquePoints[i].y,uniquePoints[i].z);
         fprintf(fptr2,"%f %f %f ",uniqueNormals[i].x,uniqueNormals[i].y,uniqueNormals[i].z);
         if (i < nCADPts)
             fprintf(fptr2,"1 \n");
         else
             fprintf(fptr2,"0 \n");
    }

    points.clear();
    uniquePoints.clear();
    normals.clear();
    uniqueNormals.clear();
    fclose(fptr2);
    free(isElemOnCADBoundary);
}
/************************************************************/
void CoreFunctions::setTopOptConstraints_Multimaterial(void)
{
    int symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
            2*liveVar.topoptConstraints.ySymmetry
        + 4*liveVar.topoptConstraints.zSymmetry;

    topopt.setAxiSymmetry(liveVar.topoptConstraints.cyclicSymmetry);
    topopt.setStressSafetyFactor(liveVar.topoptConstraints.stressSafetyFactor);
    topopt.setSymmetryIndex(symmetryIndex);
    topopt.setDrawDirection(liveVar.topoptConstraints.drawDirection);
    topopt.setExtrudeDirection(liveVar.topoptConstraints.extrudeDirection);
}
/************************************************************/
bool CoreFunctions::singleLoadOptimize_MultiMaterial(void)
{
    int nFEA,symmetryIndex;
    double convergedCostFrac,CostDecrement,D0,S0,CostDecrementFinal;
    double targetCostFraction,timeForOneFEA,nFEAEstimated, currentCostFraction, estimatedTimeForCompletion;
    double hours, minutes, seconds,actualTime, maxDeformation;

    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("BC is undefined; please see specify BC");
        return false;
    }
    if (!verifyDirichletConditions()) {
        writeWarningToGUIWindow("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. ");
        return false;
    }

    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }

    changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED))
        createTopOptMesh();

    initializeTopOptVariables();

    topopt.enableMultiMaterialVerbose();
    topopt.initializeTopOptForMultiMaterial();

    Timer timer;
    Timer globalTimer;

    timer.stop();
    timer.start();

    topopt.deleteStaticFEAResults();
    topopt.solvePrimaryAndAdjointProblems_MultiMaterial(&nFEA);
    timeForOneFEA = timer.getElapsedTimeInSec();

    D0 = topopt.getDeformationMax();

    writeMessageToParetoLog("Initial Max Displacement: %e\n",D0);
    writeMessageToParetoLog("Allowed Displacement: %e\n",liveVar.topoptConstraints.maxDisplacementAllowed);

    targetCostFraction = liveVar.desiredVolFraction;
    currentCostFraction = topopt.getCurrentVolumeFraction();
    CostDecrement = liveVar.stepSize;


    nFEAEstimated = 3*(currentCostFraction-targetCostFraction)/CostDecrement;

    estimatedTimeForCompletion = nFEAEstimated*timeForOneFEA;
    hours = floor(estimatedTimeForCompletion/3600);
    estimatedTimeForCompletion -= hours*3600;
    minutes = floor(estimatedTimeForCompletion/60);
    estimatedTimeForCompletion -= minutes*60;
    seconds =(estimatedTimeForCompletion);
    writeMessageToParetoLog("-----------------------------------------------------------\n");
    writeMessageToParetoLog("Time for 1 FEA: %.3f (secs)\n",timeForOneFEA);
    writeMessageToParetoLog("Estimated # of FEAs: %.0f \n",nFEAEstimated);
    writeMessageToParetoLog("Estimated time: %.0f hour(s), %.0f minute(s), %.2f second(s)\n",hours,minutes,seconds);
    writeMessageToParetoLog("-----------------------------------------------------------\n");

    writeMessageToParetoLog("Pushing elasticity solution to tessellation ... ");
    topopt.pushInitialStaticSolutionToTessellation();

    liveVar.timeForOneFEA = timeForOneFEA;
    liveVar.STATIC_FEA_SOLVED = true;
    liveVar.whichLoadSetElasticityFEASolved = liveVar.currentBCSet;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;

    resetProgressBar("Static FEA complete!");
    liveVar.plotAxis = 1;
    //Set plotting routine mesh
    setStatusMessage("Static FEA Complete");

    liveVar.displayVolFraction = topopt.getCurrentCostFraction_MultiMaterial();

    CostDecrementFinal = CostDecrement;
    timer.stop();
    topopt.initializeSolution();

    globalTimer.start();
    nFEA = 0;
    symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
                        2*liveVar.topoptConstraints.ySymmetry
                    + 4*liveVar.topoptConstraints.zSymmetry;

    setTopOptConstraints_Multimaterial();

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;


    liveVar.TOPOPT_SOLVED = true;// for live update

    convergedCostFrac = topopt.TraceParetoForMultiMaterial(targetCostFraction,
        CostDecrement,liveVar.maxFEA,&nFEA,topOptMultiMaterialCallBackFunction,
        liveVar.topoptConstraints.drawDirection,symmetryIndex,&CostDecrementFinal,liveVar.maxTimeAllowed);


    actualTime= globalTimer.getElapsedTimeInSec();
    globalTimer.stop();

    cout <<"---------------------------------------------" << endl;
    cout << "Final Cost fraction: " << topopt.getCurrentCostFraction_MultiMaterial() << endl;

    liveVar.displayVolFraction = topopt.getCurrentCostFraction_MultiMaterial();


    hours = floor(actualTime/3600);
    actualTime -= hours*3600;
    minutes = floor(actualTime/60);
    actualTime -= minutes*60;
    seconds =(actualTime);

    liveVar.plotAxis = 0;

    liveVar.TOPOPT_SOLVED = true;
    liveVar.isTopOptComplete = true;

    resetProgressBar("MultiMaterial Topopt complete!");
    setStatusMessage("MultiMaterial Topology Optimization Complete");
    return true;
}
/************************************************************/
void CoreFunctions::retainSurfacesForMultiload(void)
{
    double bcValue;
    int dirichlet;

    if (ALLOW_INDEPENDENT_RESTRAINTS) {
        for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++){
            for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
                for (int i = 0; i < 3; i++){
                    dirichlet = (liveVar.BCType[bcSet][faceGroup][i] == 0);
                    if (dirichlet) {
                        topopt.retainNodesWithLabel(faceGroup);
                    }
                }
            }
        }
    }

    // in multi-load problems, we must also retain nodes where forces are applied on the loads, else they will be deleted
    for (int bcSet = 0; bcSet <liveVar.nBCSet; bcSet++) {
        for (int faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
            bcValue = 0;
            for (int i = 0; i < 3; i++)
                bcValue += fabs(liveVar.BCValue[bcSet][faceGroup][i]);
            if ((liveVar.BCType[bcSet][faceGroup][0] == ELASTICITY_NEUMANN) && (bcValue > 0)){// this is a force face
                topopt.retainNodesWithLabel(faceGroup);
            }
        }
    }
    liveVar.forceRemesh = false;
    topopt.InitializeTopOpt(1);
}
/************************************************************/
bool CoreFunctions::multiLoadOptimize(void)
{
    int bcSet,i,faceGroup,activeGroups[STL_MAX_GROUPS], nFEA;
    double D0,S0;
    bool dirichlet1, dirichlet2;
    double targetVolumeFraction,volDecrement,nFEAEstimated, currentVolFraction;
    double hours, minutes, seconds;
	char tempStr[256];


    if (!liveVar.BC_STRUCTURAL_APPLIED){
        writeWarningToGUIWindow("BC is undefined; please see specify BC");
        return false;
    }
    if (!checkForFiniteForce()){
        writeWarningToGUIWindow("No force has been applied on the geometry");
        //return false;
    }
    changeTopOptBC();
    if ((liveVar.forceRemesh) || (!liveVar.MESH_APPLIED ))
        createTopOptMesh();

    if (!verifyDirichletConditions()){
        writeMessageToGUIWindow("Warning: Geometry is not fixed in all three directions.\nThe displacements may be incorrect, and FEA will take longer.\n ");
    }

    if (topopt.getForceNorm() == 0) {
        writeWarningToGUIWindow("Error: Zero force detected");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        return false;
    }
    if (!verifySymmetryConditions()){
        writeWarningToGUIWindow("Error: Incorrect symmetry being imposed. Please verify. ");
        return false;
    }


    liveVar.isTopOptComplete = false;
	if (!verifyDirichletConditions()) {
        writeWarningToGUIWindow("Warning: The geometry is not fixed in all three directions.\nThe displacements may be incorrect. ");
		return false;
	}
    vox.getActiveGroups(activeGroups);
    // Check if all BCTypes are same
    if (!ALLOW_INDEPENDENT_RESTRAINTS) {
        for (bcSet = 1; bcSet < liveVar.nBCSet; bcSet++){
            for ( faceGroup = 0; faceGroup < vox.getNumTriGroups(); faceGroup++) {
                if (!activeGroups[faceGroup]) continue;
                for (i = 0; i < 3; i++){
                    dirichlet1 = (liveVar.BCType[0][faceGroup][i] == 0);
                    dirichlet2 = (liveVar.BCType[bcSet][faceGroup][i] == 0);
                    if ( (dirichlet1 && !dirichlet2) || (!dirichlet1 && dirichlet2) ) {
                        writeWarningToGUIWindow("Error in multiload boundary conditions. All bcSets must share the same dirichlet conditions");
                        return false;
                    }
                }
            }
        }
    }
    retainSurfacesForMultiload();

    liveVar.timeForOneFEA = 0;

    for (int bcSet = 0; bcSet < liveVar.nBCSet; bcSet++){
        writeMessageToGUIWindow("Solving FEA for BC Set = %d",bcSet);
        liveVar.currentBCSet = bcSet;
        changeTopOptBC();
        timer.start();
        if (ALLOW_INDEPENDENT_RESTRAINTS)
            liveVar.forceRemesh = true;
        if (!solveStatic()){
            writeWarningToGUIWindow("Error: Could not solve FEA");
            return false;
        }


        liveVar.timeForOneFEA += timer.getElapsedTimeInSec();

        getMaxDeformationAndStressOverMesh(&D0,&S0);

        writeMessageToGUIWindow("Initial Max Displacement: %e",D0);
        writeMessageToGUIWindow("Allowed Displacement: %e\n",liveVar.topoptConstraints.maxDisplacementAllowed);
        writeMessageToGUIWindow("Initial Max Stress: %e",S0);
        writeMessageToGUIWindow("Allowed Stress: %e",liveVar.Y );


        if (S0 > liveVar.Y/liveVar.topoptConstraints.stressSafetyFactor){
            writeWarningToGUIWindow("Error: Initial stress %.2g exceeds allowable %.2g ",S0,liveVar.Y);
            return false;
        }

    }

    initializeTopOptVariables();
    topopt.initializeSolution();
    topopt.resetIsoCounter();


    liveVar.timeForOneFEA /= liveVar.nBCSet;

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.currentBCSet = 0;


    volDecrement = liveVar.stepSize;
    targetVolumeFraction = liveVar.desiredVolFraction;
    currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();


    nFEAEstimated = 6*liveVar.nBCSet*(currentVolFraction-targetVolumeFraction)/volDecrement;
    nFEAEstimated = MAX(1,nFEAEstimated);

	liveVar.estimatedTimeForTopOpt = nFEAEstimated*liveVar.timeForOneFEA;
    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Time for 1 FEA: %.3f (secs)", liveVar.timeForOneFEA);
    writeMessageToGUIWindow("Estimated # of FEAs: %.0f", nFEAEstimated);
	printEstimatedTime(liveVar.estimatedTimeForTopOpt, tempStr);
    writeMessageToGUIWindow("%s ", tempStr);


    double isoValueConverged,volDecrementFinal;
    writeMessageToGUIWindow("Multiload method: Augmented Lagrangian Method ");


    targetVolumeFraction = liveVar.desiredVolFraction;
    currentVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();
    volDecrement = liveVar.stepSize;

    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    volDecrementFinal = volDecrement;
    topopt.initializeSolution();

    writeMessageToGUIWindow("maxDispAllowed: %.2e, maxStressAllowed = %.2e\n",liveVar.topoptConstraints.maxDisplacementAllowed,liveVar.Y);

    topopt.setMaxDisplacement(liveVar.topoptConstraints.maxDisplacementAllowed);

    timer.stop();
    timer.start();
    nFEA = 0;

    for (int bcSet = 0;bcSet < liveVar.nBCSet; bcSet++) {
        if (liveVar.NumberOfConstraints[bcSet] == 0) {// no explicit constraints imposed for this bcSet. Apply standard displacement and stress constraints
            liveVar.ConstraintType[bcSet][0] = 1;// compliance
            liveVar.ConstraintData[bcSet][0][0] = 1;// it is an upper limit
            liveVar.ConstraintData[bcSet][0][1] = 1;// hard constraint
            liveVar.ConstraintData[bcSet][0][2] = 1;// not used
            liveVar.NumberOfConstraints[bcSet] =1;

            /*
            ConstraintType[bcSet][1] = 2;// stress
            ConstraintData[bcSet][1][0] = 1;// it is an upper limit
            ConstraintData[bcSet][1][1] = 1;// hard constraint
            ConstraintData[bcSet][1][2] = 1;// not used
            NumberOfConstraints[bcSet] =2;
            */

            if (liveVar.NumberOfConstraints[bcSet] > MAX_CONSTRAINTS){
                writeMessageToGUIWindow("Number of constraints exceeds allowable");
                return false;
            }
        }
    }

 
    liveVar.topoptObjective = (TOPOPT_QUANTITY) VOLUME;// for multi objective problems, always minimize volume


    setTopOptConstraints();
    resetProgressBar("");
    setStatusMessage("Optimizing Topology");

    liveVar.TOPOPT_SOLVED = true;//for live update

    liveVar.convergedVolFraction = topopt.TraceParetoCurve_MLMC(targetVolumeFraction,volDecrement,&nFEA,
                                                                &isoValueConverged,topOptCallBackFunction,&volDecrementFinal,
                                                                liveVar.ConstraintType,liveVar.NumberOfConstraints,liveVar.ConstraintData,liveVar.topoptObjective,liveVar.nBCSet,liveVar.BCType, liveVar.BCValue,
                                                                vox.getNumTriGroups(), liveVar.activeBCSets);

    double actualTime,tempDouble;
    actualTime= timer.getElapsedTimeInSec();

    tempDouble = actualTime;
    timer.stop();

    if (topopt.getCurrentIsoCounter() == -1){
        writeWarningToGUIWindow("Error: Could not carry out TopOpt, probably due to an incorrect constraint");
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.plotElasticityBC = true;
        liveVar.TOPOPT_SOLVED = false;
        return false;
    }

    hours = floor(tempDouble/3600);
    tempDouble -= hours*3600;
    minutes = floor(tempDouble/60);
    tempDouble -= minutes*60;
    seconds =(tempDouble);
    writeMessageToGUIWindow("-----------------------------------------------------------");
    writeMessageToGUIWindow("Actual # of FEAs: %d ",nFEA);
    writeMessageToGUIWindow("Actual time: %.0f hour(s), %.0f minute(s), %.2f second(s)",hours,minutes,seconds);
    writeMessageToGUIWindow("Final volume fraction: %.2f\n",topopt.getCurrentVolumeFractionOfOptimizedParts());

    liveVar.displayVolFraction = topopt.getCurrentVolumeFractionOfOptimizedParts();

    string resultFile = liveVar.projectDirectory + liveVar.modelName + string("_finalIsoBinary.stl");
    writeMessageToGUIWindow("Saving STL to %s ",resultFile.c_str());
    topopt.dumpIsoBinary(topopt.getCurrentIsoCounter(),resultFile.c_str());

    writeMessageToGUIWindow("Pushing stress results to iso");
    topopt.pushFinalStaticSolutionToIsoSurface();// the elasticity solution to final iso surface


    char finalResultMessage[1000];
    sprintf(finalResultMessage,"Optimization completed.\nFinal volume fraction = %.2f\n",liveVar.convergedVolFraction);
    sprintf(tempStr,"Number of FEAs: %d \n",nFEA);
    strcat(finalResultMessage,tempStr);
    printEstimatedTime(actualTime,tempStr);
    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n");
    writeMessageToGUIWindow("%s ",tempStr);

    topopt.getLastTopoptErrorMessage(tempStr);
    strcat(finalResultMessage,tempStr);
    strcat(finalResultMessage,"\n----------------------------------------------");
    writeMessageToGUIWindow("%s",finalResultMessage);
    
    liveVar.TOPOPT_SOLVED = true;
    liveVar.plotAxis = 0;
    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    liveVar.isTopOptComplete = true;

    resetProgressBar("Topopt complete!");
    setStatusMessage("Topology Optimization Complete");
    return true;
}
/************************************************************/
void CoreFunctions::loadPRJ(const char *extension)
// Load a  project
{
    int  bcSet,tri,group,tempInt,tempInt2,tempInt3;
    char str[256],tempStr[256];
    double temp,projectVersion;
    char xyzFile[256],materialName[256];
    char outputFile[256];
    double xietamu[3],x,y,z,vonmises,delta;
    int nPoints,voxelElem,ptNumber,voxelElemPrev,currentPartId;
    double pt[3],uvw[3],stresses[6],materialProperties[7],materialPrice = 1;

    bool loadProjectVerbose = false;
    bool assemblyMode = false;


    setStatusMessage("Loading Project");
    oneTimeInitialization();
    assemblyMode = false;
    liveVar.assemblyMode = false;
    writeMessageToGUIWindow("Loading project file %s ",liveVar.prjFileName.c_str() );

    liveVar.modelName = liveVar.prjFileName;
    liveVar.projectDirectory = liveVar.prjFileName;

    size_t npos = liveVar.modelName.rfind('/');
    liveVar.modelName = liveVar.modelName.substr(npos+1);
    liveVar.modelName = liveVar.modelName.substr(0,liveVar.modelName.length() - 4);// remove .prj


    liveVar.projectDirectory = liveVar.projectDirectory.substr(0,npos+1);

    writeMessageToGUIWindow("Model name: %s",liveVar.modelName.c_str());
    writeMessageToGUIWindow("Project directory: %s",liveVar.projectDirectory.c_str());
    writeMessageToGUIWindow("---------------------------------------------");

    //    double boundingBox[3][2]; // example
    //boundingBox[0][0] = 0.0330; boundingBox[0][1] = 0.1940;
    //boundingBox[1][0] = 0.0270; boundingBox[1][1] = 0.2190;
    //boundingBox[2][0] = 0; boundingBox[2][1] = 0.1500;
    //vox.createBoundingBoxSTLFile(boundingBox,"BrainBox.stl");
    FILE *fptr = fopen(liveVar.prjFileName.c_str(),"r");

    if (fptr == NULL){
        writeMessageToGUIWindow("Project file %s not found\n",liveVar.prjFileName.c_str() );
        writeMessageToGUIWindow("---------------------------------------------");
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
    strcpy(materialName,"UnknownMaterial");
    while (find_word(fptr,"#KEY")) {
        fscanf(fptr,"%s",str);
        //cout << "Key: " << str << endl;
        if (strcmp(str,"VERSION")==0){
            version_found = true;
            fscanf(fptr,"%lf",&(projectVersion));
            if (loadProjectVerbose) writeMessageToGUIWindow("Project version is %.2f ",projectVersion);
            if ((liveVar.usingGUI) && (projectVersion < PARETO_VERSION)) {
                writeWarningToGUIWindow("Project is is an older version. \nPlease save project  \n");
            }
        }
        else if (strcmp(str,"USERNAME")==0){
            fscanf(fptr,"%s",&(liveVar.userName));
        }
        else if (strcmp(str,"ASSEMBLY")==0){
            fscanf(fptr,"%d",&(assemblyMode));
        }
        else if (strcmp(str,"HEXMESH_VERBOSE")==0){
            fscanf(fptr,"%d",&(tempInt));
            if (tempInt) topopt.enableHexmeshVerbose();
            else topopt.disableHexmeshVerbose();
        }
        else if (strcmp(str,"GEOMETRY")==0){
            if (!version_found){
                writeWarningToGUIWindow("Project may be of an older version. \nPlease read manual for possible format changes\nIt is recommended that you recreate project file from GUI");
				break;
			}
            fscanf(fptr,"%s",tempStr);
            if (strstr(tempStr,".STL") ||  strstr(tempStr,".stl")  ){
                writeMessageToGUIWindow("Opening file %s in directory %s; part = %d ",tempStr,liveVar.projectDirectory.c_str(),liveVar.nSTLFilesLoaded);
                liveVar.geomFileName[liveVar.nSTLFilesLoaded]  = liveVar.projectDirectory  + string(tempStr);
                if (!loadGeom(false)){
                    writeWarningToGUIWindow("Load Project: Could not load stl file %s ",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
                    break;
                }
                liveVar.assemblyMode = assemblyMode;//only update after the first geometry is read.
            }
            else if (strstr(tempStr,".OFF") ||  strstr(tempStr,".off")  ){
                writeMessageToGUIWindow("Opening file %s in directory %s; part = %d ",tempStr,liveVar.projectDirectory.c_str(),liveVar.nSTLFilesLoaded);
                liveVar.geomFileName[liveVar.nSTLFilesLoaded]  = liveVar.projectDirectory  + string(tempStr);
                if (!loadGeom(false)){
                    writeWarningToGUIWindow("Load Project: Could not load off file %s ",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
                    break;
                }
                liveVar.assemblyMode = assemblyMode;//only update after the first geometry is read.
            }
            else {
                writeWarningToGUIWindow(" Geometry file %s does not seem to have .stl or .off extension\n Make sure the file name does not have a space in it ",tempStr);
                break;
            }
        }
        else if (strcmp(str,"UPDATE_GEOM")==0){// update the geometry and transform the boundary conditions
            fscanf(fptr,"%s",tempStr);
            if (strstr(tempStr,".STL") ||  strstr(tempStr,".stl")  ){
                if (loadProjectVerbose) writeMessageToGUIWindow("Opening file %s in directory %s part = %d \n",tempStr,liveVar.projectDirectory.c_str(),liveVar.nSTLFilesLoaded);
                liveVar.geomFileName[liveVar.nSTLFilesLoaded]  = liveVar.projectDirectory  + string(tempStr);
                if (!updateGeom(false)){
                    writeWarningToGUIWindow("Load Project: Could not update stl file %s \n",liveVar.geomFileName[liveVar.nSTLFilesLoaded].c_str());
                    break;
                }
            }
            else {
                writeMessageToGUIWindow(" Geometry file %s does not seem to have .stl extension ",tempStr);
                writeMessageToGUIWindow(" Make sure the file name does not have a space in it ");
                break;
            }
        }
        else if (strcmp(str,"PART")==0){
            fscanf(fptr,"%d ",&(currentPartId));// used below in Material
        }
        else if (strcmp(str,"MATERIALNAME")==0){
            fscanf(fptr,"%s ",materialName);//used below, assumes the order of materialName first
            liveVar.materialName = string(materialName);
        }
        else if (strcmp(str,"MATERIAL")==0){ //older project
            fscanf(fptr,"%e %e %e %e %e %e",&(liveVar.E),&(liveVar.nu),&(liveVar.rho),&(liveVar.K), &(liveVar.alpha), &(liveVar.specificHeat));
            topopt.setIsotropicMaterialOfPart(currentPartId,liveVar.E,liveVar.nu,liveVar.rho,liveVar.K,liveVar.alpha,liveVar.specificHeat,materialName);
            liveVar.Y = 0.001*liveVar.E;// best guess
        }
        else if (strcmp(str,"E")==0){
            fscanf(fptr,"%le ",&(materialProperties[0]));
            liveVar.E = materialProperties[0];//this will get overwritten for multiple bodies
        }
        else if (strcmp(str,"nu")==0){
            fscanf(fptr,"%le ",&(materialProperties[1]));
            liveVar.nu = materialProperties[1];//this will get overwritten for multiple bodies
        }
        else if (strcmp(str,"Y")==0){
            fscanf(fptr,"%le ",&(materialProperties[2]));
            liveVar.Y = materialProperties[2];//this will get overwritten for multiple bodies
        }
        else if (strcmp(str,"rho")==0){
            fscanf(fptr,"%le ",&(materialProperties[3]));
            liveVar.rho = materialProperties[3];
        }
        else if (strcmp(str,"K")==0){
            fscanf(fptr,"%le ",&(materialProperties[4]));
            liveVar.K = materialProperties[4];
        }
        else if (strcmp(str,"alpha")==0){
            fscanf(fptr,"%le ",&(materialProperties[5]));
            liveVar.alpha = materialProperties[5];
        }
        else if (strcmp(str,"Cp")==0){
            fscanf(fptr,"%le ",&(materialProperties[6]));
            liveVar.specificHeat = materialProperties[6];
        }
        else if (strcmp(str,"PRICE")==0){
            fscanf(fptr,"%le ",&(materialPrice));
            liveVar.price = materialPrice;
        }
        else if (strcmp(str,"MATERIAL_END")==0){
            if (liveVar.paretoUnits == UNITS_MM){
                // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
                // but this lead to inconsistency in units; we fix this here!
                materialProperties[3] /= 1000;
            }
            if (liveVar.paretoUnits == UNITS_INCH){
                // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
                // but this lead to inconsistency in units; we fix this here!
                materialProperties[3] /= 386.22;
            }
            topopt.setIsotropicMaterialPropertiesOfPart(currentPartId,materialProperties,materialName,materialPrice);
            bool materialSelectionDone;;
            materialSelectionDone = true;
            for (int part = 0; part < vox.getNumParts(); part++) {
                if (topopt.getMaterialIDofPart(part) < 0)
                    materialSelectionDone = false;
            }
            if (materialSelectionDone)
                liveVar.MATERIAL_APPLIED = true;
        }
        else if (strcmp(str,"NONDESIGN")==0){
            fscanf(fptr,"%d ",&(tempInt));
            if (tempInt == 1) topopt.setPartToNonDesign(currentPartId);
        }
        else if (strcmp(str,"UNITS")==0){
            fscanf(fptr,"%d ",&(liveVar.paretoUnits));
            liveVar.UNITS_APPLIED = true;
        }
        else if (strcmp(str,"TEMPERATURE_UNITS")==0){
            fscanf(fptr,"%d ",&(liveVar.temperatureUnits));
            liveVar.UNITS_APPLIED = true;
        }
        else if (strcmp(str,"TRIGROUP")==0){
            while (1){
                fscanf(fptr,"%d %d",&tri,&group);
                if (tri == -1)
                    break;
                vox.setTriGroup(tri,group);
            }
        }
        else if (strcmp(str,"BCSET")==0){
            fscanf(fptr,"%d ",&(liveVar.currentBCSet));
            bcSet = liveVar.currentBCSet;
            liveVar.nBCSet = MAX(liveVar.nBCSet,bcSet+1);
            if (liveVar.nBCSet >= MAX_BC_SET-1) {
                writeMessageToGUIWindow("Number of boundary conditions exceeds allowed");
                break;
            }
        }
        else if (strcmp(str,"BC")==0){
            fscanf(fptr,"%d ",&group);
            fscanf(fptr,"%d %d %d %lf %lf %lf %lf %lf %lf",&(liveVar.BCType[bcSet][group][0]),&(liveVar.BCType[bcSet][group][1]), &(liveVar.BCType[bcSet][group][2]),
                    &(liveVar.BCValue[bcSet][group][0]),&(liveVar.BCValue[bcSet][group][1]), &(liveVar.BCValue[bcSet][group][2]),
                    &(liveVar.BCValue[bcSet][group][3]),&(liveVar.BCValue[bcSet][group][4]), &(liveVar.BCValue[bcSet][group][5]));
            liveVar.BC_STRUCTURAL_APPLIED = true;
            // copy the Dirichlet to all other BC sets
            // We cannot allow different Dirichlet for different loads
            // if one BCset has zero dirichlet then so should all bcsets
            if (!ALLOW_INDEPENDENT_RESTRAINTS) {
                for (int i = 0; i < 3; i++){
                    if (liveVar.BCType[bcSet][group][i] == ELASTICITY_DIRICHLET)
                        for (int tempBCSet = 0; tempBCSet < MAX_BC_SET;tempBCSet++)
                            liveVar.BCType[tempBCSet][group][i] = ELASTICITY_DIRICHLET;
                }
            }
        }
        else if (strcmp(str,"THERMALBC")==0){
            fscanf(fptr,"%d ",&group);
            fscanf(fptr,"%d %lf %lf %lf",&(liveVar.ThermalBCType[bcSet][group]),&(liveVar.ThermalBCArea[bcSet][group]),&(liveVar.ThermalBCValue[bcSet][group]),&(liveVar.ThermalBCRef[bcSet][group]));
            liveVar.BC_THERMAL_APPLIED = true;
        }
        else if (strcmp(str,"ELASTICITY_ACCELERATION")==0){
            fscanf(fptr,"%f %f %f",&(liveVar.XAcceleration),&(liveVar.YAcceleration),&(liveVar.ZAcceleration));
            liveVar.ElasticityAcceleration[liveVar.currentBCSet][0] = liveVar.XAcceleration;
            liveVar.ElasticityAcceleration[liveVar.currentBCSet][1] = liveVar.YAcceleration;
            liveVar.ElasticityAcceleration[liveVar.currentBCSet][2] = liveVar.ZAcceleration;
            if (PARETO_VERSION >= 2017.02){
                fscanf(fptr,"%f ",&(liveVar.radiansPerSecZAxis));
                liveVar.ElasticityAcceleration[liveVar.currentBCSet][3] = liveVar.radiansPerSecZAxis;
            }
            liveVar.ACCELERATION_APPLIED = true;
        }
        else if (strcmp(str,"SYMMETRY")==0){
            fscanf(fptr,"%d %d %d",&(tempInt),&(tempInt2),&(tempInt3));
            liveVar.topoptConstraints.xSymmetry = tempInt;
            liveVar.topoptConstraints.ySymmetry = tempInt2;
            liveVar.topoptConstraints.zSymmetry = tempInt3;
        }
        else if (strcmp(str,"ZERO_STRAIN_TEMPERATURE") == 0){
            fscanf(fptr,"%f ",&(liveVar.zeroStrainTemperature));
        }
        else if (strcmp(str,"THERMAL_EFFECTS") == 0){
            fscanf(fptr,"%d ",&(liveVar.includeThermalEffects));
        }
        else if (strcmp(str,"MESHFILE")==0){ // read mesh
            //if ((liveVar.BC_STRUCTURAL_APPLIED)  || (liveVar.BC_THERMAL_APPLIED)){
                fscanf(fptr,"%s",tempStr);
                writeMessageToGUIWindow("CoreFunctions: Attempting to read mesh file %s",tempStr);
                string meshFile  = liveVar.projectDirectory  + string(tempStr);

                if (!readMeshAndAllocate(meshFile.c_str())){
                    writeWarningToGUIWindow("Could not read mesh file %s",tempStr);
                    break;
                }
                liveVar.displayGeom = DISPLAY_GEOM_MESH;
                liveVar.geomTransparent = true;
                liveVar.MESH_APPLIED = true;
                liveVar.plotElasticityBC = false;
                cout << "Mesh Loaded"<< endl;
            /*}
            else {
                writeWarningToGUIWindow("Mesh parameters should appear after BC ");
            }*/
        }
        else if (strcmp(str,"SIMPDENSITYFILE")==0){ // read SIMP DENSITY FILE
            //if ((liveVar.BC_STRUCTURAL_APPLIED)  || (liveVar.BC_THERMAL_APPLIED)){
            if (liveVar.MESH_APPLIED == true){
                fscanf(fptr,"%s",tempStr);
                writeMessageToParetoLog("CoreFunctions: Attempting to read simp density file %s",tempStr);
                string simpDensityFile  = liveVar.projectDirectory  + string(tempStr);

                if ( !topopt.readSIMPDensityBinary(simpDensityFile.c_str())){
                    writeMessageToParetoLog("Could not read simp file %s",tempStr);
                    break;
                }

               writeMessageToParetoLog("SIMP density Loaded");
            }
            else {
                writeWarningToGUIWindow("SIMP density should appear after mesh is read ");
            }
        }
        else if (strcmp(str,"MESH")==0){
            if ((liveVar.BC_STRUCTURAL_APPLIED)  || (liveVar.BC_THERMAL_APPLIED)){
                fscanf(fptr,"%s",tempStr);
                sscanf(tempStr,"%d ",&(liveVar.nElemsToMesh));
                liveVar.meshQuality = MESH_VERY_COARSE;
                if (liveVar.nElemsToMesh > 10000)
                    liveVar.meshQuality = MESH_COARSE;
                if (liveVar.nElemsToMesh > 25000)
                    liveVar.meshQuality = MESH_MEDIUM;
                if (liveVar.nElemsToMesh > 50000)
                    liveVar.meshQuality = MESH_FINE;
                if (liveVar.nElemsToMesh > 100000)
                    liveVar.meshQuality = MESH_VERY_FINE;
            }
            else {
                writeWarningToGUIWindow("Mesh parameters should appear after BC ");
            }
        }
        else if (strcmp(str,"MESHGRID")==0){
            if ((liveVar.BC_STRUCTURAL_APPLIED)  || (liveVar.BC_THERMAL_APPLIED)){
                int nelx,nely,nelz;
                fscanf(fptr,"%d %d %d",&nelx,&nely,&nelz);
                vox.setVoxelGrid(nelx,nely,nelz);
            }
            else {
                writeWarningToGUIWindow("Mesh parameters should appear after BC ");
            }
        }
        else if (strcmp(str,"NODEBC")==0){ // direct BC on Nodes
            writeMessageToGUIWindow("CoreFunctions: Reading NodeBC");
            fscanf(fptr,"%d ",&group);
            fscanf(fptr,"%d %d %d %lf %lf %lf %lf %lf %lf",&(liveVar.BCType[bcSet][group][0]),&(liveVar.BCType[bcSet][group][1]), &(liveVar.BCType[bcSet][group][2]),
                    &(liveVar.BCValue[bcSet][group][0]),&(liveVar.BCValue[bcSet][group][1]), &(liveVar.BCValue[bcSet][group][2]),
                    &(liveVar.BCValue[bcSet][group][3]),&(liveVar.BCValue[bcSet][group][4]), &(liveVar.BCValue[bcSet][group][5]));
            liveVar.BC_STRUCTURAL_APPLIED = true;
            topopt.setBCOnNodesWithLabel(group,liveVar.BCType[bcSet][group],liveVar.BCValue[bcSet][group]);
        }
        else if (strcmp(str,"MASS_DAMPING")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.alphaM = (float) temp;
        }
        else if (strcmp(str,"STIFFNESS_DAMPING")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.betaK = (float) temp;
        }
        else if (strcmp(str,"NEWMARK_GAMMA")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.NewmarkGamma = (float) temp;
        }
        else if (strcmp(str,"NEWMARK_BETA")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.NewmarkBeta = (float) temp;
        }
        else if (strcmp(str,"TRANSIENT_TIME")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.totalTime = (float) temp;
        }
        else if (strcmp(str,"TRANSIENT_STEP")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.timeStep = (float) temp;
        }
        else if (strcmp(str,"TRACK_POINT")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.trackPoint[0] = (float) temp;
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.trackPoint[1] = (float) temp;
            fscanf(fptr,"%lf",&(temp));
            liveVar.structuralDynamics.trackPoint[2] = (float) temp;

            int dof;
            fscanf(fptr,"%d",&(dof));
            liveVar.structuralDynamics.trackDOF = dof;

        }
        else if (strcmp(str,"NONLINEAR_STEPS")==0){
            int tempInt;
            fscanf(fptr,"%d",&(tempInt));
            liveVar.nonlinear.nSteps = tempInt;
        }
        else if (strcmp(str,"NONLINEAR_ITERATIONS")==0){
            int tempInt;
            fscanf(fptr,"%d",&(tempInt));
            liveVar.nonlinear.nIterations = tempInt;
        }
        else if (strcmp(str,"NONLINEAR_TOLERANCE")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.nonlinear.tolerance = temp;
        }

        else if (strcmp(str,"FINALMESH")==0){// assume final voxel mesh file resides in current directory
            fscanf(fptr,"%s",tempStr);
            if (strstr(tempStr,".vxb") ||  strstr(tempStr,".VXB")  ){
                writeMessageToGUIWindow("Opening mesh file %s in directory %s ",tempStr,liveVar.projectDirectory.c_str());
                string fullPathName;
                fullPathName = liveVar.projectDirectory + string(tempStr);
                if (!generateResultsFromFinalMesh(fullPathName)){
                    writeWarningToGUIWindow("Could not generate results ");
                    break;
                }
            }
            else {
                writeWarningToGUIWindow("Mesh file %s does not seem to have .vxb extension\n Make sure the file name does not have a space in it ",tempStr);
                continue;
            }
        }
        else if (strcmp(str,"FINALISO")==0){// the final iso surface resides in current directory
            fscanf(fptr,"%s",tempStr);
            if (strstr(tempStr,".off") ||  strstr(tempStr,".OFF")  ){
                writeMessageToGUIWindow("Opening iso file %s in directory %s ",tempStr,liveVar.projectDirectory.c_str());
                string fullPathName;
                fullPathName = liveVar.projectDirectory + string(tempStr);
                if (!topopt.readExtendedOffFile(fullPathName.c_str())){
                    writeWarningToGUIWindow("Could not load iso surface file %s ",tempStr);
                    break;
                }
                liveVar.forceRemesh = false;
                liveVar.TOPOPT_SOLVED = true;
                solveStatic();
                liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
                liveVar.displayField = DISPLAY_FIELD_NONE;
                liveVar.isTopOptComplete = true;
            }
            else {
                writeWarningToGUIWindow("Iso surface file %s does not seem to have .off extension\n Make sure the file name does not have a space in it ",tempStr);
                break;
            }
        }
        else if (strcmp(str,"USEGPU")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.useGPU = tempInt;
        }
        else if (strcmp(str,"MESHASPECT")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.maxAspectRatio = MIN(temp, 10.0);
            liveVar.maxAspectRatio = MAX(liveVar.maxAspectRatio, 1.0);
        }
        else if (strcmp(str,"NODEBASED")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.nodeBasedKU = tempInt;
        }

        else if (strcmp(str,"COMMENT")==0){
            fgets(str,255,fptr);// remainder of line
            fgets(str,255,fptr);
            writeMessageToGUIWindow("Comment: %s ",str);
        }
        else if (strcmp(str,"DEFLATION")==0){// enable/disable
            fscanf(fptr,"%d %d",&(liveVar.workSpaceMethod),&(liveVar.nWorkSpaceGroups));
            if (liveVar.nWorkSpaceGroups <=0)
                liveVar.nWorkSpaceGroups = getRecommendedDeflationGroups(liveVar.nElemsToMesh);

        }
		else if (strcmp(str, "PRECONDITIONER") == 0) {// enable/disable, useful in SIMP mode
			fscanf(fptr, "%d", &(liveVar.usePreconditioner));
		}
        else if (strcmp(str,"MAXTIME")==0){// time allowed
            fscanf(fptr,"%lf",&(temp));
            liveVar.maxTimeAllowed = (float) temp;
            //printf("Max time allowed is %f \n",liveVar.maxTimeAllowed);
        }
        else if (strcmp(str,"SOLVETHERMAL") == 0) {
            if (!liveVar.MESH_APPLIED) {
                createTopOptMesh();
                fscanf(fptr,"%d",&(liveVar.currentBCSet));
                changeTopOptBC();
                solveThermoStatic();
            }
            else {
                writeWarningToGUIWindow("Could not solve thermal problem; please create mesh ");
				break;
            }
        }
        else if (strcmp(str,"SOLVESTATIC") == 0) {
            if (!liveVar.MESH_APPLIED) {
                createTopOptMesh();
            }
            if (liveVar.MESH_APPLIED) {
                fscanf(fptr,"%d",&(liveVar.currentBCSet));
                changeTopOptBC();

                if (!checkForFiniteForce()){
                    writeWarningToGUIWindow("Warning: No force has been applied on the geometry");
                    //break;
                }
                solveStatic();
            }
            else {
                writeWarningToGUIWindow("Could not solve static problem; please create mesh ");
            }
        }
        else if (strcmp(str,"SOLVESTATIC_ONMESH") == 0) {
            if (liveVar.MESH_APPLIED) {
                //if (!checkForFiniteForce()){
                   // writeWarningToGUIWindow("Warning: No force has been applied on the geometry");
                    //break;
                //}
                if (liveVar.workSpaceMethod != DEFLATION_NONE)
                    topopt.enableDeflation(liveVar.workSpaceMethod,liveVar.nWorkSpaceGroups);
                else
                    topopt.disableDeflation();

                timer.start();
                topopt.solveElasticityStatic();
                topopt.postprocess();
                //solveStaticWithMaxTemplates();



                writeMessageToGUIWindow("#CG Iterations = %d",topopt.getLastCGIteration());
                writeMessageToGUIWindow("Time taken = %.3f (sec) ",timer.getElapsedTimeInSec());

                writeMessageToGUIWindow("Max Displacement: %0.2e",topopt.getDeformationMax());
                writeMessageToGUIWindow("Max Von Mises: %0.2e",topopt.getMaxVonMisesStress());
                writeMessageToGUIWindow("Time taken = %.3f (sec)",timer.getElapsedTimeInSec());

                liveVar.STATIC_FEA_SOLVED = true;
            }
            else {
                writeWarningToGUIWindow("Could not solve static problem; please create mesh ");
            }
        }
        else if (strcmp(str,"SOLVEMODAL") == 0) {
            if (liveVar.STATIC_FEA_SOLVED) {
                fscanf(fptr,"%d %d",&(liveVar.currentBCSet),&(liveVar.nEigenModes));
                changeTopOptBC();
                solveModal();
            }
            else {
                writeWarningToGUIWindow("Could not solve modal problem; please solve static FEA first");
            }
        }
        else if (strcmp(str,"CONSTRAINT")==0){
            fscanf(fptr,"%d ",&(liveVar.ConstraintType[bcSet][liveVar.NumberOfConstraints[bcSet]]));
            if (liveVar.ConstraintType[bcSet][liveVar.NumberOfConstraints[bcSet]] == DISPLACEMENT) { //Deng
                fscanf(fptr,"%lf %lf %lf %lf %lf %lf",&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][0]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][1]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][2]),
                        &(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][3]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][4]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][5]) );
            }
            else{
                fscanf(fptr,"%lf %lf %lf",&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][0]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][1]),&(liveVar.ConstraintData[bcSet][liveVar.NumberOfConstraints[bcSet]][2]) );
            }

            liveVar.NumberOfConstraints[bcSet]++;
        }
        else if (strcmp(str,"DUMP")==0){
            fscanf(fptr,"%d",&tempInt);
            double offset[] = {0,0,0};
            int closestTri;
            double tempLoc[3],locationDelta[3],locationStress[3],deltaMax,vonMisesMax;
            int	nodeDeltaMax,nodeStressMax;

            if (liveVar.STATIC_FEA_SOLVED ) {
                cout << "DUMPING " << tempInt << endl;
                if (tempInt == DUMP_BASIC){// Basic result: nNodes, max delta info and max stress info
                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_deltaSigma.txt");
                    FILE *fptr = fopen(tempStr,"w");
                    if (fptr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",tempStr);
                        if (fptr == NULL) continue;
                    }

                    deltaMax = topopt.getDeformationMax(&nodeDeltaMax);
                    vonMisesMax = topopt.getMaxVonMisesStress(&nodeStressMax);
                    topopt.getNodeXYZ(nodeDeltaMax,tempLoc);
                    vox.distanceOfPointToSTL(tempLoc,locationDelta,&closestTri);
                    topopt.getNodeXYZ(nodeStressMax,tempLoc);
                    vox.distanceOfPointToSTL(tempLoc,locationStress,&closestTri);
                    fprintf(fptr,"%d\n",topopt.getNumElems());
                    fprintf(fptr,"%.3e %.3e %.3e %.5e\n",locationDelta[0]+offset[0],locationDelta[1]+offset[1],locationDelta[2]+offset[2],deltaMax);
                    fprintf(fptr,"%.3e %.3e %.3e %.5e\n",locationStress[0]+offset[0],locationStress[1]+offset[1],locationStress[2]+offset[2],vonMisesMax);
                    fprintf(fptr,"%.3f \n",topopt.getCurrentVolumeFractionOfOptimizedParts());
                    fclose(fptr);
                }
                else if (tempInt == DUMP_TEMP){// Temperature at all  points requested
                    sprintf(xyzFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(xyzFile,"_XYZ.txt");

                    sprintf(outputFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(outputFile,"_TEMP.txt");

                    FILE *xyzFilePtr = fopen(xyzFile,"r");
                    if (xyzFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",xyzFile);
                        continue;
                    }
                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        fclose(xyzFilePtr);
                        continue;
                    }
                    fscanf(xyzFilePtr,"%d",&nPoints);
                    fprintf(outputFilePtr,"%d\n",nPoints);

                    voxelElemPrev = 0;
                    double pt[3];
                    for (int i =0; i < nPoints; i++) {
                        fscanf(xyzFilePtr,"%lf %lf %lf ",&x,&y,&z);
                        pt[0] = x - offset[0];
                        pt[1] = y - offset[1];
                        pt[2] = z - offset[2];
                        topopt.getTemperatureAt(pt,&temp);
                        fprintf(outputFilePtr,"%.2e  \n",temp);
                    }
                    fclose(xyzFilePtr);
                    fclose(outputFilePtr);
                }
                else if (tempInt == DUMP_UVW){// u v w and stress at all  points requested
                    sprintf(xyzFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(xyzFile,"_XYZ.txt");
                    sprintf(outputFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(outputFile,"_UVWSTRESS.txt");

                    FILE *xyzFilePtr = fopen(xyzFile,"r");
                    if (xyzFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",xyzFile);
                        continue;
                    }

                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        fclose(xyzFilePtr);
                        continue;
                    }
                    fscanf(xyzFilePtr,"%d",&nPoints);
                    fprintf(outputFilePtr,"%d\n",nPoints);

                    voxelElemPrev = 0;

                    for (int i =0; i < nPoints; i++) {
                        fscanf(xyzFilePtr,"%lf %lf %lf ",&x,&y,&z);
                        pt[0] = x - offset[0];
                        pt[1] = y - offset[1];
                        pt[2] = z - offset[2];
                        if (topopt.getElasticitySolutionAt(pt,uvw,stresses,&vonmises))
                            fprintf(outputFilePtr,"%.2e %.2e %.2e %.2e \n",uvw[0],uvw[1],uvw[2],vonmises);
                        else // use previous value, assuming point is close by
                            fprintf(outputFilePtr,"%.2e %.2e %.2e %.2e  \n",uvw[0],uvw[1],uvw[2],vonmises);//previous value
                    }
                    fclose(xyzFilePtr);
                    fclose(outputFilePtr);

                }
                else if (tempInt == DUMP_UVW_TEMP){// u v w and stress, and temperature at all  points requested
                    sprintf(xyzFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(xyzFile,"_XYZ.txt");
                    sprintf(outputFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(outputFile,"_UVWSTRESS.txt");
                    FILE *xyzFilePtr = fopen(xyzFile,"r");
                    if (xyzFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",xyzFile);
                        continue;
                    }
                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        fclose(xyzFilePtr);
                        continue;
                    }
                    fscanf(xyzFilePtr,"%d",&nPoints);
                    fprintf(outputFilePtr,"%d\n",nPoints);

                    voxelElemPrev = 0;
                    for (int i =0; i < nPoints; i++) {
                        fscanf(xyzFilePtr,"%lf %lf %lf ",&x,&y,&z);
                        pt[0] = x - offset[0];
                        pt[1] = y - offset[1];
                        pt[2] = z - offset[2];
                        topopt.getElasticitySolutionAt(pt,uvw,stresses,&vonmises);
                        topopt.getTemperatureAt(pt,&temp);
                        fprintf(outputFilePtr,"%.2e %.2e %.2e %.2e %.2e \n",uvw[0],uvw[1],uvw[2],vonmises,temp);
                    }
                    fclose(xyzFilePtr);
                    fclose(outputFilePtr);

                }
                else if (tempInt == DUMP_VOXELMESH){
                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_voxelMesh.txt");
                    vox.dumpVoxelData(tempStr);
                }
                else if (tempInt == DUMP_ABAQUS_RPT){// u v w at all  mesh nodes in ABAQUS .rpt format
                    if (loadProjectVerbose) writeMessageToGUIWindow("Dumping .rpt file");
                    sprintf(outputFile,"%s",liveVar.userName.c_str());
                    strcat(outputFile,".rpt");

                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        continue;
                    }
                    fprintf(outputFilePtr,"**************************************** \n");
                    fprintf(outputFilePtr,"Generated by Pareto \n\n\n");
                    fprintf(outputFilePtr," Label          @Loc 1          @Loc 1          @Loc 1 \n");
                    fprintf(outputFilePtr,"-----------------------------------------------------------------\n");

                    for (int i =0; i < topopt.getNumNodes(); i++) {
                        topopt.getNodeDeformation(i,&uvw[0],&uvw[1],&uvw[2],&vonmises);
                        fprintf(outputFilePtr,"%d %10e %10e %10e \n",i+1,uvw[0],uvw[1],uvw[2]);//note that node number starts from 1
                    }
                    fclose(outputFilePtr);
                }
                else if (tempInt == DUMP_BND){// identify nodes that are constrained and nodes on which forces are applied
                    //#DIRICHLET
                    //nNodes
                    //n1 n2 n3 n4  ...
                    //#NEUMANN
                    //nNodes
                    //N1 N2 N3 N4 ...
                    int nDirichletNodes,nNeumannNodes;
                    sprintf(outputFile,"%s",liveVar.userName.c_str());
                    strcat(outputFile,".bnd");

                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        continue;
                    }
                    nDirichletNodes =0;
                    for (int i =0; i < topopt.getNumNodes(); i++) {
                        if (topopt.isNodeFree(i)) continue;
                        nDirichletNodes++;
                    }
                    fprintf(outputFilePtr,"%d \n",nDirichletNodes);

                    for (int i =0; i < topopt.getNumNodes(); i++) {
                        if (topopt.isNodeFree(i)) continue;
                        fprintf(outputFilePtr,"%d ",i);
                        nDirichletNodes++;
                    }
                    fprintf(outputFilePtr,"\n");

                    double force[3],magnitude;
                    nNeumannNodes =0;
                    for (int i =0; i < topopt.getNumNodes(); i++) {
                        topopt.getNeumannForce(i,force);
                        magnitude = NORM3D(force);
                        if (magnitude >0){
                            nNeumannNodes++;
                        }
                    }
                    fprintf(outputFilePtr,"%d \n",nNeumannNodes);
                    for (int i =0; i < topopt.getNumNodes(); i++) {
                        topopt.getNeumannForce(i,force);
                        magnitude = NORM3D(force);
                        if (magnitude >0){
                            fprintf(outputFilePtr,"%d ",i);
                            nNeumannNodes++;
                        }
                    }

                    fprintf(outputFilePtr,"\n");
                    fclose(outputFilePtr);
                }
                else if (tempInt == DUMP_MESH){//  dump mesh file
                    sprintf(outputFile,"%s",liveVar.userName.c_str());
                    strcat(outputFile,".msh");
                    topopt.dumpMeshBinary(outputFile);
                }
                else if (tempInt == DUMP_UVW_NODES){//  dump results
                    sprintf(outputFile,"%s",liveVar.userName.c_str());
                    strcat(outputFile,".uvw");
                    topopt.dumpUVW(outputFile);
                }
                else if (tempInt == DUMP_SIMP_DENSITY){//  dump results
                    sprintf(outputFile,"%s",liveVar.userName.c_str());
                    strcat(outputFile,".rho");
                    writeMessageToParetoLog("Writing SIMP density ");
                    topopt.dumpSIMPDensityBinary(outputFile);
                }
            }
            if (liveVar.MODAL_FEA_SOLVED) {
                if (tempInt == DUMP_MODAL){// Modal fea eigen values and modes requested
                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_eigenValues.txt");
                    FILE *fptrTemp = fopen(tempStr,"w");
                    if (fptrTemp == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",tempStr);
                        continue;
                    }
                    fprintf(fptrTemp,"%d \n",liveVar.nEigenModes);
                    for (int i = 0; i < liveVar.nEigenModes; i++) {
                        fprintf(fptrTemp,"%e \n",liveVar.eigenValues[i]);
                    }
                    fclose(fptrTemp);

                    sprintf(xyzFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(xyzFile,"_XYZ.txt");

                    FILE *xyzFilePtr = fopen(xyzFile,"r");
                    if (xyzFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",xyzFile);
                        continue;
                    }

                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_eigenModes.txt");

                    FILE *outputFilePtr = fopen(tempStr,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",tempStr);
                        fclose(outputFilePtr);
                        continue;
                    }
                    // Find the maximum deflection for scaling
                    double deformationMax[MAX_EIGEN_MODES];
                    for (int i = 0; i < liveVar.nEigenModes; i++) {
                        deformationMax[i] = 0;
                        for (int node = 0; node < topopt.getNumNodes(); node++) {
                            uvw[0] = liveVar.eigenModes[i][3*node];
                            uvw[1] = liveVar.eigenModes[i][3*node+1];
                            uvw[2] = liveVar.eigenModes[i][3*node+2];
                            delta = sqrt(uvw[0]*uvw[0] + uvw[1]*uvw[1]+ uvw[2]*uvw[2]);
                            deformationMax[i] = MAX(deformationMax[i],delta);
                        }
                        fprintf(outputFilePtr,"%d %lf\n",i, deformationMax[i]);
                    }

                    fscanf(xyzFilePtr,"%d",&nPoints);
                    fprintf(outputFilePtr,"%d\n",nPoints);
                    int nodes[8];
                    double xi,eta,mu,N[8],uNodes[8],vNodes[8],wNodes[8];
                    for (int i =0; i < nPoints; i++) {
                        fscanf(xyzFilePtr,"%d %lf %lf %lf ",&ptNumber,&x,&y,&z);
                        x -= offset[0];
                        y -= offset[1];
                        z -= offset[2];
                        //vox.closestVoxelNodeToPoint(x,y,z,&voxelNode);
                        vox.closestVoxelElementToPoint(x,y,z,&(voxelElem),xietamu);
                        if (voxelElem >=0) {
                            topopt.getElemNodes(voxelElem,nodes);
                            xi = MAX(-1,MIN(1,xietamu[0]));
                            eta = MAX(-1,MIN(1,xietamu[1]));
                            mu = MAX(-1,MIN(1,xietamu[2]));
                            topopt.computeN(xi,eta,mu,N);
                        }
                        // else use the previous set of nodes, xi eta and mu

                        for (int mode = 0; mode < liveVar.nEigenModes; mode++) {
                            // gather
                            for (int m = 0; m < 8; m++) {
                                uNodes[m] = liveVar.eigenModes[mode][3*nodes[m]];
                                vNodes[m] = liveVar.eigenModes[mode][3*nodes[m]+1];
                                wNodes[m] = liveVar.eigenModes[mode][3*nodes[m]+2];
                            }
                            uvw[0] = uvw[1] = uvw[2] = 0;
                            for (int m = 0;m < 8; m++) {
                                uvw[0] += N[m]*uNodes[m];
                                uvw[1] += N[m]*vNodes[m];
                                uvw[2] += N[m]*wNodes[m];
                            }
                            fprintf(outputFilePtr,"%.3f %.3f %.3f \n",uvw[0],uvw[1],uvw[2]);
                        }
                    }
                    fclose(xyzFilePtr);
                    fclose(outputFilePtr);
                }
            }

            if (liveVar.TOPOPT_SOLVED)  {
                if (tempInt == DUMP_TOPOPT_ISO){// Intermediate topologies requested
                    char tempStr[256];
                    string resultFile;
                    int nSavedVolFractions;
                    double volFractions[MAX_ISO_SAVES],maxDisp[MAX_ISO_SAVES],maxStress[MAX_BC_SET][MAX_ISO_SAVES];
                    double compliance[MAX_BC_SET][MAX_ISO_SAVES],timeTaken[MAX_ISO_SAVES];

                    // Save the last error message
                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_topoptError.txt");
                    FILE *fptr = fopen(tempStr,"w");

                    if (fptr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",tempStr);
                        continue;
                    }
                    char errorMessage[1024];
                    topopt.getLastTopoptErrorMessage(errorMessage);
                    fprintf(fptr,"%s\n",errorMessage);
                    fclose(fptr);

                    // Save all the displacements and stresses at intermediate topologies
                    nSavedVolFractions = topopt.getCurrentIsoCounter()+1;
                    sprintf(tempStr,CLIENTFILES,liveVar.userName.c_str());
                    strcat(tempStr,"_topoptResults.txt");
                    fptr = fopen(tempStr,"w");
                    if (fptr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",tempStr);
                        continue;
                    }
                    topopt.getResultsAtAllVolFracs(volFractions,maxDisp,maxStress,compliance,timeTaken);
                    fprintf(fptr,"%d \n",nSavedVolFractions);
                    for (int i = 0; i < nSavedVolFractions; i++) {
                        fprintf(fptr,"%.3f %e %e %.2f \n",volFractions[i],maxDisp[i],maxStress[i],timeTaken[i]);
                    }
                    fclose(fptr);
                    double actualFrac,time,isoCutoff;
                    int isoCounter;
                    //save isosurface at intermediate topologies
                    for (int i = 0; i < nSavedVolFractions; i++) {
                        sprintf(tempStr,".%.3f", volFractions[i]);
                        resultFile = liveVar.projectDirectory + liveVar.modelName +string(tempStr) + string(".stl");

                        isoCounter = topopt.getClosestCounterAtVolFraction(volFractions[i],&actualFrac,&isoCutoff,&time);
                        topopt.dumpIsoBinary(isoCounter,resultFile.c_str());
                    }
                }
                if (tempInt == DUMP_ISO_UVW){//  stress at the center of each triangle of the final isosurface
                    int isoCounter;
                    double xTri[3],yTri[3],zTri[3],triNormal[3];

                    isoCounter = topopt.getCurrentIsoCounter();
                    sprintf(outputFile,CLIENTFILES,liveVar.userName.c_str());
                    strcat(outputFile,"_ISO_STRESS.txt");

                    FILE *outputFilePtr = fopen(outputFile,"w");
                    if (outputFilePtr == NULL){
                        writeWarningToGUIWindow("Could not open file %s ",outputFile);
                        continue;
                    }
                    nPoints = topopt.getNumIsoTriangles(isoCounter);//  stress at the center of each triangle
                    double vertexNormals0[3],vertexNormals1[3],vertexNormals2[3];
                    for (int tri = 0; tri < topopt.getNumIsoTriangles(isoCounter); tri++) {
                        topopt.getIsoTriPts(isoCounter,tri,xTri,yTri,zTri,triNormal,vertexNormals0,vertexNormals1,vertexNormals2);
                        pt[0] = (xTri[0] + xTri[1] + xTri[2])/3;
                        pt[1] = (yTri[0] + yTri[1] + yTri[2])/3;
                        pt[2] = (zTri[0] + zTri[1] + zTri[2])/3;
                        if (topopt.getElasticitySolutionAt(pt,uvw,stresses,&vonmises))
                            fprintf(outputFilePtr,"%.2e \n",vonmises);
                        else // use previous value, assuming point is close by
                            fprintf(outputFilePtr,"%.2e  \n",vonmises);//previous value
                    }
                    fclose(outputFilePtr);
                }
            }
        }
        else if (strcmp(str,"EXTRUDEDIR")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.isExtrudeActive = true;
            liveVar.topoptConstraints.extrudeDirection = tempInt;
        }
        else if (strcmp(str,"DRAWDIR")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.drawDirection = tempInt;
            liveVar.topoptConstraints.isDrawDirectionActive = true;
        }
        else if (strcmp(str,"CYCLICSYM")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.cyclicSymmetry =  tempInt;
            liveVar.topoptConstraints.isCyclicSymmetryActive = true;
        }
        else if (strcmp(str,"XGRIDPATTERN")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.xGridPattern =  tempInt;
            liveVar.topoptConstraints.isxGridActive = true;
        }
        else if (strcmp(str,"YGRIDPATTERN")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.yGridPattern =  tempInt;
            liveVar.topoptConstraints.isyGridActive = true;
        }
        else if (strcmp(str,"ZGRIDPATTERN")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.zGridPattern =  tempInt;
            liveVar.topoptConstraints.iszGridActive = true;
        }
        else if (strcmp(str,"MINFEATURESIZE")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.minFeatureSize = temp;
            liveVar.topoptConstraints.isMinFeatureSizeActive = true;
        }
        else if (strcmp(str,"MAXFEATURESIZE")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.maxFeatureSize = temp;
            liveVar.topoptConstraints.isMaxFeatureSizeActive = true;
        }
        else if (strcmp(str,"STRESSSAFETYFACTOR")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.stressSafetyFactor = temp;
            liveVar.topoptConstraints.isStressSafetyActive = true;
        }
        else if (strcmp(str,"MAXDISPLACEMENT")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.maxDisplacementAllowed = temp;
            liveVar.topoptConstraints.isMaxDisplacementActive = true;
        }
        else if (strcmp(str,"MAXTEMPERATURE")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.maxTemperatureAllowed = temp;
            liveVar.topoptConstraints.isTemperatureLimitActive = true;
        }
        else if (strcmp(str,"MINEIGENVALUE")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.topoptConstraints.minEigenValueAllowedHz = temp;
            liveVar.topoptConstraints.isMinEigenActive = true;
        }
        else if (strcmp(str,"CONNECTEDTOPOLOGY")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.keepTopologyConnected = (bool) tempInt;
        }
        else if (strcmp(str,"KEEPDIR")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.topoptConstraints.keepDirichlet = (bool) tempInt;
        }
        else if (strcmp(str,"SAVEINTERMEDIATE")==0){
            fscanf(fptr,"%d",&(tempInt));
            liveVar.saveIntermediate = (bool) tempInt;
        }
        else if (strcmp(str,"OBJECTIVE")==0){
            fscanf(fptr,"%d",&(liveVar.topoptObjective));
        }
        else if (strcmp(str,"VOLFRAC")==0){
            fscanf(fptr,"%lf",&temp);
            liveVar.desiredVolFraction = (float) temp;
        }
        else if (strcmp(str,"STEPSIZE")==0){
            fscanf(fptr,"%lf",&(temp));
            liveVar.stepSize = temp;
        }
        else if (strcmp(str,"TOPOPT")==0) {
            char str[256];
            int nActiveBCSets = 0;
            fgets(str,255,fptr);// rest of line
            fgets(str,255,fptr);// next line
            char* token = strtok(str, " ");
            for (int i = 0; i < liveVar.nBCSet;i++)
                liveVar.activeBCSets[i] = false;
            while ((token) && (token[0] != '\n')) {
                int bcset = atoi(token);
                if (bcset > liveVar.nBCSet-1)
                    writeWarningToGUIWindow("Invalid BCSet: %d ",bcset);
                else {
                    nActiveBCSets++;
                    liveVar.activeBCSets[bcset] = true;// this is an active BC Set
                    liveVar.currentBCSet = bcset;

                }
                token = strtok(NULL, " ");
            }
            if ((!liveVar.MESH_APPLIED ))
                createTopOptMesh();
            if (liveVar.MESH_APPLIED) {
                if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)){
                    multiLoadOptimize();
                }
                else {
                    changeTopOptBC();
                    singleLoadOptimize();
                }
            }
        }
        else if(strcmp(str,"SAVE_TIFF")==0) {
            char str[256];
            fgets(str,255,fptr);// rest of line
            fscanf(fptr,"%s",str);// next line, file name
            ;//SaveAsTIFF(str);
        }
        else {
            //writeMessageToGUIWindow("Unknown keyword %s ",str);
            continue;
        }
    }
    // older versions: the constraints map incorrectly. So reset to default
    if (projectVersion < 2019.20) {
       resetLiveVariablesTopOptConstraints();
    }
    fclose(fptr);
    setStatusMessage("Project Loaded");
    cout << "Project Loaded"<< endl;
    liveVar.CONSTRAINTS_APPLIED = true;
}
/*********************************************************************/
void CoreFunctions::setTemperatureUnits(int tempUnits)
{
    liveVar.temperatureUnits = tempUnits;

}
/*********************************************************************/
void CoreFunctions::setAngleUnits(int angleUnits)
{
    liveVar.angleUnits = angleUnits;

}
/*********************************************************************/
double CoreFunctions::convertAngleToRadians(double angle)
{
    if(liveVar.angleUnits == UNITS_ANGLE_RADIAN)
        return angle;
    else
        return angle*3.14159/180;
}
/*********************************************************************/
void CoreFunctions::getMaterialDataAsString(string &materialSummary)
{

    string unitsString;
    char tempStr[256],materialName[256];
    double materialProperties[7];

    materialSummary = string("");
    for (int part = 0; part < vox.getNumParts(); part++){
        topopt.getMaterialPropertiesOfPart(part,materialProperties,materialName);
        if (liveVar.paretoUnits == UNITS_MM){
            // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
            // but this lead to inconsistency in units; we fix this here!
            materialProperties[3] *= 1000;
        }
        if (liveVar.paretoUnits == UNITS_INCH){
            // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
            // but this lead to inconsistency in units; we fix this here!
            materialProperties[3] *= 386.22;
        }
        if (topopt.shouldPartBeOptimized(part))
            sprintf(tempStr,"Part %d: %s\n",part,materialName);
        else
            sprintf(tempStr,"Part %d*: %s\n",part,materialName);

        materialSummary += string(tempStr);

        getPropertyUnits(PROPERTY_PRESSURE, unitsString);
        sprintf(tempStr, "Elastic Modulus: %.4g %s \n", materialProperties[0], unitsString.c_str());
        materialSummary += string(tempStr);

        sprintf(tempStr, "Poisson Ratio: %.3f \n",  materialProperties[1]);
        materialSummary += string(tempStr);

        getPropertyUnits(PROPERTY_PRESSURE, unitsString);
        sprintf(tempStr, "Yield Stress: %.4g %s \n",  materialProperties[2], unitsString.c_str());
        materialSummary += string(tempStr);

        getPropertyUnits(PROPERTY_DENSITY, unitsString);
        sprintf(tempStr, "Density: %.4g %s\n",  materialProperties[3], unitsString.c_str());
        materialSummary += string(tempStr);
    }
}
/*********************************************************************/
void CoreFunctions::getPropertyUnits(int propertyID, string &propertyName)
{
    // DON'T CHANGE THESE STRINGS
    //affects ParetoCreo

    string mass[] = {"kg","lbm","kg","g"};
    string acceleration[]= {"m/s^2","in/s^2","mm/s^2","cm/s^2"};
    string velocity[] = {"m/sec","in/sec","mm/sec","cm/sec"};
    string distance[] = {"m","in","mm","cm"};
    string area[] = {"m^2","in^2","mm^2","cm^2"};
    string volume[] = {"m^3","in^3","mm^3","cm^3"};
    string density[] = {"kg/m^3","lbm/in^3","kg/mm^3","g/cm^3"};
    string force[] = {"N","lbf","N","N"};
    string pressure[] = {"N/m^2","lbf/in^2","1e6 Pa","N/cm^2"};
    string conductivity[] = {"W/m-K","W/in-K","W/m-K","W/m-K"};
    string specificHeat[] = {"J/kg-K","J/kg-K","J/kg-K","J/kg-K"};
    string thermalExpansion[] = {"m/m-K","in/in-K","mm/mm-K","cm/cm-K"};
    string time[] = {"sec", "min", "hour","day"};
    string temperature[] = {"C","K","F"};
    string work[] = {"N-m","lbf-in","N-mm"};
    string price[] = {"US$/kg","US$/lbm","US$/kg"};
    string angle[] = {"rad","deg"};
    string heatFlux[] = {"W/m^2","W/m^2","W/m^2","W/m^2"};
    string heatCoefficient[] = {"W/m^2-K","W/m^2-K","W/m^2-K","W/m^2-K"};


    switch(propertyID)
    {
    case PROPERTY_YOUNG_MODULUS:
    case PROPERTY_YIELD_STRENGTH :
    case PROPERTY_PRESSURE :
        propertyName = pressure[liveVar.paretoUnits];
        break;
    case PROPERTY_ACCELERATION :
        propertyName = acceleration[liveVar.paretoUnits];
        break;
    case PROPERTY_FORCE :
        propertyName = force[liveVar.paretoUnits];
        break;
    case PROPERTY_VELOCITY :
        propertyName = velocity[liveVar.paretoUnits];
        break;
    case PROPERTY_LENGTH :
        propertyName = distance[liveVar.paretoUnits];
        break;
    case PROPERTY_TEMPERATURE :
        propertyName = temperature[liveVar.temperatureUnits];
        break;
    case PROPERTY_DENSITY :
        propertyName = density[liveVar.paretoUnits];
        break;
    case PROPERTY_SPECIFIC_HEAT :
        propertyName = specificHeat[liveVar.paretoUnits];
        break;
    case PROPERTY_THERMAL_CONDUCTIVITY :
        propertyName = conductivity[liveVar.paretoUnits];
        break;
    case PROPERTY_THERMAL_EXPANSION :
        propertyName = thermalExpansion[liveVar.paretoUnits];
        break;
    case PROPERTY_TIME:
        propertyName = time[liveVar.timeUnits];
        break;
    case PROPERTY_MASS:
        propertyName = mass[liveVar.paretoUnits];
        break;
    case PROPERTY_WORK:
        propertyName = work[liveVar.paretoUnits];
        break;
    case PROPERTY_PRICE_PER_UNITMASS:
        propertyName = price[liveVar.paretoUnits];
        break;
    case PROPERTY_ANGLE:
        propertyName = angle[liveVar.angleUnits];
        break;
    case PROPERTY_VOLUME:
        propertyName = volume[liveVar.paretoUnits];
        break;
    case PROPERTY_AREA:
        propertyName = area[liveVar.paretoUnits];
        break;
    case PROPERTY_HEAT_FLUX:
        propertyName = heatFlux[liveVar.paretoUnits];
        break;
    case PROPERTY_HEAT_COEFFICIENT:
        propertyName = heatCoefficient[liveVar.paretoUnits];
        break;
    default:
        propertyName = string("");
    }
}
/*********************************************************************/
