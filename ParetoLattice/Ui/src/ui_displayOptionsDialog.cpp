#include "ui_displayOptionsDialog.hpp"
#include "CoreFunctions.hpp"
#include <iostream>

/*****************************************************************/
DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();

    geomMap["None"] = DISPLAY_GEOM_NONE;
    geomMap["LatticeGraph"] = DISPLAY_GEOM_LATTICE;
    geomMap["STLGeometry"] =DISPLAY_GEOM_INITIAL;
    geomMap["SolidLattice"] = DISPLAY_GEOM_OTHER;
}
/*****************************************************************/
void DisplayOptionsDialog::updateDisplay()
{
    string geomToPlot =geomComboBox->currentText().toStdString();

    if ((geomMap[geomToPlot] == DISPLAY_GEOM_TOPOLOGY) && (!liveVar.TOPOPT_SOLVED)){
        writeWarningToLog("Solve topology optimization for this display option.");
        return;
    }
    liveVar.displayGeom = geomMap[geomToPlot];
    liveVar.geomTransparent = transparentGeomCheckBox->isChecked();
    liveVar.plotBoundingBox =  boundingBoxCheckBox->isChecked();
    liveVar.plotAxis = axisCheckBox->isChecked();
    viewer->updateGL();
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::resetView()
{
    viewer->camera()->showEntireScene();
    viewer->updateGL();
}

/*****************************************************************/
void DisplayOptionsDialog::closeDialog()
{
    parentWidget()->show();
    close();
}
/*****************************************************************/
void DisplayOptionsDialog::showEvent(QShowEvent *)
{

    // first store the liveVar data, else when updated they get overwritten
    bool geomTransparent = liveVar.geomTransparent;
    bool plotAxis = liveVar.plotAxis;
    bool plotBoundingBox = liveVar.plotBoundingBox;

    // update the combo box using liveVar
    geomComboBox->setCurrentIndex(1);
    axisCheckBox->setChecked(plotAxis);
    transparentGeomCheckBox->setChecked(geomTransparent);
    boundingBoxCheckBox->setChecked(plotBoundingBox);

}
/*****************************************************************/
DisplayOptionsDialog::~DisplayOptionsDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void DisplayOptionsDialog :: makeConnections()
{
    ComboBox( geomComboBox, [=]{ updateDisplay();});
    CheckBox( boundingBoxCheckBox, [=]{ updateDisplay();});
    CheckBox( transparentGeomCheckBox, [=]{ updateDisplay();});
    CheckBox( axisCheckBox, [=]{ updateDisplay();});
    PushButton( resetViewPushButton, [=]{ resetView();});
    PushButton( closePushButton, [=]{ closeDialog();});
}
