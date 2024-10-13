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

    static bool firstTime = false;
    if (firstTime){
        QMessageBox msgBox;

        msgBox.setText("Mouse and keyboard behavior\n\n"
                       "To rotate: Use left button\n"
                       "To zoom: Use middle button, or left button + control key\n"
                       "To pan: Use right button\n\n"

                       "To select surface: Use left button + alt key\n"
                       "To unselect: Use right button + alt key\n");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        int32_t userReply = msgBox.exec();
        firstTime = false;
    }

}
/////////////////////////////////////////////////////
void ProjectDialog::saveProjectDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED) return;
    //static QString lastSelectedDirectory;
    static QString prjName,dummy ;

    static QString prjExtension("Pareto Project File (*.prj)") ;
    prjName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "Project.prj");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,prjName,prjExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    liveVar.prjFileName = fileName;
    coreFunctions.savePRJ();
    closeProjectDialog();
}
/////////////////////////////////////////////////////
void ProjectDialog::saveProjectWithResultsDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED) return;
    //static QString lastSelectedDirectory;
    static QString prjName,dummy ;

    static QString prjExtension("Pareto Project File (*.prj)") ;
    prjName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + "Project.prj");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,prjName,prjExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    liveVar.prjFileName = fileName;
    coreFunctions.savePRJ(true);// with results
    closeProjectDialog();
}
/////////////////////////////////////////////////////
void ProjectDialog::saveMeshDialog()
{
    if (!liveVar.MESH_APPLIED) return;
    //static QString lastSelectedDirectory;
    static QString meshName,dummy ;

    static QString meshExtension("Mesh File (*.mesh)") ;
    meshName = QString::fromStdString(liveVar.projectDirectory + "/" + liveVar.modelName + ".mesh");
    QString qstr = QFileDialog::getSaveFileName(this,dummy,meshName,meshExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    liveVar.meshFileName = fileName;
    coreFunctions.saveMesh();
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
    PushButton( savePRJWithResultsPushButton, [=]{ saveProjectWithResultsDialog();});
    PushButton( saveMeshPushButton, [=]{ saveMeshDialog();});
    PushButton( closePushButton, [=]{ closeProjectDialog();});

}
