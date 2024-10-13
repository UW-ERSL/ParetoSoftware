#ifndef TEMPLATE_RENDERER_H
#define TEMPLATE_RENDERER_H

#include "Renderer.hpp"

class TemplateRenderer: public Renderer {
public:
      TemplateRenderer();
      void drawSomething(void);
      void drawAxis(void);
      void plotLatticeGraph(void);
      void plotSolidLattice(void);
      void plotLatticeGraphData(void);
      void plotSolidLatticeData(void);
      void drawNiceArrow(double startPt[3],double endPt[3]);
      void draw3DText(const char *text,double pos[3],double scale);
      ~TemplateRenderer();

private:

};
#endif // TEMPLATE_RENDERER_H
