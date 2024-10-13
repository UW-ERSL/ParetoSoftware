
#include "gCodeReader.hpp"
#include <iostream>
#include<math.h>
#include "ParetoApplication.h"

using namespace std;
extern ParetoApplication       topopt;

/*******************************************************/
gCodeReader::gCodeReader()
{
    gc_CodeNumberOfLines  = 0;
    gc_CodeNumberOfLinesToPlot = 0;

    gc_commandType = NULL;
    gc_xPosition = NULL;
    gc_yPosition= NULL;
    gc_zPosition= NULL;
    gc_extrudedLength= NULL;
    gc_miscCommands= NULL;

    gc_absolutePositioning = true;
    gc_isExtrusionCumulative =false;
    gc_interpolationType = 0;
    gc_cumulativeLengthPrinted = 0;
    gc_origin[0] = gc_origin[1] = gc_origin[2] = 0;
    gc_cumulativeExtrusionLength= 0;
    gc_gcodeUnits = UNITS_MM;
    gc_filamentDiameter = 5;

}
/*******************************************************/
void gCodeReader::safeFree(void *ptr)
{
    if (ptr == NULL) return;
    free(ptr);
}
/*******************************************************/
gCodeReader::~gCodeReader(void)
{ 
    if (gc_CodeNumberOfLines == 0) return;
    freeGCodeMemory();
}
/*******************************************************/
void gCodeReader::printGCodeInformation(void)
{
    cout << "GCode lines:  " << gc_CodeNumberOfLines << endl;
    cout << "GCode units: " << gc_gcodeUnits << endl;
    cout << "GCode filamentDiameter: " << gc_filamentDiameter << endl;
    cout << "GCode layer height: " << gc_layerHeight << endl;
    cout << "GCode time for building Part : "<<gc_totalTimeToPrint<<" seconds " <<endl;
    cout << "X Range: " << gc_boundingBox[0][0] << " " << gc_boundingBox[0][1] << endl;
    cout << "Y Range: " << gc_boundingBox[1][0] << " " << gc_boundingBox[1][1] << endl;
    cout << "Z Range: " << gc_boundingBox[2][0] << " " << gc_boundingBox[2][1] << endl;
}
/*******************************************************/
void gCodeReader::freeGCodeMemory(void)
{ //

    if (gc_CodeNumberOfLines == 0) return;
    safeFree(gc_commandType);
    safeFree(gc_xPosition);
    safeFree(gc_yPosition);
    safeFree(gc_zPosition);
    safeFree(gc_extrudedLength);
    safeFree(gc_miscCommands);

    gc_commandType = NULL;
    gc_xPosition = NULL;
    gc_yPosition= NULL;
    gc_zPosition= NULL;
    gc_commandStartTime= NULL;
    gc_commandEndTime= NULL;
    gc_extrudedLength= NULL;
    gc_miscCommands= NULL;
    gc_CodeNumberOfLines = 0;
    gc_cumulativeLengthPrinted =0;


    gc_printHeadChange.clear();
}
/*******************************************************/
void gCodeReader::allocateGCodeMemory(void)
{ 
    gc_gcodeUnits = UNITS_MM;

    gc_commandType = (int*)malloc((gc_CodeNumberOfLines)*sizeof(int));
    gc_xPosition = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_yPosition = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_zPosition = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_commandStartTime = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_commandEndTime = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_extrudedLength = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));
    gc_miscCommands = (double *)malloc((gc_CodeNumberOfLines)*sizeof(double));

    memset(gc_commandType,0,(gc_CodeNumberOfLines)*sizeof(int));
    memset(gc_xPosition,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_yPosition,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_zPosition,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_commandStartTime,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_commandEndTime,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_extrudedLength,0,(gc_CodeNumberOfLines)*sizeof(double));
    memset(gc_miscCommands,0,(gc_CodeNumberOfLines)*sizeof(double));

    for(int i = 0; i<gc_CodeNumberOfLines;i++)
        gc_commandType[i] = emptyLine; // 999 stands for an empty line



    // Multiple print head
    gc_printHead initialPrintHead;
    initialPrintHead.lineNumber = 0;
    initialPrintHead.toolNumber = 0;
    gc_printHeadChange.push_back(initialPrintHead);

}
/*******************************************************/
void gCodeReader::setGCodeUnits(int units)
{
    gc_gcodeUnits = units;
}
/*******************************************************/
int gCodeReader::getGCodeUnits()
{
    return gc_gcodeUnits;
}
/*******************************************************/
bool gCodeReader::countNumberOfLinesInGCode(string gCodeFileName)
{ // reads the file and saves the no of lines and allocates memory
    string line;
    fstream GCodefile;

    GCodefile.open(gCodeFileName.c_str(), ios::in) ;
    if( GCodefile.is_open())cout <<"file read successful \n";
    else return false;
    gc_CodeNumberOfLines  = 0;
    while(!GCodefile.eof()) {
        while(getline(GCodefile,line)){
            gc_CodeNumberOfLines++;
        }
    }
    GCodefile.close();
    if(gCode_Verbose) cout<<"No of lines : "<<gc_CodeNumberOfLines<<endl;

    return true;
}
/**************************************************************/
double gCodeReader::getEndTimeOfLine(int line)
{
    if (line <0) return 0;
    if (line >= gc_CodeNumberOfLines) return gc_commandEndTime[line-1];
    return gc_commandEndTime[line];
}

/**************************************************************/
void gCodeReader::createSTLFromBoundingBox(double boundingBox[][2],char fileName[])
{
    double v1[3],v2[3],v3[3],nrm;
    double xMin = boundingBox[0][0];
    double yMin = boundingBox[1][0];
    double zMin = boundingBox[2][0];
    double xMax = boundingBox[0][1];
    double yMax = boundingBox[1][1];
    double zMax = boundingBox[2][1];
    double x[] = {xMin, xMax, xMax, xMin, xMin, xMax, xMax, xMin};
    double y[] = {yMin, yMin, yMax, yMax, yMin, yMin, yMax, yMax};
    double z[] = {zMin, zMin, zMin, zMin, zMax, zMax, zMax, zMax};
    char stlFileName[600];
    strcpy(stlFileName,fileName);
    std::cout <<xMin<<": "<<yMin<<": "<<zMin<<": "<<xMax<<": "<<yMax<<": "<<zMax<<endl;
    int *nodes;
    int tri[12][3] =  {{0, 3, 1}, {1, 3, 2}, {0, 5, 4}, {0, 1, 5}, {1, 6, 5}, {1, 2, 6}, {2, 7, 6},{ 2, 3, 7}, {3, 0, 4}, {3, 4, 7}, {4, 5, 7},{ 5, 6, 7}};
    FILE *stlFptr = fopen(stlFileName,"w");

    fprintf(stlFptr,"solid GCode \n");
    for (int i = 0; i < 12; i++) {
        nodes = tri[i];
        v1[0] = x[nodes[1]] - x[nodes[0]];
        v1[1] = y[nodes[1]] - y[nodes[0]];
        v1[2] = z[nodes[1]] - z[nodes[0]];

        v2[0] = x[nodes[2]] - x[nodes[0]];
        v2[1] = y[nodes[2]] - y[nodes[0]];
        v2[2] = z[nodes[2]] - z[nodes[0]];

        CROSS3D(v3,v1,v2);
        nrm = NORM3D(v3);
        for (int j = 0; j < 3;j++)
            v3[j] = v3[j]/nrm;

        fprintf(stlFptr,"\tfacet normal %f %f %f \n",v3[0],v3[1],v3[2]);
        fprintf(stlFptr,"\t\touter loop \n");

        for (int j = 0; j < 3; j++)
            fprintf(stlFptr,"\t\t\t vertex %f %f %f\n",x[nodes[j]],y[nodes[j]],z[nodes[j]]);

        fprintf(stlFptr,"\t\tendloop \n");
        fprintf(stlFptr,"\tendfacet \n");
    }
    fprintf(stlFptr,"endsolid \n");
    fclose(stlFptr);

}
/****************************************************/
void	gCodeReader::setGCodeLayerWidth(double layerWidth)
{
    gc_layerWidth = layerWidth;
}
/****************************************************/
void	gCodeReader::setGcodeFilamentDiameter(double filamentDia)
{
    gc_filamentDiameter = filamentDia;
}
/****************************************************/
void    gCodeReader::setGCodeNozzleDiameter(double nozzleDia)
{
    gc_nozzleDiameter = nozzleDia;
    gc_nozzleDiameter = gc_nozzleDiameter;
}
/****************************************************/
void gCodeReader::setGCodeLayerHeight(double layerHeight)
{
    gc_layerHeight = layerHeight;
}
/****************************************************/
bool gCodeReader::      readGCode(string gCodeFileName)
{
    int totalNumberOfLines;
    int tokenCount = 0;
    int currentLineNumber;
    double currentPosition[3];

    freeGCodeMemory();
    countNumberOfLinesInGCode(gCodeFileName);
    allocateGCodeMemory();
    totalNumberOfLines = gc_CodeNumberOfLines;
    string commandToken[50];
    std::size_t timeStampFound;
    double currentTime,timeStamp,previousTime,timePerLine, timeTemp;
    int previousTimeStampLine;

    ifstream infile;
    infile.open(gCodeFileName.c_str());
    if(!infile.is_open() ) return false;
    currentLineNumber = 0 ;
    std::string line = "" ;
    gc_cumulativeExtrusionLength = 0;
    currentTime = 0;
    previousTimeStampLine = 0;
    gc_feedRate = 10000;
    if ( infile.is_open( ) ) {
        while (!infile.eof()) {
            tokenCount = 0;
            getline(infile , line ) ;
            istringstream iss(line);
            while (iss >> commandToken[tokenCount])
                tokenCount++;

            runGCodeLineNumber(commandToken,tokenCount,currentLineNumber);
            gc_cumulativeExtrusionLength += gc_extrudedLength[currentLineNumber];
            currentPosition[0] = gc_xPosition[currentLineNumber];
            currentPosition[1] = gc_yPosition[currentLineNumber];
            currentPosition[2] = gc_zPosition[currentLineNumber];
            gc_commandStartTime[currentLineNumber] = 0;
            timeStampFound = line.find("TIME_ELAPSED:");
            if(timeStampFound != string::npos ){
                timeStamp = atof(line.substr(timeStampFound + 13).c_str()); // 13 is the offset for TIME_ELAPSED:
                gc_commandEndTime[currentLineNumber] = timeStamp;
                gc_totalTimeToPrint = timeStamp;
                // go back and update time stamp of all previous line numbers
                // assumes equal time for each line (including comment lines!)
                previousTime = gc_commandStartTime[previousTimeStampLine];
                timePerLine =(timeStamp-previousTime)/(currentLineNumber-previousTimeStampLine+1);
                timeTemp = previousTime;
                for (int j = previousTimeStampLine; j < currentLineNumber;j++ ){
                    gc_commandEndTime[j] = timeTemp+timePerLine;
                    gc_commandStartTime[j+1] =timeTemp+timePerLine;
                    timeTemp = timeTemp+timePerLine;
                }
                previousTimeStampLine =currentLineNumber;
            }
            /*
            else { // use the last feedrate
                double timeForThisLine = gc_extrudedLength[currentLineNumber]/gc_feedRate;
                gc_commandEndTime[currentLineNumber] = currentTime+timeForThisLine;
                gc_commandStartTime[currentLineNumber+1] = gc_commandEndTime[currentLineNumber];
                gc_totalTimeToPrint = gc_commandEndTime[currentLineNumber];
                currentTime = gc_commandEndTime[currentLineNumber];
                previousTimeStampLine = currentLineNumber;
            }*/

            currentLineNumber++ ;
            if (currentLineNumber == totalNumberOfLines)
                break;
        }
        infile.close( ) ;
    }
    /*
    for (int j = previousTimeStampLine; j < currentLineNumber;j++ ){
        gc_commandEndTime[j] = gc_commandEndTime[previousTimeStampLine];
        gc_commandStartTime[j+1] = gc_commandEndTime[previousTimeStampLine];
    }
    */
    if(totalNumberOfLines == 0) return false; //file is empty

    computeBoundingBoxFromGCode();
    //estimateLayerHeight();
    return true;
}
/******************************************************************/
int gCodeReader::getNumberOfGCodeLines(void)
{
    return gc_CodeNumberOfLines;
}
/******************************************************************/
void gCodeReader::setNumberOfGCodeLinesToPlot(int line)
{
    gc_CodeNumberOfLinesToPlot = line;
}
/******************************************************************/
int gCodeReader::getNumberOfGCodeLinesToPlot(void)
{
    return gc_CodeNumberOfLinesToPlot;
}
/******************************************************************/
double gCodeReader::getMaterialLengthExtrudedOfGCodeLine(int line)
{
    // get the material extruded in the given line of gcode. It
    // is the difference between prev extr and current extr if it is cumulative
    // otherwise just the data in the given line.

    if(( line > getNumberOfGCodeLines() ) || (line < 1 ) ) return 0;
    if(gc_isExtrusionCumulative == true)
        return gc_extrudedLength[line] - gc_extrudedLength[line - 1];
    else
        return gc_extrudedLength[line];
}
/******************************************************************/
double gCodeReader::getOrientationOfElemsInGCodeLine(int lineNumber) // return the orientation of the elem wrt x-y plane. This info is obtained
//from the raster path
{
    double theta = 0;
    double startPt[3],endPt[3],extrudedLength;
    int commandType;
    getLocationDataOfGCodeLine(lineNumber,startPt,endPt,extrudedLength,commandType);
    if(commandType == 101)
    {
        theta = atan2(endPt[1] -startPt[1],endPt[0]-startPt[0]);
    }
    return theta;
}
/******************************************************************/
int  gCodeReader::getLocationDataOfGCodeLine(int lineNumber,double startPt[3],double endPt[3], double &extrudedLength ,int &commandType)
{
    commandType = 0;
    if ((lineNumber < 1) || (lineNumber >= gc_CodeNumberOfLines))
        return -1;
    endPt[0] = gc_xPosition[lineNumber];
    endPt[1] = gc_yPosition[lineNumber];
    endPt[2] = gc_zPosition[lineNumber];
    if(lineNumber == 0)
        startPt[0] = startPt[1] = startPt[2] = 0;
    else
    {
        startPt[0] = gc_xPosition[lineNumber -1];
        startPt[1] = gc_yPosition[lineNumber -1];
        startPt[2] = gc_zPosition[lineNumber -1];
    }
    extrudedLength = getMaterialLengthExtrudedOfGCodeLine(lineNumber);
    commandType = gc_commandType[lineNumber];

    return 0;
}
/******************************************************************/
int   gCodeReader::getElemsWithGCodeLineNumber(int lineNumber,int *elemsBroughtBack)
{
    if ((lineNumber < 1) || (lineNumber >= gc_CodeNumberOfLines)) return 0;
    int numElems;
    double startPt[3],endPt[3],extrudedLength,distTravelled,radius;
    int commandType;

    getLocationDataOfGCodeLine(lineNumber,startPt,endPt,extrudedLength,commandType);
    if(commandType != 101) return 0; //only G01 commands result in movement

    if (abs(startPt[2] - endPt[2]) > getGCodeLayerHeight()/2) return 0; // discard z movement
    distTravelled = DIST3D(startPt,endPt);
    if( (distTravelled <gc_layerHeight*0.01)) return 0;
    radius = gc_layerHeight/2.0;
    for (int i = 0; i < 3; i++){
        startPt[i] = startPt[i] - radius*(endPt[i]-startPt[i])/extrudedLength; // extend slightly to close gaps
        endPt[i] = endPt[i] + radius*(endPt[i]-startPt[i])/extrudedLength; // extend slightly to close gaps
    }

    numElems = topopt.getElemsInsideLongBox(startPt,endPt,1.5*radius,topopt.getNumElems(),elemsBroughtBack);
    return numElems;
}
/******************************************************************/
void gCodeReader::estimateLayerHeight()
{
    int counter;
    double zDiffCumulative;
    counter = 0;
    zDiffCumulative = 0;
    for(int i = 1;i< gc_CodeNumberOfLines;i++){
        if ((gc_commandType[i] == 100)  && (gc_zPosition[i] > gc_zPosition[i-1])){
            counter += 1;
            zDiffCumulative += (gc_zPosition[i]-gc_zPosition[i-1]);
        }
    }
    if (counter > 0)
        gc_layerHeight = zDiffCumulative/counter;
    else
        gc_layerHeight = 1;
    cout << "Estimated layer height (mm): " << gc_layerHeight << endl;
}
/******************************************************************/
void gCodeReader::computeBoundingBoxFromGCode()
{
    /*
     * Compute the bounding box from the gcodes
     * Optionally, include the z height of the build plate
     *
     * */
    double boundingBox[3][2];
    for (int i = 0; i < 3; i++) {
        boundingBox[i][0] = HUGE_VAL;
        boundingBox[i][1] =- HUGE_VAL;
    }
    double extrudedLengthOfLine;
    for(int i = 0;i< gc_CodeNumberOfLines;i++){
        extrudedLengthOfLine = getMaterialLengthExtrudedOfGCodeLine(i);
        if((gc_commandType[i] == 101)  && (extrudedLengthOfLine > 0) ){
            boundingBox[0][0] = MIN(boundingBox[0][0],gc_xPosition[i]);
            boundingBox[0][1] = MAX(boundingBox[0][1],gc_xPosition[i]);

            boundingBox[1][0] = MIN(boundingBox[1][0],gc_yPosition[i]);
            boundingBox[1][1] = MAX(boundingBox[1][1],gc_yPosition[i]);

            boundingBox[2][0] = MIN(boundingBox[2][0],gc_zPosition[i]);
            boundingBox[2][1] = MAX(boundingBox[2][1],gc_zPosition[i]);
        }
    }
    double range[3];
    for (int i = 0; i < 3; i++)
        range[i] = boundingBox[i][1] - boundingBox[i][0];
    for(int i =0; i<3;i++){
        gc_boundingBox[i][0] = boundingBox[i][0] - 0.05*range[i];
        gc_boundingBox[i][1] = boundingBox[i][1] + 0.05*range[i];

    }
    return;
}
/******************************************************************/
void gCodeReader::getBoundingBoxFromGCode(double boundingBox[3][2], double center[3])
{
    for(int i =0; i<3;i++){
        boundingBox[i][0] = gc_boundingBox[i][0];
        boundingBox[i][1] = gc_boundingBox[i][1];
        center[i] = 0.5*(gc_boundingBox[i][0] + gc_boundingBox[i][1]);
    }
    return;
}
/***********************************************************/
double gCodeReader::getGCodeLayerHeight(void)
{
    return gc_layerHeight;
}
/***********************************************************/
double gCodeReader::getGCodeLayerWidth(void)
{
    return gc_layerWidth;
}
/***********************************************************/
int  gCodeReader::getNumberOfGCodeLayers(void)
{
    // return the number of gcode layers being printed
    double boundingBox[3][2],center[3],ZHeight;
    int numLayers;
    getBoundingBoxFromGCode(boundingBox,center);
    ZHeight = boundingBox[2][1] - boundingBox[2][0];
    double layerheight = getGCodeLayerHeight();
    numLayers = ZHeight/layerheight;
    return numLayers;
}
/***********************************************************/
float gCodeReader::getGCodeTimeToPrint(void)
{
    return gc_totalTimeToPrint;
}

/***************************************************************/
double gCodeReader::getGCodeFilamentDiameter(void)
{
    return gc_filamentDiameter;
}

/***************************************************************/
int gCodeReader::runGCodeLineNumber(string commandToken[] , int tokenCount, int currentLineNumber)
{ // int lineNumber was the previous passing val
    int count = 0;
    int returnToken = -1;
    if(commandToken[0].compare("G0") == 0 || commandToken[0].compare("G00") == 0){ // if it is a rapid  interpolation command
        returnToken = executeG0(commandToken , tokenCount , currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("G1") == 0 || commandToken[0].compare("G01") == 0){ // if it is a linear interpolation command
        returnToken = executeG1(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].substr(0,1).compare("T") == 0){  // Print head change
        returnToken = executePrintHeadChange(commandToken,tokenCount,currentLineNumber);
    }
    else if(commandToken[0].compare("G4") == 0){
        returnToken = executeG4(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("G20") == 0){
        returnToken = executeG20(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("G21") == 0){
        returnToken = executeG21(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("G28") == 0){
        returnToken = executeG28(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("G90") == 0){
        returnToken = executeG90(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else  if(commandToken[0].compare("G92") == 0){
        returnToken = executeG92(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M0") == 0 || commandToken[0].compare("M00") == 0){

        returnToken = executeM0(commandToken , tokenCount, currentLineNumber);
        count++;
    }
    else  if(commandToken[0].compare("M82") == 0){
        returnToken = executeM82(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M83") == 0){
        returnToken = executeM83(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M104") == 0){
        returnToken = executeM109(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else  if(commandToken[0].compare("M106") == 0){
        returnToken = executeM106(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M107") == 0){
        returnToken = executeM107(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M109") == 0){ // 104 and 109 are essentiallly the same
        returnToken = executeM109(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M140") == 0){
        returnToken = executeM140(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(commandToken[0].compare("M8888") == 0){
        returnToken = executeM8888(commandToken, tokenCount, currentLineNumber);
        count++;
    }
    else if(count ==0){ // if none of them were executed
        returnToken  = executeNULLCommand(currentLineNumber);
    }
    return returnToken  ;
}
/*******************************************/
int gCodeReader::executeG0(string commandToken[] , int tokenCount, int currentLineNumber)
{
    gc_interpolationType = 0; //G0 is rapid interpolation command
    bool xCommand = false; bool yCommand = false; bool zCommand = false;
    gc_commandType[currentLineNumber ] = 100;
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("X") != std::string::npos) { // see if we have a x position command
            double x = atof(commandToken[i].substr(1).c_str()) ;
            xCommand =true;
            if(gc_absolutePositioning)
                gc_xPosition[currentLineNumber] = x + gc_origin[0];
            else
                gc_xPosition[currentLineNumber] = x + gc_xPosition[currentLineNumber-1];
        }
        if (commandToken[i].find("Y") != std::string::npos) { // see if we have a y position command
            double y = atof(commandToken[i].substr(1).c_str()) ;
            yCommand = true;
            if(gc_absolutePositioning)
                gc_yPosition[currentLineNumber] = y + gc_origin[1];
            else
                gc_yPosition[currentLineNumber] =gc_yPosition[currentLineNumber-1] +  y;
        }
        if (commandToken[i].find("Z") != std::string::npos) { // see if we have a z position command
            double z = atof(commandToken[i].substr(1).c_str()) ;
            zCommand =true;
            if(gc_absolutePositioning)
                gc_zPosition[currentLineNumber] = z + gc_origin[2];
            else
                gc_zPosition[currentLineNumber] =gc_zPosition[currentLineNumber-1] +  z;
        }
        if (commandToken[i].find("E") != std::string::npos) { // see if we have a extruder position command
            double e = atof(commandToken[i].substr(1).c_str()) ;
            // gc_extrudedLength is always for that line number, NOT CUMULATIVE
            gc_extrudedLength[currentLineNumber]  =  e;

        }
        if (commandToken[i].find("F") != std::string::npos) { // see if we have a gc_feedRate position command
            gc_feedRate = atof(commandToken[i].substr(1).c_str()) ;

        }
    }
    if(xCommand ==false) gc_xPosition[currentLineNumber] = gc_xPosition[currentLineNumber-1];
    if(yCommand ==false) gc_yPosition[currentLineNumber] = gc_yPosition[currentLineNumber-1];
    if(zCommand ==false) gc_zPosition[currentLineNumber] = gc_zPosition[currentLineNumber-1];

    return 100;
}
/*******************************************************/
int gCodeReader::executeG1(string commandToken[] , int tokenCount, int currentLineNumber)
{
    bool xCommand = false; bool yCommand = false; bool zCommand = false;
    gc_interpolationType = 1; //G0 is rapid interpolation command
    gc_commandType[currentLineNumber ] = 101;
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("X") != std::string::npos) { // see if we have a x position command
            double x = atof(commandToken[i].substr(1).c_str()) ;
            xCommand = true;
            if(gc_absolutePositioning)
                gc_xPosition[currentLineNumber] = x + gc_origin[0];
            else
                gc_xPosition[currentLineNumber] = x + gc_xPosition[currentLineNumber-1];
        }
        if (commandToken[i].find("Y") != std::string::npos) { // see if we have a y position command
            double y = atof(commandToken[i].substr(1).c_str()) ;
            yCommand = true;
            if(gc_absolutePositioning)
                gc_yPosition[currentLineNumber] = y + gc_origin[1];
            else
                gc_yPosition[currentLineNumber] =gc_yPosition[currentLineNumber-1] +  y;
        }
        if (commandToken[i].find("Z") != std::string::npos) { // see if we have a z position command
            double z = atof(commandToken[i].substr(1).c_str()) ;
            zCommand = true;
            if(gc_absolutePositioning)
                gc_zPosition[currentLineNumber] = z + gc_origin[2];
            else
                gc_zPosition[currentLineNumber] = gc_zPosition[currentLineNumber-1] +  z;
        }
        if (commandToken[i].find("E") != std::string::npos) { // see if we have a extruder position command
            double e = atof(commandToken[i].substr(1).c_str()) ;
            // gc_extrudedLength is always for that line number
            gc_extrudedLength[currentLineNumber]  = e;
        }
        if (commandToken[i].find("F") != std::string::npos) { // see if we have a gc_feedRate position command
             gc_feedRate= atof(commandToken[i].substr(1).c_str()) ;
        }

    }
    if(xCommand ==false) gc_xPosition[currentLineNumber] = gc_xPosition[currentLineNumber-1];
    if(yCommand ==false) gc_yPosition[currentLineNumber] = gc_yPosition[currentLineNumber-1];
    if(zCommand ==false) gc_zPosition[currentLineNumber] = gc_zPosition[currentLineNumber-1];
    double distanceToTravel = sqrt(pow( (gc_xPosition[currentLineNumber] - gc_xPosition[currentLineNumber-1]) , 2 ) + ( (gc_yPosition[currentLineNumber] - gc_yPosition[currentLineNumber-1]) , 2) + pow(gc_zPosition[currentLineNumber] - gc_zPosition[currentLineNumber -1] , 2) );
    gc_cumulativeLengthPrinted +=  distanceToTravel;

    return 101;
}
/*******************************************************/
void gCodeReader::setPrintHeadCurrentPositionSameAsPrevious(int currentLineNumber)
{
    gc_xPosition[currentLineNumber] = gc_xPosition[currentLineNumber -1];
    gc_yPosition[currentLineNumber] = gc_yPosition[currentLineNumber -1];
    gc_zPosition[currentLineNumber] = gc_zPosition[currentLineNumber -1];
    gc_extrudedLength[currentLineNumber ] = gc_extrudedLength[currentLineNumber-1];
}
/*******************************************************/
int gCodeReader::executePrintHeadChange(string commandToken[], int tokenCount, int currentLineNumber)
{
    gc_commandType[currentLineNumber] = GCODE_COMMAND_PRINTHEADCHANGE;
    gc_printHead newPrintHead;
    newPrintHead.lineNumber = currentLineNumber;
    newPrintHead.toolNumber = stoi(commandToken[0].substr(1));
    //cout<<" Changed to tool "<<newPrintHead.toolNumber <<" in line "<<newPrintHead.lineNumber<<endl;
    gc_printHeadChange.push_back(newPrintHead);
    return GCODE_COMMAND_PRINTHEADCHANGE;
}
/*******************************************************/
int gCodeReader::executeG4(string commandToken[] , int tokenCount, int currentLineNumber){

    gc_commandType[currentLineNumber] = 104;

    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 104;
}
/********************************************************/

int gCodeReader::executeG21(string commandToken[], int tokenCount, int currentLineNumber)
{ // set units to mm
    gc_commandType[currentLineNumber] = 121;
    gc_gcodeUnits = UNITS_MM;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);

    return 121;
}
/**********************************************************/
int gCodeReader::executeG20(string commandToken[], int tokenCount, int currentLineNumber)
{ // set units to inches
    gc_gcodeUnits = UNITS_INCH;
    gc_commandType[currentLineNumber] = 122;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);

    return 120;
}
/**********************************************************/
int gCodeReader::executeG28(string commandToken[] , int tokenCount, int currentLineNumber){
    int count = 0;
    gc_commandType[currentLineNumber] = 128;

    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("X") != std::string::npos) { // see if we have a x position command
            gc_xPosition[currentLineNumber ] = gc_origin[0];
            count ++;
        }
        if (commandToken[i].find("Y") != std::string::npos) { // see if we have a y position command
            gc_yPosition[currentLineNumber ] = gc_origin[1];
            count ++;
        }
        if (commandToken[i].find("Z") != std::string::npos) { // see if we have a z position command
            gc_zPosition[currentLineNumber ] = gc_origin[2];
            count ++;
        }
    }
    if(count == 0 ){ // if given only the G28 command, set all axis to 0
        gc_xPosition[currentLineNumber] = gc_origin[0];
        gc_yPosition[currentLineNumber] = gc_origin[1];
        gc_zPosition[currentLineNumber] = gc_origin[2];
    }
    return 128;
}
/*****************************************************/
int gCodeReader::executeG90(string commandToken[] , int tokenCount, int currentLineNumber){
    //G90 sets to absolute programming
    gc_absolutePositioning = true;
    gc_commandType[currentLineNumber] = 190;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 190;
}

/*******************************************************/
int gCodeReader::executeG91(string commandToken[] , int tokenCount, int currentLineNumber){
    //G91 sets positioning to relative method
    gc_absolutePositioning = false;
    gc_commandType[currentLineNumber] = 191;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 191;
}
/******************************************************/
int gCodeReader::executeG92(string commandToken[] , int tokenCount, int currentLineNumber){
    //G92 sets new positions without any motion. to relative method

    gc_commandType[currentLineNumber] = 192;
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("E") != std::string::npos) { // see if we have a extruder position command
            double e = atof(commandToken[i].substr(1).c_str()) ;
            // gc_extrudedLength is always for that line number
            gc_extrudedLength[currentLineNumber]  = e;

        }
    }
    gc_xPosition[currentLineNumber] = gc_xPosition[currentLineNumber-1];
    gc_yPosition[currentLineNumber] = gc_yPosition[currentLineNumber-1];
    gc_zPosition[currentLineNumber] = gc_zPosition[currentLineNumber-1];
    return 192;
}
/**********************************************************/
int gCodeReader::executeNULLCommand(int currentLineNumber){
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_commandType[currentLineNumber] = emptyLine;
    return emptyLine;
}

/***********************************************************/
/**********************M-CODES******************************/
/***********************************************************/
// ATTENTION ::: for m codes, return 300+command number
/***********************************************************/
int gCodeReader::executeM0(string commandToken[] , int tokenCount, int currentLineNumber){

    gc_commandType[currentLineNumber] = 300;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 300;
}
/***********************************************************/
int gCodeReader::executeM82(string commandToken[] , int tokenCount, int currentLineNumber){
    gc_isExtrusionCumulative = true;// cumulative extrusion
    gc_commandType[currentLineNumber] = 382;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 382;
}
/************************************************/
int gCodeReader::executeM83(string commandToken[] , int tokenCount, int currentLineNumber){
    gc_isExtrusionCumulative = false; // not cumulative
    gc_commandType[currentLineNumber] = 383;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 383;
}
/**************************************************/
int gCodeReader::executeM84(string commandToken[] , int tokenCount, int currentLineNumber){
    gc_commandType[currentLineNumber] = 384;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    return 384;
}
/************************************************/
int gCodeReader::executeM106(string commandToken[] , int tokenCount, int currentLineNumber){ // set fan speed. get function to correlate heat trans coeff and fan speed
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("S") != std::string::npos) {
            gc_fanSpeed = atof(commandToken[i].substr(1).c_str()) ;
        }
    }
    gc_commandType[currentLineNumber] = 406;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_miscCommands[currentLineNumber] = gc_fanSpeed;
    return 406;
}
/**********************************************/
int gCodeReader::executeM107(string commandToken[] , int tokenCount, int currentLineNumber)
{
    gc_fanSpeed = 0; // switch off the fan
    gc_commandType[currentLineNumber] = 407;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_miscCommands[currentLineNumber] = gc_fanSpeed;
    return 407;
}/************************************************/
int gCodeReader::executeM109(string commandToken[] , int tokenCount, int currentLineNumber)
{ // assume heating at an avg speed
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("S") != std::string::npos) {
            gc_extruderTemperature = atof(commandToken[i].substr(1).c_str()) ;
        }
    }
    gc_commandType[currentLineNumber] = 409;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_miscCommands[currentLineNumber] = gc_extruderTemperature;
    return 409;
}
/***********************************************/
int gCodeReader::executeM140(string commandToken[] , int tokenCount, int currentLineNumber)
{
    for(int i = 0; i< tokenCount ; i++){ // for each command token
        if (commandToken[i].find("S") != std::string::npos) {
            gc_bedTemperature = atof(commandToken[i].substr(1).c_str()) ;
        }
    }
    gc_commandType[currentLineNumber] = 440;
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_miscCommands[currentLineNumber] = gc_bedTemperature;
    return 440;
}
/**********************************************/
int gCodeReader::executeM8888(string commandToken[] , int tokenCount, int currentLineNumber)
{
    setPrintHeadCurrentPositionSameAsPrevious( currentLineNumber);
    gc_commandType[currentLineNumber ] = 8888;
    return 8888;
}
/***************************************************/
double gCodeReader::getGCodeNozzleDiameter(void)
{
    return gc_nozzleDiameter;
}
/***************************************************/
void gCodeReader::getGeometryToGCodeScalingFactor(double geomBB[3][2],double scalingFactor[3])
{
    // This func is to fit the geom perfectly to the gcode.
    // Using the geom bb and the geom bb, compute scaling factor along x, y,z separately
    double gcodeBB[3][2],center[3];
    getBoundingBoxFromGCode(gcodeBB, center);
    for (int axis = 0 ; axis < 3; axis++){
        scalingFactor[axis] = ( gcodeBB[axis][1] - gcodeBB[axis][0] )/( geomBB[axis][1] - geomBB[axis][0] );
    }
    return;
}
/***************************************************/
double gCodeReader::getGCodeToSTLScalingFactor(int stlUnits){
    /*#define meters 0
    #define inches 1
    #define millimeters 2
    */

    return 1;
    double scalingFactor;
    if(stlUnits == 0)
        scalingFactor = 1;
    else if(stlUnits == 1)
        scalingFactor = 25.4e-3;
    else if (stlUnits = 2)
        scalingFactor = 1e-3;


    return scalingFactor;
}
/**********************************************************************/
void gCodeReader::shiftGCodeby(double offset[3])
{
    for (int line = 0; line < gc_CodeNumberOfLines; line++){
        gc_xPosition[line] +=offset[0];
        gc_yPosition[line] +=offset[1];
        gc_zPosition[line] +=offset[2];
    }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            gc_boundingBox[i][j]  +=offset[i];
}

/**********************************************************************/
void gCodeReader::alignGCodeWithSTL(void)
{

    double offset[3],maxXLocSTL[3],maxXLocGCode[3];

    return;
    for (int line = 0; line < gc_CodeNumberOfLines; line++){
        gc_xPosition[line] -=offset[0];
        gc_yPosition[line] -=offset[1];
        gc_zPosition[line] -=offset[2];
    }

    for (int line = 0; line < gc_CodeNumberOfLines; line++){
        gc_xPosition[line] -=offset[0];
        gc_yPosition[line] -=offset[1];
        gc_zPosition[line] -=offset[2];
    }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            gc_boundingBox[i][j]  -=offset[i];
}
/*********************************************************************/
void gCodeReader::shiftGCodeToOrigin(void)
{
    double offset[3];

    offset[0] = (gc_boundingBox[0][0] + gc_boundingBox[0][1])/2;
    offset[1] = (gc_boundingBox[1][0] + gc_boundingBox[1][1])/2;
    offset[2] = gc_boundingBox[2][0];

    //offset[0] = gc_xPosition[0];
    //offset[1] = gc_xPosition[1];
    //offset[2] = gc_xPosition[2];

    for (int line = 0; line < gc_CodeNumberOfLines; line++){
        gc_xPosition[line] -=offset[0];
        gc_yPosition[line] -=offset[1];
        gc_zPosition[line] -=offset[2];
    }
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            gc_boundingBox[i][j]  -=offset[i];

}
/**********************************************************************/
void gCodeReader::scaleGCode(double scale)
{
    for (int line = 0; line < gc_CodeNumberOfLines; line++){
        gc_xPosition[line] *=scale;
        gc_yPosition[line] *=scale;
        gc_zPosition[line] *=scale;
    }
    gc_filamentDiameter *= scale;
    gc_nozzleDiameter *= scale;
    gc_layerWidth *= scale;
    gc_layerHeight *= scale;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            gc_boundingBox[i][j] *= scale;

    for (int i = 0; i < gc_CodeNumberOfLines; i++)
        gc_extrudedLength[i]  *= scale;
}

/**********************************************************************/
double gCodeReader::getNetFilamentLengthExtruded(){
    double filLen = 4*getVolumeOfPrintedObject()/(3.14159*pow(getGCodeFilamentDiameter(),2));
    return filLen;
}
/**********************************************************/
double gCodeReader::getVolumeOfPrintedObject(){
    double vol = 3.14159*getGCodeLayerHeight()*getGCodeNozzleDiameter()*getGCodeNetLengthPrinted()*0.25;
    return vol ;

}
/**********************************************************************/
double gCodeReader::getGCodeNetLengthPrinted()
{
    // Return the net sum of the printed segments
    return gc_cumulativeLengthPrinted;
}
/**********************************************************************/
double gCodeReader::getDistanceTravelled(double startPt[3],double endPt[3])
{
    // compute L2 norm dist between start Pt and end Pt
    double distance = 0;
    for(int i = 0 ; i < 3 ; i++)
        distance += pow( (startPt[i] - endPt[i]) , 2);
    return sqrt(distance);
}
/**********************************************************************/
int gCodeReader::getPrintHeadAtIndex(int index)
{
    if((index > gc_printHeadChange.size() )|| (index == TOOL_NOT_FOUND_ERROR)) {
        index = gc_printHeadChange.size() -1;
    }
    return gc_printHeadChange.at(index).toolNumber;
}
/**********************************************************************/
int gCodeReader::getCurrentPrintHeadIndex(int lineNumber)
{
    /*
     * @param lineNumber : gCode line number enquired
     * @return : return the index of vector at which line occurs
     * @info : Compound this with function getPrintHeadAtIndex to get
     *          the tool used at current index
     * */
    for(int i = 0 ; i < gc_printHeadChange.size()-1 ; i++)
    {
        if((gc_printHeadChange.at(i).lineNumber <= lineNumber) && (gc_printHeadChange.at(i+1).lineNumber >= lineNumber))
            return i;
    }
    return TOOL_NOT_FOUND_ERROR;
}
/**********************************************************************/
double  gCodeReader:: getNormalizedPositionOfElemCenterWithBuildVector(double elemCenter[3], double bldVec[3])
{
    // return {$\vec{b}.\vec{r_k}$}
    double norm = sqrt( pow(bldVec[0],2) + pow(bldVec[1],2) + pow(bldVec[2],2) );
    for(int i = 0 ; i < 3 ; i++)
        bldVec[i] /= norm;
    double lay = 0;
    for(int i = 0 ; i < 3 ; i++)
        lay += elemCenter[i]*bldVec[i];
    return lay;
}
/**********************************************************************/
bool gCodeReader:: getIfOverhanging(double normal[3], double buildDir[3], double ovhangAngle)
{
    // normalize the vectors
    double normNormal = sqrt( pow(normal[0],2) + pow(normal[1],2) + pow(normal[2],2) );
    double normBuildDir = sqrt( pow(buildDir[0],2) + pow(buildDir[1],2) + pow(buildDir[2],2) );
    for(int i = 0 ; i < 3 ; i++)
    {
        normal[i] /= normNormal;
        buildDir[i] /= normBuildDir;
    }

    double ovVal = DOTPROD3D(normal,buildDir) - cos(ovhangAngle);
    bool isOvhang = false;
    if(ovVal >= 0 )
        isOvhang = true;
    return isOvhang;
}
/**********************************************************************/











