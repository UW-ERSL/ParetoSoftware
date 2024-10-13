#include "TriangulateWires.hpp"
#include "LatticeRenderer.hpp"
#include "../ParetoTopOpt/src/Renderer.cpp"

extern TriangulateWires triwires;
/**********************************************************************/
TemplateRenderer::TemplateRenderer()
{
    ;
}
/**********************************************************************/
void TemplateRenderer::drawSomething(void)
{
    ;
}
/**********************************************************************/
void TemplateRenderer::plotLatticeGraphData(void)
{
    int pareto_viewport[4], startX, startY,deltaY,nNodes,nStruts;
    char str[256];

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    sprintf(str,"Model: %s ",liveVar.modelName.c_str());
    print2DText(startX, startY,str);
    startY += deltaY;

    nStruts = triwires.getNumberOfBeams();
    nNodes = triwires.getNumberOfNodes();

    sprintf(str,"#Nodes: %d ",nNodes);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#Beams: %d ",nStruts);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#minBeamLength: %.2g ",triwires.getMinEdgeLength());
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#maxBeamLength: %.2g ",triwires.getMaxEdgeLength());
    print2DText(startX, startY,str);
    startY += deltaY;

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();

}
/**********************************************************************/
void TemplateRenderer::plotSolidLatticeData(void)
{
    int pareto_viewport[4], startX, startY,deltaY,nNodes,nStruts;
    char str[256];

    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity();

    glGetIntegerv (GL_VIEWPORT, pareto_viewport);
    gluOrtho2D (0,pareto_viewport[2], 0, pareto_viewport[3]);
    glDepthFunc (GL_ALWAYS);


    deltaY = 30;// font size dependent
    startX = 20;
    startY = deltaY;

#ifdef PARETOWORKS
    startX += 300;
    startY += 80;
    deltaY = 20;
#endif

    sprintf(str,"Model: %s ",liveVar.modelName.c_str());
    print2DText(startX, startY,str);
    startY += deltaY;

    nStruts = triwires.getNumberOfBeams();
    nNodes = triwires.getNumberOfNodes();

    sprintf(str,"#Nodes: %d ",nNodes);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#Beams: %d ",nStruts);
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#minBeamLength: %.2g ",triwires.getMinEdgeLength());
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#maxBeamLength: %.2g ",triwires.getMaxEdgeLength());
    print2DText(startX, startY,str);
    startY += deltaY;

    sprintf(str,"#Triangles: %d ",triwires.getNumberOfTriangles());
    print2DText(startX, startY,str);
    startY += deltaY;

    glDepthFunc (GL_LESS);
    glPopMatrix ();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix ();

}
/**********************************************************************/
void TemplateRenderer::drawNiceArrow(double startPt[3],double endPt[3])
{
    double vec[3],length,baseRadius, dir[3],angle,lengthOfArrow,sinTheta;
    SUBTRACT3D(vec,startPt,endPt);
    length = NORM3D(vec);
    lengthOfArrow = length;
    baseRadius = 0.05*MIN(length, 0.1*triwires.getModelScale());

    // we need to align the desired direction with the z axis
    //i.e., we need to find the rotation axis and the angle of rotation to align
    // rotation axis is perpendicular to direction and z axis (Cross product)
    dir[0] = vec[1]; dir[1] = -vec[0]; dir[2] = 0; // to rotate frame
    sinTheta = NORM3D(dir)/length;

    // angle is the dot product of dir and z axis
    //angle = (180.0/3.1415)*asin(nrm/length);
    angle = (180.0/3.1415)*asin(sinTheta);
    if (vec[2] > 0) angle = 180-angle;
    GLUquadric* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    glPushMatrix();
    glTranslatef(startPt[0], startPt[1], startPt[2]);


    glRotatef(angle, dir[0], dir[1], dir[2]);// Draw the cylinder part of the arrow
    gluCylinder(q, baseRadius, baseRadius, lengthOfArrow, 10, 1);
    gluQuadricOrientation(q, GLU_INSIDE);// Normals for the cap need to be flipped
    gluDisk(q, 0,baseRadius, 10, 1);// Cap the bottom

    glTranslatef(0,0,lengthOfArrow);// Draw the cylinder part of the arrow
    gluDisk(q, 0,3*baseRadius, 10, 1);// Cap the bottom of the cone2
    gluQuadricOrientation(q, GLU_OUTSIDE);// Normals for the cap need to be flipped
    gluCylinder(q, 3*baseRadius, 0.0f, 5*baseRadius, 10, 1);// Draw the cone part of the arrow

    glPopMatrix();
    gluDeleteQuadric(q);
}
/************************************************************/
void TemplateRenderer::draw3DText(const char *text,double pos[3],double scale)
{
    float scale2 = scale*0.03*triwires.getModelScale();

    glPushMatrix();
    glTranslated(pos[0],pos[1],pos[2]);
    glScalef(scale2, scale2, scale2);
    //t3dDraw3D("text", -1, 0, 0.2f);// some bug, crashes
    glPopMatrix();

}
/******************************************************************************/
void TemplateRenderer::drawAxis(void)
{
    double xyzMin[3],xyzMax[3],scale;

    scale =triwires.getModelScale();

    double AxisLength = 0.07*scale;
    double startPt[]= {0,0,0},endPt[] ={0,0,0};
    float color[3];
    //glEnable(GL_LIGHTING);
    triwires.getBoundingBox(xyzMin,xyzMax);

    startPt[0] = xyzMin[0]-0.025*scale;
    startPt[1] = xyzMin[1]-0.025*scale;
    startPt[2] = xyzMin[2]-0.025*scale;

    endPt[0] =startPt[0]+ AxisLength;endPt[1] = startPt[1];endPt[2] = startPt[2];
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt);

    endPt[0] =startPt[0];endPt[1] = startPt[1]+AxisLength;endPt[2] = startPt[2];
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt);

    endPt[0] =startPt[0];endPt[1] = startPt[1];endPt[2] = startPt[2]+AxisLength;
    color[0] = 0.25;color[1] = 0.25;color[2] = 0.25;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    drawNiceArrow(startPt,endPt);

    glColor3f(color[0], color[1], color[2]);
    endPt[0] =startPt[0]+ 1.5*AxisLength;endPt[1] = startPt[1];endPt[2] = startPt[2];
    draw3DText("X",endPt,1);
    endPt[0] =startPt[0];endPt[1] = startPt[1]+ 1.5*AxisLength;endPt[2] = startPt[2];
    draw3DText("Y",endPt,1);
    endPt[0] =startPt[0];endPt[1] = startPt[1];endPt[2] = startPt[2]+ 1.5*AxisLength;
    draw3DText("Z",endPt,1);
    //glDisable(GL_LIGHTING);
}

/**********************************************************************/
void TemplateRenderer::plotLatticeGraph(void)
{
    int nStruts,nNodes,nodes[2];
    double pt0[3],pt1[3];
    nStruts = triwires.getNumberOfBeams();
    nNodes = triwires.getNumberOfNodes();

    liveVar.modelScale = triwires.getModelScale();
    if (liveVar.plotAxis) drawAxis();
    if (nStruts == 0) return;

    glDisable(GL_LIGHTING);

    glLineWidth(2.0);
    glColor3f(0.34,0.12,0.12);

    for (int strut = 0; strut < nStruts; strut++){
        triwires.getBeamNodes(strut,nodes);
        triwires.getNodeLocation(nodes[0],pt0);
        triwires.getNodeLocation(nodes[1],pt1);

        glBegin(GL_LINES);
            glVertex3d(pt0[0],pt0[1],pt0[2]);
            glVertex3d(pt1[0],pt1[1],pt1[2]);
        glEnd();
    }

    glEnable(GL_LIGHTING);

    plotLatticeGraphData();
}
void TemplateRenderer::plotSolidLattice(void)
{
    double xTri[3],yTri[3],zTri[3],triNormal[3],p01[3],p02[3];
    GLfloat color[4] ={0,0, 0, 0.25};

    if (liveVar.plotAxis) drawAxis();
    glPushMatrix();
    glLineWidth(2.0);

    for (int tri = 0; tri < triwires.getNumberOfTriangles(); tri++) {
        triwires.getTriPoints(tri,xTri,yTri,zTri);
        p01[0] = xTri[1]-xTri[0];
        p01[1] = yTri[1]-yTri[0];
        p01[2] = zTri[1]-zTri[0];

        p02[0] = xTri[2]-xTri[0];
        p02[1] = yTri[2]-yTri[0];
        p02[2] = zTri[2]-zTri[0];
        CROSS3D(triNormal,p01,p02);

        getColorOfPart(0,color);// use group ID to color

        glDisable(GL_COLOR_MATERIAL);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
        glNormal3f(triNormal[0],triNormal[1],triNormal[2]);

        glPolygonMode(GL_FRONT, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex3f(xTri[0],yTri[0],zTri[0]);
        glVertex3f(xTri[1],yTri[1],zTri[1]);
        glVertex3f(xTri[2],yTri[2],zTri[2]);
        glEnd();


        glPolygonMode(GL_FRONT, GL_FILL);
    }
    glEnable(GL_COLOR_MATERIAL);
    glDisable( GL_BLEND );
    glLineWidth(2.0);
    glColor3f(0,0,0);
    glDisable(GL_COLOR_MATERIAL);
    glPopMatrix();
}

TemplateRenderer::~TemplateRenderer()
{
    ;
}

