#include "DelaunaySurface.h"
#include <deque>
#include <cassert>
#include <fstream>
#include <iostream>
#include "trilib.hpp"
#include "ParetoApplication.h"
using namespace std;
extern ParetoApplication topopt;

////////////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: setPoints( const vector<Array3D> &pnts)
{
    inPoints = pnts;
}
////////////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: setPoints( const vector<Array3D> &pnts, const vector<Array3D> &pnorms)
{
    assert( pnts.size() == pnorms.size() );

    inPoints = pnts;
    pNormals = pnorms;
}
////////////////////////////////////////////////////////////////////////////////////
bool DelaunaySurface :: isOutside( const CellPtr &tet) const
{
    Array3D center  = {0.0, 0.0, 0.0};
    for( int i = 0; i < 4; i++) {
        center[0] += tet->nodes[i]->xyz[0];
        center[1] += tet->nodes[i]->xyz[1];
        center[2] += tet->nodes[i]->xyz[2];
    }
    center[0] *= 0.25;
    center[1] *= 0.25;
    center[2] *= 0.25;

    // check if the center of tet is outside mesh
    if (topopt.findNonVoidElemClosestTo(center[0], center[1], center[2], 0.9) < 0)// the tet is outside
        return true;

    // check if any of the edge centers are outside mesh
    int edgeList[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
    Array3D edgeCenter  = {0.0, 0.0, 0.0};
    for (int edge = 0; edge < 6; edge++){
        edgeCenter[0] = edgeCenter[1] = edgeCenter[2] = 0.0;

        edgeCenter[0] += tet->nodes[edgeList[edge][0]]->xyz[0];
        edgeCenter[0] += tet->nodes[edgeList[edge][1]]->xyz[0];
        edgeCenter[0] /=2.0;

        edgeCenter[1] += tet->nodes[edgeList[edge][0]]->xyz[1];
        edgeCenter[1] += tet->nodes[edgeList[edge][1]]->xyz[1];
        edgeCenter[1] /=2.0;

        edgeCenter[2] += tet->nodes[edgeList[edge][0]]->xyz[2];
        edgeCenter[2] += tet->nodes[edgeList[edge][1]]->xyz[2];
        edgeCenter[2] /=2.0;

        if (topopt.findNonVoidElemClosestTo(edgeCenter[0], edgeCenter[1], edgeCenter[2], 1.25) < 0)// the tet is outside
            return true;
    }


    double avgDtprd;
    Array3D  A;
    //next check against normal at points
    avgDtprd = 0;
    for( int i = 0; i < 4; i++) {
        A[0] = center[0] - tet->nodes[i]->xyz[0];
        A[1] = center[1] - tet->nodes[i]->xyz[1];
        A[2] = center[2] - tet->nodes[i]->xyz[2];

        double dl = sqrt(A[0]*A[0] + A[1]*A[1] + A[2]*A[2] );
        A[0] /= dl;
        A[1] /= dl;
        A[2] /= dl;
        Array3D B = pNormals[tet->nodes[i]->id];
        avgDtprd += dot_product(A,B);
        //if (dot_product(A,B) > 0) return true;
    }
    avgDtprd /= 4.0;
    if (avgDtprd > 0.2) return true;
    return false;// tet is in
}

////////////////////////////////////////////////////////////////////////////////////

void DelaunaySurface :: extractSurface(double featureSize)
{
    for( auto cell : tetmesh.cells)
        if( isOutside(cell) ) cell->active = 0;

    buildMeshTopology();

    // eliminate tets with boundary faces whose edge
    // lengths are  large compared to feature size


    vector<CellPtr> isolated;
    for( auto cell : tetmesh.cells) {
        int nCount = 0;
        if( cell->active) {
            for( int i = 0; i < 4; i++) {
                auto f0 = cell->getFaceAt(i);
                if( f0->isBoundary() ) nCount++;
            }
            if( nCount == 4)
                isolated.push_back(cell);
        }
    }

    cout << "Info: #isolated tets: " << isolated.size() << endl;
    for( auto cell : isolated)  cell->active = 0;

    int nOut = 0;
    for( auto cell : tetmesh.cells)
        if( cell->active) nOut++;

    newTriangles.clear();
    Array3D A, B;
    for( auto face: tetmesh.faces) {
        if( face->isBoundary() ) {
            int v0 = face->nodes[0]->id;
            int v1 = face->nodes[1]->id;
            int v2 = face->nodes[2]->id;
            double maxEdgeLength;
            maxEdgeLength = DIST3D(inPoints[v0],inPoints[v1]);
            maxEdgeLength = MAX(maxEdgeLength,DIST3D(inPoints[v0],inPoints[v2]));
            maxEdgeLength = MAX(maxEdgeLength,DIST3D(inPoints[v1],inPoints[v2]));
            if (maxEdgeLength > 3*featureSize){
                continue;
            }
            A     = normal( inPoints[v0], inPoints[v1], inPoints[v2]);
            B[0]  = (pNormals[v0][0] + pNormals[v1][0] + pNormals[v2][0])/3.0;
            B[1]  = (pNormals[v0][1] + pNormals[v1][1] + pNormals[v2][1])/3.0;
            B[2]  = (pNormals[v0][2] + pNormals[v1][2] + pNormals[v2][2])/3.0;

            if( dot_product(A,B) > 0.0)
                newTriangles.push_back( {v0,v1,v2} );
            else
                newTriangles.push_back( {v0,v2,v1} );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: readTetmesh(const char *nodeFile,
                                    const char *eleFile)
{
    ifstream ifile(nodeFile, ios::in);

    int numNodes, ndim, nattrib, nb;
    int id;

    ifile >> numNodes >> ndim >> nattrib >> nb;
    assert( ndim == 3);
    assert( nattrib == 0);
    assert( nb  == 0);

    tetmesh.nodes.resize(numNodes);
    for( int i = 0; i < numNodes; i++) {
        auto v = std::make_shared<Node>();
        ifile >> id >> v->xyz[0] >> v->xyz[1] >> v->xyz[2];
        v->id  = id;
        assert( id == i);
        tetmesh.nodes[i] = v;
    }
    ifile.close();

    ifile.open(eleFile, ios::in);

    int numElems;

    ifile >> numElems >> ndim >> nb;
    assert( ndim == 4);
    assert( nb   == 0);

    tetmesh.cells.resize( numElems);
    int v0, v1, v2, v3;
    for( int i = 0; i < numElems; i++) {
        ifile >> id >> v0 >> v1 >> v2 >> v3;
        auto cell = make_shared<Cell>();
        cell->nodes[0] = tetmesh.nodes[v0];
        cell->nodes[1] = tetmesh.nodes[v1];
        cell->nodes[2] = tetmesh.nodes[v2];
        cell->nodes[3] = tetmesh.nodes[v3];
        tetmesh.cells[i] = cell;
    }
}

///////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: buildMeshTopology()
{
    for( auto cell : tetmesh.cells) {
        if(cell->active ) {
            for( int i = 0; i < 4; i++) {
                auto f0 = cell->getFaceAt(i);
                assert(f0);
                int err = f0->addCell(cell);
                assert(!err);
            }
        }
    }
    tetmesh.faces.clear();
    for( auto v: tetmesh.nodes) {
        for( auto f: v->faces)
            tetmesh.faces.push_back(f);
    }
}
///////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: saveSurfaceAsOffFile( const string &filename)
{
    ofstream ofile( filename.c_str(), ios::out);
    ofile << "OFF" << endl;
    ofile <<  inPoints.size() << " " << newTriangles.size() << " 0 " << endl;

    for( auto p : inPoints)
        ofile << p[0] << " " << p[1] << " " << p[2] << endl;

    for( auto t : newTriangles)
        ofile << "3 " << t[0] << " " << t[1] << " " << t[2] << endl;
}
///////////////////////////////////////////////////////////////////////////////
void DelaunaySurface :: saveAllTriangles ( const string &filename, double edgeLengthCutoff)
{
    Array3D A, B;
    ofstream ofile( filename.c_str(), ios::out);
    ofile << tetmesh.faces.size() << " " << 0 << endl;
    int counter = 0;

    for( auto face: tetmesh.faces) {
        int v0 = face->nodes[0]->id;
        int v1 = face->nodes[1]->id;
        int v2 = face->nodes[2]->id;
        double maxEdgeLength;
        maxEdgeLength = DIST3D(inPoints[v0],inPoints[v1]);
        maxEdgeLength = MAX(maxEdgeLength,DIST3D(inPoints[v0],inPoints[v2]));
        maxEdgeLength = MAX(maxEdgeLength,DIST3D(inPoints[v1],inPoints[v2]));
        if (maxEdgeLength > edgeLengthCutoff){
            continue;
        }
        A     = normal( inPoints[v0], inPoints[v1], inPoints[v2]);
        B[0]  = (pNormals[v0][0] + pNormals[v1][0] + pNormals[v2][0])/3.0;
        B[1]  = (pNormals[v0][1] + pNormals[v1][1] + pNormals[v2][1])/3.0;
        B[2]  = (pNormals[v0][2] + pNormals[v1][2] + pNormals[v2][2])/3.0;

        if( dot_product(A,B) < 0.0)
            ofile << counter << " " << v0 << " " << v1 << " "  << v2 << endl;
        else
            ofile << counter << " " << v0 << " " << v2 << " "  << v1 << endl;
    }
}
