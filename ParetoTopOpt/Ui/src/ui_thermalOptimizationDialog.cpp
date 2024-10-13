#include "ui_thermalOptimizationDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>


/*****************************************************************/
void ThermalOptimizationDialog::closeDialog()
{
    emit setTopOptComplete();
    parentWidget()->show();
    close();
}
/*****************************************************************/
void ThermalOptimizationDialog::setTopOptVariables()
{

    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void ThermalOptimizationDialog::onOptimizationFinished()
{
    viewer->updateGL();
}
/*****************************************************************/
ThermalOptimizationDialog::ThermalOptimizationDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
ThermalOptimizationDialog::~ThermalOptimizationDialog()
{
    ;
}
/////////////////////////////////////////////////////
void ThermalOptimizationDialog::updateLoadSetCallBack(void)
{

   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/*****************************************************************/
void ThermalOptimizationDialog::showEvent(QShowEvent *e)
{
    desiredVolFracSpinBox->setValue(liveVar.desiredVolFraction);
    stepSizeSpinBox->setValue(liveVar.stepSize);
    saveIntermediateCheckBox->setChecked(liveVar.saveIntermediate);
}
/*****************************************************************/
void ThermalOptimizationDialog::stop()
{
    liveVar.stopOptimization = true;

}

/*****************************************************************/
void runThermalOptimization(void)
{

    coreFunctions.singleLoadThermalOptimize();

}
/*****************************************************************/
void ThermalOptimizationDialog::optimize()
{



    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();

    liveVar.LATTICE_DESIGNED = false;//
    liveVar.TOPOPT_SOLVED = false;
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    topopt.resetLinearSolvesCounter();
    runThermalOptimization();

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void ThermalOptimizationDialog :: makeConnections()
{

    PushButton(optimizePushButton,[=]{optimize();});
    PushButton(stopOptimizationPushButton,[=]{stop();});
}
