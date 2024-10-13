#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#ifdef _WIN32
#include <omp.h>
#endif
using namespace std; 
typedef unsigned char byte;
#include "ThermoElasticity.h"
#include "Timer.h"
using namespace std;
#pragma warning(disable:4996)

#pragma warning( disable : 4101 )//disable warning with openmp

/**********************************************************************/
ThermoElasticity::ThermoElasticity()
{
    h_KETemplates_thermal = NULL;
    h_Kexponent = 3.0; // for pseudo-density scaling
    h_convection_coeff = 7.112e-06;
    h_sol_thermal = NULL;
    h_sol_thermalElem = NULL;
    h_radiation = NULL;
    h_radiationNonLinear = NULL;
    h_f_thermal = NULL;
    h_f_thermalWithRadiation = NULL;
    h_nodeWithMaxTemp = 0;
    h_isThereRadiation = false;
    for(int i = 0; i<HEXMESH_MAX_FACE_LABELS;i++){
        h_radiationTRef[i] = 0.0;
        h_convectionTRef[i] = 0.0;//
    }
    h_relAccuracyThermal = 1e-5;//low accuracy is sufficient for thermal problems
    h_KeNodalThermal = NULL;

    h_thermoElasticReferenceTemperature = 23;//

    h_thermo_elasticity_verbose = false;

}
/**********************************************************************/
void ThermoElasticity::enableThermoElasticityVerbose(void)
{
    writeMessageToParetoLog("ThermoElasticity: Enabling ThermoElasticity Verbose");
    h_thermo_elasticity_verbose = true;
}
/**********************************************************************/
void ThermoElasticity::disableThermoElasticityVerbose(void)
{
    writeMessageToParetoLog("ThermoElasticity: Disabling ThermoElasticity Verbose");
    h_thermo_elasticity_verbose = false;
}
/**********************************************************************/
void ThermoElasticity::setThermoElasticReferenceTemperature(double T)
{
    h_thermoElasticReferenceTemperature = T;
}
/**********************************************************************/
void ThermoElasticity::setConvectionCoefficient(double h)
{
    h_convection_coeff = h;
}
/**********************************************************************/
void ThermoElasticity::ComputeKe_thermal(double dx,double dy, double dz, double K, double Ke_thermal[8][8])
{
    // Ke_thermal for an isotropic element of arbitrary aspect ratio
    int i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j< 8; j++)
            Ke_thermal[i][j] = 0.0; //initialize Ke

    Ke_thermal[0][0]= (K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[1][0]=(K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[2][0]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[3][0]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[4][0]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[5][0]= - (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[6][0]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[7][0]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[0][1]= (K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[1][1]=(K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[2][1]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[3][1]= (K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[4][1]= - (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[5][1]= (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[6][1]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[7][1]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[0][2]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[1][2]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[2][2]=(K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[3][2]= (K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[4][2]=  - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[5][2]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[6][2]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[7][2]=- (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[0][3]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[1][3]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[2][3]= (K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[3][3]=(K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[4][3]=- (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[5][3]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[6][3]= - (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[7][3]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[0][4]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[1][4]= - (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[2][4]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[3][4]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[4][4]= (K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[5][4]=(K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[6][4]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[7][4]= (K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[0][5]=- (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[1][5]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[2][5]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[3][5]=- (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[4][5]=(K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[5][5]=(K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[6][5]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[7][5]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[0][6]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[1][6]=- (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[2][6]= (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[3][6]= - (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[4][6]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[5][6]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[6][6]=(K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
    Ke_thermal[7][6]=(K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[0][7]= - (K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(36*dx*dz);
    Ke_thermal[1][7]= - (K*dx*dz)/(36*dy) - (K*dy*(dx*dx + dz*dz))/(36*dx*dz);
    Ke_thermal[2][7]=- (K*dy*dz)/(18*dx) - (K*dx*(2*dy*dy - dz*dz))/(36*dy*dz);
    Ke_thermal[3][7]=(K*dx*dz)/(18*dy) - (K*dy*(2*dx*dx - dz*dz))/(18*dx*dz);
    Ke_thermal[4][7]=(K*dy*(dx*dx + dz*dz))/(18*dx*dz) - (K*dx*dz)/(9*dy);
    Ke_thermal[5][7]=(K*dy*(dx*dx - 2*dz*dz))/(36*dx*dz) - (K*dx*dz)/(18*dy);
    Ke_thermal[6][7]=(K*dx*(dy*dy + dz*dz))/(18*dy*dz) - (K*dy*dz)/(9*dx);
    Ke_thermal[7][7]= (K*dx*dz)/(9*dy) + (K*dy*(dx*dx + dz*dz))/(9*dx*dz);
}
/**********************************************************************/
void ThermoElasticity::computeKeNodalThermal(void)
{
    // for a single elem, the node numbering (per K matrix construction)
    // is clockwise on z = 0 plane, then next z plane
    // In a 27 node grid, the node numbering and elem numbering are
    // first along x, then along y and finally along z
    // Thus, nodes associated with elem 0 are: [0 1 4 3 9 10 13 12]
    //  nodes associated with elem 4 are: [9 10 13 12 18 19 22 21], etc
    int nodes[8][8] = {{0,     1,     4,     3,     9,    10,    13,    12},
                       {1,     2,     5,     4,    10,    11,    14,    13},
                       {3,     4,     7,     6,    12,    13,    16,    15},
                       {4,     5,     8,     7,    13,    14,    17,    16},
                       {9,     10,    13,    12,    18,    19,    22,    21},
                       {10,    11,    14,    13,    19,    20,    23,    22},
                       {12,    13,    16,    15,    21,    22,    25,    24},
                       {13,    14,    17,    16,    22,    23,    26,    25}};

    double KG[27]; // the  K matrix the center node in a block of 27 x 27 nodes
    int i,j,k,m,bits[8],dof[8];

    if (!h_voxelgrid) {
        writeMessageToParetoLog("ThermoElasticity: computeKeNodalThermal only implemented for uniform voxel grid");
        return;
    }

    // In a block of 8 elems, an elem may or may not exist.
    // Thus there are 2^8  = 256 combinations.
    // One of which can be neglected, but thrown in for completeness
    safeFree(h_KeNodalThermal);h_KeNodalThermal = NULL;

    // In a block of 8 elems, an elem may or may not exist.
    // Thus there are 2^8  = 256 combinations.
    // One of which can be neglected, but thrown in for completeness
    h_KeNodalThermal =	(double *)malloc(256*1*27*sizeof(double));

    for (i = 0; i < 256; i++) {// 256 possible cases
        convertTo8Bits(i,bits);// tells you which elem is present
        // Initialize the stiffness for center node
        for (m = 0; m < 27; m++)
            KG[m] = 0;

        for (j = 0; j < 8; j++) {// check all 8 positions
            if (bits[j] == 1) {// if elem is present
                // get the dof in 8 x 8 block for this elem
                for (k = 0; k < 8; k++)
                    dof[k] = nodes[j][k];

                for (k = 0; k < 8; k++) {//assemble this element contribution to center node
                    if (dof[k] !=  13)
                        continue;// only assemble the center node
                    for (m = 0; m < 8; m++)
                        KG[dof[m]] += h_KETemplates_thermal[8*k+m];
                }
            }
        }
        // Now enter the assembled sub matrix
        for (k =0; k < 27; k++)
            h_KeNodalThermal[27*i + k] = KG[k];
    }
}
/**********************************************************************/
bool ThermoElasticity::isNodeFree_thermal(int node)
{
    return(h_free_thermal[node]);
}
/**********************************************************************/
void ThermoElasticity::enforceDirichlet_thermal(double *x)
{
    int i;
#pragma omp parallel for 
    for (i =0; i < h_nNodes; i++)
        if (!h_free_thermal[i])
            x[i] = h_dirichlet_thermal[i];
}
/**********************************************************************/
void ThermoElasticity::enforceDirichletZero_thermal(double *x)
{
    int i;
#pragma omp parallel for 
    for (i =0; i < h_nNodes; i++)
        if (!h_free_thermal[i])
            x[i] = 0;
}
/**********************************************************************/
void ThermoElasticity::initializeRandomVector_thermal(double *x)
{
    int i;
    for (i =0; i < h_nNodes; i++) {
        x[i] = (h_free_thermal[i])*0 + h_dirichlet_thermal[i]*(!h_free_thermal[i]);// T0 if Dirichlet else 0.0
        //x[i] = 360.0;
    }
    //
}
/**********************************************************************/
void ThermoElasticity::getElemNodesTemperature(int elem,double temperature[8])
{
    int node;

    if (h_sol_thermal == NULL) return; // thermal problem has not been solved
    for (int m = 0;m < 8; m++) {
        node = h_elems[(h_nElems)*m + elem];
        temperature[m] = h_sol_thermal[node];

    }
}
/**********************************************************************/
bool ThermoElasticity::getTemperatureAt(double pt[3],double *temperature)
{
    int m,nodes[8],elem;
    double xietamu[3],tnode[8],N[8],x[8],y[8],z[8];
    *temperature = 0;
    if (!h_voxelgrid) return false;

    if (h_sol_thermal == NULL) return false; // thermal problem has not been solved

    // first find the element
    elem = findNonVoidElemClosestTo(pt[0],pt[1],pt[2],3);// within relative tolerance
    if ((elem < 0) || (elem >= h_nElems)){
        return false;
    }
    getElemNodes(elem,nodes);

    *temperature = 0;
    for (int m = 0;m < 8; m++) {
        x[m] = h_x[nodes[m]];
        y[m] = h_y[nodes[m]];
        z[m] = h_z[nodes[m]];
        tnode[m] = h_sol_thermal[nodes[m]];
    }
    // find the normalized coordinates of point
    xietamu[0] = (2*pt[0] - (x[1]+x[0]))/(x[1] - x[0]);
    xietamu[1] = (2*pt[1] - (y[2]+y[1]))/(y[2] - y[1]);
    xietamu[2] = (2*pt[2] - (z[4]+z[0]))/(z[4] - z[0]);

    for (int m =0; m <3; m++)
        xietamu[m] = MAX(-1,MIN(xietamu[m],1));

    computeN(xietamu[0],xietamu[1],xietamu[2],N);
    *temperature = 0;
    for ( m = 0;m < 8; m++)
        *temperature += N[m]*tnode[m];

    return true;
}
/**********************************************************************/
bool ThermoElasticity::getTemperatureInAllNodes(double *temperature)
{
    if (h_sol_thermal == NULL) return false; // thermal problem has not been solved
    for (int node = 0; node < h_nNodes; node++)
        temperature[node] = h_sol_thermal[node];

    return true;
}
/**********************************************************************/
bool ThermoElasticity::getTemperatureInAllElems(double *temperature)
{
    if (h_sol_thermal == NULL) return false; // thermal problem has not been solved
    for (int elem = 0; elem < h_nElems; elem++){
        temperature[elem] = h_sol_thermalElem[elem];
    }
    return true;
}
/**********************************************************************/
double ThermoElasticity::getTemperatureInElem(int elem)
{
    if (h_sol_thermal == NULL) return 0; // thermal problem has not been solved
    if ((elem < 0) || (elem >= h_nElems)) return 0;
    return h_sol_thermalElem[elem];
}
/**********************************************************************/
bool ThermoElasticity::getTemperatureGradientInElem(int elem,double gradient[3])
{

    if (h_sol_thermal == NULL) return false; // thermal problem has not been solved
    if ((elem < 0) || (elem >= h_nElems)) return false;
    double J00,J11,J22,gradNxi[8],gradNeta[8],gradNmu[8],nodeTemperature[8];

    computeGrad_H8(0,0,0, gradNxi, gradNeta, gradNmu);
    J00 = 2./h_dx;//Jinv
    J11 = 2./h_dy;
    J22 = 2./h_dz;
    gradient[0] = gradient[1] = gradient[2] = 0.0;
    getElemNodesTemperature(elem,nodeTemperature);
    for (int i = 0; i < 8; i++) {// contribution from all 8 nodes
        gradient[0] += J00*gradNxi[i]*nodeTemperature[i];// ux
        gradient[1] += J11*gradNeta[i]*nodeTemperature[i];// vy
        gradient[2] += J22*gradNmu[i]*nodeTemperature[i];// wz
    }

    return true;
}
/**********************************************************************/
int ThermoElasticity::allocateMemory_thermal(void)
{
    releaseCPUMemory_thermal();

    h_free_thermal =				(bool *)malloc((h_nNodes)*sizeof(bool));
    h_dirichlet_thermal =			(double *)malloc((h_nNodes)*sizeof(double));
    h_f_thermal =					(double *)malloc((h_nNodes)*sizeof(double));
    h_f_thermalWithRadiation =		(double *)malloc((h_nNodes)*sizeof(double));
    h_convection =					(double *)malloc((h_nNodes)*sizeof(double));
    h_radiation =					(double *)malloc((h_nNodes)*sizeof(double));
    h_radiationNonLinear =			(double *)malloc((h_nNodes)*sizeof(double));
    h_sol_thermal =					(double *)malloc((h_nNodes)*sizeof(double));

    h_sol_thermalElem =				(double *)malloc((h_nElems)*sizeof(double));// useful for thermoelastic and nonlinear problems


    memset(h_free_thermal,1,h_nNodes*sizeof(bool));
    memset(h_dirichlet_thermal,0,h_nNodes*sizeof(double));
    memset(h_f_thermal,0,h_nNodes*sizeof(double));
    memset(h_f_thermalWithRadiation,0,h_nNodes*sizeof(double));
    memset(h_convection,0,h_nNodes*sizeof(double));
    memset(h_radiation,0,h_nNodes*sizeof(double));
    memset(h_radiationNonLinear,0,h_nNodes*sizeof(double));
    memset(h_sol_thermal,0,h_nNodes*sizeof(double));

    memset(h_sol_thermalElem,0,h_nElems*sizeof(double));

    if (h_thermo_elasticity_verbose)
        writeMessageToParetoLog("ThermoElasticity: Allocated Node Memory");

    return 1;
}
/**********************************************************************/
void ThermoElasticity::resetBC_thermal(void)
{
    h_isThereRadiation = false;
    memset(h_free_thermal,true,h_nNodes*sizeof(bool));
    memset(h_dirichlet_thermal,0,h_nNodes*sizeof(double));
    memset(h_f_thermal,0,h_nNodes*sizeof(double));
    memset(h_f_thermalWithRadiation,0,h_nNodes*sizeof(double));
    memset(h_convection,0,h_nNodes*sizeof(double));
    memset(h_radiation,0,h_nNodes*sizeof(double));
    memset(h_radiationNonLinear,0,h_nNodes*sizeof(double));

    for(int i = 0; i<HEXMESH_MAX_FACE_LABELS;i++)
        h_radiationTRef[i] = 0.0;
}
/**********************************************************************/
void ThermoElasticity::clearThermalConvectionLoad()
{
    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if (!isNodeWithinEvolvingMesh(node)) {
            h_f_thermal[node] = 0;// remove load.
            h_convection[node] = 0;//remove convection coefficient.
        }
        if ( ( abs(h_f_thermal[node]) > 0) && (h_convection[node] == 0) ) continue;// flux prescribed, skip
        h_f_thermal[node] = 0;//else remove load.
        h_convection[node] = 0;//remove convection coefficient.
    }
}
/**********************************************************************/
void ThermoElasticity::applyConvectionOnAllExposedNodes()
{
    double cornerArea,a,pt[3];
    int count1;
    a = (h_dx+h_dy+h_dz)/3.0;// average edge length
    cornerArea = 3*(a/2*a/2);// area exposed around a node ... estimate
    count1 = 0;

    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if ( ( abs(h_f_thermal[node]) > 0) && (h_convection[node] == 0) ) continue;// flux prescribed, skip
        if ((!isNodeOnEvolvingBoundary(node))) continue;
        h_f_thermal[node] = (double) cornerArea*h_convection_coeff*h_thermoElasticReferenceTemperature;
        h_convection[node] = (double) cornerArea*h_convection_coeff;
        count1++;
    }
}
/**********************************************************************/
void ThermoElasticity::applyScaledConvectionOnZ0Surface(double scale)
{
    // for Kolher project
    double cornerArea,a,pt[3];
    int count1;
    a = (h_dx+h_dy+h_dz)/3.0;// average edge length
    cornerArea = 3*(a/2*a/2);// area exposed around a node ... estimate
    count1 = 0;

    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if ((!isNodeOnEvolvingBoundary(node))) continue;
        getNodeXYZ(node,pt);
        if (fabs(pt[2]) <h_zMin + h_dz/2){
            h_f_thermal[node] = scale*(double) cornerArea*h_convection_coeff*h_thermoElasticReferenceTemperature;
            h_convection[node] = scale*(double) cornerArea*h_convection_coeff;;//
        }
    }
}
/**********************************************************************/
void ThermoElasticity::applyDirichletOnBaseNodes(double T0)
{
    double pt[3];
    int count = 0;
    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if ((!isNodeOnEvolvingBoundary(node))) continue;
        getNodeXYZ(node,pt);
        if (fabs(pt[2]) < h_dz/5) {
            h_dirichlet_thermal[node] = T0;
            h_free_thermal[node] = false;
            count++;
        }
    }
}
/**********************************************************************/
void ThermoElasticity::applyThermalFluxOnAllExposedNodes(double q)
{
    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if ((!isNodeOnEvolvingBoundary(node))) continue;
        if (h_sol_thermal[node] < h_thermoElasticReferenceTemperature)
            h_f_thermal[node] = 0;
        else
            h_f_thermal[node] = q;
    }
}
/**********************************************************************/
void ThermoElasticity::setBCOnSingleNode_thermal(int node,int BCType,double BCValue) 
{
    //used when exact field is known
    if ((node <0) || (node >= h_nNodes)) {
        writeMessageToParetoLog("ThermoElasticity: Incorrect node %d ",node);
        return;
    }
    h_free_thermal[node] = true;

    if (BCType == THERMAL_DIRICHLET) {
        h_free_thermal[node] = false;
        h_dirichlet_thermal[node] = BCValue;
    }
    else if (BCType == THERMAL_NEUMANN)
        h_f_thermal[node] = (double) BCValue;
    else
        writeMessageToParetoLog("ThermoElasticity: Incorrect BC type in setBCOnSingleNode_thermal");

}
/**********************************************************************/
void ThermoElasticity::setBCOnNodes_thermal(int nodeLabel,int myBCtype_thermal,double myBC_area,double myBCValue_thermal,double myBCref_thermal)
{
    int nCornerNodes=0,nEdgeNodes=0,nFaceNodes=0;
    double sigma;

    if (h_thermo_elasticity_verbose)	writeMessageToParetoLog("ThermoElasticity: setBCOnNodes_thermal");

    for (int node = 0; node < h_nNodes; node++) {
        if ((h_nodeLabels[node] == nodeLabel)) {// node lies on a face with the given face ID or label
            if (isNodeOnCorner(node))
                nCornerNodes++;
            else if (isNodeOnEdge(node))
                nEdgeNodes++;
            else
                nFaceNodes++;
        }
    }
    if (nFaceNodes == 0) return;
    if ( (myBCtype_thermal == THERMAL_NEUMANN) && (myBCValue_thermal == 0)) return;


    double eta = (nFaceNodes + 0.5*nEdgeNodes + 0.25*nCornerNodes)/(nFaceNodes);//correction factor

    if(myBCtype_thermal == THERMAL_RADIATION){
        sigma = BOLTZMAN;
        h_isThereRadiation = true;
        h_radiationTRef[nodeLabel] = myBCref_thermal;
    }
    if(myBCtype_thermal == THERMAL_CONVECTION)
        h_convectionTRef[nodeLabel] = myBCref_thermal;

    for (int node = 0; node < h_nNodes; node++) {
        if (h_nodeLabels[node] == nodeLabel) {// node lies on a face with the given face ID or label
            if (myBCtype_thermal == THERMAL_DIRICHLET) {
                h_free_thermal[node] = false; //0:fix; 1:free
                h_dirichlet_thermal[node] = myBCValue_thermal;
            }
            else if (myBCtype_thermal == THERMAL_NEUMANN) {
                if (isNodeOnCorner(node))
                    h_f_thermal[node] += (double) 0.25*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
                else if (isNodeOnEdge(node))
                    h_f_thermal[node] += (double) 0.5*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
                else
                    h_f_thermal[node] += (double) eta*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
            }
            else if (myBCtype_thermal == THERMAL_CONVECTION) {
                if (isNodeOnCorner(node))
                    h_f_thermal[node] += (double) 0.25*myBC_area*myBCValue_thermal*myBCref_thermal/h_NumNodesWithLabel[nodeLabel];
                else if (isNodeOnEdge(node))
                    h_f_thermal[node] += (double) 0.5*myBC_area*myBCValue_thermal*myBCref_thermal/h_NumNodesWithLabel[nodeLabel];
                else
                    h_f_thermal[node] += (double) eta*myBC_area*myBCValue_thermal*myBCref_thermal/h_NumNodesWithLabel[nodeLabel];

                // also precompute the convection vector for later, the T_ref is removed (and later replaced with T)
                if (isNodeOnCorner(node))
                    h_convection[node] += (double) 0.25*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
                else if (isNodeOnEdge(node))
                    h_convection[node] += (double) 0.5*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
                else
                    h_convection[node] += (double) eta*myBC_area*myBCValue_thermal/h_NumNodesWithLabel[nodeLabel];
            }
            else if (myBCtype_thermal == THERMAL_RADIATION) {
                if (isNodeOnCorner(node))
                    h_radiation[node] += (double) 0.25*myBC_area*myBCValue_thermal*sigma/h_NumNodesWithLabel[nodeLabel];//
                else if (isNodeOnEdge(node))
                    h_radiation[node] += (double) 0.5*myBC_area*myBCValue_thermal*sigma/h_NumNodesWithLabel[nodeLabel];//
                else
                    h_radiation[node] += (double) eta*myBC_area*myBCValue_thermal*sigma/h_NumNodesWithLabel[nodeLabel];//

            }
            else
                ;// invalid
        }
    }
}
/**********************************************************************/
void	ThermoElasticity::computeRadiationNonLinear(bool JacobianVersion) 
{
    // compute h_r*(T^2+Tref^2)*(T+TRef)
    // and h_f_thermalWithRadiation
    double T,TRef;
    int i,nodeLabel;

    if (h_thermo_elasticity_verbose) 	writeMessageToParetoLog("ThermoElasticity: computeRadiationNonLinear");
    if (!h_isThereRadiation) return;

#pragma omp parallel for private(i,nodeLabel,T,TRef)
    for ( i =0; i < h_nNodes; i++) {
        if (!doesNodeExist(i)) {
            h_f_thermalWithRadiation[i] = 0.0;
            h_radiationNonLinear[i] =  0.0;
            continue;
        }
        nodeLabel = h_nodeLabels[i];
        T = h_sol_thermal[i];
        TRef = h_radiationTRef[nodeLabel];

        if (fabs(h_radiation[i]) == 0) {
            h_f_thermalWithRadiation[i] = h_f_thermal[i];
            h_radiationNonLinear[i] =  0.0;
            continue;//no radiation on this node
        }

        if (JacobianVersion){
            h_radiationNonLinear[i] = h_radiation[i]*4*T*T*T;//derivative, used in the K Matrix
        }
        else{
            h_radiationNonLinear[i] = h_radiation[i]*(T*T+TRef*TRef)*(T+TRef);// also used in the K Matrix
            h_f_thermalWithRadiation[i] = h_radiationNonLinear[i]*TRef;
        }
    }
}
/**********************************************************************/
void ThermoElasticity::KuCPUNodeBased_thermal(double *x,double *y, double transientFactor)
// y = K*x
{
    // Carry out K*x on CPU, also enforce Dirichlet boundary conditions
    register int node2,j,node,KEIndex;
    register double KE,TResult=0.0;

#pragma omp parallel for private(KEIndex,node2,j,node,KE,TResult)
    for ( node = 0; node < h_nNodes; node++) {
        TResult=0.0;
        if ((doesNodeExist(node)) &&  (h_free_thermal[node])) {// process only if the dof  is free
            KEIndex = h_nodeSignature[node];// from 0 to 255
            for (j =0; j < 27; j++) {//
                node2 = h_neighborNodes[j*h_nNodes + node];// process neighbor
                if (node2 == -1) continue;
                KE = h_KeNodalThermal[27*KEIndex + j];
                TResult += KE*x[node2];
            }
        }
        y[node] = TResult;
        // add the h_convection component
        y[node] += transientFactor*h_convection[node]*x[node];
        // add the h_radiation component
        y[node] +=  transientFactor*h_radiationNonLinear[node]*x[node];//
        // write back the result
        y[node] = (h_free_thermal[node])*y[node] ;// Dirichlet condition is enforced
    }
}
/**********************************************************************/
void ThermoElasticity::KuCPU_thermal(double *x,double *y,double transientFactor)
{
    if ((h_nodeBasedAssembly) && (h_voxelgrid))
        KuCPUNodeBased_thermal(x,y,transientFactor);
    else
        KuCPUElemBased_thermal(x,y,transientFactor);
}

/**********************************************************************/
void ThermoElasticity::KuCPUElemBased_thermal(double *x,double *y,double transientFactor) 
// y = K*x
{
    // Carry out K*x on CPU, also enforce Dirichlet boundary conditions
    register int i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,row,nTemplateIDs;
    register double KE, TResult=0.0,TVal,KScale;

    KScale = 1.0;//default
    nTemplateIDs = getNumTemplateIDs();
#pragma omp parallel for private(i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,KE,TResult,TVal,row)
    for ( node = 0; node < h_nNodes; node++) {
        TResult= 0.0;
        if (!doesNodeExist(node)){
            y[node] = 0;
            continue;
        }
        if ((doesNodeExist(node)) &&  (h_free_thermal[node])) {// process only if one of the dof  is free
            maxElem = h_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElem - nodeIndex*MAXELEMS;
                if (getElemPseudoDensity(elem) < 0.5) continue;
                KScale = pow(getElemPseudoDensity(elem),h_Kexponent);// K = K0*(rho)^n;
                elemID = getElemTemplateID(elem);
                row = 8*nodeIndex; //row index in KE matrix corresponding to each node (dofs)
                for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
                    node2 = h_elems[(h_nElems)*i + elem];
                    TVal = x[node2];

                    KEindex = (row + i)*(nTemplateIDs) + elemID;
                    KE = KScale*h_KETemplates_thermal[KEindex];
                    TResult += KE*TVal;
                }
            }
        }
        y[node] = TResult;
        // add the h_convection component
        y[node] += transientFactor*h_convection[node]*x[node];
        // add the h_radiation component
        y[node] +=  transientFactor*h_radiationNonLinear[node]*x[node];//
        // write back the result
        y[node] = (h_free_thermal[node])*y[node] ;// Dirichlet condition is enforced
    }
}
/**********************************************************************/
void ThermoElasticity::reactionFluxThermal(double *r,double transientFactor) 
// y = K*x
{
    // Carry out K*x on CPU, also enforce Dirichlet boundary conditions
    register int i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,row,nTemplateIDs;
    register double KE, fInternal=0.0,TVal;

    nTemplateIDs = getNumTemplateIDs();
#pragma omp parallel for private(i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,KE,fInternal,TVal,row)
    for ( node = 0; node < h_nNodes; node++) {
        fInternal=0.0;
        if ((doesNodeExist(node)) &&  (h_free_thermal[node])) {// process only if one of the dof  is free
            maxElem = h_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElem - nodeIndex*MAXELEMS;
                if (getElemPseudoDensity(elem) < h_pseudoDensityCutoff)// element exists but is void (during topology optimization)
                    continue;
                elemID = getElemTemplateID(elem);
                row = 8*nodeIndex; //row index in KE matrix corresponding to each node (dofs)
                for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
                    node2 = h_elems[(h_nElems)*i + elem];
                    TVal = h_sol_thermal[node2];

                    KEindex = (row + i)*(nTemplateIDs) + elemID;
                    KE = h_KETemplates_thermal[KEindex];
                    fInternal += KE*TVal;
                }
            }
        }
        // add the h_convection component
        fInternal += transientFactor*h_convection[node]*h_sol_thermal[node];
        // add the h_radiation component
        fInternal +=  transientFactor*h_radiationNonLinear[node]*h_sol_thermal[node];//
        // write back the result
        fInternal = (h_free_thermal[node])*fInternal ;// Dirichlet condition is enforced
        r[node] = fInternal - h_f_thermal[node];
    }
}
/**********************************************************************/
void  ThermoElasticity::releaseCPUMemory_thermal(void)
{
    if (h_thermo_elasticity_verbose)	writeMessageToParetoLog("ThermoElasticity: releasing CPU memory");
    // CPU memory deallocation

    return;

    safeFree(h_sol_thermal);h_sol_thermal = NULL;
    safeFree(h_free_thermal);h_free_thermal = NULL;
    safeFree(h_dirichlet_thermal);h_dirichlet_thermal = NULL;
    safeFree(h_f_thermal);h_f_thermal = NULL;
    safeFree(h_f_thermalWithRadiation);h_f_thermalWithRadiation = NULL;

    safeFree(h_convection);h_convection = NULL;
    safeFree(h_radiation);h_radiation = NULL;
    safeFree(h_radiationNonLinear);h_radiationNonLinear = NULL;

    safeFree(h_KETemplates_thermal);h_KETemplates_thermal = NULL;
    safeFree(h_KeNodalThermal);h_KeNodalThermal = NULL;

    safeFree(h_sol_thermalElem);h_sol_thermalElem = NULL;

}
/**********************************************************************/
void ThermoElasticity::copy_thermal(double *x, double *y)
{
#pragma omp parallel for 
    for (int i =0; i < h_nNodes; i++)
        y[i] = x[i];
}
/**********************************************************************/
double ThermoElasticity::Norm_thermal(double *x)
{// Assumes that x is of length h_nNodes, 1 dof at each node
    double alpha =0.;
    int i;
#pragma omp parallel for reduction (+: alpha )
    for ( i =0; i < h_nNodes; i++)
        alpha += x[i]*x[i];
#pragma omp barrier 
    return sqrt(alpha);
}
/**********************************************************************/
double ThermoElasticity::errorNorm_thermal(double *x,double *y)
{// Assumes that x and y are of length h_nNodes, 1 dof at each node
    double alpha =0.;
    int i;
#pragma omp parallel for reduction (+: alpha )
    for ( i =0; i < h_nNodes; i++)
        alpha += pow(x[i]-y[i],2.0);
#pragma omp barrier 
    return sqrt(alpha);
}
/**********************************************************************/
double ThermoElasticity::dotprod_thermal(double *x, double *y)
{// Assumes that x and y are of length h_nNodes
    double alpha;
    int i;

    alpha = 0.0;
#pragma omp parallel for reduction (+: alpha )
    for ( i =0; i < h_nNodes; i++)
        alpha += x[i]*y[i];
#pragma omp barrier 
    return alpha;
}
/**********************************************************************/
double ThermoElasticity::normalize_thermal(double *x, double *y)
{// y = x/||x||
    double nrm;

    nrm = Norm_thermal(x);
#pragma omp parallel for
    for (int i =0; i < h_nNodes; i++)
        y[i] = x[i]/nrm;

    return nrm;
}
/**********************************************************************/
void ThermoElasticity::zeaxpy_thermal(double alpha,double beta,double *z,double *x, double *y)
// z = alpha*x + beta*y (vector)
{
#pragma omp parallel for
    for (int i =0; i < h_nNodes; i++)
        z[i] = alpha*x[i] + beta*y[i];
}
/**********************************************************************/
bool ThermoElasticity::isTemperatureFixedOnNode(int node)
{
    if (h_free_thermal[node])
        return false;
    else
        return true;
}
/**********************************************************************/
bool ThermoElasticity::isThereHeatFluxOnNode(int node)
{
    if (fabs(h_f_thermal[node]) > 0)
        return true;
    else
        return false;
}

/**********************************************************************/
void ThermoElasticity::ScaleByVector_thermal(double *z,double *x, double *y)
// z = x./y
{
#pragma omp parallel for
    for (int i =0; i < h_nNodes; i++)
        z[i] = x[i]/y[i];
}
/**********************************************************************/
void ThermoElasticity::axpy_thermal(double alpha, double *x, double *y)
{
    // y = y + alpha*x
#pragma omp parallel for
    for (int i =0; i < h_nNodes; i++)
        y[i] += alpha*x[i];
}
/**********************************************************************/
void ThermoElasticity::scale_thermal(double alpha, double *x)
{
#pragma omp parallel for
    for (int i =0; i < h_nNodes; i++)
        x[i] *= alpha;
}
/**********************************************************************/
void ThermoElasticity::initializeSolution_thermal(void)
{
    if(h_sol_thermal == NULL) return;
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: initializeSolution()");
    initializeRandomVector_thermal(h_sol_thermal);

    //copy_thermal(h_dirichlet_thermal,h_sol_thermal);
}
/**********************************************************************/
double ThermoElasticity::computeElemInnerProductThermal(int elem,double u1[8],double u2[8])
// Compute u1'*Ke*u2
{
    int i,j,k,elemID,nTemplateIDs;
    double temp[8];

    double result;
    elemID = getTemplateId(elem);
    result = 0;
    k = 0;
    nTemplateIDs = getNumTemplateIDs();
    for (i =0; i < 8; i++) {
        temp[i] = 0;
        for( j = 0; j < 8;j++) {
            temp[i] += h_KETemplates_thermal[k*(nTemplateIDs) + elemID]*u2[j];
            k++;
        }
    }
    for( i = 0; i < 8;i++)
        result += u1[i]*temp[i];
    return result;
}
/**********************************************************************/
double ThermoElasticity::computeCompliance_thermal(void)
{
    int i;
    double J=0;

#pragma omp parallel for reduction (+: J )
    for (i =0; i < h_nNodes; i++) {
        if (!doesNodeExist(i)) continue;
        J += h_sol_thermal[i]*h_f_thermal[i];
    }
    return J;
}
/**********************************************************************/
void	ThermoElasticity::computeResidual(double *currentSol,double *residual)
{
    KuCPU_thermal(currentSol,residual);// residual = K*sol -f, includes convection and radiation
    axpy_thermal(-1,h_f_thermalWithRadiation,residual);
}

/**********************************************************************/
bool	ThermoElasticity::solveThermalProblem(bool resetTemperature)
{
    int nCGIter;
    double transientFactor = 1;
    if (h_thermo_elasticity_verbose) 	writeMessageToParetoLog("ThermoElasticity: Solving Thermal Problem");
    // solve once for all BC
    if (resetTemperature) // optional, especially during topology optimization
        initializeSolution_thermal();
    initializeKEThermal(); //initilize h_KETemplates_thermal

    if ((h_voxelgrid) &&  (h_nodeBasedAssembly))
        computeKeNodalThermal();

    if (h_isThereRadiation ) disableDeflation();// bug in thermal deflation with radiation
    if (ws_deflationType !=  DEFLATION_NONE) {// deflation
        CreateDeflationGroups();
        if (!sparseWKWThermal(transientFactor))
            disableDeflation();
    }
    if (h_isThereRadiation) {// at least one radiation BC
        //computeRadiationNonLinear();
        //if (!performCGThermal(h_f_thermalWithRadiation,h_sol_thermal,&nCGIter)) return false;
        //solveThermalProblemPicard(5);
        solveThermalProblemNewtonRaphson(100);
    }
    else
        if (!performCGThermal(h_f_thermal,h_sol_thermal,&nCGIter)) return false;

    computeElementTemperature();
    computeTemperatureMax();
    return true;
}
/**********************************************************************/
void ThermoElasticity::computeElementTemperature(void)
{
    // now compute temperature at center of each element
    int node,counter;
    double TElem;

    for (int elem = 0; elem < h_nElems; elem++) {
        TElem = 0;
        counter = 0;
        for (int m = 0;m < 8; m++) {
            node = h_elems[(h_nElems)*m + elem];
            if (!doesNodeExist(node)) continue;
            TElem += h_sol_thermal[node];
            counter++;
        }
        if (counter > 0)
            h_sol_thermalElem[elem] = (TElem/counter);
    }
}
/**********************************************************************/
bool	ThermoElasticity::solveThermalProblemNewtonRaphson(int maxIter)
{
    double normErr,*residual,*deltaSol,normDelta;
    int iter,nCGIter;

    deltaSol = (double *)malloc((h_nNodes)*sizeof(double));
    residual = (double *)malloc((h_nNodes)*sizeof(double));

    memset(deltaSol,0,(h_nNodes)*sizeof(double));
    memset(residual,0,(h_nNodes)*sizeof(double));
    if (h_thermo_elasticity_verbose) 	writeMessageToParetoLog("ThermoElasticity: Newton Raphson Solver");
    for ( iter =1; iter < maxIter ; iter++){
        computeRadiationNonLinear();// compute the nonlinear h_radiation term and nonlinear thermal force
        computeResidual(h_sol_thermal,residual);
        computeRadiationNonLinear(true);// nonlinear h_radiation for Jacobian
        performCGThermal(residual,deltaSol,&nCGIter); //using Jacobian
        axpy_thermal(-1,deltaSol,h_sol_thermal);
        enforceDirichlet_thermal(h_sol_thermal);
        normDelta =Norm_thermal(deltaSol);
        normErr = normDelta/(Norm_thermal(h_sol_thermal)+1e-16);
        writeMessageToParetoLog("ThermoElasticity: Newton Raphson Iteration %d; norm(deltaT): %e normErr: %e normRes: %e", iter,normDelta,normErr,Norm_thermal(residual));
        if ((iter > 1) && (normErr < h_relAccuracyThermal)) break;
    }
    free(deltaSol);
    free(residual);
    if (iter >= maxIter) {
        writeMessageToParetoLog("ThermoElasticity: Newton Raphson did not converge; Iteration %d; norm(deltaT): %e normErr: %e normRes: %e", iter,normDelta,normErr,Norm_thermal(residual));
        return false;
    }
    else
        return true;
}
/**********************************************************************/
bool	ThermoElasticity::solveThermalProblemPicard(int maxIter)
{
    double *solOld,*diff,normErr,*residual;
    int nCGIter;

    solOld = (double *)malloc(h_nNodes*sizeof(double));
    diff = (double *)malloc(h_nNodes*sizeof(double));
    residual = (double *)malloc(h_nNodes*sizeof(double));
    memset(solOld,0,h_nNodes*sizeof(double));
    for (int iter =1; iter < maxIter; iter++){
        computeResidual(h_sol_thermal,residual);
        computeRadiationNonLinear();// compute the nonlinear h_radiation term
        performCGThermal(h_f_thermalWithRadiation,h_sol_thermal,&nCGIter);
        zeaxpy_thermal(-1,1,diff,h_sol_thermal,solOld);
        normErr = Norm_thermal(diff)/(Norm_thermal(h_sol_thermal)+1e-16);
        writeMessageToParetoLog("ThermoElasticity: Picard Iteration %d; normErr: %e; normRes: %e", iter,normErr,Norm_thermal(residual));
        if (normErr < h_relAccuracyThermal) break;
        copy_thermal(h_sol_thermal,solOld);
    }
    free(solOld);
    free(diff);
    free(residual);

    return true;
}
/**********************************************************************/
bool ThermoElasticity::pushThermalSolutionToTessellation(void)
{
    double pt[3],temperature;

    if (h_numTesselationPoints == 0) {
        writeMessageToParetoLog("Tesselation: triangles = %d, points = %d ",h_numTesselationTriangles,h_numTesselationPoints);
        return false;
    }

    for (int i = 0; i < h_numTesselationPoints; i++) {
        pt[0] =h_tessellationXYZ[3*i];
        pt[1] =h_tessellationXYZ[3*i+1];
        pt[2] =h_tessellationXYZ[3*i+2];
        getTemperatureAt(pt,&temperature);
        h_tessellationTemperature[i] = temperature;
    }

    return true;
}
//*********************************************
bool ThermoElasticity::getTessellationThermalSolutionOfTriangle(int tri,double temperature[3])
{
    int nodes[3];
    if ((tri < 0) || (tri >= h_numTesselationTriangles)) return false;

    for (int i = 0; i < 3; i++){
        nodes[i] = h_tessellationTriangles[3*tri+i];
        temperature[i] = h_tessellationTemperature[nodes[i]];
    }

    return true;
}
/**********************************************************************/
void ThermoElasticity::initializeKEThermal(void)
{
    int templateElem,nTemplateIDs,templateID;
    double Ke_thermal[8][8],thermalConductivity;

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: initializeKEThermal");
    safeFree(h_KETemplates_thermal);
    nTemplateIDs = getNumTemplateIDs();
    h_KETemplates_thermal =	(double *)malloc((nTemplateIDs)*8*8*sizeof(double));
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: nTemplateIDs %d",nTemplateIDs);

    for (templateElem = 0; templateElem < nTemplateIDs; templateElem++) {
        for (int elem = 0; elem < h_nElems; elem++){// find an elem with that ID, BAD ALGORITHM!!!
            templateID = getTemplateId(elem);
            if (templateID == templateElem){
                thermalConductivity = getThermalConductivityOfElem(elem);
                break;
            }
        }

        if (h_voxelgrid)
            ComputeKe_thermal(h_dx,h_dy,h_dz,thermalConductivity,Ke_thermal);
        int k =0;
        for (int i =0; i < 8; i++) {
            for(int j = 0; j < 8;j++) {
                h_KETemplates_thermal[k*(nTemplateIDs) + templateElem] = (double) Ke_thermal[i][j];
                k++;
            }
        }
    }
}
/**********************************************************************/
bool	ThermoElasticity::performCGThermal(double *f,double *u,int *niter,double transientFactor ) 
{
    bool success;

    success = false;
    if (ws_deflationType == DEFLATION_NONE) {//no deflation
        if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: In performCGThermal without deflation");
        success = performSimpleCGThermal(f,u,niter);
    }
    else {
        if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Attempting performCGThermal with deflation");
        if (isPreconditionerEnabled()){
            if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Attempting performCGThermal with preconditioned deflation");
            success = performPreconditionedDeflatedCGThermal(f,u, niter);
        }
        if (!success)
            success = performDeflatedCGThermal(f,u, niter);
        if (!success)
            success = performSimpleCGThermal(f,u,niter);
    }
    return success;
}
/**********************************************************************/
bool	ThermoElasticity::performSimpleCGThermal(double *f,double *u,int *niter,double transientFactor ) 
{
    // Solves Kx = f
    // Assumes K and f have been defined, and Dirichlet conditions have been set
    // Also assumes CG iterations and tolerances have been defined.
    double rho,alpha,beta,rho_new,normr,normr0;
    double *r,*p,*a;
    bool success;
    Timer timer;

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Performing Simple CG on CPU");
    if (h_thermo_elasticity_verbose) {
        if ((h_nodeBasedAssembly) && (h_voxelgrid))
            writeMessageToParetoLog("ThermoElasticity: Node-based KU");
        else
            writeMessageToParetoLog("ThermoElasticity: Elem-based KU");
    }
    // use temporary CPU variables
    r = (double *) malloc(h_nNodes*sizeof(double));
    p = (double *) malloc(h_nNodes*sizeof(double));
    a = (double *) malloc(h_nNodes*sizeof(double));

    safeFree(h_cgNorm);
    h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));

    // Assume u  has been initialized (see CPU initialization)
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: normf = %e, norm(sol) =%e",Norm_thermal(f),Norm_thermal(u));
    KuCPU_thermal(u,a,transientFactor);// a = K*sol//
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
    normr = sqrt(dotprod_thermal(r,r));
    normr0 = normr;
    copy_thermal(r,a);// Set a = r
    copy_thermal(a,p);// Set p = a
    rho = dotprod_thermal(a,r);
    *niter = 1;     // Init counter for number of iterations
    success = true;
    timer.start();
    while ( (fabs(normr0) > 0) && ((*niter < 10) || (normr > h_relAccuracyThermal*normr0)))  {// Test break condition
        KuCPU_thermal(p,a,transientFactor);// a = K*p;//
        beta = dotprod_thermal(a,p);
        alpha = rho/beta;

        h_cgNorm[*niter-1] = log10(normr/normr0);
        axpy_thermal(alpha,p,u);//u = u + alpha*p;
        axpy_thermal(-alpha,a,r);//r = r - alpha*a;
        normr = sqrt(dotprod_thermal(r,r));
        if (*niter == 1) normr0 = normr;//update for consistent scaling convergence
        copy_thermal(r,a);// Set a = r
        rho_new =  dotprod_thermal(a,r);//rho = a'*r
        zeaxpy_thermal(1,(rho_new/(rho)),p,a,p);//p = a + rho_new/rho * p;
        rho = rho_new;
        h_nCGItersActual = *niter;
        (*niter)++;
        if (normr >1e3*normr0)  {
            writeMessageToParetoLog("ThermoElasticity: Diverging at %d iter, resErr = %e ",*niter,normr);
            success = false;
            break;
        }
        if (_isnan(fabs(normr))){
            writeMessageToParetoLog("ThermoElasticity: Diverging at %d iter, resErr = %e ",*niter,normr);
            success = false;
            break;
        }
        if ((h_thermo_elasticity_verbose) && (*niter%100 ==0))
            writeMessageToParetoLog("ThermoElasticity: Simple CG %d: %e ",*niter,normr/normr0);
        if (*niter == h_CGMaxIter)  {
            writeMessageToParetoLog("ThermoElasticity: Did not converge in %d iter, resErr = %e ",*niter,normr);
            success = false;
            break;
        }
    }

    if (h_thermo_elasticity_verbose)  writeMessageToParetoLog("ThermoElasticity: CG on CPU; nIter = %d, RelResErr = %e ",*niter,normr/(normr0+1e-15));
    free(a);
    free(p);
    free(r);
    return success;
}
/**********************************************************************/
double ThermoElasticity::getTemperatureMax(void)
{
    return h_temperatureMax;

}
/**********************************************************************/
void ThermoElasticity::computeTemperatureMax(void)
{
    double T;
    int i;

    if(h_sol_thermal == NULL) return;
    h_temperatureMin = HUGE_VAL;
    h_temperatureMax = -HUGE_VAL;

    for ( i =0; i < h_nNodes;i++) {
        if (!doesNodeExist(i)) continue;
        T = h_sol_thermal[i];
        h_temperatureMin = (T < h_temperatureMin)? T:h_temperatureMin;
        if (T > h_temperatureMax){
            h_temperatureMax = T;
            h_nodeWithMaxTemp = i;
        }
    }
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: minT = %.2f, maxT = %.2f", h_temperatureMin, h_temperatureMax);

}
/**********************************************************/
void ThermoElasticity::getTemperatureRange(double *tMin,double *tMax,int *whichNodeMax)
{
    *tMin = h_temperatureMin;
    *tMax = h_temperatureMax;
    if (whichNodeMax != NULL) *whichNodeMax = h_nodeWithMaxTemp;
}
/**********************************************************/
void ThermoElasticity::setTemperatureRange(double tMin,double tMax)
// useul for plotting where the range does not vary
{
    h_temperatureMin = tMin;
    h_temperatureMax = tMax;
}
/**********************************************************/
void ThermoElasticity::getHdeltaT(int elem,double HdeltaT[24])
{
    /* See equation 2.8 in "Stress constrained thermo-elastic topology optimization " Structural and Multidisciplinary Optimization archive
   Volume 56 Issue 6, December 2017 Pages 1413-1427 */
    double dx,dy,dz,alpha,Y,nu,T0,rho,K,specificHeat;
    double HPrime[24][8];
    int elemnodes[8];
    char materialName[256];

    for(int j = 0; j< 24 ; j++)
        HdeltaT[j] = 0.0;

    if (!h_voxelgrid){
        writeMessageToParetoLog("ThermoElasticity::getHdeltaT only implemented for voxel grid ");
        return;
    }
    if (getElemPseudoDensity(elem) < h_pseudoDensityCutoff) return;
    dx = h_dx;
    dy = h_dy;
    dz = h_dz;

    T0 = h_thermoElasticReferenceTemperature;// reference temperature
    getMaterialOfPart(0,&Y,&nu,&rho,&K,&alpha,&specificHeat,materialName);

    HPrime[0][0] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[0][1] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[0][2] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][3] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][4] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][5] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][6] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[0][7] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][0] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[1][1] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][2] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][3] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[1][4] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][5] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][6] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][7] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[2][0] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[2][1] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][2] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[2][3] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][4] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[2][5] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][6] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[2][7] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[3][0] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[3][1] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[3][2] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][3] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][4] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][5] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][6] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[3][7] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[4][0] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][1] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[4][2] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[4][3] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][4] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[4][5] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][6] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][7] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[5][0] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][1] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[5][2] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][3] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[5][4] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][5] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[5][6] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][7] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[6][0] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][1] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][2] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[6][3] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[6][4] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[6][5] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[6][6] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][7] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][0] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][1] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[7][2] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[7][3] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][4] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[7][5] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][6] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][7] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[8][0] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[8][1] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][2] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[8][3] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][4] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[8][5] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][6] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[8][7] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[9][0] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][1] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][2] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[9][3] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[9][4] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[9][5] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[9][6] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][7] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[10][0] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[10][1] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][2] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][3] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[10][4] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][5] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[10][6] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[10][7] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[11][0] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][1] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[11][2] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][3] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[11][4] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][5] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[11][6] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][7] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[12][0] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][1] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][2] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[12][3] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[12][4] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[12][5] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[12][6] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][7] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][0] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][1] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[13][2] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[13][3] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][4] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[13][5] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][6] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][7] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[14][0] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[14][1] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][2] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[14][3] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][4] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[14][5] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][6] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[14][7] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][0] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][1] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][2] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[15][3] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[15][4] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[15][5] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[15][6] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][7] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[16][0] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[16][1] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][2] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][3] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[16][4] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][5] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[16][6] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[16][7] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[17][0] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][1] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[17][2] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][3] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[17][4] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][5] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[17][6] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][7] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][0] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][1] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][2] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][3] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][4] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][5] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][6] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[18][7] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][0] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[19][1] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][2] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][3] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[19][4] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][5] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][6] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][7] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][0] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[20][1] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][2] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[20][3] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][4] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[20][5] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][6] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[20][7] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[21][0] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[21][1] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[21][2] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][3] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][4] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][5] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][6] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[21][7] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[22][0] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][1] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[22][2] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[22][3] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][4] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[22][5] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][6] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][7] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[23][0] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][1] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[23][2] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][3] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[23][4] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][5] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[23][6] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][7] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);


    getElemNodes(elem,elemnodes);
    double nodeTemp[8];
    for(int j = 0; j<8 ; j++)
        nodeTemp[j] = h_sol_thermal[elemnodes[j]];


    for(int i = 0; i < 24 ; i++) {
        HdeltaT[i] = 0;
        for (int j = 0; j < 8; j++)
            HdeltaT[i] += HPrime[i][j]*(nodeTemp[j]-T0);
    }
}
/**********************************************************/
void ThermoElasticity::getHPrime(double HPrime[24][8])
{
    /* See equation 2.8 in "Stress constrained thermo-elastic topology optimization " Structural and Multidisciplinary Optimization archive
   Volume 56 Issue 6, December 2017 Pages 1413-1427 */
    double dx,dy,dz,alpha,Y,nu,T0,rho,K,specificHeat;
    char materialName[256];


    dx = h_dx;
    dy = h_dy;
    dz = h_dz;

    T0 = h_thermoElasticReferenceTemperature;// reference temperature
    getMaterialOfPart(0,&Y,&nu,&rho,&K,&alpha,&specificHeat,materialName);

    HPrime[0][0] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[0][1] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[0][2] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][3] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][4] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][5] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[0][6] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[0][7] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][0] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[1][1] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][2] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][3] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[1][4] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[1][5] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][6] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[1][7] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[2][0] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[2][1] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][2] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[2][3] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][4] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[2][5] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[2][6] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[2][7] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[3][0] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[3][1] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[3][2] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][3] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][4] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][5] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[3][6] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[3][7] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[4][0] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][1] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[4][2] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[4][3] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][4] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[4][5] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][6] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[4][7] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[5][0] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][1] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[5][2] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][3] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[5][4] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][5] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[5][6] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[5][7] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[6][0] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][1] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][2] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[6][3] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[6][4] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[6][5] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[6][6] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[6][7] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][0] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][1] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[7][2] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[7][3] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][4] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[7][5] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][6] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[7][7] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[8][0] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[8][1] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][2] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[8][3] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][4] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[8][5] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[8][6] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[8][7] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[9][0] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][1] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][2] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[9][3] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[9][4] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[9][5] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[9][6] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[9][7] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[10][0] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[10][1] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][2] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][3] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[10][4] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[10][5] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[10][6] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[10][7] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[11][0] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][1] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[11][2] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][3] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[11][4] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][5] = (Y*alpha*dx*dy)/(nu*1.44E+2-7.2E+1);
    HPrime[11][6] = (Y*alpha*dx*dy)/(nu*7.2E+1-3.6E+1);
    HPrime[11][7] = (Y*alpha*dx*dy)/(nu*3.6E+1-1.8E+1);
    HPrime[12][0] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][1] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][2] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[12][3] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[12][4] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[12][5] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[12][6] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[12][7] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][0] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][1] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[13][2] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[13][3] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][4] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[13][5] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][6] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[13][7] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[14][0] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[14][1] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][2] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[14][3] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][4] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[14][5] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[14][6] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[14][7] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][0] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][1] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][2] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[15][3] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[15][4] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[15][5] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[15][6] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[15][7] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[16][0] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[16][1] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][2] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][3] = (Y*alpha*dx*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[16][4] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[16][5] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[16][6] = (Y*alpha*dx*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[16][7] = (Y*alpha*dx*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[17][0] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][1] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[17][2] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][3] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[17][4] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][5] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[17][6] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[17][7] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][0] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][1] = (Y*alpha*dy*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[18][2] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][3] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][4] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][5] = (Y*alpha*dy*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[18][6] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[18][7] = (Y*alpha*dy*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][0] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[19][1] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][2] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][3] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[19][4] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[19][5] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][6] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[19][7] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][0] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[20][1] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][2] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[20][3] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][4] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[20][5] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[20][6] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[20][7] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[21][0] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[21][1] = (Y*alpha*dy*dz)/(nu*1.44E+2-7.2E+1);
    HPrime[21][2] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][3] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][4] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][5] = (Y*alpha*dy*dz)/(nu*7.2E+1-3.6E+1);
    HPrime[21][6] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[21][7] = (Y*alpha*dy*dz)/(nu*3.6E+1-1.8E+1);
    HPrime[22][0] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][1] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[22][2] = (Y*alpha*dx*dz*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[22][3] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][4] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[22][5] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][6] = (Y*alpha*dx*dz*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[22][7] = (Y*alpha*dx*dz*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[23][0] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][1] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[23][2] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][3] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);
    HPrime[23][4] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][5] = (Y*alpha*dx*dy*(-1.0/7.2E+1))/(nu*2.0-1.0);
    HPrime[23][6] = (Y*alpha*dx*dy*(-1.0/3.6E+1))/(nu*2.0-1.0);
    HPrime[23][7] = (Y*alpha*dx*dy*(-1.0/1.8E+1))/(nu*2.0-1.0);

}

/**********************************************************/
void ThermoElasticity::addThermalForce(void)
{
    if (!h_voxelgrid){
        writeMessageToParetoLog("ThermoElasticity::addThermalForce only implemented for voxel grid ");
        return;
    }

    int elemNodes[8];
    double HPrime[24][8],nodeTemp[8],fThermalElem[24],elemVol;
    getHPrime(HPrime);

    elemVol = h_dx*h_dy*h_dz;

    for(int elem = 0; elem<h_nElems ; elem++){
        if (getElemPseudoDensity(elem) < h_pseudoDensityCutoff) continue;
        getElemNodes(elem,elemNodes);
        getElemNodesTemperature(elem, nodeTemp);
        for (int k = 0; k < 24; k++){
            fThermalElem[k] = 0;
            for(int j = 0; j<8 ; j++)
               fThermalElem[k] += HPrime[k][j]*(nodeTemp[j]-h_thermoElasticReferenceTemperature);
        }
        for (int j = 0; j < 8; j++){
             if (!isNodeFree(elemNodes[j])) continue;// don't apply force on fixed nodes
             h_f[3*elemNodes[j]] += fThermalElem[3*j];
             h_f[3*elemNodes[j]+1] += fThermalElem[3*j+1];
             h_f[3*elemNodes[j]+2] += fThermalElem[3*j+2];
        }
    }

}
/**********************************************************/
void ThermoElasticity::addThermalStrainDueToSolidification(double TMelting, int nElems,int *elems)
{
    // Reference: ANALYSIS OF THERMAL STRESSES IN SHAPE DEPOSITION MANUFACTURING OF METAL PARTS
    // PhD thesis, Alexander H Nickel, Stanford
    // When metal solidifies from molten to Tref, there is a thermal strain

    double dx,dy,dz,alpha,E,nu,T0,rho,K,specificHeat;
    double fElem[24];
    int elemnodes[8],elem;
    char materialName[256];

    dx = h_dx;
    dy = h_dy;
    dz = h_dz;

    T0 = h_thermoElasticReferenceTemperature;// reference temperature
    getMaterialOfPart(0,&E,&nu,&rho,&K,&alpha,&specificHeat,materialName);


    //obtained from MatLab by doing triple integration of (dJ*B'*D*alpha*dummyvector')
    fElem[0] = (E*alpha*dy*dz)/(nu*8.0-4.0);
    fElem[1] = (E*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[2] = (E*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[3] = (E*alpha*dy*dz)/(nu*8.0-4.0);
    fElem[4] = (E*alpha*dy*dz)/(nu*8.0-4.0);
    fElem[5] = (E*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[6] = (E*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[7] = (E*alpha*dy*dz)/(nu*8.0-4.0);
    fElem[8] = (E*alpha*dx*dz)/(nu*8.0-4.0);
    fElem[9] = (E*alpha*dx*dz)/(nu*8.0-4.0);
    fElem[10] = (E*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[11] = (E*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[12] = (E*alpha*dx*dz)/(nu*8.0-4.0);
    fElem[13] = (E*alpha*dx*dz)/(nu*8.0-4.0);
    fElem[14] = (E*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[15] = (E*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[16] = (E*alpha*dx*dy)/(nu*8.0-4.0);
    fElem[17] = (E*alpha*dx*dy)/(nu*8.0-4.0);
    fElem[18] = (E*alpha*dx*dy)/(nu*8.0-4.0);
    fElem[19] = (E*alpha*dx*dy)/(nu*8.0-4.0);
    fElem[20] = (E*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[21] = (E*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[22] = (E*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    fElem[23] = (E*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);

    for(int i = 0; i<nElems ; i++){
        elem = elems[i];
        if (getElemPseudoDensity(elem) < h_pseudoDensityCutoff) continue;
        getElemNodes(elem,elemnodes);
        for(int j = 0; j<8 ; j++) {
            if (isNodeFree(elemnodes[j])) {
                h_f[3*elemnodes[j]]   += fElem[j]*(T0-TMelting);
                h_f[3*elemnodes[j]+1] += fElem[j+8]*(T0-TMelting);
                h_f[3*elemnodes[j]+2] += fElem[j+16]*(T0-TMelting);
            }
        }
    }
}
/**********************************************************/
void ThermoElasticity::setKeTemplatesThermal(double *vec)
{
    int nTemplateIDs = getNumTemplateIDs();
    memcpy(h_KETemplates_thermal,vec,nTemplateIDs*8*8*sizeof(double));
    if ((h_voxelgrid) &&  (h_nodeBasedAssembly))
        computeKeNodalThermal();
}
/**********************************************************/
void ThermoElasticity::set_thermalStaticSol(double *vec)
{
    copy_thermal(vec,h_sol_thermal);
    computeTemperatureMax();
}
/**********************************************************/
double ThermoElasticity::getThermalForce(int node)
{
    return h_f_thermal[node];
}
/**********************************************************/
void ThermoElasticity::get_fThermal(double *vec)
{
    copy_thermal(h_f_thermal,vec);
}
/**********************************************************/
void ThermoElasticity::set_fThermal(double *vec)
{
    copy_thermal(vec,h_f_thermal);
}

/**********************************************************/
void ThermoElasticity::get_convectionThermal(double *vec)
{
    copy_thermal(h_convection,vec);
}
/**********************************************************/
void ThermoElasticity::set_convectionThermal(double *vec)
{
    copy_thermal(vec,h_convection);
}
/**********************************************************************/
void ThermoElasticity::getWorkSpaceVectorThermal(double* inVector, VectorXd &outVector)
/* Function to transform full vector in to a workspace vector
    Input: int allDof (nodes * dof/node), int nDof (free degrees of freedom)
    pre allocated pointers for
    - coordinates (1 x allDof)
    - group array or ws_nodeGroupNumber (1 x allDof)
    - array of freeDOFs (1 x nDof)
    - input Vector in solution space (1 x nDof)
    - output Vector of workspace (1 x 4*#groups)

    T = T0 + Tx*x + Ty*y + Tz*z;
    */
{
    int group, node, start = 0, stop, index;
    double gX, gY, gZ, xnode,ynode,znode;

    start = 0;

    for (int i =0; i < 4*ws_nGroups; i++)
        outVector(i) = 0.0;


    //#pragma omp parallel for private(group,gX,gY,gZ,start,stop,node,index,xnode,ynode,znode)
    for (group = 0; group < ws_nGroups; group++){
        gX = h_CGPatch[group*3];
        gY = h_CGPatch[group*3 + 1];
        gZ = h_CGPatch[group*3 + 2];
        if (group != 0) start = ws_groupEndNodeIndex[group - 1];
        stop = ws_groupEndNodeIndex[group];
        for (index = start; index < stop; index ++){
            node = ws_groupSortedNodes[index];
            if(doesNodeExist(node)) {
                xnode = (h_x[node] - gX);
                ynode = (h_y[node] - gY);
                znode = (h_z[node] - gZ);

                outVector(4*group)		+= *(inVector + node);
                outVector(4*group + 1)	+= *(inVector + node) * xnode;
                outVector(4*group + 2)	+= *(inVector + node) * ynode;
                outVector(4*group + 3)	+= *(inVector + node) * znode;

            }
        }
    }
}
/**********************************************************************/
void ThermoElasticity::getSolutionSpaceVectorThermal(VectorXd &inVector, double* outVector)
/* Function to transform workspace vector in to a full vector
    Input: int allDof (nodes * dof/node), int nDof (free degrees of freedom)
    pre allocated pointers for
    - coordinates (1 x allDof)
    - group array or ws_nodeGroupNumber (1 x allDof)
    - array of freeDOFs (1 x nDof)
    - input Vector of workspace (1 x 4*#groups)
    - output Vector in solution space (1 x nDof)
    */
{
    int group, node;
    double xnode,ynode,znode;

    memset(outVector, 0, h_nNodes*sizeof(double));

#pragma omp parallel for private(node,xnode,ynode,znode,group)
    for (node = 0; node < h_nNodes; node++)	{
        if (!doesNodeExist(node)) continue;
        group = ws_nodeGroupNumber[node];
        xnode = (h_x[node] - h_CGPatch[3*group + 0]);
        ynode = (h_y[node] - h_CGPatch[3*group + 1]);
        znode = (h_z[node] - h_CGPatch[3*group + 2]);
        if (doesNodeExist (node)){
            *(outVector + node) = inVector(4*group) + inVector(4*group + 1) * xnode
                    + inVector(4*group + 2) * ynode + inVector(4*group + 3) * znode ;
        }

    }
}
/******************************************************************************/
bool ThermoElasticity::KreductionThermal(double transientFactor)
{
    int k, group2, group1, node1, node2;
    int node_i, node_j, elemID,KEindex,nTemplateIDs;
    double K_val,xnode1,ynode1,znode1,xnode2,ynode2,znode2,additionalTerm;

    if (ws_nGroups <= 0) return false;
    KDeflatedThermal.resize(4*ws_nGroups,4*ws_nGroups);
    KDeflatedThermal.setZero(4*ws_nGroups,4*ws_nGroups);

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: In KreductionThermal;ws_nGroups= %d ",ws_nGroups);
    nTemplateIDs = getNumTemplateElements();
    for ( k = 0; k < h_nElems; k++) {
        if (getElemPseudoDensity(k) <h_pseudoDensityCutoff) continue; // element exists but is void (example: during topology optimization)
        elemID = getTemplateId(k);
        for (node_i = 0; node_i < 8; node_i++) {
            //node of a single voxel
            node1 = h_elems[node_i*h_nElems + k];
            if (!h_free_thermal[node1]) continue;// node is fixed (thermally)
            if (!doesNodeExist(node1)){
                writeMessageToParetoLog("Error in Kreduction");
                continue;
            }
            group1 = ws_nodeGroupNumber[node1]; // Group number on mesh

            xnode1 = (h_x[node1] - h_CGPatch[3*group1 + 0]);
            ynode1 = (h_y[node1] - h_CGPatch[3*group1 + 1]);
            znode1 = (h_z[node1] - h_CGPatch[3*group1 + 2]);
            for (node_j = 0; node_j < 8; node_j++) {
                node2 = h_elems[node_j*h_nElems + k];
                if (!h_free_thermal[node2]) continue;// node is fixed (thermally)
                if (!doesNodeExist(node2)){
                    writeMessageToParetoLog("Error in Kreduction");
                    continue;//should not come here
                }
                group2 = ws_nodeGroupNumber[node2];

                xnode2 = (h_x[node2] - h_CGPatch[3*group2 + 0]);
                ynode2 = (h_y[node2] - h_CGPatch[3*group2 + 1]);
                znode2 = (h_z[node2] - h_CGPatch[3*group2 + 2]);

                KEindex = (8*node_i + node_j)*(nTemplateIDs) + elemID;
                K_val = h_KETemplates_thermal[KEindex];

                KDeflatedThermal(4*group1,4*group2) += K_val;
                KDeflatedThermal(4*group1,4*group2 + 1) += K_val*xnode2;
                KDeflatedThermal(4*group1,4*group2 + 2) += K_val*ynode2;
                KDeflatedThermal(4*group1,4*group2 + 3) += K_val*znode2;

                KDeflatedThermal(4*group1+1,4*group2) += K_val*xnode1;
                KDeflatedThermal(4*group1+1,4*group2 + 1) += K_val*xnode2*xnode1;
                KDeflatedThermal(4*group1+1,4*group2 + 2) += K_val*ynode2*xnode1;
                KDeflatedThermal(4*group1+1,4*group2 + 3) += K_val*znode2*xnode1;

                KDeflatedThermal(4*group1+2,4*group2) += K_val*ynode1;
                KDeflatedThermal(4*group1+2,4*group2 + 1) += K_val*xnode2*ynode1;
                KDeflatedThermal(4*group1+2,4*group2 + 2) += K_val*ynode2*ynode1;
                KDeflatedThermal(4*group1+2,4*group2 + 3) += K_val*znode2*ynode1;

                KDeflatedThermal(4*group1+3,4*group2) += K_val*znode1;
                KDeflatedThermal(4*group1+3,4*group2 + 1) += K_val*xnode2*znode1;
                KDeflatedThermal(4*group1+3,4*group2 + 2) += K_val*ynode2*znode1;
                KDeflatedThermal(4*group1+3,4*group2 + 3) += K_val*znode2*znode1;

            }
        }
    }
    /// add the lumped (diagonal) convection terms
    for (node1 = 0; node1 < h_nNodes; node1++) {
        if (!h_free_thermal[node1]) continue;// node is fixed (thermally)
        if (!doesNodeExist(node1)){
            //writeMessageToParetoLog("Error in Kreduction");
            continue;
        }
        group1 = ws_nodeGroupNumber[node1]; // Group number on mesh

        xnode1 = (h_x[node1] - h_CGPatch[3*group1 + 0]);
        ynode1 = (h_y[node1] - h_CGPatch[3*group1 + 1]);
        znode1 = (h_z[node1] - h_CGPatch[3*group1 + 2]);

        additionalTerm = transientFactor*h_convection[node1] + transientFactor*h_radiationNonLinear[node1];

        KDeflatedThermal(4*group1,4*group1) += additionalTerm;
        KDeflatedThermal(4*group1,4*group1 + 1) += additionalTerm*xnode1;
        KDeflatedThermal(4*group1,4*group1 + 2) += additionalTerm*ynode1;
        KDeflatedThermal(4*group1,4*group1 + 3) += additionalTerm*znode1;

        KDeflatedThermal(4*group1+1,4*group1) += additionalTerm*xnode1;
        KDeflatedThermal(4*group1+1,4*group1 + 1) += additionalTerm*xnode1*xnode1;
        KDeflatedThermal(4*group1+1,4*group1 + 2) += additionalTerm*ynode1*xnode1;
        KDeflatedThermal(4*group1+1,4*group1 + 3) += additionalTerm*znode1*xnode1;

        KDeflatedThermal(4*group1+2,4*group1) += additionalTerm*ynode1;
        KDeflatedThermal(4*group1+2,4*group1 + 1) += additionalTerm*xnode1*ynode1;
        KDeflatedThermal(4*group1+2,4*group1 + 2) += additionalTerm*ynode1*ynode1;
        KDeflatedThermal(4*group1+2,4*group1 + 3) += additionalTerm*znode1*ynode1;

        KDeflatedThermal(4*group1+3,4*group1) += additionalTerm*znode1;
        KDeflatedThermal(4*group1+3,4*group1 + 1) += additionalTerm*xnode1*znode1;
        KDeflatedThermal(4*group1+3,4*group1 + 2) += additionalTerm*ynode1*znode1;
        KDeflatedThermal(4*group1+3,4*group1 + 3) += additionalTerm*znode1*znode1;

    }

    //force symmetry
    for (int i = 0; i <4*ws_nGroups;i++)
        for (int j=i+1; j < 4*ws_nGroups; j++)
            KDeflatedThermal(i,j) = KDeflatedThermal(j,i);


    int rowStart,rowEnd;
    double maxValue = KDeflatedThermal.maxCoeff();
    for (int g = 0; g <ws_nGroups;g++) {
        if (ws_groupCount[g] == 0){
            rowStart = 4*g;
            rowEnd = 4*g + 4;
            for (int i = rowStart; i < rowEnd;i++)
                KDeflatedThermal(i,i) = maxValue;
        }
    }
    return true;
}
/**********************************************************************/
//Allocate double* WKW, and check whether ws_nodeGroupNumber is in place
bool ThermoElasticity::sparseWKWThermal(double transientFactor)// Neumann condition assumed, diriclet applied after K*u
{

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Constructing sparseWKW ");

    if (!KreductionThermal(transientFactor)) return false;

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Attempting LLT of sparseWKW Thermal");
    Eigen::SparseMatrix<double>	 sparseWKWThermal = KDeflatedThermal.sparseView();
    sparseSolverThermal.compute(sparseWKWThermal);
    if (sparseSolverThermal.info() != 0) {
        if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Error in LLT decomposition");
        return false;
    }
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: SparseWKW constructed");
    return true;
}
/**********************************************************************/
void	ThermoElasticity::computeThermalJacobiPreconditioner(double *preconditionerJacobi)
{
    // compute JacobiPreconditioner for thermal
    // assumes memory has been allocated for preconditionerJacobi (h_nodes*sizeof(double))

    register int nTemplateIDs,node,codedElem,nodeIndex,elem,elemID,maxElem,node2,i,j,KEindex;  //additional variables for initializing preconditioning matrix
    register double KScale,row,KE;

    if(h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Computing Thermal Jacobi Preconditioner");

    memset(preconditionerJacobi,0,h_nNodes*sizeof(double));
    nTemplateIDs = getNumTemplateIDs();
    for ( node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)){
            preconditionerJacobi[node] = 1;//diagonal of 1
            continue;
        }
        maxElem = h_nNeighboringElems[node];// how many elements connected to node
        for (j =0; j < maxElem; j++) {//
            codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
            nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
            elem = codedElem - nodeIndex*MAXELEMS;
            if (getElemPseudoDensity(elem) < 1e-6) continue;// truly void
            KScale = pow(getElemPseudoDensity(elem),h_Kexponent);// K = K0*(rho)^n;
            elemID = getElemTemplateID(elem);
            row = 8*nodeIndex;
            for (i = 0; i < 8; i++) {// find this node in the neighboring element
                node2 = h_elems[(h_nElems)*i + elem];
                if (node == node2) {
                    KEindex = (row + i)*(nTemplateIDs) + elemID;
                    KE = KScale*h_KETemplates_thermal[KEindex];
                    preconditionerJacobi[node] += KE;
                }
            }
        }
    }
    if(h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Done computing Thermal Jacobi Preconditioner");

}
/**********************************************************************/
bool ThermoElasticity::performDeflatedCGThermal(double *f,double *u,int *niter,double transientFactor)
{
    // Solves Ku = f
    // Assumes K and f have been defined, and Dirichlet conditions have been set
    // Also assumes CG iterations and tolerances have been defined.
    // Solution is returned in h_sol_thermal
    bool success;

    double rho,alpha,beta,rho_new,normr,normr0;
    double *r,*p,*a,*d, *t;

    if (h_thermo_elasticity_verbose)writeMessageToParetoLog("ThermoElasticity: Performing deflated CG ");

    // use temporary CPU variables
    r = (double *) malloc(h_nNodes*sizeof(double));
    p = (double *) malloc(h_nNodes*sizeof(double));
    a = (double *) malloc(h_nNodes*sizeof(double));
    d = (double *) malloc(h_nNodes*sizeof(double));
    t = (double *) malloc(h_nNodes*sizeof(double));

    safeFree(h_cgNorm);
    h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));

    VectorXd mu,result;
    mu.resize(4*ws_nGroups);

    // Performing conjugate gradient here with residual start
    KuCPU_thermal(u,a);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
    getWorkSpaceVectorThermal(r, mu); // mu = W'*h_f
    //Compute [W'*K*W]*result = W'*r = mu (from above)
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);// h_sol = W*result
    enforceDirichletZero_thermal(t);
    axpy_thermal(1,t,u);
    KuCPU_thermal(u,a,transientFactor);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
    normr0 = dotprod_thermal(r,r);

    *niter = 1;     // Init counter for number of iterations
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoDeflation: %d: residualNorm: %e ",*niter,normr0);
    if (normr0 < 1e-16) {
        free(a);
        free(p);
        free(r);
        free(d);
        free(t);
        return true;
    }

    normr = 10*normr0;
    //********************************************************
    KuCPU_thermal(r,t,transientFactor);
    getWorkSpaceVectorThermal(t, mu);
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);
    enforceDirichletZero_thermal(t);
    zeaxpy_thermal(-1,1,p,t,r);// p = r - t
    //********************************************
    rho = dotprod_thermal(r,r);
    *niter = 1;     // Init counter for number of iterations

    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoDeflation: Starting iterations ");
    if (_isnan(normr0)) return false;
    success = true;
    while (normr > h_relAccuracyThermal*normr0)   {
        KuCPU_thermal(p,a,transientFactor);// a = K*p;
        beta = dotprod_thermal(a,p);
        alpha = rho/beta;
        axpy_thermal(alpha, p,u);//u = u + alpha*p;
        if (*niter%1000 == 0)	{ // restart CG with recomputing residual
            KuCPU_thermal(u,a,transientFactor);// a = K*sol
            zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
        }
        else
            axpy_thermal(-alpha,a,r);//r = r - alpha*a;

        h_cgNorm[*niter-1] = log10(normr/normr0);

        rho_new =  dotprod_thermal(r,r);//rho = a'*r
        normr = (rho_new);

        if (_isnan(normr)) return false;
        KuCPU_thermal(r,t,transientFactor);

        getWorkSpaceVectorThermal(t, mu);		// mu = W'*t
        result = sparseSolverThermal.solve(mu);
        getSolutionSpaceVectorThermal(result, t); // t = W*result
        enforceDirichletZero_thermal(t);//

        zeaxpy_thermal(1,(rho_new/rho),p,r,p);//p = r + rho_new/rho * p;
        zeaxpy_thermal(-1,1,p,t,p);//p = -t + p
        rho = rho_new;
        h_nCGItersActual = *niter;
        (*niter)++;
        if ((h_thermo_elasticity_verbose) && (*niter%100 ==0)) writeMessageToParetoLog("ThermoDeflation: %d: %e ",*niter,normr/normr0);
        if (*niter >= ws_DCGMaxIter)  {
            if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: DCG did not converge in %d iter, resErr = %e ",*niter,normr/normr0);
            success = false;
            break;
        }
    }

    if (h_thermo_elasticity_verbose)  writeMessageToParetoLog("ThermoElasticity: CG on CPU; nIter = %d, resErr = %e ",*niter,normr/(normr0+1e-15));

    computeTemperatureMax();
    free(a);
    free(p);
    free(r);
    free(d);
    free(t);
    return success;
}
/**********************************************************************/
bool ThermoElasticity::performPreconditionedDeflatedCGThermal(double *f,double *u,int *niter,double transientFactor)
{
    // Solves Ku = f using preconditioner and deflation to address soft elements
    // Assumes K and f have been defined, and Dirichlet conditions have been set
    // Also assumes CG iterations and tolerances have been defined.
    // Solution is returned in h_sol_thermal
    bool success;

    double rho,alpha,beta,rho_new,normr,normr0;
    double *r,*p,*a,*d, *t,*z,*preconditionerJacobi;
    Timer timer;

    if (h_thermo_elasticity_verbose)writeMessageToParetoLog("ThermoElasticity: Performing preconditioned deflated CG");

    // use temporary CPU variables
    z = (double *) malloc(h_nNodes*sizeof(double));
    r = (double *) malloc(h_nNodes*sizeof(double));
    p = (double *) malloc(h_nNodes*sizeof(double));
    a = (double *) malloc(h_nNodes*sizeof(double));
    d = (double *) malloc(h_nNodes*sizeof(double));
    t = (double *) malloc(h_nNodes*sizeof(double));

    safeFree(h_cgNorm);
    h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));

    VectorXd mu,result;
    mu.resize(4*ws_nGroups);

    // Performing conjugate gradient here with residual start
    KuCPU_thermal(u,a);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
    getWorkSpaceVectorThermal(r, mu); // mu = W'*h_f

    //Compute [W'*K*W]*result = W'*r = mu (from above)
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);// h_sol = W*result
    enforceDirichletZero_thermal(t);
    axpy_thermal(1,t,u);
    KuCPU_thermal(u,a,transientFactor);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f

    //Jacobi Preconditioning
    preconditionerJacobi= (double*) malloc(h_nNodes*sizeof(double));
    computeThermalJacobiPreconditioner(preconditionerJacobi);

    ScaleByVector_thermal(z,r,preconditionerJacobi);
    normr0 = dotprod_thermal(z,r);
    if (_isnan(normr0)) {
        if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoDeflation: Preconditioned deflation failed");
        free(preconditionerJacobi);
        free(a);
        free(p);
        free(r);
        free(d);
        free(t);
        free(z);
        return false;
    }
    *niter = 1;     // Init counter for number of iterations
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoDeflation: %d: %e ",*niter,normr0);
    if (normr0 < 1e-16) {
        free(preconditionerJacobi);
        free(a);
        free(p);
        free(r);
        free(d);
        free(t);
        return true;
    }

    normr = 10*normr0;
    //********************************************************
    KuCPU_thermal(z,t,transientFactor);
    getWorkSpaceVectorThermal(t, mu);
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);
    enforceDirichletZero_thermal(t);
    zeaxpy_thermal(-1,1,p,t,z);// p = z - t
    //********************************************
    rho = dotprod_thermal(r,z);
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Starting iterations");

    success = true;
    timer.start();
    while (normr > h_relAccuracyThermal*normr0)   {
        KuCPU_thermal(p,a,transientFactor);// a = K*p;
        beta = dotprod_thermal(a,p);
        alpha = rho/beta;
        axpy_thermal(alpha, p,u);//u = u + alpha*p;
        if (*niter%1000 == 0)	{ // restart CG with recomputing residual
            KuCPU_thermal(u,a,transientFactor);// a = K*sol
            zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
        }
        else
            axpy_thermal(-alpha,a,r);//r = r - alpha*a;

        ScaleByVector_thermal(z,r,preconditionerJacobi);	//here z is z0

        h_cgNorm[*niter-1] = log10(normr/normr0);

        rho_new =  dotprod_thermal(r,z);//rho = a'*r
        normr = (rho_new);

        if (_isnan(normr)) return false;
        KuCPU_thermal(z,t,transientFactor);

        getWorkSpaceVectorThermal(t, mu);		// mu = W'*t
        result = sparseSolverThermal.solve(mu);
        getSolutionSpaceVectorThermal(result, t); // t = W*result
        enforceDirichletZero_thermal(t);//

        zeaxpy_thermal(1,(rho_new/rho),p,z,p);//p = r + rho_new/rho * p;
        zeaxpy_thermal(-1,1,p,t,p);//p = -t + p
        rho = rho_new;
        h_nCGItersActual = *niter;
        (*niter)++;
        if (timer.getElapsedTimeInSec() > 120) {//update every 2 mins or so.
            writeMessageToParetoLog("ThermoDeflation: Iter: %d	 Rel-err: %.2e ",*niter,normr/normr0);
            timer.stop();
            timer.start();
        }
        if ((h_thermo_elasticity_verbose) && (*niter%100 ==0)) writeMessageToParetoLog("ThermoDeflation: %d: %e",*niter,normr/normr0);
        if (*niter >= ws_DCGMaxIter)  {
            if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Preconditioned deflated CG did not converge in %d iter, resErr = %e",*niter,normr/normr0);
            success = false;
            break;
        }
    }

    if (h_thermo_elasticity_verbose)  writeMessageToParetoLog("ThermoDeflation: Exiting preconditioned deflated CG on CPU; nIter = %d, resErr = %e",*niter,normr/(normr0+1e-15));

    //computeTemperatureMax();
    free(preconditionerJacobi);
    free(a);
    free(p);
    free(r);
    free(d);
    free(t);
    free(z);
    return success;
}
/**********************************************************/
ThermoElasticity::~ThermoElasticity()
{
    if (h_thermo_elasticity_verbose) writeMessageToParetoLog("ThermoElasticity: Destructing ThermoElasticity object");
    if (h_nNodes == 0) return;

    h_nNodes = 0;
    releaseCPUMemory_thermal();
}
