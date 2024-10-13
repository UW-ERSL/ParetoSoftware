#include "ui_topologyOptimizationDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>

using namespace QtConcurrent;
QFuture<void> optimizationProcess;
QFutureWatcher<void> optimizationProcessWatcher;

/*****************************************************************/
void TopologyOptimizationDialog::closeDialog()
{
    emit setTopOptComplete();
    parentWidget()->show();
    close();
}
/*****************************************************************/
void TopologyOptimizationDialog::setTopOptVariables()
{
    liveVar.useAllLoads = useAllLoadsCheckBox->isChecked();
    liveVar.currentBCSet = useLoadSpinBox->value();
    liveVar.desiredVolFraction = desiredVolFracSpinBox->value();
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.saveIntermediate = saveIntermediateCheckBox->isChecked();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void TopologyOptimizationDialog::onOptimizationFinished()
{
    viewer->updateGL();
}
/*****************************************************************/
TopologyOptimizationDialog::TopologyOptimizationDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
TopologyOptimizationDialog::~TopologyOptimizationDialog()
{
    ;
}
/////////////////////////////////////////////////////
void TopologyOptimizationDialog::updateLoadSetCallBack(void)
{
   liveVar.currentBCSet = useLoadSpinBox->value();
   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/*****************************************************************/
void TopologyOptimizationDialog::showEvent(QShowEvent *e)
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
}
/*****************************************************************/
void TopologyOptimizationDialog::stop()
{
    liveVar.stopOptimization = true;

}
/*****************************************************************/
void TopologyOptimizationDialog::simpMessage()
{
    if (useSIMPMethodBox->isChecked())
        ;//writeWarningToGUIWindow("Currently the SIMP implementation does not consider any of the topopt constraints");
}
/*****************************************************************/
void runOptimization(void)
{
    if (liveVar.useSIMPMethod){
        coreFunctions.changeTopOptBC();
        coreFunctions.singleLoadOptimizeSIMP();
    }
    else if ((liveVar.nBCSet > 1) && (liveVar.useAllLoads)){
        if ((liveVar.topoptObjective == EIGENVALUE) || (liveVar.topoptConstraints.isMinEigenActive)){
            QMessageBox msgBox;
            msgBox.setText("Eigenvalue constraints and objectives have not yet been implemented for multi-load problems.");
            msgBox.addButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowModality(Qt::NonModal);
            msgBox.show();
            msgBox.exec();
            return;
        }
        coreFunctions.multiLoadOptimize();
    }
    else {
        coreFunctions.changeTopOptBC();
        coreFunctions.singleLoadOptimize();
    }
}
/*****************************************************************/
void flushQt(void)
{
    QCoreApplication::processEvents();
}
/*****************************************************************/
void TopologyOptimizationDialog::optimize()
{
    switch (objectiveCombo->currentIndex()){
        case 0: liveVar.topoptObjective = COMPLIANCE; break;
        case 1: liveVar.topoptObjective = VONMISES; break;
        case 2: liveVar.topoptObjective = EIGENVALUE; break;
    }

    if ((liveVar.topoptObjective == EIGENVALUE) && (liveVar.topoptConstraints.minEigenValueAllowedHz <= 0)){
        QMessageBox msgBox;
        msgBox.setText("With Eigenvalue maximization as objective, you must also impose Eigenvalue constraint (lower limit). Please add Eigenvalue constraint under TopOpt constraints.");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        msgBox.exec();
        return;
    }
    liveVar.stepSize = stepSizeSpinBox->value();
    liveVar.useAllLoads  = useAllLoadsCheckBox->isChecked();
    liveVar.useSIMPMethod = useSIMPMethodBox->isChecked();
    liveVar.useTSFieldForIsoSurfaceExtraction = useTSFieldForIsoSurface->isChecked();
    liveVar.snapToGeom = useTSFieldForIsoSurface->isChecked();// use same checkbox for both
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
    runOptimization();

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void TopologyOptimizationDialog :: makeConnections()
{
    SpinBoxi( useLoadSpinBox, [=]{ updateLoadSetCallBack();});
    PushButton(optimizePushButton,[=]{optimize();});
    PushButton(stopOptimizationPushButton,[=]{stop();});
    CheckBox( useSIMPMethodBox, [=]{ simpMessage();});
}
