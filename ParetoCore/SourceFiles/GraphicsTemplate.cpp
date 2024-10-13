#pragma warning(disable : 4996)
#pragma warning(disable : 4005)

#include <string.h>
#include <process.h>
#include <stdio.h>
#include <GL/glut.h>
#include <windows.h>


#define ROTATE 1
#define TRANSLATE 2
#define ZOOM 3

float GraphicsModelSize;
void (*userDisplayFunction)(void);
char userDefinedTitle[1000];
static float xy_aspect;
static int main_window = 0;
static float view_rotate[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
static float obj_pos[] = { 0.0, 0.0, 0.0 };
GLint viewport [4];

static GLfloat GLtrans[3] = {0,0,0};
static GLfloat GLrot[2] = {20,0};
static GLfloat GLLengthScale = 1.0;
static GLint curX=0,curY =0;
static int GLState = 0;
void myGlutDisplay(void);


/************************************************************/
void minimizeGTGracefully(int control)
{
	glutSetWindow(main_window);
	glutIconifyWindow();
}

/************************************************************/
void closeGTGracefully(int control)
{
	glutSetWindow(main_window);
	main_window = 0;
	_endthread();
}
/************************************************************/
void printText(int x, int y, const char *string)
{	
	glDisable( GL_LIGHTING );
	glColor3f(0,0,0);
	glRasterPos2f(x, y);
	for (int i = 0; string[i] != '\0'; ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string[i]);

	glEnable( GL_LIGHTING );
}
/**************************************************************/
void glStart2D()
{
	glutSetWindow(main_window);
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glGetIntegerv (GL_VIEWPORT, viewport);
	gluOrtho2D (0,viewport[2], 0, viewport[3]);
	glDepthFunc (GL_ALWAYS);
}
/**************************************************************/
void glEnd2D()
{
	glDepthFunc (GL_LESS);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix ();
}
/**************************************************/
void myGlutKeyboard(unsigned char Key, int x, int y)
{
	if (main_window == 0)
		return;
	switch(Key){
	case 27: 
		GLtrans[0] = 0;GLtrans[1] = 0;GLtrans[2] = 0;
		GLrot[0] = 20;GLrot[1] = 0;
		GLLengthScale = 1.0;
		break;
	case 'q':
		exit(0);
		break;
	};
	glutPostRedisplay();
}
/****************************************************/
void myGlutMenu( int value )
{
	myGlutKeyboard( value, 0, 0 );
}
/****************************************************/
void myGlutIdle( void )
{
	if ( glutGetWindow() != main_window ) 
		glutSetWindow(main_window);  
	glutPostRedisplay();
}
/**************************************************/
void myGlutMotion(int x, int y )
{
	glutPostRedisplay(); 
}
//**************************************************
static void updateGT(int dx,int dy)
{
    switch(GLState) {
    case ROTATE:
		GLrot[0] += (dx * 180.0f) / 10000.0f;
		GLrot[1] += (dy * 180.0f) / 10000.0f;
		break;
    case TRANSLATE:
		GLtrans[0] += GraphicsModelSize*dx/2000.0f;
		GLtrans[1] -= GraphicsModelSize*dy/2000.0f;
		break;
	case ZOOM:
		if (abs(dy) > abs(dx)) {
			if (dy < 0)  
				GLLengthScale *= 0.96;
			else
				GLLengthScale *=1.04;
		}
		break;
    }
}
/**********************************************************************/
void motionGT(int x, int y) 
{
	if (main_window == 0)
		return;
	updateGT(x-curX,y-curY);
	glutPostRedisplay();
}
/**********************************************************************/
void mouseGT(int button, int state, int x, int y) 
{
	if (main_window == 0)
		return;
	static int omx, omy, mx, my;

	if (state == GLUT_UP)
		GLState = 0;//stop all changes
	else {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			GLState = TRANSLATE;
			break;
		case GLUT_MIDDLE_BUTTON:
			GLState = ROTATE;
			break;
		case GLUT_RIGHT_BUTTON:
			GLState = ZOOM;
			break;
		default:
			break;
		}
		curX = x;
		curY = y;
	}
}
/**********************************************************************/
void setViewRoutine(int viewMode)
{
	GLtrans[0] =0;GLtrans[1] =0; GLtrans[2] =0;
	GLLengthScale = 1.0;
	switch(viewMode) {
		case 0:
			GLrot[0] = 20;GLrot[1] = 0;
			break;
		case 1:
			GLrot[0] = 0;GLrot[1] = 0;
			break;
		case 2:
			GLrot[0] = 180;GLrot[1] = 180;
			break;
		case 3:
			GLrot[0] = 0;GLrot[1] = 90;
			break;
		case 4:
			GLrot[0] = 0;GLrot[1] = -90;
			break;
		case 5:
			GLrot[0] = 90;GLrot[1] = 0;
			break;
		case 6:
			GLrot[0] = -90;GLrot[1] = 0;
			break;
	}
	myGlutDisplay();
}
/************************************************************/
void setGraphicsModelSize(float size)
{
	GraphicsModelSize = size;
	GLLengthScale = 1.0;
}

/**********************************************************/
void myGlutDisplay( void )
{
	if (main_window == 0) return;
	glClearColor( 1.0f,1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective(30.0,xy_aspect,0.1,200);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -2.6f );
	glTranslatef( obj_pos[0], obj_pos[1], -obj_pos[2] ); 
	glMultMatrixf( view_rotate );
	float scale = 1.0/GraphicsModelSize; // model occupies  screen
	glScalef( scale, scale, scale );
	glTranslatef(GLtrans[0], GLtrans[1], GLtrans[2]);
	glRotatef(GLrot[0], 0.0, 1.0, 0.0);
	glRotatef(GLrot[1], 1.0, 0.0, 0.0);
	glScalef(GLLengthScale, GLLengthScale, GLLengthScale );
	(*userDisplayFunction)();// call any user function
	glEnable( GL_LIGHTING );
	int strLen = (int) strlen(userDefinedTitle);
	
	glStart2D();
	printText((int)(0.5*viewport[2] - 1.0*strLen*5),(int)(0.9*viewport[3]),userDefinedTitle);
	glEnd2D();
	glutSwapBuffers(); 
}
/*****************************************************/
void myGlutReshape( int width, int height )
{
	if (main_window == 0) return;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	xy_aspect = (float)width / (float)height;
	myGlutDisplay();
}
/************************************************************/
int resetXY(void)
{
	for (int i = 0; i < 16; i++)
		view_rotate[i] = 0;
	view_rotate[0] = 1;
	view_rotate[5] = 1;
	view_rotate[10] = 1;
	view_rotate[15] = 1;

	glutPostRedisplay();
	return 1;
}

/************************************************************/
int resetXZ(void)
{
	for (int i = 0; i < 16; i++)
		view_rotate[i] = 0;
	view_rotate[0] = 1;
	view_rotate[6] = -1;
	view_rotate[9] = 1;
	view_rotate[15] = 1;

	glutPostRedisplay();
	return 1;
}
/************************************************************/
void redrawGraphics(void)
{
	if (main_window == 0) return;
	glutSetWindow(main_window);
	glutPostRedisplay();
}
/************************************************************/
int resetYZ(void)
{
	for (int i = 0; i < 16; i++)
		view_rotate[i] = 0;
	view_rotate[2] = -1;
	view_rotate[5] = 1;
	view_rotate[8] = 1;
	view_rotate[15] = 1;

	glutPostRedisplay();
	return 1;
}
/************************************************************/
void startAnimation(void)
{
	glutIdleFunc(myGlutDisplay);
}
/************************************************************/
void stopAnimation(void)
{
	glutIdleFunc(NULL);
}
/************************************************************/
void initializeWindows(void)
{
	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/
	GLfloat light0_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light0_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
	GLfloat light0_position[] = {.5f, .5f, 1.0f, 0.0f};

	GLfloat light1_ambient[] =  {0.1f, 0.1f, 0.3f, 1.0f};
	GLfloat light1_diffuse[] =  {.6f, .6f, 1.0f, 1.0f};
	GLfloat light1_position[] = {0.2f, 1.5f, 0.2f, 0.0f};

	GLfloat lights_rotation[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

	if (main_window == 0) {// Open window,etc
		int argc = 1;
		char *argv[] = {"dummy argument"};

		glutInit(&argc, argv);
		glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
		glutInitWindowPosition( 20, 120 );
		glutInitWindowSize( 570, 500 );

		main_window = glutCreateWindow( "Graphics Window" );
		//HWND hwnd = FindWindow( (LPCSTR) "GLUT", (LPCSTR)"Graphics Window" );
		HWND hwnd = FindWindow( (LPCWSTR) "GLUT", (LPCWSTR)"Graphics Window" );
		//RemoveMenu(GetSystemMenu ( hwnd, FALSE),SC_CLOSE,MF_BYCOMMAND);
		glutDisplayFunc( myGlutDisplay );
		glutReshapeFunc(myGlutReshape);
		glutMouseFunc(mouseGT);
		glutMotionFunc(motionGT);
		glutKeyboardFunc(myGlutKeyboard);
	}
	else {
		glutSetWindow(main_window);
		glutShowWindow();
	}
	glEnable(GL_LIGHTING);
	glEnable( GL_NORMALIZE );

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	glLightfv(GL_LIGHT1, GL_AMBIENT_AND_DIFFUSE, light1_diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
}

/************************************************************/
void GraphicsTemplate(float modelScale,void (*userFunction)(void),char *str)
{
	// Set the model size and user display function
	GraphicsModelSize = modelScale;
	userDisplayFunction = userFunction;
	strcpy(userDefinedTitle,str);
	if (main_window == 0){
		initializeWindows();
		glutSetWindow(main_window);
		glShadeModel(GL_SMOOTH);	
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable( GL_NORMALIZE );
		glutMainLoop();
	}
	else{
		glutSetWindow(main_window);
		glEnable(GL_DEPTH_TEST);
		glShadeModel(GL_SMOOTH);
	}
}
