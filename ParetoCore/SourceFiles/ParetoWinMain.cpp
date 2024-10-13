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

extern bool checkUserMacAddress(void);
extern bool checkTemporaryLicense(char temporaryLicense[]);


#include <GL/glui.h>
#pragma comment(lib, "glut64.lib")
#pragma comment(lib, "glui64.lib")
#pragma comment(lib, "COMMONGUILib.lib")


#pragma warning(disable:4996)

extern struct LiveVariables liveVar;
extern void oneTimeInitialization();
extern void initializeLiveVariables();
extern void cleanUp();
extern void GUI_ParetoWin();

extern bool checkUserMacAddress(char userMACAddress[]);

void (*otherPlottingFunction)();//used in ParetoApplications

void noPlotting(void)
{
	;// needed for compilation used in ParetoApplications
}

/////////////////////////////

// To use a DOS console-free mode, uncomment below and change subsystem in Linker-System to Windows
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,  int  nCmdShow)
// To use a DOS-console mode, uncomment below and change subsystem in Linker-System to Console
//int main(int argc, char *argv[])
{
	char tempLicense[256];
	bool OK = checkUserMacAddress();
	if (!OK) {// permanent license not valid
		// check for temporary license
		FILE *fptr = fopen("ParetoWinLicense.txt","r");
		if (fptr == NULL){
			printf("Could not find license file ParetoWinLicense.txt\n");
			printf("Error: Contact support@sciartsoft.com\n");
			MessageBoxA(NULL,"Could not find ParetoWinLicense.txt; contact support@sciartsoft.com","Pareto",MB_OK);	
			return 0;
		}
		// check 
		fscanf(fptr,"%s",tempLicense);
		fclose(fptr);
		printf("Temporary license: %s \n",tempLicense);
		OK = checkTemporaryLicense(tempLicense);
		if (!OK) {
			char tempStr[1000];
			sprintf(tempStr,"Temporary license %s has expired; contact support@sciartsoft.com",tempLicense);
			printf("%s\n",tempStr);
			MessageBoxA(NULL,tempStr,"Pareto",MB_OK);	
			return 0;
		}
	}

	oneTimeInitialization();
	initializeLiveVariables();
	otherPlottingFunction = noPlotting;
	liveVar.usingGUI  = 1;
	liveVar.usingGUIMode = PARETO;

	GUI_ParetoWin();
	
	cleanUp();
	return 0;
}
