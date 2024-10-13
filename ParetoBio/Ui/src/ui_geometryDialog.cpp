#include "ui_geomDialog.hpp"
#include <iostream>
#include "CoreFunctions.hpp"

///////////////////////////////////////////////////////////////////////////////

GeometryDialog :: GeometryDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    unitMap["meter"] = STL_M;
    unitMap["millimeter"] = STL_MM;
    unitMap["centimeter"] = STL_CM;
    unitMap["inch"] = STL_INCH;
}

///////////////////////////////////////////////////////////////////////////////

GeometryDialog :: ~GeometryDialog()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: closeDialog()
{
    emit setGeometryComplete();
    parentWidget()->show();
    close();
}

///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: generateIsoSTL()
{
    if (!liveVar.GEOMETRY_UPLOADED)
        return;

    coreFunctions.distanceFunctionIsosurface();
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: loadSTL()
{
    //static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Geometry File "),
                                                QString(""),
                                              *new QString( "Mesh Format (*.off *.stl)"));

    string fileName = qstr.toUtf8().constData();
    if (fileName.empty()){
        return;
    }

    if ( (fileName.find(".off") == std::string::npos) && (fileName.find(".stl") == std::string::npos)
         && (fileName.find(".STL") == std::string::npos)){
        writeWarningToLog("File format not recognized; acceptable extensions: .off or .stl");
        return;
    }

    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
    }
    viewer->updateGL();
    closeDialog();

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
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::saveAsOff()
{
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select save File "),
                                                lastSelectedDirectory,
                                                *new QString( "Mesh Format (*.off )"));

    string fileName = qstr.toUtf8().constData();
    cout<<"File : " << fileName<<endl;
    vox.saveAsOff(fileName.c_str());
}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::saveSTL()
{
    //saveScaleLineEdit->setEnabled(1);
    // saveLabel->setText(QString("stl scale"));
    // saveScaleLineEdit->setText(QString::number(1) ) ;
    //  saveScaleLineEdit->setValidator(new QDoubleValidator(this));
    //  int saveScale = saveScaleLineEdit->text().toDouble();
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select save File "),
                                                lastSelectedDirectory,
                                                *new QString( "Mesh Format (*.off *.obj *.stl)"));

    string fileName = qstr.toUtf8().constData();
    cout<<"File : " << fileName<<endl;
}

///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: updateSTL()
{

    if (!liveVar.GEOMETRY_UPLOADED){
        return;
    }
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Mesh File "),
                                                QString(""),
                                                *new QString( "Mesh Format (*.off *.obj *.stl)"));

    string fileName = qstr.toUtf8().constData();


    if (fileName.empty()){
        return;
    }
    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulUpdate = coreFunctions.updateGeom(liveVar.assemblyMode);

    if(successfulUpdate) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.plotElasticityBC = true;
    }
    viewer->updateGL();

    closeDialog();

}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog :: makeConnections()
{
    PushButton( loadSTLPushButton, [=]{loadSTL(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
