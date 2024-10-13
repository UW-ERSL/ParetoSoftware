#include "TriangulateWires.hpp"
#include "ui_latticeAnalysisDialog.hpp"
#include <iostream>
#include "CoreFunctions.hpp"
extern Voxelizer voxTemp;

///////////////////////////////////////////////////////////////////////////////

LatticeAnalysisDialog :: LatticeAnalysisDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
///////////////////////////////////////////////////////////////////////////////
LatticeAnalysisDialog :: ~LatticeAnalysisDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: readTetMeshFEAResultAndResizeBeams()
{
    TriangulateWires triwires;
    // load user defined graph file in .off format
    QString qstr = QFileDialog::getOpenFileName(this,
                                                *new QString("Select Nodal Stress File "),
                                                QString(""),
                                                *new QString( "Nodal Stress File (*.txt)"));

    string nodalStressFile = qstr.toUtf8().constData();
    if (nodalStressFile.empty()){
        return;
    }

    ifstream ifile(nodalStressFile, ios::in);
    if( ifile.fail() ) {
        cout << "Error: Can't open input file " << nodalStressFile << " " << endl;
        return ;
    }
    int nNodes;
    ifile >> nNodes;
    if( nNodes <= 0) {
        cout << "Error: nNodes must be positive " << endl;
        return;
    }
    if (nNodes != topopt.getLatticeGeneratorNumberOfNodes()){
        cout << "Error: nNodes does not match lattice nodes " << endl;
        return;
    }
    double sigma,sigmaMax;
    vector<double> nodalStress;
    nodalStress.resize(nNodes);
    sigmaMax = 0;
    for( size_t i = 0; i < nNodes; i++) {
        ifile >> sigma;
        nodalStress[i] = sigma;
        sigmaMax = MAX(sigmaMax,sigma);
    }
    ifile.close();

    int nodes[2];
    double sigmaAverage;
    double defaultRadius = 0.00025;//default
    topopt.setLatticeGeneratorRadiusOfAllStruts(defaultRadius);
    vector<double> beamRadius;
    double cutoff;
    beamRadius.resize(topopt.getLatticeGeneratorNumberOfStruts());

    cutoff = 0.175;
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
        topopt.getLatticeGeneratorNodesOfStrut(beam,nodes);
        sigmaAverage = (nodalStress[nodes[0]] + nodalStress[nodes[1]])/2.0;
        beamRadius[beam] = 2*sigmaAverage/sigmaMax*defaultRadius;
        topopt.setLatticeGeneratorradiusOfTrunkStrut(beam,beamRadius[beam]);
        if (sigmaAverage/sigmaMax < cutoff)
            topopt.suppressLatticeGeneratorStrut(beam);
    }

    string latticeGraphFile("optimizedLatticeGraphFile.off");
    string latticeGeometryFile("optimizedLatticeGeometryFile.stl");
    int polySides = 6;

    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());
    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(defaultRadius);
    int actualNumBeams;
    actualNumBeams = 0;
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         if (topopt.isStrutSuppressed(beam)) continue;
         triwires.setBeamRadius(actualNumBeams,beamRadius[beam]);
         actualNumBeams++;
    }
    triwires.setUnion(true);
    triwires.triangulate();

    triwires.saveAs(latticeGeometryFile );
    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
    liveVar.modelScale = voxTemp.getModelScale();
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();

}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: transferStructuralLoads()
{
    // transfer loads from 3D geometry to lattice frame.
    //Algorithm: for each lattice node, find the closest triangle
    // If sufficient close by, transfer loads and restraints
    double pt[3],distance,maxDistance,bcValueOfTri[3],ptClosest[3];
    double forcePerNode[3],totalForce;
    int bcSet,bcTypeOfTri[3];
    topopt.resetLatticeFrameFEA();
    maxDistance = 0.001*vox.getModelScale();
    bcSet = 0;

    // for force value, we need to compute the number of nodes first
    int nNodesWithForce;
    nNodesWithForce = 0;

    totalForce = 100;// hard coded;

    for (int triGroup = 1; triGroup <= vox.getNumTriGroups(); triGroup++){// skip group 0
        for (int node = 0; node < topopt.getLatticeGeneratorNumberOfNodes(); node++){
            topopt.getLatticeGeneratorXYZOfTrussNode(node,pt);
            distance = vox.distanceOfPointToSTLGeomTriGroup(pt,triGroup,ptClosest);
            if (distance < maxDistance){
                for (int j =0; j < 3; j++) {
                    bcTypeOfTri[j] = liveVar.BCType[bcSet][triGroup][j];
                    bcValueOfTri[j] = liveVar.BCValue[bcSet][triGroup][j];
                }
                if (bcTypeOfTri[0] == ELASTICITY_NEUMANN)
                    nNodesWithForce++;
            }
        }
    }
    forcePerNode[0]  = forcePerNode[1]  = forcePerNode[2] = 0;
    forcePerNode[1] =  -totalForce/nNodesWithForce;//hard coded
    for (int triGroup = 1; triGroup <= vox.getNumTriGroups(); triGroup++){// skip group 0
        for (int node = 0; node < topopt.getLatticeGeneratorNumberOfNodes(); node++){
            topopt.getLatticeGeneratorXYZOfTrussNode(node,pt);
            distance = vox.distanceOfPointToSTLGeomTriGroup(pt,triGroup,ptClosest);
            if (distance < maxDistance){
                for (int j =0; j < 3; j++) {
                    bcTypeOfTri[j] = liveVar.BCType[bcSet][triGroup][j];
                    bcValueOfTri[j] = liveVar.BCValue[bcSet][triGroup][j];
                }
                if (bcTypeOfTri[0] == ELASTICITY_DIRICHLET)
                    topopt.fixLatticeNodeFrame(node);
                else if (bcTypeOfTri[0] == ELASTICITY_NEUMANN)
                    topopt.applyLatticeNodalForce(node, forcePerNode);
            }
        }
    }
    liveVar.plotElasticityBC = true;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: latticeFrameStaticFEA()
{
    if (topopt.getLatticeNumberOfFixedNodes() == 0){
        writeWarningToLog("Loads not transferred");
        return;
    }
    if (!topopt.assembleLatticeFrameFEA()) {
        writeWarningToLog("Could not assembly Lattice stiffness matrix");
    }
    if (!topopt.solveLatticeFrameFEA()){
        writeWarningToLog("Could not solve Lattice FEA");
    }
    parentWidget()->show();
    liveVar.displayGeom = DISPLAY_GEOM_LATTICE;// lattice graph
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
    liveVar.geomTransparent = true;
    liveVar.plotElasticityBC = true;
    liveVar.plotSummaryResults = true;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: latticeFrameModalFEA()
{
    if (topopt.getLatticeNumberOfFixedNodes() == 0){
        writeWarningToLog("Loads not transferred");
        return;
    }
    if (!topopt.isLatticeFrameFEASolved()) {
        writeWarningToLog("Please solve static FEA first");
        return;
    }

    if (!topopt.assembleLatticeMassMatrix()) {
        writeWarningToLog("Could not assembly Lattice mass matrix");
        return;
    }

    if (!topopt.solveLatticeModalFEA()){
        writeWarningToLog("Could not solve Lattice Modal FEA");
    }
    parentWidget()->show();
    liveVar.displayGeom = DISPLAY_GEOM_LATTICE;// lattice graph
    liveVar.displayField = DISPLAY_FIELD_MODAL;
    liveVar.plotElasticityBC = false;
    liveVar.geomTransparent = true;
    liveVar.plotSummaryResults = true;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: suppressThinBeams()
{
    int polySides = 6;
    double maxBeamRadius,beamRadius,defaultRadius;
    double cutoff;
    string latticeGraphFile("optimizedLatticeGraphFile.off");
    string latticeGeometryFile("optimizedLatticeGeometryFile.stl");
    TriangulateWires triwires;


    latticeGeometryFile = liveVar.projectDirectory + liveVar.modelName + string("_latticeCleaned.stl");
    if (topopt.isLatticeFrameFEASolved() == 0){
        writeWarningToLog("Please solve Frame FEA first");
        return;
    }
    //find the max beam radius
    maxBeamRadius = 0;
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         beamRadius = topopt.getLatticeGeneratorRadiusOfTrunkStrut(beam);
         maxBeamRadius = MAX(maxBeamRadius,beamRadius);
    }

    // first unsuppress all beams
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++)
        topopt.unSuppressLatticeGeneratorStrut(beam);

    // identify beams that need to be suppressed
    cutoff = beamRadiusCutoffSpinBox->value();
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         beamRadius = topopt.getLatticeGeneratorRadiusOfTrunkStrut(beam);
         if (beamRadius < cutoff*maxBeamRadius)
             topopt.suppressLatticeGeneratorStrut(beam);
    }
    // remove all hanging beams

    defaultRadius = 0.5*maxBeamRadius;
    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());
    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(defaultRadius);
    int actualNumBeams;
    actualNumBeams = 0;
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         if (topopt.isStrutSuppressed(beam)) continue;
         beamRadius = topopt.getLatticeGeneratorRadiusOfTrunkStrut(beam);
         triwires.setBeamRadius(actualNumBeams,beamRadius);
         actualNumBeams++;
    }
    triwires.setUnion(true);
    triwires.triangulate();
    triwires.saveAs(latticeGeometryFile );
    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
    liveVar.modelScale = voxTemp.getModelScale();
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: latticeFrameOptimization()
{
    int polySides = 6;
    double beamRadius,defaultRadius,maxBeamRadius;
    TriangulateWires triwires;
    string latticeGraphFile("optimizedLatticeGraphFile.off");
    string latticeGeometryFile;

    latticeGeometryFile = liveVar.projectDirectory + liveVar.modelName + string("_latticeOptimized.stl");
    if (topopt.isLatticeFrameFEASolved() == 0){
        writeWarningToLog("Please solve Frame FEA first");
        return;
    }

    topopt.optimizeLatticeStructureMMA();

    cout << "Max displacement " << topopt.getLatticeDeltaMax() << endl;
    //find the max beam radius
    maxBeamRadius = 0;
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         beamRadius = topopt.getLatticeGeneratorRadiusOfTrunkStrut(beam);
         maxBeamRadius = MAX(maxBeamRadius,beamRadius);
    }

    defaultRadius = 0.5*maxBeamRadius;
    topopt.saveLatticeGraphToOffFile(latticeGraphFile.c_str());
    triwires.readGraph(latticeGraphFile);
    triwires.setPolySides(polySides);
    triwires.setBeamRadius(defaultRadius);
    for (size_t beam = 0; beam < topopt.getLatticeGeneratorNumberOfStruts(); beam++){
         beamRadius = topopt.getLatticeGeneratorRadiusOfTrunkStrut(beam);
         triwires.setBeamRadius(beam,beamRadius);
    }
    triwires.setUnion(true);
    triwires.triangulate();
    triwires.saveAs(latticeGeometryFile );
    voxTemp.deleteCurrentModel();
    voxTemp.readSTLGeom(latticeGeometryFile.c_str(),0, false,NULL,false);
    liveVar.modelScale = voxTemp.getModelScale();
    liveVar.displayGeom = DISPLAY_GEOM_OTHER;// lattice loaded via voxTemp
    liveVar.displayField = DISPLAY_FIELD_NONE;
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeAnalysisDialog :: makeConnections()
{
    PushButton( readTetMeshNodalStressPushButton, [=]{readTetMeshFEAResultAndResizeBeams();});
    PushButton( transferLoadsPushButton, [=]{transferStructuralLoads();});
    PushButton( staticAnalysisPushButton, [=]{latticeFrameStaticFEA();});
    PushButton( modalAnalysisPushButton, [=]{latticeFrameModalFEA();});
    PushButton( optimizePushButton, [=]{latticeFrameOptimization();});
    PushButton( removeThinBeamsPushButton, [=]{suppressThinBeams();});
    PushButton( closePushButton, [=]{closeDialog(); });
}
///////////////////////////////////////////////////////////////////////////////
