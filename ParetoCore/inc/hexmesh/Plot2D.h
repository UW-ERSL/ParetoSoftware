#ifndef PLOT2D_H
#define PLOT2D_H

#include <GL/glui.h>
#include <vector>

#define MAX_CURVES 10
#define MAX_DATA_SIZE 20000
#define WINDOW_SIZE 600
#define AXIS_SIZE (0.65*WINDOW_SIZE)
#define OFFSET (0.2*WINDOW_SIZE)

class Plot2D {
public:
		static int grapher_main_window;
		static int nCurvesToPlot;
		static char xLABEL[256],yLABEL[256],Title[256];
		// the following depend on the number of curves to plot
		static int		nDATAPOINTS[MAX_CURVES];
		static char	LEGEND[MAX_CURVES][256];
		static double xMIN,yMIN,xMAX,yMAX,xRANGE,yRANGE;
		std::vector<double>  xDATA[MAX_CURVES];
		std::vector<double>  yDATA[MAX_CURVES];

		static float grapher_xy_aspect;
		static GLint grapher_viewport[4];

		static void grapher_myGlutDisplay( void );
		static void grapher_drawAxes(void);
		static void grapher_printText(void *font,int x, int y, const char string[]);
		static void grapher_myGlutReshape( int x, int y );
		static void addCurve(std::vector<double> ySamples, char *legend = NULL);

public:
			Plot2D(void);
			void openNewWindow(void);
			~Plot2D();
};

#endif