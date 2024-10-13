#pragma once 

#include <fstream>
#include <iostream>
#include <limits>
#include <assert.h>
#include <math.h>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <string.h>
#include <algorithm>
#include <array>

#include "ConvexHull/QuickHull.hpp"
#include "Quaternion.hpp"

#ifdef _WIN32
const double M_PI = 3.14159265358979323846;
#endif

////////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void cross_product( const std::array<T,3> &A, const std::array<T,3> &B, std::array<T,3> &C)
{
    C[0] = A[1]*B[2] - A[2]*B[1];
    C[1] = A[2]*B[0] - A[0]*B[2];
    C[2] = A[0]*B[1] - A[1]*B[0];
}
////////////////////////////////////////////////////////////////////////////////////

template<class T>
inline double dot_product( const std::array<T,3> &A, const std::array<T,3> &B)
{
    return A[0]*B[0] + A[1]*B[1] + A[2]*B[2];
}

////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline double magnitude( const std::array<T,3> &A )
{
    return sqrt( A[0]*A[0] + A[1]*A[1] + A[2]*A[2] );
}
////////////////////////////////////////////////////////////////////////////////////
template<class T>
inline double length( const std::array<T,3> &A, const std::array<T,3> &B)
{
    double dx = A[0] - B[0];
    double dy = A[1] - B[1];
    double dz = A[2] - B[2];
    return sqrt(dx*dx +  dy*dy + dz*dz);
}

////////////////////////////////////////////////////////////////////////////////////

template<class T>
inline double getVecAngle( const std::array<T,3> &A, const std::array<T,3> &B)
{
    double AB = dot_product(A,B);
    double Am = magnitude(A);
    double Bm = magnitude(B);

    if( Am < 1.0E-15 || Bm < 1.0E-15) return 0.0;

    double x = AB/(Am*Bm);

    if( x > 1.0) x = 1.0;
    if( x < -1.0) x = -1.0;
    return acos(x);
}
////////////////////////////////////////////////////////////////////////////////////

typedef std::array<double,2>  Array2D;
typedef std::array<double,3>  Array3D;
typedef std::array<size_t,2>  Array2I;
typedef std::array<size_t,3>  Array3Int;

class TriangulateWires
{
    Array3D getVector( const Array3D &head, const Array3D &tail)
    {
        Array3D v;
        v[0] = head[0] - tail[0];
        v[1] = head[1] - tail[1];
        v[2] = head[2] - tail[2];

        return v;
    }

    Array3D getTriangleNormal( const Array3D &p0, const Array3D &p1, const Array3D &p2)
    {
        Array3D C;
        auto A = getVector(p1,p0);
        auto B = getVector(p2,p0);
        cross_product(A,B,C);
        double mag = magnitude(C);
        if( mag > 1.0E-10) {
            C[0] /= mag;
            C[1] /= mag;
            C[2] /= mag;
        }
        return C;
    }

    struct Face
    {
        Face() {}
        Face( const Array3Int &v, size_t g, int e) {
            nodes = v;
            groupID = g;
            entity  = e;
        }
        bool    entity;
        size_t  groupID;
        Array3Int nodes;
    };

    struct TriMesh
    {
        std::vector<Array3D> vCoords;
        std::vector<Face>    faces;

    };

    struct Node
    {
        Array3D  xyz;
        std::set<size_t>  adjEdges;
    };

    struct Junction : public Node
    {
        int      numBoundNodes;
        double   radius;
        double   minRadius,   maxRadius;
        std::vector<size_t>   boundNodes;
        std::vector<Face>     faces;
    };

    struct Edge
    {
        double   length;
        Array2I  nodes;
        int      getOtherNode(int thisnode) const {
            if( nodes[0] == thisnode) return nodes[1];
            if( nodes[1] == thisnode) return nodes[0];
            return -1;
        }
    };

    struct Profile
    {
        double   minAngle;
        int      nearBeam;
        double   minOffset;
        double   minRadius;
        double   radius;
        Array3D  center;
        std::vector<Array3D>  nodes;
    };


    struct Beam : public Edge
    {
        int getProfileID( int id) const {
            if( std::min( nodes[0], nodes[1]) == id) return 0;
            if( std::max( nodes[0], nodes[1]) == id) return 1;
            return -1;
        }
        Profile                profile[2];
        std::vector<Array3Int>   sideTriangles;
        std::vector<Array3Int>   capTriangles[2];
    };

    typedef std::array<Array3D,3> TrianglePoints;

public:
    TriangulateWires() {modelScale= 1;}
    ~TriangulateWires() {}

    // Read the lattice Graph ...
    void readGraph( const std::string &s);

    size_t getNumberOfNodes(void ) {
        return junctions.size();
    }

    size_t getNumberOfBeams(void ) {
        return beams.size();
    }

    void getBeamNodes(int beam, int nodes[2] ) {
        nodes[0] = beams[beam].nodes[0];
        nodes[1] = beams[beam].nodes[1];
    }
    void getNodeLocation(int node, double pt[3] ) {
        const Array3D &p0 = junctions[node].xyz;
        for (int i =0; i < 3; i++)
            pt[i] = p0[i];
    }
    void getBeamCenter(int beam, double pt[3] ) {
        int node1,node2;
        node1 = beams[beam].nodes[0];
        node2 = beams[beam].nodes[1];
        const Array3D &p1 = junctions[node1].xyz;
        const Array3D &p2 = junctions[node2].xyz;
        for (int i =0; i < 3; i++)
            pt[i] = (p1[i]+p2[i])/2;
    }

    double getModelScale( ) {
        return modelScale;
    }
    double getMinEdgeLength() const {
        return minEdgeLength;
    }
    double getMaxEdgeLength() const {
        return maxEdgeLength;
    }
    void getBoundingBox(double boundingBox[3][2]) {
        for (int i = 0; i < 3; i++){
            boundingBox[i][0] = boxMin[i];
            boundingBox[i][1] = boxMax[i];
        }
    }
    void getBoundingBox(double xyzMin[3],double xyzMax[3]) {
        for (int i = 0; i < 3; i++){
            xyzMin[i] = boxMin[i];
            xyzMax[i] = boxMax[i];
        }
    }

    void setBeamRadius( const std::string &f);

    // Read the beam radius if not specified in the input file. It will override the
    // value of beams if it specified in the input file.
    void setBeamRadius( double r);

    void setBeamRadius( size_t beamID, double r) {
        if( beamID < beams.size()) {
            beams[beamID].profile[0].radius = r;
            beams[beamID].profile[1].radius = r;
        }
    }

    double getBeamRadius( size_t beamID) {
        if( beamID < beams.size()) {
            double r0 = beams[beamID].profile[0].radius;
            double r1 = beams[beamID].profile[1].radius;
            return 0.5*(r0+r1);
        }
        return 0;
    }

    void setBeamRadius( const std::vector<double> &r);

    void setPolySides(int n) {
        polySides = std::max(3,n);
    }

    void setUnion( bool u) {
        make_union = u;
    }

    void setReduceNodeVolume( bool v) {
        reduceNodeVolume = v;
    }

    void triangulate();

    size_t getNumberOfTriangles() const {
        return trimesh.faces.size();
    }
    void getTriPoints(int tri,double xTri[3],double yTri[3],double zTri[3]);

    // Save the triangulate model in STL file with extension *.stl.
    void saveAs(const std::string &s);

private:
    int  polySides = 3;
    int  numNodesPerBeam;
    bool edgeRadiusAssigned = false;
    bool nodeRadiusAssigned = false;
    bool make_union = true;
    bool reduceNodeVolume = true;
    bool allowBeamThinning = false;
    double uniformBeamRadius = true;
    double minEdgeLength, maxEdgeLength;
    Array3D             boxMin;      // bounding Box min
    Array3D             boxMax;      // bounding Box max
    double      modelScale;

    //Input model ...
    std::vector<Junction> junctions;
    std::vector<Beam>     beams;

    // A single mesh to hold all the triangles and nodes. (Output).
    TriMesh trimesh;

    // Read methods..
    void readMTL( const std::string &f);
    void readOFF( const std::string &f);

    void  processNode( size_t id);
    void  setConvexHullRadius( size_t id);
    int   setConvexHull( size_t id);
    void  shrinkNode( size_t id);

    void beamCap( size_t id);
    void buildSideTriangles();
    void buildCapTriangles();
    void buildCapTriangles(size_t edgeID, size_t nodeID);

    void alignAlong(std::vector<Array3D> &points, const Array3D &srcVec, const Array3D &dstVec);
    bool isNormalProfile(  const std::vector<Array3D> &points, const Array3D &axis);
    bool checkIntersections(int nodeID);
    void setProfileCenter(size_t edgeID, size_t nodeID);
    void setProfile(size_t edgeID, size_t nodeID);
    void setProfile(size_t edgeID);
    void checkBeamsIntersections();
    void scale( std::vector<Array3D> &p);
    void checkAndCorrectBeamRadius(void);
    void refineMesh(TriMesh &nodemesh, size_t newnode);
};
////////////////////////////////////////////////////////////////////////////////////

