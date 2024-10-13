#include "ui_ZernikeDialog.hpp"
#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>
#include "CoreFunctions.hpp"


extern CoreFunctions           coreFunctions;
/////////////////////////////////////////////////////
ZernikeDialog::ZernikeDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
ZernikeDialog::~ZernikeDialog()
{

}
/////////////////////////////////////////////////////
void ZernikeDialog::showEvent(QShowEvent *e)
{
    //first store values before updating, else liveVar gets updated


    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
}
/////////////////////////////////////////////////////
void ZernikeDialog::computeZernikeCoefficients()
{
    string outFile;
    int gridSize = gridSizeSpinBox->value();
    int order = ZernikeOrderSpinBox->value();
    outFile = liveVar.projectDirectory + liveVar.modelName + string("_") + to_string(gridSize) + string("_") + to_string(order)  + string("_zernike.txt");

    coreFunctions.computeZernikeCoefficients(gridSize,order,outFile);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(outFile)));
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void ZernikeDialog::closeZernikeDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void ZernikeDialog::makeConnections()
{
    PushButton( computeZernikePushButton, [=]{ computeZernikeCoefficients();});
}
/////////////////////////////////////////////////////
