#include "ui_paretoProjects.hpp"
extern void writeMessageToLog(const char format[],...);
/***************************************************/
ParetoProjects :: ParetoProjects( QWidget *parent) : QDialog(parent)
{

    setupUi(this);
    makeConnections();

}

/***************************************************/

ParetoProjects :: ~ParetoProjects()
{
}
/***************************************************/
void ParetoProjects::closeDialog()
{
    parentWidget()->show();
    close();

}
/*****************************************************/
void ParetoProjects::openSupportStructureDialog()
{

    if(nullptr == supportStructure)
        supportStructure = new Ui_SupportStructure(this);

    writeMessageToLog("starting support structure application ...");
    supportStructure->setViewer( viewer );
    supportStructure->show();
}

/***************************************************/
void ParetoProjects:: makeConnections()
{
    PushButton(supportStructurePushButton, [=]{ openSupportStructureDialog();});
    PushButton(closePushButton, [=]{ closeDialog();});

}
