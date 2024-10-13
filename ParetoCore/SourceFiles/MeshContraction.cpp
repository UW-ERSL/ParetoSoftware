#include "MeshContraction.hpp"

//#include "Eigen/SparseQR"

using namespace Eigen;

////////////////////////////////////////////////////////////////////////////////
void JMeshContraction::initWeights()
{
    setArea();

    lmesh.faceArea[0] = lmesh.faceArea[1];
    lmesh.nodeArea[0] = lmesh.nodeArea[1];

    size_t numNodes = lmesh.nodes.size();
    lmesh.WL.resize(numNodes);
    lmesh.WH.resize(numNodes);

    size_t numFaces = lmesh.faces.size();
    double sum = 0.0;
    for( size_t i = 0; i < numFaces; i++)
        sum += lmesh.faceArea[0][i];

    double avgArea = sum/(double)numFaces;
    double Wl = 1*sqrt(avgArea);

    for( int i = 0; i < numNodes; i++) {
        lmesh.WL[i] = 0.01;
        lmesh.WH[i] = 1;
    }
}
////////////////////////////////////////////////////////////////////////////////

void JMeshContraction::getNodePt( int node, Point3D &pCoords)
{
	for (int i = 0; i < 3; i++)
		pCoords[i] = lmesh.nodes[node][i];
}
////////////////////////////////////////////////////////////////////////////////

void JMeshContraction::setMesh( vector<Point3I> &triConnect, vector<Point3D> &pCoords)
{
    lmesh.faces = triConnect;
    lmesh.nodes = pCoords;
    initWeights();
}

////////////////////////////////////////////////////////////////////////////////
double JMeshContraction:: getLength( const Point3D &a, const Point3D &b)
{
    double dx = a[0] - b[0];
    double dy = a[1] - b[1];
    double dz = a[2] - b[2];
    return sqrt(dx*dx + dy*dy + dz*dz);
}
////////////////////////////////////////////////////////////////////////////////
void JMeshContraction:: setArea()
{
    double sum = 0.0;
    size_t numFaces  = lmesh.faces.size();
    size_t numNodes  = lmesh.nodes.size();

    lmesh.nodeArea[1].resize(numNodes);
    for( size_t i = 0; i < numNodes; i++)
        lmesh.nodeArea[1][i] = 0.0;

    lmesh.faceArea[1].resize(numFaces);
    for( size_t id = 0; id < numFaces; id++) {
        int id0 = lmesh.faces[id][0];
        int id1 = lmesh.faces[id][1];
        int id2 = lmesh.faces[id][2];

        Point3D p0 = lmesh.nodes[id0];
        Point3D p1 = lmesh.nodes[id1];
        Point3D p2 = lmesh.nodes[id2];

        double  a  = getLength(p1,p2);
        double  b  = getLength(p2,p0);
        double  c  = getLength(p0,p1);
        double  s  = 0.5*(a+b+c);
        double tarea = sqrt(s*(s-a)*(s-b)*(s-c));
        sum += tarea;
        lmesh.faceArea[1][id]   = tarea;
        lmesh.nodeArea[1][id0] += tarea;
        lmesh.nodeArea[1][id1] += tarea;
        lmesh.nodeArea[1][id2] += tarea;
    }
    cout << "Total Surface Area " << sum <<endl;
}
////////////////////////////////////////////////////////////////////////////////

void JMeshContraction:: setEdgesWeight( size_t id)
{
    int id0 = lmesh.faces[id][0];
    int id1 = lmesh.faces[id][1];
    int id2 = lmesh.faces[id][2];

    Point3D p0 = lmesh.nodes[id0];
    Point3D p1 = lmesh.nodes[id1];
    Point3D p2 = lmesh.nodes[id2];

    double  a  = getLength(p1,p2);
    double  b  = getLength(p2,p0);
    double  c  = getLength(p0,p1);

    double cosA = (b*b + c*c - a*a)/(2.0*b*c);
    if( cosA >  1.0) cosA =  1.0;
    if( cosA < -1.0) cosA = -1.0;
    double alpha = acos(cosA);

    double cosB = (a*a + c*c - b*b)/(2.0*a*c);
    if( cosB >  1.0) cosB =  1.0;
    if( cosB < -1.0) cosB = -1.0;
    double beta = acos(cosB);

    double cosC = (a*a + b*b - c*c)/(2.0*a*b);
    if( cosC >  1.0) cosC =  1.0;
    if( cosC < -1.0) cosC = -1.0;
    double gamma = acos(cosC);

    double cotA = 1.0/tan(alpha);
    double cotB = 1.0/tan(beta);
    double cotC = 1.0/tan(gamma);

    L.coeffRef(id0,id1) += lmesh.WL[id0]*cotC;
    L.coeffRef(id1,id0) += lmesh.WL[id1]*cotC;

    L.coeffRef(id1,id2) += lmesh.WL[id1]*cotA;
    L.coeffRef(id2,id1) += lmesh.WL[id2]*cotA;

    L.coeffRef(id2,id0) += lmesh.WL[id2]*cotB;
    L.coeffRef(id0,id2) += lmesh.WL[id0]*cotB;

    L.coeffRef(id0,id0) -= lmesh.WL[id0]*cotA;
    L.coeffRef(id1,id1) -= lmesh.WL[id1]*cotB;
    L.coeffRef(id2,id2) -= lmesh.WL[id2]*cotC;
}

////////////////////////////////////////////////////////////////////////////////

void JMeshContraction:: buildLMatrix()
{
    size_t numNodes = lmesh.nodes.size();

    L.resize( 2*numNodes, numNodes);
    L.setZero();

    size_t numFaces = lmesh.faces.size();
    for(size_t i = 0; i < numFaces; i++) setEdgesWeight(i);

    for(size_t i = 0; i < numNodes; i++)
        L.coeffRef(numNodes+i, i) = lmesh.WH[i];
}

////////////////////////////////////////////////////////////////////////////////
void JMeshContraction:: solve(int nIterations)
{
	for (int iter = 0; iter < nIterations; iter++) {
		buildLMatrix();

		Eigen::SparseMatrix<double> Lt = L.transpose();
		Eigen::SparseMatrix<double> A  = Lt*L;

		Eigen::VectorXd x;
		size_t numNodes = lmesh.nodes.size();
		b.resize(2*numNodes);

		SimplicialLDLT<Eigen::SparseMatrix<double>> solver;

		solver.compute(A);

		if( solver.info() != Eigen::Success) 
			cout << "Compute failed " << endl;

		// X-Coordinate
		for( int i = 0; i < numNodes; i++) {
			b[i] = 0.0;
			b[numNodes+i] = lmesh.WH[i]*lmesh.nodes[i][0];
		}
		auto Bx = Lt*b;
		x = solver.solve(Bx);
		double xshift = 0.0;
		for( int i = 0; i < numNodes; i++) {
			xshift = max( xshift, fabs(x(i)-lmesh.nodes[i][0]));
			lmesh.nodes[i][0] = x(i);
		}

		// Y-Coordinate
		for( int i = 0; i < numNodes; i++) {
			b[i] = 0.0;
			b[numNodes+i] = lmesh.WH[i]*lmesh.nodes[i][1];
		}
		auto By = Lt*b;
		x = solver.solve(By);
		double yshift = 0.0;
		for( int i = 0; i < numNodes; i++) {
			yshift = max( yshift, fabs(x(i)-lmesh.nodes[i][1]));
			lmesh.nodes[i][1] = x(i);
		}

		// Z-Coordinate
		for( int i = 0; i < numNodes; i++) {
			b[i] = 0.0;
			b[numNodes+i] = lmesh.WH[i]*lmesh.nodes[i][2];
		}
		auto Bz = Lt*b;
		x = solver.solve(Bz);
		double zshift = 0.0;
		for( int i = 0; i < numNodes; i++) {
			zshift = max( zshift, fabs(x(i)-lmesh.nodes[i][2]));
			lmesh.nodes[i][2] = x(i);
		}

		// Now change the weights for the  next iterations.

		setArea();
		for( int i = 0; i < numNodes; i++) {
			lmesh.WL[i] *= 2.0;
			lmesh.WH[i] *= sqrt( lmesh.nodeArea[0][i]/lmesh.nodeArea[1][i]);
		}
		cout << "Max Shift " << xshift << " " << yshift << " " << zshift << endl;
	}

}
