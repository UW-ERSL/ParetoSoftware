#ifndef LATTICE_TRUSS_FEA_H_
#define LATTICE_TRUSS_FEA_H_

#include <algorithm>
#include<iostream>
#include<math.h>
#include<fstream>
#include<stdlib.h>
#include<vector>
#include<string.h>
#include<time.h>
#include<limits>
#include <stdio.h>
#include <sstream>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include "LatticeGenerator.hpp" // import Lattice functions


using namespace std;


class LatticeTrussFEA : public virtual LatticeGenerator
{


private:
        bool                                lf_FEA_SOLVED;
        double                              lf_deltaMax;
        vector<int>                         lf_fixedDOF;// all the fixed dof
        VectorXd                            lf_solutionVector;// 3*N, u v w
        VectorXd                            lf_forceVector;// 3*N, fx, fy, fz
        MatrixXd                            lf_denseKMatrix;
        Eigen::SparseMatrix<double>			lf_sparseKMatrix;
        SimplicialLDLT<SparseMatrix<double>> lf_sparseKMatrixSolver;

protected:

public:
                LatticeTrussFEA();
        bool    isLatticeTrussFEASolved(void);
        double  getLatticeDeformationMax(void);
        bool    getDisplacementAtNode(int node, double uvw[3]);
        bool    resetLatticeTrussFEA(void);
        bool    fixLatticeNode(int node);
        bool    fixLatticeNodeXDOF(int node);
        bool    fixLatticeNodeYDOF(int node);
        bool    fixLatticeNodeZDOF(int node);
        bool    assembleLatticeTrussFEA(void);
        bool    applyNodalForce(int node,double force[3]);
        bool    solveLatticeTrussFEA(void);
};

#endif
