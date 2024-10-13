#include "ui_microStructuralOptimizationDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include "MicroStructure.hpp"
extern MicroStructure *microStructure;
using namespace QtConcurrent;

/*****************************************************************/
void MicroStructuralOptimizationDialog::closeDialog()
{
    emit setTopOptComplete();
    parentWidget()->show();
    close();
}
/*****************************************************************/
void MicroStructuralOptimizationDialog::setTopOptVariables()
{
    liveVar.noOfMicroStructureGroups = numGroupsSpinBox->value();
    liveVar.typeOfClustering = typeOfClusteringSpinBox->value();
    liveVar.useAllLoads = useAllLoadsCheckBox->isChecked();   
    liveVar.currentBCSet = useLoadSpinBox->value();
    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();
    liveVar.numMicroElems = (numMicroElemsComboBox->currentText()).toInt();
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void MicroStructuralOptimizationDialog::onOptimizationFinished()
{
    viewer->updateGL();
}
/*****************************************************************/
MicroStructuralOptimizationDialog::MicroStructuralOptimizationDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
MicroStructuralOptimizationDialog::~MicroStructuralOptimizationDialog()
{
    ;
}
/////////////////////////////////////////////////////
void MicroStructuralOptimizationDialog::updateLoadSetCallBack(void)
{
   liveVar.currentBCSet = useLoadSpinBox->value();
   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/*****************************************************************/
void MicroStructuralOptimizationDialog::showEvent(QShowEvent *e)
{

    objectiveCombo->setCurrentIndex(0);
    if (liveVar.topoptObjective == VONMISES )
        objectiveCombo->setCurrentIndex(1);
    else if (liveVar.topoptObjective == EIGENVALUE )
        objectiveCombo->setCurrentIndex(2);

    useLoadSpinBox->setRange(0,liveVar.nBCSet-1);
    useAllLoadsCheckBox->setChecked(liveVar.useAllLoads);
    useLoadSpinBox->setValue(liveVar.currentBCSet);
    desiredVolFracSpinBox->setValue(liveVar.desiredVolFraction);
    stepSizeSpinBox->setValue(liveVar.stepSize);
    saveIntermediateCheckBox->setChecked(liveVar.saveIntermediate);
    numMicroElemsComboBox->setCurrentIndex(0);
    numGroupsSpinBox->setValue(liveVar.noOfMicroStructureGroups);
    typeOfClusteringSpinBox->setValue(liveVar.typeOfClustering);
    liveVar.maxAspectRatio = 1;
}
/*****************************************************************/
void MicroStructuralOptimizationDialog::stop()
{
    liveVar.stopOptimization = true;
}
/*****************************************************************/
void MicroStructuralOptimizationDialog::runMicroStructuralOptimization(void)
{
    liveVar.numMicroElems = (numMicroElemsComboBox->currentText()).toInt();
    liveVar.noOfMicroStructureGroups = numGroupsSpinBox->value();
    liveVar.typeOfClustering = typeOfClusteringSpinBox->value();   
    if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads)){
        coreFunctions.multiLoadOptimize();
    }
    else {
        const int maxNoOfMicroStructureGroups = 15;
        coreFunctions.changeTopOptBC();
        microStructure = new MicroStructure[maxNoOfMicroStructureGroups];

        delete[] microStructure;
        microStructure = NULL;
        cout<<"All done"<<endl;
    }
}
/*****************************************************************/
void MicroStructuralOptimizationDialog::optimize()
{
    switch (objectiveCombo->currentIndex()){
        case 0: liveVar.topoptObjective = COMPLIANCE; break;
        case 1: liveVar.topoptObjective = VONMISES; break;
        case 2: liveVar.topoptObjective = EIGENVALUE; break;
    }
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.useAllLoads  = useAllLoadsCheckBox->isChecked();
    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();
    liveVar.numMicroElems = (numMicroElemsComboBox->currentText()).toInt();
    liveVar.LATTICE_DESIGNED = false;//
    liveVar.TOPOPT_SOLVED = true;// for live update
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;   
    topopt.resetLinearSolvesCounter();        
    microStructure = NULL;
    runMicroStructuralOptimization();
    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void MicroStructuralOptimizationDialog :: makeConnections()
{
    SpinBoxi( useLoadSpinBox, [=]{ updateLoadSetCallBack();});

    PushButton(optimizePushButton,[=]{optimize();});
    PushButton(stopOptimizationPushButton,[=]{stop();});
}
