#include "ui_thermoElasticAnalysisDialog.hpp"
#include "CoreFunctions.hpp"

extern CoreFunctions           coreFunctions;
/////////////////////////////////////////////////////
ThermoElasticAnalysisDialog::ThermoElasticAnalysisDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
ThermoElasticAnalysisDialog::~ThermoElasticAnalysisDialog()
{

}
/////////////////////////////////////////////////////
void ThermoElasticAnalysisDialog::showEvent(QShowEvent *e)
{
    bool meshApplied = liveVar.MESH_APPLIED;

    liveVar.currentBCSet = loadSpinBox->value();
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.plotThermalBC = true;
    liveVar.plotTopOptConstraints = true;

    loadSpinBox->setRange(0,liveVar.nBCSet-1);

    liveVar.MESH_APPLIED = meshApplied;// save the previous value

    viewer->updateGL();
}
/////////////////////////////////////////////////////
void ThermoElasticAnalysisDialog::updateLoadSetCallBack(void)
{
   liveVar.currentBCSet = loadSpinBox->value();
   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = false;
   liveVar.plotThermalBC = true;
   viewer->updateGL();
}
/////////////////////////////////////////////////////
void ThermoElasticAnalysisDialog::updateLiveVariables(void)
{
    liveVar.MESH_APPLIED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;

    viewer->updateGL();
}
/////////////////////////////////////////////////////
void ThermoElasticAnalysisDialog::solveThermalStatic()
{
    updateLiveVariables();
    coreFunctions.solveThermoStatic();
    closeThermoElasticAnalysisDialog();
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void ThermoElasticAnalysisDialog::closeThermoElasticAnalysisDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void ThermoElasticAnalysisDialog::makeConnections()
{
    SpinBoxi( loadSpinBox, [=]{ updateLoadSetCallBack();});
    PushButton( staticThermalSolvePushButton, [=]{ solveThermalStatic();});
    PushButton( closePushButton, [=]{ closeThermoElasticAnalysisDialog();});

}
/////////////////////////////////////////////////////
