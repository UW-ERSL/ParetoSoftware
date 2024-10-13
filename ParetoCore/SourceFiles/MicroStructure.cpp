#include <math.h>
#include <stdlib.h>
#include <string> 
#ifndef __APPLE__
#include <omp.h>
#endif
#include "cmatrix.h"
#include "MicroStructure.h"
#include "Timer.h"

/**********************************************************************/
MicroStructure::MicroStructure()
{
	for (int problem = 0; problem < 6; problem++)
		for (int component = 0; component < 6; component++)
			mu_elemstresses[problem][component] = NULL;
	
	xMin = 0;
	yMin = 0;
	zMin = 0;
	xMax = 0;
	yMax = 0;
	zMax = 0;
}
/**********************************************************************/
void MicroStructure::initializeMicroStructure(MICRO_OBJECTIVE objective,double xyzMin[3],double xyzMax[3])
{
	int BCmode = 0;
	double bBoxVolume,meshVolume;

	setQuantityOfInterest(COMPLIANCE);
	InitializeTopOpt(BCmode);// memory allocation 
	mu_objective = objective;
	ts_symmetryIndex = 7;
	mu_exploitSymmetry = false; // only exploit symmetry if voxel mesh is close to bounding box volume
	ts_restartTopology = true;

	xMin = xyzMin[0];
	yMin = xyzMin[1];
	zMin = xyzMin[2];
	xMax = xyzMax[0];
	yMax = xyzMax[1];
	zMax = xyzMax[2];

	bBoxVolume = (xMax-xMin)*(yMax-yMin)*(zMax-zMin);
	meshVolume = h_nElems*h_dx*h_dy*h_dz;

	if (fabs(bBoxVolume-meshVolume)/bBoxVolume < 1e-5){
		printf("Microstructure: Exploiting symmetry\n");
		mu_exploitSymmetry = true;

	}
	else {
		printf("Microstructure: Not exploiting symmetry\n");
		mu_exploitSymmetry = false;
	}
	for (int problem = 0; problem < 6; problem++){
		for (int component = 0; component < 6; component++){
			safeFree(mu_elemstresses[problem][component]);
			mu_elemstresses[problem][component] = (double *)malloc(h_nElems*sizeof(double));
		}
	}
	// compute the H Matrix, Equation 28 of Ozdemir  "Topological derivative based ...", 2014
	double alpha1, alpha2,E,nu;
	int i,j;

	int materialID;
	materialID = 0;
	getIsotropicMaterialProperties(materialID,&E, &nu);

	alpha1 = -15.0/(E)*(1-nu*nu)/(7.0-5*nu);
	alpha2 = 3./(2.*E)*(1-nu)*(1+5*nu)/(7.0-5*nu);

	// H = alpha1*I_4 + alpha2*(I_2*I_2)

	for ( i =0; i < 6;i++){
		for ( j = 0; j < 6; j++)
			H[i][j] = 0.0;
		H[i][i] = alpha1;
	}
	for ( i =0; i < 3;i++)
		for ( j = 0; j < 3; j++)
			H[i][j] += alpha2;

	if (mu_objective == X_RIGIDITY) {
		T1[0] = 1.0;
		T2[0] = 1.0;
		for (i =1;i <6;i++){
			T1[i] = 0.0;
			T2[i] = 0.0;
		}
	}
	else if (mu_objective == BULK_MODULUS) {
		for (i =0; i <3;i++){
			T1[i] = 1.0;
			T2[i] = 1.0;
		}
		for (i =3; i <6;i++){
			T1[i] = 0.0;
			T2[i] = 0.0;
		}
	}
	else if (mu_objective == BULK_MODULUS_2D) {
		for (i =0; i <1;i++){
			T1[i] = 1.0;
			T2[i] = 1.0;
		}
		for (i = 1; i <6;i++){
			T1[i] = 0.0;
			T2[i] = 0.0;
		}
	}
	else if (mu_objective == SHEAR_MODULUS) {
		for (i =0; i <6;i++){
			T1[i] = 0.0;
			T2[i] = 0.0;
			T3[i] = 0.0;
		}
		T1[3] = 1.0;
		T2[4] = 1.0;
		T3[5] = 1.0;
	}
	else if (mu_objective == NEG_POISSON) {
		for (i =0; i <6;i++){
			T1[i] = 0.0;
			T2[i] = 0.0;
			T3[i] = 0.0;
			T4[i] = 0.0;
			T5[i] = 0.0;
			T6[i] = 0.0;
		}
		T1[0] = 1.0;
		T2[1] = 1.0;
		T3[2] = 1.0;
	}
}
/**********************************************************************/
void  MicroStructure::saveMicrostructureResult(double volFrac,double isoValue,double J,bool forceSave)
{
	if (iso_save_counter >= 0) {// already saved previously
		// save only if volumefraction difference is greater than or equal to 
		if ( (!forceSave) && (ts_volFracSave[iso_save_counter] - volFrac) < ts_saveEvery)
			return;
	}
	// otherwise save
	iso_save_counter++;
	//printf("Saving %d, vol frac = %.3f \n",iso_save_counter,volFrac);
	iso_save_counter = MIN(iso_save_counter,MAX_ISO_SAVES-1);
	ts_isoValueSave[iso_save_counter] = isoValue;
	ts_volFracSave[iso_save_counter] = volFrac;
	fixNodalTSValues(isoValue);
#pragma omp parallel for
	for (int node = 0;node < h_nNodes; node++) {
		if (isNodeOnCubeBoundary(node)){
			ts_topSensNode[node] = isoValue - 0.0001*fabs(isoValue);
		}
		ts_topSensNodeSave[iso_save_counter][node] = ts_topSensNode[node];
	}

	mu_JExactSave[iso_save_counter] = getHashinSthrikmanBound(volFrac);
	mu_JComputedSave[iso_save_counter] = J;
}
/**********************************************************************/
void	MicroStructure::doubleContract(double A[6][6],double B[6][6],double C[6][6])
{
	// C = A:B
	//assumes various symmetries of the 4th order tensors
	int i,j,k;
	for ( i =0; i < 6;i++){
		for ( j = 0; j < 6; j++){
			C[i][j] = 0;
			for ( k = 0; k < 6; k++)
				C[i][j] += A[i][k]*B[k][j];
		}
	}
}
/**********************************************************************/
void	MicroStructure::doubleContract(double A[6][6],double B[6],double C[6])
{
	// c = A:b
	//assumes various symmetries of the 4th order tensors and 2nd order tensor
	int i,j;
	for ( i =0; i < 6;i++){
		C[i] = 0;
		for (j = 0; j < 6; j++)
			C[i] += A[i][j]*B[j];
	}
}
/**********************************************************************/
void	MicroStructure::doubleContract(double A[6],double B[6],double *C)
{
	// c = a:b
	int i;
	*C = 0;
	for (i =0; i < 6;i++)
		*C += A[i]*B[i];
}
/**********************************************************************/
bool MicroStructure::isNodeOnCubeBoundary(int node)
{
	double Tol;

	Tol = 0.001*h_dx;
	
	if ( (fabs(h_x[node] - xMin) < Tol) || (fabs(h_x[node] - xMax) < Tol) ) return true;
	if ( (fabs(h_y[node] - yMin) < Tol) || (fabs(h_y[node] - yMax) < Tol) ) return true;
	if ( (fabs(h_z[node] - zMin) < Tol) || (fabs(h_z[node] - zMax) < Tol) ) return true;

	return false;
}
/**********************************************************************/
void MicroStructure::applyStrainMatrix(double strain[6])
{
	int node;
	double x,y,z;

	#pragma omp parallel for private(x,y,z)
	for (node = 0; node < h_nNodes; node++) {
		h_free[node] = 7; // free
		if (!isNodeOnCubeBoundary(node)) continue;
		if (!doesNodeExist(node)) continue;
		x = h_x[node];
		y = h_y[node];
		z = h_z[node];
		h_free[node] = 0;//fixed
		h_dirichlet[3*node] =    strain[0]*x + strain[3]*y + strain[4]*z;
		h_dirichlet[3*node+1] =  strain[1]*y + strain[3]*x + strain[5]*z;
		h_dirichlet[3*node+2] =  strain[2]*z + strain[4]*x + strain[5]*y;
	}
}
/**********************************************************************/
void MicroStructure::printCMatrix(double CMatrix[6][6])
{	
	printf("\n------Computed CMatrix---------\n");
	for (int i =0; i < 6; i++, printf("\n"))
		for (int j =0; j < 6; j++)
			printf("%.2e ",CMatrix[i][j]);
	printf("-------------------------------\n");
}
/**********************************************************************/
double MicroStructure::getHashinSthrikmanBound(double volFrac)
{
	double phi,E,nu,K,G,KStar,GStar;
	// Reference: Andreassen, et. al, "Design of manufacturable ..." Mechanics of Materials, 2014

	int materialID;
	materialID = 0;
	getIsotropicMaterialProperties(materialID,&E, &nu);

	phi = volFrac;
	K = E/(3*(1-2*nu));
	G = E/(2*(1+nu));
	KStar = K*phi/(1+ 3./5*(1-phi)*K/G);
	GStar = G*phi/(1+ 2./3*(1-phi)*(6*K+12*G)/(9*K+8*G));

	if (mu_objective == BULK_MODULUS) 
		return KStar/K;
	else if (mu_objective == SHEAR_MODULUS) 
		return GStar/G;
	else 
		return 1.0;
}
/**********************************************************************/
void MicroStructure::printHashinSthrikmanBounds(void)
{
	double phi,microStructureVolume,bBoxVolume,E,nu,K,G,KStar,GStar;
	// Reference: Andreassen, et. al, "Design of manufacturable ..." Mechanics of Materials, 2014
	int materialID;
	materialID = 0;
	getIsotropicMaterialProperties(materialID,&E, &nu);

	microStructureVolume = getMeshVolume();
	bBoxVolume = (xMax-xMin)*(yMax-yMin)*(zMax-zMin);
	phi = getCurrentVolumeFraction();

	K = E/(3*(1-2*nu));
	G = E/(2*(1+nu));
	KStar = K*phi*G/(2./3*(1-phi)*K + G);
	GStar = G*phi*(9*K+8*G)/(  5*(3*K + 4*G) -6*(K+2*G)*phi );
	printf("\nphi = %.3f, GStar/G = %.2f, KStar/K = %.2f",phi,GStar/G,KStar/K);
}
/**********************************************************************/
int MicroStructure::solveConstantStrainProblem(int problem,double CMatrix[6][6])
{
	int i,elem,ex,ey,ez,elemMapped,nIter;
	double strain[6], stressAverage[6],bBoxVolume,elemVolume;
	int map1[] = {1,0,2,3,5,4}; //captures how the stresses map from x to y 
	int map2[] = {2,1,0,5,4,3}; //captures how the stresses map from x to z 
	int map3[] = {0,1,2,4,3,5}; //
	int map4[] = {0,1,2,5,4,3}; //

	bBoxVolume = (xMax-xMin)*(yMax-yMin)*(zMax-zMin);
	elemVolume = h_dx*h_dy*h_dz;

	memset(strain,0,6*sizeof(double));
	for (i = 0; i < 6; i++)
		memset(mu_elemstresses[problem][i],0,h_nElems*sizeof(double));
	
	if ((!mu_exploitSymmetry) || (problem == 0) || (problem == 3) ) {// solve explicitly
		if (problem < 3) strain[problem] = 1.0;
		else if (problem < 6) strain[problem] = 0.5;

		applyStrainMatrix(strain);
		initializeSolution();
		if (!solveElasticityStatic()) return FAILURE; 
		postprocess();
		//save the stresses
		for (i = 0; i < 6; i++) 
			memcpy(mu_elemstresses[problem][i],h_elemstresses[i],h_nElems*sizeof(double));
	}
	else {//copy results; 
		// assumptions: 
		// 1.problems are solved in order (0,1,2,3,4,5)	
		// 2. h_nelx = h_nely = h_nelz
		// 3. mesh exhibits 3-fold symmetry
		if (problem == 1) {//copy stresses from problem = 0
			#pragma omp parallel for private(ex,ey,ez,elemMapped,i)
			for (elem = 0; elem < h_nElems; elem++) {
				ez = elem/(h_nelx*h_nely);
				ey = (elem-ez*h_nelx*h_nely)/(h_nelx);
				ex = (elem-ez*h_nelx*h_nely - ey*h_nelx);
				elemMapped = ez*h_nelx*h_nely + ex*h_nelx + ey;
				for (i = 0; i < 6; i++)
					mu_elemstresses[problem][map1[i]][elemMapped] = mu_elemstresses[0][i][elem];
			}
		}
		else if (problem == 2) {//copy stresses from problem = 0
			#pragma omp parallel for private(ex,ey,ez,elemMapped,i)
			for (elem = 0; elem < h_nElems; elem++) {
				ez = elem/(h_nelx*h_nely);
				ey = (elem-ez*h_nelx*h_nely)/(h_nelx);
				ex = (elem-ez*h_nelx*h_nely - ey*h_nelx);
				elemMapped = ex*h_nelx*h_nely + ey*h_nelx + ez;
				for (i = 0; i < 6; i++)
					mu_elemstresses[problem][map2[i]][elemMapped] = mu_elemstresses[0][i][elem];
			}
		}
		else if (problem == 4) {//copy stresses from problem = 3
			#pragma omp parallel for private(ex,ey,ez,elemMapped,i)
			for (elem = 0; elem < h_nElems; elem++) {
				ez = elem/(h_nelx*h_nely);
				ey = (elem-ez*h_nelx*h_nely)/(h_nelx);
				ex = (elem-ez*h_nelx*h_nely - ey*h_nelx);
				elemMapped = ey*h_nelx*h_nely + ez*h_nelx + ex;
				for (i = 0; i < 6; i++)
					mu_elemstresses[problem][map3[i]][elemMapped] = mu_elemstresses[3][i][elem];
			}
		}
		else if (problem == 5) {//copy stresses from problem = 3
			#pragma omp parallel for private(ex,ey,ez,elemMapped,i)
			for (elem = 0; elem < h_nElems; elem++) {
				ez = elem/(h_nelx*h_nely);
				ey = (elem-ez*h_nelx*h_nely)/(h_nelx);
				ex = (elem-ez*h_nelx*h_nely - ey*h_nelx);
				elemMapped = ex*h_nelx*h_nely + ey*h_nelx + ez;
				for (i = 0; i < 6; i++)
					mu_elemstresses[problem][map4[i]][elemMapped] = mu_elemstresses[3][i][elem];
			}
		}
	}
	// compute CMatrix
	for (i = 0; i < 6; i++) {
		stressAverage[i] = 0.0;
		for (elem = 0; elem < h_nElems; elem++){
			if (getElemPseudoDensity(elem) == 0) continue;
			stressAverage[i] += elemVolume*mu_elemstresses[problem][i][elem];
		}
		stressAverage[i] /= bBoxVolume;	
		CMatrix[problem][i] = stressAverage[i];
	}

	return SUCCESS;
}
/**********************************************************************/
int MicroStructure::computeCMatrix(double CMatrix[6][6])
{
	// pose and solve 6 independent problems
	// The stress results are stored for all 6 problems
	int problem;

	if (xMin == xMax){
		printf("MicroStructure: Please initialize prior to calling this function \n");
		return FAILURE;
	}
	for (problem = 0; problem < 6; problem++) 
		solveConstantStrainProblem(problem,CMatrix);

	return SUCCESS;
}
/**********************************************************************/
void  MicroStructure::getJatAllVolumeFractions(double *vf, double *JComputedRelative, double *JExactRelative)
{
	for (int i =0; i <= iso_save_counter;i++){
		vf[i] = ts_volFracSave[i]/ts_volFracSave[0];
		JComputedRelative[i] = mu_JComputedSave[i]/mu_JComputedSave[0];
		JExactRelative[i] = mu_JExactSave[i]/mu_JExactSave[0];
	}
}
/**********************************************************************/
double MicroStructure::findDeterminantOfCMatrix(double C[6][6])
{
	TCLMatrix tempMatrix(6,6,0.0);
	int i,j;
	for ( i =0; i < 6; i++)
		for ( j =0; j < 6; j++)
			tempMatrix(i,j) = C[i][j];

	return tempMatrix.det();
}
/**********************************************************************/
void MicroStructure::extractMaterialProperties(double C[6][6],double *E,double *nu, double *K,double *G)
{
	double Cinv[6][6],J;
	int i,j;

	findInverseOfMaterialMatrix(C,Cinv);

	//assumes isotropy
	*nu = -(1./3)*(Cinv[0][1]/Cinv[0][0] + Cinv[1][2]/Cinv[1][1] + Cinv[0][2]/Cinv[2][2]);
	*E =  3./(Cinv[0][0]+Cinv[1][1]+Cinv[2][2]);

	J = 0;
	for ( i =0; i < 3; i++)
		for ( j =0; j < 3; j++)
			J += Cinv[i][j];
	*K = 1./J;

	J = 0;
	for ( i =3; i < 6; i++)
		J += Cinv[i][i];
	*G = 3./J;
}
/**********************************************************************/
int MicroStructure::findInverseOfMaterialMatrix(double C[6][6],double CInv[6][6])
{
	int i,j;
	TCLMatrix tempMatrix(3,3,0.0);
	// assume orthotropic material, so lots of zeros
	for ( i =0; i < 6; i++)
		for ( j =0; j < 6; j++)
			CInv[i][j] = 0.0;

	for ( i =0; i < 3; i++)
		for ( j =0; j < 3; j++)
			tempMatrix(i,j) = C[i][j];// only invert the 3x3 matrix

	if (tempMatrix.inv()){
		for ( i =0; i < 3; i++)
			for ( j =0; j < 3; j++)
				CInv[i][j] = tempMatrix(i,j);
	}
	else{
		printf("MicroStructure: Could not invert C Matrix \n");
		return FAILURE;
	}

	for ( i =3; i < 6; i++)
		CInv[i][i] = 1./C[i][i];
	return SUCCESS;
}
/**********************************************************************/
double  MicroStructure::computeTopologicalSensitivity(void)
{
	// Given T1 and T2, find topological sensitivity field
	// Equations 31 and 38 of Ozdemir,  "Topological derivative based ...", 2014
	int i,j,k,elem;
	double KTensor[6][6],temp1[6][6],CKC[6][6];
	double S1[6],S2[6],c[6];
	double CMatrix[6][6],CMatrixInv[6][6],J,temp,values[10];

	computeCMatrix(CMatrix);
	findInverseOfMaterialMatrix(CMatrix,CMatrixInv);
	if (mu_objective == X_RIGIDITY) {
		doubleContract(CMatrixInv,T1,c);
		doubleContract(c,T2,&J);
	}
	else if (mu_objective == BULK_MODULUS){
		doubleContract(CMatrixInv,T1,c);
		doubleContract(c,T2,&temp);
		J = 1./temp;
	}
	else if (mu_objective == SHEAR_MODULUS){
		J = 3./(CMatrixInv[3][3] + CMatrixInv[4][4] + CMatrixInv[5][5]);
	}
	else if (mu_objective == NEG_POISSON) {
		doubleContract(CMatrixInv,T1,c);
		doubleContract(c,T1,&(values[0]));
		doubleContract(CMatrixInv,T1,c);
		doubleContract(c,T2,&(values[1]));
		doubleContract(CMatrixInv,T1,c);
		doubleContract(c,T3,&(values[2]));


		doubleContract(CMatrixInv,T2,c);
		doubleContract(c,T1,&(values[3]));
		doubleContract(CMatrixInv,T2,c);
		doubleContract(c,T2,&(values[4]));
		doubleContract(CMatrixInv,T2,c);
		doubleContract(c,T3,&(values[5]));


		doubleContract(CMatrixInv,T3,c);
		doubleContract(c,T1,&(values[6]));
		doubleContract(CMatrixInv,T3,c);
		doubleContract(c,T2,&(values[7]));
		doubleContract(CMatrixInv,T3,c);
		doubleContract(c,T3,&(values[8]));

		J = -(1./6)*((values[1]+values[2])/values[0] + (values[3]+values[5])/values[4] + (values[6]+values[7])/values[8]);

	}
	ts_TSMin = 1e12;
	ts_TSMax = -1e12;
	for (elem = 0; elem < h_nElems; elem++) {
		//ts_topSensElem[elem] = 0;
		if (getElemPseudoDensity(elem) <0.5) continue;// use the previous TS value
		for (i =0; i < 6; i++) {
			for (k = 0; k < 6;k++)
				S1[k] = mu_elemstresses[i][k][elem];
			doubleContract(H,S1,c);
			for (j =0; j < 6; j++) {
				for (k = 0; k < 6;k++)
					S2[k] = mu_elemstresses[j][k][elem];
				doubleContract(c,S2,&(KTensor[i][j]));
			}
		}
		doubleContract(CMatrixInv,KTensor,temp1);
		doubleContract(temp1,CMatrixInv,CKC);
		if ( (mu_objective == X_RIGIDITY) || (mu_objective == BULK_MODULUS) || (mu_objective == BULK_MODULUS_2D)) {
			doubleContract(CKC,T1,c);
			doubleContract(c,T2,&(temp));
			ts_topSensElem[elem] = -temp;
		}
		else if (mu_objective == SHEAR_MODULUS) {
			ts_topSensElem[elem] = -(CKC[3][3] + CKC[4][4] + CKC[5][5]);
		}
		else if ( (mu_objective == NEG_POISSON) ) {
			//J = -(1./6)*((values[1]+values[2])/values[0] + (values[3]+values[5])/values[4] + (values[6]+values[7])/values[8]);

			ts_topSensElem[elem] = 0;

			doubleContract(CKC,T1,c);
			doubleContract(c,T2,&temp);
			ts_topSensElem[elem] -= temp/values[0];
			doubleContract(CKC,T1,c);
			doubleContract(c,T3,&temp);
			ts_topSensElem[elem] -= temp/values[0];
			doubleContract(CKC,T1,c);
			doubleContract(c,T1,&temp);
			ts_topSensElem[elem] += temp*(values[1]+values[2])/(values[0]*values[0]);

			doubleContract(CKC,T2,c);
			doubleContract(c,T1,&temp);
			ts_topSensElem[elem] -= temp/values[4];
			doubleContract(CKC,T2,c);
			doubleContract(c,T3,&temp);
			ts_topSensElem[elem] -= temp/values[4];
			doubleContract(CKC,T2,c);
			doubleContract(c,T2,&temp);
			ts_topSensElem[elem] += temp*(values[3]+values[5])/(values[4]*values[4]);

			doubleContract(CKC,T3,c);
			doubleContract(c,T1,&temp);
			ts_topSensElem[elem] -= temp/values[8];
			doubleContract(CKC,T3,c);
			doubleContract(c,T2,&temp);
			ts_topSensElem[elem] -= temp/values[8];
			doubleContract(CKC,T3,c);
			doubleContract(c,T3,&temp);
			ts_topSensElem[elem] += temp*(values[6]+values[7])/(values[8]*values[8]);


			ts_topSensElem[elem] *=-1;

		}
		ts_TSMin = MIN(ts_TSMin,ts_topSensElem[elem]);
		ts_TSMax = MAX(ts_TSMax,ts_topSensElem[elem]);
	}
	for (elem = 0; elem < h_nElems;elem++) {
		ts_topSensElem[elem] = (ts_topSensElem[elem]-ts_TSMin)/(ts_TSMax-ts_TSMin);
		if (getElemPseudoDensity(elem) < 0.5) ts_topSensElem[elem] = 0.0;
		
	}
	ts_TSMin = 0;
	ts_TSMax = 1;

	// J is the quantity that is being maximized
	return J;
}
/**********************************************************************/
void  MicroStructure::shiftByHalfCell(void)
// Shifts the microstructure solution by half-cell to be consistent with published results
{
	int nx,ny,nz,gridnode,offsetGridNode,nxBar,nyBar,nzBar,node,offsetNode;
	double value,*tempTSField;
	int isoCounter;
	// create a temporary field
	
	isoCounter = getCurrentIsoCounter();
	tempTSField = (double *)malloc(h_nNodes*sizeof(double));
	memset(tempTSField,0,h_nNodes*sizeof(double));

	// sweep through nodes and enter into new location
	for (nx = 0; nx <= h_nelx; nx++){
		nxBar = (int) ((h_nelx)*0.5 + nx)%(h_nelx+1);
		for (ny = 0; ny <= h_nely; ny++){
			nyBar = (int) ((h_nely)*0.5 + ny)%(h_nely+1);
			for (nz = 0; nz <= h_nelz; nz++){
				nzBar = (int) ((h_nelz)*0.5 + nz)%(h_nelz+1);

				gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
				node = h_gridNodeToRealNode[gridnode];
				value = ts_topSensNodeSave[isoCounter][node];	
				offsetGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
				offsetNode = h_gridNodeToRealNode[offsetGridNode];
				tempTSField[offsetNode] = value;
			}
		}
	}
	memcpy(ts_topSensNodeSave[isoCounter],tempTSField,h_nNodes*sizeof(double));
	free(tempTSField);
}

/**********************************************************************/
void  MicroStructure::stackCell(void)
// Creates a stack of two  microstructures
{
	int nx,ny,nz,gridnode,offsetGridNode,nxBar,nyBar,nzBar,node,offsetNode;
	double *tempTSField;
	int isoCounter;

	isoCounter = getCurrentIsoCounter();
	// create a temporary field
	tempTSField = (double *)malloc(h_nNodes*sizeof(double));
	memset(tempTSField,0,h_nNodes*sizeof(double));
	// sweep through nodes and enter into new location
	for (nx = 0; nx <= h_nelx; nx++){
		nxBar = (int) (2*nx)%(h_nelx);
		for (ny = 0; ny <= h_nely; ny++){
			nyBar = (int) (2*ny)%(h_nely);
			for (nz = 0; nz <= h_nelz; nz++){
				gridnode = nz*(h_nely+1)*(h_nelx+1) + ny*(h_nelx+1) + nx;
				node = h_gridNodeToRealNode[gridnode];
				nzBar = (int) (2*nz)%(h_nelz);
				offsetGridNode = nzBar*(h_nely+1)*(h_nelx+1) + nyBar*(h_nelx+1) + nxBar;
				offsetNode = h_gridNodeToRealNode[offsetGridNode];
				tempTSField[node] = ts_topSensNodeSave[isoCounter][offsetNode];;
			}
		}
	}
	memcpy(ts_topSensNodeSave[isoCounter],tempTSField,h_nNodes*sizeof(double));
	free(tempTSField);
}
/**********************************************************************/
void  MicroStructure::resetCell(void)
{
	int isoCounter;
	isoCounter = getCurrentIsoCounter();
	memcpy(ts_topSensNodeSave[isoCounter],ts_topSensNode,h_nNodes*sizeof(double));
}
/**********************************************************************/
int  MicroStructure::IterateMicroStructureToVolume(double desiredVolumeFraction, double *JConverged,
							 double *isoValueConverged)
// Fixed-point iteration to desired volume fraction
{
	double isoValue;
	int nIter,nElemsToRemove;
	double J,JOld;
	Timer timer;

	JOld = (*JConverged);
	J = JOld;
	if (MICROSTRUCTURE_VERBOSE) printf("MicroStructure: Iterating to volume \n");
	nIter = 0;
	while (nIter < ts_max_topoptIterations) {// to avoid cycles
		if (!computeIsovalue(desiredVolumeFraction,&isoValue,&nElemsToRemove))	return ITER_TO_VOL_ISOVALUE_FAILURE;	
		setElementToVoid(ts_restartTopology,ts_topSensElem,isoValue,nElemsToRemove);
		if (MICROSTRUCTURE_VERBOSE) printf("MicroStructure: J = %.2f \n",J);
		if ((nIter >= ts_min_topoptIterations) && (fabs((JOld-J)/JOld) < ts_relativeJTolerance) ){
			*JConverged = J;
			*isoValueConverged = isoValue;
			return ITER_TO_VOL_SUCCESS;;//success
		}
		J = computeTopologicalSensitivity();
		connectivityTSSmoothener();
		JOld = J;
		nIter++;
	}
	return ITERATION_ERROR;
}
/**********************************************************************/
double MicroStructure::optimizeMicrostructure(double desiredVolFrac,double volDecrementMax, int maxFEA,int *nFEA, 
											  bool (*callbackFunction)(double isoValueConverged),
										double *isoValueConverged,double *volDecrementStart)
{
	double J,JTemp,J0,isoTemp,currentVolfrac,convergedVolFraction,tempVolDecrement;
	int flag,nElemsToRemove;
	Timer timer;
	
	if (MICROSTRUCTURE_VERBOSE) printf("MicroStructure: Optimizing ... \n");
	if (ts_topSensElem == NULL) {
		printf("MicroStructure: Not initialized!\n");
		return 1.0;
	}
	currentVolfrac = getCurrentVolumeFraction();// we might be restarting from a previously converged result
	convergedVolFraction = currentVolfrac;
	if (MICROSTRUCTURE_VERBOSE) printf("MicroStructure: currentVolfrac = %f \n",currentVolfrac);
	resetLinearSolvesCounter();
	*nFEA = 0;
	J = computeTopologicalSensitivity();
	connectivityTSSmoothener();

	if (currentVolfrac > 0.99) {
		J0 = J;
		iso_save_counter = -1;//start saving
	}
	tempVolDecrement = *volDecrementStart;
	computeIsovalue(currentVolfrac,&isoTemp,&nElemsToRemove);
	saveMicrostructureResult(currentVolfrac,isoTemp,J0, false);
	pushToSavedData();
	
	while (currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac) {// small error at the end acceptable
		convergedVolFraction = currentVolfrac;
		tempVolDecrement = MIN(tempVolDecrement,currentVolfrac-desiredVolFrac);
		JTemp = J;
		*nFEA = getLinearSolvesCounter();
		if (*nFEA > maxFEA) {
			printf("MicroStructure: #FEA exceeded max allowed of %d \n",maxFEA);
			break;
		}
		if (MICROSTRUCTURE_VERBOSE) printf("-------------------------------------------------------\n");		
		if (MICROSTRUCTURE_VERBOSE)  printf("\n");
		flag = IterateMicroStructureToVolume(currentVolfrac-tempVolDecrement,&JTemp,&isoTemp);
		if ((flag == ITER_TO_VOL_SUCCESS))  {//converged	
			//removeHangingElems();
			currentVolfrac = getCurrentVolumeFraction();
			convergedVolFraction = currentVolfrac;
			saveMicrostructureResult(convergedVolFraction,isoTemp,JTemp,false);// this is for plotting
			pushToSavedData();// this is for recovery from failure
			J = JTemp;	
			printf("Optimized: %.3f -> %.3f;  JStar/J0 = %.2f\n",currentVolfrac+tempVolDecrement,currentVolfrac,getHashinSthrikmanBound(convergedVolFraction));
			*isoValueConverged = isoTemp;
			tempVolDecrement = min(volDecrementMax,1.1*(tempVolDecrement));
			(*callbackFunction)(*isoValueConverged);
		}
		else {
			printf("MicroStructure: Failed at %.3f \n",currentVolfrac-tempVolDecrement);
			retrieveFromSavedData();
			postprocess();
			currentVolfrac = convergedVolFraction;
			tempVolDecrement *= 0.5;
		}
		if ((currentVolfrac > (1+ts_volErrorFraction)*desiredVolFrac) && (tempVolDecrement < ts_volErrorFraction)){
			currentVolfrac = convergedVolFraction;
			break;
		}
	}
	saveMicrostructureResult(convergedVolFraction,isoTemp, JTemp,true);
	*volDecrementStart = tempVolDecrement;
	return convergedVolFraction;
}
/**********************************************************************/
MicroStructure::~MicroStructure()
{
	if (MICROSTRUCTURE_VERBOSE) printf("Releasing MicroStructure Object \n");
	for (int problem = 0; problem < 6; problem++)
		for (int component = 0; component < 6; component++)
			safeFree(mu_elemstresses[problem][component]);
}
