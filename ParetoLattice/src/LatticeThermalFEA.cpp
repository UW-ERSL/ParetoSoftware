#include "LatticeThermalFEA.hpp"
#include "cmatrix.h"

using namespace std;
#pragma warning(disable:4996)


/**********************************************************************/
LatticeThermalFEA::LatticeThermalFEA()
{
   lf_temperatureMax = 0;
   lf_FEA_SOLVED = false;
}
/**********************************************************************/
bool LatticeThermalFEA::assembleLatticeThermalFEA(void)
{
    int nLatticeNodes,nStruts,part,node0,node1;
    double materialProperties[7],K,area,radius,length;
    char materialName[256];

    nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    nStruts = getLatticeGeneratorNumberOfStruts();
    if ( (nLatticeNodes == 0) || (nStruts == 0) ){
        writeMessageToParetoLog("LatticeThermalFEA: Cannot assemble; nLatticeNodes or nStruts is zero");
        return false;
    }
    writeMessageToParetoLog("Assembling Lattice Thermal Stiffness Matrix");
    part = 0;
    getMaterialPropertiesOfPart(part,materialProperties,materialName);
    K = materialProperties[4];// conductivity
    lf_denseKMatrix.resize(nLatticeNodes,nLatticeNodes);
    lf_denseKMatrix.setZero(nLatticeNodes,nLatticeNodes);
    std::cout.precision(10);
    for (int strut = 0; strut < nStruts; strut++){
        radius = getLatticeGeneratorRadiusOfTrunkStrut(strut);
        area = 3.14159*radius*radius;
        length = lg_LatticeGraph.lengthOfStrut[strut];

        if ((area < TOL) || (length < TOL)){
            writeMessageToParetoLog("LatticeThermalFEA: Cannot assemble, strut %d, area= %.2g, length = %.2g", strut,area,length);
            return false;
        }
        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];

        lf_denseKMatrix(node0,node0) += K*area/length;
        lf_denseKMatrix(node1,node1) += K*area/length;
        lf_denseKMatrix(node0,node1) -= K*area/length;
        lf_denseKMatrix(node0,node1) -= K*area/length;

    }
    return true;
}
/**********************************************************************/
bool LatticeThermalFEA::applyLatticeNodalHeatFlux(int node,double q)
{
    if ((node < 0) || (node >= getLatticeGeneratorNumberOfNodes() )){
        writeMessageToParetoLog("LatticeThermalFEA: cannot apply force, incorrect node %d",node);
        return false;
    }
    lf_forceVector(node) = q;
    return true;
}
/**********************************************************************/
bool LatticeThermalFEA::fixLatticeNodeThermal(int node)
{
    lf_fixedDOF.push_back(node);
    return true;
}
/**********************************************************************/
bool LatticeThermalFEA::resetLatticeThermalFEA(void)
{
    lf_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeThermalFEA: Cannot resetNodalForce; nLatticeNodes is zero");
        return false;
    }
    lf_fixedDOF.clear();

    lf_forceVector.resize(nLatticeNodes);
    lf_forceVector.setZero(nLatticeNodes);
    return true;
}
/**********************************************************************/
bool LatticeThermalFEA::solveLatticeThermalFEA(void)
{
    lf_FEA_SOLVED = false;
    int nLatticeNodes = getLatticeGeneratorNumberOfNodes();
    if (nLatticeNodes == 0){
        writeMessageToParetoLog("LatticeThermalFEA: Cannot solve; nLatticeNodes is zero");
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
            writeMessageToParetoLog("LatticeThermalFEA: Sparse LLT decomposition failed");
            return false;
        }
        lf_solutionVector = lf_sparseKMatrixSolver.solve(lf_forceVector);
    }
    else
        lf_solutionVector = lf_denseKMatrix.llt().solve(lf_forceVector);

    double T;
    lf_temperatureMin = HUGE_VAL;
    lf_temperatureMax = -HUGE_VAL;
    for (int node = 0;node< lg_LatticeGraph.nodes.size();node++){
        T = lf_solutionVector[node];
        lf_temperatureMin = MIN(lf_temperatureMin,T);
        lf_temperatureMax = MAX(lf_temperatureMax,T);
    }

    writeMessageToParetoLog("LatticeThermalFEA: Static FEA successful");
    writeMessageToParetoLog("TMin = %.2g, TMax = %.2g",lf_temperatureMin,lf_temperatureMax);

    lf_FEA_SOLVED = true;
    return true;
}
/**********************************************************************/
bool LatticeThermalFEA::isLatticeThermalFEASolved(void)
{
    return lf_FEA_SOLVED;
}
/**********************************************************************/
double LatticeThermalFEA::getTemperatureAtNode(int node)
{
    if (!lf_FEA_SOLVED)
        return 0;
    if (node >= lf_solutionVector.size()){
        writeMessageToParetoLog("LatticeThermalFEA getDisplacementAtNode Error: node %d, size of solution: %d", node,lf_solutionVector.size() );
        return 0;
    }
    return lf_solutionVector[node];
}
/**********************************************************************/
double LatticeThermalFEA::getLatticeTemperatureMax(void)
{
    if (!lf_FEA_SOLVED) return 0;
    return lf_temperatureMax;
}
/**********************************************************************/
double LatticeThermalFEA::getLatticeTemperatureMin(void)
{
    if (!lf_FEA_SOLVED) return 0;
    return lf_temperatureMin;
}
