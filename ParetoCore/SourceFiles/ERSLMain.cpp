#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "AFElasticityGPUDefines.h"
#include "CommonGUI.h"
#include "TopOpt.h"
#include "ParetoApplication.h"
#include "Timer.h"
#include "Voxelizer.h"
#include "text3d.h"

#ifdef _WIN32
#include <process.h>
#endif

#ifdef COMPILE_GPUCODE
#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "cudart.lib")
#pragma comment(lib, "cutil64.lib")
#pragma comment(lib, "shrUtils64.lib")
#pragma comment(lib, "cusparse.lib")
#pragma comment(lib, "cublas.lib")
#endif


#if (COMPILE_MODE == BATCH_MODE)
#pragma comment(lib, "msvcrt.lib")
#else
#include <GL/glui.h>
#pragma comment(lib, "glut64.lib")
#pragma comment(lib, "glui64.lib")
#pragma comment(lib, "COMMONGUILib.lib")
#endif

#pragma warning(disable:4996)

extern struct LiveVariables liveVar;
extern void oneTimeInitialization();
extern void initializeLiveVariables();
extern void loadPRJ(char *extension = ".prj");
extern void cleanUp();
extern void getDateMonthYear(int dateMonthYear[3]);

extern void GUI_ParetoWin();
extern void GUI_ParetoResearch();
extern void GUI_ThermoElasticity();

extern void DOSWindowChange(int control);
extern bool checkUserMacAddress(void);
extern bool checkTemporaryLicense(char temporaryLicense[]);

void (*otherPlottingFunction)();//used in ParetoApplications

void noPlotting(void)
{
	;// needed for compilation used in ParetoApplications
}

/************************************************************/

#if COMPILE_MODE == RESEARCH_MODE
int main(int argc, char *argv[])
{
	if (!checkUserMacAddress()) {
		printf("Invalid license \n");
		getchar();
		return 0;
	}

	oneTimeInitialization();
	initializeLiveVariables();
	otherPlottingFunction = noPlotting;
	liveVar.usingGUI  = 1;
	liveVar.usingGUIMode = PARETO;

	GUI_ParetoResearch();
	
	cleanUp();
	return 0;
}

#elif COMPILE_MODE == BATCH_MODE

int main(int argc, char *argv[])
{
	char tempLicense[256];
	bool OK;

	OK = true;
#ifdef _WIN32
	OK = checkUserMacAddress();
#else
	OK = false;
#endif
	if (!OK) {// permanent license not valid
		// check for temporary license
		//cout << "Current directory is " << ExePath() << endl;
		FILE *fptr = fopen("ParetoWinLicense.txt","r");
		if (fptr == NULL){
			printf("Could not find license file ParetoWinLicense.txt\n");
			printf("Error: Contact support@sciartsoft.com\n");
			return 0;
		}
		// check 
		fscanf(fptr,"%s",tempLicense);
		fclose(fptr);
		printf("Temporary license: %s \n",tempLicense);
		OK = checkTemporaryLicense(tempLicense);
		if (!OK) {
			printf("Error: Contact support@sciartsoft.com\n");
			return 0;
		}
	}

	if (argc < 2) {
		printf("Batch mode requires project file \n");
		printf("Example: Pareto beam.prj\n");
		return 0;
	}
	oneTimeInitialization();
	liveVar.usingGUIMode = PARETO;
	initializeLiveVariables();

	liveVar.usingGUI = 0;
	strcpy(liveVar.prjFileName,argv[1]);
	loadPRJ();
	
	cleanUp();
	return 0;
}
#else

#endif
