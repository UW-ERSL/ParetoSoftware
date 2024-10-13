/************************************************************************/
/* Copyright (c) 2001-2009 by SoftIntegration, Inc. All Rights Reserved */
/* THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SoftIntegration       */
/* The copyright notice above does not evidence any                     */
/* actual or intended publication of such source code.                  */
/************************************************************************/
#ifndef _PLOT_H
#define _PLOT_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* macro sigl_* are deprecated */
#define sigl_major_version 2
#define sigl_minor_version 7
#define sigl_micro_version 1
#define SIGL_MAJOR_VERSION 2
#define SIGL_MINOR_VERSION 7
#define SIGL_MICRO_VERSION 1
#define SIGLVER 270
 
typedef enum {
    /* switch */
    PLOT_OFF = 0,
    PLOT_ON,

    /* plot type for 2D/3D plot */
    PLOT_PLOTTYPE_LINES,
    PLOT_PLOTTYPE_IMPULSES,
    PLOT_PLOTTYPE_STEPS,
    PLOT_PLOTTYPE_FSTEPS,
    PLOT_PLOTTYPE_POINTS,
    PLOT_PLOTTYPE_LINESPOINTS,
    PLOT_PLOTTYPE_DOTS,
    PLOT_PLOTTYPE_HISTEPS,
    PLOT_PLOTTYPE_BOXERRORBARS,
    PLOT_PLOTTYPE_BOXES,
    PLOT_PLOTTYPE_BOXXYERRORBARS,
    PLOT_PLOTTYPE_CANDLESTICKS,
    PLOT_PLOTTYPE_FINANCEBARS,
    PLOT_PLOTTYPE_FILLEDCURVES,
    PLOT_PLOTTYPE_VECTORS,
    PLOT_PLOTTYPE_XERRORBARS,
    PLOT_PLOTTYPE_XYERRORBARS,
    PLOT_PLOTTYPE_YERRORBARS,
    PLOT_PLOTTYPE_XERRORLINES,
    PLOT_PLOTTYPE_XYERRORLINES,
    PLOT_PLOTTYPE_YERRORLINES,

    /* plot type for 3D plot */
    PLOT_PLOTTYPE_SURFACES,

    /* scale type */
    PLOT_SCALETYPE_LINEAR,
    PLOT_SCALETYPE_LOG,

    /* output type */
    PLOT_OUTPUTTYPE_DISPLAY,
    PLOT_OUTPUTTYPE_FILE,
    PLOT_OUTPUTTYPE_STREAM,

    /* 3D coordinate system */
    PLOT_COORD_CARTESIAN,
    PLOT_COORD_SPHERICAL,
    PLOT_COORD_CYLINDRICAL,

    /* tics direction */
    PLOT_TICS_IN,
    PLOT_TICS_OUT,

    /* units for polar plots */
    PLOT_ANGLE_DEG,
    PLOT_ANGLE_RAD,

    /* test justification */
    PLOT_TEXT_LEFT,
    PLOT_TEXT_RIGHT,
    PLOT_TEXT_CENTER,

    /* plot grid type, obsolete */
    /* PLOT_GRID_RECTANGULAR, */
    /* PLOT_GRID_POLAR, */

    /* fillStype style */
    PLOT_FILLSTYLE_EMPTY,
    PLOT_FILLSTYLE_SOLID,
    PLOT_FILLSTYLE_TRANSPARENT_SOLID,
    PLOT_FILLSTYLE_PATTERN,
    PLOT_FILLSTYLE_TRANSPARENT_PATTERN

    //PLOT_OUTPUTTYPE_GIF /* to be deleted */
} plotinfo_t;

    /* boxFill style (obsolete, use fillStyle) */
#define PLOT_BOXFILL_EMPTY               PLOT_FILLSTYLE_EMPTY
#define PLOT_BOXFILL_SOLID               PLOT_FILLSTYLE_SOLID
#define PLOT_BOXFILL_TRANSPARENT_SOLID   PLOT_FILLSTYLE_TRANSPARENT_SOLID
#define PLOT_BOXFILL_PATTERN             PLOT_FILLSTYLE_PATTERN
#define PLOT_BOXFILL_TRANSPARENT_PATTERN PLOT_FILLSTYLE_TRANSPARENT_PATTERN

/* line clipping */
#define PLOT_CLIPPING_POINTS 0X0001
#define PLOT_CLIPPING_ONE    0X0002
#define PLOT_CLIPPING_TWO    0X0004

/* axis */
#define PLOT_AXIS_X   0X00001
#define PLOT_AXIS_Y   0X00002
#define PLOT_AXIS_Z   0X00004
#define PLOT_AXIS_XY  0X00003
#define PLOT_AXIS_XYZ 0X00007
#define PLOT_AXIS_X2  0X00008
#define PLOT_AXIS_Y2  0X00010


/* 3D contour mode */
#define PLOT_CONTOUR_BASE    0X01
#define PLOT_CONTOUR_SURFACE 0X02

/* plot border sides */
#define PLOT_BORDER_BOTTOM 0X0001
#define PLOT_BORDER_LEFT   0X0002
#define PLOT_BORDER_TOP    0X0004
#define PLOT_BORDER_RIGHT  0X0008
#define PLOT_BORDER_ALL    0X000F

class CPlot {
    private:
      void * m_class; 
      class CPlot *m_chclass, *m_next; 
    public:
      CPlot();  
      CPlot(const char *licensestr);  
     ~CPlot();  
      int arrow(double x_head, double y_head, double x_tail, double y_tail);
      int arrow(double x_head, double y_head, double y_tail, double z_tail, char *option);
      int arrow(double x_head, double y_head, double z_head,
                double x_tail, double y_tail, double z_tail);
      int arrow(double x_head, double y_head, double z_head,
                double x_tail, double y_tail, double z_tail, char *option);
      void autoScale(int axis, int option);
      void axes(int num, char *axes);
      void axis(int axis, int flag);
      void axis(int axis, int type, int width, char *colorname);
      void axisRange(int axis, double minimum, double maximum);
      void axisRange(int axis, double minimum, double maximum, double incr); /* obsolete, use ticsRange() */
      void barSize(double size);
      void border(int option, int side);
      void borderOffsets(double left, double right, double top, double bottom);
      void boundingBoxOrigin(double org_x, double org_y); // obsolete, use origin()
      void boxBorder(int num);
      void boxBorder(int num, int linetype);
      // boxFill() is obsolete, use fillStyle()
      void boxFill(int num, int style);
      void boxFill(int num, int style, double density);
      void boxFill(int num, int style, int patternnum);
      void boxWidth(double width);
      void changeViewAngle(double rot_x, double rot_z);
      int circle(double x_center, double y_center, double r);
      void colorBox(int option);
      void contourLabel(int option);
      void contourLevels(double levels[], int n);
      void contourMode(int mode);
      void coordSystem(int coord_system);
      void coordSystem(int coord_system, int angle_unit);
      int data(void *a, int row, int col);
      int data2DCurve(double x[], double y[], int n);
      int data3DCurve(double x[], double y[], double z[], int n);
      int data3DSurface(double x[], double y[], double z[], int n, int m);
      int dataFile(const char *file);
      int dataFile(const char *file, const char *option);
      int dataSetNum();
      void deleteData();
      void deletePlots();
      void dimension(int dim);
      void displayTime(double x_offset, double y_offset);
      void enhanceText();
      void fillStyle(int num, int style);
      void fillStyle(int num, int style, double density);
      void fillStyle(int num, int style, int patternnum);
      void font(char *fontname, int fontsize);
      void fontScale(double scale);
      int func2D(double x0, double xf, int n, double (*func)(double x));
      int func3D(double x0, double xf, double y0, double  yf, int nx, int ny,
                 double (*func)(double x, double y));
      int funcp2D(double x0, double xf, int n, double (*func)(double x, void *param), void *param);
      int funcp3D(double x0, double xf, double y0, double  yf, int nx, int ny,
                  double (*func)(double x, double y, void *param), void *param);
      void getGrid(int *flag, char *option);
      char *getLabel(int axis);
      plotinfo_t getOutputType();
      class CPlot* getSubplot(int row, int col);
      char *getTitle();
      void grid(int flag);
      void grid(int flag, char *option);
      int  isUsed();  /* return true if the plot class has been used */
      void label(int axis, const char *label);
      void legend(char *legend, int num);
      void legendLocation(double x, double y);
      void legendLocation(double x, double y, double z);
      void legendOption(char *option);
      int line(double x1, double y1, double x2, double y2);
      int line(double x1, double y1, double z1, double x2, double y2, double z2);
      void lineType(int num, int type, int width);
      void lineType(int num, int type, int width, char *color);
      void lineWidth(int width);
      int loglog(double x[], double y[], int n);
      void mathCoord(void);
      void margins(double left, double right, double top, double bottom);
      void nativeCmd(char *cmd);
      void origin(double x, double y);
      void outputType(int output_type);
      void outputType(int output_type, char *terminal);
      void outputType(int output_type, char *terminal, char *filename);
      void plotType(int plot_type, int num);
      void plotType(int plot_type, int num, char * option);
      void plotType(int plot_type, int num, int dot_type);
      void plotType(int plot_type, int num, int line_type, int line_width);
      //void plotType(int plot_type, int num, int point_type, int point_size);
      void plotType(int plot_type, int num, int line_type, int line_width, int point_type, int point_size);
      int plotting();
      int point(double x, double y);
      int point(double x, double y, double z);
      void pointType(int num, int type, int size);
      void pointType(int num, int type, int size, char *colorname);
      void pointType(int num, int type, int size, int rgb);
      void polarPlot(int option);
      int polygon(double x[], double y[], int n);
      int polygon(double x[], double y[], double z[], int n);
      int rectangle(double x, double y, double width, double height);
      void removeHiddenLine(int flag);
      void scaleType(int axis, int scale_type);
      void scaleType(int axis, int scale_type, double base);
      int semilogx(double x[], double y[], int n);
      int semilogy(double x[], double y[], int n);
      void showMesh(int option);
      void size(double xscale, double yscale);
      void size3D(double scale, double scale_z);
      void sizeOutput(int xpixels, int ypixels);
      void sizeRatio(double ratio);
      void smooth(int num, char *option);
      int subplot(int row, int col);
      void text(char *string, int just, double x, double y);
      void text(char *string, int just, double x, double y, double z);
      void tics(int axis, int option);
      void ticsDay(int axis);
      void ticsDirection(int option);
      void ticsFormat(int axis, char *format);
      void ticsLabel(int axis, char *label, double position);
      void ticsLevel(double level);
      void ticsLocation(int axis, char *location);
      void ticsMirror(int axis, int option);
      void ticsMonth(int axis);
      void ticsPosition(int axis, double pos);
      void ticsRange(int axis, double incr);
      void ticsRange(int axis, double incr, double start);
      void ticsRange(int axis, double incr, double start, double end);
      void title(const char *title);
};

/* prototypes for plotting functions */
int fplotxy(double (*func)(double x), double x0, double xf, int n);
int fplotxy(double (*func)(double x), double x0, double xf, int n,
            const char *title, const char *xlabel, const char *ylabel);
int fplotxy(double (*func)(double x), double x0, double xf, int n,
            const char *title, const char *xlabel, const char *ylabel, class CPlot *pl);

int fplotxyz(double (*func)(double x, double y), double x0, double xf, double y0, double yf, 
             int nx, int ny);
int fplotxyz(double (*func)(double x, double y), double x0, double xf, double y0, double yf, 
             int nx, int ny, const char *title, const char *xlabel, const char *ylabel, const char *zlabel);
int fplotxyz(double (*func)(double x, double y), double x0, double xf, double y0, double yf, 
             int nx, int ny, const char *title, const char *xlabel, const char *ylabel, const char *zlabel, class CPlot *pl);

int plotxy(double x[], double y[], int n);
int plotxy(double x[], double y[], int n, 
           const char *title, const char *xlabel, const char *ylabel);
int plotxy(double x[], double y[], int n,
           const char *title, const char *xlabel, const char *ylabel, class CPlot *pl);

int plotxyf(const char *file);
int plotxyf(const char *file, const char *title, const char *xlabel, const char *ylabel);
int plotxyf(const char *file, const char *title, const char *xlabel, const char *ylabel, class CPlot *pl);

int plotxyz(double x[], double y[], double z[], int n);
int plotxyz(double x[], double y[], double z[], int n,
            const char *title, const char *xlabel, const char *ylabel, const char *zlabel);
int plotxyz(double x[], double y[], double z[], int n,
            const char *title, const char *xlabel, const char *ylabel, const char *zlabel, class CPlot *pl);
int plotxyz(double x[], double y[], double z[], int nx, int ny);
int plotxyz(double x[], double y[], double z[], int nx, int ny,
            const char *title, const char *xlabel, const char *ylabel, const char *zlabel);
int plotxyz(double x[], double y[], double z[], int nx, int ny,
            const char *title, const char *xlabel, const char *ylabel, const char *zlabel, class CPlot *pl);

int plotxyzf(const char *file);
int plotxyzf(const char *file, const char *title, const char *xlabel, const char *ylabel, const char *zlabel);
int plotxyzf(const char *file, const char *title, const char *xlabel, const char *ylabel, const char *zlabel, class CPlot *pl);

int lindata(double first, double last, double *a, int n);
int funcarray(double (*func)(double x), double *y, double *x, int n);

#endif	/* _PLOT_H */
