#include "LatticeTrussFEA.hpp"
#include "cmatrix.h"

using namespace std;
#pragma warning(disable:4996)


/**********************************************************************/
LatticeTrussFEA::LatticeTrussFEA()
{
   lf_deltaMax = 0;
   lf_FEA_SOLVED = false;
}
/**********************************************************************/
bool LatticeTrussFEA::assembleLatticeTrussFEA(void)
{
    int nLatticeNodes,nStruts,part,node0,node1;
    double cx,cy,cz;
    double kElem[3][3],materialProperties[7],E,area,k,radius,length;
    char materialName[256];

    nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    nStruts = getLatticeGeneratorNumberOfStruts();
    if ( (nLatticeNodes == 0) || (nStruts == 0) ){
        writeMessageToParetoLog("LatticeTrussFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    writeMessageToParetoLog("Assembling Lattice Stiffness Matrix");
    part = 0;
    getMaterialPropertiesOfPart(part,materialProperties,materialName);
    E = materialProperties[0];
    lf_denseKMatrix.resize(3*nLatticeNodes,3*nLatticeNodes);
    lf_denseKMatrix.setZero(3*nLatticeNodes,3*nLatticeNodes);
    std::cout.precision(10);
    for (int strut = 0; strut < nStruts; strut++){
        cx = lg_LatticeGraph.cosXOfStrut[strut];
        cy = lg_LatticeGraph.cosYOfStrut[strut];
        cz = lg_LatticeGraph.cosZOfStrut[strut];

        radius = getLatticeGeneratorRadiusOfStrut(strut);
        area = 3.14159*radius*radius;
        length = lg_LatticeGraph.lengthOfStrut[strut];

        if ((area < TOL) || (length < TOL)){
            writeMessageToParetoLog("LatticeTrussFEA: Cannot assemble, strut %d, area= %.2g, length = %.2g", strut,area,length);
            return false;
        }
        k  = E*area/length;
        kElem[0][0] = k*cx*cx;kElem[0][1] = k*cx*cy;kElem[0][2] = k*cx*cz;
        kElem[1][0] = k*cy*cx;kElem[1][1] = k*cy*cy;kElem[1][2] = k*cy*cz;
        kElem[2][0] = k*cz*cx;kElem[2][1] = k*cz*cy;kElem[2][2] = k*cz*cz;

        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];

        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                lf_denseKMatrix(3*node0+i,3*node0+j) += kElem[i][j];
                lf_denseKMatrix(3*node1+i,3*node1+j) += kElem[i][j];
                lf_denseKMatrix(3*node0+i,3*node1+j) -= kElem[i][j];
                lf_denseKMatrix(3*node1+i,3*node0+j) -= kElem[i][j];
            }
        }
    }
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::applyNodalForce(int node,double force[3])
{
    if ((node < 0) || (node >= getLatticeGeneratorNumberOfNodes() )){
        writeMessageToParetoLog("LatticeTrussFEA: cannot apply force, incorrect node %d",node);
        return false;
    }
    for (int i = 0; i <3; i++)
        lf_forceVector(3*node+i) = force[i];

    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::fixLatticeNodeXDOF(int node)
{
    lf_fixedDOF.push_back(3*node);
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::fixLatticeNodeYDOF(int node)
{
    lf_fixedDOF.push_back(3*node+1);
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::fixLatticeNodeZDOF(int node)
{
    lf_fixedDOF.push_back(3*node+2);
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::fixLatticeNode(int node)
{
    lf_fixedDOF.push_back(3*node);
    lf_fixedDOF.push_back(3*node+1);
    lf_fixedDOF.push_back(3*node+2);
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::resetLatticeTrussFEA(void)
{
    lf_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeTrussFEA: Cannot resetNodalForce; nLatticeNodes is zero");
        return false;
    }
    lf_fixedDOF.clear();

    lf_forceVector.resize(3*nLatticeNodes);
    lf_forceVector.setZero(3*nLatticeNodes);
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::solveLatticeTrussFEA(void)
{
    lf_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeTrussFEA: Cannot solve; nLatticeNodes is zero");
        return false;
    }
    // take care of fixed dof

    double maxValue = lf_denseKMatrix.maxCoeff();
    for (int i = 0; i < lf_fixedDOF.size(); i++){
        int dof;
        dof = lf_fixedDOF.at(i);
        // kill all columns and rows
        /*for (int k = 0; k < 3*nLatticeNodes; k++){
            //lf_denseKMatrix(k,dof) = 0;// column
            lf_denseKMatrix(dof,k) = 0;// row       
        }*/
        lf_forceVector(dof) = 0;
        lf_denseKMatrix(dof,dof) = 1e5*maxValue;
    }
    //cout << lf_denseKMatrix << endl;
    bool convertIntoSparse;
    convertIntoSparse = true;
    if (convertIntoSparse){
        // convert dense into sparse matrix
        lf_sparseKMatrix = lf_denseKMatrix.sparseView();
        writeMessageToParetoLog("Lattice FEA: Sparse Matrix; #rows = %d; nnz = %d ",lf_sparseKMatrix.rows(),lf_sparseKMatrix.nonZeros());
        // construct LLT of sparse matrix
        lf_sparseKMatrixSolver.compute(lf_sparseKMatrix);
        if (lf_sparseKMatrixSolver.info() != 0) {
            writeMessageToParetoLog("LatticeTrussFEA: Sparse LLT decomposition failed");
            return false;
        }
        lf_solutionVector = lf_sparseKMatrixSolver.solve(lf_forceVector);
    }
    else
        lf_solutionVector = lf_denseKMatrix.llt().solve(lf_forceVector);

    double u,v,w,delta,deltaMax,uMax,vMax,wMax,uMin,vMin,wMin;
    uMin  = vMin = wMin = HUGE_VAL;
    uMax = vMax = wMax = deltaMax = -HUGE_VAL;
    for (int node = 0;node< lg_LatticeGraph.nodes.size();node++){
        u = lf_solutionVector[3*node];
        v = lf_solutionVector[3*node+1];
        w = lf_solutionVector[3*node+2];

        uMin = MIN(uMin,u);
        vMin = MIN(vMin,v);
        wMin = MIN(wMin,w);

        uMax = MAX(uMax,u);
        vMax = MAX(vMax,v);
        wMax = MAX(wMax,w);
        delta = sqrt(u*u + v*v + w*w);
        deltaMax = MAX(deltaMax,delta);
    }
    lf_deltaMax = deltaMax;
    writeMessageToParetoLog("LatticeTrussFEA: Static FEA successful");
    writeMessageToParetoLog("uMin = %.2g, uMax = %.2g",uMin,uMax);
    writeMessageToParetoLog("vMin = %.2g, vMax = %.2g",vMin,vMax);
    writeMessageToParetoLog("wMin = %.2g, wMax = %.2g",wMin,wMax);
    writeMessageToParetoLog("deltaMax = %.2g",deltaMax);
    lf_FEA_SOLVED = true;
    return true;
}
/**********************************************************************/
bool LatticeTrussFEA::isLatticeTrussFEASolved(void)
{
    return lf_FEA_SOLVED;
}
/**********************************************************************/
bool LatticeTrussFEA::getDisplacementAtNode(int node, double uvw[3])
{
    if (!lf_FEA_SOLVED)
        return false;
    if (3*node+2 >= lf_solutionVector.size()){
        writeMessageToParetoLog("LatticeTrussFEA getDisplacementAtNode Error: node %d, size of solution: %d", node,lf_solutionVector.size() );
        return false;
    }
    uvw[0] = lf_solutionVector[3*node];
    uvw[1] = lf_solutionVector[3*node+1];
    uvw[2] = lf_solutionVector[3*node+2];
}
/**********************************************************************/
double LatticeTrussFEA::getLatticeDeformationMax(void)
{
    if (!lf_FEA_SOLVED)
        return 0;
    return lf_deltaMax;
}

