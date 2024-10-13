#include "ui_verboseDialog.hpp"

#include "CoreFunctions.hpp"
#include <QtConcurrent/QtConcurrent>

using namespace QtConcurrent;


/////////////////////////////////////////////////////
VerboseDialog::VerboseDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

}
/////////////////////////////////////////////////////
VerboseDialog::~VerboseDialog()
{

}
/////////////////////////////////////////////////////
void VerboseDialog::showEvent(QShowEvent *e)
{
    //first store values before updating, else liveVar gets updated
    int verbose[6];
    verbose[0] = liveVar.stlVerbose;
    verbose[1] = liveVar.voxVerbose;
    verbose[2] = liveVar.hexmeshVerbose;
    verbose[3] = liveVar.deflationVerbose;
    verbose[4] = liveVar.thermalVerbose;
    verbose[5] = liveVar.topoptVerbose;
}
/////////////////////////////////////////////////////
void VerboseDialog::updateLiveVariables(void)
{


    liveVar.stlVerbose = STLVerboseCheckBox->isChecked();
    liveVar.voxVerbose = VoxVerboseCheckBox->isChecked();
    liveVar.hexmeshVerbose = ElasticityVerboseCheckBox->isChecked();
    liveVar.plasticityVerbose = PlasticityVerboseCheckBox->isChecked();
    liveVar.deflationVerbose = DeflationVerboseCheckBox->isChecked();
    liveVar.thermalVerbose = ThermalVerboseCheckBox->isChecked();
    liveVar.topoptVerbose = TopOptVerboseCheckBox->isChecked();

    cout <<"Disabling all verbose" << endl;
    vox.disableSTLVerbose();
    vox.disableVoxVerbose();
    topopt.disableHexmeshVerbose();
    topopt.disableAFPlasticityVerbose();
    topopt.disableDeflationVerbose();
    topopt.disableThermoElasticityVerbose();
    topopt.disableTSVerbose();

    cout << "-----------------" << endl;
    cout <<"Verbose update:" << endl;

    if (liveVar.stlVerbose) vox.enableSTLVerbose();
    if (liveVar.voxVerbose) vox.enableVoxVerbose();
    if (liveVar.hexmeshVerbose) topopt.enableHexmeshVerbose();
    if (liveVar.plasticityVerbose) topopt.enableAFPlasticityVerbose();
    if (liveVar.deflationVerbose) topopt.enableDeflationVerbose();
    if (liveVar.thermalVerbose) topopt.enableThermoElasticityVerbose();
    if (liveVar.topoptVerbose) topopt.enableTSVerbose();
     cout << "-----------------" << endl;
}
/////////////////////////////////////////////////////
void VerboseDialog::closeVerboseDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void VerboseDialog::makeConnections()
{

    CheckBox( STLVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( VoxVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( ElasticityVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( PlasticityVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( DeflationVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( ThermalVerboseCheckBox, [=]{ updateLiveVariables();});
    CheckBox( TopOptVerboseCheckBox, [=]{ updateLiveVariables();});

    PushButton( closePushButton, [=]{ closeVerboseDialog();});
}
/////////////////////////////////////////////////////
