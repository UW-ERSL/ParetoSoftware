#include "TemplateViewer.hpp"
#include "TemplateRenderer.hpp"
#include <QTemporaryDir>
#include <manipulatedFrame.h>
#include "gCodeReader.hpp"
extern Renderer renderer;
extern TemplateRenderer  templateRenderer;// additional renderer
extern ParetoApplication topopt;
extern LiveVariables liveVar;
extern Voxelizer vox;
TemplateViewer *thisViewer= NULL;
extern gCodeReader             gCode;
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
TemplateViewer :: TemplateViewer(QWidget *w) : QGLViewer(w)
{
    thisViewer = this;
    setAxisIsDrawn(true);
    setGridIsDrawn(true);
}
///////////////////////////////////////////////////////////////////////////////
void TemplateViewer::initializeGL()
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

    f.setPointSize(14);
    setFont(f);

    char OpenGLVersion[256];
    int majorVersion,minorVersion;
    strcpy(OpenGLVersion,(char *)glGetString(GL_VERSION));
    sscanf(OpenGLVersion,"%d.%d",&majorVersion,&minorVersion);
    writeMessageToLog("OpenGL Version %s", OpenGLVersion);
    if ((majorVersion == 1) && (minorVersion < 2)){
          writeWarningToLog("Your OpenGL version is outdated.\nApplication may not work as expected.\nPlease update your graphics driver.");
    }

    QMessageBox msgBox;

    msgBox.setText("Keyboard\n"
                   "g: Hide/display grid \n"
                   "a: Hide/display axis \n\n"
                   "Mouse\n"
                   "Left: rotate\n"
                   "Middle: zoom\n"
                   "Right: pan\n"
                   "Double-left: XY view.\n"
                   "Double-right: Fit object.\n");
    msgBox.addButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowModality(Qt::NonModal);
    msgBox.show();
}
void TemplateViewer :: draw()
{
    glPushMatrix();
    double startPt[3],endPt[3];
    static double previousModelScale = 1;

    startPt[0] = 0.0;
    startPt[1] = 0.0;
    startPt[2] = 0.0;

    endPt[0] = 0.0;
    endPt[1] = 0.0;
    endPt[2] = 1.0*liveVar.modelScale;
    if (liveVar.GEOMETRY_UPLOADED){
        // If the geometry has changed, reset view
        if ((liveVar.modelScale > 2*previousModelScale) ||
         (liveVar.modelScale < 0.5*previousModelScale)) {
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
            if (liveVar.GEOMETRY_UPLOADED){
                renderer.plotGeometry();
            }
            break;
        case DISPLAY_FIELD_DISPLACEMENT:
        case DISPLAY_FIELD_DISPLACEMENT_X:
        case DISPLAY_FIELD_DISPLACEMENT_Y:
        case DISPLAY_FIELD_DISPLACEMENT_Z:
        case DISPLAY_FIELD_STRESS:
            if (liveVar.STATIC_FEA_SOLVED)
                renderer.plotFieldOverSTL();
            break;
        case DISPLAY_FIELD_TEMPERATURE:
            if (liveVar.THERMAL_FEA_SOLVED)
                renderer.plotTemperatureOverMesh();
            break;
        }
        break;
    case DISPLAY_GEOM_MESH:
        switch (liveVar.displayField){
            case DISPLAY_FIELD_NONE:
                if (liveVar.MESH_APPLIED)
                    renderer.plotMeshSurface();
                break;
            case DISPLAY_FIELD_TEMPERATURE:
            case DISPLAY_FIELD_MOISTURE:
                if (liveVar.THERMAL_FEA_SOLVED)
                    templateRenderer.plotMoistureOverMeshSurface();
                break;
            case DISPLAY_FIELD_DISPLACEMENT:
                if (liveVar.STATIC_FEA_SOLVED)
                    templateRenderer.plotDisplacementOverMeshSurface();
                break;

        }
        break;
    case DISPLAY_GEOM_GCODE:
            templateRenderer.drawGCode();
        break;
    case DISPLAY_GEOM_GCODE_FILAMENT:
            templateRenderer.drawGCodeFilament();
            break;
    }
    glPopMatrix();

}
void TemplateViewer::mouseDoubleClickEvent(QMouseEvent* e)
{

    if((e->button() == Qt::RightButton)){
         camera()->showEntireScene();
    }
    if((e->button() == Qt::LeftButton)){
         qglviewer::Vec dirVec;
         dirVec.x = 0;
         dirVec.y = 0;
         dirVec.z = -1;
         camera()->setViewDirection(dirVec);
         camera()->showEntireScene();
    }
    QGLViewer::mousePressEvent(e);// pass on to qgl
}

void TemplateViewer::mousePressEvent(QMouseEvent* e)
{
    qglviewer::Vec dir,orig;
    double normal[3],totalArea,origin[3],direction[3],tParam;
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

        triangle = vox.findClosestTriangleIntesectingRay(origin,direction,&tParam);
        if (triangle >= 0) {
            if (vox.isTriangleHighlighted(triangle)){//already highlighted
                //vox.resetHighlight();
                //liveVar.nFacesSelected = 0;
                return; //nothing to do
            }
            liveVar.selectedPart = vox.getParentPartOfTriangle(triangle);
            vox.getNormalOfTriangle(triangle,normal);
            totalArea = 0;
            vox.highlightTrianglesRecursive(triangle, depth, normal, 1.0*liveVar.cutoffAngle,&totalArea);
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



