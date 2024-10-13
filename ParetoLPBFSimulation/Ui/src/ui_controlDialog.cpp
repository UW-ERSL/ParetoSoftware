#include "ui_controlDialog.hpp"


///////////////////////////////////////////////////////////////////////////////

ControlDialog :: ControlDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    updatePushButtons();
    QFile HelpFile(":/resources/ParetoGuide.pdf");;
    HelpFile.copy(qApp->applicationDirPath().append("/ParetoGuide.pdf"));
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

    //updatePushButtons();
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
void ControlDialog::openThermalLoadsDialog()
{
    if (!liveVar.MATERIAL_APPLIED){
        writeWarningToLog("Material not applied");
        return;
    }
    if( nullptr == thermalLoadsDialog )
        thermalLoadsDialog = new ThermalLoadsWindow( this );

    QObject::connect(thermalLoadsDialog, SIGNAL(setThermalLoadsComplete()), this, SLOT(updatePushButtons()));
    thermalLoadsDialog->setViewer( viewer );
    thermalLoadsDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openStructuralLoadsDialog()
{
    if (!liveVar.MATERIAL_APPLIED){
        writeWarningToLog("Material not applied");
        return;
    }
    if( nullptr == structuralLoadsDialog )
        structuralLoadsDialog = new StructuralLoadsWindow( this );

    QObject::connect(structuralLoadsDialog, SIGNAL(setStructuralLoadsComplete()), this, SLOT(updatePushButtons()));
    structuralLoadsDialog->setViewer( viewer );
    structuralLoadsDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openAnalysisDialog()
{
    if (!liveVar.MATERIAL_APPLIED){
        writeWarningToLog("Material not applied");
        return;
    }
    if( nullptr == analysisDialog )
        analysisDialog = new StructuralAnalysisDialog( this );


    analysisDialog->setViewer( viewer );
    analysisDialog->show();
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
    thermalLoadsPushButton->setIcon(QIcon(notAllowed));
    structuralLoadsPushButton->setIcon(QIcon(notAllowed));
    analysisPushButton->setIcon(QIcon(notAllowed));
    displayOptionsPushButton->setIcon(QIcon(notAllowed));
    projectsPushButton->setIcon(QIcon(notAllowed));
    helpPushButton->setIcon(QIcon(notAllowed));

    verbosePushButton->setIcon(QIcon(available));
    unitPushButton->setIcon(QIcon(available));
    projectsPushButton->setIcon(QIcon(available));
    helpPushButton->setIcon(QIcon(available));


    unitPushButton->setIcon(QIcon(available));// to allow drag and drop
    displayOptionsPushButton->setIcon(QIcon(available)); // to allow drag and drop
    //geometryPushButton->setIcon(QIcon(available)); // to allow drag and drop

    if (liveVar.UNITS_APPLIED){
        unitPushButton->setIcon(QIcon(done));
        geometryPushButton->setIcon(QIcon(available));
    }
    if (liveVar.GEOMETRY_UPLOADED){
        geometryPushButton->setIcon(QIcon(done));
        displayOptionsPushButton->setIcon(QIcon(available));
        materialPushButton->setIcon(QIcon(available));
    }
    if (liveVar.MATERIAL_APPLIED){
        materialPushButton->setIcon(QIcon(done));
        thermalLoadsPushButton->setIcon(QIcon(available));
        structuralLoadsPushButton->setIcon(QIcon(available));
        analysisPushButton->setIcon(QIcon(available));
    }

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog :: makeConnections()
{
    PushButton( verbosePushButton, [=]{ openVerboseDialog();});
    PushButton( unitPushButton, [=]{ openUnitDialog();});
    PushButton( geometryPushButton, [=]{ openGeometryDialog();});
    PushButton( materialPushButton, [=]{ openMaterialDialog();});
    PushButton( thermalLoadsPushButton, [=]{ openThermalLoadsDialog();});
    PushButton( structuralLoadsPushButton, [=]{ openStructuralLoadsDialog();});
    PushButton( analysisPushButton, [=]{ openAnalysisDialog();});
    PushButton( displayOptionsPushButton, [=]{ openDisplayOptionsDialog();});

    PushButton( projectsPushButton, [=]{ openProjectsDialog();});
    PushButton( helpPushButton, [=]{QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath().append("/ParetoGuide.pdf")));});
}
///////////////////////////////////////////////////////////////////////////////
