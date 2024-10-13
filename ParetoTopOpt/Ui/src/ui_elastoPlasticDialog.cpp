#include "ui_ElastoPlasticDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>

using namespace QtConcurrent;

/*****************************************************************/
void ElastoPlasticAnalysisDialog::closeDialog()
{
    parentWidget()->show();
    close();
}
/*****************************************************************/
ElastoPlasticAnalysisDialog::ElastoPlasticAnalysisDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
ElastoPlasticAnalysisDialog::~ElastoPlasticAnalysisDialog()
{
    ;
}

/*****************************************************************/
void ElastoPlasticAnalysisDialog::showEvent(QShowEvent *e)
{

}
/*****************************************************************/
void ElastoPlasticAnalysisDialog::stopAnalysis()
{
    ;
}
/*****************************************************************/
void ElastoPlasticAnalysisDialog::analyze()
{

    liveVar.nElastoPlasticForceSteps  = nForceStepsSpinBox->value();
    coreFunctions.solveElastoPlastic();
    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void ElastoPlasticAnalysisDialog :: makeConnections()
{

    PushButton(analyzePushButton,[=]{analyze();});
    PushButton(stopAnalysisPushButton,[=]{stopAnalysis();});
}
