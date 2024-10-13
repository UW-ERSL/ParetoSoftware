#ifndef LATTICE_HEXMESH_H_
#define LATTICE_HEXMESH_H_

#include <stdio.h>
#include <stdlib.h>

#include <array>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cassert>
#include <limits>
#include <string>
using namespace std;

typedef std::array<double,3> Array3D;
typedef std::array<int,4>    Array4I;
typedef std::array<int,8>    Array8I;

class Node;
typedef std::shared_ptr<Node>  NodePtr;

class Edge;
typedef std::shared_ptr<Edge>  EdgePtr;

class Face;
typedef std::shared_ptr<Face>  FacePtr;

class Mesh;
typedef std::shared_ptr<Mesh>  MeshPtr;

struct Node
{
    size_t   id;             // Unique ID of the node.
    double   radius;         // Radius of the sphere.
    Array3D  xyz;            // Coordinates of the node.
    MeshPtr  hexmesh;        // Hex mesh of the sphere.
    vector<EdgePtr> edges;   // edges attached to this node.
};

struct Edge
{
    Array3D getCenter() const {
        Array3D pc = {0.0, 0.0, 0.0};
        for( int i = 0; i < 2; i++) {
            pc[0] += nodes[i]->xyz[0];
            pc[1] += nodes[i]->xyz[1];
            pc[2] += nodes[i]->xyz[2];
        }
        pc[0] *= 0.50;
        pc[1] *= 0.50;
        pc[2] *= 0.50;
        return pc;
    }

    std::array<NodePtr,2>  nodes;
    MeshPtr  hexmesh;
    std::vector<FacePtr>  profiles[2];   // Used for hexmesh...
};

///////////////////////////////////////////////////////////////////////

struct Face
{
    std::array<NodePtr,4>  nodes;  // Quad-face
    Array3D getCenter() const {
        Array3D pc = {0.0, 0.0, 0.0};
        for( int i = 0; i < 4; i++) {
            pc[0] += nodes[i]->xyz[0];
            pc[1] += nodes[i]->xyz[1];
            pc[2] += nodes[i]->xyz[2];
        }
        pc[0] *= 0.25;
        pc[1] *= 0.25;
        pc[2] *= 0.25;
        return pc;
    }

    bool used_for_hex = 0;

};

///////////////////////////////////////////////////////////////////////

struct Cell
{
    std::array<NodePtr,8>  nodes;

    FacePtr getFaceAt(int dir)
    {
        auto face = std::make_shared<Face>();
        switch( dir)
        {
        case -1:
            face->nodes = { nodes[0], nodes[4], nodes[7], nodes[3] };
            break;
        case +1:
            face->nodes = { nodes[1], nodes[2], nodes[6], nodes[5] };
            break;
        case -2:
            face->nodes = { nodes[0], nodes[1], nodes[5], nodes[4] };
            break;
        case +2:
            face->nodes = { nodes[3], nodes[7], nodes[6], nodes[2] };
            break;
        case -3:
            face->nodes = { nodes[0], nodes[3], nodes[2], nodes[1] };
            break;
        case +3:
            face->nodes = { nodes[4], nodes[5], nodes[6], nodes[7] };
            break;
        }
        return face;
    }
};

typedef std::shared_ptr<Cell>  CellPtr;

///////////////////////////////////////////////////////////////////////

struct Mesh
{
    std::vector<NodePtr> nodes;
    std::vector<EdgePtr> edges;
    std::vector<CellPtr> cells;

    std::vector<NodePtr> boundnodes;
    std::vector<FacePtr> boundfaces;

    void add( const MeshPtr &m) {
        for( auto v: m->nodes) nodes.push_back(v);
        for( auto c: m->cells) cells.push_back(c);
    }

    void saveAs( const string &filename) {

        ofstream ofile( filename.c_str(), ios::out);

        ofile << "OFF" << endl;
        ofile << nodes.size() << " " << cells.size() << " 0 " << endl;

        for( auto v : nodes)
            ofile << v->xyz[0] << " " << v->xyz[1] << " " << v->xyz[2] << endl;

        for( auto c : cells)
            ofile << "8 " << c->nodes[0]->id << " "
                          << c->nodes[1]->id << " "
                      << c->nodes[2]->id << " "
                      << c->nodes[3]->id << " "
                      << c->nodes[4]->id << " "
                      << c->nodes[5]->id << " "
                      << c->nodes[6]->id << " "
                          << c->nodes[7]->id << endl;
    }

    void saveAsParetoMesh( const string &filename) {

        /*
        #Nodes
        nNodes
        0         -7.5          25.          20.
        1         -7.5    21.041666          20.
        2         -7.5    17.083334          20.
        3         -7.5       13.125          20.
        4         -7.5   9.16666698          20.
        5         -7.5   5.20833349          20.
        6         -7.5         1.25          20.
        ...

        #Elems
        nElements
        0  43  44  51  50   0   2   9   8
        1  44  45  52  51   2   3  10   9
        2  45  46  53  52   3   4  11  10
        3  46  47  54  53   4   5  12  11

        ...
        #PseudoDensity (for each element, in or out)
        0.0 1.0 0.0 0.0 1.0 1 0 1 1 1 1 ...

        */

        ofstream ofile( filename.c_str(), ios::out);

        ofile << "#Nodes" << endl;
        ofile << nodes.size()  << endl;

        int counter;
        counter = 0;
        for( auto v : nodes){
            ofile << counter << " " << v->xyz[0] << " " << v->xyz[1] << " " << v->xyz[2] << endl;
            counter++;
        }

        ofile << "#Elems" << endl;
        ofile << cells.size()  << endl;

        counter = 0;
        for( auto c : cells){
            ofile << counter << " "
                  << c->nodes[0]->id << " "
                  << c->nodes[1]->id << " "
                  << c->nodes[2]->id << " "
                  << c->nodes[3]->id << " "
                  << c->nodes[4]->id << " "
                  << c->nodes[5]->id << " "
                  << c->nodes[6]->id << " "
                  << c->nodes[7]->id << endl;
            counter++;
        }
    }
};

#endif
