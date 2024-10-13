
#include <iostream>
using namespace std;
#include "CoreFunctions.hpp"
#include "Renderer.hpp"
#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "LiveVariables.hpp"


ParetoApplication       topopt;
ParetoApplication       topoptTemp;//used in certain applications (ex: lattice structure design)
Voxelizer               vox;
Voxelizer				voxTemp;
LiveVariables           liveVar;
CoreFunctions           coreFunctions;
Renderer                renderer;


#ifdef COMPILE_GPUCODE
#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "cudart.lib")
#pragma comment(lib, "cutil64.lib")
#pragma comment(lib, "shrUtils64.lib")
#pragma comment(lib, "cusparse.lib")
#pragma comment(lib, "cublas.lib")
#endif

// dummy functions
bool getYesNoResponse(const char format[],...){ return true;}
void print2DText(int x, int y, const char string[], float clr[3] = NULL){ return;}
void updateProgressBar(const char title[], int percentComplete){ return;}
void resetProgressBar(const char title[]){ return;}
void setStatusMessage(const char str[]){ return;}
bool t3dInit(char const *){ return true;}

/****************************************************************************/
void writeWarningToLog(const char format[], ...)
{
	va_list(args);
	char tempStr[1024 * 16];
	va_start(args, format);
	vsprintf(tempStr, format, args);
	va_end(args);

	cout << tempStr << endl;

	if (liveVar.logFilePtr == NULL)return;

	fprintf(liveVar.logFilePtr, "%s \n", tempStr);
	fflush(liveVar.logFilePtr);

}
/****************************************************************************/
void writeMessageToLog(const char format[], ...)
{
	va_list(args);
	char tempStr[1024 * 16];
	va_start(args, format);
	vsprintf(tempStr, format, args);
	va_end(args);


	cout << tempStr << endl;

	if (liveVar.logFilePtr == NULL)return;

	fprintf(liveVar.logFilePtr, "%s \n", tempStr);
	fflush(liveVar.logFilePtr);
}

/****************************************************************************/
void writeWarningToGUIWindow(const char format[], ...)
{
	va_list(args);
	char tempStr[1024 * 16];
	va_start(args, format);
	vsprintf(tempStr, format, args);
	va_end(args);

	cout << tempStr << endl;
}
/****************************************************************************/
void writeMessageToGUIWindow(const char format[], ...)
{
	va_list(args);
	char tempStr[1024 * 16];
	va_start(args, format);
	vsprintf(tempStr, format, args);
	va_end(args);

	cout << tempStr << endl;
	if (liveVar.logFilePtr == NULL)return;

	fprintf(liveVar.logFilePtr, "%s \n", tempStr);
	fflush(liveVar.logFilePtr);
}
/****************************************************************************/
int main(int argc, char** argv)
{
    std::cout.precision(4);
    coreFunctions.oneTimeInitialization();
    coreFunctions.initializeLiveVariables();
    coreFunctions.resetLiveVariables();

    if (argc < 2) {
        cout << "Batch mode requires project file " << endl;
        cout << "Example: ParetoBatchMode LBracket.prj " << endl;
        return 0;
    }

    liveVar.prjFileName = std::string(argv[1]);

	coreFunctions.loadPRJ(".prj");
    return 0;
}
