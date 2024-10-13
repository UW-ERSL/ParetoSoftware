#include "TriangulateWires.hpp"
#include "ui_latticeGeneratorDialog.hpp"
#include "CoreFunctions.hpp"

TriangulateWires triwires;

extern Voxelizer voxTemp;
extern LatticeParameters latticeParams;
extern bool getYesNoResponse(const char format[],...);
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
    //first store values before updating, else liveVar gets updated

    double spacing,radius;
    LATTICETYPE latticeType;
    LatticeSource latticeSource;
    int circularDivision;

    latticeSource = latticeParams.latticeSource;
    spacing = latticeParams.latticeSpacingInMM;
    radius = latticeParams.latticeRadiusInMM;
    circularDivision = latticeParams.circularDivision;
    latticeType = latticeParams.latticeType;


    trunkSpacingSpinBox->setValue(spacing);
    trunkRadiusSpinBox->setValue(radius);
    circularDivisionSpinBox->setValue(circularDivision);
    latticeTypeCombo->setCurrentIndex((int)latticeType);


}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesign(void)
{


    latticeParams.latticeSpacingInMM = trunkSpacingSpinBox->value();
    latticeParams.latticeRadiusInMM = trunkRadiusSpinBox->value();
    latticeParams.circularDivision = circularDivisionSpinBox->value();
    latticeParams.latticeType = (LATTICETYPE) latticeTypeCombo->currentIndex();


}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesignFromGraph(void)
{
    int polySides;
    double trunkRadius;
    string offFile, stlFile;
    string latticeGeometryFile("latticeGeometry.stl");

    Timer timer;


    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        trunkRadius = trunkRadiusSpinBox->value()/1000;
        break;
    case UNITS_MM:
        trunkRadius = trunkRadiusSpinBox->value();
        break;
    case UNITS_INCH:
        trunkRadius = trunkRadiusSpinBox->value()/25.4;
        break;
    }
    polySides = circularDivisionSpinBox->value();

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


    latticeGeometryFile = liveVar.projectDirectory + liveVar.modelName + string("_3DGeometry.off");

    topopt.readLatticeGraphFromOffFile(latticeGraphFile.c_str());
    coreFunctions.changeTopOptBC();// to apply BC on voxel nodes this is for FEA
    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);

    vector<double> beamRadii;

    if (trunkRadius > 0)
        topopt.setLatticeGeneratorRadiusOfAllStruts(trunkRadius);// this is for FEA
    else {
        topopt.setLatticeGeneratorRadiusOfAllStruts(1.0);//default for memory creation
        // load user defined radius file in .txt format
        QString qstr2 = QFileDialog::getOpenFileName(this,
                                                    *new QString("Select Graph Radius File "),
                                                    QString(""),
                                                    *new QString( "Radius File (*.*)"));

        string latticeRadiusFile = qstr2.toUtf8().constData();
        // read the radius file
        ifstream ifile(latticeRadiusFile, ios::in);
        if( ifile.fail() ) {
            cout << "Error: Can't open input file " << latticeRadiusFile << endl;
            return;
        }
        int  numEdgesInput;
        double radius;
        beamRadii.resize(triwires.getNumberOfBeams());
        for( size_t i = 0; i < triwires.getNumberOfBeams(); i++) {
            beamRadii[i] = 1;
        }
        ifile >> numEdgesInput; // how many need to be reset
        for( size_t i = 0; i < numEdgesInput; i++) {
            ifile >>radius;
            topopt.setLatticeGeneratorradiusOfTrunkStrut(i,radius);
            beamRadii[i] = radius;
        }
    }

    if (trunkRadius > 0)
        triwires.setBeamRadius(trunkRadius);
    else{
        triwires.setBeamRadius(beamRadii);
    }

    cout <<"Assigned to triWires " << endl;
    triwires.setUnion(true);

    timer.start();
    triwires.triangulate();
    cout <<"Time taken (secs) " << timer.getElapsedTimeInSec() << endl;
    cout <<"Done creating 3D lattice geometry " << endl;

    triwires.saveAs(latticeGeometryFile );
    cout <<"Saving 3D lattice geometry " << latticeGeometryFile.c_str() << endl;
    if (!liveVar.GEOMETRY_UPLOADED){// no other geometry loaded
        vox.deleteCurrentModel();
        vox.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
        liveVar.GEOMETRY_UPLOADED =  true;
        liveVar.modelScale = vox.getModelScale();
    }
    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
    liveVar.modelScale = voxTemp.getModelScale();
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
    closeLatticeStructureDialog();
}
/////////////////////////////////////////////////////
void LatticeStructureDialog::save3DOffFileDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED) return;
    //static QString lastSelectedDirectory;
    static QString offFileName,dummy ;

    static QString fileExtension("3D Off File (*.off)") ;

    QString qstr = QFileDialog::getSaveFileName(this,dummy,offFileName,fileExtension);

    string fileName = qstr.toStdString();
    if (fileName.empty()) return;
    triwires.saveAs(fileName);
}

/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesignFromTetMesh(bool dualGraph)
{
    int polySides;
    double trunkRadius;
    string offFile, stlFile;
    string latticeGraphFile("Graph.off");
    string latticeGeometryFile("latticeGeometry.stl");
    double metric,minMetric;
    Timer timer;

    TriangulateWires triwires;
    typedef std::array<int,4> TetElement;
    typedef std::array<int,2> Edge;



    int nNodesInTetMesh,nTets,nTetEdges;
    std::vector<Point3D> tetMeshNodes,graphMeshNodes;
    std::vector<TetElement> tetMeshElements;
    std::vector<Edge>       Graph;
    std::vector<Edge>       TetEdges;
    double x,y,z;
    int n0,n1,n2,n3;

    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.stl");
    offFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.off");

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        trunkRadius = trunkRadiusSpinBox->value()/1000;
        break;
    case UNITS_MM:
        trunkRadius = trunkRadiusSpinBox->value();
        break;
    case UNITS_INCH:
        trunkRadius = trunkRadiusSpinBox->value()/25.4;
        break;
    }
    polySides = circularDivisionSpinBox->value();

    // load user defined tetmesh file in .txt format
    QString qstr = QFileDialog::getOpenFileName(this, *new QString("Select TetMesh File "),QString(""),
                                                *new QString( "Tet Mesh (*.tetmesh)"));

    string tetMeshFile = qstr.toUtf8().constData();
    if (tetMeshFile.empty()){
        writeWarningToLog("Error: Can't find file %s ", tetMeshFile.c_str());
        return;
    }
    // Read tetmesh file
    ifstream ifile(tetMeshFile.c_str(), ios::in);
    if( ifile.fail() ) {
        writeWarningToLog("Error: Can't open file %s ", tetMeshFile.c_str());
        return;
    }

    ifile >> nNodesInTetMesh;
    tetMeshNodes.resize(nNodesInTetMesh);
    for( size_t i = 0; i < nNodesInTetMesh; i++) {
        ifile >> x >> y >> z;
        tetMeshNodes[i][0] = x;
        tetMeshNodes[i][1] = y;
        tetMeshNodes[i][2] = z;
    }

    ifile >> nTets;
    tetMeshElements.resize(nTets);
    for( size_t i = 0; i < nTets; i++) {
        ifile >> n0 >> n1 >> n2 >> n3;
        tetMeshElements[i][0] = n0-1;//assumes node numbering starts at 1
        tetMeshElements[i][1] = n1-1;
        tetMeshElements[i][2] = n2-1;
        tetMeshElements[i][3] = n3-1;
    }

    ifile >> nTetEdges;
    TetEdges.resize(nTetEdges);
    for( size_t i = 0; i < nTetEdges; i++) {
        ifile >> n0 >> n1 ;
        TetEdges[i][0] = n0-1;//assumes node numbering starts at 1
        TetEdges[i][1] = n1-1;
    }
    ifile.close();

    // Create the  graph
    int nNodesInGraph, nEdgesInGraph;
    // two options: direct graph from tetmesh or its dual
    if (dualGraph){
        nNodesInGraph = nNodesInTetMesh + nTets;
        nEdgesInGraph = 4*nTets;
    }
    else {
        nNodesInGraph = nNodesInTetMesh;
        nEdgesInGraph = nTetEdges;
    }

    graphMeshNodes.resize(nNodesInGraph);
    Graph.resize(nEdgesInGraph);

    // copy over the tetmesh nodes
    for( size_t i = 0; i < nNodesInTetMesh; i++) {
        for (int j = 0; j < 3; j++)
            graphMeshNodes[i][j] = tetMeshNodes[i][j];
    }

    if (dualGraph){// add additional nodes
        minMetric = 1.0;
        for( size_t tet = 0; tet < nTets; tet++) {
            double tetPoints[4][3], baryCoords[3];
            baryCoords[0] = baryCoords[1] = baryCoords[2] = 0.25;
            for (int node = 0; node < 4; node++){
                for (int j = 0; j < 3; j++)
                    tetPoints[node][j] = tetMeshNodes[tetMeshElements[tet][node]][j];
            }
            //findOptimalBarycentricCoordinates(tetPoints,baryCoords);
            //metric =  findPrintabilityMetric(tetPoints, baryCoords);

            minMetric = min(minMetric,metric);
            double temp;
            temp = (1- baryCoords[0] - baryCoords[1] - baryCoords[2]);
            for (int j = 0; j < 3; j++)
                graphMeshNodes[nNodesInTetMesh+tet][j] =
                        tetMeshNodes[tetMeshElements[tet][0]][j]*baryCoords[0] +
                        tetMeshNodes[tetMeshElements[tet][1]][j]*baryCoords[1] +
                        tetMeshNodes[tetMeshElements[tet][2]][j]*baryCoords[2] +
                        tetMeshNodes[tetMeshElements[tet][3]][j]*temp;
        }

        cout << "Smallest angle to horizontal is " << 90 - 180*acos(minMetric)/3.1415 << endl;
        // create the graph
        for( size_t tet = 0; tet < nTets; tet++) {
            Graph[4*tet+0][0] = nNodesInTetMesh+tet; //center node
            Graph[4*tet+0][1] = tetMeshElements[tet][0]; //node0

            Graph[4*tet+1][0] = nNodesInTetMesh+tet; //center node
            Graph[4*tet+1][1] = tetMeshElements[tet][1]; //node1

            Graph[4*tet+2][0] = nNodesInTetMesh+tet; //center node
            Graph[4*tet+2][1] = tetMeshElements[tet][2]; //node2

            Graph[4*tet+3][0] = nNodesInTetMesh+tet; //center node
            Graph[4*tet+3][1] = tetMeshElements[tet][3]; //node3
        }
    }
    else{// not a dual graph
        for( size_t edge = 0; edge < nEdgesInGraph; edge++) {
            Graph[edge][0] = TetEdges[edge][0]; //center node
            Graph[edge][1] = TetEdges[edge][1]; //node0
        }
    }

    // save the graph file
    ofstream ofile(latticeGraphFile.c_str(), ios::out);
    ofile << "OFF" << endl;
    ofile << graphMeshNodes.size() << " 0  "  << Graph.size() << endl;
    for( size_t i = 0; i < graphMeshNodes.size(); i++)
        ofile << graphMeshNodes[i][0] << " " << graphMeshNodes[i][1] << "  " << graphMeshNodes[i][2] << endl;

    for( size_t i = 0; i < Graph.size(); i++)
        ofile  << Graph[i][0] << " " << Graph[i][1] << endl;
    ofile.close();

    topopt.readLatticeGraphFromOffFile(latticeGraphFile.c_str());

    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(trunkRadius);
    triwires.setUnion(true);

    timer.start();
    triwires.triangulate();
    cout <<"Time taken (secs) " << timer.getElapsedTimeInSec() << endl;

    cout <<"Done creating lattice geometry " << endl;
    cout <<"Time taken (secs) " << timer.getElapsedTimeInSec() << endl;
    cout <<"#Beams: " << triwires.getNumberOfBeams() << endl;
    cout <<"#Triangles: " << triwires.getNumberOfTriangles() << endl;

    if (triwires.getNumberOfTriangles() > 2e6){
        writeWarningToLog("Model size too large to import!");
    }
    else {
        triwires.saveAs(latticeGeometryFile );

        double boundingBox[3][2];
        voxTemp.deleteCurrentModel();
        voxTemp.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
        liveVar.modelScale = voxTemp.getModelScale();
        voxTemp.getBoundingBox(boundingBox);

        if (!liveVar.GEOMETRY_UPLOADED){// no other geometry loaded
            vox.deleteCurrentModel();
            vox.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
            liveVar.GEOMETRY_UPLOADED =  true;
            liveVar.modelScale = vox.getModelScale();
        }

        liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
        liveVar.displayField = DISPLAY_FIELD_NONE;
    }

    viewer->updateGL();
    closeLatticeStructureDialog();
}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesignFromParetoMesh(void)
{
    char latticeSpecs[1024];
    double latticeUnitCellSizeInModelUnits,materialProperties[7],area,boundingBox[3][2],geometryVolume;
    bool success;
    int polySides;
    double trunkRadius,xScale,yScale,zScale;
    string  stlFile;
    string latticeGraphFile;
    TriangulateWires triwires;

    // load user defined tetmesh file in .txt format
    QString qstr = QFileDialog::getOpenFileName(this, *new QString("Select ParetoMesh File "),QString(""),
                                                *new QString( "Parto Mesh (*.msh)"));

    string paretoMeshFile = qstr.toUtf8().constData();
    if (paretoMeshFile.empty()){
        writeWarningToLog("Error: Can't find file %s ", paretoMeshFile.c_str());
        return;
    }

    if (!topopt.readParetoMeshFile(paretoMeshFile.c_str())){
        writeWarningToLog("CoreFunction: Could not read mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }
    liveVar.MESH_APPLIED = true;

    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.stl");
    latticeGraphFile = liveVar.projectDirectory + liveVar.modelName + string("_latticeGraph.off");


    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value()/1000.0;
        trunkRadius = trunkRadiusSpinBox->value()/1000;
        break;
    case UNITS_MM:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value();
        trunkRadius = trunkRadiusSpinBox->value();
        break;
    case UNITS_INCH:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value()/25.4;
        trunkRadius = trunkRadiusSpinBox->value()/25.4;
        break;
    }
    polySides = circularDivisionSpinBox->value();


    Timer timer;

    timer.start();
    writeMessageToLog("Computing graph from Pareto mesh ");

    topopt.computeLatticeGraphFromVoxelMesh((LATTICETYPE) latticeTypeCombo->currentIndex());
    writeMessageToLog("Graph computed; time taken (secs):  %.2f ", timer.getElapsedTimeInSec());

    //now scale the lattice graph
    topopt.scaleLatticeGraph(xScale,yScale,zScale);
    topopt.setLatticeGeneratorRadiusOfAllStruts(trunkRadius);// this is for FEA
    coreFunctions.changeTopOptBC();// to apply BC on voxel nodes this is for FEA

    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());

    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(trunkRadius);// this is for computing 3D shape
    triwires.setUnion(true);

    writeMessageToLog("Creating lattice structure ");
    timer.start();
    triwires.triangulate();
    writeMessageToLog("Done creating lattice structure ");
    writeMessageToLog("Time taken (secs) %.2f ", timer.getElapsedTimeInSec());
    writeMessageToLog("#Beams: %d",triwires.getNumberOfBeams());
    writeMessageToLog("#Triangles: %d", triwires.getNumberOfTriangles());

    if (triwires.getNumberOfTriangles() > 2e6){
        writeWarningToLog("Model size too large to import!");
    }
    else {
        writeMessageToParetoLog("Saving %s", stlFile.c_str());
        //triwires.saveAs(stlFile);
        voxTemp.deleteCurrentModel();
        voxTemp.readSTLGeom(stlFile.c_str(),0, false,NULL,false);
        liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
        liveVar.displayField = DISPLAY_FIELD_NONE;
    }

    viewer->updateGL();
    closeLatticeStructureDialog();
}
/////////////////////////////////////////////////////
void LatticeStructureDialog::createLatticeDesignFromVoxelMesh(void)
{
    char latticeSpecs[1024];
    double latticeUnitCellSizeInModelUnits,materialProperties[7],area,boundingBox[3][2],geometryVolume;
    bool success;
    int polySides;
    double trunkRadius,xScale,yScale,zScale;
    string voxFile, stlFile;
    int numberOfLatticeVoxElems,nTrianglesExpected;
    double unitCellVolume;
    int symmetryIndex;
    string latticeGraphFile;
    TriangulateWires triwires;

    // apply dummy material
    materialProperties[0] = liveVar.E;
    materialProperties[1] = liveVar.nu;
    materialProperties[2] = liveVar.Y;
    materialProperties[3] = liveVar.rho;
    materialProperties[4] = liveVar.K;
    materialProperties[5] = liveVar.alpha;
    materialProperties[6] = liveVar.specificHeat;

    if (liveVar.paretoUnits == UNITS_MM){
        // for mmKs, we prescribe E and Y as N/mm^2 and density as kg/mm^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 1000;
    }
    if (liveVar.paretoUnits == UNITS_INCH){
        // for IPS, we prescribe E and Y as lbf/in^2 and density as lbm/in^3
        // but this lead to inconsistency in units; we fix this here!
        materialProperties[3] /= 386.22;
    }

    voxFile = liveVar.projectDirectory + string("temp.vox");
    stlFile = liveVar.projectDirectory + liveVar.modelName + string("_lattice.stl");
    latticeGraphFile = liveVar.projectDirectory + liveVar.modelName + string("_latticeGraph.off");
    vox.getBoundingBox(boundingBox);
    geometryVolume = vox.getVolumeOfSTLGeom(&area);

    switch (liveVar.paretoUnits) {
    case UNITS_METER:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value()/1000.0;
        trunkRadius = trunkRadiusSpinBox->value()/1000;
        break;
    case UNITS_MM:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value();
        trunkRadius = trunkRadiusSpinBox->value();
        break;
    case UNITS_INCH:
        latticeUnitCellSizeInModelUnits = trunkSpacingSpinBox->value()/25.4;
        trunkRadius = trunkRadiusSpinBox->value()/25.4;
        break;
    }
    polySides = circularDivisionSpinBox->value();


    symmetryIndex = 1*liveVar.topoptConstraints.xSymmetry +
            2*liveVar.topoptConstraints.ySymmetry +
            3*liveVar.topoptConstraints.zSymmetry;
    unitCellVolume = pow(latticeUnitCellSizeInModelUnits,3.0);
    numberOfLatticeVoxElems = NEARESTINT(geometryVolume/unitCellVolume);


    // First scale the stl inversely proportional to scaling
    vox.scaleSTLGeomAboutOriginAlongX(1./xScale);
    vox.scaleSTLGeomAboutOriginAlongY(1./yScale);
    vox.scaleSTLGeomAboutOriginAlongZ(1./zScale);

    nTrianglesExpected = numberOfLatticeVoxElems*2*polySides*12;
    sprintf(latticeSpecs,"%d lattices will be created with roughly %d triangles; proceed?",numberOfLatticeVoxElems,nTrianglesExpected);
    if(!getYesNoResponse(latticeSpecs)){
        // rescale the stl
        vox.scaleSTLGeomAboutOriginAlongX(xScale);
        vox.scaleSTLGeomAboutOriginAlongY(yScale);
        vox.scaleSTLGeomAboutOriginAlongZ(zScale);
        return;
    }
    // For multiple bodies, only some of the bodies may be latticized
    // To ensure that these lattices are embedded into the non-latticed geometry
    // some of the border voxels are also convered into latticed geometry
    //vox.setIncludeNodesCloseToBoundary(false);// do not include nodes close to boundary
    success = vox.voxelize(numberOfLatticeVoxElems,liveVar.maxAspectRatio,symmetryIndex,callBackLatticeGenerator);
    //vox.setIncludeNodesCloseToBoundary(true);//default

    // rescale the stl
    vox.scaleSTLGeomAboutOriginAlongX(xScale);
    vox.scaleSTLGeomAboutOriginAlongY(yScale);
    vox.scaleSTLGeomAboutOriginAlongZ(zScale);
    if (!success) {
        writeWarningToLog("CoreFunction: Could not create mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }


    vox.dumpVoxelDataBinary(voxFile.c_str());
    for (int part = 0; part < vox.getNumParts(); part++)
        topopt.setIsotropicMaterialPropertiesOfPart(part,materialProperties,liveVar.materialName.c_str());

    if (!topopt.readVoxelFileBinary(voxFile.c_str())){
        writeWarningToLog("CoreFunction: Could not read mesh... ");
        liveVar.MESH_APPLIED = false;
        return;
    }


    // for multiple parts, to avoid gaps, swap elements near boundary from non-design to design
    if (topopt.getNumberOfParts() > 1){
        writeMessageToLog("Swapping elements near boundary");
        int originalPart,nodes[8],node;
        double pt[3];
        bool foundPart;
        for (int elem = 0; elem < topopt.getNumElems(); elem++) {
            originalPart = topopt.getPartIdOfElem(elem);
            if (!topopt.shouldPartBeOptimized(originalPart)){// consider non-design elements
                if (!topopt.isElemInInteriorOfPart(elem,originalPart)){// consider only boundary elements
                    topopt.getElemNodes(elem,nodes);
                    for (int m =0; m < 8; m++){
                        node = nodes[m];
                        topopt.getNodeXYZ(node,pt);
                        foundPart = false;
                        for (int part = 0; part < topopt.getNumberOfParts(); part++){
                            if (vox.isPointInsidePart(pt,part)){
                               foundPart = true;
                               if ((part !=originalPart) && (topopt.shouldPartBeOptimized(part)))
                                    topopt.setPartIdOfElem(part,elem);
                            }
                            if (foundPart) break;
                        }
                    }
                }
            }
        }
    }
    liveVar.MESH_APPLIED = true;
    remove(voxFile.c_str());

    Timer timer;

    timer.start();
    writeMessageToLog("Computing graph from voxel mesh ");
    topopt.computeLatticeGraphFromVoxelMesh((LATTICETYPE) latticeTypeCombo->currentIndex());
    writeMessageToLog("Graph computed; time taken (secs):  %.2f ", timer.getElapsedTimeInSec());

    //now scale the lattice graph
    topopt.scaleLatticeGraph(xScale,yScale,zScale);
    topopt.setLatticeGeneratorRadiusOfAllStruts(trunkRadius);// this is for FEA
    coreFunctions.changeTopOptBC();// to apply BC on voxel nodes this is for FEA

    // for multiple parts, suppress edges outside
    if (topopt.getNumberOfParts() > 1){
        double pt[3],part;
        for (int node = 0; node < topopt.getLatticeGeneratorNumberOfNodes(); node++){
            topopt.getLatticeGeneratorXYZOfTrussNode(node,pt);
            topopt.setLatticeGeneratorParentBodyOfTrussNode(node,0);//default
            for (part = 1; part < topopt.getNumberOfParts(); part++){// check from 1 onwards
                if (vox.isPointInsidePart(pt,part)){
                    topopt.setLatticeGeneratorParentBodyOfTrussNode(node,part);
                    break;
                }
            }
        }
        int part0,part1,edgeNodes[2];
        for (int edge = 0; edge < topopt.getLatticeGeneratorNumberOfStruts(); edge++){
            topopt.getLatticeGeneratorNodesOfStrut(edge,edgeNodes);
            part0 = topopt.getLatticeGeneratorParentBodyOfTrussNode(edgeNodes[0]);
            part1 = topopt.getLatticeGeneratorParentBodyOfTrussNode(edgeNodes[1]);
            if (!topopt.shouldPartBeOptimized(part0) && !topopt.shouldPartBeOptimized(part1)){
                topopt.suppressLatticeGeneratorStrut(edge);
            }
        }
    }
    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());

    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(trunkRadius);// this is for computing 3D shape
    triwires.setUnion(true);

    writeMessageToLog("Creating lattice structure ");
    timer.start();
    triwires.triangulate();
    double tend =timer.getElapsedTimeInSec();
    writeMessageToLog("Time taken (secs) %.5f ",tend );
    writeMessageToLog("#Beams: %d",triwires.getNumberOfBeams());
    writeMessageToLog("#Triangles: %d", triwires.getNumberOfTriangles());
    writeMessageToLog("Done creating lattice structure ");
    if (triwires.getNumberOfTriangles() > 2e6){
        writeWarningToLog("Model size too large to import!");
    }
    else {
        writeMessageToParetoLog("Saving %s", stlFile.c_str());
        triwires.saveAs(stlFile);
        voxTemp.deleteCurrentModel();
        voxTemp.readSTLGeom(stlFile.c_str(),0, false,NULL,false);
        liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
        liveVar.displayField = DISPLAY_FIELD_NONE;
    }
    viewer->updateGL();
    closeLatticeStructureDialog();
}

/////////////////////////////////////////////////////
void LatticeStructureDialog::updateCombOption()
{
    /*
    if (combOptionCheckBox->isChecked()){
        combSpacingSpinBox->setEnabled(true);
        combRadiusSpinBox->setEnabled(true);
    }
    else {
        combSpacingSpinBox->setEnabled(false);
        combRadiusSpinBox->setEnabled(false);
    }
    */
}
/////////////////////////////////////////////////////
void LatticeStructureDialog::closeLatticeStructureDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void LatticeStructureDialog::makeConnections()
{
    PushButton(createLatticePushButton, [=]{ createLatticeDesign();});
    PushButton( save3DOffFilePushButton, [=]{ save3DOffFileDialog();});
    PushButton(closePushButton,[=]{closeLatticeStructureDialog();});
}
/////////////////////////////////////////////////////
