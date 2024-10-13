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
void GeometryDialog :: loadSTL()
{
    QString qstr = QFileDialog::getOpenFileName(this, *new QString("Select Geometry File "),
                                                QString(""),  *new QString( "Mesh Format (*.off *.stl)"));

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
    double modelScale = vox.getModelScale();
    vox.scaleSTLGeomAboutOrigin(0.001); // convert from mm to meters.
    vox.setModelScale(0.001*modelScale);

    double boundingBox[3][2],offset[3];
    vox.getBoundingBox(boundingBox);

    offset[0] = -(boundingBox[0][0] + boundingBox[0][1] )/2;
    offset[1] = -(boundingBox[1][0] + boundingBox[1][1] )/2;
    offset[2] = -boundingBox[2][0];
    vox.translateSTLGeom( offset);
    liveVar.modelScale = liveVar.modelScale*0.001;
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
    }
    liveVar.plotAxis = false;
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
void GeometryDialog :: makeConnections()
{
    PushButton( loadSTLPushButton, [=]{loadSTL(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
