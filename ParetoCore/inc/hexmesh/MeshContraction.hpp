#pragma once

#include <vector>
#include <array>
#include <iostream>
using namespace std;
typedef std::array<int,3> Point3I;
typedef std::array<double,3> Point3D;


#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

class JMeshContraction
{
   typedef Eigen::SparseMatrix<double>  SpMat;

   struct MyEdge
   {
      void setNodes( int v0, int v1) {
           nodes[0] = min(v0,v1);
           nodes[1] = max(v0,v1);
      }
      bool operator == ( const MyEdge &rhs) {
           if( rhs.nodes[0] != nodes[0] ) return 0;
           if( rhs.nodes[1] != nodes[1] ) return 0;
           return 1;
      }
      int faces[2];
      int nodes[2];
    };
      
    struct MyMesh {
        vector<Point3I>  faces;       // Triangle Connectivity...
        vector<Point3D>  nodes;       // Node Coordinates
        vector<MyEdge>   edges;
        vector<double>   faceArea[2]; // Unsigned area of each face
        vector<double>   nodeArea[2]; // One Ring area of nodes
        vector<double>   WL;          // Contraction weight ...
        vector<double>   WH;          // Shape weight...
        double surfaceArea[3];  // 0: in the begining: 1 previous step: 2 current step.
        void   makeDelaunay();
        bool   isDelaunay(const MyEdge &e) const;
    };

public:
	void getNodePt(int node, Point3D &pCoord);
    void setMesh(vector<Point3I> &triangles, vector<Point3D> &pCoord);
    void solve(int nIterations = 1);
private:
    MyMesh   lmesh;
    SpMat    L;
    Eigen::VectorXd  b;

    double smallAngle, largeAngle;

    double getLength(const Point3D &p0, const Point3D &p1);
    void getIntrinsicDelaunay();
    void setEdgesWeight( size_t id);
    void setArea();
    void buildLMatrix();
    void initWeights();
};
