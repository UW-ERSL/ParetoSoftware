#include "ui_geomDialog.hpp"
#include <iostream>
#include "CoreFunctions.hpp"

extern void writeMessageToGUIWindow(const char format[],...);

using namespace std;

extern CoreFunctions           coreFunctions;
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
        writeWarningToGUIWindow("File format not recognized; acceptable extensions: .off or .stl");
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
void GeometryDialog :: loadGeometry()
{
    //static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Geometry File "),
                                                QString(""),
                                              *new QString( "Mesh Format (*.off *.stl *.obj *.step *.stp *.iges .igs)"));

    string fileName = qstr.toUtf8().constData();

    if (fileName.empty()){
        return;
    }
    //accepted formats, must consider lower and upper cases
    //don't change the order, append if needed.
    char *acceptedFormats[] = {".stl",".STL",".off",".OFF",".obj",".OBJ",".step",".STEP",".stp",".STP",".iges",".IGES",".igs",".IGS"};
    int nFormats = 14;
    int formatId,extensionIndex;
    // check format
    for ( formatId = 0; formatId < nFormats; formatId++){
        extensionIndex = fileName.find(acceptedFormats[formatId]);
        if (extensionIndex != std::string::npos)
            break;
    }
    if (formatId == nFormats){
        string msg("File format not recognized; acceptable extensions: ");
        for ( formatId = 0; formatId < nFormats; formatId++){
            msg.append(acceptedFormats[formatId]);
        }
        writeWarningToGUIWindow(msg.c_str());
        return;
    }
    writeMessageToParetoLog("Input format: %s",acceptedFormats[formatId] );

    switch (formatId){
        case 0: case 1: case 2: case 3: case 4: case 5: break;// stl off or obj format nothing to do

        default:
            writeWarningToGUIWindow("File format %s not recognized",fileName.c_str());
            return;
            break;
    }


    // proceed to load .stl, obj or .off file
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
void GeometryDialog::createTetmesh()
{

    if (!liveVar.GEOMETRY_UPLOADED) return;

    coreFunctions.createTetmesh();
    viewer->updateGL();

}
///////////////////////////////////////////////////////////////////////////////
void GeometryDialog::scaleGeometry()
{
    double STLArea,STLVolume;
    if (!liveVar.GEOMETRY_UPLOADED) return;

    vox.scaleSTLGeomAboutOrigin(geomScalingSpinBox->value());

    liveVar.modelScale = vox.getModelScale();

    STLVolume = vox.getVolumeOfSTLGeom(&STLArea);
    writeMessageToGUIWindow("#Triangles = %d; #Nodes = %d ",vox.getNumTriangles(),vox.getNumTriPoints());
    writeMessageToGUIWindow("STL volume = %e; STL Area = %e ",STLVolume,STLArea);

    viewer->updateGL();

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
void GeometryDialog::saveAsSTL()
{
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select save File "),
                                                lastSelectedDirectory,
                                                *new QString( "Mesh Format (*.stl)"));

    string fileName = qstr.toUtf8().constData();
    cout<<"File : " << fileName<<endl;
    vox.saveAsSTL(fileName.c_str());
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
    //PushButton( loadSTLPushButton, [=]{loadGeometry(); });
    PushButton( updateSTLPushButton, [=]{updateSTL(); });
    PushButton( createTetmeshButton, [=]{createTetmesh(); });
    PushButton( scaleGeometryButton, [=]{scaleGeometry(); });
    PushButton( saveAsOffButton, [=]{saveAsOff(); });
    PushButton( saveAsSTLButton,[=]{saveAsSTL();});
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
