#include "ui_latticeStructureDialog.hpp"

#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>


/////////////////////////////////////////////////////
LatticeStructureDialog::LatticeStructureDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
LatticeStructureDialog::~LatticeStructureDialog()
{

}
/////////////////////////////////////////////////////
void LatticeStructureDialog::showEvent(QShowEvent *e)
{
    //first store values before updating, else liveVar gets updated

}
/////////////////////////////////////////////////////
void LatticeStructureDialog::updateLiveVariables(void)
{

    liveVar.latticeType = latticeTypeCombo->currentIndex();
    liveVar.latticeUnitCellSizeInMM = unitSizeSpinBox->value();
    liveVar.latticeResolution = edgeDivisionSpinBox->value();
    liveVar.latticeFillRatio = fillRatioSpinBox->value();
    liveVar.latticeSurfaceRetainType = criticalSurfaceCombo->currentIndex();
    liveVar.latticeSmoothening = smootheningSpinBox->value();

    viewer->updateGL();
}
/*****************************************************************/
void LatticeStructureDialog::stopLatticeStructureDesign(void)
{
    liveVar.stopOptimization = true;
}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesign(void)
{
    updateLiveVariables();
    coreFunctions.createLatticeStructureMesh();
    viewer->updateGL();
}

/////////////////////////////////////////////////////
void LatticeStructureDialog::closeLatticeStructureDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void LatticeStructureDialog::makeConnections()
{
     PushButton( createLatticePushButton, [=]{ createLatticeDesign();});
     PushButton(stopLatticeDesignPushButton,[=]{stopLatticeStructureDesign();});
     PushButton(closePushButton,[=]{closeLatticeStructureDialog();});

}
/////////////////////////////////////////////////////
