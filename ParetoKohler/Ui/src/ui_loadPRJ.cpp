#include "ui_loadPRJ.hpp"
#include "CoreFunctions.hpp"


/////////////////////////////////////////////////////
ProjectDialog::~ProjectDialog()
{

}
/////////////////////////////////////////////////////
ProjectDialog :: ProjectDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/////////////////////////////////////////////////////
void ProjectDialog::loadProjectDialog()
{
    //static QString lastSelectedDirectory("");

    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select PRJ File "),
                                                QString(),
                                                *new QString( "project File (*.prj)"));
    if (qstr.isNull()) return;

    string fileName = qstr.toUtf8().constData();
    if (fileName.empty()) return;
    liveVar.prjFileName = fileName;
    coreFunctions.loadPRJ("prj");
    viewer->updateGL();
    closeProjectDialog();
    setProjectLoadingComplete();
}
/////////////////////////////////////////////////////
void ProjectDialog::saveProjectDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED) return;
    //static QString lastSelectedDirectory;
    static QString prjName,dummy ;

    static QString prjExtension("Project File (*.prj)") ;
    prjName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "Project.prj");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,prjName,prjExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    liveVar.prjFileName = fileName;
    coreFunctions.savePRJ();

    closeProjectDialog();
}
/////////////////////////////////////////////////////
void ProjectDialog::closeProjectDialog()
{
    parentWidget()->show();
    close();
}
/////////////////////////////////////////////////////
void ProjectDialog::makeConnections(){
    PushButton( loadPRJPushButton, [=]{ loadProjectDialog();});
    PushButton( savePRJPushButton, [=]{ saveProjectDialog();});
    PushButton( closePushButton, [=]{ closeProjectDialog();});

}
