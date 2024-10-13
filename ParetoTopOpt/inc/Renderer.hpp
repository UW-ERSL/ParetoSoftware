#ifndef RENDERER_H
#define RENDERER_H

#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "LiveVariables.hpp"
#include "text3D.hpp"

#ifdef USE_QT
#include <qglviewer.h>
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif

class Renderer {

public:
    void        plotNothing(void);
    void        plotGeometry(bool plotGeom = true);
    void        plotDisplacementOverMeshSurface(void);
    void        plotTemperatureOverMeshSurface(void);
    void        plotMeshSurface(void);
    void        plotMesh(void);
    void        plotMeshComponents(void);
    void        plotMeshWithPseudoDensity(void);
    void        plotMeshWithTopologicalSensitivity(void);
    void        plotStressOnMesh(void);
    void        plotTemperatureOverMesh(void);
    void        plotFieldOverSTL(void);
    void        plotFieldOverMesh(void);
    void        plotWorkSpacePatch(void);
    void        plotStructuralAnalysisResults(void);
    void        plotThermalAnalysisResults(void);
    void        plotFinalSTL(void);
    void        plotFieldOverFinalSTL(void);
    void        plotMeshSurfacePseudoDensity(void);
    void        plotEigenModeOverSTL(void);
    void        plotEigenModeOverFinalSTL(void);
    void        plotContoursFromAutoDesign(void);
    void        selectSTLGeomTriangles(void);
    void        plotLatticeStructure(void);
    void        plotBooleanFieldOverSTLMesh(bool* fieldVariable, bool displayEdges = true);
    void        plotXYMultiple(vector<double> xData, vector<vector<double>> yData,vector <char *> yLegend, bool smooth = false,
                              const char *xTitle="",const char *yTitle="",const char *chartTitle = "",int width = 300,int height = 300);

    char        userDefinedTitle[1024];

    int         Getdec(char hexchar);
    void        HexColorToFloat(const char* HexColor, float* FloatColor);
    void        getColorOfPart(int part,GLfloat color[4]);
    void        getColorOfIsosurface(int isosurfaceId,GLfloat color[4]);
    void        getInterpolatedColor(double value, float color[3]);
    void        drawAxis(void);
    void        plotBoundingBox();
    void        plotTempGeom(bool displayEdges);// geometry stored in voxTemp
    void        plotSTLGeom(bool displayEdges);
    void        drawNiceCone(double startPt[3], double dir[3], double height , double baseRadius);
    void        drawCylinder(double startPt[3], double endPt[3], double radius, double aspectRatio = 1);
    void        drawHollowCylinder(double startPt[3], double endPt[3]);
    void        drawLine(double startPt[3],double endPt[3], float lineWidth = 1.0);
    void        drawNiceArrow(double startPt[3],double endPt[3], bool withBaseCap, bool doubleArrow,double baseRadiusScale = 0.05 );
    void        draw3DText(const char *text,double pos[3],double scale = 1);
    void        drawNormals(void);
    void        plotElasticityBC(void);
    void        plotThermalBC(void);
    void        drawNiceSphere(double centerPt[],double radius );
    void        plotTopOptConstraints();
    void        plotCyclicSymmetryPlanes(int zCyclicSymmetry);
    void        plotBuildPlatform(int buildDirection);
    void        plotSymmetryPlanes(bool xSym,bool ySym, bool zSym);
    void        plotDrawDirection(int drawDirection);
    void        plotThroughCutDirection(int throughCutDirection);
    void        plotGridPattern(int xGrid,int yGrid,int zGrid);
    void        plotGeomData(void);
    void        plotMeshData(void);
    void        plotMeshComponentData(void);
	void        displayStatusMessage(void);
    void        plotColorBars(void);
    void        getStaticFEATitle(double *fMin,double *fMax,char title[],int *maxFieldNode);
    void        plotFieldOverTriangle(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],double vonmises[3],double temperature[3],double dist[3],
                               double scale,double triNormal[3],double fieldMin,double fieldMax);
    void        plotNonDesignSTLGeom(bool displayEdges);
    void        plotModalResults(void);

    void        plotSIMPResults(void);
    void        plotTopOptResults(void);
    void        plotLatticeResults(void);
    void        plotFieldOverNonDesignSTL(void);
    void        plotFieldOverTriangleRecursive(double x[3],double y[3],double z[3],double scale,double u[3],double v[3],double w[3],double vonmises[3],double t[3],double d[3],
                            double triNormal[3],double fieldMin,double fieldMax, double edgeLengthDesired,int currentDepth);
    void        plotFieldOverTriangleSmooth(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],double vonmises[3],
                                            double temperature[3],double dist[3],
                                            double scale,double triNormal[3],double vertexNormals0[3],double vertexNormals1[3],double vertexNormals2[3],double fieldMin,double fieldMax);
    void        plotSupportStructuresInitialGeometry(int direction);
    void        plotSupportStructuresFinalTopology(int direction);
    void        plotEigenModeOverTriangleRecursive(double x[3],double y[3],double z[3],double scale,double u[3],double v[3],double w[3],
                                                double triNormal[3],double fieldMin,double fieldMax, double edgeLengthDesired,int currentDepth);
    void        plotEigenModeOverTriangle(double x[3], double y[3], double z[3],double u[3],double v[3],double w[3],
                                            double scale,double triNormal[3],double fieldMin,double fieldMax);
};

#endif // RENDERER_H
