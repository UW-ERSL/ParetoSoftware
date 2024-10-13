#include "TriangulateWires.hpp"
#include "ui_latticeGraphDialog.hpp"
#include "CoreFunctions.hpp"

TriangulateWires triwires;

extern Voxelizer voxTemp;
extern LatticeParameters latticeParams;
extern bool getYesNoResponse(const char format[],...);
extern void writeWarningToGUIWindow(const char format[],...);
/************************************************************/
bool callBackLatticeGenerator(double fractionCompleted)
{
    return true;
}
/////////////////////////////////////////////////////
LatticeStructureDialog::LatticeStructureDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

}
/////////////////////////////////////////////////////
LatticeStructureDialog::~LatticeStructureDialog()
{

}
/////////////////////////////////////////////////////
void LatticeStructureDialog::showEvent(QShowEvent *e)
{

    latticeTypeCombo->setCurrentIndex(7);//octet
}

///////////////////////////////////////////////////////////////////////////////
void LatticeStructureDialog::loadLatticeGraph()
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
    viewer->updateGL();
    parentWidget()->show();
    close();
}

/////////////////////////////////////////////////////
void LatticeStructureDialog::readSTL(void)
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

    if ( (fileName.find(".stl") == std::string::npos)
        && (fileName.find(".STL") == std::string::npos)){
        writeWarningToGUIWindow("File format not recognized; acceptable extensions: .STL or .stl");
        return;
    }

    liveVar.geomFileName[liveVar.nSTLFilesLoaded] = fileName;
    bool successfulLoad = coreFunctions.loadGeom(liveVar.assemblyMode);
    if(successfulLoad) {
        liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
        liveVar.GEOMETRY_UPLOADED = true;
        liveVar.GEOMETRY_MODIFIED = true;
    }
    viewer->updateGL();

}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeGraphFromVoxelMesh(void)
{
    char latticeSpecs[1024];
    double latticeUnitCellSize,area,boundingBox[3][2],geometryVolume;
    bool success;
    double trunkRadius,xScale,yScale,zScale;
    string voxFile, stlFile;
    int numberOfLatticeVoxElems,nTrianglesExpected;
    double unitCellVolume;
    int symmetryIndex;
    string latticeGraphFile;

    voxFile = liveVar.projectDirectory + string("temp.vox");
    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.stl");
    latticeGraphFile = liveVar.projectDirectory + liveVar.modelName + string("_latticeGraph.off");
    vox.getBoundingBox(boundingBox);
    geometryVolume = vox.getVolumeOfSTLGeom(&area);

    symmetryIndex = 0;

    int nUnitCells;
    nUnitCells = nUnitCellsSpinBox->value();
    success = vox.voxelize(nUnitCells,liveVar.maxAspectRatio,symmetryIndex,callBackLatticeGenerator);

    if (!success) {
        writeWarningToLog("CoreFunction: Could not create mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }
    vox.dumpVoxelDataBinary(voxFile.c_str());
    if (!topopt.readVoxelFileBinary(voxFile.c_str())){
        writeWarningToLog("CoreFunction: Could not read mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }
    liveVar.MESH_APPLIED = true;
    remove(voxFile.c_str());

    writeMessageToLog("Computing graph from voxel mesh ");
    topopt.computeLatticeGraphFromVoxelMesh((LATTICETYPE) latticeTypeCombo->currentIndex());
    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());
    triwires.readGraph(latticeGraphFile);


    liveVar.LATTICE_UPLOADED = true;
    liveVar.GEOMETRY_MODIFIED = true;
    liveVar.displayGeom = DISPLAY_GEOM_LATTICE;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.modelScale = triwires.getModelScale();
    viewer->updateGL();
    parentWidget()->show();
    close();
}


/**********************************************************/
void LatticeStructureDialog::makeConnections()
{
    PushButton(readLatticeGraphPushButton, [=]{ loadLatticeGraph();});
    PushButton(readSTLPushButton, [=]{ readSTL();});
    PushButton(createLatticePushButton, [=]{ createLatticeGraphFromVoxelMesh();});

}
/////////////////////////////////////////////////////
