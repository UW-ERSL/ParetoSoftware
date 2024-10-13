#include "ui_materialOptimizationDialog.hpp"
#include "ui_constraintsDialog.hpp"

#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>

/*****************************************************************/
void MaterialOptimizationDialog::closeDialog()
{
    emit setTopOptComplete();
    parentWidget()->show();
    close();
}
/*****************************************************************/
void MaterialOptimizationDialog::setTopOptVariables()
{

    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void MaterialOptimizationDialog::onOptimizationFinished()
{
    viewer->updateGL();
}
/*****************************************************************/
MaterialOptimizationDialog::MaterialOptimizationDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
MaterialOptimizationDialog::~MaterialOptimizationDialog()
{
    ;
}
/*****************************************************************/
void MaterialOptimizationDialog::showEvent(QShowEvent *e)
{
    objectiveCombo->setCurrentIndex(0);
    if (liveVar.topoptObjective == VONMISES )
        objectiveCombo->setCurrentIndex(1);
    else if (liveVar.topoptObjective == EIGENVALUE )
        objectiveCombo->setCurrentIndex(2);

    desiredVolFracSpinBox->setValue(liveVar.desiredVolFraction);
    stepSizeSpinBox->setValue(liveVar.stepSize);
    saveIntermediateCheckBox->setChecked(liveVar.saveIntermediate);
}
/*****************************************************************/
void MaterialOptimizationDialog::stop()
{
    liveVar.stopOptimization = true;
}
/*****************************************************************/
void runMultiMaterialOptimization(void)
{
    if (liveVar.numberOfMaterialIndicesStored < 2){
        QMessageBox msgBox;
        msgBox.setText("At least two materials must be defined for multi-material optimization.");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        msgBox.exec();
        return;
    }

    coreFunctions.singleLoadOptimize_MultiMaterial();
}
/*****************************************************************/
void MaterialOptimizationDialog::optimize()
{
    switch (objectiveCombo->currentIndex()){
        case 0: liveVar.topoptObjective = COMPLIANCE; break;
        case 1: liveVar.topoptObjective = VONMISES; break;
        case 2: liveVar.topoptObjective = EIGENVALUE; break;
    }


    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();

    liveVar.LATTICE_DESIGNED = false;//
    liveVar.TOPOPT_SOLVED = true;// for live update
    liveVar.plotTopOptConstraints = false;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    liveVar.plotThermalBC = false;
    liveVar.plotBoundingBox = false;

    topopt.resetLinearSolvesCounter();

    int mostExpensiveMaterial = 0;
    double price = topopt.getPriceOfMaterial(0);
    for (int matl = 0; matl < topopt.getNumMaterials(); matl++){
        if (topopt.getPriceOfMaterial(matl) > price){
            price = topopt.getPriceOfMaterial(matl);
            mostExpensiveMaterial = matl;
        }
    }
    topopt.setMaterialIndexOfPart(0,mostExpensiveMaterial);// start off with the most expensive material

    cout << " mostExpensiveMaterial = " << mostExpensiveMaterial << endl;

    runMultiMaterialOptimization();

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void MaterialOptimizationDialog::openMaterialDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == materialDialog )
        materialDialog = new MaterialDialog( this );

    topopt.keepUnusedMaterial(true);// call this before setting material
    QObject::connect( materialDialog, SIGNAL(setMaterialComplete()), this, SLOT(updateMaterialIndexCallBack()));
    materialDialog->setViewer( viewer );
    materialDialog->updatePropertyLineEdits();
    materialDialog->updateMaterialLabelText();
    materialDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void MaterialOptimizationDialog :: updateMaterialIndexCallBack()
{

    // Store the material properties
    if (materialIndexSpinBox->value() > liveVar.numberOfMaterialIndicesStored){
        QMessageBox msgBox;
        msgBox.setText("Material index has been skipped");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        msgBox.exec();
        return;
    }
    /* E = materialProperties[0];
    nu = materialProperties[1];
    Y = materialProperties[2];// Yield strength
    rho = materialProperties[3];
    K = materialProperties[4];
    alpha = materialProperties[5];
    Cp = materialProperties[6];*/

    materialProperties[materialIndexSpinBox->value()][0] = liveVar.E;
    materialProperties[materialIndexSpinBox->value()][1] = liveVar.nu;
    materialProperties[materialIndexSpinBox->value()][2] = liveVar.Y;
    materialProperties[materialIndexSpinBox->value()][3] = liveVar.rho;
    materialProperties[materialIndexSpinBox->value()][4] = liveVar.K;
    materialProperties[materialIndexSpinBox->value()][5] = liveVar.alpha;
    materialProperties[materialIndexSpinBox->value()][6] = liveVar.specificHeat;
    materialProperties[materialIndexSpinBox->value()][7] = liveVar.price;


    liveVar.nodeBasedKU = 0;// node based KU not allowed anymore
    topopt.setTStoAnalytical(false); // cannot use analytical TS anymore

    liveVar.numberOfMaterialIndicesStored = MAX(liveVar.numberOfMaterialIndicesStored,materialIndexSpinBox->value()+1);

    liveVar.currentBCSet = 0;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void MaterialOptimizationDialog :: makeConnections()
{
    PushButton(materialSelectionPushButton, [=] { openMaterialDialog();});
    PushButton(optimizePushButton,[=]{optimize();});
    PushButton(stopOptimizationPushButton,[=]{stop();});
}
