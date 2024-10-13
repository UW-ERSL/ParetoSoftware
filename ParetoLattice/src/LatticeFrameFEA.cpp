#include "LatticeFrameFEA.hpp"
#include "MMA_optimizer.hpp"
#include "cmatrix.h"
using namespace std::placeholders; // MMA - need this header

using namespace std;
#pragma warning(disable:4996)


/**********************************************************************/
LatticeFrameFEA::LatticeFrameFEA()
{
    lf_STATIC_FEA_SOLVED = false;
    lf_MODAL_FEA_SOLVED = false;
}
/**********************************************************************/
bool LatticeFrameFEA::assembleLatticeFrameFEA(void)
{
    int nLatticeNodes,nStruts,part,node0,node1;
    double materialProperties[7],E,nu,A,G,radius,L,I,J;
    Point3D xAxis,yAxis,zAxis;
    MatrixXd KElem(6,6),TElem(6,6),TTranspose(6,6),KT(6,6);
    char materialName[256];
    //see http://what-when-how.com/the-finite-element-method/fem-for-frames-finite-element-method-part-1/

    nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    nStruts = getLatticeGeneratorNumberOfStruts();
    if ( (nLatticeNodes == 0) || (nStruts == 0) ){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    part = 0;
    getMaterialPropertiesOfPart(part,materialProperties,materialName);
    E = materialProperties[0];// Young's modulus
    nu = materialProperties[1];// Poisson's ratio
    G = E/2/(1+nu);

    lf_denseKMatrix.resize(6*nLatticeNodes,6*nLatticeNodes);
    lf_denseKMatrix.setZero(6*nLatticeNodes,6*nLatticeNodes);
    std::cout.precision(10);

    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 6; j++){
            KElem(i,j) = 0;
            TElem(i,j) = 0;
            TTranspose(i,j) = 0;
            KT(i,j) = 0;
        }
    }

    for (int strut = 0; strut < nStruts; strut++){
        radius = getLatticeGeneratorRadiusOfTrunkStrut(strut);
        L = lg_LatticeGraph.lengthOfStrut[strut];
        if ((radius < TOL) || (L < TOL)){
            writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble, strut %d, radius= %.2g, length = %.2g", strut,radius,L);
            return false;
        }
        A = 3.14159*radius*radius;
        I = 3.14159*pow(radius,4.0)/4.0;
        J = 3.14159*pow(radius,4.0)/2.0;

        // the following 3 define local Cs in global CS
        // the strut is assumed to be cylindrical
        // so, orientation is immaterial
        xAxis = lg_LatticeGraph.xAxis[strut];// along strut
        yAxis = lg_LatticeGraph.yAxis[strut];
        zAxis = lg_LatticeGraph.zAxis[strut];

        for (int i = 0; i < 3; i++){
            TElem(0,i) = xAxis[i];
            TElem(1,i) = yAxis[i];
            TElem(2,i) = zAxis[i];

            TElem(3,3+i) = xAxis[i];
            TElem(4,3+i) = yAxis[i];
            TElem(5,3+i) = zAxis[i];
        }
        TTranspose = TElem.transpose();
        KElem(0,0) = E*A/L;
        KElem(1,1) = 12*E*I/pow(L,3.0);
        KElem(1,5) = 6*E*I/pow(L,2.0);
        KElem(5,1) = 6*E*I/pow(L,2.0);
        KElem(2,2) = 12*E*I/pow(L,3.0);
        KElem(2,4) = -6*E*I/pow(L,2.0);
        KElem(4,2) = -6*E*I/pow(L,2.0);
        KElem(3,3) = G*J/L;
        KElem(4,4) = 4*E*I/L;
        KElem(5,5) = 4*E*I/L;

        KT = TTranspose*KElem*TElem;
        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];

        for (int i = 0; i < 6; i++){
            for (int j = 0; j < 6; j++){
                lf_denseKMatrix(6*node0+i,6*node0+j) += KT(i,j);
                lf_denseKMatrix(6*node1+i,6*node1+j) += KT(i,j);
                lf_denseKMatrix(6*node0+i,6*node1+j) -= KT(i,j);
                lf_denseKMatrix(6*node1+i,6*node0+j) -= KT(i,j);
            }
        }
    }
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::assembleLatticeMassMatrix(void)
{
    int nLatticeNodes,nStruts,part,node0,node1;
    double materialProperties[7],E,nu,rho,A,G,radius,L,I,J,m;
    Point3D xAxis,yAxis,zAxis;
    MatrixXd MElemDiag(6,6),MElemOffDiag(6,6),TElem(6,6),TTranspose(6,6);
    MatrixXd MTDiag(6,6),MTOffDiag(6,6);
    char materialName[256];
    //see http://what-when-how.com/the-finite-element-method/fem-for-frames-finite-element-method-part-1/

    nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    nStruts = getLatticeGeneratorNumberOfStruts();
    if ( (nLatticeNodes == 0) || (nStruts == 0) ){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    part = 0;
    getMaterialPropertiesOfPart(part,materialProperties,materialName);
    E = materialProperties[0];// Young's modulus
    nu = materialProperties[1];// Poisson's ratio
    rho = materialProperties[3];// density
    G = E/2/(1+nu);

    lf_denseMMatrix.resize(6*nLatticeNodes,6*nLatticeNodes);
    lf_denseMMatrix.setZero(6*nLatticeNodes,6*nLatticeNodes);

    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 6; j++){
            MElemDiag(i,j) = 0;
            MElemOffDiag(i,j) = 0;
            TElem(i,j) = 0;
            TTranspose(i,j) = 0;
            MTDiag(i,j) = 0;
            MTOffDiag(i,j) = 0;
        }
    }

    for (int strut = 0; strut < nStruts; strut++){
        radius = getLatticeGeneratorRadiusOfTrunkStrut(strut);
        L = lg_LatticeGraph.lengthOfStrut[strut];
        if ((radius < TOL) || (L < TOL)){
            writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble, strut %d, radius= %.2g, length = %.2g", strut,radius,L);
            return false;
        }
        A = 3.14159*radius*radius;
        I = 3.14159*pow(radius,4.0)/4.0;
        J = 3.14159*pow(radius,4.0)/2.0;

        // the following 3 define local Cs in global CS
        // the strut is assumed to be cylindrical
        // so, no orientation is immaterial
        xAxis = lg_LatticeGraph.xAxis[strut];// along strut
        yAxis = lg_LatticeGraph.yAxis[strut];
        zAxis = lg_LatticeGraph.zAxis[strut];

        for (int i = 0; i < 3; i++){
            TElem(0,i) = xAxis[i];
            TElem(1,i) = yAxis[i];
            TElem(2,i) = zAxis[i];

            TElem(3,3+i) = xAxis[i];
            TElem(4,3+i) = yAxis[i];
            TElem(5,3+i) = zAxis[i];
        }
        TTranspose = TElem.transpose();

        m = rho*A*L/2/105.0;
        MElemDiag(0,0) = m*70;
        MElemDiag(1,1) = m*78;
        MElemDiag(1,5) = m*22*L/2;
        MElemDiag(5,1) = m*22*L/2;
        MElemDiag(2,2) = m*78;
        MElemDiag(2,4) = -m*22*L/2;
        MElemDiag(4,2) = -m*22*L/2;
        MElemDiag(3,3) = m*70*J/A;
        MElemDiag(4,4) = m*8*L*L/4;
        MElemDiag(5,5) = m*8*L*L/4;
        MTDiag = TTranspose*MElemDiag*TElem;

        MElemOffDiag(0,0) = m*35;
        MElemOffDiag(1,1) = m*27;
        MElemOffDiag(1,5) = -m*13*L/2;
        MElemOffDiag(5,1) = -m*13*L/2;
        MElemOffDiag(2,2) = m*27;
        MElemOffDiag(2,4) = m*13*L/2;
        MElemOffDiag(4,2) = m*13*L/2;
        MElemOffDiag(3,3) = -m*35*J/A;
        MElemOffDiag(4,4) = -m*6*L*L/4;
        MElemOffDiag(5,5) = -m*6*L*L/4;
        MTOffDiag = TTranspose*MElemOffDiag*TElem;

        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];

        for (int i = 0; i < 6; i++){
            for (int j = 0; j < 6; j++){
                lf_denseMMatrix(6*node0+i,6*node0+j) += MTDiag(i,j);
                lf_denseMMatrix(6*node1+i,6*node1+j) += MTDiag(i,j);
                lf_denseMMatrix(6*node0+i,6*node1+j) += MTOffDiag(i,j);
                lf_denseMMatrix(6*node1+i,6*node0+j) += MTOffDiag(i,j);
            }
        }
    }
    return true;
}
/**********************************************************************/
bool    LatticeFrameFEA::isLatticeFrameNodeFixed(int node)
{
    for (int j = 0; j < lf_fixedNodes.size(); j++)
        if (lf_fixedNodes.at(j) == node) return true;

    return false;
}
/**********************************************************************/
bool LatticeFrameFEA::applyLatticeNodalForce(int node,double f[3])
{
    if ((node < 0) || (node >= getLatticeGeneratorNumberOfNodes() )){
        writeMessageToParetoLog("LatticeFrameFEA: cannot apply force, incorrect node %d",node);
        return false;
    }
    for (int i = 0; i < 3;i++){// the last 3 moments are assumed to be zero
        lf_forceVector(6*node+i) = f[i];
    }
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::fixLatticeNodeFrame(int node)
{
    // fix all 6 dof
    lf_fixedNodes.push_back(node);
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::resetLatticeFrameFEA(void)
{
    lf_STATIC_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot resetNodalForce; nLatticeNodes is zero");
        return false;
    }
    lf_fixedNodes.clear();
    lf_forceVector.resize(6*nLatticeNodes);
    lf_forceVector.setZero(6*nLatticeNodes);
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::enforceDirichlet(VectorXd x)
{
    int node;

    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (x.rows() != 6*nLatticeNodes) return false;

    for (int j = 0; j < lf_fixedNodes.size(); j++){
        node = lf_fixedNodes.at(j);
        for (int i = 0; i < 6; i++)
            x(6*node + i) = 0;
    }
    return true;
}

/**********************************************************************/
bool LatticeFrameFEA::solveLatticeFrameFEA(void)
{
    lf_STATIC_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot solve; nLatticeNodes is zero");
        return false;
    }
    double maxValue = lf_denseKMatrix.maxCoeff();
    // for all fixed nodes, make the diagonals huge
    for (int j = 0; j < lf_fixedNodes.size(); j++){
        int dof, node;
        node = lf_fixedNodes.at(j);
        for (int i = 0; i < 6; i++){
            dof = 6*node + i;
            lf_forceVector(dof) = 0;
            lf_denseKMatrix(dof,dof) = 1e5*maxValue;
        }
    }
    // are there nodes with no struts? This will lead to zero diagonals
    for (int dof = 0; dof < 6*nLatticeNodes; dof++){
        if (fabs(lf_denseKMatrix(dof,dof)) < TOL){
            //cout << "vanishing dof: " << dof << endl;
            lf_denseKMatrix(dof,dof) = 1e5*maxValue;
            lf_forceVector(dof) = 0;
        }
    }

    Timer timer;
    timer.start();
    lf_sparseKMatrix = lf_denseKMatrix.sparseView();
    lf_sparseKMatrixSolver.compute(lf_sparseKMatrix);
    if (lf_sparseKMatrixSolver.info() != 0) {
        writeMessageToParetoLog("LatticeFrameFEA: Sparse LLT decomposition failed");
        return false;
    }
    lf_displacementVector = lf_sparseKMatrixSolver.solve(lf_forceVector);

    cout << "Time taken (seconds) " << timer.getElapsedTimeInSec() << endl;

    double u,v,w,delta;
    lf_deltaMax = -HUGE_VAL;
    for (int node = 0;node< lg_LatticeGraph.nodes.size();node++){
        u = lf_displacementVector[6*node+0];
        v = lf_displacementVector[6*node+1];
        w = lf_displacementVector[6*node+2];
        delta = sqrt(u*u + v*v + w*w);
        lf_deltaMax = MAX(lf_deltaMax,delta);
    }
    lf_STATIC_FEA_SOLVED = true;
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::solveLatticeModalFEA(void)
{
    // Inverse iteration method
    Timer timer;
    VectorXd    xBar,x;
    double denom,lambda,prevLambda,err;

    if (!lf_STATIC_FEA_SOLVED){
        writeMessageToParetoLog("LatticeModalFEA: Please solve static FEA first");
        return false;
    }
    lf_MODAL_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeModalFEA: Cannot solve; nLatticeNodes is zero");
        return false;
    }

    timer.start();

    lf_sparseMMatrix = lf_denseMMatrix.sparseView();

    x.resize(6*nLatticeNodes);
    xBar.resize(6*nLatticeNodes);
    x = x.Random(6*nLatticeNodes);
    prevLambda = 0;
    for (int iter = 1; iter <= 100; iter++){
        xBar = lf_sparseKMatrixSolver.solve(lf_sparseMMatrix*x);
        denom = xBar.transpose()*lf_sparseMMatrix*xBar;
        x = xBar/denom;
        lambda = x.transpose()*lf_sparseKMatrix*x;
        if (iter%2 == 1) continue;// odd numbers diverges !?
        err = fabs(prevLambda-lambda)/lambda;
        //cout << iter << ": "<<  denom << ", " << lambda << ", " << err << endl;
        if (err < 1e-5) break;
        prevLambda = lambda;
    }

    cout << "Time taken (seconds) " << timer.getElapsedTimeInSec() << endl;

    lf_modalVector = x;
    lf_firstModalFrequency = lambda/(2*3.1415);

    double u,v,w,delta,deltaMax;
    deltaMax = -HUGE_VAL;
    for (int node = 0;node< lg_LatticeGraph.nodes.size();node++){
        u = lf_modalVector[6*node+0];
        v = lf_modalVector[6*node+1];
        w = lf_modalVector[6*node+2];
        delta = sqrt(u*u + v*v + w*w);
        deltaMax = MAX(deltaMax,delta);
    }
    // Now scale per static solution for convenience
     for (int i = 0;i< 6*lg_LatticeGraph.nodes.size();i++)
         lf_modalVector[i] = lf_modalVector[i]*lf_deltaMax/deltaMax;


    lf_MODAL_FEA_SOLVED = true;
}
/**********************************************************************/
bool LatticeFrameFEA::solveLatticeModalFEAFull(void)
{
    if (!lf_STATIC_FEA_SOLVED){
        writeMessageToParetoLog("LatticeModalFEA: Please solve static FEA first");
        return false;
    }
    lf_MODAL_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeModalFEA: Cannot solve; nLatticeNodes is zero");
        return false;
    }
    GeneralizedSelfAdjointEigenSolver<MatrixXd> ges;

    /*double maxValue = lf_denseMMatrix.maxCoeff();
    // for all fixed nodes, make the diagonals huge
    for (int j = 0; j < lf_fixedNodes.size(); j++){
        int dof, node;
        node = lf_fixedNodes.at(j);
        for (int i = 0; i < 6; i++){
            dof = 6*node + i;
            lf_denseMMatrix(dof,dof) = 1e5*maxValue;
        }
    }
    */

    lf_sparseKMatrix = lf_denseKMatrix.sparseView();
    lf_sparseMMatrix = lf_denseMMatrix.sparseView();


    ges.compute(lf_sparseKMatrix,lf_sparseMMatrix);
    cout <<  ges.eigenvalues().transpose() << endl;

    lf_MODAL_FEA_SOLVED = true;
    return true;
}
/**********************************************************************/
double LatticeFrameFEA::computeLatticeCompliance(vector<double> strutRadiiScaled)
{
    int nStruts;
    double compliance;
    nStruts = getLatticeGeneratorNumberOfStruts();
    for (int strut = 0; strut < nStruts; strut++)
        lg_LatticeGraph.radiusOfTrunkStrut[strut]  = lf_initialDefaultRadius*strutRadiiScaled[strut];

    assembleLatticeFrameFEA();
    solveLatticeFrameFEA();

    compliance = (lf_displacementVector.transpose())*lf_forceVector;
    return compliance;
}
/**********************************************************************/
bool LatticeFrameFEA::computeLatticeInitialVolume(void)
{
    double volume;
    int nStruts;
    double A,L,radius;

    nStruts = getLatticeGeneratorNumberOfStruts();
    if (nStruts == 0 ){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    volume = 0;
    for (int strut = 0; strut < nStruts; strut++){
        radius = lg_LatticeGraph.radiusOfTrunkStrut[strut];
        L = lg_LatticeGraph.lengthOfStrut[strut];
        A = 3.14159*radius*radius;
        volume += A*L;
    }
    lf_initialVolume = volume;
    return true;
}

/**********************************************************************/
bool LatticeFrameFEA::computeLatticeVolumeConstraint(vector<double> strutRadiiScaled, vector<double> &constraints)
{
    double volume;
    int nStruts;
    double A,L,radius;

    nStruts = getLatticeGeneratorNumberOfStruts();
    if (nStruts == 0 ){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        constraints[0] = -1;
        return false;
    }
    volume = 0;
    for (int strut = 0; strut < nStruts; strut++){
        radius =  lf_initialDefaultRadius*strutRadiiScaled[strut];
        L = lg_LatticeGraph.lengthOfStrut[strut];
        A = 3.14159*radius*radius;
        volume += A*L;
    }
    constraints[0] = volume/lf_initialVolume - 1.0;
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::computeLatticeVolumeConstraintGradient(vector<double> strutRadiiScaled, vector<double> &volumeGradient)
{

    int nStruts;
    double AGradient,L,radius;

    nStruts = getLatticeGeneratorNumberOfStruts();
    if (nStruts == 0){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    for (int strut = 0; strut < nStruts; strut++){
        radius = lf_initialDefaultRadius*strutRadiiScaled[strut];
        L = lg_LatticeGraph.lengthOfStrut[strut];
        AGradient = 2*3.14159*radius;
        volumeGradient[strut] = lf_initialDefaultRadius*AGradient*L/lf_initialVolume;
    }
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::computeLatticeComplianceGradient(vector<double> strutRadiiScaled, vector<double> &gradient)
{
    // compute the strain energy in each beam element
    int nLatticeNodes,nStruts,part,node0,node1;
    double materialProperties[7],E,nu,AGradient,G,radius,L,IGradient,JGradient;
    Point3D xAxis,yAxis,zAxis;
    MatrixXd KElemGradient(6,6),TElem(6,6),TTranspose(6,6),KTGradient(6,6),KStrutGradient(12,12);
    VectorXd strutSolution(12);
    char materialName[256];

    nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    nStruts = getLatticeGeneratorNumberOfStruts();
    if ( (nLatticeNodes == 0) || (nStruts == 0) ){
        writeMessageToParetoLog("LatticeFrameFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    //computeLatticeCompliance(strutRadiiScaled);// make sure the displacements are current before computing gradient
    part = 0;
    getMaterialPropertiesOfPart(part,materialProperties,materialName);
    E = materialProperties[0];// Young's modulus
    nu = materialProperties[1];// Poisson's ratio
    G = E/2/(1+nu);
    for (int i = 0; i < 6; i++){
        for (int j = 0; j < 6; j++){
            KElemGradient(i,j) = 0;
            TElem(i,j) = 0;
            TTranspose(i,j) = 0;
            KTGradient(i,j) = 0;
        }
    }
    for (int strut = 0; strut < nStruts; strut++){
        radius = lf_initialDefaultRadius*strutRadiiScaled[strut];
        L = lg_LatticeGraph.lengthOfStrut[strut];
        AGradient = 2*3.14159*radius;
        IGradient = 4*3.14159*pow(radius,3.0)/4.0;
        JGradient = 4*3.14159*pow(radius,3.0)/2.0;

        // the following 3 define local Cs in global CS
        // the strut is assumed to be cylindrical
        // so, orientation is immaterial
        xAxis = lg_LatticeGraph.xAxis[strut];// along strut
        yAxis = lg_LatticeGraph.yAxis[strut];
        zAxis = lg_LatticeGraph.zAxis[strut];

        for (int i = 0; i < 3; i++){
            TElem(0,i) = xAxis[i];
            TElem(1,i) = yAxis[i];
            TElem(2,i) = zAxis[i];

            TElem(3,3+i) = xAxis[i];
            TElem(4,3+i) = yAxis[i];
            TElem(5,3+i) = zAxis[i];
        }
        TTranspose = TElem.transpose();
        KElemGradient(0,0) = E*AGradient/L;
        KElemGradient(1,1) = 12*E*IGradient/pow(L,3.0);
        KElemGradient(1,5) = 6*E*IGradient/pow(L,2.0);
        KElemGradient(5,1) = 6*E*IGradient/pow(L,2.0);
        KElemGradient(2,2) = 12*E*IGradient/pow(L,3.0);
        KElemGradient(2,4) = -6*E*IGradient/pow(L,2.0);
        KElemGradient(4,2) = -6*E*IGradient/pow(L,2.0);
        KElemGradient(3,3) = G*JGradient/L;
        KElemGradient(4,4) = 4*E*IGradient/L;
        KElemGradient(5,5) = 4*E*IGradient/L;

        KTGradient = TTranspose*KElemGradient*TElem;

        KStrutGradient.block(0,0,6,6) = KTGradient;
        KStrutGradient.block(6,6,6,6) = KTGradient;
        KStrutGradient.block(0,6,6,6) = -KTGradient;
        KStrutGradient.block(6,0,6,6) = -KTGradient;

        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];
        for (int i = 0; i < 6; i++){
            strutSolution(i) = lf_displacementVector[6*node0+i];
            strutSolution(6+i) = lf_displacementVector[6*node1+i];
        }
        gradient[strut] = -(strutSolution.transpose())*KStrutGradient*strutSolution;
        gradient[strut] *= lf_initialDefaultRadius;// due to scaling
    }
    return true;
}
/**********************************************************************/
double LatticeFrameFEA::optimalityCriteria(vector<double> &strutRadiiScaled,
                                           vector<double> gradient)
{
    vector<double> lagrange,constraints;
    int nStruts;
    double lagrangeMin,lagrangeMax,lagrangeMid,radius,L,modifiedScaledRadius,deltaChangeMax;
    double maxChangeInScaledRadii;
    int OCIter;

    nStruts = getLatticeGeneratorNumberOfStruts();
    lagrange.resize(nStruts,0);
    constraints.resize(1,0);
    lagrangeMin = 0;
    lagrangeMax = 1e5;
    deltaChangeMax = 0.1;
    OCIter = 0;
    while (lagrangeMax-lagrangeMin > 1e-3){
        //cout << lagrangeMin << ", " << lagrangeMax << endl;
        lagrangeMid = (lagrangeMax+lagrangeMin)/2.0;
        maxChangeInScaledRadii = 0;
        for (int strut = 0; strut < nStruts; strut++){
            radius = lf_initialDefaultRadius*strutRadiiScaled[strut];
            L = lg_LatticeGraph.lengthOfStrut[strut];
            lagrange[strut] = fabs(gradient[strut]/lf_initialDefaultRadius)/(2*3.1415*radius*L);
            modifiedScaledRadius = strutRadiiScaled[strut]*(lagrange[strut]/lagrangeMid);
            modifiedScaledRadius = MIN(strutRadiiScaled[strut]+deltaChangeMax,modifiedScaledRadius);
            modifiedScaledRadius = MAX(strutRadiiScaled[strut]-deltaChangeMax,modifiedScaledRadius);
            modifiedScaledRadius = MAX(0.01,MIN(modifiedScaledRadius,6.0));
            maxChangeInScaledRadii = MAX(maxChangeInScaledRadii,fabs(strutRadiiScaled[strut]-modifiedScaledRadius));
            strutRadiiScaled[strut] = modifiedScaledRadius;
        }
        //cout  << "maxChangeInScaledRadii: " << maxChangeInScaledRadii << endl;
        computeLatticeVolumeConstraint(strutRadiiScaled,constraints);
        if (constraints[0] > 0)// not satisifed
            lagrangeMin = lagrangeMid;
        else
            lagrangeMax = lagrangeMid;

        OCIter++;
    }
    cout << "InnerIter: " << OCIter << endl;
    return maxChangeInScaledRadii;
}
/**********************************************************************/
bool LatticeFrameFEA::optimizeLatticeStructureOC(void)
{
    //Algorithm
    // 1. Solve FEA
    // 2. Compute gradient
    // 3. Compute the element Lagrange multipliers
    // 4. Update the radii and multipliers based on optimality criteria until convgerence
    // 5. Repeat Steps 1 through 4 until converegence
    int iter;
    int nStruts,numMaxIter;
    double relFuncTol,initialDesignVal;
    vector<double> scaledRadii,complianceGradient,lagrange,constraints;
    double maxChangeInScaledRadii,compliance;

    nStruts = getLatticeGeneratorNumberOfStruts();
    numMaxIter = 100;
    relFuncTol = 1e-3;
    lf_initialDefaultRadius = getLatticeGeneratorRadiusOfTrunkStrut(0);
    computeLatticeInitialVolume();
    writeMessageToParetoLog("initial volume = %.2g",lf_initialVolume);

    initialDesignVal = 1.0;//non-dimensional
    scaledRadii.resize(nStruts,initialDesignVal);
    complianceGradient.resize(nStruts,0);
    lagrange.resize(nStruts,0);
    Timer timer;
    timer.start();
    iter = 0;
    while (iter < 10){
        compliance = computeLatticeCompliance(scaledRadii);//FEA
        cout << "OuterIter:" << iter << ": compliance " << compliance << endl;
        computeLatticeComplianceGradient(scaledRadii,complianceGradient);
        maxChangeInScaledRadii = optimalityCriteria(scaledRadii,complianceGradient);
        iter++;
    }
    computeLatticeCompliance(scaledRadii);//FEA
    cout << "\nTime taken: " << timer.getElapsedTimeInSec() << endl;
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::optimizeLatticeStructureMMA(void)
{
    MMA_optimizer optim; // MMA - init the object
    int nStruts,nDesignVariables,nConstraints,numMaxIter;
    double relFuncTol,initialDesignVal,minDesignVals,maxDesignVals,J0;

    nStruts = getLatticeGeneratorNumberOfStruts();
    nDesignVariables = nStruts;
    nConstraints =  1; // a volume constraint

    numMaxIter = 100;
    relFuncTol = 1e-3;
    lf_initialDefaultRadius = getLatticeGeneratorRadiusOfTrunkStrut(0);


    initialDesignVal = 1.0;//non-dimensional
    minDesignVals = 0.05;//non-dimensional
    maxDesignVals = 2.0;//non-dimensional
    // MMA
    optim.initialize(nDesignVariables,nConstraints,numMaxIter,relFuncTol); //MMA
    vector<double> initVals(nStruts,initialDesignVal);
    vector<double> minVals(nStruts,minDesignVals);
    vector<double> maxVals(nStruts,maxDesignVals);
    optim.setDesignVariableInitialValuesAndBounds(initVals,minVals,maxVals); //design variables

    J0 = computeLatticeCompliance(initVals);
    computeLatticeInitialVolume();
    writeMessageToParetoLog("Initial compliance = %.2g",J0);
    writeMessageToParetoLog("Initial volume = %.2g",lf_initialVolume);

    auto fp1 = std::bind(&LatticeFrameFEA::computeLatticeCompliance,this,_1);//objective
    auto fp2 = std::bind(&LatticeFrameFEA::computeLatticeVolumeConstraint,this,_1,_2); // constraint
    auto fp3 = std::bind(&LatticeFrameFEA::computeLatticeComplianceGradient,this,_1,_2); //gradient of objective
    auto fp4 = std::bind(&LatticeFrameFEA::computeLatticeVolumeConstraintGradient,this,_1,_2); //gradient of constraint
    auto fp5 = std::bind(&LatticeFrameFEA::terminateOptimizerCallBack,this,_1); //MMA

    Timer timer;
    timer.start();
    optim.optimize(fp1,fp2,fp3,fp4,fp5);  // MMA - call the optimizer
    cout << "\nTime taken: " << timer.getElapsedTimeInSec() << endl;
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::terminateOptimizerCallBack(vector<double> &strutRadiiScaled)
{
    return false;
}
/**********************************************************************/
bool LatticeFrameFEA::isLatticeFrameFEASolved(void)
{
    return lf_STATIC_FEA_SOLVED;
}
/**********************************************************************/
int LatticeFrameFEA::getLatticeNumberOfFixedNodes(void)
{
    return lf_fixedNodes.size();
}
/**********************************************************************/
bool LatticeFrameFEA::getLatticeForceAtNode(int node,double force[3])
{
    for (int i = 0; i < 3; i++)
        force[i] = lf_forceVector[6*node+i];// only the force being returned, not the moment
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::getLatticeDisplacementAtNode(int node,double uvw[3])
{
    if (!lf_STATIC_FEA_SOLVED)
        return false;
    if (6*node >= lf_displacementVector.size()){
        writeMessageToParetoLog("LatticeFrameFEA getDisplacementAtNode Error: node %d, size of solution: %d", node,lf_displacementVector.size() );
        return false;
    }
    for (int i = 0; i < 3; i++)
        uvw[i] = lf_displacementVector[6*node+i];
    return true;
}
/**********************************************************************/
bool LatticeFrameFEA::getLatticeModalDisplacementAtNode(int node,double uvw[3])
{
    if (!lf_MODAL_FEA_SOLVED)
        return false;
    if (6*node >= lf_modalVector.size()){
        writeMessageToParetoLog("LatticeFrameFEA getLatticeModalDisplacementAtNode Error: node %d, size of solution: %d", node,lf_modalVector.size() );
        return false;
    }
    for (int i = 0; i < 3; i++)
        uvw[i] = lf_modalVector[6*node+i];
    return true;
}
/**********************************************************************/
double LatticeFrameFEA::getLatticeDeltaMax(void)
{
    if (!lf_STATIC_FEA_SOLVED) return 0;
    return lf_deltaMax;
}
/**********************************************************************/
double LatticeFrameFEA::getLatticeFirstModalFrequency(void)
{
    if (!lf_MODAL_FEA_SOLVED) return 0;
    return lf_firstModalFrequency;
}
