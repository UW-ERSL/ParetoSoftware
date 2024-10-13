#include "ui_controlDialog.hpp"
#include <iostream>

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
void ControlDialog::openLatticeStructureDialog()
{
    if( nullptr == latticeStructureDialog )
        latticeStructureDialog = new LatticeStructureDialog( this );

    latticeStructureDialog->setViewer( viewer );
    latticeStructureDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openCreateSolidLatticeDialog()
{
    if( nullptr == latticeRadiiDialog )
        latticeRadiiDialog = new LatticeRadiiDialog( this );

    latticeRadiiDialog->setViewer( viewer );
    latticeRadiiDialog->show();
}

///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openDisplayOptionsDialog()
{
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
void ControlDialog::openSaveFilesDialog()
{
    if( nullptr == saveFilesDialog )
        saveFilesDialog = new SaveFilesDialog( this );

    saveFilesDialog->setViewer( viewer );
    saveFilesDialog->show();
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

    displayOptionsPushButton->setStyleSheet(leftAlign);
    latticeCreateDialogPushButton->setIcon(QIcon(leftAlign));
    saveFilePushButton->setStyleSheet(leftAlign);
    helpPushButton->setStyleSheet(leftAlign);
    verbosePushButton->setIcon(QIcon(notAllowed));
    unitPushButton->setIcon(QIcon(notAllowed));

    latticeCreateDialogPushButton->setIcon(QIcon(notAllowed));
    createSolidLatticePushButton->setIcon(QIcon(notAllowed));
    displayOptionsPushButton->setIcon(QIcon(notAllowed));
    saveFilePushButton->setIcon(QIcon(notAllowed));
    helpPushButton->setIcon(QIcon(notAllowed));

    verbosePushButton->setIcon(QIcon(available));
    unitPushButton->setIcon(QIcon(available));
    saveFilePushButton->setIcon(QIcon(available));
    helpPushButton->setIcon(QIcon(available));


    unitPushButton->setIcon(QIcon(available));// to allow drag and drop
    displayOptionsPushButton->setIcon(QIcon(available)); // to allow drag and drop
    latticeCreateDialogPushButton->setIcon(QIcon(available));
    createSolidLatticePushButton->setIcon(QIcon(available));
    if (liveVar.LATTICE_UPLOADED){
        createSolidLatticePushButton->setIcon(QIcon(available));
        displayOptionsPushButton->setIcon(QIcon(available));
    }
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog :: makeConnections()
{
    PushButton( verbosePushButton, [=]{ openVerboseDialog();});
    PushButton( unitPushButton, [=]{ openUnitDialog();});
    PushButton( displayOptionsPushButton, [=]{ openDisplayOptionsDialog();});
    PushButton( latticeCreateDialogPushButton, [=]{ openLatticeStructureDialog();});
    PushButton( createSolidLatticePushButton, [=]{ openCreateSolidLatticeDialog();});
    PushButton( saveFilePushButton, [=]{ openSaveFilesDialog();});
    PushButton( helpPushButton, [=]{QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath().append("/ParetoLatticeGuide.pdf")));});
}
///////////////////////////////////////////////////////////////////////////////
