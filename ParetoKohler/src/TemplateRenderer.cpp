#include "TemplateRenderer.hpp"
#include "../ParetoTopOpt/src/Renderer.cpp"
#include "gCodeReader.hpp"
extern gCodeReader             gCode;
TemplateRenderer::TemplateRenderer()
{
    ;
}

void TemplateRenderer::drawGCode(void)
{
    double startPt[3],endPt[3],extrudedLength;
    int commandType,gCodeUnits;
    char str[256];
    int  startX, startY,deltaY;
    double layerHt,time2Print;

    int nLines = gCode.getNumberOfGCodeLines();
    layerHt = gCode.getGCodeLayerHeight();
    gCodeUnits = gCode.getGCodeUnits();
    time2Print = gCode.getGCodeTimeToPrint()/60;


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

    sprintf(str,"GCodeLines: %d ",nLines);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"Layer height: %.2f mm",1000*layerHt);
    print2DText(startX, startY,str);
    startY += deltaY;

    if (time2Print < 60)
        sprintf(str,"Time to print: %.1f (min)",time2Print);
    else{
        int hr;
        double mins;
        hr = time2Print/60;
        mins = time2Print - 60*hr;
        sprintf(str,"Time to print: %d (hr) %.0f (min)",hr,mins);
    }
    print2DText(startX, startY,str);
    startY += deltaY;

    liveVar.displayGeom = DISPLAY_GEOM_GCODE;
    glLineWidth(2.0);
    if (liveVar.plotBoundingBox) plotBoundingBox();
    //color[0] = 255.0/255;color[1] = 204.0/255.0;color[2] = 153./255;
    //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    for(int line = 0; line < gCode.getNumberOfGCodeLinesToPlot();line++){
        gCode.getLocationDataOfGCodeLine(line, startPt,endPt, extrudedLength ,commandType);
        if(commandType != 101) continue;//only G01 commands result in movement

        glBegin(GL_LINES);
            glVertex3d(startPt[0],startPt[1],startPt[2]);
            glVertex3d(endPt[0],endPt[1],endPt[2]);
        glEnd();
    }
    int finalLine = gCode.getNumberOfGCodeLinesToPlot();
    time2Print = gCode.getEndTimeOfLine(finalLine)/60;
    if (time2Print < 60)
        sprintf(str,"line: %d, time: %.2f (min)",finalLine,time2Print);
    else{
        int hr;
        double mins;
        hr = time2Print/60;
        mins = time2Print - 60*hr;
        sprintf(str,"line: %d, time: %d (hr) %.0f (min)",finalLine,hr,mins);
    }
    print2DText(startX, startY,str);

    if (liveVar.plotAxis) drawAxis();
    if (liveVar.geomTransparent) {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        plotSTLGeom(liveVar.displayEdges);
        glDisable( GL_BLEND );
        if (liveVar.displayGeom == DISPLAY_GEOM_FINAL)
            plotTempGeom(liveVar.displayEdges);
    }
}
void TemplateRenderer::drawGCodeFilament(void)
{
    double startPt[3],endPt[3],extrudedLength,radius;
    int commandType,gCodeUnits;
    char str[256];
    int  startX, startY,deltaY;
    double layerHt,time2Print,dir[3],feederHeight,feederRadius,feederTip[3],layerWidth;
    float color[4];

    int nLines = gCode.getNumberOfGCodeLines();
    layerHt = gCode.getGCodeLayerHeight();
    gCodeUnits = gCode.getGCodeUnits();
    time2Print = gCode.getGCodeTimeToPrint()/60;


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

    sprintf(str,"GCodeLines: %d ",nLines);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"Layer height: %.2f mm",1000*layerHt);
    print2DText(startX, startY,str);
    startY += deltaY;

    if (time2Print < 60)
        sprintf(str,"Time to print: %.1f (min)",time2Print);
    else{
        int hr;
        double mins;
        hr = time2Print/60;
        mins = time2Print - 60*hr;
        sprintf(str,"Time to print: %d (hr) %.0f (min)",hr,mins);
    }
    print2DText(startX, startY,str);
    startY += deltaY;

    liveVar.displayGeom = DISPLAY_GEOM_GCODE_FILAMENT;
    glLineWidth(2.0);
    if (liveVar.plotBoundingBox) plotBoundingBox();
    color[0] = 255.0/255;color[1] = 204.0/255.0;color[2] = 153./255;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);

    radius = gCode.getGCodeLayerHeight()/2;

    for(int line = 0; line < gCode.getNumberOfGCodeLinesToPlot();line++){
        gCode.getLocationDataOfGCodeLine(line, startPt,endPt, extrudedLength ,commandType);
        if(commandType != 101) continue;//only G01 commands result in movement
        for (int i = 0; i < 3; i++){
            startPt[i] = startPt[i] - 1.25*radius*(endPt[i]-startPt[i])/extrudedLength; // extend slightly to close gaps
            endPt[i] = endPt[i] + 1.25*radius*(endPt[i]-startPt[i])/extrudedLength; // extend slightly to close gaps
        }
        drawCylinder(startPt,endPt,radius,1 );
    }
    feederHeight = 10*radius;
    feederRadius = 5*radius;
    dir[0] =0; dir[1] =0; dir[2] = 1.0;
    for (int i = 0; i< 3; i++)
        feederTip[i] = endPt[i] +feederHeight*dir[i];
    //drawNiceCone(feederTip,dir,feederHeight,feederRadius);
    int finalLine = gCode.getNumberOfGCodeLinesToPlot();
    time2Print = gCode.getEndTimeOfLine(finalLine)/60;
    if (time2Print < 60)
        sprintf(str,"line: %d, time: %.2f (min)",finalLine,time2Print);
    else{
        int hr;
        double mins;
        hr = time2Print/60;
        mins = time2Print - 60*hr;
        sprintf(str,"line: %d, time: %d (hr) %.0f (min)",finalLine,hr,mins);
    }
    print2DText(startX, startY,str);

    if (liveVar.plotAxis) drawAxis();
    if (liveVar.geomTransparent) {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        plotSTLGeom(liveVar.displayEdges);
        glDisable( GL_BLEND );
        if (liveVar.displayGeom == DISPLAY_GEOM_FINAL)
            plotTempGeom(liveVar.displayEdges);
    }
}
void TemplateRenderer::plotMoistureOverMeshSurface(void)
{
    int elem,nodes[8],neighboringElemsOfElem[27],neighboringElem;
    double x[8],y[8],z[8],fieldMin,fieldMax,*fieldToPlot;
    double p0[3],p1[3],p2[3],triNormal[3],p01[3],p02[3];
    int quads[6][4] = {{0,4,7,3},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{4,5,6,7}},maxFieldNode;
    GLfloat color[4] ={0.1,0.95, 0.1, 1.0};
    bool neighboringElemsExists[7];
    char title[256];
    double temp,temperature[8];
    int deltaY,startX,startY;
    double boundingBox[3][2],XCuttingPlane,YCuttingPlane,ZCuttingPlane;
    double elemCenter[3],neighborCenter[3];

    vox.getBoundingBox(boundingBox);
    XCuttingPlane = boundingBox[0][0] + liveVar.xCuttingPlanePercent*(boundingBox[0][1] - boundingBox[0][0])/100.0;
    YCuttingPlane = boundingBox[1][0] + liveVar.yCuttingPlanePercent*(boundingBox[1][1] - boundingBox[1][0])/100.0;
    ZCuttingPlane = boundingBox[2][0] + liveVar.zCuttingPlanePercent*(boundingBox[2][1] - boundingBox[2][0])/100.0;


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;
    print2DText(startX, startY,liveVar.graphicsText.c_str());

    getStaticFEATitle(&fieldMin,&fieldMax,title,&maxFieldNode);
    sprintf(title,"Moisture(%)");
    if (liveVar.plotAxis) drawAxis();
    glLineWidth(1.5);
    glPointSize(5.0);
    glDisable( GL_LIGHTING );
    for (elem = 0; elem < topopt.getNumElems(); elem++) {
        if  (topopt.getElemPseudoDensity(elem) <1e-6) continue;
        topopt.getElemCenter(elem,elemCenter);
        if (elemCenter[0] < XCuttingPlane)
            continue;
        if (elemCenter[1] < YCuttingPlane)
            continue;
        if (elemCenter[2] < ZCuttingPlane)
            continue;

        topopt.getAllNeighboringElementsOfElem(elem,neighboringElemsOfElem);
        // 0 is self, 1 is x-, 2 is y-,3 is z-, 4 is x+,5 is y+, 6 is z+
        neighboringElemsExists[0] = true;//self
        for (int m = 1; m < 7; m++) {// start with 1 for neighbors
            neighboringElemsExists[m] = false;
            neighboringElem = neighboringElemsOfElem[m];
            if (neighboringElem == -1) continue;
            if (topopt.getElemPseudoDensity(neighboringElem) <1e-6) continue;
            topopt.getElemCenter(neighboringElem,neighborCenter);
            if (neighborCenter[0] < XCuttingPlane)
                continue;
            if (neighborCenter[1] < YCuttingPlane)
                continue;
            if (neighborCenter[2] < ZCuttingPlane)
                continue;
            neighboringElemsExists[m] = true;
        }
        topopt.getElemNodes(elem,nodes);
        topopt.getElemNodeLocation(elem,x,y,z);

        topopt.getElemNodesTemperature(elem,temperature);
        fieldToPlot = &(temperature[0]);
        for (int m = 0; m < 6;m++) {// consider the six quads
            // make sure all 4 nodes are on boundary
            if (neighboringElemsExists[m+1]) continue;//offset by 1, don't plot boundary if neighbor exists

            // use 3 pts to find the normal
            p0[0] = x[quads[m][0]];
            p0[1] = y[quads[m][0]];
            p0[2] = z[quads[m][0]];

            p1[0] = x[quads[m][1]];
            p1[1] = y[quads[m][1]];
            p1[2] = z[quads[m][1]];

            p2[0] = x[quads[m][2]];
            p2[1] = y[quads[m][2]];
            p2[2] = z[quads[m][2]];

            for (int i = 0; i < 3; i++) {
                p01[i] = p1[i] - p0[i];
                p02[i] = p2[i] - p0[i];
            }
            CROSS3D(triNormal,p01,p02);

            glBegin(GL_QUADS);
            glNormal3f(triNormal[0],triNormal[1],triNormal[2]);
            temp = 1-(fieldToPlot[quads[m][0]]-fieldMin)/(fieldMax-fieldMin);


            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][0]],y[quads[m][0]],z[quads[m][0]]);

            temp = 1-(fieldToPlot[quads[m][1]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][1]],y[quads[m][1]],z[quads[m][1]]);

            temp = 1-(fieldToPlot[quads[m][2]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glVertex3f(x[quads[m][2]],y[quads[m][2]],z[quads[m][2]]);

            temp = 1-(fieldToPlot[quads[m][3]]-fieldMin)/(fieldMax-fieldMin);
            getInterpolatedColor(temp, color);
            glVertex3f(x[quads[m][3]],y[quads[m][3]],z[quads[m][3]]);
            glEnd();
            glEnable( GL_LIGHTING );
        }
    }
    plotColorBars();
    if (liveVar.geomTransparent) plotGeometry();
    glDisable( GL_LIGHTING );

}
TemplateRenderer::~TemplateRenderer()
{
    ;
}

