#include <stdlib.h>
#include <string.h>
#include <GL/glui.h>
   
#pragma warning(disable:4996)
 
#define MAX(a,b) (((a) > (b))? (a):(b))
#define MIN(a,b) (((a) < (b))? (a):(b))

static float grapher_xy_aspect;
static int grapher_main_window=0;
GLint grapher_viewport [4];
GLUI *gluiGrapher;

#define MAX_CURVES 10
#define MAX_DATA_SIZE 20000
#define WINDOW_SIZE 600
#define AXIS_SIZE (0.65*WINDOW_SIZE)
#define OFFSET (0.2*WINDOW_SIZE)

char xLABEL[256],yLABEL[256],Title[256];

static int nCurvesToPlot = 0;
// the following depend on the number of curves to plot
int		nDATAPOINTS[MAX_CURVES];
char	LEGEND[MAX_CURVES][256];
double  xDATA[MAX_CURVES][MAX_DATA_SIZE],yDATA[MAX_CURVES][MAX_DATA_SIZE],xMIN,yMIN,xMAX,yMAX,xRANGE,yRANGE;

/************************************************************/
void closeGracefully(int control)
{
	//gluiGrapher->close();
	glutSetWindow(grapher_main_window);
	glutHideWindow();
}
/************************************************************/
void bringGrapherWindowToFront(void)
{
	if (grapher_main_window == 0) return;
	glutSetWindow( grapher_main_window );
	glutPostRedisplay();
}
/************************************************************/
void grapher_printText(void *font,int x, int y, const char string[])
{	
	glRasterPos2f(x, y);
	for (int i = 0; string[i] != '\0'; ++i)
		glutBitmapCharacter(font, string[i]);
}
/**************************************************************/
void grapher_myGlutReshape( int x, int y )
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glViewport( tx, ty, tw, th );
	grapher_xy_aspect = (float)tw / (float)th;
	glutPostRedisplay();
}
/************************************************************/
void grapher_drawAxes(void)
{
	float x0,y0;
	int nTicks = 5;
	char str[256];

	glDisable( GL_LIGHTING );
	glPushMatrix();

	// x and y axis
	glBegin( GL_LINES );
	glLineWidth(4.0);
	glColor3f(0.0f,0.0f,1.0f);
	glBegin(GL_LINES);
		glVertex2f(OFFSET,OFFSET);
		glVertex2f(AXIS_SIZE+OFFSET+30,OFFSET);
		glVertex2f(OFFSET,OFFSET);
		glVertex2f(OFFSET,AXIS_SIZE+OFFSET+30);
	glEnd();

	// Xticks
	y0 = OFFSET;
	glBegin(GL_LINES);
		for (int i = 0; i <= nTicks; i++) {
			x0 = 1.0*AXIS_SIZE*i/nTicks+OFFSET;	
			glVertex2f(x0,y0);
			glVertex2f(x0,y0-5);
		}
	glEnd();
	

	// X Ticks values
	for (int i = 0; i <= nTicks; i++) {
		x0 = 1.0*AXIS_SIZE*i/nTicks+OFFSET -15;	
		sprintf(str,"%.f",xMIN + xRANGE*i/nTicks); 
		grapher_printText(GLUT_BITMAP_HELVETICA_18,x0, y0-25,str);
	}

	// Xgrid lines
	glLineStipple(1, 0xAAAA );
	glEnable( GL_LINE_STIPPLE );
	glLineWidth(0.5);
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINES);
		for (int i = 0; i <= 2*nTicks; i++) {
			x0 = 1.0*AXIS_SIZE*i/(2*nTicks)+OFFSET;	
			glVertex2f(x0,y0);
			glVertex2f(x0,y0+AXIS_SIZE);
		}
	glEnd();
	glDisable( GL_LINE_STIPPLE );

	// Yticks
	x0 = OFFSET;
	glBegin(GL_LINES);
		for (int i = 0; i <= nTicks; i++) {
			y0 = 1.0*AXIS_SIZE*i/nTicks+OFFSET;	
			glVertex2f(x0,y0);
			glVertex2f(x0-5,y0);
		}
	glEnd();


	// Y Ticks values
	for (int i = 0; i <= nTicks; i++) {
		y0 = 1.0*AXIS_SIZE*i/nTicks+OFFSET-6;	
		sprintf(str,"%.f",yMIN + yRANGE*i/nTicks); 
		grapher_printText(GLUT_BITMAP_HELVETICA_18,x0-40, y0,str);
	}

	// ygrid lines
	glLineStipple(1, 0xAAAA );
	glEnable( GL_LINE_STIPPLE );
	glLineWidth(0.5);
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_LINES);
		for (int i = 0; i <= 2*nTicks; i++) {
			y0 = 1.0*AXIS_SIZE*i/(2*nTicks)+OFFSET;	
			glVertex2f(x0,y0);
			glVertex2f(x0+AXIS_SIZE,y0);
		}
	glEnd();
	glDisable( GL_LINE_STIPPLE );

	// Labels
	glColor3f(0.1,0.1,0.1);
	grapher_printText(GLUT_BITMAP_HELVETICA_18,OFFSET + AXIS_SIZE/2.0, OFFSET-50,xLABEL);
	grapher_printText(GLUT_BITMAP_HELVETICA_18, OFFSET-100, OFFSET + AXIS_SIZE/2.0,yLABEL);
	
	glPopMatrix();
	glEnable( GL_LIGHTING );
}
/**********************************************************/
void grapher_myGlutDisplay( void )
{
	float x0,y0,x1,y1,pointSize;

	glClearColor( 1.0f, 1.0f,1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glGetIntegerv (GL_VIEWPORT, grapher_viewport);
	gluOrtho2D (0,grapher_viewport[2], 0, grapher_viewport[3]);

	grapher_drawAxes();

	glLineWidth(3);
	for (int curve = 0; curve < nCurvesToPlot; curve++) {

		pointSize = 5.0f + 2.0f*curve;
		x0 = AXIS_SIZE/2.5+OFFSET;
		y0 = AXIS_SIZE;
		y0 = y0-curve*30;
		if (curve%2 == 1) {// odd curves
			glEnable(GL_POINT_SMOOTH);
			glLineStipple(1, 0xAAAA );
			glEnable( GL_LINE_STIPPLE );
		}
		glPointSize(pointSize);
		if (nCurvesToPlot > 1) {
			glBegin(GL_POINTS);
				glVertex2f(x0,y0);
			glEnd();
			grapher_printText(GLUT_BITMAP_HELVETICA_18,x0+10, y0-5,LEGEND[curve]);
		}
		x0 = AXIS_SIZE*(xDATA[curve][0]-xMIN)/xRANGE+OFFSET;
		y0 =  AXIS_SIZE*(yDATA[curve][0]-yMIN)/yRANGE+OFFSET;

		glBegin(GL_LINES);
		for (int i = 0; i < nDATAPOINTS[curve]-1; i++) {
			x1 = AXIS_SIZE*(xDATA[curve][i+1]-xMIN)/xRANGE+OFFSET;
			y1 = AXIS_SIZE*(yDATA[curve][i+1]-yMIN)/yRANGE+OFFSET;
			glVertex2f(x0,y0);
			glVertex2f(x1,y1);
			x0 = x1;
			y0 = y1;
		}
		glEnd();
		glPointSize(pointSize);
		if (nCurvesToPlot > 1) {
			glBegin(GL_POINTS);
			for (int i = 0; i < nDATAPOINTS[curve]; i++) {
				x0 = AXIS_SIZE*(xDATA[curve][i]-xMIN)/xRANGE+OFFSET;
				y0 = AXIS_SIZE*(yDATA[curve][i]-yMIN)/yRANGE+OFFSET;
				glVertex2f(x0,y0);
			}
			glEnd();
		}
		glDisable(GL_POINT_SMOOTH);
		glDisable( GL_LINE_STIPPLE );
	}
	glEnable( GL_LIGHTING );
	/*int i = nDATAPOINTS[nCurvesToPlot-1]-2;
	x0 = AXIS_SIZE*(xDATA[nCurvesToPlot-1][i]-xMIN)/xRANGE+OFFSET;
	y0 = AXIS_SIZE*(yDATA[nCurvesToPlot-1][i]-yMIN)/yRANGE+OFFSET;
	i = nDATAPOINTS[nCurvesToPlot-1]-1;
	x0 += AXIS_SIZE*(xDATA[nCurvesToPlot-1][i]-xMIN)/xRANGE+OFFSET;
	y0 += AXIS_SIZE*(yDATA[nCurvesToPlot-1][i]-yMIN)/yRANGE+OFFSET;
	x0 /=2.0;
	y0 /=2.0;
	grapher_printText(GLUT_BITMAP_TIMES_ROMAN_24,x0,y0,Title);*/
	grapher_printText(GLUT_BITMAP_TIMES_ROMAN_24,(int)(0.4*grapher_viewport[2]),(int)(0.9*grapher_viewport[3]),Title);
	glutSwapBuffers();
}
/************************************************************/
void clearScreen(int control)
{
	glutSetWindow(grapher_main_window);
	grapher_myGlutDisplay();
}
/************************************************************/
void grapher_addCurve(int nPoints,double *x,double *y, char *legend)
{
	nDATAPOINTS[nCurvesToPlot] = nPoints;
	if (nCurvesToPlot == 0){
		xMIN = x[0];
		yMIN = y[0];
		xMAX = x[0];
		yMAX = y[0];
	}
	for (int i = 0; i < nPoints; i++) {
		xMIN = MIN(xMIN,x[i]);
		xMAX = MAX(xMAX,x[i]);
		yMIN = MIN(yMIN,y[i]);
		yMAX = MAX(yMAX,y[i]);
		xDATA[nCurvesToPlot][i] = x[i];
		yDATA[nCurvesToPlot][i] = y[i];
	}
	xRANGE = (xMAX-xMIN);
	yRANGE = (yMAX-yMIN);

	strcpy(LEGEND[nCurvesToPlot],legend);
	nCurvesToPlot++;
}
/************************************************************/
void grapher_initializeWindows(char *title,char *xLabel,char *yLabel)
{
	// Copy incoming data
	strcpy(Title,title);
	strcpy(xLABEL,xLabel);
	strcpy(yLABEL,yLabel);
	nCurvesToPlot = 0;// nothing to plot until grapher_addCurve is called

	xMIN = 0;
	xMAX = 1;
	yMIN = 0;
	yMAX = 1;
	xRANGE = (xMAX-xMIN);
	yRANGE = (yMAX-yMIN);

	if (grapher_main_window == 0) {// Open window,etc	
		glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
		glutInitWindowPosition( 250, 50 );
		glutInitWindowSize( WINDOW_SIZE, WINDOW_SIZE );

		grapher_main_window = glutCreateWindow( "Grapher");
		
		glutDisplayFunc( grapher_myGlutDisplay );
		glutReshapeFunc( grapher_myGlutReshape );  

		gluiGrapher = GLUI_Master.create_glui_subwindow( grapher_main_window, GLUI_SUBWINDOW );
		new GLUI_Button( gluiGrapher, "CloseGrapher", 0,(GLUI_Update_CB)closeGracefully );
		//new GLUI_Button( gluiGrapher, "ClearScreen", 0,(GLUI_Update_CB)clearScreen );
	}
	else {
		glutSetWindow(grapher_main_window);
		glutShowWindow();
		grapher_myGlutDisplay();
	}
}