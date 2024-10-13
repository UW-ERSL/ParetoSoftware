#include "ui_autoDesignDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>

extern void setStatusMessage(const char str[]);
/*****************************************************************/
void AutoDesignDialog::closeDialog()
{

    parentWidget()->show();
    close();
}
/*****************************************************************/
AutoDesignDialog::AutoDesignDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
AutoDesignDialog::~AutoDesignDialog()
{
    ;
}
/*****************************************************************/
void AutoDesignDialog::showEvent(QShowEvent *e)
{
    surfaceControlComboBox->setCurrentIndex(2);
    topologyControlComboBox->setCurrentIndex(2);

}
/*****************************************************************/
void AutoDesignDialog::createAutoDesignGeometry()
{
    if ((!liveVar.topoptConstraints.isExtrudeActive) &&
    (!liveVar.topoptConstraints.isDrawDirectionActive)){
        writeWarningToGUIWindow("Extrude topopt constraint must be selected to use AutoDesign");
        return;
    }
    liveVar.autoDesignTopologyError = 1.0;
    liveVar.autoDesignSurfaceError = 1.0;

    switch (topologyControlComboBox->currentIndex()){
        case 0: liveVar.autoDesignTopologyError = 5.0; break;
        case 1: liveVar.autoDesignTopologyError = 2.0; break;
        case 2: liveVar.autoDesignTopologyError = 1.0; break;
        case 3: liveVar.autoDesignTopologyError = 0.5; break;
        case 4: liveVar.autoDesignTopologyError = 0.1; break;

    }

    switch (surfaceControlComboBox->currentIndex()){
        case 0: liveVar.autoDesignSurfaceError = 5.0; break;
        case 1: liveVar.autoDesignSurfaceError = 2.0; break;
        case 2: liveVar.autoDesignSurfaceError = 1.0; break;
        case 3: liveVar.autoDesignSurfaceError = 0.5; break;
        case 4: liveVar.autoDesignSurfaceError = 0.1; break;
    }
    liveVar.autoDesignSmootheningIterations = smootheningSpinBox->value();
    liveVar.autoDesignContourOffset = 5;

    string fileName = liveVar.projectDirectory + liveVar.modelName + string("_autoDesign.off");
    if (!coreFunctions.autoDesign(fileName)){
        writeWarningToGUIWindow("Unable to create design");
        return;
    }
    writeMessageToGUIWindow("File %s saved", fileName.c_str());
    setStatusMessage("AutoDesign file saved");

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void AutoDesignDialog :: makeConnections()
{
    PushButton(createGeometryPushButton,[=]{createAutoDesignGeometry();});
}
