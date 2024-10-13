/*
 * gCodeReader.hpp

 * Author: aaditya Chandrasekhar
 */

#pragma once
#define gCode_Verbose true
#include <algorithm>
#include<iostream>
#include<math.h>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<string.h>
#include<time.h>
#include<limits>
#include <stdio.h>
#include <sstream>
#include <list>

#define UNITS_METER 0
#define UNITS_INCH 1
#define UNITS_MM 2
#define UNITS_CM 3

#define DOTPROD3D(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SCALE(v,scale) v[0] /=scale; v[1] /=scale; v[2]/=scale;
#define NORM3D(v1) (sqrt(v1[0]*v1[0]+v1[1]*v1[1]+v1[2]*v1[2]));


using namespace std;
#define GCODE_COMMAND_PRINTHEADCHANGE 145 // TODO : change this to enum
#define TOOL_NOT_FOUND_ERROR -2

#define emptyLine 999
class gCodeReader {
private:
    string currentLine;
    int executeG0(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeG1(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeG4(string commandToken[], int tokenCount, int currentLineNumber);
    int executeG20(string commandToken[], int tokenCount,int currentLineNumber);
    int executeG21(string commandToken[], int tokenCount,int currentLineNumber);
    int executeG28(string commandToken[], int tokenCount,int currentLineNumber);

    int executeG90(string commandToken[] , int tokenCount, int currentLineNumber);
    int executeG91(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeG92(string commandToken[] , int tokenCount,int currentLineNumber);

    int executePrintHeadChange(string commandToken[] , int tokenCount,int currentLineNumber);

    /********************************/
    int executeM0(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM82(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM83(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM84(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM106(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM107(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM109(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM140(string commandToken[] , int tokenCount,int currentLineNumber);
    int executeM8888(string commandToken[] , int tokenCount,int currentLineNumber);
    void setPrintHeadCurrentPositionSameAsPrevious(int currentLineNumber);

    int     executeNULLCommand(int currentLineNumber);
    double  getMaterialLengthExtrudedOfGCodeLine(int line);
    void    computeBoundingBoxFromGCode();
    void    convertGCodeDistanceToMeter(); // Internally working with meters. Hence, convert all distance travelled units to meters
protected:

    string gCodeFileName;

    int gc_CodeNumberOfLines;
    int gc_CodeNumberOfLinesToPlot; //  for controlled simulation

    //print settings
    int gc_gcodeUnits;
    bool gc_absolutePositioning;
    bool gc_isExtrusionCumulative;//relative (false) or absolute (true)


    int gc_interpolationType;
    double gc_origin[3];

    //print parameters
    double gc_filamentDiameter;
    double gc_nozzleDiameter;
    double gc_layerWidth;
    double gc_layerHeight;

    double gc_feedRate;
    double gc_totalTimeToPrint;

    double gc_bedTemperature;
    double gc_fanSpeed;
    double gc_extruderTemperature;

    //things to keep track of
    double gc_cumulativeExtrusionLength;//keep track of total extrusion
    int *gc_commandType;
    double *gc_xPosition;
    double *gc_yPosition;
    double *gc_zPosition;
    double *gc_commandStartTime;
    double *gc_commandEndTime;
    double *gc_extrudedLength;// non cumulative, i.e., for that length
    double *gc_miscCommands;
    double  gc_cumulativeLengthPrinted;
    double gc_boundingBox[3][2];


    // Multiple printhead extrusion
    struct gc_printHead
    {
        int lineNumber; // Store which gcode line head changes
        int toolNumber; // store which tool is changes to
    };
    std::vector<gc_printHead> gc_printHeadChange;


    bool    countNumberOfLinesInGCode(string gCodeFileName);
    void    allocateGCodeMemory(void);
    int     runGCodeLineNumber(string commandToken[] , int tokenCount ,  int currentLineNumber);
    void    estimateLayerHeight();
public:
    gCodeReader();
    bool		readGCode(string gCodeFileName);
    void        printGCodeInformation(void);
    void        safeFree(void *ptr);
    void        freeGCodeMemory(void);
    //get functions
    void        getGeometryToGCodeScalingFactor(double geomBoundingBox[3][2],double scalingFactor[3]);
    double      getNetFilamentLengthExtruded();
    double      getEndTimeOfLine(int line);
    double      getVolumeOfPrintedObject();
    float		getGCodeTimeToPrint(void);
    double		getGCodeFilamentDiameter(void);
    double      getGCodeLayerHeight(void);
    double      getGCodeLayerWidth(void);
    int         getNumberOfGCodeLayers(void);
    int			getNumberOfGCodeLines(void);
    double      getGCodeNozzleDiameter(void);
    double      getGCodeNetLengthPrinted(void);
    double		getGCodeToSTLScalingFactor(int stlUnits);
    int         getLocationDataOfGCodeLine(int lineNumber, double startPt[3], double endPt[3], double &extrudedLength , int &commandType);
    double      getOrientationOfElemsInGCodeLine(int lineNumber);
    int         getElemsWithGCodeLineNumber(int lineNumber, int *elems);
    int         getGCodeUnits();
    double      getDistanceTravelled(double startPt[3],double endPt[3]); // compute L2 norm dist between start Pt and end Pt
    double      getNormalizedPositionOfElemCenterWithBuildVector(double elemCenter[3], double bldVec[3]);
    bool        getIfOverhanging(double normal[3], double buildDir[3], double ovhangAngle);
    double      computeNetOverhangArea(double buildDir[3], double ovhangAngle);
    int         getNumberOfGCodeLinesToPlot(void);

    //set functions
    void		setGcodeFilamentDiameter(double filamentDia);
    void        setGCodeNozzleDiameter(double nozzleDia);
    void        setGCodeLayerWidth(double layerWidth);
    void        setGCodeLayerHeight(double layerHeight);
    void        setGCodeUnits(int units);
    void        setNumberOfGCodeLinesToPlot(int line);

    void        scaleGCode(double scale);
    void        shiftGCodeToOrigin(void);
    void        gCodeReader::alignGCodeWithSTL(void);
    void        shiftGCodeby(double offset[3]);

    //gcode to STL
    void		createSTLFromBoundingBox(double boundingBox[][2],char fileName[]);
    void		getBoundingBoxFromGCode(double boundingBox[3][2],double center[3]);

    //build plate
    void        getBuildPlateDimension(double buildPlateDimension[3]);
    void        setBuildPlateBoundingBox(double buildPlate[3][2]);
    void        getBuildPlateBoundingBox(double boundingBox[3][2]);
    // multiple print head
    int         getCurrentPrintHeadIndex(int lineNumber);
    int         getPrintHeadAtIndex(int index);
    ~gCodeReader();

};



