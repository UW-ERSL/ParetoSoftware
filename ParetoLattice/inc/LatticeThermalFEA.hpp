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


class LatticeThermalFEA : public virtual LatticeGenerator
{


private:
        bool                                lf_FEA_SOLVED;
        double                              lf_temperatureMax;
        double                              lf_temperatureMin;
        vector<int>                         lf_fixedDOF;// all the fixed dof
        VectorXd                            lf_solutionVector;// N, Temp
        VectorXd                            lf_forceVector;// N, q
        MatrixXd                            lf_denseKMatrix;
        Eigen::SparseMatrix<double>			lf_sparseKMatrix;
        SimplicialLDLT<SparseMatrix<double>> lf_sparseKMatrixSolver;

protected:

public:
                LatticeThermalFEA();
        bool    isLatticeThermalFEASolved(void);
        double  getLatticeTemperatureMax(void);
        double  getLatticeTemperatureMin(void);
        double  getTemperatureAtNode(int node);
        bool    resetLatticeThermalFEA(void);
        bool    fixLatticeNodeThermal(int node);
        bool    assembleLatticeThermalFEA(void);
        bool    applyLatticeNodalHeatFlux(int node,double q);
        bool    solveLatticeThermalFEA(void);
};

#endif
