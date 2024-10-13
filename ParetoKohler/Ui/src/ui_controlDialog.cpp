#include "ui_controlDialog.hpp"
#include <iostream>
///////////////////////////////////////////////////////////////////////////////

ControlDialog :: ControlDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    updatePushButtons();
    QFile HelpFile(":/resources/KohlerCPSGuide.pdf");;
    HelpFile.copy(qApp->applicationDirPath().append("/KohlerCPSGuide.pdf"));
}
///////////////////////////////////////////////////////////////////////////////
ControlDialog :: ~ControlDialog()
{
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openVerboseDialog()
{
    if( nullptr == verboseDialog )
        verboseDialog = new VerboseDialog( this );
    verboseDialog->setViewer( viewer );
    verboseDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openUnitDialog()
{
    if( nullptr == unitDialog )
        unitDialog = new UnitsDialog( this );
    QObject::connect( unitDialog, SIGNAL(setUnitsComplete()), this, SLOT(updatePushButtons()));
    unitDialog->setViewer( viewer );
    unitDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openGeometryDialog()
{
    if (!liveVar.UNITS_APPLIED){
        writeWarningToLog("Units are not set");
        return;
    }
    if( nullptr == geomDialog )
        geomDialog = new GeometryDialog( this );
    QObject::connect( geomDialog, SIGNAL(setGeometryComplete()), this, SLOT(updatePushButtons()));
    geomDialog->setViewer( viewer );
    geomDialog->show();

    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
        x[i] = i/50.0 - 1; // x goes from -1 to 1
        y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openMaterialDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToLog("Geometry not loaded");
        return;
    }
    if( nullptr == materialDialog )
        materialDialog = new MaterialDialog( this );
    QObject::connect( materialDialog, SIGNAL(setMaterialComplete()), this, SLOT(updatePushButtons()));
    materialDialog->setViewer( viewer );
    materialDialog->updatePropertyLineEdits();
    materialDialog->updateMaterialLabelText();
    materialDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openGCodeDialog()
{
    if( nullptr == gCodeDialog )
        gCodeDialog = new GCodeDialog( this );

    QObject::connect( gCodeDialog, SIGNAL(gCodeComplete()), this, SLOT(updatePushButtons()));
    gCodeDialog->setViewer( viewer );
    gCodeDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openMeshDialog()
{
    if( nullptr == meshDialog )
        meshDialog = new MeshDialog( this );

    QObject::connect( meshDialog, SIGNAL(setMeshComplete()), this, SLOT(updatePushButtons()));
    meshDialog->setViewer( viewer );
    meshDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openPrintingDialog()
{

    if( nullptr == printingDialog )
        printingDialog = new PrintingDialog( this );

    QObject::connect( printingDialog, SIGNAL(setMoistureComplete()), this, SLOT(updatePushButtons()));
    printingDialog->setViewer( viewer );
    printingDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openDryingDialog()
{

    if( nullptr == dryingDialog )
        dryingDialog = new DryingDialog( this );

    QObject::connect( printingDialog, SIGNAL(setMoistureComplete()), this, SLOT(updatePushButtons()));
    dryingDialog->setViewer( viewer );
    dryingDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openDisplayOptionsDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToLog("Geometry not loaded");
        return;
    }
    if( nullptr == displayOptionsDialog )
        displayOptionsDialog = new DisplayOptionsDialog( this );
    displayOptionsDialog->setViewer( viewer );
    displayOptionsDialog->show();
}

///////////////////////////////////////////////////////////////////////////////
void ControlDialog::closeControlDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openProjectsDialog()
{
     return;
    if( nullptr == projectDialog )
        projectDialog = new ProjectDialog( this );
    QObject::connect( projectDialog, SIGNAL(setProjectLoadingComplete()), this, SLOT(updatePushButtons()));
    projectDialog->setViewer( viewer );
    projectDialog->show();

}
/********************************************************/
void ControlDialog::updatePushButtons()
{
    const char *leftAlign ="Text-align:left";
    const char *notAllowed = ":/resources/notAllowed.png";
    const char *available = ":/resources/available.png";
    const char *done = ":/resources/done.png";

    verbosePushButton->setStyleSheet(leftAlign);
    unitPushButton->setStyleSheet(leftAlign);
    geometryPushButton->setStyleSheet(leftAlign);
    displayOptionsPushButton->setStyleSheet(leftAlign);
    materialPushButton->setStyleSheet(leftAlign);
    projectsPushButton->setStyleSheet(leftAlign);
    helpPushButton->setStyleSheet(leftAlign);

    verbosePushButton->setIcon(QIcon(notAllowed));
    unitPushButton->setIcon(QIcon(notAllowed));
    geometryPushButton->setIcon(QIcon(notAllowed));
    materialPushButton->setIcon(QIcon(notAllowed));
    gCodePushButton->setIcon(QIcon(notAllowed));
    meshPushButton->setIcon(QIcon(notAllowed));
    printingPushButton->setIcon(QIcon(notAllowed));
    dryingPushButton->setIcon(QIcon(notAllowed));
    displayOptionsPushButton->setIcon(QIcon(notAllowed));
    projectsPushButton->setIcon(QIcon(notAllowed));
    helpPushButton->setIcon(QIcon(notAllowed));

    verbosePushButton->setIcon(QIcon(available));
    unitPushButton->setIcon(QIcon(available));
    projectsPushButton->setIcon(QIcon(available));
    helpPushButton->setIcon(QIcon(available));
    unitPushButton->setIcon(QIcon(available));
    geometryPushButton->setIcon(QIcon(available));

    if (liveVar.UNITS_APPLIED){
        unitPushButton->setIcon(QIcon(done));
        geometryPushButton->setIcon(QIcon(available));
    }
    if (liveVar.GEOMETRY_UPLOADED){
        geometryPushButton->setIcon(QIcon(done));
        displayOptionsPushButton->setIcon(QIcon(available));
        //materialPushButton->setIcon(QIcon(available));
        gCodePushButton->setIcon(QIcon(available));
    }

    if (liveVar.GCODE_LOADED){
        gCodePushButton->setIcon(QIcon(done));
        meshPushButton->setIcon(QIcon(available));
    }
    if (liveVar.MESH_APPLIED){
        gCodePushButton->setIcon(QIcon(done));
        meshPushButton->setIcon(QIcon(done));
        printingPushButton->setIcon(QIcon(available));
        dryingPushButton->setIcon(QIcon(available));
    }

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog :: makeConnections()
{
    PushButton( verbosePushButton, [=]{ openVerboseDialog();});
    PushButton( unitPushButton, [=]{ openUnitDialog();});
    PushButton( geometryPushButton, [=]{ openGeometryDialog();});
    PushButton( gCodePushButton, [=]{ openGCodeDialog();});
    PushButton( materialPushButton, [=]{ openMaterialDialog();});
    PushButton( meshPushButton, [=]{ openMeshDialog();});
    PushButton( printingPushButton, [=]{ openPrintingDialog();});
    PushButton( dryingPushButton, [=]{ openDryingDialog();});
    PushButton( displayOptionsPushButton, [=]{ openDisplayOptionsDialog();});
    PushButton( projectsPushButton, [=]{ openProjectsDialog();});
    PushButton( helpPushButton, [=]{QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath().append("/KohlerCPSGuide.pdf")));});
}
///////////////////////////////////////////////////////////////////////////////
