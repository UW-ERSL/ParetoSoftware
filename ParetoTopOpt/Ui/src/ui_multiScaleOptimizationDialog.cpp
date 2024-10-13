#include "ui_multiScaleOptimizationDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>
//extern MicroStructure *microStructure;
using namespace QtConcurrent;

/*****************************************************************/
void MultiScaleOptimizationDialog::closeDialog()
{
    emit setTopOptComplete();
    parentWidget()->show();
    close();
}
/*****************************************************************/
void MultiScaleOptimizationDialog::setTopOptVariables()
{
    liveVar.noOfMicroStructureGroups = numClustersSpinBox->value();
    liveVar.typeOfClustering = typeOfClusteringSpinBox->value();
    liveVar.useAllLoads = useAllLoadsCheckBox->isChecked();   
    liveVar.currentBCSet = useLoadSpinBox->value();
    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();
    liveVar.numMicroElems = (numMicroElemsComboBox->currentText()).toInt();
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void MultiScaleOptimizationDialog::onOptimizationFinished()
{
    viewer->updateGL();
}
/*****************************************************************/
MultiScaleOptimizationDialog::MultiScaleOptimizationDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
MultiScaleOptimizationDialog::~MultiScaleOptimizationDialog()
{
    ;
}
/////////////////////////////////////////////////////
void MultiScaleOptimizationDialog::updateLoadSetCallBack(void)
{
   liveVar.currentBCSet = useLoadSpinBox->value();
   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/*****************************************************************/
void MultiScaleOptimizationDialog::showEvent(QShowEvent *e)
{

    objectiveCombo->setCurrentIndex(0);
    if (liveVar.topoptObjective == VONMISES )
        objectiveCombo->setCurrentIndex(1);
    else if (liveVar.topoptObjective == EIGENVALUE )
        objectiveCombo->setCurrentIndex(2);
    liveVar.typeOfClustering = 3;
    useLoadSpinBox->setRange(0,liveVar.nBCSet-1);
    useAllLoadsCheckBox->setChecked(liveVar.useAllLoads);
    useLoadSpinBox->setValue(liveVar.currentBCSet);
    desiredVolFracSpinBox->setValue(liveVar.desiredVolFraction);
    stepSizeSpinBox->setValue(liveVar.stepSize);
    saveIntermediateCheckBox->setChecked(liveVar.saveIntermediate);
    numMicroElemsComboBox->setCurrentIndex(0);
    numElemsSpinBox->setValue(liveVar.nElemsToMesh);
    numClustersSpinBox->setValue(liveVar.noOfMicroStructureGroups);
    typeOfClusteringSpinBox->setValue(liveVar.typeOfClustering);
    liveVar.maxAspectRatio = 1;
}
/*****************************************************************/
void MultiScaleOptimizationDialog::stop()
{
    liveVar.stopOptimization = true;
}

/*****************************************************************/
void MultiScaleOptimizationDialog::updateMinMaxNumClusters()
{
    if(typeOfClusteringSpinBox->value()==3){
        numClustersSpinBox->setRange(3,20);
        numClustersSpinBox->setSingleStep(1);
        if(numClustersSpinBox->value()<3)
            numClustersSpinBox->setValue(3);
    }
    else{
        numClustersSpinBox->setRange(1,20);
        numClustersSpinBox->setSingleStep(1);
    }

}
/*****************************************************************/
void MultiScaleOptimizationDialog::optimize()
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
    liveVar.numMicroElems = (numMicroElemsComboBox->currentText()).toInt();
    liveVar.noOfMicroStructureGroups = numClustersSpinBox->value();
    liveVar.typeOfClustering = typeOfClusteringSpinBox->value();
    liveVar.nElemsToMesh = numElemsSpinBox->value();


    coreFunctions.singleLoadOptimize_multiscale();
    cout<<"All done"<<endl;

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void MultiScaleOptimizationDialog :: makeConnections()
{
    SpinBoxi( useLoadSpinBox, [=]{ updateLoadSetCallBack();});
    SpinBoxi(typeOfClusteringSpinBox, [=] {updateMinMaxNumClusters();});
    PushButton(optimizePushButton,[=]{optimize();});
    PushButton(stopOptimizationPushButton,[=]{stop();});
}
