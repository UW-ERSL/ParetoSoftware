#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <vector>
#include <set>
#include <iostream>
#include <limits>
#include <fstream>
#include <math.h>
#include <assert.h>

using namespace std;

typedef std::array<double,3> Array3D;
typedef std::array<int,2>    Array2I;

struct Node
{
    int id;
    Array3D xyz;
};

vector<vector<Node>> nodes;
set<Array2I> edges;

///////////////////////////////////////////////////////////////////////////////

void genPoints( double org, double len, double z, int n, vector<Node> &p)
{
    double dl = len/(double)(n-1);

    p.resize(n*n);

    int index = 0;
    Array3D xyz;
    for( int j = 0; j < n; j++) {
        for( int i = 0; i < n; i++) {
            xyz[0] = org + i*dl;
            xyz[1] = org + j*dl;
            xyz[2] = z;
            p[index].xyz = xyz;
            index++;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////

void Connect( const vector<Node> &upper, const vector<Node> &lower)
{
    int index;
    Array2I newedge;

    for( int i = 0; i < upper.size(); i++) {
        double mindist = numeric_limits<double>::max();
        for( int j = 0; j < lower.size(); j++) {
            double dx = upper[i].xyz[0] - lower[j].xyz[0];
            double dy = upper[i].xyz[1] - lower[j].xyz[1];
            double dz = upper[i].xyz[2] - lower[j].xyz[2];
            double dl = dx*dx + dy*dy + dz*dz;
            if( dl < mindist ) mindist = dl;
        }
        for( int j = 0; j < lower.size(); j++) {
            double dx = upper[i].xyz[0] - lower[j].xyz[0];
            double dy = upper[i].xyz[1] - lower[j].xyz[1];
            double dz = upper[i].xyz[2] - lower[j].xyz[2];
            double dl = dx*dx + dy*dy + dz*dz;
            if( fabs(dl-mindist) < 0.0001) {
                newedge[0] = min(upper[i].id, lower[j].id);
                newedge[1] = max(upper[i].id, lower[j].id);
                edges.insert(newedge);
            }
        }
    }

    for( int i = 0; i < lower.size(); i++) {
        double mindist = numeric_limits<double>::max();
        for( int j = 0; j < upper.size(); j++) {
            double dx = upper[j].xyz[0] - lower[i].xyz[0];
            double dy = upper[j].xyz[1] - lower[i].xyz[1];
            double dz = upper[j].xyz[2] - lower[i].xyz[2];
            double dl = dx*dx + dy*dy + dz*dz;
            if( dl < mindist ) mindist = dl;
        }

        for( int j = 0; j < upper.size(); j++) {
            double dx = upper[j].xyz[0] - lower[i].xyz[0];
            double dy = upper[j].xyz[1] - lower[i].xyz[1];
            double dz = upper[j].xyz[2] - lower[i].xyz[2];
            double dl = dx*dx + dy*dy + dz*dz;
            if( fabs(dl-mindist) < 0.0001) {
                newedge[0] = min(lower[i].id, upper[j].id);
                newedge[1] = max(lower[i].id, upper[j].id);
                edges.insert(newedge);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    if( argc != 3) {
        cout << "Usages: " << argv[0] << " ntop nlayers" << endl;
        cout << "ntop (>2)    : Size of grid NtopXNtop on the upper plane" << endl;
        cout << "nlayers (>2) : Size of layers in the voxels" << endl;
        return 1;
    }
	

    int ntop    = atoi( argv[1] );
    int nlayers = atoi( argv[2] );

    assert( nlayers < ntop-1);

    double dz = 1.0 /(double)(nlayers-1);
    nodes.resize(nlayers);

    int globalID = 0;
    genPoints( 0.0, 1.0, 1.0, ntop, nodes[0] );
    for( int j = 0; j < nodes[0].size(); j++)
        nodes[0][j].id = globalID++;

    int nx;
    for( int i = 1; i < nlayers-1; i++) {
        int nx = ntop - i;
        genPoints( 0.1, 0.8, (1.0-i*dz), nx, nodes[i] );
        for( int j = 0; j < nodes[i].size(); j++)
            nodes[i][j].id = globalID++;
    }

    genPoints( 0.0, 1.0, 0.0, 2, nodes[nlayers-1] );
    for( int j = 0; j < nodes[nlayers-1].size(); j++)
        nodes[nlayers-1][j].id = globalID++;

    for( int i = 0; i < nlayers-1; i++) Connect( nodes[i], nodes[i+1] );

    cout << globalID << endl;

    Array2I newedge;

    newedge[0] = 0;
    newedge[1] = globalID-4;
    edges.insert( newedge);

    newedge[0] = ntop-1;
    newedge[1] = globalID-3;
    edges.insert( newedge);

    newedge[0] = ntop*ntop - 1;
    newedge[1] = globalID-1;
    edges.insert( newedge);

    newedge[0] = ntop*ntop - ntop;
    newedge[1] = globalID-2;
    edges.insert( newedge);

    newedge[0] = globalID -4;
    newedge[1] = globalID -3;
    edges.insert( newedge);

    newedge[0] = globalID -3;
    newedge[1] = globalID -1;
    edges.insert( newedge);

    newedge[0] = globalID -1;
    newedge[1] = globalID -2;
    edges.insert( newedge);

    newedge[0] = globalID -2;
    newedge[1] = globalID -4;
    edges.insert( newedge);


	double scale = 10.0;
    cout << "Info: Graph created " << endl;
    ofstream ofile("comb.off");

    ofile << "OFF" << endl;
    int nCount = 0;
    for( int i = 0; i < nodes.size(); i++)
        nCount += nodes[i].size();

    ofile <<  nCount << "  0  " << edges.size() << endl;
    for( int i = 0; i < nlayers; i++) {
        for( int j = 0; j < nodes[i].size(); j++)
            ofile << scale*nodes[i][j].xyz[0] << " " << scale*nodes[i][j].xyz[1] << " " << scale*nodes[i][j].xyz[2] << endl;
    }
    for( auto edge: edges)
        ofile << edge[0] << " " << edge[1] << endl;

    return 0;
}



