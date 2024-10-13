#include "LatticeHexmesh.hpp"
#include "ui_latticeHexmeshDialog.hpp"
#include <iostream>
#include "CoreFunctions.hpp"
extern Voxelizer voxTemp;
extern ParetoApplication       topopt;

extern MeshPtr readGraph( const string &filename);
extern void buildTube( const EdgePtr &edge);
extern void buildJunction( const NodePtr &vtx);
extern MeshPtr accumulateHexMesh( const MeshPtr &graph);
///////////////////////////////////////////////////////////////////////////////

LatticeHexmeshDialog :: LatticeHexmeshDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
///////////////////////////////////////////////////////////////////////////////
LatticeHexmeshDialog :: ~LatticeHexmeshDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void LatticeHexmeshDialog :: createHexmesh()
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

    auto  graph  = readGraph( latticeGraphFile.c_str() );
    for( auto v: graph->nodes )
        buildJunction(v);

    for( auto e: graph->edges )
        buildTube(e);

    auto hexmesh = accumulateHexMesh(graph);
    hexmesh->saveAs("hexmesh.off");
    hexmesh->saveAsParetoMesh("hexmesh.mesh");
    topopt.readParetoMeshFile("hexmesh.mesh");

    liveVar.MESH_APPLIED = true;
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.plotAxis = false;
    closeDialog();
}

///////////////////////////////////////////////////////////////////////////////
void LatticeHexmeshDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void LatticeHexmeshDialog :: makeConnections()
{

    PushButton( createHexmeshPushButton, [=]{createHexmesh();});
    PushButton( closePushButton, [=]{closeDialog(); });
}

///////////////////////////////////////////////////////////////////////////////
