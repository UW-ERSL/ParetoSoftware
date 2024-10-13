#include "TriangulateWires.hpp"

#include "ui_inputLatticeGraphDialog.hpp"

#include <iostream>
#include "CoreFunctions.hpp"
extern TriangulateWires triwires;
extern LatticeParameters latticeParams;
extern TemplateViewer *thisViewer;
///////////////////////////////////////////////////////////////////////////////

InputLatticeGraphDialog :: InputLatticeGraphDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    unitMap["meter"] = STL_M;
    unitMap["millimeter"] = STL_MM;
    unitMap["centimeter"] = STL_CM;
    unitMap["inch"] = STL_INCH;
}

///////////////////////////////////////////////////////////////////////////////

InputLatticeGraphDialog :: ~InputLatticeGraphDialog()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
void InputLatticeGraphDialog :: closeDialog()
{
    emit setGeometryComplete();
    parentWidget()->show();
    close();
}

///////////////////////////////////////////////////////////////////////////////
void InputLatticeGraphDialog::loadLatticeGraph()
{

        // load user defined graph file in .off format
        QString qstr = QFileDialog::getOpenFileName(this,
                                                    *new QString("Select Graph File "),
                                                    QString(""),
                                                    *new QString( "Graph File (*.off)"));

        string latticeGraphFile = qstr.toUtf8().constData();
        if (latticeGraphFile.empty()){
            return;
        }

        liveVar.projectDirectory = latticeGraphFile;
        liveVar.modelName  = latticeGraphFile;

        //modelName ... strip all path data and extension
        size_t pos = liveVar.modelName.rfind('/');
        liveVar.modelName = (liveVar.modelName).substr(pos+1);
        liveVar.modelName = (liveVar.modelName).substr(0,liveVar.modelName.length()-4);// strip .off extension
        pos = liveVar.projectDirectory.rfind('/');
        liveVar.projectDirectory = (liveVar.projectDirectory).substr(0,pos+1);
        writeMessageToGUIWindow("Project directory: %s",liveVar.projectDirectory.c_str());

        triwires.readGraph(latticeGraphFile);
        liveVar.LATTICE_UPLOADED = true;
        liveVar.GEOMETRY_MODIFIED = true;
        liveVar.displayGeom = DISPLAY_GEOM_LATTICE;
        liveVar.displayField = DISPLAY_FIELD_NONE;
        liveVar.modelScale = triwires.getModelScale();
        closeDialog();
}
///////////////////////////////////////////////////////////////////////////////
void InputLatticeGraphDialog :: makeConnections()
{
    PushButton( loadLatticeGraphPushButton, [=]{loadLatticeGraph(); });
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
