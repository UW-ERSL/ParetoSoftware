#include "TopOptViewer.hpp"
#include <QTemporaryDir>
#include <manipulatedFrame.h>
extern Renderer renderer;
extern ParetoApplication topopt;
extern LiveVariables liveVar;
extern Voxelizer vox;
TopOptViewer *thisViewer= NULL;

extern void writeMessageToGUIWindow(const char format[],...);
///////////////////////////////////////////////////////////////////////////////
void print2DText(int x, int y, const char string[], float clr[3] = NULL)
{
    if (thisViewer == NULL) return;

    glDisable(GL_LIGHTING);
    if (clr)
        glColor3f(clr[0],clr[1],clr[2]);
    else
        glColor3f(0.0f,0.0f,0.0f);


    thisViewer->drawText(x, y,QString(string));
    glEnable(GL_LIGHTING);
}
///////////////////////////////////////////////////////////////////////////////
void displayUserMessage(const char string[], float clr[3] = NULL)
{
    if (thisViewer == NULL) return;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    thisViewer->startScreenCoordinatesSystem();
    thisViewer->drawText(200,200,string);
    thisViewer->stopScreenCoordinatesSystem();
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}
///////////////////////////////////////////////////////////////////////////////
TopOptViewer :: TopOptViewer(QWidget *w) : QGLViewer(w)
{
    thisViewer = this;

}
///////////////////////////////////////////////////////////////////////////////
void TopOptViewer::initializeGL()
{
  //  char temp[50];

    /****************************************/
    /*   Initialize GLUT and create window  */
    /****************************************/
    GLfloat light0_ambient[] =  {0.f, 0.f, 0.f, 1.0f};
    GLfloat light0_specular[] =  {1.0f,1.0f, 1.0f, 1.0f};
    GLfloat light0_diffuse[] =  {1.f, 1.0f, 1.0f, 1.0f};
    GLfloat light0_position[] = {10.5f, 10.5f, 10.0f, 0.0f};

    GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
    GLfloat light1_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
    GLfloat light1_position[] = {0.0f, 1.0f,5.0f, 0.0f};

    GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

    camera()->setType(qglviewer::Camera::ORTHOGRAPHIC);
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


    QGLViewer::initializeGL();


    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glClearColor( 1.0f, 1.0f,1.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    setMouseBinding(Qt::ControlModifier, Qt::LeftButton, CAMERA, ZOOM);


    QFont f;

    f.setPointSize(12);
    setFont(f);

    char OpenGLVersion[256];
    int majorVersion,minorVersion;
    strcpy(OpenGLVersion,(char *)glGetString(GL_VERSION));
    sscanf(OpenGLVersion,"%d.%d",&majorVersion,&minorVersion);
    writeMessageToGUIWindow("OpenGL Version %s", OpenGLVersion);
    if ((majorVersion == 1) && (minorVersion < 2)){
          writeWarningToGUIWindow("Your OpenGL version is outdated.\nPareto may not work as expected.\nPlease update your graphics driver.");
    }

}
void TopOptViewer :: draw()
{
    glPushMatrix();
    int isoCounter;
    static double previousModelScale = 1;

    if (liveVar.GEOMETRY_UPLOADED){
        if (liveVar.modelScale != previousModelScale){
            double boundingBox[3][2];
            vox.getBoundingBox(boundingBox);
            setSceneRadius(0.0001);// to overcome a strange bug in qglviewer.
            camera()->showEntireScene();

            qglviewer::Vec pos(0, 0.0, liveVar.modelScale);
            qglviewer::Vec center((boundingBox[0][0]+boundingBox[0][1])/2, (boundingBox[1][0]+boundingBox[1][1])/2, (boundingBox[2][0]+boundingBox[2][1])/2);
            center[0] -= 0.25*(boundingBox[0][1]-boundingBox[0][0]);

            camera()->setSceneCenter(center);
            camera()->setPosition(pos);
            camera()->lookAt(center);

            setSceneRadius(0.35*liveVar.modelScale);
            camera()->showEntireScene();
            previousModelScale = liveVar.modelScale;
            updateGL();
            updateGL();
        }
    }
    switch(liveVar.displayGeom) {
    case DISPLAY_GEOM_NONE:
        renderer.plotNothing();
        break;
    case DISPLAY_GEOM_INITIAL:
        switch (liveVar.displayField){
        case DISPLAY_FIELD_NONE:
            if (liveVar.GEOMETRY_UPLOADED)
                renderer.plotGeometry();
            break;
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_STRESS:
            if (liveVar.STATIC_FEA_SOLVED)
                renderer.plotFieldOverSTL();
            break;
        case DISPLAY_FIELD_MODAL:
                if (liveVar.MODAL_FEA_SOLVED)
                    renderer.plotFieldOverSTL();//renderer.plotEigenModeOverSTL();
                break;
        case DISPLAY_FIELD_TEMPERATURE:
            if (liveVar.THERMAL_FEA_SOLVED)
                renderer.plotFieldOverSTL();
            break;
        }
        break;
    case DISPLAY_GEOM_FINAL:
        if (liveVar.GEOMETRY_UPLOADED)
            renderer.plotGeometry();
        break;

    case DISPLAY_GEOM_MESH:     
            if (!liveVar.MESH_APPLIED)
                return;
            switch (liveVar.displayField){
            case DISPLAY_FIELD_NONE:
                if (liveVar.GEOMETRY_UPLOADED)
                    renderer.plotMeshSurface();
                break;
            case DISPLAY_FIELD_DISPLACEMENT:
            case DISPLAY_FIELD_STRESS:
                if (liveVar.STATIC_FEA_SOLVED)
                    renderer.plotMeshSurface();
                break;
            case DISPLAY_FIELD_MODAL:
                    if (liveVar.MODAL_FEA_SOLVED)
                        renderer.plotFieldOverMesh();
                    break;
            case DISPLAY_FIELD_TEMPERATURE:
                if (liveVar.THERMAL_FEA_SOLVED)
                    renderer.plotFieldOverMesh();
                break;
            case DISPLAY_FIELD_SIMP_DENSITY:
                renderer.plotMeshWithPseudoDensity();
                break;
            case DISPLAY_FIELD_TOPOLOGICAL_SENSITIVITY:
                renderer.plotMeshWithTopologicalSensitivity();
                break;
            }
    case DISPLAY_GEOM_MESHCOMPONENTS:
            if (liveVar.MESH_APPLIED)
                renderer.plotMeshComponents();
            break;
    case DISPLAY_GEOM_LATTICE:
            renderer.plotLatticeStructure();
        break;
    case DISPLAY_GEOM_CONTOURS:
            renderer.plotContoursFromAutoDesign();
        break;
    case DISPLAY_GEOM_TOPOLOGY:
        if (liveVar.useSIMPMethod){
            renderer.plotTempGeom(false);
            break;
        }
        isoCounter = topopt.getClosestCounterAtVolFraction(liveVar.displayVolFraction);
        if (isoCounter <0) break;
        if (!liveVar.TOPOPT_SOLVED) break;

        if (topopt.getNumIsoTriangles(isoCounter) <= 0)
            break;
        switch (liveVar.displayField){
        case DISPLAY_FIELD_NONE:
            renderer.plotFinalSTL();
            break;
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_STRESS:
            if (liveVar.TOPOPT_SOLVED)
                renderer.plotFieldOverFinalSTL();
            break;
        case DISPLAY_FIELD_MODAL:
            if (liveVar.MODAL_FEA_SOLVED)
                renderer.plotFieldOverFinalSTL();//plotEigenModeOverFinalSTL
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            if (liveVar.THERMAL_FEA_SOLVED){
                renderer.plotFieldOverFinalSTL();
            }
            break;
        }
        break;
    case DISPLAY_GEOM_OTHER:
        renderer.plotTempGeom(liveVar.displayEdges);
        break;
    }
    glPopMatrix();

}
void TopOptViewer::mouseDoubleClickEvent(QMouseEvent* e)
{
    if((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::NoModifier)){
        QMessageBox msgBox;

        msgBox.setText("Mouse and keyboard behavior\n\n"
                       "To select surface: Use left button \n"
                       "To unselect: Use right button\n\n"

                       "To rotate: Use left button\n"
                       "To zoom: Use middle button, or left button + control key\n"
                       "To pan: Use right button\n\n"

                       "To reset view: Double click right button.");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        msgBox.exec();
    }
    if((e->button() == Qt::RightButton)){
         camera()->showEntireScene();
    }
    QGLViewer::mousePressEvent(e);// pass on to qgl


}

void TopOptViewer::mousePressEvent(QMouseEvent* e)
{
    qglviewer::Vec point,cameraPos,dir,orig;
    bool found;
    double pt[3],ptClosest[3],normal[3],totalArea,origin[3],direction[3],tParam;
    int triangle ;
    int depth = 0;

    if((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::NoModifier)){
        camera()->convertClickToLine(e->pos(),orig,dir);
        origin[0] = orig.x;
        origin[1] = orig.y;
        origin[2] = orig.z;
        direction[0] = dir.x;
        direction[1] = dir.y;
        direction[2] = dir.z;

        triangle = vox.findClosestTriangleIntesectingRaySkippingParts(liveVar.partsToHide,origin,direction,&tParam);
        if (triangle >= 0) {
            int triGroup;
            if (vox.isTriangleHighlighted(triangle)){//already highlighted
                //vox.resetHighlight();
                //liveVar.nFacesSelected = 0;
                return; //nothing to do
            }
            liveVar.selectedPart = vox.getParentPartOfTriangle(triangle);
            triGroup = vox.getTriGroup(triangle);
            if (triGroup > 0){// already in a group
                vox.highlightTriangleGroup(triGroup,&totalArea);
            }
            else{
                vox.getNormalOfTriangle(triangle,normal);
                totalArea = 0;
                vox.highlightTrianglesRecursive(triangle, depth, normal, 1.0*liveVar.cutoffAngle,&totalArea);
            }
            liveVar.nFacesSelected++;
        }
    }
    if((e->button() == Qt::RightButton) && (e->modifiers() == Qt::NoModifier) ){
        camera()->convertClickToLine(e->pos(),orig,dir);
        origin[0] = orig.x;
        origin[1] = orig.y;
        origin[2] = orig.z;

        direction[0] = dir.x;
        direction[1] = dir.y;
        direction[2] = dir.z;

        triangle = vox.findClosestTriangleIntesectingRay(origin,direction,&tParam);
        if (triangle >= 0) {
            if (vox.isTriangleHighlighted(triangle)){// unselect these faces
                vox.getNormalOfTriangle(triangle,normal);
                totalArea = 0;
                vox.unHighlightTrianglesRecursive(triangle, depth, normal, 1.0*liveVar.cutoffAngle,&totalArea);
            }
        }
    }
    QGLViewer::mousePressEvent(e);// pass on to qgl
}



