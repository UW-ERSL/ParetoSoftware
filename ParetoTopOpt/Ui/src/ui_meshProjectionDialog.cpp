#include "ui_meshProjectionDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>

extern void setStatusMessage(const char str[]);
/*****************************************************************/
void MeshProjectionDialog::closeDialog()
{

    parentWidget()->show();
    close();
}
/*****************************************************************/
MeshProjectionDialog::MeshProjectionDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
MeshProjectionDialog::~MeshProjectionDialog()
{
    ;
}
/*****************************************************************/
void MeshProjectionDialog::showEvent(QShowEvent *e)
{
   ;
}
/*****************************************************************/
void MeshProjectionDialog::projectIsosurface()
{
    double relativeProjectionDistance,relativeRemeshDensity;
    relativeProjectionDistance = projectionDistanceSpinBox->value();
    relativeRemeshDensity = remeshDensitySpinBox->value();
    coreFunctions.projectIsosurfaceMeshOntoCAD(relativeProjectionDistance,relativeRemeshDensity);

    parentWidget()->show();
    viewer->updateGL();
    closeDialog();

    return;
}
///////////////////////////////////////////////////////////////////////////////
void MeshProjectionDialog :: makeConnections()
{
    PushButton(projectIsosurfacePushButton,[=]{projectIsosurface();});
}
