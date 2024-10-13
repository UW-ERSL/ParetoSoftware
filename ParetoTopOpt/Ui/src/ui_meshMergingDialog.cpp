#include "ui_meshMergingDialog.hpp"
#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include <iostream>

extern void setStatusMessage(const char str[]);
/*****************************************************************/
void MeshMergingDialog::closeDialog()
{

    parentWidget()->show();
    close();
}
/*****************************************************************/
MeshMergingDialog::MeshMergingDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************************/
MeshMergingDialog::~MeshMergingDialog()
{
    ;
}
/*****************************************************************/
void MeshMergingDialog::showEvent(QShowEvent *e)
{
   ;
}
/*****************************************************************/
void MeshMergingDialog::mergeMesh()
{
    double CADControlParam,IsosurfaceControlParam;
    CADControlParam = CADControlSpinBox->value();
    IsosurfaceControlParam = IsosurfaceControlSpinBox->value();
    coreFunctions.mergeCADandIsosurfaceMesh(CADControlParam,IsosurfaceControlParam);
    return;
}
///////////////////////////////////////////////////////////////////////////////
void MeshMergingDialog :: makeConnections()
{
    PushButton(mergeMeshesPushButton,[=]{mergeMesh();});
}
