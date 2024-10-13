#ifndef TEMPLATE_RENDERER_H
#define TEMPLATE_RENDERER_H

#include "Renderer.hpp"
#include "TemplateViewer.hpp"
class TemplateRenderer: public Renderer {
public:
      TemplateRenderer();
      void drawGCode(void);
      void drawGCodeFilament(void);
      void plotMoistureOverMeshSurface(void);
      ~TemplateRenderer();

private:
    TemplateViewer *viewer;
};
#endif // TEMPLATE_RENDERER_H
