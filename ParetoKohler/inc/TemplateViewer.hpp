#pragma once

#include <memory>

#include <QtWidgets>
#include <qglviewer.h>

#include "CoreFunctions.hpp"

#define SpinBoxi(box, f) connect(box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), f)
#define SpinBoxd(box, f) connect(box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), f)
#define CheckBox(box, f) connect(box, static_cast<void(QCheckBox::*)(int)>(&QCheckBox::stateChanged), f)
#define ComboBox(box, f) connect(box, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), f)
#define RadioButton(button, f) connect(button, static_cast<void(QRadioButton::*)(bool)>(&QRadioButton::toggled), f)
#define PushButton(button, f)  connect(button, &QPushButton::clicked, f)
#define LineEdit(line,f) connect(line, &QLineEdit::returnPressed, f)

extern void writeWarningToLog(const char format[],...);
extern void writeMessageToLog(const char format[],...);
extern void writeSettingsToLog(const char format[],...);
extern void displayUserMessage(const char string[], float clr[3]);
extern void updateProgressBar(const char title[], int percentComplete);
extern LiveVariables liveVar;
extern ParetoApplication topopt;
extern Voxelizer vox;
extern CoreFunctions coreFunctions;


class TemplateViewer : public QGLViewer
{
public:

public:
   TemplateViewer(QWidget *parent_ = nullptr);
  ~TemplateViewer() {}


protected :
   virtual void draw();
   virtual void initializeGL();
   void mousePressEvent(QMouseEvent* e);
   void mouseDoubleClickEvent(QMouseEvent* e);
};
