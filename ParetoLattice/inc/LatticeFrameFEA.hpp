#ifndef LATTICE_FRAME_FEA_H_
#define LATTICE_FRAME_FEA_H_

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
#include <Eigen/Eigenvalues>

#include "LatticeGenerator.hpp" // import Lattice functions


using namespace std;


class LatticeFrameFEA : public virtual LatticeGenerator
{


private:
        bool                                lf_STATIC_FEA_SOLVED;
        bool                                lf_MODAL_FEA_SOLVED;
        double                              lf_deltaMax;
        double                              lf_firstModalFrequency;
        double                              lf_initialDefaultRadius;
        double                              lf_initialVolume;
        vector<int>                         lf_fixedNodes;// all dof are fixed for this node (for simplicity)
        VectorXd                            lf_displacementVector;// u,v,w,thx,thy,thz for each node
        VectorXd                            lf_modalVector;// u,v,w,thx,thy,thz for each node
        VectorXd                            lf_forceVector;
        MatrixXd                            lf_denseKMatrix;
        Eigen::SparseMatrix<double>         lf_sparseKMatrix;
        SimplicialLDLT<SparseMatrix<double>> lf_sparseKMatrixSolver;
        MatrixXd                            lf_denseMMatrix;
        Eigen::SparseMatrix<double>         lf_sparseMMatrix;

protected:

public:
                LatticeFrameFEA();
        bool    isLatticeFrameFEASolved(void);
        bool    isLatticeFrameNodeFixed(int node);
        bool    enforceDirichlet(VectorXd x);
        bool    resetLatticeFrameFEA(void);
        bool    fixLatticeNodeFrame(int node);
        bool    assembleLatticeMassMatrix(void);
        bool    assembleLatticeFrameFEA(void);
        bool    getLatticeForceAtNode(int node,double force[3]);
        bool    getLatticeDisplacementAtNode(int node,double uvw[3]);
        bool    getLatticeModalDisplacementAtNode(int node,double uvw[3]);
        double  getLatticeDeltaMax(void);
        double  getLatticeFirstModalFrequency(void);
        int     getLatticeNumberOfFixedNodes(void);
        bool    applyLatticeNodalForce(int node, double f[]);
        bool    solveLatticeModalFEAFull(void);
        bool    solveLatticeModalFEA(void);
        bool    solveLatticeFrameFEA(void);
        double  computeLatticeCompliance(vector<double> strutRadiiScaled);
        bool    computeLatticeInitialVolume(void);
        bool    computeLatticeVolumeConstraint(vector<double> strutRadiiScaled, vector<double> &constraints);
        bool    computeLatticeVolumeConstraintGradient(vector<double> strutRadiiScaled, vector<double> &volumeGradient);
        bool    computeLatticeComplianceGradient(vector<double> strutRadiiScaled, vector<double> &gradient);
        bool    terminateOptimizerCallBack(vector<double> &strutRadiiScaled);
        bool    optimizeLatticeStructureMMA(void);
        bool    optimizeLatticeStructureOC(void);
        double  optimalityCriteria(vector<double> &strutRadiiScaled, vector<double> gradient);
};

#endif
