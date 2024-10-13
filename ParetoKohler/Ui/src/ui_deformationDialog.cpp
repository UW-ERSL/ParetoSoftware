#include "ui_deformationDialog.hpp"
#include "gCodeReader.hpp"

extern gCodeReader             gCode;
extern void resetProgressBar(const char title[]);
extern void print2DText(int x, int y, const char string[], float clr[3] = NULL);

/***************************************************/
DeformationDialog::DeformationDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

}
///////////////////////////////////////////////////////////////////////////////
void DeformationDialog :: simulateDeformationDuringPrinting()
{

}
///////////////////////////////////////////////////////////////////////////////
void DeformationDialog :: simulateDeformationDuringDrying()
{

}
///////////////////////////////////////////////////////////////////////////////
void DeformationDialog :: stopSimulation()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void DeformationDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void DeformationDialog :: makeConnections()
{
    PushButton( simulatePrintingPushButton, [=]{simulateDeformationDuringPrinting(); });
    PushButton( simulateDryingPushButton, [=]{simulateDeformationDuringDrying(); });
    PushButton( stopPushButton, [=]{stopSimulation(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}
/***************************************************/
DeformationDialog::~DeformationDialog()
{
    ;
}
