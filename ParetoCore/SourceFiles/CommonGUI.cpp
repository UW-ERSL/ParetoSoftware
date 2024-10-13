#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#else
int MB_OK = 0;
void MessageBoxA(int a,char *b,char *c,int d)
{
	;//dummy call for compilation
}
#endif

#include <stdlib.h>
#include <string.h>
#include <GL/glui.h>
#include "AFElasticityGPUDefines.h"
#include "CommonGUI.h"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "Timer.h"
#include <ctime>
#include "tiffio.h"
#include <windows.h>
#include <process.h>     
#include "text3d.h"
#pragma warning(disable:4996)

static float pareto_xy_aspect;

int pareto_main_window=0,graph_window;
GLint pareto_viewport [4];

static GLfloat GLtrans[3] = {0,0,0};
static GLfloat GLrot[2] = {20,0};
static GLfloat GLLengthScale = 1.0;
static GLint curX=0,curY =0;
static int GLState = 0;
GLdouble currentRayStart[3],currentRayEnd[3];
GLdouble mvmatrix[16], projmatrix[16];

GLUI *glui;
GLUI *displayGlui;
GLUI *messageGlui = NULL;
GLUI_Listbox *BClist;
GLUI_EditText *BCdirText;
GLUI_EditText *BCValueText,*ThermalBCValueText,*ThermalBCRefText;
GLUI_EditText *XForceText,*YForceText,*ZForceText,*PressureText,*TorqueText,*NormalForceText;
GLUI_Checkbox *XDirCheckbox,*YDirCheckbox,*ZDirCheckbox;
GLUI_Checkbox *XSymCheckbox,*YSymCheckbox,*ZSymCheckbox;

extern int BCType[MAX_BC_SET][STL_MAX_GROUPS][3];
extern double BCValue[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
extern double ElasticityAcceleration[MAX_BC_SET][4];
extern int ThermalBCType[MAX_BC_SET][STL_MAX_GROUPS];
extern double ThermalBCValue[MAX_BC_SET][STL_MAX_GROUPS];
extern double ThermalBCMaterial[MAX_BC_SET][STL_MAX_GROUPS];//h or sigma
extern double ThermalBCRef[MAX_BC_SET][STL_MAX_GROUPS];// for convection and radiation


extern struct LiveVariables liveVar;
extern char userDefinedTitle[256];

// These must be defined by user

extern void (*userPlottingFunction)();
extern void (*otherPlottingFunction)();
extern PARETO_GUI_STATE guiState;
void pareto_myGlutDisplay();
bool GetMACaddress(char str[]);
int GetAllMACaddresses(char str[12][14]);
unsigned long hashString(char *str,int tmp);
void solveStatic();
void SIMPOptimize();
void setPlotRoutine(int control);
void plotEigenModeOverSTL();
void plotFieldOverSTL();
void plotGeometry();
void plotMesh();
void plotSmoothMesh();
void plotMeshWithPseudoDensity();
void plotStressOnMesh();
void plotLoad();
void plotFieldOverMesh();
void plotFieldOverSTL();
void plotEigenModeOverIsoSurface();
void plotIsoSurface();
void plotIsoSurfaceInverted();
void plotPseudoDensitySurface();
void plotSphereMap();
bool saveIsoSurface(char *fileName,double *offset,bool snapToGeom,double desiredFractionOfIsoTriangles,bool fromMeshPseudoDensity);
void plotTopSensField();
void plotNothing();
void plotMeshComponents();
void plotTemperatureOverMesh();
void plotFinalSTL();
void plotFieldOverFinalSTL();
void plotXY(vector<double> xData, vector<double> yData, bool smooth = false,char *xTitle="",char *yTitle="",char *chartTitle = "",int width = 300,int height = 300);
void plotXYMultiple(vector<double> xData, vector<vector<double>> yData, vector <char *> yLegend,bool smooth = false,char *xTitle="",char *yTitle="",char *chartTitle = "",int width = 300,int height = 300);
void plotAMPlatform(int);
void plotMeshSurfacePseudoDensity(void);

void ParetoBCTypeChanged(int);

extern void alert(char *msg);
void changeTopOptBC();
void applyElasticityBC();
void applyThermalBC();
void savePRJ();
void loadPRJ(char *extension = ".prj");
void createTopOptMesh();
void deformMeshToBoundary();
void solveThermoStatic();
void solveThermoElastic();
void createTopOptMesh();
void solveModal();
void solveBuckling();
bool  updateGeom(bool assembly);
void recomputeBoundaryConditions();
void solveEigenStatic();
void singleLoadOptimize();
void singleLoadOptimizeWithConstraints();
void multiLoadOptimize();
void callBackUserFunction(int);
int  loadDMatrices(char fileName[]);
bool createSkeleton();
void solveToolPathThermoStatic();
void solveThermoTransient();

extern ParetoApplication topopt;
extern GPUData h_gpuData;
extern Voxelizer vox;
extern Voxelizer voxInitialDesign;
extern Timer globalTimer;

extern Timer timer;
extern int replaceTitle;

void (*userPlottingFunction)();
extern void setRotate(double rot[2]);
extern void setTranslate(double trans[2]);
extern void setZoom(double zoom);
extern GLdouble currentRayStart[3],currentRayEnd[3];

extern int BCType[MAX_BC_SET][STL_MAX_GROUPS][3];
extern double BCValue[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
extern int NumberOfConstraints[MAX_BC_SET];

extern PARETO_GUI_STATE guiState;
extern GLUI *glui;

extern void pareto_setViewRoutine(int control);
extern void pareto_setcamera(float *camera);
void BCTypeChanged(int control);
void plotEigenModeOverSTL();
void plotFieldOverSTL();
void plotGeometry();
void plotMesh();
void plotMeshSurface();
void plotStressOnMesh();
void plotLoad();
void plotFieldOverMesh();
void plotFieldOverSTL();
void plotThermalFieldOverMesh();
void plotThermalFieldOverSTL();
void plotEigenModeOverIsoSurface();
void plotIsoSurface();
void plotTopSensField();
void plotNothing();
void plotWorkSpacePatch();
void getMaxDeformationAndStressOverSTL(double *maxDeformation,double *maxStress);
void getMaxDeformationAndStressOverMesh(double *maxDeformation,double *maxStress);
void setMainWindow();
void SaveAsBMP(const char *fileName);
int GetFileNameToSave(char *filter, char *ext,char szFileName[]);
int GetFileNameToOpen(char *filter, char *ext,char szFileName[]);
int find_word(FILE *filein,char *word);
void resetLiveVariables();
void initializeLiveVariables();
void safeFree(void *ptr);
void allocateMemoryForEigenModes();
void  freeEigenModesMemory();
bool checkForFiniteForce();
int loadGeom(bool assembly);

extern void createLatticeStructureMesh();

int mouseYOffset = 0;// status bar messes things up. So we have to shift the mouse clicks


extern char * materialNamesStr[];
extern void multiLoadOptimizeThread(void* Param);
extern void singleLoadOptimizeThread(void* Param);

int isNumeric (const char * s)
{
    if (s == NULL || *s == '\0' || isspace(*s))
      return 0;
    char * p;
    strtod (s, &p);
    return *p == '\0';
}
/**************************************************************/
void solveThermoToolPathFEAThread(void* Param)
{
	solveToolPathThermoStatic();
}
/**************************************************************/
void solveThermoTransientFEAThread(void* Param)
{
	solveThermoTransient();
}
/**************************************************************/
void solveThermoStaticFEAThread(void* Param)
{
	solveThermoStatic();
}
/**************************************************************/
void solveStaticFEAThread(void* Param)
{
	solveStatic();
}
/************************************************************/
void setMainWindow()
{
	glutSetWindow( pareto_main_window );
	glutPostRedisplay();
}
/************************************************************/
void FaceTypeChanged(int control)
{
	switch (liveVar.faceType){
		case FACE_TRIANGLE: liveVar.cutoffAngle = 0;break;
		case FACE_PLANE: liveVar.cutoffAngle = 2;break;
		case FACE_FINE_CYL: liveVar.cutoffAngle = 10;break;
		case FACE_COARSE_CYL: liveVar.cutoffAngle = 15;break;
		case FACE_OTHER: liveVar.cutoffAngle = 20;break;
		case FACE_ALL: liveVar.cutoffAngle = 180;break;
	}
}
/************************************************************/
void CutoffChanged(int control)
{
	liveVar.displayMode = DISPLAY_FINAL_STL;
	setPlotRoutine(1);
	glui->sync_live();
	glutPostRedisplay();
}
/************************************************************/
void resetDisplay(int control)
{
	GLtrans[0] = 0;GLtrans[1] = 0;GLtrans[2] = 0;
	GLrot[0] = 20;GLrot[1] = 0;
	glui->sync_live();
}
/**********************************************************************/
void ElasticityBCLabelChanged(int control)
{
	int bcLabel = liveVar.CurrentElasticityBCLabel;
	int bcSet = liveVar.currentBCSet;
	double bcValue[3],bcValueNorm; 
	int bcType[3];

	liveVar.displayMode = DISPLAY_GEOM;
	//liveVar.forceRemesh = 1;
	//int nTrianglesInGroup = vox.highlightTriangleGroup(bcLabel,&area);
	glui->sync_live();
	
	// first disable everyone
	XForceText->disable();
	YForceText->disable();
	ZForceText->disable();
	NormalForceText->disable();
	PressureText->disable();
	TorqueText->disable();

	XForceText->set_float_val(0);
	YForceText->set_float_val(0);
	ZForceText->set_float_val(0);

	NormalForceText->set_float_val(0);
	PressureText->set_float_val(0);
	TorqueText->set_float_val(0);

	for(int i = 0; i < 3; i++) {
		bcType[i] = BCType[bcSet][bcLabel][i];
		bcValue[i] = BCValue[bcSet][bcLabel][i];
	}
	bcValueNorm = NORM3D(bcValue);

	if ((bcType[0] == ELASTICITY_DIRICHLET) || (bcType[1] == ELASTICITY_DIRICHLET) || (bcType[2] == ELASTICITY_DIRICHLET)) {
		liveVar.ParetoBCType= PARETO_BC_FIXED;
	}
	else if ((bcType[0] == ELASTICITY_NEUMANN) || (bcType[1] == ELASTICITY_NEUMANN) || (bcType[2] == ELASTICITY_NEUMANN)) {
		if (bcValueNorm > 0){
			liveVar.ParetoBCType= PARETO_BC_FORCE;
			XForceText->set_float_val(bcValue[0]);
			YForceText->set_float_val(bcValue[1]);
			ZForceText->set_float_val(bcValue[2]);
		}
		else
			liveVar.ParetoBCType= PARETO_BC_CLEAR;
	}
	else if ((bcType[0] == ELASTICITY_TORQUE) || (bcType[1] == ELASTICITY_TORQUE) || (bcType[2] == ELASTICITY_TORQUE)) {
		liveVar.ParetoBCType= PARETO_BC_TORQUE;
		TorqueText->set_float_val(bcValueNorm);
	}
	ParetoBCTypeChanged(1);
}
/**********************************************************************/
void pareto_setViewRoutine(int control)
{
	GLtrans[0] =0;GLtrans[1] =0; GLtrans[2] =0;
	GLLengthScale = 1.0;
	switch(liveVar.viewMode) {
case VIEW_ISO:
	GLrot[0] = 20;GLrot[1] = 0;
	break;
case VIEW_XYPLUS:
	GLrot[0] = 0;GLrot[1] = 0;
	break;
case VIEW_XYMINUS:
	GLrot[0] = 180;GLrot[1] = 180;
	break;
case VIEW_XZPLUS:
	GLrot[0] = 0;GLrot[1] = 90;
	break;
case VIEW_XZMINUS:
	GLrot[0] = 0;GLrot[1] = -90;
	break;
case VIEW_YZPLUS:
	GLrot[0] = 90;GLrot[1] = 0;
	break;
case VIEW_YZMINUS:
	GLrot[0] = -90;GLrot[1] = 0;
	break;
	}
	//pareto_myGlutDisplay();
}
/************************************************************/
void ThermalBCTypeChanged(int control)
{
	if (pareto_main_window == 0) return;
	liveVar.displayMode = DISPLAY_GEOM;

	liveVar.ThermalBCValue = 0;
	if (liveVar.ThermalBCType == PARETO_BC_TEMP){
		switch (liveVar.paretoUnits) {
			case UNITS_METER:
				ThermalBCValueText->set_name(" Temp (C)"); 
				break;
			case UNITS_MM:
				ThermalBCValueText->set_name(" Temp (C)");
				break;
			case UNITS_INCH:
				ThermalBCValueText->set_name(" Temp (F)");
				break;
		}
		liveVar.ThermalBCValue = 23;
		ThermalBCRefText->disable();
	}
	else if (liveVar.ThermalBCType == PARETO_BC_HEAT){
		switch (liveVar.paretoUnits) {
			case UNITS_METER:
				ThermalBCValueText->set_name("q (W/m^2)"); 
				break;
			case UNITS_MM:
				ThermalBCValueText->set_name("q (W/mm^2)"); 
				break;
			case UNITS_INCH:
				ThermalBCValueText->set_name("q (W/m^2)"); 
				break;
		}
		liveVar.ThermalBCValue = 100;
		ThermalBCRefText->disable();
	}
	else if (liveVar.ThermalBCType == PARETO_BC_CONVECTION){
		ThermalBCRefText->enable();
		switch (liveVar.paretoUnits) {
			case UNITS_METER:
				ThermalBCValueText->set_name("h(W/m^2-C)"); 
				break;
			case UNITS_MM:
				ThermalBCValueText->set_name("h(W/m^2-C)"); 
				break;
			case UNITS_INCH:
				ThermalBCValueText->set_name("h(W/m^2-F)"); 
				break;
		}
		liveVar.ThermalBCValue = 100;
	}
	else if (liveVar.ThermalBCType == PARETO_BC_RADIATION){
		ThermalBCValueText->set_name("emissivity:");
		liveVar.ThermalBCValue = 1.0;
		ThermalBCRefText->enable();
	}
	glui->sync_live();
	setPlotRoutine(1);
} 
/************************************************************/
void ThermalBCLabelChanged(int control)
{
	int bcLabel = liveVar.CurrentThermalBCLabel;
	int bcSet = liveVar.currentBCSet;
	int bcType;
	double bcValue,area; 


	int nTrianglesInGroup = vox.highlightTriangleGroup(bcLabel,&area);
	if (nTrianglesInGroup == 0){
		printf("No faces with label %d\n",bcLabel); 
		liveVar.ThermalBCValue = 23;
		liveVar.ThermalBCType = PARETO_BC_HEAT;
		ThermalBCTypeChanged(0);
		glui->sync_live();

		return;
	}
	printf("BC Id: %d; ",bcLabel);
	
	bcType = ThermalBCType[bcSet][bcLabel];
	bcValue = ThermalBCValue[bcSet][bcLabel];
	
	if (bcType == THERMAL_DIRICHLET) {
		liveVar.ThermalBCValue = bcValue;
		liveVar.ThermalBCType = PARETO_BC_TEMP;
		printf("Temperature fixed \n");
	}
	else if (bcType == THERMAL_NEUMANN) {
		liveVar.ThermalBCValue = bcValue/area;
		liveVar.ThermalBCType = PARETO_BC_HEAT;
		printf("Heat Flux \n");
	}
	else if (bcType == THERMAL_CONVECTION) {
		liveVar.ThermalBCType = PARETO_BC_CONVECTION;
		liveVar.ThermalBCReference = ThermalBCRef[bcSet][bcLabel];
		printf("Convection \n");
	}
	else if (bcType == THERMAL_RADIATION) {
		liveVar.ThermalBCType = PARETO_BC_RADIATION;
		liveVar.ThermalBCReference = ThermalBCRef[bcSet][bcLabel];
		printf("Radiation \n");
		//printf("%e %e \n",liveVar.ThermalBCMatl , 	liveVar.ThermalBCReference);
	}
	ThermalBCTypeChanged(0);
	glui->sync_live();
}
/************************************************************/
void DOSWindowChange(int control)
{
	if ((liveVar.DOSWindowCheck) && (!liveVar.DOSWindowOpen)) {
		AllocConsole();
		RemoveMenu(GetSystemMenu ( GetConsoleWindow (), FALSE),SC_CLOSE,MF_BYCOMMAND); 
		freopen("CONOUT$", "wb", stdout);
		MoveWindow(GetConsoleWindow(),20,620,600,200,1);
		liveVar.DOSWindowOpen = true;
		liveVar.DOSWindowOpen = true;
	}
	if ((!liveVar.DOSWindowCheck) && (liveVar.DOSWindowOpen)) {
		FreeConsole();
		liveVar.DOSWindowOpen = false;
	}
}
/************************************************************/
void ParetoBCTypeChanged(int control)
{
	if (pareto_main_window == 0) return;
	liveVar.displayMode = DISPLAY_GEOM;

	XForceText->disable();
	YForceText->disable();
	ZForceText->disable();
	NormalForceText->disable();
	PressureText->disable();
	TorqueText->disable();

	if (liveVar.ParetoBCType == PARETO_BC_FORCE){
		XForceText->enable();
		YForceText->enable();
		ZForceText->enable();
	}
	else if (liveVar.ParetoBCType == PARETO_BC_NORMAL_FORCE){
		NormalForceText->enable();
	}
	else if (liveVar.ParetoBCType == PARETO_BC_PRESSURE){
		PressureText->enable();
	}
	else if (liveVar.ParetoBCType == PARETO_BC_TORQUE){
		liveVar.cutoffAngle = 20;// only cylinders
		liveVar.faceType = FACE_FINE_CYL;
		TorqueText->enable();
	}
	else{
	}
	glui->sync_live();
	setPlotRoutine(1);

}	
/************************************************************/
void meshQualityChange(int control)
{
	switch (liveVar.meshQuality){
		case MESH_VERY_COARSE: liveVar.nElemsToMesh = 10000;break;
		case MESH_COARSE: liveVar.nElemsToMesh = 25000;break;
		case MESH_MEDIUM: liveVar.nElemsToMesh = 50000;break;
		case MESH_FINE: liveVar.nElemsToMesh = 100000;break;
		case MESH_VERY_FINE: liveVar.nElemsToMesh = 250000;break;
	}

	liveVar.nWorkSpaceGroups = MAX(64,MIN(MAX_WORKSPACE_GROUPS,NEARESTINT(liveVar.nElemsToMesh/750.0)));
	liveVar.forceRemesh = 1;
	glui->sync_live();
}

/************************************************************/
void meshChange(int control)
{
	liveVar.nWorkSpaceGroups = MAX(64,MIN(MAX_WORKSPACE_GROUPS,NEARESTINT(liveVar.nElemsToMesh/750.0)));
	liveVar.forceRemesh = 1;
	glui->sync_live();
}
/************************************************************/
void verboseChangeCallback(int control)
{

	if (liveVar.stlVerbose) vox.enableSTLVerbose();
	else vox.disableSTLVerbose();

	if (liveVar.voxVerbose) vox.enableVoxVerbose();
	else vox.disableVoxVerbose();

	if (liveVar.hexmeshVerbose) topopt.enableHexmeshVerbose();
	else topopt.disableHexmeshVerbose();

	if (liveVar.deflationVerbose) topopt.enableDeflationVerbose();
	else topopt.disableDeflationVerbose();

	if (liveVar.thermalVerbose) topopt.enableThermoElasticityVerbose();
	else topopt.disableThermoElasticityVerbose();

	if (liveVar.topoptVerbose) topopt.enableTSVerbose();
	else topopt.disableTSVerbose();

}
/************************************************************/
void buttonCallback(int control)
{
	callBackUserFunction(control);
	glui->sync_live();
	glutPostRedisplay();
}
/************************************************************/
void changeMaterial(int control)
{
	double EScale = 1.0,rhoScale = 1.0,alphaScale =1.0,KScale = 1.0,E,nu,Y,rho,alpha,K;

	if ((control == 1) && (guiState > GEOM_SUCCESFUL)) {// attempting to change units
		alert("If you need to switch units, you must reload geometry in new units\n");
		return;
	}
	if (guiState >= HEX_SUCCESFUL)
		guiState = BC_SUCCESFUL;
	glui->sync_live();
	// By default all materials properties are in m,kg,s

	if (liveVar.paretoUnits == UNITS_MM) {// convert into mm,gm,s
		EScale = 1e-6;//N per mm^2
		rhoScale = 1e-6;
		alphaScale = 1.0;
		KScale = 1.0;
	}
	else if (liveVar.paretoUnits == UNITS_INCH) {// convert into in,lb,s
		EScale = 0.2248/(39.36*39.36);//0.0001450377*386.07;
		rhoScale = 3.612729e-005;
		alphaScale = 1.0;
		KScale = 1.0;
	}
	if (liveVar.material == ALLOY_STEEL) {
		E = 2.1e11*EScale;
		nu = 0.28;
		Y = 500e6*EScale;
		rho = 7700*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 16*KScale;
	}
	else if (liveVar.material == ABS_PLASTIC) {
		E = 2.e9*EScale;
		nu = 0.39;
		Y = 42e6*EScale;
		rho = 1020*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 0.2*KScale;
	}
	else if (liveVar.material == Ti6Al4V) {
		E = 11.12e10*EScale;
		nu = 0.33;
		Y = 903.213e6*EScale;
		rho = 4326*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 1.01*KScale;
	}
	else if (liveVar.material == AL_1060) {
		E = 6.9e10*EScale;
		nu = 0.33;
		Y = 110e6*EScale;
		rho = 2700*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 205*KScale;
	}
	else if (liveVar.material == CASTIRON_GR25) {
		E = 91e9*EScale;
		nu = 0.26;
		Y = 110e6*EScale;
		rho = 7200*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 46*KScale;
	}
	else if (liveVar.material == AMS5862) {
		E = 2e11*EScale;
		nu = 0.27;
		Y = 1000e6*EScale;
		rho = 7833*rhoScale;
		alpha = 6e-6*alphaScale;
		K = 46*KScale;
	}
	else if (liveVar.material == OTHER_MATL) {
		E = 1;
		nu = 0.33;
		Y = 1.0;
		rho = 1;
		alpha = 1;
		K = 1;
	}
	liveVar.E = E;
	liveVar.nu = nu;
	liveVar.Y = Y;
	liveVar.rho = rho;
	liveVar.alpha = alpha;
	liveVar.K = K;
	glui->sync_live();
}


//**************************************************
static void update(int dx,int dy)
{
	switch(GLState) {
		case ROTATE:
			GLrot[0] += (dx * 180.0f) / 1000.0f;
			GLrot[1] += (dy * 180.0f) / 1000.0f;
			break;
		case TRANSLATE:
			GLtrans[0] += dx/1000.0f;
			GLtrans[1] -= dy/1000.0f;
			break;
		case ZOOM:
			if (abs(dy) > abs(dx)) {
				if (dy < 0)  
					GLLengthScale *= 0.98;
				else
					GLLengthScale *=1.02;
			}
			break;
	}
}
//**************************************************
void printOrientation()
{
	printf("-------------------------\n");
	printf("Rotate: %.3f %.3f \n",GLrot[0],GLrot[1]);
	printf("Translate: %.3f %.3f \n",GLtrans[0],GLtrans[1]);
	printf("Zoom: %.3f\n",GLLengthScale);
	printf("-------------------------\n");
}
//**************************************************
void setRotate(double rot[2])
{
	GLrot[0] = rot[0];
	GLrot[1] = rot[1];
}
//**************************************************
void setTranslate(double trans[2])
{
	GLtrans[0] = trans[0];
	GLtrans[1] = trans[1];
}
//**************************************************
void setZoom(double zoom)
{
	GLLengthScale = zoom;
}
/**********************************************************************/
void motion(int x, int y) 
{
	update(x-curX,y-curY);
	glutPostRedisplay();
	curX = x;
	curY = y;
}
/**************************************************************/
void	mouseToRay(int mouse_x,int mouse_y,GLdouble rayStart[3],GLdouble rayEnd[3])
// Given a 2D mouse position, return the 3D ray 
{
	GLint realy; /* OpenGL y coordinate position */

	realy = pareto_viewport[3] - (GLint) mouse_y - 1 + mouseYOffset;
	gluUnProject((GLdouble) mouse_x, (GLdouble) realy, 0.0,mvmatrix, projmatrix, pareto_viewport, &(rayStart[0]), &(rayStart[1]),&(rayStart[2]));
	gluUnProject((GLdouble) mouse_x, (GLdouble) realy, 1.0,mvmatrix, projmatrix, pareto_viewport, &(rayEnd[0]),&(rayEnd[1]), &(rayEnd[2]));

	return ;
}
/**********************************************************************/
void mouse(int button, int state, int x, int y) 
{
	static int omx, omy, mx, my,i;
	int triangle;
	double normal[3],totalArea,tMin,pt[3];
	double uvw[3],stresses[6],vonmises;
	static int saveSnap = 0;


	if (state == GLUT_UP){
		GLState = 0;//stop all changes
		glutPostRedisplay();
	}
	else {
		glutPostRedisplay();
		// check if left and right are pressed simulaneously ... useful for no middle button problem
		if ( (GLState == ROTATE) && (button == GLUT_RIGHT_BUTTON))
			button = GLUT_MIDDLE_BUTTON;
		if ( (GLState == TRANSLATE) && (button == GLUT_LEFT_BUTTON))
			button = GLUT_MIDDLE_BUTTON;
		switch (button) {
			case GLUT_LEFT_BUTTON:
				GLState = ROTATE;
				mouseToRay(x,y,currentRayStart,currentRayEnd);
				//printf("(%.2f %.2f %.2f)--->",currentRayStart[0],currentRayStart[1],currentRayStart[2]);
				//printf("(%.2f %.2f %.2f) \n",currentRayEnd[0],currentRayEnd[1],currentRayEnd[2]);

				if (liveVar.displayMode == DISPLAY_GEOM) {
					triangle = vox.findClosestTriangleIntesectingRay(currentRayStart,currentRayEnd,&tMin);
					if (triangle < 0) {
						
						break;//no triangle selected
					}
					if (vox.isTriangleHighlighted(triangle)){
						vox.resetHighlight();
						liveVar.nFacesSelected = 0;
						break;
					}
					if (glutGetModifiers()  != GLUT_ACTIVE_CTRL) {
						vox.resetHighlight();// if control key is not pressed, clear selection
						liveVar.nFacesSelected = 0;//reset
					}
					// use the ray to find triangle intersected
					liveVar.selectedPart = vox.getParentPartOfTriangle(triangle);
					vox.getNormalOfTriangle(triangle,normal);
					totalArea = 0;
					vox.highlightTrianglesRecursive(triangle,0,normal,1.0*liveVar.cutoffAngle,&totalArea);
					liveVar.nFacesSelected++;
					printf("Selected Part: %d; nDistinctFacesSelected = %d \n",liveVar.selectedPart,liveVar.nFacesSelected);
					pareto_myGlutDisplay();
				}
				else if ((liveVar.displayMode == DISPLAY_INITIAL_STRESS) ) {// for probing stress values
					triangle = vox.findClosestTriangleIntesectingRay(currentRayStart,currentRayEnd,&tMin);
					if (triangle >= 0) {
						for (int i =0; i < 3; i++)
							pt[i] = currentRayStart[i] + tMin*currentRayEnd[i];

						topopt.getElasticitySolutionAt(pt,uvw,stresses,&vonmises);
						double disp = sqrt(uvw[0]*uvw[0] + uvw[1]*uvw[1] + uvw[2]*uvw[2]);
						if (disp < 1e-30) break;
						printf("----------------------------------------\n");
						printf("Point: (%.3f, %.3f, %.3f) \n",pt[0],pt[1],pt[2]);
						printf("Displacement: %e \n",disp);
						printf("VonMises Stress: %e \n",vonmises);
						printf("----------------------------------------\n");
					}
				}
				break;
			case GLUT_MIDDLE_BUTTON:
				GLState = ZOOM;
				break;
			case GLUT_RIGHT_BUTTON:
				GLState = TRANSLATE;
				break;
			default:
				break;
		}
		
		curX = x;
		curY = y;
	}
}
/************************************************************/
void pareto_printText(int x, int y, const char string[])
{	
	glDisable( GL_LIGHTING );
	glColor3f(0,0,0);
	glRasterPos2f(x, y);
	for (int i = 0; string[i] != '\0'; ++i){
		if (string[i] == '\n'){
			y -= 20;
			glRasterPos2f(x, y);
		}
		else
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
	}
	glEnable( GL_LIGHTING );
}
/************************************************************/
void pareto_printText(int x, int y, const char string[],float color[3],void *font)
{	
	glDisable( GL_LIGHTING );
	glColor3f(color[0],color[1],color[2]);
	glRasterPos2f(x, y);
	for (int i = 0; string[i] != '\0'; ++i)
		glutBitmapCharacter(font, string[i]);
	glEnable( GL_LIGHTING );
}

//******************************************************
double distOfPtToLine(double *pt,double *p1,double *p2)
{
	double p1_p2[3],L,pt_p1[3],pt_p2[3],crs[3];
	SUBTRACT3D(p1_p2,p2,p1);
	L = NORM3D(p1_p2);

	SUBTRACT3D(pt_p1,p1,pt);
	SUBTRACT3D(pt_p2,p2,pt);
	CROSS3D(crs,pt_p1,pt_p2);
	return NORM3D(crs)/L;
}

/**************************************************************/
void pareto_glstart2D()
{
	//Assume we are in MODEL_VIEW already
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity();

	glGetIntegerv (GL_VIEWPORT, pareto_viewport);
	gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
	glDepthFunc (GL_ALWAYS);
}

/**************************************************************/
void pareto_glEnd2D()
{
	glDepthFunc (GL_LESS);
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix ();
}

/*****************************************************/
void pareto_myGlutReshape( int x, int y )
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glViewport( tx, ty, tw, th );
	pareto_xy_aspect = (float)tw / (float)th;
	glutPostRedisplay();
}
/*****************************************************/
void pareto_myGlutDisplay( void )
{
	float scale,color[]={0.0f,0.3f,1.0f};
	double center[3];

	scale = 0.5*GLLengthScale/liveVar.modelScale; // model occupies  screen

	vox.getSTLGeomCenter(center);
	glutSetWindow(pareto_main_window);
	glGetIntegerv(GL_VIEWPORT, pareto_viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	glClearColor( 1.0f, 1.0f,1.0f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(25.0,pareto_xy_aspect,0.1,200);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	GLfloat zVal = -center[2]-1.0f;
	glTranslatef(-center[0],-center[1],zVal); // 7) set Z to display
	glTranslatef(center[0],center[1],center[2]); // 6) bring back from oigin 
	glTranslatef(GLtrans[0], GLtrans[1], GLtrans[2]); // 5) translate
	glScalef( scale, scale, scale ); // 4) scale
	glRotatef(GLrot[0], 0.0f, 1.0f, 0.0f);// 3) rotate about y 
	glRotatef(GLrot[1], 1.0f, 0.0f, 0.0f);// 2) rotate about x
	glTranslatef(-center[0],-center[1],-center[2]); // 1) bring to origin 
		
	(*userPlottingFunction)();// call the primary user function
	otherPlottingFunction(); // user can call additional plotting as needed
	glEnable( GL_LIGHTING );
	/*if (liveVar.plotTitle) {
		int strLen = (int) strlen(userDefinedTitle);
		pareto_glstart2D();
		pareto_printText((int)(0.5*pareto_viewport[2] - 1.0*strLen*5),(int)(0.95*pareto_viewport[3]),userDefinedTitle);
		pareto_glEnd2D();
	}*/

	 pareto_glstart2D();
	// glRasterPos2f(10,10);
	 //t3dDraw2D(liveVar.userMessage,-1,0,0.3f);
	pareto_printText(10,10,liveVar.userMessage,color,GLUT_BITMAP_9_BY_15);
	pareto_glEnd2D();

	glPopMatrix();
	glutSwapBuffers();
	if (liveVar.plotAnimation) glutPostRedisplay();
}
/**************************************************/
void plotDataString(char str[])
{
	int strLen = (int) strlen(str);
	pareto_glstart2D();
		pareto_printText((int)(10),(int)(0.95*pareto_viewport[3]),str);
	pareto_glEnd2D();
}
/**************************************************/
void myGlutKeyboard(unsigned char Key, int x, int y)
{
	switch(Key){
	case 27: 
		GLtrans[0] = 0;GLtrans[1] = 0;GLtrans[2] = 0;
		GLrot[0] = 20;GLrot[1] = 0;
		GLLengthScale = 1.0;
		break;
	case 'a': 
		if (liveVar.plotAxis) liveVar.plotAxis = 0;
		else liveVar.plotAxis = 1;
		break;
	case 'x': 
		if (liveVar.boundingBox) liveVar.boundingBox = 0;
		else liveVar.boundingBox = 1;
		break;
	case 'b': 
		if (liveVar.plotElasticityBC) liveVar.plotElasticityBC = 0;
		else liveVar.plotElasticityBC = 1;
		break;
	case 'o':// zoom out
		GLLengthScale *= 0.98;
		break;
	case 'i':// zoom out
		GLLengthScale *= 1.02;
		break;
	case 's': 
		if (liveVar.plotSymmetryPlanes) liveVar.plotSymmetryPlanes = 0;
		else liveVar.plotSymmetryPlanes = 1;
		break;
	case 't': 
		if (liveVar.geomTransparent) liveVar.geomTransparent = 0;
		else liveVar.geomTransparent = 1;
		break;
	case 'w': 
		if (liveVar.displayEdges) liveVar.displayEdges = 0;
		else liveVar.displayEdges = 1;
		break;
	};

	glutPostRedisplay();
	glui->sync_live();
}
/************************************************************/
void pareto_quiet()
{
	pareto_myGlutDisplay();
}
/************************************************************/
void pareto_redraw()
{
	(*userPlottingFunction)();
}

/************************************************************/
void handleResize(int w, int h) 
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}
/************************************************************/
void specialKeyFunction(int key, int x, int y)
{
	double deg = 2;
	switch (key) {
	case 100: // left arrow
		GLrot[0] -= deg;
		break;
	case 101: // up arrow
		GLrot[1] -= deg;
		break;
	case 102: // right arrow
		GLrot[0] += deg;
		break;
	case 103: // down arrow
		GLrot[1] += deg;
		break;
	}
	glutPostRedisplay();
}
/************************************************************/
void pareto_initializeWindows(char title[])
{
	int argc =2;
	char *argv[] = {"test"};
	char temp[50];

	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/
	GLfloat light0_ambient[] =  {0.f, 0.f, 0.f, 1.0f};
	GLfloat light0_specular[] =  {1.0f,1.0f, 1.0f, 1.0f};
	GLfloat light0_diffuse[] =  {1.f, 1.0f, 1.0f, 1.0f};
	GLfloat light0_position[] = {10.5f, 10.5f, 10.0f, 0.0f};

	GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light1_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
	GLfloat light1_position[] = {-1.5f, 0.0f,1.0f, 0.0f};

	GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 250, 50 );
	glutInitWindowSize( 1100, 800 ); 

	sprintf(temp,"%.2f", PARETO_VERSION);
	strcat(title,temp);
	sprintf(temp,"; Build Date: %.2f",PARETO_BUILD_DATE);
	strcat(title,temp);


	pareto_main_window = glutCreateWindow(title);

	if (!t3dInit()) printf("Could not find charset \n");

	glEnable(GL_LIGHTING);
	glEnable( GL_NORMALIZE );
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	glutIdleFunc(pareto_myGlutDisplay);
	glutDisplayFunc(pareto_myGlutDisplay);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(myGlutKeyboard);
	glutSpecialFunc(specialKeyFunction);

	GLUI_Master.set_glutReshapeFunc( pareto_myGlutReshape );  

}
/************************************************************/
#if COMPILE_MODE != BATCH_MODE
int GetFileNameToOpen(char *filter, char *ext,char szFileName[])
{
	szFileName[0] = '\0';
	OPENFILENAME ofn = {0};
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);

	ofn.lpstrFilter = filter;
	//ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFileName;
	char *pPath = getenv ("SCIARTHOME");
	if (pPath==NULL)
		;//printf ("Set SCIARTHOME environment variable for easy navigation");
	else
		ofn.lpstrInitialDir = pPath;

	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;

	if(GetOpenFileName(&ofn))
		return SUCCESS;
	else 
		return FAILURE;

}
/**************************************************************/
int GetFileNameToSave(char *filter, char *ext,char szFileName[])
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = filter;
	//ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFileName;
	char *pPath = getenv ("SCIARTHOME");
	if (pPath==NULL)
		;//printf ("Set SCIARTHOME environment variable for easy navigation");
	else
		ofn.lpstrInitialDir = pPath;

	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ext;

	if(GetSaveFileName(&ofn)){
		return SUCCESS;
	}
	else {
		return FAILURE;
	}
}
/**********************************************************************/
void SaveAsTIFF(const char *fileName)
{
	/*
	GLubyte *data=NULL;
	GLint viewPort[4];
	float dpi;

	dpi = liveVar.tiffDPI;
	glGetIntegerv(GL_VIEWPORT,viewPort);

	data=(GLubyte *) malloc(viewPort[2] * viewPort[3] * sizeof(GLubyte) * 3);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0,0,viewPort[2],viewPort[3],GL_RGB,GL_UNSIGNED_BYTE,data);

	TIFF *out= TIFFOpen(fileName, "w");
	uint32 width=viewPort[2];
	uint32 height=viewPort[3];
	int sampleperpixel = 3;   
	TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height); 
	TIFFSetField(out, TIFFTAG_XRESOLUTION, dpi);
	TIFFSetField(out, TIFFTAG_YRESOLUTION, dpi);
	TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);   
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE,8);    
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   

	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);
	

	tsize_t linebytes = sampleperpixel * width; 
	unsigned char *buf = NULL;       
	buf =(unsigned char *)_TIFFmalloc(linebytes);
	
	for (uint32 row = 0; row < height; row++){
		memcpy(buf, &data[(height-row-1)*linebytes], linebytes);   
		TIFFWriteScanline(out, buf, row, 0);
	}
	
	(void) TIFFClose(out);
	if (buf)
    _TIFFfree(buf);
	free(data);
	*/
}

/**************************************************************/
void SaveAsBMP(const char *fileName)
{
	FILE *file;
	unsigned long imageSize;
	GLbyte *data=NULL;
	GLint viewPort[4];
	GLenum lastBuffer;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	bmfh.bfType='MB';
	bmfh.bfReserved1=0;
	bmfh.bfReserved2=0;
	bmfh.bfOffBits=54;
	glGetIntegerv(GL_VIEWPORT,viewPort);
	imageSize=((viewPort[2]+((4-(viewPort[2]%4))%4))*viewPort[3]*3)+2;
	bmfh.bfSize=imageSize+sizeof(bmfh)+sizeof(bmih);
	data=(GLbyte*)malloc(imageSize);
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	glPixelStorei(GL_PACK_ROW_LENGTH,0);
	glPixelStorei(GL_PACK_SKIP_ROWS,0);
	glPixelStorei(GL_PACK_SKIP_PIXELS,0);
	glPixelStorei(GL_PACK_SWAP_BYTES,1);
	glGetIntegerv(GL_READ_BUFFER,(GLint*)&lastBuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0,0,viewPort[2],viewPort[3],GL_BGR_EXT,GL_UNSIGNED_BYTE,data);
	data[imageSize-1]=0;
	data[imageSize-2]=0;
	glReadBuffer(lastBuffer);
	file=fopen(fileName,"wb");
	bmih.biSize=40;
	bmih.biWidth=viewPort[2];
	bmih.biHeight=viewPort[3];
	bmih.biPlanes=1;
	bmih.biBitCount=24;
	bmih.biCompression=0;
	bmih.biSizeImage=imageSize;
	bmih.biXPelsPerMeter=4589;
	bmih.biYPelsPerMeter=4589;
	bmih.biClrUsed=0;
	bmih.biClrImportant=0;
	fwrite(&bmfh,sizeof(bmfh),1,file);
	fwrite(&bmih,sizeof(bmih),1,file);
	fwrite(data,imageSize,1,file);
	free(data);
	fclose(file);
}
#else
int GetFileNameToOpen(char *filter, char *ext,char szFileName[])
{
	return 0;
}
int GetFileNameToSave(char *filter, char *ext,char szFileName[])
{
	return 0;
}
void SaveAsBMP(const char *fileName)
{
	;
}
#endif

/************************************************************/
void setPlotRoutine(int control)
{
	int isoCounter;
	if (liveVar.usingGUI==0)
		return; // Nothing to display

	userPlottingFunction = plotGeometry;//default
	switch(liveVar.displayMode) {
case DISPLAY_NOTHING:
	userPlottingFunction = plotNothing;
	break;
case DISPLAY_GEOM:
	if (guiState >= GEOM_SUCCESFUL) userPlottingFunction = plotGeometry;
	break;
case DISPLAY_MESH:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotMesh;
	break;
case DISPLAY_MESH_SURFACE:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotMeshSurface;
	break;
case DISPLAY_MESH_SURFACE_PSEUDODENSITY:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotMeshSurfacePseudoDensity;
	break;
case DISPLAY_PSEUDODENSITY_MESH:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotMeshWithPseudoDensity;
	break;
case DISPLAY_TEMP_ON_MESH:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotTemperatureOverMesh;
	break;
case DISPLAY_DELTA_OVER_MESH:
case DISPLAY_STRESS_OVER_MESH:
	if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotFieldOverMesh;
	break;
case DISPLAY_MESH_COMPONENTS:
	//if (guiState >= HEX_SUCCESFUL) 	userPlottingFunction = plotMeshComponents;
	break;
case DISPLAY_INITIAL_DELTA:
	if ((guiState >= STATICFEA_SUCCESFUL))
		userPlottingFunction = plotFieldOverSTL;
	else if (guiState >= GEOM_SUCCESFUL) 
		userPlottingFunction = plotGeometry;
	break;
case DISPLAY_INITIAL_STRESS:
	if ((guiState >= STATICFEA_SUCCESFUL))
		userPlottingFunction = plotFieldOverSTL;
	else if (guiState >= GEOM_SUCCESFUL) 
		userPlottingFunction = plotGeometry;
	break;
case DISPLAY_MODAL:
	if ((guiState == MODALFEA_SUCCESFUL) && (topopt.getCurrentIsoCounter() < 0))
		userPlottingFunction = plotEigenModeOverSTL;
	else if ((guiState == MODALFEA_SUCCESFUL) && (topopt.getCurrentIsoCounter() >= 0))
		userPlottingFunction = plotGeometry;
	else if (guiState >= GEOM_SUCCESFUL) 
		userPlottingFunction = plotGeometry;
	break;
case DISPLAY_TEMPERATURE:
	if (guiState >= THERMALFEA_SUCCESFUL)
		userPlottingFunction = plotFieldOverSTL;
	break;
case DISPLAY_ISO:
	if (guiState == TOPOPT_SUCCESFUL) userPlottingFunction = plotIsoSurface;
	break;
case DISPLAY_MESHERROR:
	userPlottingFunction = plotFieldOverSTL;
	break;
case DISPLAY_CONTOUR:
	//userPlottingFunction = plotMeshContour;
	break;
case DISPLAY_TSFIELD: 
	if (guiState >= STATICFEA_SUCCESFUL) userPlottingFunction = plotTopSensField;
	break;
case DISPLAY_ISO_INVERTED:
	if (guiState >= HEX_SUCCESFUL) userPlottingFunction = plotIsoSurfaceInverted;
	break;
case DISPLAY_SPHEREMAP:
	if (guiState >= GEOM_SUCCESFUL) userPlottingFunction = plotSphereMap;
	break;
case DISPLAY_DEFLATION_GROUPS: 
	if (guiState >= STATICFEA_SUCCESFUL) userPlottingFunction = plotWorkSpacePatch;
	break;
case DISPLAY_FINAL_STL: 
	isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction);
	if (topopt.getNumIsoTriangles(isoCounter) > 0) userPlottingFunction = plotFinalSTL;
	break;
case DISPLAY_FINAL_DELTA: 
	isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction);
	if (topopt.getNumIsoTriangles(isoCounter) > 0) userPlottingFunction = plotFieldOverFinalSTL;
	break;
case DISPLAY_FINAL_STRESS: 
	isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction);
	if (topopt.getNumIsoTriangles(isoCounter) > 0) userPlottingFunction = plotFieldOverFinalSTL;
	break;
	}

	//if (liveVar.usingGUI) pareto_myGlutDisplay();
}
/************************************************************/
void GUI_ParetoWin()
{
	int argc = 1;
	char title[256];
	char *argv[] = {"dummy argument"};

	userPlottingFunction = &plotNothing;
	glutInit(&argc, argv);

	strcpy(title,"ParetoWin: Version ");

	pareto_initializeWindows(title);
	GLUI_EditText *edittext;
	GLUI_Spinner *spinner;
	GLUI_Checkbox *checkbox;
	GLUI_Rollout *rollout;
	GLUI_Listbox *list;
	GLUI_Button *btn;
	
/***********************************************************************************/
	/*** Create the side subwindow ***/
	glui = GLUI_Master.create_glui_subwindow( pareto_main_window, GLUI_SUBWINDOW_RIGHT );
	glui->bkgd_color.set(187, 211, 249);

	
/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Geometry & Units", 0 );
	rollout->set_w(220);
	rollout->set_font(GLUT_BITMAP_9_BY_15);

	//checkbox = new GLUI_Checkbox( rollout, "Open Console", &(liveVar.DOSWindowCheck),-1,DOSWindowChange );
	list = new GLUI_Listbox(rollout,"Units:",&(liveVar.paretoUnits),1,changeMaterial);
	list->add_item(UNITS_METER,"m,N,s");
	list->add_item(UNITS_INCH,"in,lb,s");
	list->add_item(UNITS_MM,"mm,N,s");
	

	btn = new GLUI_Button( rollout, "Load ascii/binary stl", LOADGEOM_BUTTON,buttonCallback );	
	btn = new GLUI_Button( rollout, "Update stl", RELOADGEOM_BUTTON,buttonCallback );	
	rollout->set_w(220);
	
	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Material", 0 );
	rollout->set_w(220);
	list = new GLUI_Listbox(rollout,"Material:",&(liveVar.material),2,changeMaterial);
	list->add_item(ALLOY_STEEL,materialNamesStr[ALLOY_STEEL]);
	list->add_item(Ti6Al4V,materialNamesStr[Ti6Al4V]);
	list->add_item(ABS_PLASTIC,materialNamesStr[ABS_PLASTIC]);
	list->add_item(AL_1060,materialNamesStr[AL_1060]);
	list->add_item(CASTIRON_GR25,materialNamesStr[CASTIRON_GR25]);
	list->add_item(AMS5862,materialNamesStr[AMS5862]);
	list->add_item(OTHER_MATL,materialNamesStr[OTHER_MATL]);

	edittext = new GLUI_EditText(rollout,"E: ",&(liveVar.E));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	liveVar.E = 2.1e11;
	edittext = new GLUI_EditText(rollout,"nu: ",&(liveVar.nu));
	edittext->set_w(200);
	edittext->set_float_limits(0,0.45);
	liveVar.nu = 0.28;
	edittext = new GLUI_EditText(rollout,"Y: ",&(liveVar.Y));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);


	edittext = new GLUI_EditText(rollout,"rho: ",&(liveVar.rho));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	liveVar.rho =7700.0;
	
	checkbox = new GLUI_Checkbox( rollout, "Do not optimize?", &(liveVar.nonDesignCheck) );
	new GLUI_Button( rollout, "Apply", MATERIAL_BUTTON,buttonCallback );
	
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Boundary Conditions", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Load Set:  ",&(liveVar.currentBCSet),BCSET_SPINNER,ElasticityBCLabelChanged);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);

	list = new GLUI_Listbox(rollout,"Select:",&(liveVar.faceType),-1,(GLUI_Update_CB)FaceTypeChanged );
	list->add_item(FACE_TRIANGLE,"Triangle");
	list->add_item(FACE_PLANE,"Plane");
	list->add_item(FACE_FINE_CYL,"Fine Cylin");
	list->add_item(FACE_COARSE_CYL,"Coarse Cylin");
	list->add_item(FACE_OTHER,"Other");

	list = new GLUI_Listbox(rollout,"BCType:",&(liveVar.ParetoBCType),-1,(GLUI_Update_CB)ParetoBCTypeChanged );
	list->add_item(PARETO_BC_CLEAR,"None");
	list->add_item(PARETO_BC_FIXED,"Fixed");
	list->add_item(PARETO_BC_XFIXED,"XFixed");
	list->add_item(PARETO_BC_YFIXED,"YFixed");
	list->add_item(PARETO_BC_ZFIXED,"ZFixed");
	list->add_item(PARETO_BC_FORCE,"Force");
	list->add_item(PARETO_BC_NORMAL_FORCE,"NormalForce");
	list->add_item(PARETO_BC_PRESSURE,"Pressure");
	list->add_item(PARETO_BC_TORQUE,"Torque");
	list->add_item(PARETO_BC_SLIDING,"Sliding");
	list->add_item(PARETO_BC_RETAIN,"Retain");


	XForceText = new GLUI_EditText(rollout,"XForce: ",&(liveVar.XForce));
	XForceText->set_w(200);
	XForceText->disable();
	YForceText = new GLUI_EditText(rollout,"YForce: ",&(liveVar.YForce));
	YForceText->set_w(200);
	YForceText->disable();
	ZForceText = new GLUI_EditText(rollout,"ZForce: ",&(liveVar.ZForce));
	ZForceText->set_w(200);
	ZForceText->disable();
	NormalForceText = new GLUI_EditText(rollout,"NForce: ",&(liveVar.NormalForce));
	NormalForceText->set_w(200);
	NormalForceText->disable();
	PressureText = new GLUI_EditText(rollout,"Pressure: ",&(liveVar.Pressure));
	PressureText->set_w(200);
	PressureText->disable();
	TorqueText = new GLUI_EditText(rollout,"Torque: ",&(liveVar.Torque));
	TorqueText->set_w(200);
	TorqueText->disable();

	new GLUI_Button( rollout, "ApplyBC", APPLY_NEWELASTICITY_BC_BUTTON,buttonCallback );


		/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Acceleration", 0 );
	rollout->set_w(220);
	
	edittext = new GLUI_EditText(rollout,"X(L/s^2): ",&(liveVar.XAcceleration));
	edittext->set_w(200);
	edittext = new GLUI_EditText(rollout,"Y(L/s^2): ",&(liveVar.YAcceleration));
	edittext->set_w(200);
	edittext = new GLUI_EditText(rollout,"Z(L/s^2): ",&(liveVar.ZAcceleration));
	edittext->set_w(200);

	new GLUI_Button( rollout, "Apply", APPLY_ACCELERATION_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Display Options", 0 );
	rollout->set_w(220);

	list = new GLUI_Listbox(rollout,"Display",&(liveVar.displayMode),1,setPlotRoutine);
	list->add_item(DISPLAY_GEOM,"Geometry");
	list->add_item(DISPLAY_INITIAL_DELTA,"InitialDisp");
	list->add_item(DISPLAY_INITIAL_STRESS,"InitialStress");
	list->add_item(DISPLAY_FINAL_STL,"FinalSTL");
	list->add_item(DISPLAY_FINAL_DELTA,"FinalDisp");
	list->add_item(DISPLAY_FINAL_STRESS,"FinalStress");


	list = new GLUI_Listbox(rollout,"Views:",&(liveVar.viewMode),1,pareto_setViewRoutine);
	list->add_item(VIEW_ISO,"Iso");
	list->add_item(VIEW_XYPLUS,"XY+");
	list->add_item(VIEW_XYMINUS,"XY-");
	list->add_item(VIEW_XZPLUS,"XZ+");
	list->add_item(VIEW_XZMINUS,"XZ-");
	list->add_item(VIEW_YZPLUS,"YZ+");
	list->add_item(VIEW_YZMINUS,"YZ-");

	checkbox = new GLUI_Checkbox( rollout, "Transparent", &(liveVar.geomTransparent) );
	checkbox = new GLUI_Checkbox( rollout, "TriangleEdges", &(liveVar.displayEdges) );
	checkbox = new GLUI_Checkbox( rollout, "Axis", &(liveVar.plotAxis) );
	checkbox = new GLUI_Checkbox( rollout, "Animate", &(liveVar.plotAnimation));
	checkbox = new GLUI_Checkbox( rollout, "Elasticity BC", &(liveVar.plotElasticityBC) );
	checkbox = new GLUI_Checkbox( rollout, "SymmetryPlanes", &(liveVar.plotSymmetryPlanes));
	checkbox = new GLUI_Checkbox( rollout, "ConcaveEdges", &(liveVar.showConcaveEdges));

	new GLUI_Button( rollout, "Reset View", RESET_DISPLAY_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Save TIF", SAVE_TIF_BUTTON,buttonCallback );


	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "FEA", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Load Set:  ",&(liveVar.currentBCSet),BCSET_SPINNER,ElasticityBCLabelChanged);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);
	
	list = new GLUI_Listbox(rollout,"Quality:",&(liveVar.meshQuality),1,meshQualityChange);
	list->add_item(MESH_VERY_COARSE,"Very Coarse");
	list->add_item(MESH_COARSE,"Coarse");
	list->add_item(MESH_MEDIUM,"Medium");
	list->add_item(MESH_FINE,"Fine");
	list->add_item(MESH_VERY_FINE,"Very Fine");


	edittext = new GLUI_EditText(rollout,"#Elements: ",&(liveVar.nElemsToMesh),-1,meshChange);
	edittext->set_w(200);
	edittext->set_int_limits(10,vox.getMaxVoxels());
	checkbox = new GLUI_Checkbox( rollout, "Remesh?", &(liveVar.forceRemesh));

	
	XSymCheckbox = new GLUI_Checkbox( rollout, "X Symmetry", &(liveVar.xSym),-1,meshChange );
	YSymCheckbox = new GLUI_Checkbox( rollout, "Y Symmetry", &(liveVar.ySym),-1,meshChange );
	ZSymCheckbox = new GLUI_Checkbox( rollout, "Z Symmetry", &(liveVar.zSym),-1,meshChange );

	
	new GLUI_Button( rollout, "Static Analysis", SOLVE_ELASTOSTATICS_BUTTON,buttonCallback );
	


	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "TopOpt Constraints", 0 );
	rollout->set_w(220);
	list = new GLUI_Listbox(rollout,"DrawDirection:",&(liveVar.drawDirection));
	list->add_item(0,"None");
	list->add_item(1,"XDir");
	list->add_item(2,"YDir");
	list->add_item(3,"ZDir");


	list = new GLUI_Listbox(rollout,"ThroughCutDir:",&(liveVar.throughCutDirection));
	list->add_item(0,"None");
	list->add_item(1,"XDir");
	list->add_item(2,"YDir");
	list->add_item(3,"ZDir");

	list = new GLUI_Listbox(rollout,"Cyclic(Z):",&(liveVar.cyclicSymmetry));
	list->add_item(0,"None");
	//list->add_item(1,"Full");
	//list->add_item(2,"(2)180Deg");
	list->add_item(3,"(3)120Deg");
	list->add_item(4,"(4)90Deg");
	list->add_item(5,"(5)72Deg");
	list->add_item(6,"(6)60Deg");
	list->add_item(7,"(7)51Deg");
	list->add_item(8,"(8)45Deg");

	spinner = new GLUI_Spinner( rollout,"RelMinFeatSize: ",&(liveVar.nSmoothingIterations));
	spinner->set_int_limits(1,10);
	spinner->set_w(20);


	edittext = new GLUI_EditText(rollout,"DispLimit:",&liveVar.maxDisplacementAllowed);
	edittext->set_w(200);
	edittext->set_float_limits(1e-20,1e20);

	edittext = new GLUI_EditText(rollout,"StressSF:",&liveVar.stressSafetyFactor);
	edittext->set_w(200);
	edittext->set_float_limits(1,100);

	checkbox = new GLUI_Checkbox( rollout, "Keep Fixed Faces", &(liveVar.keepDirichlet) );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Optimize", 0 );
	rollout->set_w(220);
	checkbox = new GLUI_Checkbox( rollout, "Use All Loads", &(liveVar.useAllLoads) );

	spinner = new GLUI_Spinner( rollout,"Use Load Set: ",&(liveVar.currentBCSet),BCSET_SPINNER,ElasticityBCLabelChanged);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);

	edittext = new GLUI_EditText(rollout,"DesVolFrac:",&liveVar.desiredVolFraction);
	edittext->set_w(200);
	edittext->set_float_limits(0.01,0.99);

	edittext = new GLUI_EditText(rollout,"StepSize:",&liveVar.stepSize);
	edittext->set_w(200);
	edittext->set_float_limits(0.01,0.05);



	checkbox = new GLUI_Checkbox( rollout, "Save Intermediate?", &(liveVar.saveIntermediate) );
	new GLUI_Button( rollout, "Optimize", OPTIMIZE_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Optimization Results", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Topology@Vol:", &(liveVar.displayVolFraction),-1,(GLUI_Update_CB)CutoffChanged);
	spinner->set_float_limits(0.01,1.0);
	new GLUI_Button( rollout, "Save topology (.stl)", SAVE_STL_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Lattice", 0 );
	rollout->set_w(220);

	list = new GLUI_Listbox(rollout,"LatticeType:",&(liveVar.latticeType));
	list->add_item(BOX,"BOX");
	list->add_item(BCC,"BCC*");
	list->add_item(BOX_BCC,"BOXBCC");
	list->add_item(BCCz,"BCCz*");
	list->add_item(FCC,"FCC*");
	list->add_item(FCCz,"FCCz*");
	list->add_item(FCCBCC,"FCCBCC*");
	list->add_item(OCTET,"OCTET");
	
	spinner = new GLUI_Spinner( rollout,"UnitSize(mm):", &(liveVar.latticeUnitCellSizeInMM));
	spinner->set_int_limits(1,20);

	spinner = new GLUI_Spinner( rollout,"EdgeDivision:", &(liveVar.latticeResolution));
	spinner->set_int_limits(4,100);

	spinner = new GLUI_Spinner( rollout,"FillRatio:", &(liveVar.latticeFillRatio));
	spinner->set_float_limits(0.01,1);

	list = new GLUI_Listbox(rollout,"RetainSurf:",&(liveVar.latticeSurfaceRetainType));
	list->add_item(LATTICE_SURFACE_NONE,"None");
	list->add_item(LATTICE_SURFACE_CRITICAL,"Critical");
	list->add_item(LATTICE_SURFACE_ALL,"All");

	spinner = new GLUI_Spinner( rollout,"Smoothening:", &(liveVar.latticeSmoothening));
	spinner->set_int_limits(1,50);
		
	checkbox = new GLUI_Checkbox( rollout, "Save STL?", &(liveVar.latticeSaveSTL) );

	new GLUI_Button( rollout, "Create Lattice", CREATE_LATTICE_MESH_BUTTON,buttonCallback );


	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Projects", 0 );
	rollout->set_w(220);

	//glui->add_separator_to_panel(rollout);
	new GLUI_Button( rollout, "Save current project ", SAVEPROJECT_BUTTON,buttonCallback );
	//glui->add_separator_to_panel(rollout);
	new GLUI_Button( rollout, "Load .prj file ...   ", LOADPROJECT_BUTTON,buttonCallback );

	new GLUI_Button( glui, "HELP", HELP_BUTTON,(GLUI_Update_CB)buttonCallback );
	new GLUI_Button( glui, "QUIT", 0,(GLUI_Update_CB)exit );
	

	
	/***********************************************************************************/
	glui->set_main_gfx_window( pareto_main_window );
	glui->sync_live();
	glShadeModel(GL_SMOOTH);		// Default

	setPlotRoutine(1);
	glutMainLoop();
}
/************************************************************/
void GUI_ParetoResearch()
{
	int argc = 1;
	char title[256];
	char *argv[] = {"dummy argument"};

	userPlottingFunction = &plotNothing;
	glutInit(&argc, argv);
	strcpy(title,"PareTO; Version ");

	pareto_initializeWindows(title);
	GLUI_EditText *edittext;
	GLUI_Spinner *spinner;
	GLUI_Checkbox *checkbox;
	GLUI_Rollout *rollout;
	GLUI_Listbox *list;
	GLUI_Button *btn;

	/*** Create the side subwindow ***/
	glui = GLUI_Master.create_glui_subwindow( pareto_main_window, GLUI_SUBWINDOW_RIGHT );
	glui->bkgd_color.set(187, 211, 249);//(200,210,225);
	 
	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Verbose Options", 0 );
	rollout->set_w(220);
	rollout->set_font(GLUT_BITMAP_9_BY_15);

	checkbox = new GLUI_Checkbox( rollout, "STL Verbose Enable", &(liveVar.stlVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Vox Verbose Enable", &(liveVar.voxVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Elast Verbose Enable", &(liveVar.hexmeshVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Defl Verbose Enable", &(liveVar.deflationVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Therm Verbose Enable", &(liveVar.thermalVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "TS Verbose Enable", &(liveVar.topoptVerbose),-1,verboseChangeCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Geometry & Units", 0 );
	rollout->set_w(220);
	rollout->set_font(GLUT_BITMAP_9_BY_15);

	list = new GLUI_Listbox(rollout,"Units:",&(liveVar.paretoUnits),1,changeMaterial);
	list->add_item(UNITS_METER,"m,N,s");
	list->add_item(UNITS_INCH,"in,lb,s");
	list->add_item(UNITS_MM,"mm,N,s");
	

	//checkbox = new GLUI_Checkbox( rollout, "Assembly/MultiBody", &(liveVar.assemblyMode) );
	btn = new GLUI_Button( rollout, "Load ascii/binary stl", LOADGEOM_BUTTON,buttonCallback );	
	btn = new GLUI_Button( rollout, "Update stl", RELOADGEOM_BUTTON,buttonCallback );	
	btn = new GLUI_Button( rollout, "Save geom as obj", SAVEGEOM_AS_OBJ_BUTTON,buttonCallback );	
	
	rollout->set_w(220);

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Material", 0 );
	rollout->set_w(220);
	list = new GLUI_Listbox(rollout,"Material:",&(liveVar.material),2,changeMaterial);
	list->add_item(ALLOY_STEEL,materialNamesStr[ALLOY_STEEL]);
	list->add_item(Ti6Al4V,materialNamesStr[Ti6Al4V]);
	list->add_item(ABS_PLASTIC,materialNamesStr[ABS_PLASTIC]);
	list->add_item(AL_1060,materialNamesStr[AL_1060]);
	list->add_item(AMS5862,materialNamesStr[AMS5862]);
	list->add_item(OTHER_MATL,materialNamesStr[OTHER_MATL]);

	edittext = new GLUI_EditText(rollout,"E: ",&(liveVar.E));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	liveVar.E = 2.1e11;

	edittext = new GLUI_EditText(rollout,"nu: ",&(liveVar.nu));
	edittext->set_w(200);
	edittext->set_float_limits(0,0.45);
	liveVar.nu = 0.28;

	edittext = new GLUI_EditText(rollout,"Y: ",&(liveVar.Y));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);

	edittext = new GLUI_EditText(rollout,"rho: ",&(liveVar.rho));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);

	edittext = new GLUI_EditText(rollout,"K: ",&(liveVar.K));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);

	edittext = new GLUI_EditText(rollout,"alpha: ",&(liveVar.alpha));
	edittext->set_w(200);
	edittext->set_float_limits(1e-12,1e20);

	checkbox = new GLUI_Checkbox( rollout, "Do not optimize?", &(liveVar.nonDesignCheck) );
	new GLUI_Button( rollout, "Apply", MATERIAL_BUTTON,buttonCallback );


	new GLUI_Button( rollout, "LoadDMatrix", LOAD_DMATRICES_BUTTON,buttonCallback );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Elasticity BC", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Load Set:  ",&(liveVar.currentBCSet),BCSET_SPINNER,buttonCallback);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);

	list = new GLUI_Listbox(rollout,"Select:",&(liveVar.faceType),-1,(GLUI_Update_CB)FaceTypeChanged );
	list->add_item(FACE_TRIANGLE,"Triangle");
	list->add_item(FACE_PLANE,"Plane");
	list->add_item(FACE_FINE_CYL,"Fine Cylin");
	list->add_item(FACE_COARSE_CYL,"Coarse Cylin");
	list->add_item(FACE_OTHER,"Other");
	list->add_item(FACE_ALL,"All");

		list = new GLUI_Listbox(rollout,"BCType:",&(liveVar.ParetoBCType),-1,(GLUI_Update_CB)ParetoBCTypeChanged );
	list->add_item(PARETO_BC_CLEAR,"None");
	list->add_item(PARETO_BC_FIXED,"Fixed");
	list->add_item(PARETO_BC_XFIXED,"XFixed");
	list->add_item(PARETO_BC_YFIXED,"YFixed");
	list->add_item(PARETO_BC_ZFIXED,"ZFixed");
	list->add_item(PARETO_BC_FORCE,"Force");
	list->add_item(PARETO_BC_NORMAL_FORCE,"NormalForce");
	list->add_item(PARETO_BC_PRESSURE,"Pressure");
	list->add_item(PARETO_BC_TORQUE,"Torque"); 
	list->add_item(PARETO_BC_SLIDING,"Sliding");
	list->add_item(PARETO_BC_RETAIN,"Retain");


	XForceText = new GLUI_EditText(rollout,"XForce: ",&(liveVar.XForce));
	XForceText->set_w(200);
	XForceText->disable();
	YForceText = new GLUI_EditText(rollout,"YForce: ",&(liveVar.YForce));
	YForceText->set_w(200);
	YForceText->disable();
	ZForceText = new GLUI_EditText(rollout,"ZForce: ",&(liveVar.ZForce));
	ZForceText->set_w(200);
	ZForceText->disable();
	NormalForceText = new GLUI_EditText(rollout,"NForce: ",&(liveVar.NormalForce));
	NormalForceText->set_w(200);
	NormalForceText->disable();
	PressureText = new GLUI_EditText(rollout,"Pressure: ",&(liveVar.Pressure));
	PressureText->set_w(200);
	PressureText->disable();
	TorqueText = new GLUI_EditText(rollout,"Torque: ",&(liveVar.Torque));
	TorqueText->set_w(200);
	TorqueText->disable();

	new GLUI_Button( rollout, "ApplyBC", APPLY_NEWELASTICITY_BC_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Thermal BC", 0 );
	rollout->set_w(220);

	list = new GLUI_Listbox(rollout,"Select:",&(liveVar.faceType),-1,(GLUI_Update_CB)FaceTypeChanged );
	list->add_item(FACE_TRIANGLE,"Triangle");
	list->add_item(FACE_PLANE,"Plane");
	list->add_item(FACE_FINE_CYL,"Fine Cylin");
	list->add_item(FACE_COARSE_CYL,"Coarse Cylin");
	list->add_item(FACE_OTHER,"Other");
	list->add_item(FACE_ALL,"All");


	list = new GLUI_Listbox(rollout,"BCType:",&(liveVar.ThermalBCType),-1,(GLUI_Update_CB)ThermalBCTypeChanged );
	list->add_item(PARETO_BC_TEMP,"Temperature");
	list->add_item(PARETO_BC_HEAT,"HeatFlux");
	list->add_item(PARETO_BC_CONVECTION,"Convection");
	list->add_item(PARETO_BC_RADIATION,"Radiation");

	ThermalBCValueText = new GLUI_EditText(rollout,"Temp (C): ",&(liveVar.ThermalBCValue));
	ThermalBCValueText->set_w(200);

	ThermalBCRefText = new GLUI_EditText(rollout,"TRef: ",&(liveVar.ThermalBCReference));
	ThermalBCRefText->set_w(200);
	ThermalBCRefText->disable();

	new GLUI_Button( rollout, "ApplyThermalBC", APPLY_THERMAL_BC_BUTTON,buttonCallback );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Acceleration", 0 );
	rollout->set_w(220);

	edittext = new GLUI_EditText(rollout,"X(L/s^2): ",&(liveVar.XAcceleration));
	edittext->set_w(200);
	edittext = new GLUI_EditText(rollout,"Y(L/s^2): ",&(liveVar.YAcceleration));
	edittext->set_w(200);
	edittext = new GLUI_EditText(rollout,"Z(L/s^2): ",&(liveVar.ZAcceleration));
	edittext->set_w(200);


	edittext = new GLUI_EditText(rollout,"w_z(rad/s): ",&(liveVar.radiansPerSecZAxis));
	edittext->set_w(200);

	new GLUI_Button( rollout, "Apply", APPLY_ACCELERATION_BUTTON,buttonCallback );

	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Display Options", 0 );
	list = new GLUI_Listbox(rollout,"Display:",&(liveVar.displayMode),1,setPlotRoutine);
	list->add_item(DISPLAY_GEOM,"Geometry");
	list->add_item(DISPLAY_MESH_SURFACE,"GridMesh");
	list->add_item(DISPLAY_INITIAL_DELTA,"InitialDelta");
	list->add_item(DISPLAY_INITIAL_STRESS,"InitialStress");
	list->add_item(DISPLAY_TEMPERATURE,"Temperature");
	list->add_item(DISPLAY_MODAL,"Modal");
	list->add_item(DISPLAY_FINAL_STL,"FinalSTL");
	list->add_item(DISPLAY_FINAL_DELTA,"FinalDisp");
	list->add_item(DISPLAY_FINAL_STRESS,"FinalStress");
	list->add_item(DISPLAY_STRESS_OVER_MESH,"StressOnMesh");
	list->add_item(DISPLAY_DEFLATION_GROUPS,"DeflGroups");


	list = new GLUI_Listbox(rollout,"Views:",&(liveVar.viewMode),1,pareto_setViewRoutine);
	list->add_item(VIEW_ISO,"Iso");
	list->add_item(VIEW_XYPLUS,"XY+");
	list->add_item(VIEW_XYMINUS,"XY-");
	list->add_item(VIEW_XZPLUS,"XZ+");
	list->add_item(VIEW_XZMINUS,"XZ-");
	list->add_item(VIEW_YZPLUS,"YZ+");
	list->add_item(VIEW_YZMINUS,"YZ-");
	 
	list = new GLUI_Listbox(rollout,"CuttingPlane:",&(liveVar.cuttingPlane));
	list->add_item(NO_CUTTINGPLANE,"None");
	list->add_item(XYPLUS_CUTTINGPLANE,"XY+");
	list->add_item(XYMINUS_CUTTINGPLANE,"XY-");
	list->add_item(XZPLUS_CUTTINGPLANE,"XZ+");
	list->add_item(XZMINUS_CUTTINGPLANE,"XZ-");
	list->add_item(YZPLUS_CUTTINGPLANE,"YZ+");
	list->add_item(YZMINUS_CUTTINGPLANE,"YZ-");


	spinner = new GLUI_Spinner( rollout,"CuttingValue", &(liveVar.cuttingPlaneValue));
	spinner->set_float_limits(-1,1);

	
	spinner = new GLUI_Spinner( rollout,"EigenMode", &(liveVar.displayEigenMode) );
	spinner->set_int_limits( 1, MAX_EIGEN_MODES );

	//checkbox = new GLUI_Checkbox( rollout, "Show Triangles", &(liveVar.showTriangles) );
	checkbox = new GLUI_Checkbox( rollout, "Transparent (t)", &(liveVar.geomTransparent) );
	checkbox = new GLUI_Checkbox( rollout, "WireFrame (w)", &(liveVar.displayEdges) );
	checkbox = new GLUI_Checkbox( rollout, "BoundingBox (x)", &(liveVar.boundingBox) );
	checkbox = new GLUI_Checkbox( rollout, "Axis (a)", &(liveVar.plotAxis) );
	checkbox = new GLUI_Checkbox( rollout, "Title", &(liveVar.plotTitle));
	checkbox = new GLUI_Checkbox( rollout, "Animate", &(liveVar.plotAnimation),1,setPlotRoutine);
	checkbox = new GLUI_Checkbox( rollout, "ElasticityBC (b)", &(liveVar.plotElasticityBC) );
	checkbox = new GLUI_Checkbox( rollout, "Thermal BC", &(liveVar.plotThermalBC) );
	checkbox = new GLUI_Checkbox( rollout, "Symmetry (s)", &(liveVar.plotSymmetryPlanes));
	checkbox = new GLUI_Checkbox( rollout, "NonDesignGeom ", &(liveVar.plotNonDesignGeometry));
	checkbox = new GLUI_Checkbox( rollout, "ConcaveEdges", &(liveVar.showConcaveEdges));
	//checkbox = new GLUI_Checkbox( rollout, "Contours", &(liveVar.plotContours));
	//checkbox = new GLUI_Checkbox( rollout, "Normals (n)", &(liveVar.plotNormals));

	list = new GLUI_Listbox(rollout,"Supports:",&(liveVar.plotSupportStructures));
	list->add_item(0,"None");
	list->add_item(1,"+XDir");
	list->add_item(-1,"-XDir");
	list->add_item(2,"+YDir");
	list->add_item(-2,"-YDir");
	list->add_item(3,"+ZDir");
	list->add_item(-3,"-ZDir");

	new GLUI_Button( rollout, "Reset View", RESET_DISPLAY_BUTTON,buttonCallback );
	spinner = new GLUI_Spinner( rollout,"TiffDPI", &(liveVar.tiffDPI) );
	spinner->set_int_limits( 50, 2000 );
	new GLUI_Button( rollout, "Save TIF", SAVE_TIF_BUTTON,buttonCallback );

	rollout->set_w(220);
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "FEA", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Load Set:  ",&(liveVar.currentBCSet),BCSET_SPINNER,buttonCallback);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);
	checkbox = new GLUI_Checkbox( rollout, "Remesh?", &(liveVar.forceRemesh));

	checkbox = new GLUI_Checkbox( rollout, "Preconditioner", &(liveVar.usePreconditioner) );

	list = new GLUI_Listbox(rollout,"Defl. method:",&(liveVar.workSpaceMethod));
	list->add_item(DEFLATION_NONE,"None");
	list->add_item(DEFLATION_RIGID,"Rigid");

	edittext = new GLUI_EditText(rollout,"NumGroups: ",&(liveVar.nWorkSpaceGroups));
	edittext->set_w(180);
	edittext->set_int_limits(1,10000);


	checkbox = new GLUI_Checkbox( rollout, "Node based:", &(liveVar.nodeBasedKU) );

#ifdef COMPILE_GPUCODE
	checkbox = new GLUI_Checkbox( rollout, "Use GPU", &(liveVar.useGPU) );
#endif

	list = new GLUI_Listbox(rollout,"Quality:",&(liveVar.meshQuality),1,meshQualityChange);
	list->add_item(MESH_VERY_COARSE,"Very Coarse");
	list->add_item(MESH_COARSE,"Coarse");
	list->add_item(MESH_MEDIUM,"Medium");
	list->add_item(MESH_FINE,"Fine");
	list->add_item(MESH_VERY_FINE,"Very Fine");


	edittext = new GLUI_EditText(rollout,"#Elements: ",&(liveVar.nElemsToMesh),-1,meshChange);
	edittext->set_w(200);
	edittext->set_int_limits(10,vox.getMaxVoxels());

	
	XSymCheckbox = new GLUI_Checkbox( rollout, "X Symmetry", &(liveVar.xSym),-1,meshChange );
	YSymCheckbox = new GLUI_Checkbox( rollout, "Y Symmetry", &(liveVar.ySym),-1,meshChange );
	ZSymCheckbox = new GLUI_Checkbox( rollout, "Z Symmetry", &(liveVar.zSym),-1,meshChange );


	new GLUI_Button( rollout, "Static Analysis", SOLVE_ELASTOSTATICS_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Plot CG Convergence", PLOT_CGCONVERGENCE_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Modal Analysis", SOLVEMODAL_BUTTON,buttonCallback ); 
		new GLUI_Button(rollout, "Buckling Analysis", SOLVEBUCKLING_BUTTON, buttonCallback);
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Thermal FEA", 0 );
	rollout->set_w(220);

	spinner = new GLUI_Spinner(rollout, "Load Set:  ", &(liveVar.currentBCSet), BCSET_SPINNER, buttonCallback);
	spinner->set_int_limits(0, MAX_BC_SET - 1);
	spinner->set_w(20);

	list = new GLUI_Listbox(rollout,"Quality:",&(liveVar.meshQuality),1,meshQualityChange);
	list->add_item(MESH_VERY_COARSE,"Very Coarse");
	list->add_item(MESH_COARSE,"Coarse");
	list->add_item(MESH_MEDIUM,"Medium");
	list->add_item(MESH_FINE,"Fine");
	list->add_item(MESH_VERY_FINE,"Very Fine");


	edittext = new GLUI_EditText(rollout,"#Elements: ",&(liveVar.nElemsToMesh),-1,meshChange);
	edittext->set_w(200);
	edittext->set_int_limits(10,vox.getMaxVoxels());

	new GLUI_Checkbox( rollout, "Nonlinear?", &(liveVar.useNonLinearThermal) );
	new GLUI_Button( rollout, "Static Analysis", SOLVE_THERMOSTATICS_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Transient Analysis", SOLVE_TRANSIENT_THERMAL_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "SIMP", 0);
	rollout->set_w(220);

	edittext = new GLUI_EditText(rollout, "DesVolFrac:", &liveVar.desiredVolFraction);
	edittext->set_w(200);
	edittext->set_float_limits(0.01, 0.99);

	new GLUI_Button(rollout, "SIMPOptimize", SIMP_OPTIMIZE_BUTTON, buttonCallback);
	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "TopOpt Constraints", 0 );
	rollout->set_w(220);
	list = new GLUI_Listbox(rollout,"DrawDirection:",&(liveVar.drawDirection));
	list->add_item(0,"None");
	list->add_item(1,"XDir");
	list->add_item(2,"YDir");
	list->add_item(3,"ZDir");


	list = new GLUI_Listbox(rollout,"ThroughCutDir:",&(liveVar.throughCutDirection));
	list->add_item(0,"None");
	list->add_item(1,"XDir");
	list->add_item(2,"YDir");
	list->add_item(3,"ZDir");

	list = new GLUI_Listbox(rollout,"Cyclic(Z):",&(liveVar.cyclicSymmetry));
	list->add_item(0,"None");
	//list->add_item(1,"Full");
	//list->add_item(2,"(2)180Deg");
	list->add_item(3,"(3)120Deg");
	list->add_item(4,"(4)90Deg");
	list->add_item(5,"(5)72Deg");
	list->add_item(6,"(6)60Deg");
	list->add_item(7,"(7)51Deg");
	list->add_item(8,"(8)45Deg");

	
	spinner = new GLUI_Spinner( rollout,"RelMinFeatSize: ",&(liveVar.nSmoothingIterations));
	spinner->set_int_limits(1,10);
	spinner->set_w(20);

		
	edittext = new GLUI_EditText(rollout,"DispLimit:",&liveVar.maxDisplacementAllowed);
	edittext->set_w(200);
	edittext->set_float_limits(1e-20,1e20);

	edittext = new GLUI_EditText(rollout,"StressSF:",&liveVar.stressSafetyFactor);
	edittext->set_w(200);
	edittext->set_float_limits(1,100);


	edittext = new GLUI_EditText(rollout,"SnapTol:",&liveVar.snapToGeomRelativeTol);
	edittext->set_w(200);
	edittext->set_float_limits(0,5.0);


	checkbox = new GLUI_Checkbox( rollout, "Keep Fixed Faces", &(liveVar.keepDirichlet) );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Optimize", 0 );
	rollout->set_w(220);
	checkbox = new GLUI_Checkbox( rollout, "Use All Loads", &(liveVar.useAllLoads) );

	spinner = new GLUI_Spinner( rollout,"Use Load Set: ",&(liveVar.currentBCSet),BCSET_SPINNER,ElasticityBCLabelChanged);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);

	list = new GLUI_Listbox(rollout,"Obj:",&(liveVar.topoptObjective));
	list->add_item(VOLUME,"Min Volume");
	list->add_item(COMPLIANCE,"Min Compliance");
	list->add_item(VONMISES,"Max Strength");
	list->add_item(EIGENVALUE, "Max EigenValue");

	edittext = new GLUI_EditText(rollout,"DesVolFrac:",&liveVar.desiredVolFraction);
	edittext->set_w(200);
	edittext->set_float_limits(0.01,0.99);

	edittext = new GLUI_EditText(rollout,"StepSize:",&liveVar.stepSize);
	edittext->set_w(200);
	edittext->set_float_limits(0.01,0.05);

	checkbox = new GLUI_Checkbox( rollout, "Save Intermediate?", &(liveVar.saveIntermediate) );
	new GLUI_Button( rollout, "Optimize", OPTIMIZE_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Optimization Results", 0 );
	rollout->set_w(220);

	spinner = new GLUI_Spinner( rollout,"Topology@Vol", &(liveVar.displayVolFraction),-1,(GLUI_Update_CB)CutoffChanged);
	spinner->set_float_limits(0.01,1.0);

	new GLUI_Button( rollout, "Save topology (.stl)", SAVE_STL_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Load Mesh", LOADMESH_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Save Mesh", SAVEMESH_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Save Mesh Surface", SAVE_SURFACE_MESH_BUTTON,buttonCallback );

	new GLUI_Button( rollout, "Plot Pareto Curves", PLOT_PARETO_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Plot CG Iterations", PLOT_CGITERATIONS_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Create HTML Report", CREATEREPORT_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Lattice", 0 );
	rollout->set_w(220);

	//checkbox = new GLUI_Checkbox( rollout, "Remesh?", &(liveVar.forceRemesh));

	list = new GLUI_Listbox(rollout,"LatticeType:",&(liveVar.latticeType));
	list->add_item(BOX,"BOX");
	list->add_item(BCC,"BCC*");
	list->add_item(BCCz, "BCCz*");
	list->add_item(BOX_BCC,"BOXBCC");
	list->add_item(FCC,"FCC*");
	list->add_item(FCCz,"FCCz*");
	list->add_item(FCCBCC,"FCCBCC*");
	list->add_item(OCTET,"OCTET");
	list->add_item(C11,"C11");
	
	spinner = new GLUI_Spinner( rollout,"UnitSize(mm):", &(liveVar.latticeUnitCellSizeInMM));
	spinner->set_int_limits(1,20);

	spinner = new GLUI_Spinner( rollout,"EdgeDivision:", &(liveVar.latticeResolution));
	spinner->set_int_limits(4,100);

	spinner = new GLUI_Spinner( rollout,"FillRatio:", &(liveVar.latticeFillRatio));
	spinner->set_float_limits(0.01,1);

	list = new GLUI_Listbox(rollout,"RetainSurf:",&(liveVar.latticeSurfaceRetainType));
	list->add_item(LATTICE_SURFACE_NONE,"None");
	list->add_item(LATTICE_SURFACE_CRITICAL,"Critical");
	list->add_item(LATTICE_SURFACE_ALL,"All");

	spinner = new GLUI_Spinner( rollout,"Smoothening:", &(liveVar.latticeSmoothening));
	spinner->set_int_limits(1,50);
		
	checkbox = new GLUI_Checkbox( rollout, "Save STL?", &(liveVar.latticeSaveSTL) );

	//new GLUI_Button( rollout, "Show Template", CREATE_TEMPLATE_LATTICE,buttonCallback );

	new GLUI_Button( rollout, "Create Lattice", CREATE_LATTICE_MESH_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Skeleton", 0 );
	rollout->set_w(220);

	new GLUI_Button( rollout, "Remove Nonmanifold", REMOVE_NONMANIFOLD_ELEMENTS_BUTTON,buttonCallback );

	new GLUI_Button( rollout, "Create Skeleton Graph", CREATE_SKELETON_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Fiber Analysis", 0);
	rollout->set_w(220);

	liveVar.doeFiber.fiberFillPercent = 15;
	liveVar.doeFiber.fiberLengthMicrons = 150;
	liveVar.doeFiber.fiberDiaMicrons = 30;
	liveVar.doeFiber.coneAngleDegrees = 15;

	spinner = new GLUI_Spinner(rollout, "FillPercent:", &(liveVar.doeFiber.fiberFillPercent));
	spinner->set_int_limits(0, 100);

	spinner = new GLUI_Spinner(rollout, "Length(microns):", &(liveVar.doeFiber.fiberLengthMicrons));
	spinner->set_int_limits(1, 10000);

	spinner = new GLUI_Spinner(rollout, "Dia(microns):", &(liveVar.doeFiber.fiberDiaMicrons));
	spinner->set_int_limits(1, 10000);

	spinner = new GLUI_Spinner(rollout, "ConeAngle(deg):", &(liveVar.doeFiber.coneAngleDegrees));
	spinner->set_int_limits(0, 20);

	new GLUI_Button(rollout, "Thermal Analysis", DOE_FIBER_THERMAL_ANALYSIS, buttonCallback);

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Projects", 0 );
	rollout->set_w(220);

	//glui->add_separator_to_panel(rollout);
	new GLUI_Button( rollout, "Save current project ", SAVEPROJECT_BUTTON,buttonCallback );
	//glui->add_separator_to_panel(rollout);
	new GLUI_Button( rollout, "Load .prj file ...   ", LOADPROJECT_BUTTON,buttonCallback );

	new GLUI_Button( glui, "HELP", HELP_BUTTON,(GLUI_Update_CB)buttonCallback );
	new GLUI_Button( glui, "QUIT", 0,(GLUI_Update_CB)exit );
	
	new GLUI_Button( glui, "Save TIF", SAVE_TIF_BUTTON,buttonCallback );
	/***********************************************************************************/
	glui->set_main_gfx_window( pareto_main_window );
	glui->sync_live();
	glShadeModel(GL_SMOOTH);		// Default
	glutMainLoop();
}
/************************************************************/
void GUI_ThermoElasticity()
{
	int argc = 1;
	char title[256];
	char *argv[] = {"dummy argument"};

	userPlottingFunction = &plotNothing;
	glutInit(&argc, argv);

	strcpy(title,"ThermoElasticity; Version ");

	pareto_initializeWindows(title);
	GLUI_EditText *edittext;
	GLUI_Spinner *spinner;
	GLUI_Checkbox *checkbox;
	GLUI_Rollout *rollout;
	GLUI_Listbox *list;
	GLUI_Button *btn;

	/*** Create the side subwindow ***/
	glui = GLUI_Master.create_glui_subwindow( pareto_main_window, GLUI_SUBWINDOW_RIGHT );
	glui->bkgd_color.set(200,210,225);

	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Verbose Options", 0 );
	rollout->set_w(220);
	rollout->set_font(GLUT_BITMAP_9_BY_15);

	checkbox = new GLUI_Checkbox( rollout, "STL Verbose Enable", &(liveVar.stlVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Vox Verbose Enable", &(liveVar.voxVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Elast Verbose Enable", &(liveVar.hexmeshVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "Therm Verbose Enable", &(liveVar.thermalVerbose),-1,verboseChangeCallback );
	checkbox = new GLUI_Checkbox( rollout, "TS Verbose Enable", &(liveVar.topoptVerbose),-1,verboseChangeCallback );


	/***********************************************************************************/

	rollout = new GLUI_Rollout(glui, "Geometry", 0 );
	rollout->set_w(220);
	rollout->set_font(GLUT_BITMAP_9_BY_15);
	checkbox = new GLUI_Checkbox( rollout, "Assembly", &(liveVar.assemblyMode) );
	btn = new GLUI_Button( rollout, "Read ascii/binary stl", LOADGEOM_BUTTON,buttonCallback );	
	rollout->set_w(220);
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Units & Material", 0 );
	rollout->set_w(220);

	list = new GLUI_Listbox(rollout,"Units:",&(liveVar.paretoUnits),1,changeMaterial);
	list->add_item(UNITS_METER,"m,N,s");
	list->add_item(UNITS_MM,"mm,uN,s");
	list->add_item(UNITS_INCH,"in,lb,s");
	list = new GLUI_Listbox(rollout,"Material:",&(liveVar.material),1,changeMaterial);
	list->add_item(ALLOY_STEEL,"Alloy Steel");
	list->add_item(Ti6Al4V,"Ti6Al4V");
	list->add_item(ABS_PLASTIC,"ABS Plastic");
	list->add_item(AL_1060,"Al 1060");
	list->add_item(OTHER_MATL,"Custom");

	liveVar.alpha = 6e-6;
	liveVar.K = 16;//conductivity

	edittext = new GLUI_EditText(rollout,"E: ",&(liveVar.E));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	liveVar.E = 2.1e11;
	edittext = new GLUI_EditText(rollout,"nu: ",&(liveVar.nu));
	edittext->set_w(200);
	edittext->set_float_limits(0,0.45);
	liveVar.nu = 0.28;
	edittext = new GLUI_EditText(rollout,"Y: ",&(liveVar.Y));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	edittext = new GLUI_EditText(rollout,"rho: ",&(liveVar.rho));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);
	liveVar.rho =7700.0;

	edittext = new GLUI_EditText(rollout,"K: ",&(liveVar.K));
	edittext->set_w(200);
	edittext->set_float_limits(1e-8,1e20);

	edittext = new GLUI_EditText(rollout,"alpha: ",&(liveVar.alpha));
	edittext->set_w(200);
	edittext->set_float_limits(1e-12,1e20);


	edittext = new GLUI_EditText(rollout,"Cp: ",&(liveVar.specificHeat));
	edittext->set_w(200);
	edittext->set_float_limits(1e-12,1e20);


	new GLUI_Button( rollout, "Apply", MATERIAL_BUTTON,buttonCallback );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Elasticity BC", 0 );
	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"Load Set:  ",&(liveVar.currentBCSet),BCSET_SPINNER,buttonCallback);
	spinner->set_int_limits(0,MAX_BC_SET-1);
	spinner->set_w(20);

	list = new GLUI_Listbox(rollout,"Select:",&(liveVar.faceType),-1,(GLUI_Update_CB)FaceTypeChanged );
	list->add_item(FACE_TRIANGLE,"Triangle");
	list->add_item(FACE_PLANE,"Plane");
	list->add_item(FACE_FINE_CYL,"Fine Cylin");
	list->add_item(FACE_COARSE_CYL,"Coarse Cylin");
	list->add_item(FACE_OTHER,"Other");
	list->add_item(FACE_ALL,"All");

	list = new GLUI_Listbox(rollout,"BCType:",&(liveVar.ParetoBCType),-1,(GLUI_Update_CB)ParetoBCTypeChanged );
	list->add_item(PARETO_BC_CLEAR,"None");
	list->add_item(PARETO_BC_FIXED,"Fixed");
	list->add_item(PARETO_BC_FORCE,"Force");
	list->add_item(PARETO_BC_PRESSURE,"Pressure");
	list->add_item(PARETO_BC_TORQUE,"Torque");
	list->add_item(PARETO_BC_SLIDING,"Sliding");


	XForceText = new GLUI_EditText(rollout,"XForce: ",&(liveVar.XForce));
	XForceText->set_w(200);
	XForceText->disable();
	YForceText = new GLUI_EditText(rollout,"YForce: ",&(liveVar.YForce));
	YForceText->set_w(200);
	YForceText->disable();
	ZForceText = new GLUI_EditText(rollout,"ZForce: ",&(liveVar.ZForce));
	ZForceText->set_w(200);
	ZForceText->disable();
	PressureText = new GLUI_EditText(rollout,"Pressure: ",&(liveVar.Pressure));
	PressureText->set_w(200);
	PressureText->disable();
	TorqueText = new GLUI_EditText(rollout,"Torque: ",&(liveVar.Torque));
	TorqueText->set_w(200);
	TorqueText->disable();

	new GLUI_Button( rollout, "ApplyBC", APPLY_NEWELASTICITY_BC_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Thermal BC", 0 );
	rollout->set_w(220);

	list = new GLUI_Listbox(rollout,"Select:",&(liveVar.faceType),-1,(GLUI_Update_CB)FaceTypeChanged );
	list->add_item(FACE_TRIANGLE,"Triangle");
	list->add_item(FACE_PLANE,"Plane");
	list->add_item(FACE_FINE_CYL,"Fine Cylin");
	list->add_item(FACE_COARSE_CYL,"Coarse Cylin");
	list->add_item(FACE_OTHER,"Other");
	list->add_item(FACE_ALL,"All");


	list = new GLUI_Listbox(rollout,"Type:",&(liveVar.ThermalBCType),-1,(GLUI_Update_CB)ThermalBCTypeChanged );
	list->add_item(PARETO_BC_TEMP,"Temperature");
	list->add_item(PARETO_BC_HEAT,"HeatFlux");
	list->add_item(PARETO_BC_CONVECTION,"Convection");
	list->add_item(PARETO_BC_RADIATION,"Radiation");

	liveVar.ThermalBCValue = 23.0;
	ThermalBCValueText = new GLUI_EditText(rollout,"K: ",&(liveVar.ThermalBCValue));
	ThermalBCValueText->set_w(200);
	
	liveVar.ThermalBCReference =23.0;
	ThermalBCRefText = new GLUI_EditText(rollout,"TRef: ",&(liveVar.ThermalBCReference));
	ThermalBCRefText->set_w(200);
	ThermalBCRefText->disable();

	new GLUI_Button( rollout, "ApplyThermalBC", APPLY_THERMAL_BC_BUTTON,buttonCallback );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Display Options", 0 );
	
	list = new GLUI_Listbox(rollout,"Display:",&(liveVar.displayMode),1,setPlotRoutine);
	list->add_item(DISPLAY_GEOM,"Geometry");
	list->add_item(DISPLAY_MESH_SURFACE,"GridMesh");
	list->add_item(DISPLAY_INITIAL_DELTA,"InitialDeform");
	list->add_item(DISPLAY_INITIAL_STRESS,"InitialStress");
	list->add_item(DISPLAY_TEMPERATURE,"Temperature");

	rollout->set_w(220);
	spinner = new GLUI_Spinner( rollout,"EigenMode", &(liveVar.displayEigenMode) );
	spinner->set_int_limits( 1, MAX_EIGEN_MODES );


	checkbox = new GLUI_Checkbox( rollout, "Transparent", &(liveVar.geomTransparent) );
	checkbox = new GLUI_Checkbox( rollout, "TriangleEdges", &(liveVar.displayEdges) );
	checkbox = new GLUI_Checkbox( rollout, "BoundingBox", &(liveVar.boundingBox) );
	checkbox = new GLUI_Checkbox( rollout, "Axis", &(liveVar.plotAxis) );
	checkbox = new GLUI_Checkbox( rollout, "Title", &(liveVar.plotTitle));
	checkbox = new GLUI_Checkbox( rollout, "Animate", &(liveVar.plotAnimation));
	checkbox = new GLUI_Checkbox( rollout, "Elasticity BC", &(liveVar.plotElasticityBC) );
	checkbox = new GLUI_Checkbox( rollout, "Thermal BC", &(liveVar.plotThermalBC) );
	checkbox = new GLUI_Checkbox( rollout, "Symmetry", &(liveVar.plotSymmetryPlanes));

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Static FEA", 0 );
	rollout->set_w(220);
	
	checkbox = new GLUI_Checkbox( rollout, "Node-Based", &(liveVar.nodeBasedKU),-1,meshChange );

	list = new GLUI_Listbox(rollout,"Defl. method:",&(liveVar.workSpaceMethod),-1,meshChange);
	list->add_item(DEFLATION_NONE,"None");
	list->add_item(DEFLATION_RIGID,"Rigid");

	edittext = new GLUI_EditText(rollout,"NumGroups: ",&(liveVar.nWorkSpaceGroups));
	edittext->set_w(180);
	edittext->set_int_limits(1,5000);

	edittext = new GLUI_EditText(rollout,"NumElements: ",&(liveVar.nElemsToMesh),-1,meshChange);
	edittext->set_w(200);
	edittext->set_int_limits(10,vox.getMaxVoxels());

	XSymCheckbox = new GLUI_Checkbox( rollout, "X Symmetry", &(liveVar.xSym),-1,meshChange );
	YSymCheckbox = new GLUI_Checkbox( rollout, "Y Symmetry", &(liveVar.ySym),-1,meshChange );
	ZSymCheckbox = new GLUI_Checkbox( rollout, "Z Symmetry", &(liveVar.zSym),-1,meshChange );


	new GLUI_Button( rollout, "Static Analysis", SOLVE_ELASTOSTATICS_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Plot CG Convergence", PLOT_CGCONVERGENCE_BUTTON,buttonCallback );
	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Thermal", 0 );
	rollout->set_w(220);

	new GLUI_Checkbox( rollout, "Nonlinear?", &(liveVar.useNonLinearThermal) );


	new GLUI_Button( rollout, "Thermal Analysis", SOLVE_THERMOSTATICS_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Thermo Elastic", SOLVE_THERMOELASTIC_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Plot Sensor Data", PLOT_SENSORDATA_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Sensors", 0 );
	rollout->set_w(220);

	spinner = new GLUI_Spinner( rollout,"Sensor Number: ", &(liveVar.sensorNumber) );
	spinner->set_int_limits( 0, MAX_PARETOSENSORS-1 );

	new GLUI_Button( rollout, "Plot Sensor Data", PLOT_SENSORDATA_BUTTON,buttonCallback );

	/***********************************************************************************/
	rollout = new GLUI_Rollout(glui, "Projects", 0 );
	rollout->set_w(220);

	new GLUI_Button( rollout, "Save current project ", SAVEPROJECT_BUTTON,buttonCallback );
	new GLUI_Button( rollout, "Load .prj file ...   ", LOADPROJECT_BUTTON,buttonCallback );

	new GLUI_Button( glui, "HELP", HELP_BUTTON,(GLUI_Update_CB)buttonCallback );
	new GLUI_Button( glui, "QUIT", 0,(GLUI_Update_CB)exit );

	/***********************************************************************************/
	glui->set_main_gfx_window( pareto_main_window );
	glui->sync_live();
	glShadeModel(GL_SMOOTH);		// Default
	glutMainLoop();
}
/************************************************************/
void createHTMLReport()
{
	char fileName[512],str[512];
	//define units
	char *lengthU[]={"m", "in","mm"};
	char *timeU[]={"s","s","s"};
	char *forceU[]={"N","lbf","&mu;N"};
	char *volumeU[]={"m<sup>3</sup>","in<sup>3</sup>","mm<sup>3</sup>"};
	char *densityU[]={"kg / m<sup>3</sup>","lbm / in<sup>3</sup>","g / mm<sup>3</sup>"};
	char *areaU[]={"m<sup>2</sup>","in<sup>2</sup>","mm<sup>2</sup>"};
	char *pressureU[]={"Pa","psi","Pa"};
	char dimless[] = {"-"};
	int unitSet = liveVar.paretoUnits;
	double xyzMin[3], xyzMax[3];
	vox.getBoundingBox(xyzMin, xyzMax);
	double vSize[3],STLVolume,STLArea;
	int gridSize[3];
	STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
	vox.getVoxelSize(vSize);
	vox.getNumGridElems(gridSize);

	
	//create report folders
	strcpy(liveVar.reportFolder,"..\\ReportsFolder\\");
#ifdef _WIN32
	CreateDirectoryA(liveVar.reportFolder, NULL);// if does not exists
	strcat(liveVar.reportFolder,liveVar.modelName);
	CreateDirectoryA(liveVar.reportFolder, NULL);
#endif

	strcpy(fileName,liveVar.reportFolder);
	strcat(fileName,"\\");
	strcat(fileName,liveVar.modelName);
	strcat(fileName,".html");

	printf("---------------------------------------------\n");
	printf("Creating report ... \n");
	FILE *fptr = fopen(fileName,"w");

	if (fptr == NULL){
		printf("Creating report file %s unsuccesful\n",fileName);
		printf("---------------------------------------------\n");
		return;
	}

	fprintf(fptr,"<!DOCTYPE html>\n");
	fprintf(fptr,"<html>\n");
	fprintf(fptr,"<head>\n");
	fprintf(fptr,"<meta charset=\"UTF-8\" />");
	fprintf(fptr,"<title>%s Report</title>\n", liveVar.modelName);
	//fprintf(fptr,"<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" />\n");
	fprintf(fptr,"</head>\n");
	fprintf(fptr,"<body>\n");
	fprintf(fptr,"<h1>1. Geometry and Boundary Conditions</h1>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"Geometry.tiff\" ALT=\"Geometry\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 1: Initial Geometry</strong></p>\n");

	// Geometry information
	fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
	fprintf(fptr,"<tr>\n");
	fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\"> Quantity </div></th>\n");
	fprintf(fptr,"<th width=\"300\" scope=\"col\"><div align=\"center\">Value</div></th>\n");
	fprintf(fptr,"<th width=\"50\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Volume</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",STLVolume);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",volumeU[unitSet]);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Surface Area</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",STLArea);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",areaU[unitSet]);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">#Triangles</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%d</div></td>\n",vox.getNumTriPoints());
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">#Points</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%d</div></td>\n",vox.getNumTriangles());
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Bounding box</div></td>\n");
	//bounding box
	fprintf(fptr,"<td><div align=\"center\">Min: (%f,%f,%f)</div>",xyzMin[0],xyzMin[1],xyzMin[2]);
	fprintf(fptr,"<div align=\"center\">Max: (%f,%f,%f)</div>",xyzMax[0],xyzMax[1],xyzMax[2]);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthU[unitSet]);
	fprintf(fptr,"</tr>\n</table>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Table 1: Geometric information</strong></p>\n");
	//material properties
	fprintf(fptr,"<h1>2. Material</h1>\n");
	fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
	fprintf(fptr,"<tr>\n");
	fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Material Property</div></th>\n");
	fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Value</div></th>\n");
	fprintf(fptr,"<th width=\"150\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">E</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.E);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",pressureU[unitSet]);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">&nu;</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.nu);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">&rho;</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.rho);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",densityU[unitSet]);
	fprintf(fptr,"</tr>\n</table>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Table 2: Material Properties</strong></p>\n");


	//FEA Info
	fprintf(fptr,"<h1>3. Static FEA </h1>\n");

	// FEA results
	fprintf(fptr,"<h2>3.1 Stress</h2>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"InitialStress.tiff\" ALT=\"Stress\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 3: Stress Analysis</strong></p>\n");

	fprintf(fptr,"<h2>3.2 Displacement</h2>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"InitialDelta.tiff\" ALT=\"Displacement\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 4: Displacement Analysis</strong></p>\n");

	// FEA TABLE
	fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
	fprintf(fptr,"<tr>\n");
	fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Parameter</div></th>\n");
	fprintf(fptr,"<th width=\"250\" scope=\"col\"><div align=\"center\">Values</div></th>\n");
	fprintf(fptr,"<th width=\"100\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Voxel Size</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">(%f, %f, %f)</div>",vSize[0],vSize[1],vSize[2]);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthU[unitSet]);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Background Grid</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">(%d, %d, %d)</div>",gridSize[0],gridSize[1],gridSize[2]);
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Max displacement</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",topopt.getDeformationMax());
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthU[unitSet]);
	fprintf(fptr,"</tr>\n<tr>\n");
	fprintf(fptr,"<td><div align=\"center\">Max Von Mises Stress</div></td>\n");
	fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",topopt.getMaxVonMisesStress());
	fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",pressureU[unitSet]);
	fprintf(fptr,"</tr>\n</table>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Table 3: FEA Summary </strong></p>\n");


	//Optimization
	fprintf(fptr,"<h1>5. Optimization </h1>\n");

	fprintf(fptr,"<h2>5.1 Isosurface</h2>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"FinalIso.bmp\" ALT=\"Mesh\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 5: Isosurface</strong></p>\n");

	fprintf(fptr,"<h2>5.2 Final Stress</h2>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"FinalStress.bmp\" ALT=\"Stress\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 6: Final Stress </strong></p>\n");

	fprintf(fptr,"<h2>5.3 Final Displacement</h2>\n");
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"FinalDelta.bmp\" ALT=\"Displacement\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 7: Final Displacement </strong></p>\n");


	fprintf(fptr,"<h2>5.4 Pareto Curves</h2>\n");

	strcpy(str,liveVar.reportFolder);
	strcat(str,"\\ParetoCurves.tiff");

#ifdef _WIN32
	Sleep(200);
#endif
	//SaveAsTIFF(str);
	fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"ParetoCurves.tiff\" ALT=\"Displacement\" width=\"400\" height=\"400\"></div>\n");
	fprintf(fptr,"<p align=\"center\"><strong>Figure 5: Pareto Curves</strong></p>\n");

	// The END
	fprintf(fptr,"</body>\n");
	fprintf(fptr,"</html>\n");
	fclose(fptr);
	printf("Done creating report ... \n");
	printf("---------------------------------------------\n");
}
/************************************************************/
void callBackUserFunction( int control )
{
	double volFractions[MAX_ISO_SAVES],relCompliance[MAX_ISO_SAVES],relmaxDisp[MAX_ISO_SAVES],relmaxStress[MAX_ISO_SAVES],timeTaken[MAX_ISO_SAVES],CGIterations[500],CGNorm[20000],materialProperties[7];
	char BMPFileName[256],TIFFileName[256],objFileName[256];
	bool assembly;
	double offset[] = {0,0,0};
	int materialId,tempInt;
	vector<double> xData;
	vector<vector<double>> yData;
	vector<char *> xLabels;
	vector<char *> yLegend;
	
	Timer timer;

	switch(control) {
case LOADGEOM_BUTTON:
	assembly = false;
	if (liveVar.assemblyMode == 1) assembly = true;
	if (!assembly) liveVar.nSTLFilesLoaded = 0;// start from scratch
	if (GetFileNameToOpen("STL Files (*.stl,*.off)\0*.stl\0All Files (*.*)\0*.*\0",".stl",liveVar.geomFileName[liveVar.nSTLFilesLoaded]))
		loadGeom(assembly);	

	/*if (vox.getNumberOfConcaveEdges() > 0) {
		MessageBoxA(NULL,"Warning!!\nPart has concave edges that may lead to stress concentration!\nUse Display Options->Concave Edges to highlight these edges","Pareto",MB_OK | MB_APPLMODAL );
	}*/
	glui->sync_live();
	break;
case RELOADGEOM_BUTTON:
	assembly = false;
	liveVar.nSTLFilesLoaded = 0;
	if (GetFileNameToOpen("STL Files (*.stl)\0*.stl\0All Files (*.*)\0*.*\0",".stl",liveVar.geomFileName[liveVar.nSTLFilesLoaded]))
		updateGeom(assembly);	
	break;
case SAVEGEOM_AS_OBJ_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		printf("---------------------------------------------\n");
		printf("Initial geometry undefined \n");
		printf("---------------------------------------------\n");
		break;
	}
	strcpy(objFileName,liveVar.modelName);
	strcat(objFileName,".obj");
	printf("Saving OBJ file name %s \n",objFileName);
	vox.saveAsObj(objFileName);
	break;
case RESET_DISPLAY_BUTTON:
	resetDisplay(0);
	break;
case MATERIAL_BUTTON:
	materialProperties[0] = liveVar.E;
	materialProperties[1] = liveVar.nu;
	materialProperties[2] = liveVar.Y;
	materialProperties[3] = liveVar.rho;
	materialProperties[4] = liveVar.K;
	materialProperties[5] = liveVar.alpha;
	materialProperties[6] = liveVar.specificHeat;

	for (int i = 0; i < 7; i++){
		if (materialProperties[i] < 0){
			alert("Material properties must all be positive\n");
			return;
		}
	}
	if (vox.getNumParts() == 1){// only one part exists
		liveVar.selectedPart = 0;
		topopt.setIsotropicMaterialPropertiesOfPart(liveVar.selectedPart,materialProperties,materialNamesStr[liveVar.material]);
		if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(liveVar.selectedPart);
		else topopt.setPartToBeOptimized(liveVar.selectedPart);
		printf("Material applied to part %d \n",liveVar.selectedPart);
	}
	else if (liveVar.selectedPart != -1) {// one of many parts selected
		topopt.setIsotropicMaterialPropertiesOfPart(liveVar.selectedPart,materialProperties,materialNamesStr[liveVar.material]);
		if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(liveVar.selectedPart);
		else topopt.setPartToBeOptimized(liveVar.selectedPart);
		printf("Material applied to part %d \n",liveVar.selectedPart);
	}
	else if (liveVar.selectedPart == -1){// no part selected among many parts
		const int result = MessageBoxA(NULL, "Apply to all parts?", "Pareto",  MB_YESNO);
		switch (result){
			case IDYES:
				for (int part = 0; part < vox.getNumParts(); part++) {
					topopt.setIsotropicMaterialPropertiesOfPart(part,materialProperties,materialNamesStr[liveVar.material]);
					if (liveVar.nonDesignCheck) topopt.setPartToNonDesign(part);
					else topopt.setPartToBeOptimized(part);
				}
				printf("Material applied to all parts\n");
			break;
			case IDNO:
				printf("First select part before applying material \n");
			break;
		}
	}
	printf("---------------------------------------------\n");
	vox.resetHighlight();
	bool materialSelectionDone;;
	materialSelectionDone = true;
	for (int part = 0; part < vox.getNumParts(); part++) {
		if (topopt.getMaterialIDofPart(part) < 0) 
			materialSelectionDone = false;
	}
	if (materialSelectionDone)
		strcpy(liveVar.userMessage,"Ready to apply boundary conditions ...");

	glui->sync_live();

	break;
case BCSET_SPINNER:
	changeTopOptBC();
	break;
case APPLY_NEWELASTICITY_BC_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		printf("---------------------------------------------\n");
		printf("Initial geometry undefined \n");
		printf("---------------------------------------------\n");
		break;
	}
	applyElasticityBC();
	printf("---------------------------------------------\n");
	break;
case APPLY_ACCELERATION_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		printf("---------------------------------------------\n");
		printf("Initial geometry undefined \n");
		printf("---------------------------------------------\n");
		break;
	}
	
	ElasticityAcceleration[liveVar.currentBCSet][0] = liveVar.XAcceleration;
	ElasticityAcceleration[liveVar.currentBCSet][1] = liveVar.YAcceleration;
	ElasticityAcceleration[liveVar.currentBCSet][2] = liveVar.ZAcceleration;
	ElasticityAcceleration[liveVar.currentBCSet][3] = liveVar.radiansPerSecZAxis;
	printf("Acceleration applied \n");
	printf("---------------------------------------------\n");
	liveVar.displayMode = DISPLAY_GEOM;
	setPlotRoutine(1);
	break;
case APPLY_THERMAL_BC_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		printf("---------------------------------------------\n");
		printf("Initial geometry undefined \n");
		printf("---------------------------------------------\n");
		break;
	}
	if (liveVar.displayMode != DISPLAY_GEOM)
		printf("You must be in the geometry mode to apply boundary conditions\n");
	else
		applyThermalBC();
	printf("---------------------------------------------\n");
	break;
case SAVEMESH_BUTTON:
	topopt.dumpVoxelFileBinary(liveVar.voxFileName);
	topopt.readVoxelFileBinary(liveVar.voxFileName);//removes  void elements
	topopt.dumpVoxelFileBinary(liveVar.voxFileName);
	topopt.dumpVoxelFile("temp.voxAscii");
	break;
case LOADMESH_BUTTON:
	if (GetFileNameToSave("Project Files (*.vox)\0*.vox\0All Files (*.*)\0*.*\0",".vox",liveVar.voxFileName))
		topopt.readVoxelFileBinary(liveVar.voxFileName);

	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	guiState = HEX_SUCCESFUL;
	setPlotRoutine(1);
	break;
case SAVEPROJECT_BUTTON:
	if (GetFileNameToSave("Project Files (*.prj)\0*.prj\0All Files (*.*)\0*.*\0",".prj",liveVar.prjFileName))
		savePRJ();
	break;
case LOADPROJECT_BUTTON:
	if (GetFileNameToOpen("Project Files (*.prj)\0*.prj\0All Files (*.*)\0*.*\0",".prj",liveVar.prjFileName))
		loadPRJ();
	if (liveVar.usingGUI) glui->sync_live();
	break;
case CREATEMESH_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		printf("Initial geometry undefined \n");
		printf("---------------------------------------------\n");
		break;
	}
	createTopOptMesh();
	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	guiState = HEX_SUCCESFUL;
	setPlotRoutine(1);
	break;
case CONFORMMESH_BUTTON:
	deformMeshToBoundary();
	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	setPlotRoutine(1);
	printf("---------------------------------------------\n");
	break;
case SOLVE_ELASTOSTATICS_BUTTON:
	if (guiState < BC_SUCCESFUL){
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if (!checkForFiniteForce()){
		printf("No force has been applied on the geometry\n");
		printf("---------------------------------------------\n");
	}
	changeTopOptBC();
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();
	
	//solveStatic();
	//printf("Starting thread \n");
	_beginthread(solveStaticFEAThread,0,NULL);
	if (topopt.isDesignPastYield(&materialId))
		alert("Warning: Material is past yield\n");
	break;
case SOLVE_THERMOSTATICS_BUTTON:
	if (guiState < BC_SUCCESFUL){
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();
	
	if (guiState >= HEX_SUCCESFUL) 
		_beginthread(solveThermoStaticFEAThread,0,NULL);
	//solveThermoStatic();
	break;
case SOLVE_TRANSIENT_THERMAL_BUTTON:
	if (guiState < BC_SUCCESFUL){
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();
	
	if (guiState >= HEX_SUCCESFUL) 
		_beginthread(solveThermoTransientFEAThread,0,NULL);
	//solveThermoTransient();
	break;
case SOLVE_THERMOELASTIC_BUTTON:
	if (guiState < THERMALFEA_SUCCESFUL){
		printf("Please solve thermal problem first \n");
		printf("---------------------------------------------\n");
		break;
	}
	solveThermoElastic();
	break;
case SOLVE_TOOLPATH_BUTTON:
	if (guiState < THERMALFEA_SUCCESFUL){
		printf("Please solve thermal problem first \n");
		printf("---------------------------------------------\n");
		break;
	}
	_beginthread(solveThermoToolPathFEAThread,0,NULL);
	//solveToolPathThermoStatic();
	break;
case PLOT_SENSORDATA_BUTTON:
	topopt.plotParetoSensorData(liveVar.sensorNumber);
	break;
case SOLVEMODAL_BUTTON:
	if (guiState < BC_SUCCESFUL){
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();

	solveModal();
	break;
case SOLVEBUCKLING_BUTTON:
	createTopOptMesh();
	solveThermoElastic();
	solveBuckling();
	break;
case RECOMPUTEBC_BUTTON:
	if (guiState < STATICFEA_SUCCESFUL) {
		printf("First solve FEA \n");
		break;
	}
	recomputeBoundaryConditions();
	break;
case DELETECONGRUENCE_BUTTON:
	topopt.createElementMatricesTemplates();
	topopt.disableNodalSpMV();
	timer.start();
	topopt.initializeSolution();
	topopt.performSingleKUonCPU();
	printf("Time taken = %.3f (millsec) \n",timer.getElapsedTimeInMilliSec());
	topopt.discardTemplates();
	topopt.createElementMatricesTemplates();
	topopt.disableNodalSpMV();
	topopt.initializeSolution();
	topopt.performSingleKUonCPU();
	printf("Time taken = %.3f (millsec) \n",timer.getElapsedTimeInMilliSec());
	printf("---------------------------------------------\n");
	break;
case SIMP_OPTIMIZE_BUTTON:
	if (guiState < BC_SUCCESFUL) {
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if (!checkForFiniteForce()) {
		printf("No force has been applied on the geometry\n");
		printf("---------------------------------------------\n");
	}
	changeTopOptBC();
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();

	timer.start();
	solveStatic();// don't use the thread version here since we want the process not to move forward
	liveVar.timeForOneFEA = timer.getElapsedTimeInSec();

	if (guiState < STATICFEA_SUCCESFUL) {
		MessageBoxA(NULL, "Could not solve FEA", "Pareto", MB_OK | MB_APPLMODAL);
		return;
	}
	SIMPOptimize();
	break;
case OPTIMIZE_BUTTON:
	if (guiState < BC_SUCCESFUL){
		if (guiState < GEOM_SUCCESFUL) {
			printf("Initial geometry undefined \n");
		}
		else {
			printf("BC is undefined; please see specify BC \n");
		}
		printf("---------------------------------------------\n");
		break;
	}
	if (!checkForFiniteForce()){
		printf("No force has been applied on the geometry\n");
		printf("---------------------------------------------\n");
	}
	changeTopOptBC();
	if ((liveVar.forceRemesh) || (guiState < HEX_SUCCESFUL))
		createTopOptMesh();
	
	timer.start();
	solveStatic();// don't use the thread version here since we want the process not to move forward
	liveVar.timeForOneFEA = timer.getElapsedTimeInSec();

	if (guiState < STATICFEA_SUCCESFUL ) {
		MessageBoxA(NULL,"Could not solve FEA","Pareto",MB_OK | MB_APPLMODAL );
		return;
	}
	liveVar.displayMode = DISPLAY_FINAL_STL;
	glui->sync_live();
	setPlotRoutine(1);

	topopt.resetLinearSolvesCounter();
	if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads == 1)){
		//multiLoadOptimize();
		_beginthread(multiLoadOptimizeThread,0,NULL);
	}
	else {
		changeTopOptBC();
		//singleLoadOptimize();
		_beginthread(singleLoadOptimizeThread,0,NULL);
	}	
	//liveVar.forceRemesh = 0;
	break;
case DOE_FIBER_THERMAL_ANALYSIS:
	double bBox[3][2],bBoxVolume,meshVolume,alpha, Lx, Ly, Lz,center[3],deltaX,deltaY,deltaZ,fiberRadius,fiberLength, totalFiberVolume;
	double startPt[3], endPt[3],axisDir[3],meshSize,singleFiberVolume;
	int fillPercent,Nx,Ny,Nz,*elemsIn, nElemsIn, nFibers;


	fiberRadius = liveVar.doeFiber.fiberDiaMicrons / (2 * 1e6);//convert to meters
	fiberLength = liveVar.doeFiber.fiberLengthMicrons / (1e6);//convert to meters

	singleFiberVolume = 3.14*fiberRadius*fiberRadius*fiberLength;

	cout << "fiberRadius: " << fiberRadius << endl;
	cout << "fiberLength: " << fiberLength << endl;
	meshSize = topopt.getMeshElementSize();
	cout << "meshSize: " << meshSize << endl;


	if (meshSize > fiberRadius / 3)
		cout << "Mesh is too coarse for fiber radius" << endl;

	// if volume of Bounding box is not same same as part, quit (only valid for rectangular solids)
	topopt.getBoundingBox(bBox);

	Lx = (bBox[0][1] - bBox[0][0]);
	Ly = (bBox[1][1] - bBox[1][0]);
	Lz = (bBox[2][1] - bBox[2][0]);
	bBoxVolume = Lx* Ly * Lz;
	meshVolume = topopt.getMeshVolume();

	fillPercent = liveVar.doeFiber.fiberFillPercent;

	totalFiberVolume = 0.01*fillPercent*meshVolume;

	nFibers = ceil(totalFiberVolume/ singleFiberVolume);

	cout << "nFibers: " << nFibers << endl;
	//Find N equally distributed center points in BBox
	// Define N cylinders of desired length and radius with these as centers
	// Set voxels within these cylinders to different material
	// Carry out thermal analysis

	Nx = ceil(Lx / fiberLength);

	alpha = pow(nFibers/Nx / (Ly * Lz ), 0.5);

	Ny = ceil(alpha*Ly);
	Nz = ceil(alpha*Lz);

	deltaX = Lx / Nx;
	deltaY = Ly / Ny;
	deltaZ = Lz / Nz;
	cout << Nx << " " << Ny << " " << Nz << endl;
	cout << deltaX << " " << deltaY << " " << deltaZ << endl;
	
	axisDir[0] = 1;// assumed
	axisDir[1] = 0;
	axisDir[2] = 0;

	

	//reset all to matrix
	for (int elem = 0; elem < topopt.getNumElems(); elem++) {
		topopt.setTemplateIdOfElem(0, elem);// for solving
		topopt.setMaterialIdOfElem(0, elem);// for plotting
		topopt.setPartIdOfElem(0, elem);// for plotting
	}

	int nFibersActual;
	
	nFibersActual = 0;
	elemsIn = (int *)malloc(topopt.getNumElems() * sizeof(int));
	
	for (int iy = 0; iy < Ny; iy++) {
		
		center[1] = bBox[1][0] + (iy + 0.5)*deltaY;
		cout << iy << "/" << Ny << endl;
		for (int iz = 0; iz < Nz; iz++) {
			center[2] = bBox[2][0] + (iz + 0.5)*deltaZ;
			for (int ix = 0; ix < Nx; ix++) {
				center[0] = bBox[0][0] + (ix + 0.5)*deltaX;
				axisDir[1] = 0.6*(0.5 - 1.0*rand() / RAND_MAX);
				axisDir[2] = 0.6*(0.5 - 1.0*rand() / RAND_MAX);
				axisDir[0] = sqrt(1 - axisDir[1] * axisDir[1] - axisDir[2] * axisDir[2]);
				for (int kk = 0; kk < 3; kk++) {
					startPt[kk] = center[kk] - 1.1*axisDir[kk] * deltaX / 2;
					endPt[kk] = center[kk] + 1.1*axisDir[kk] * deltaX / 2;
				}
				//PRINT3D(center);
				nElemsIn = topopt.getElemsInsideCylinder(startPt, endPt, fiberRadius, topopt.getNumElems(), elemsIn);
				if (nElemsIn > 0) nFibersActual++;
				for (int index = 0; index < nElemsIn; index++) {
					topopt.setTemplateIdOfElem(1, elemsIn[index]);// for solving
					topopt.setMaterialIdOfElem(1, elemsIn[index]);// for plotting
					topopt.setPartIdOfElem(1, elemsIn[index]);// for plotting
				}
			}
		}
	}
	free(elemsIn);
	
	cout << "nFibersActual " << nFibersActual << endl;
	topopt.setThermoElasticReferenceTemperature(23);//
	topopt.solveThermalProblem();
	printf("Pushing thermal solution to tessellation ... \n");
	topopt.pushThermalSolutionToTessellation();
	printf("---------------------------------------------\n");

	setPlotRoutine(1);
	break;
case EXTRACT_CONTOURS_BUTTON:
	if (guiState < TOPOPT_SUCCESFUL){
		printf("Please optimize topology first \n");
		break;
	}
	/*
	vox.getVoxelSize(dSize);
	topopt.generateContourOfMeshComponents(liveVar.smootheningMethod,liveVar.contourFeatureTolerance, 0.1*liveVar.contourSimplificationTolerance*dSize[0],liveVar.contourSmoothening,liveVar.contourOffset);
	liveVar.displayMode = DISPLAY_CONTOUR;
	liveVar.geomTransparent = 0;
	liveVar.plotContours = 1;
	liveVar.plotElasticityBC = 0;
	setPlotRoutine(1);
	strcpy(tempstr,liveVar.modelName);
	strcat(tempstr,"_contourData.txt");
	topopt.dumpContourData(tempstr);
	*/
	printf("---------------------------------------------\n");
	break;
case SAVE_BMP_BUTTON:
	strcpy(BMPFileName,"test.bmp");
	if (GetFileNameToSave("BMP Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0",".bmp",BMPFileName)){
		setMainWindow();
		SaveAsBMP(BMPFileName);
	}
	break;
case SAVE_TIF_BUTTON:
	strcpy(TIFFileName,"test.tif");
	if (GetFileNameToSave("Tiff Files (*.tif)\0*.tif\0All Files (*.*)\0*.*\0",".tif",TIFFileName)){
		setMainWindow();
#if COMPILE_MODE != BATCH_MODE
		SaveAsTIFF(TIFFileName);
#endif
	}
	break;
case CREATEREPORT_BUTTON:
	if (guiState < TOPOPT_SUCCESFUL){
		MessageBoxA(NULL," First run top opt \n","Pareto",MB_OK);
		break;
	}
	createHTMLReport();
	break;
case SAVE_STL_BUTTON:
	printf("---------------------------------------------\n");
	if (guiState < TOPOPT_SUCCESFUL){
		printf(" First run top opt \n");
		printf("---------------------------------------------\n");
		break;
	}
	char resultFile[256],temp[256];
	double volfrac,actualFrac,time,isoCutoff;
	int isoCounter;
	volfrac = liveVar.displayVolFraction;
	isoCounter = topopt.getClosestCounterAtVolFraction(volfrac,&actualFrac,&isoCutoff,&time);
	volfrac = MAX(volfrac,topopt.getCurrentVolumeFraction());

	strcpy(resultFile,liveVar.modelName);
	sprintf(temp,"_%.2f_.stl",volfrac);
	strcat(resultFile,temp);
	if (!GetFileNameToSave("STL Files (*.stl)\0*.stl\0All Files (*.*)\0*.*\0",".stl",resultFile))
		break;
	//strcpy(stlModelName,liveVar.stlFileName);
	printf("Saving STL to %s \n",resultFile);
	topopt.dumpIsoBinary(isoCounter,resultFile);
	
	printf("---------------------------------------------\n");
	break;
case SAVE_SURFACE_MESH_BUTTON:

	printf("---------------------------------------------\n");
	if (guiState < TOPOPT_SUCCESFUL){
		printf(" First run top opt \n");
		printf("---------------------------------------------\n");
		break;
	}
	volfrac = liveVar.displayVolFraction;
	isoCounter = topopt.getClosestCounterAtVolFraction(volfrac,&actualFrac,&isoCutoff,&time);
	volfrac = MAX(volfrac,topopt.getCurrentVolumeFraction());

	strcpy(resultFile,liveVar.modelName);
	sprintf(temp,"_mesh_%.2f.stl",volfrac);
	strcat(resultFile,temp);
	printf("Saving mesh surface to %s \n",resultFile);

	topopt.createAndDumpIsoSurfaceFromMesh(resultFile,isoCounter);
	break;
case SAVE_MESH_BUTTON:
	printf("---------------------------------------------\n");
	if (guiState < STATICFEA_SUCCESFUL){
		printf(" First run FEA \n");
		printf("---------------------------------------------\n");
		break;
	}
	strcpy(resultFile,liveVar.modelName);
	sprintf(temp,".vox");
	strcat(resultFile,temp);
	printf("Saving voxel mesh to %s \n",resultFile);
	topopt.dumpVoxelFileBinary(resultFile);
	topopt.readVoxelFileBinary(resultFile);
	topopt.refineAndDumpVoxelFileBinary(resultFile);
	guiState = HEX_SUCCESFUL;
	liveVar.displayMode = DISPLAY_MESH_SURFACE;
	setPlotRoutine(0);
	break;
case CREATE_TEMPLATE_LATTICE:
	double boundingBox[3][2];
	liveVar.nSTLFilesLoaded = 0;
	strcpy(liveVar.geomFileName[0],"latticeUnitTemp.stl");
	boundingBox[0][0] = boundingBox[1][0] = boundingBox[2][0] = 0;
	boundingBox[0][1] = boundingBox[1][1] = boundingBox[2][1] = 0.005;
	vox.createBoundingBoxSTLFile(boundingBox,liveVar.geomFileName[0]);
	int savedType,saveSmoothening,saveSTL,saveResolution;
	float savedFillRatio;
	savedType= liveVar.latticeType;// these will be reset when the geometry is loaded below, so save user preference
	savedFillRatio = liveVar.latticeFillRatio;
	saveSmoothening = liveVar.latticeSmoothening;
	saveSTL = liveVar.latticeSaveSTL;
	saveResolution = liveVar.latticeResolution;
	loadGeom(false);
	liveVar.latticeType = savedType;
	liveVar.latticeFillRatio = savedFillRatio;
	liveVar.latticeResolution = saveResolution;
	liveVar.latticeSmoothening = saveSmoothening;
	liveVar.latticeSaveSTL = saveSTL;
	liveVar.latticeResolution = saveResolution;
	createLatticeStructureMesh();
	liveVar.displayMode = DISPLAY_FINAL_STL;
	guiState = TOPOPT_SUCCESFUL;
	setPlotRoutine(1);
	printf("---------------------------------------------\n");
	break;
case CREATE_LATTICE_MESH_BUTTON:
	if (guiState < GEOM_SUCCESFUL) {
		alert("Please load geometry \n");
		break;
	}
	createLatticeStructureMesh();
	liveVar.displayMode = DISPLAY_FINAL_STL;
	guiState = TOPOPT_SUCCESFUL;
	setPlotRoutine(1);
	printf("---------------------------------------------\n");
	break;
case PLOT_PARETO_BUTTON:
	int N;
	if (guiState < TOPOPT_SUCCESFUL){
		alert(" First run top opt \n");
		break;
	}
	N = topopt.getCurrentIsoCounter();
	// There are N+1 samples
	topopt.getResultsAtAllVolFracs(volFractions,relmaxDisp,relmaxStress,relCompliance,timeTaken);

	for (int i =N; i >= 0;i--) {// we have to go backward to ensure that we normalize  [0] last
		relCompliance[i] /= relCompliance[0];
		relmaxStress[i] /= relmaxStress[0];
	}

	xData.clear();
	yData.clear();
	yData.push_back( vector<double>());//  curve 1
	yData.push_back( vector<double>());//  curve 2
	for (int i =N; i >=0;i--){	// There are N+1 samples
		xData.push_back(100*volFractions[i]); 
		yData[0].push_back(relCompliance[i]);
		yData[1].push_back(relmaxStress[i]);
	}
	yLegend.push_back("Compliance");
	yLegend.push_back("Stress");
	plotXYMultiple(xData,yData,yLegend,true,"Volume Percent","Relative Values","Pareto Curves");
	printf("---------------------------------------------\n");
	xData.clear();
	yData[0].clear();
	yData[1].clear();
	yData.clear();

	break;
case PLOT_CGCONVERGENCE_BUTTON:
	if (guiState < THERMALFEA_SUCCESFUL){
		printf("---------------------------------------------\n");
		printf(" First run FEA \n");
		printf("---------------------------------------------\n");
		break;
	}
	N = topopt.getCGNorm(CGNorm);
	xData.clear();
	yData.clear();
	yData.push_back( vector<double>());// one curve
	for (int i =0; i < N;i++){
		xData.push_back(1.0*i); 
		yData[0].push_back(CGNorm[i]);
	}
	yLegend.push_back("");
	plotXY(xData,yData[0],false,"CGIteration","log10(ResidualError)","Residual Error");
	printf("---------------------------------------------\n");
	xData.clear();
	yData[0].clear();
	yData.clear();
	break;
case PLOT_CGITERATIONS_BUTTON:
	if (guiState < STATICFEA_SUCCESFUL){
		printf("---------------------------------------------\n");
		printf(" First run top opt \n");
		printf("---------------------------------------------\n");
		break;
	}

	N = topopt.getLinearSolvesCounter();
	topopt.getCGIterations(CGIterations);
	xData.clear();
	yData.clear();
	yData.push_back( vector<double>());// one curve
	for (int i =0; i < N;i++){
		xData.push_back(1.0*i); 
		yData[0].push_back(CGIterations[i]);
	}
	plotXY(xData,yData[0],false,"FEA #","#CGIterations","Optimization Run");
	printf("---------------------------------------------\n");
	break;
case CREATE_SKELETON_BUTTON:
	createSkeleton();
	break;
case REMOVE_NONMANIFOLD_ELEMENTS_BUTTON:
	printf("Number of non-manifold elements (before): %d \n",topopt.getNumOfNonManifoldElements());
	topopt.removeNonManifoldElements();
	printf("Number of non-manifold elements (after): %d \n",topopt.getNumOfNonManifoldElements());
	break;
case HELP_BUTTON:
	tempInt= (int)ShellExecute( NULL, _T("open"), _T(".\\ParetoWinGuide.pdf"), NULL, NULL, SW_SHOWNORMAL );
	if ( tempInt <= 32 ) {
        MessageBoxA(NULL,"Could not find Pareto User Guide. Contact support@sciartsoft.com","ParetoWorks",MB_OK | MB_APPLMODAL); 
	}
	break;
default:
	break;
	}
	glui->sync_live();
	if (messageGlui != NULL) messageGlui->sync_live();
}
