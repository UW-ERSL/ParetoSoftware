#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define DIST3D(p1,p2) (sqrt((p2[0]-p1[0])*(p2[0]-p1[0])+(p2[1]-p1[1])*(p2[1]-p1[1])+(p2[2]-p1[2])*(p2[2]-p1[2])))
using namespace std;
using namespace Eigen;
/**********************************************************************/
void AFElasticity::resetDeformationGradient()
{
	if (h_gP == 1) {
		memset(deformGrad, 0, 9*h_nElems*sizeof(double));// Reset to zero vector
	}
	else {
		memset(deformGrad, 0, 72*h_nElems*sizeof(double));// Reset to zero vector
	}

}
/**********************************************************************/
void AFElasticity::updateDeformGradAndStress()
{
	int elem, i, j, gp;
	double gradNx[8],gradNy[8],gradNz[8], detJ;
	double H[3][3], S[3][3];

	//resetDeformationGradient();
	
	if (h_gP == 1){
		if (h_voxelgrid) {
			detJ = computeShapeGradient(0, 0, 0, 0, gradNx, gradNy, gradNz);
			for (elem = 0; elem < h_nElems; elem++) {
				// Current node Stress and Displacement Gradient from h_sol
				computeStressAndDeformGradAtGP(elem, gradNx, gradNy, gradNz, H, S);
				// Update Deformation Gradient
				for (i = 0; i < 3; i++)
					for (j = 0; j < 3; j++)
						if (i == j)
							deformGrad[(3*i + j)*h_nElems + elem] = (1 + H[i][j]);
						else
							deformGrad[(3*i + j)*h_nElems + elem] = H[i][j];
	

				// Update PK2 Stress
				pk2Stress [0*h_nElems + elem] = S[0][0];
				pk2Stress [1*h_nElems + elem] = S[1][1];
				pk2Stress [2*h_nElems + elem] = S[2][2];
				pk2Stress [3*h_nElems + elem] = S[0][1];
				pk2Stress [4*h_nElems + elem] = S[0][2];
				pk2Stress [5*h_nElems + elem] = S[1][2];
			}
		}
		else
			for (elem = 0; elem < h_nElems; elem++) {
				// Current shape functions gradient for 8-node element
				detJ = computeShapeGradient(elem, 0, 0, 0, gradNx, gradNy, gradNz);
				// Current node Stress and Displacement Gradient from h_sol
				computeStressAndDeformGradAtGP(elem, gradNx, gradNy, gradNz, H, S);
				// Update Deformation Gradient
				for (i = 0; i < 3; i++)
					for (j = 0; j < 3; j++)
						if (i == j)
							deformGrad[(3*i + j)*h_nElems + elem] = (1 + H[i][j]);
						else
							deformGrad[(3*i + j)*h_nElems + elem] = H[i][j];

				// PK2 Stress tensor
				pk2Stress [0*h_nElems + elem] = S[0][0];
				pk2Stress [1*h_nElems + elem] = S[1][1];
				pk2Stress [2*h_nElems + elem] = S[2][2];
				pk2Stress [3*h_nElems + elem] = S[0][1];
				pk2Stress [4*h_nElems + elem] = S[0][2];
				pk2Stress [5*h_nElems + elem] = S[1][2];
			}
	}
	// For 8 gauss points
	else {
		double a = 1.0/sqrt(3.0);
		double xiValues[8] = {-a,-a,a,a,-a,-a,a,a};
		double etaValues[8] = {-a,a,-a,a,-a,a,-a,a};
		double muValues[8] = {-a,-a,-a,-a,a,a,a,a};

		for (gp = 0; gp < 8; gp++) {
			if (h_voxelgrid) {
				detJ = computeShapeGradient(0, xiValues[gp], etaValues[gp], muValues[gp], gradNx, gradNy, gradNz);
				for (elem = 0; elem < h_nElems; elem++) {
					// Current node Stress and Displacement Gradient from h_sol
					computeStressAndDeformGradAtGP(elem, gradNx, gradNy, gradNz, H, S);
					// Update Deformation Gradient
					for (i = 0; i < 3; i++)
						for (j = 0; j < 3; j++)
							if (i == j)
								deformGrad[(9*gp + 3*i + j)*h_nElems + elem] = (1 + H[i][j]);
							else
								deformGrad[(9*gp + 3*i + j)*h_nElems + elem] = H[i][j];

							

					// Update PK2 Stress
					pk2Stress [(6*gp + 0)*h_nElems + elem] = S[0][0];
					pk2Stress [(6*gp + 1)*h_nElems + elem] = S[1][1];
					pk2Stress [(6*gp + 2)*h_nElems + elem] = S[2][2];
					pk2Stress [(6*gp + 3)*h_nElems + elem] = S[0][1];
					pk2Stress [(6*gp + 4)*h_nElems + elem] = S[0][2];
					pk2Stress [(6*gp + 5)*h_nElems + elem] = S[1][2];
				}
			}
			else
				for (elem = 0; elem < h_nElems; elem++) {
					// Current shape functions gradient for 8-node element
					detJ = computeShapeGradient(0, xiValues[gp], etaValues[gp], muValues[gp], gradNx, gradNy, gradNz);
					// Current node Stress and Displacement Gradient from h_sol
					computeStressAndDeformGradAtGP(elem, gradNx, gradNy, gradNz, H, S);
					// Update Deformation Gradient
					for (i = 0; i < 3; i++)
						for (j = 0; j < 3; j++)
							if (i == j)
								deformGrad[(9*gp + 3*i + j)*h_nElems + elem] = (1 + H[i][j]);
							else
								deformGrad[(9*gp + 3*i + j)*h_nElems + elem] = H[i][j];

					// Update PK2 Stress
					pk2Stress [(6*gp + 0)*h_nElems + elem] = S[0][0];
					pk2Stress [(6*gp + 1)*h_nElems + elem] = S[1][1];
					pk2Stress [(6*gp + 2)*h_nElems + elem] = S[2][2];
					pk2Stress [(6*gp + 3)*h_nElems + elem] = S[0][1];
					pk2Stress [(6*gp + 4)*h_nElems + elem] = S[0][2];
					pk2Stress [(6*gp + 5)*h_nElems + elem] = S[1][2];
				}
		}
	}			
}
/**********************************************************************/
void AFElasticity::computeNLTemplate(int elem, double wt, double detJ, double Nx[8], double Ny[8], double Nz[8], double Ke[24][24])
{
	int i, j, m, n;

	TCLMatrix D(3,3,0.0);
	TCLMatrix G(3,3,0.0);
	TCLMatrix N1(3,3,0.0);
	TCLMatrix N2(3,3,0.0);
	TCLMatrix N1CP(3,3,0.0);
	TCLMatrix N2CP(3,3,0.0);
	TCLMatrix K_pair(3,3,0.0);

	int offset = 36*h_materialIdOfElem[elem];

	D[0][0] = h_DMatrices[offset];
	D[0][1] = h_DMatrices[offset + 1];
	D[0][2] = h_DMatrices[offset + 2];
	D[1][0] = h_DMatrices[offset + 6];
	D[1][1] = h_DMatrices[offset + 7];
	D[1][2] = h_DMatrices[offset + 8];
	D[2][0] = h_DMatrices[offset + 12];
	D[2][1] = h_DMatrices[offset + 13];
	D[2][2] = h_DMatrices[offset + 14];

	G[0][0] = h_DMatrices[offset + 21];
	G[1][1] = h_DMatrices[offset + 28];
	G[2][2] = h_DMatrices[offset + 35];

	for (i = 0; i < 8; i++){
		N1[0][0] = Nx[i]; N1[1][1] = Ny[i]; N1[2][2] = Nz[i];
		N1CP[1][0] = Nz[i]; N1CP[2][0] = Ny[i]; N1CP[2][1] = Nx[i];
		N1CP[0][1] = Nz[i]; N1CP[0][2] = Ny[i]; N1CP[1][2] = Nx[i];
		for (j = 0; j < 8; j++) {
			N2[0][0] = Nx[j]; N2[1][1] = Ny[j]; N2[2][2] = Nz[j];
			N2CP[1][0] = Nz[j]; N2CP[2][0] = Ny[j]; N2CP[2][1] = Nx[j];
			N2CP[0][1] = Nz[j]; N2CP[0][2] = Ny[j]; N2CP[1][2] = Nx[j];

			K_pair = detJ*wt*(N1*D*N2 + N1CP*G*N2CP);
			for (m = 0; m < 3; m++)
				for (n = 0; n < 3; n++)
					Ke[3*i + m][3*j + n] = K_pair[m][n];
		}
	}
}
/**********************************************************************/
void AFElasticity::computeNonLinearKeTemplates()
{
	Timer timer;
	int i,j,k,templateElem, elem;
	double Nx[8],Ny[8],Nz[8];
	double Ke[24][24], detJ;

	
	timer.start();
	if (h_hexmesh_verbose) printf("AFElasticity: Computing Non Linear Ke for Templates\n");

	safeFree(h_KENLTemplates); h_KENLTemplates = NULL;
	/*safeFree(h_gradShape);h_gradShape = NULL;
	safeFree(h_detJ);h_detJ = NULL;*/

	if (h_hexmesh_verbose) printf("AFElasticity: #TemplateElements = %d \n",h_nTemplateElements);
	if (h_hexmesh_verbose) printf("AFElasticity: #Materials = %d \n",h_nMaterials);

	if (h_gP == 1) {
		h_KENLTemplates =	(double *)malloc((h_nTemplateElements)*24*24*sizeof(double));
		/*h_gradShape = (double *)malloc((h_nTemplateElements)*24*sizeof(double));
		h_detJ = (double *)malloc((h_nTemplateElements)*sizeof(double));*/
	}
	else {
		h_KENLTemplates =	(double *)malloc((h_nTemplateElements)*24*24*8*sizeof(double)); // for 2 X 2 X 2 gauss points
		/*h_gradShape = (double *)malloc((h_nTemplateElements)*8*24*sizeof(double));
		h_detJ = (double *)malloc((h_nTemplateElements)*8*sizeof(double));*/
	}

	for (templateElem = 0; templateElem < h_nTemplateElements; templateElem++) {
		if (h_nTemplateElements == h_nElems) 
			elem = templateElem;
		else
			for (elem = 0; elem < h_nElems; elem++)// find an elem with that ID, BAD ALGORITHM!!!
				if (h_templateID[elem] == templateElem)
					break;

		if (h_gP == 1) {
			detJ = computeShapeGradient(elem, 0, 0, 0, Nx, Ny, Nz);
			computeNLTemplate(elem, 8, detJ, Nx, Ny, Nz, Ke);

			k =0;
			for (i = 0; i < 24; i++) {
				for( j = 0; j < 24;j++) {
					h_KENLTemplates[k*(h_nTemplateElements) + templateElem] = (double) Ke[i][j];
					k++;
				}
			}
			/*for (i = 0; i < 8; i++) {
				h_gradShape[i*(h_nTemplateElements) + templateElem] = Nx[i];
				h_gradShape[(8 + i)*(h_nTemplateElements) + templateElem] = Ny[i];
				h_gradShape[(16 + i)*(h_nTemplateElements) + templateElem] = Nz[i];
			}
			h_detJ[templateElem] = detJ;*/
		}
		else {
			double a = 1.0/sqrt(3.0);
			double xiValues[8] = {-a,-a,a,a,-a,-a,a,a};
			double etaValues[8] = {-a,a,-a,a,-a,a,-a,a};
			double muValues[8] = {-a,-a,-a,-a,a,a,a,a};
			int gp;

			for (gp = 0; gp < 8; gp++) {
				detJ = computeShapeGradient(elem, xiValues[gp], etaValues[gp], muValues[gp], Nx, Ny, Nz);
				computeNLTemplate(elem, 1, detJ, Nx, Ny, Nz, Ke);
				k =0;
				for (i = 0; i < 24; i++) {
					for( j = 0; j < 24;j++) {
						h_KENLTemplates[(24*24*gp + k)*(h_nTemplateElements) + templateElem] = (double) Ke[i][j];
						k++;
					}
				}
				/*for (i = 0; i < 8; i++) {
					h_gradShape[(24*gp + i)*(h_nTemplateElements) + templateElem] = Nx[i];
					h_gradShape[(24*gp + 8 + i)*(h_nTemplateElements) + templateElem] = Ny[i];
					h_gradShape[(24*gp + 16 + i)*(h_nTemplateElements) + templateElem] = Nz[i];
				}
				h_detJ[gp*(h_nTemplateElements) + templateElem] = detJ;*/
			}
		}
	}
	if (h_hexmesh_verbose) printf("AFElasticity: Time taken to set Non Linear Templates = %f \n",timer.getElapsedTimeInSec());
}
/**********************************************************************/
void AFElasticity::computeInternalFCenter(int elem, double *f)
{
	int i, j;
	double x[8],y[8],z[8],u[8],v[8],w[8];
	int nodes[8];
	double gradNxi[8],gradNeta[8],gradNmu[8],T1,T2,T3, detJ;
	TCLMatrix J(3,3,0.0);
	TCLMatrix GL_E(3,3,0.0);
	TCLMatrix H(3,3,0.0);
	
	// Current node deformation from h_sol
	getElemNodeDeformation(elem,u,v,w);

	// if the mesh is of uniform grid, can reduce computation
	computeGrad_H8(0,0,0,gradNxi,gradNeta,gradNmu);
	// Initialize Shape function gradient
		
	if (h_voxelgrid) {
		J[0][0] = 2./h_dx;//Jinv
		J[1][1] = 2./h_dy;
		J[2][2] = 2./h_dz;
		
	 
		detJ = h_dx*h_dy*h_dz/8.0;
		for (i = 0; i < 8; i++) {// contribution from all 8 nodes
			T1 = J[0][0]*gradNxi[i];
			T2 = J[1][1]*gradNeta[i];
			T3 = J[2][2]*gradNmu[i];

			H[0][0] += T1*u[i];// ux
			H[0][1] += T2*u[i];// uy
			H[0][2] += T3*u[i];// uz
			H[1][0] += T1*v[i];// vx
			H[1][1] += T2*v[i];// vy
			H[1][2] += T3*v[i];// vz
			H[2][0] += T1*w[i];// wx
			H[2][1] += T2*w[i];// wy
			H[2][2] += T3*w[i];// wz
		}
	}
	else {

		J = 0;
		getElemNodeLocation(elem,x,y,z);
		for (i =0; i < 8; i++) {
			J[0][0] += gradNxi[i]*x[i];
			J[0][1] += gradNxi[i]*y[i];
			J[0][2] += gradNxi[i]*z[i];

			J[1][0] += gradNeta[i]*x[i];
			J[1][1] += gradNeta[i]*y[i];
			J[1][2] += gradNeta[i]*z[i];

			J[2][0] += gradNmu[i]*x[i];
			J[2][1] += gradNmu[i]*y[i];
			J[2][2] += gradNmu[i]*z[i];
		}
		detJ = J.det();
		J.inv();
		for (i = 0; i < 8; i++) {// contribution from all 8 nodes
			T1 = J[0][0]*gradNxi[i] + J[0][1]*gradNeta[i] + J[0][2]*gradNmu[i];
			T2 = J[1][0]*gradNxi[i] + J[1][1]*gradNeta[i] + J[1][2]*gradNmu[i];
			T3 = J[2][0]*gradNxi[i] + J[2][1]*gradNeta[i] + J[2][2]*gradNmu[i];

			H[0][0] += T1*u[i];// ux
			H[0][1] += T2*u[i];// vy
			H[0][2] += T3*u[i];// wz
			H[1][0] += T1*v[i];// ux
			H[1][1] += T2*v[i];// vy
			H[1][2] += T3*v[i];// wz
			H[2][0] += T1*w[i];// ux
			H[2][1] += T2*w[i];// vy
			H[2][2] += T3*w[i];// wz
		}
	}

	// Update Green Lagrange Strain
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			GL_E[i][j] = (H[i][j] + H[j][i] + H[i][0]*H[j][0] + H[i][1]*H[j][1] + H[i][2]*H[j][2])/2;
	
	// Do I need deformation gradient here?
	
		
	// Compute 2nd PK Stress in Voigt Form
	int offset = 36*h_materialIdOfElem[elem];

	// Temp variables
	TCLMatrix P(3,3,0.0);
	TCLMatrix S(3,3,0.0);
	TCLMatrix F(3,3,0.0);

	for (i = 0; i <3; i++) {
		S[0][0] += h_DMatrices[offset+i]*GL_E[i][i];//sxx
		S[1][1] += h_DMatrices[offset+6+i]*GL_E[i][i];//syy
		S[2][2] += h_DMatrices[offset+12+i]*GL_E[i][i];//szz
	}
	S[0][1] = h_DMatrices[offset+21]*(GL_E[0][1] + GL_E[1][0]);//sxy
	S[0][2] = h_DMatrices[offset+28]*(GL_E[0][2] + GL_E[2][0]);//sxz
	S[1][2] = h_DMatrices[offset+35]*(GL_E[1][2] + GL_E[2][1]);//syz

	S[2][1] = S[1][2]; S[2][0] = S[0][2]; S[1][0] = S[0][1]; 

	// Deformation gradient
	H[0][0] += 1; H[1][1] += 1; H[2][2] += 1; 
	F = ~H; // Deformation gradient Transpose
	P = S*F; // Nominal Stress from 2nd PK Stress
	getElemNodes(elem, nodes);

	if (h_voxelgrid)
	for (i = 0; i < 8; i++) {// contribution from all 8 nodes

		T1 = J[0][0]*gradNxi[i];
		T2 = J[1][1]*gradNeta[i];
		T3 = J[2][2]*gradNmu[i];
		
		f[3*nodes[i]] += 8*detJ*(T1*P[0][0] + T2*P[1][0] + T3*P[2][0]);
		f[3*nodes[i] + 1] += 8*detJ*(T1*P[0][1] + T2*P[1][1] + T3*P[2][1]);
		f[3*nodes[i] + 2] += 8*detJ*(T1*P[0][2] + T2*P[1][2] + T3*P[2][2]);
	}
	else
		for (i = 0; i < 8; i++) {// contribution from all 8 nodes
		T1 = J[0][0]*gradNxi[i] + J[0][1]*gradNeta[i] + J[0][2]*gradNmu[i];
		T2 = J[1][0]*gradNxi[i] + J[1][1]*gradNeta[i] + J[1][2]*gradNmu[i];
		T3 = J[2][0]*gradNxi[i] + J[2][1]*gradNeta[i] + J[2][2]*gradNmu[i];
			
		f[3*nodes[i]] += 8*detJ*(T1*P[0][0] + T2*P[1][0] + T3*P[2][0]);
		f[3*nodes[i] + 1] += 8*detJ*(T1*P[0][1] + T2*P[1][1] + T3*P[2][1]);
		f[3*nodes[i] + 2] += 8*detJ*(T1*P[0][2] + T2*P[1][2] + T3*P[2][2]);
	}
}
/**********************************************************************/
double AFElasticity::computeShapeGradient(int elem, double xi, double eta, double mu, double Nx[8], double Ny[8], double Nz[8])
{
	int i;
	double x[8],y[8],z[8],gradNxi[8],gradNeta[8],gradNmu[8], detJ;
	TCLMatrix J(3,3,0.0);

	// if the mesh is of uniform grid, can reduce computation
	computeGrad_H8(xi,eta,mu,gradNxi,gradNeta,gradNmu);
	if (h_voxelgrid) {
		J[0][0] = 2./h_dx;//Jinv
		J[1][1] = 2./h_dy;
		J[2][2] = 2./h_dz;
		detJ = h_dx*h_dy*h_dz/8.0;

		for (i = 0; i < 8; i++) {// contribution from all 8 nodes
			Nx[i] = J[0][0]*gradNxi[i];
			Ny[i] = J[1][1]*gradNeta[i];
			Nz[i] = J[2][2]*gradNmu[i];
		}
	}
	else {

		J = 0;
		getElemNodeLocation(elem,x,y,z);
		for (i =0; i < 8; i++) {
			J[0][0] += gradNxi[i]*x[i];
			J[0][1] += gradNxi[i]*y[i];
			J[0][2] += gradNxi[i]*z[i];

			J[1][0] += gradNeta[i]*x[i];
			J[1][1] += gradNeta[i]*y[i];
			J[1][2] += gradNeta[i]*z[i];

			J[2][0] += gradNmu[i]*x[i];
			J[2][1] += gradNmu[i]*y[i];
			J[2][2] += gradNmu[i]*z[i];
		}
		detJ = J.det();
		J.inv();
		for (i = 0; i < 8; i++) {// contribution from all 8 nodes
			Nx[i] = J[0][0]*gradNxi[i] + J[0][1]*gradNeta[i] + J[0][2]*gradNmu[i];
			Ny[i] = J[1][0]*gradNxi[i] + J[1][1]*gradNeta[i] + J[1][2]*gradNmu[i];
			Nz[i] = J[2][0]*gradNxi[i] + J[2][1]*gradNeta[i] + J[2][2]*gradNmu[i];
		}
	}

	return detJ;
}
/**********************************************************************/
void AFElasticity::computeStressAndDeformGradAtGP(int elem, double gradNx[8], double gradNy[8], double gradNz[8], double H[3][3], double S[3][3])
{
	int i,j;
	double u[8],v[8],w[8];
	double T1,T2,T3;
	TCLMatrix GL_E(3,3,0.0);
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++) {
			S[i][j] = 0;
			H[i][j] = 0;
		}
	// Current node deformation from h_sol
	getElemNodeDeformation(elem,u,v,w);
	
	for (i = 0; i < 8; i++) {// contribution from all 8 nodes
		T1 = gradNx[i];
		T2 = gradNy[i];
		T3 = gradNz[i];

		H[0][0] += T1*u[i];// ux
		H[0][1] += T2*u[i];// vy
		H[0][2] += T3*u[i];// wz
		H[1][0] += T1*v[i];// ux
		H[1][1] += T2*v[i];// vy
		H[1][2] += T3*v[i];// wz
		H[2][0] += T1*w[i];// ux
		H[2][1] += T2*w[i];// vy
		H[2][2] += T3*w[i];// wz
	}				

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			GL_E[i][j] = (H[i][j] + H[j][i] + H[i][0]*H[j][0] + H[i][1]*H[j][1] + H[i][2]*H[j][2])/2;// Green Lagrange strain
	
	// Do I need deformation gradient here?
		
	// Compute 2nd PK Stress in Voigt Form
	int offset = 36*h_materialIdOfElem[elem];			

	for (i = 0; i <3; i++) {
		S[0][0] += h_DMatrices[offset+i]*GL_E[i][i];//sxx
		S[1][1] += h_DMatrices[offset+6+i]*GL_E[i][i];//syy
		S[2][2] += h_DMatrices[offset+12+i]*GL_E[i][i];//szz
	}
	S[0][1] = h_DMatrices[offset+21]*(GL_E[0][1] + GL_E[1][0]);//sxy
	S[0][2] = h_DMatrices[offset+28]*(GL_E[0][2] + GL_E[2][0]);//sxz
	S[1][2] = h_DMatrices[offset+35]*(GL_E[1][2] + GL_E[2][1]);//syz
		
	// Stress tensor
	S[2][1] = S[1][2]; S[2][0] = S[0][2]; S[1][0] = S[0][1]; 
}
/**********************************************************************/
void AFElasticity::computeInternalF_GP(int elem, int gp, double detJ, double wt, double gradNx[8], double gradNy[8], double gradNz[8], double *f)
{
	int i, j;
	int nodes[8];
	double T1,T2,T3;

	// Do I need deformation gradient here?
	// Temp variables
	TCLMatrix P(3,3,0.0);
	TCLMatrix S(3,3,0.0);
	TCLMatrix F(3,3,0.0);
	
	// Get deformation gradient for the given gauss point
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			F[j][i] = deformGrad[(9*gp + 3*i + j)*h_nElems + elem];
	
	// Get 2nd PK Stress for element at given gauss point
	S[0][0] = pk2Stress [(6*gp + 0)*h_nElems + elem];
	S[1][1] = pk2Stress [(6*gp + 1)*h_nElems + elem];
	S[2][2] = pk2Stress [(6*gp + 2)*h_nElems + elem];
	S[0][1] = pk2Stress [(6*gp + 3)*h_nElems + elem];
	S[0][2] = pk2Stress [(6*gp + 4)*h_nElems + elem];
	S[1][2] = pk2Stress [(6*gp + 5)*h_nElems + elem];

	S[1][0] = S[0][1]; S[2][0] = S[0][2]; S[2][1] = S[1][2];
	
	// Nominal Stress from 2nd PK Stress
	P = S*F; 
	// Get node indexes for ELement
	getElemNodes(elem, nodes);
	 
	for (i = 0; i < 8; i++) {// contribution from all 8 nodes
		T1 = gradNx[i];
		T2 = gradNy[i];
		T3 = gradNz[i];
			
		f[3*nodes[i]] += wt*detJ*(T1*P[0][0] + T2*P[1][0] + T3*P[2][0]);
		f[3*nodes[i] + 1] += wt*detJ*(T1*P[0][1] + T2*P[1][1] + T3*P[2][1]);
		f[3*nodes[i] + 2] += wt*detJ*(T1*P[0][2] + T2*P[1][2] + T3*P[2][2]);
	}
}
/**********************************************************************/
void AFElasticity::updateInternalForce(double *f)
{
	double a = 1.0/sqrt(3.0), detJ = 0;
	double xiValues[8] = {-a,-a,a,a,-a,-a,a,a};
	double etaValues[8] = {-a,a,-a,a,-a,a,-a,a};
	double muValues[8] = {-a,-a,-a,-a,a,a,a,a};
	double Nx[8], Ny[8], Nz[8];

	memset(f, 0, 3*h_nNodes*sizeof(double));// Reset to zero vector

	if (h_gP == 1)
		if (h_voxelgrid) {
				detJ = computeShapeGradient(0, 0, 0, 0, Nx, Ny, Nz);
				for(int elem = 0; elem < h_nElems; elem++)
					computeInternalF_GP(elem, 0, detJ, 8, Nx, Ny, Nz,  f);
			}
			else
				for(int elem = 0; elem < h_nElems; elem++) {
					detJ = computeShapeGradient(elem, 0, 0, 0, Nx, Ny, Nz);
					computeInternalF_GP(elem, 0, detJ, 8, Nx, Ny, Nz,  f);
				}

	else
		for (int i = 0; i < 8; i++) {
			if (h_voxelgrid) {
				detJ = computeShapeGradient(0, xiValues[i], etaValues[i], muValues[i], Nx, Ny, Nz);
				for(int elem = 0; elem < h_nElems; elem++)
					computeInternalF_GP(elem, i, detJ, 1, Nx, Ny, Nz,  f);
			}
			else
				for(int elem = 0; elem < h_nElems; elem++) {
					detJ = computeShapeGradient(elem, xiValues[i], etaValues[i], muValues[i], Nx, Ny, Nz);
					computeInternalF_GP(elem, i, detJ, 1, Nx, Ny, Nz,  f);
				}
		}
	// remember non zero forces on fixed nodes
	enforceDirichlet0(f);
}

/**********************************************************************/
void AFElasticity::computeMaterialKgUNodalatGP(int node, int gP, double wt, double detJ, double N_x[8], double N_y[8], double N_z[8], double *x, double *y)
{
	// Kg for an isotropic element of arbitrary aspect ratio
	int i, j, m, n, maxElem, codedElem, nodeIndex, elem, elemID, node2, row; 
	double uVal = 0, vVal = 0, wVal = 0, t1, sxx, sxy, sxz, syy, syz, szz;

	TCLMatrix F(3,3,0.0);
	TCLMatrix Ft(3,3,0.0);
	TCLMatrix Ktemp(3,3,0.0);

	TCLMatrix K_pair(3,3,0.0);
	
	maxElem = h_nNeighboringElems[node];// how many elements connected to node
	for (j =0; j < maxElem; j++) {//
		codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
		nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
		elem = codedElem - nodeIndex*MAXELEMS;
		elemID = h_templateID[elem];
		// Get deformation Gradient for the Element
		for (m = 0; m < 3; m++)
			for (n = 0; n < 3; n++)
				F[m][n] = deformGrad[(9*gP + 3*m + n)*h_nElems + elem];
		Ft = ~F;

		sxx = pk2Stress [(6*gP + 0)*h_nElems + elem];
		syy = pk2Stress [(6*gP + 1)*h_nElems + elem];
		szz = pk2Stress [(6*gP + 2)*h_nElems + elem];
		sxy = pk2Stress [(6*gP + 3)*h_nElems + elem];
		sxz = pk2Stress [(6*gP + 4)*h_nElems + elem];
		syz = pk2Stress [(6*gP + 5)*h_nElems + elem];
		row = 24*3*nodeIndex;
			
		for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
			node2 = h_elems[(h_nElems)*i + elem];
			for (m = 0; m < 3; m++)
				for (n = 0; n < 3; n++)
					Ktemp[m][n] = h_KENLTemplates[(24*24*gP + row + 24*m + 3*i + n)*(h_nTemplateElements) + elemID];

			K_pair = F*Ktemp*Ft;
			t1 = wt*detJ*(N_x[nodeIndex]*(sxx*N_x[i] + sxy*N_y[i] + sxz*N_z[i])
					+ N_y[nodeIndex]*(sxy*N_x[i] + syy*N_y[i] + syz*N_z[i])
					+ N_z[nodeIndex]*(sxz*N_x[i] + syz*N_y[i] + szz*N_z[i]));

			uVal += ((K_pair[0][0] + t1)*x[3*node2] + K_pair[0][1]*x[3*node2 + 1] + K_pair[0][2]*x[3*node2 + 2]);
			vVal += (K_pair[1][0]*x[3*node2] + (K_pair[1][1] + t1)*x[3*node2 + 1] + K_pair[1][2]*x[3*node2 + 2]);
			wVal += (K_pair[2][0]*x[3*node2] + K_pair[2][1]*x[3*node2 + 1] + (K_pair[2][2] + t1)*x[3*node2 + 2]);
		}
	}
	y[3*node] += uVal;
	y[3*node + 1] += vVal;
	y[3*node + 2] += wVal;
}
/**********************************************************************/
void AFElasticity::computeKgUNonLinear(double *u, double *result)
{
	int node1,freeEncoded,ufree,vfree,wfree, gP;
	double Nx[8], Ny[8], Nz[8], detJ;
	
	memset(result,0,3*h_nNodes*sizeof(double));
	if (h_gP == 1) {
		detJ = computeShapeGradient(0, 0, 0, 0, Nx, Ny, Nz);
		for (node1 = 0; node1 < h_nNodes; node1++) 
			computeMaterialKgUNodalatGP(node1, 0, 8, detJ, Nx, Ny, Nz, u, result);
	}
	else {
		double a = 1.0/sqrt(3.0);
		double xiValues[8] = {-a,-a,a,a,-a,-a,a,a};
		double etaValues[8] = {-a,a,-a,a,-a,a,-a,a};
		double muValues[8] = {-a,-a,-a,-a,a,a,a,a};
		for (gP = 0; gP < 8; gP++) {
			detJ = computeShapeGradient(0, xiValues[gP], etaValues[gP], muValues[gP], Nx, Ny, Nz);
			for (node1 = 0; node1 < h_nNodes; node1++)
				computeMaterialKgUNodalatGP(node1, gP, 1, detJ, Nx, Ny, Nz, u, result);
		}
	}

	// enforce Dirichlet conditions
	for (node1 = 0; node1 < h_nNodes; node1++) {
		freeEncoded = h_free[node1];
		wfree = freeEncoded >> 2;
		vfree = (freeEncoded - wfree*4)>>1;
		ufree = (freeEncoded - (wfree << 2) -(vfree<<1));
		result[3*node1] *= ufree ;// Dirichlet condition is enforced
		result[3*node1+1] *= vfree;
		result[3*node1+2] *= wfree;
	}
}
/**********************************************************************/
void AFElasticity::computeNonLinearKtU(double *x,double *y)
{
	computeKgUNonLinear(x,y);// y = K*x 
}
/**********************************************************************/
int	AFElasticity::linearSolveCGforLargeDeformation(double *f, double *u)
{
	// Solves Ku = f
	// Assumes K and f have been defined, and Dirichlet conditions have been set
	// Also assumes CG iterations and tolerances have been defined.
	// Solution is returned in h_sol
	int  niter;
	Timer timer;

	if (h_KENLTemplates == NULL) {
		printf("AFElasticity: Ke Templates not initialized\n");
		return 0;
	}

#ifdef COMPILE_GPUCODE
	/*if (usingGPU) {
		niter = performCGOnGPU();
		computeDeformationMax();
		return niter;
	}*/
#endif
	double rho,alpha,beta,rho_new,normr,normr0;
	double *r,*p,*a;

	if (h_hexmesh_verbose) printf("AFElasticity: Performing CG on CPU\n");
	if (h_hexmesh_verbose) printf("AFElasticity: Elem-based Non-Linear KU\n");
	
	if (h_hexmesh_verbose) printf("AFElasticity: Allocating temporary memory\n");
	// use temporary CPU variables
	r = (double *) malloc(3*h_nNodes*sizeof(double));
	p = (double *) malloc(3*h_nNodes*sizeof(double));
	a = (double *) malloc(3*h_nNodes*sizeof(double));

	safeFree(h_cgNorm);
	h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
	memset(h_cgNorm,0,h_CGMaxIter*sizeof(double));

	memset(u, 0, 3*h_nNodes*sizeof(double));// Reset to zero vector
	// Assume u  has been initialized (see CPU initialization)
	timer.start();
	
	computeNonLinearKtU(u,a); // a = K*sol
	if (h_hexmesh_verbose) printf("AFElasticity: Time taken for one Kx: %f (msec) \n",timer.getElapsedTimeInMilliSec());
	if (h_hexmesh_verbose) printf("AFElasticity: Norm of sol = %e \n",Norm(u));
	if (h_hexmesh_verbose) printf("AFElasticity: normf = %e, norm(sol) =%e \n",Norm(f),Norm(u));
	zeaxpy(-1,1,r,a,f);// r = -a + f
	normr = sqrt(dotprod(r,r));
	normr0 = normr;

	copy(r,a);// Set a = r
	copy(a,p);// Set p = a 
	rho = dotprod(a,r);
	niter = 1;     // Init counter for number of iterations
	if (h_hexmesh_verbose) {
		if (h_nodeBasedAssembly) printf("AFElasticity: Starting node-based CG on CPU\n");
		else printf("AFElasticity: Starting elem-based CG on CPU\n");
		printf("AFElasticity: normr0 = %e, rho =%e \n",normr0,rho);
	}
	while ((niter < 10) || (normr > h_CGRelativeResErr*normr0))  {//% Test break condition
		h_cgNorm[niter-1] = log10(normr/normr0);
		computeNonLinearKtU(p,a);// a = K*p;
		beta = dotprod(a,p);
		alpha = rho/beta;
		axpy(alpha, p,u);//u = u + alpha*p;
		axpy(-alpha,a,r);//r = r - alpha*a;
		normr = sqrt(dotprod(r,r));
		if (niter == 1) normr0 = normr;//update for consistent scaling convergence
		copy(r,a);// Set a = r
		rho_new =  dotprod(a,r);//rho = a'*r
		zeaxpy(1,(rho_new/rho),p,a,p);//p = a + rho_new/rho * p;
		rho = rho_new;
		niter++;
		h_nCGItersActual = niter;

		if (normr >1000*normr0)  {  
			//printf("AFElasticity: Diverging at %d iter, resErr = %e \n",niter,normr);
			niter = -1;
			break;
		}	
		if ((h_hexmesh_verbose) && (niter%100 ==0)) printf(" %d: %e \n",niter,normr/normr0);
		//if (niter%100 ==0) printf("\n niter = %d: %e \n",niter,normr/normr0);
		if (niter == h_CGMaxIter)  {  
			//printf("AFElasticity: Did not converge in %d iter, resErr = %e \n",niter,normr/normr0);
			niter = -1;
			break;
		}
	}
	if (h_hexmesh_verbose)  printf("AFElasticity: CG on CPU; nIter = %d, resErr = %e \n",niter,normr/normr0);
	//printf("\nAFElasticity: CG on CPU; nIter = %d, resErr = %e \n",niter,normr);
	free(a);
	free(p);
	free(r);
	return niter;
}
/**********************************************************************/
void AFElasticity::computeNonLinearKtElement(int elem, double K[24][24])
{
	// Kg for an isotropic element of arbitrary aspect ratio
	int i, j, gP, node1, node2; 
	double t1, detJ;
	double N_x[8], N_y[8], N_z[8], s[6];
	
	int elemID = h_templateID[elem];

	TCLMatrix F(3,3,0.0);
	TCLMatrix Ft(3,3,0.0);
	TCLMatrix Kt(3,3,0.0);
	TCLMatrix K_pair(3,3,0.0);
	
	for (i = 0; i < 24; i++)
		for (j = 0; j < 24; j++)
			K[i][j] = 0;

	if (h_gP == 1) {
		gP = 0;
		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
				F[i][j] = deformGrad[(9*gP + 3*i + j)*h_nElems + elem];
		Ft = ~F;

		for (i = 0; i < 6; i++)
			s[i] = pk2Stress [(6*gP + i)*h_nElems + elem];

		detJ = computeShapeGradient(elem, 0, 0, 0, N_x, N_y, N_z);

		for (node1 = 0; node1 < 8; node1++)
			for (node2 = 0; node2 < 8; node2++) {
				t1 = 8*detJ*(N_x[node1]*(s[0]*N_x[node2] + s[3]*N_y[node2] + s[4]*N_z[node2])
					+ N_y[node1]*(s[3]*N_x[node2] + s[1]*N_y[node2] + s[5]*N_z[node2])
					+ N_z[node1]*(s[4]*N_x[node2] + s[5]*N_y[node2] + s[2]*N_z[node2]));
				
				for (i = 0; i < 3; i++)
					for (j = 0; j < 3; j++)
						Kt[i][j] = h_KENLTemplates[(576*gP + 24*(3*node1 + i) + 3*node2 + j)*(h_nTemplateElements) + elemID];
				K_pair = F*Kt*Ft;

				for (i = 0; i < 3; i++)
					K_pair[i][i] += t1;

				for (i = 0; i < 3; i++)
					for (j = 0; j < 3; j++)
						K[3*node1 + i][3*node2 + j] = K_pair[i][j];
			}
	}

	else {
		double a = 1.0/sqrt(3.0);
		double xiValues[8] = {-a,-a,a,a,-a,-a,a,a};
		double etaValues[8] = {-a,a,-a,a,-a,a,-a,a};
		double muValues[8] = {-a,-a,-a,-a,a,a,a,a};
		for (gP = 0; gP < 8; gP++) {
			for (i = 0; i < 3; i++)
				for (j = 0; j < 3; j++)
					F[i][j] = deformGrad[(9*gP + 3*i + j)*h_nElems + elem];
			Ft = ~F;

			for (i = 0; i < 6; i++)
				s[i] = pk2Stress [(6*gP + i)*h_nElems + elem];

			detJ = computeShapeGradient(elem, xiValues[gP], etaValues[gP], muValues[gP], N_x, N_y, N_z);
			
			for (node1 = 0; node1 < 8; node1++)
				for (node2 = 0; node2 < 8; node2++) {
					t1 = detJ*(N_x[node1]*(s[0]*N_x[node2] + s[3]*N_y[node2] + s[4]*N_z[node2])
						+ N_y[node1]*(s[3]*N_x[node2] + s[1]*N_y[node2] + s[5]*N_z[node2])
						+ N_z[node1]*(s[4]*N_x[node2] + s[5]*N_y[node2] + s[2]*N_z[node2]));
				
					for (i = 0; i < 3; i++)
						for (j = 0; j < 3; j++)
							Kt[i][j] = h_KENLTemplates[(576*gP + 24*(3*node1 + i) + 3*node2 + j)*(h_nTemplateElements) + elemID];
					K_pair = F*Kt*Ft;

					for (i = 0; i < 3; i++)
						K_pair[i][i] += t1;

					for (i = 0; i < 3; i++)
						for (j = 0; j < 3; j++)
							K[3*node1 + i][3*node2 + j] += K_pair[i][j];
				}
		}
	}

}
/**********************************************************************/
void AFElasticity::tangentElementWKW(int elem, int add) 
{
	int freeEncoded, u1, u2, v1, v2, w1, w2, g2, g1, n1, n2;
	int node_i, node_j;
	double K_uu, K_uv, K_uw, K_vu, K_vv, K_vw, K_wu, K_wv, K_ww,x1,y1,z1,x2,y2,z2;
	// Compute element stiffness matrix
	double K_temp[24][24];
	computeNonLinearKtElement(elem,K_temp);

	for (node_i = 0; node_i < 8; node_i++) {
		//node of a single voxel // d-o-f of node (u, v, w)
		n1 = h_elems[node_i*h_nElems + elem]; 
		
		g1 = ws_nodeGroupNumber[n1]; // Group number on mesh

		freeEncoded = h_free[n1];
		w1 = freeEncoded/4;
		v1 = (freeEncoded - w1*4)/2;
		u1 = (freeEncoded - w1*4 -v1*2);
			
		if ( (u1 == 0) && (v1 == 0) && (w1 == 0)) 
			continue;
		x1 = (h_x[n1] - h_CGPatch[3*g1 + 0]);
		y1 = (h_y[n1] - h_CGPatch[3*g1 + 1]);
		z1 = (h_z[n1] - h_CGPatch[3*g1 + 2]);

		for (node_j = 0; node_j < 8; node_j++) { 
			n2 = h_elems[node_j*h_nElems + elem]; 
			/*
			if (!doesNodeExist(n2)){
				printf("Error in Kreduction\n");
				continue;//should not come here
			}
			*/
			g2 = ws_nodeGroupNumber[n2];
								
			freeEncoded = h_free[n2];
			w2 = freeEncoded/4;
			v2 = (freeEncoded - w2*4)/2;
			u2 = (freeEncoded - w2*4 -v2*2);

			if ( (u2 == 0) && (v2 == 0) && (w2 == 0)) 
				continue;

			x2 = (h_x[n2] - h_CGPatch[3*g2 + 0]);
			y2 = (h_y[n2] - h_CGPatch[3*g2 + 1]);
			z2 = (h_z[n2] - h_CGPatch[3*g2 + 2]);

			// For 9 cases use K_val and index to populate WKW using only required node number and nodegNumber information
			K_uu = K_temp[3*node_i + 0][3*node_j + 0]*u1*u2*add;
			K_uv = K_temp[3*node_i + 0][3*node_j + 1]*u1*v2*add;
			K_uw = K_temp[3*node_i + 0][3*node_j + 2]*u1*w2*add;
			K_vu = K_temp[3*node_i + 1][3*node_j + 0]*v1*u2*add;
			K_vv = K_temp[3*node_i + 1][3*node_j + 1]*v1*v2*add;
			K_vw = K_temp[3*node_i + 1][3*node_j + 2]*v1*w2*add;
			K_wu = K_temp[3*node_i + 2][3*node_j + 0]*w1*u2*add; 
			K_wv = K_temp[3*node_i + 2][3*node_j + 1]*w1*v2*add; 
			K_ww = K_temp[3*node_i + 2][3*node_j + 2]*w1*w2*add;

			if (ws_deflationType == DEFLATION_RIGID){
				// For 9 cases use K_val and index to populate WKW using only required node number and nodeGroupNumber information
				tangentKTildeWKW(6*g1,6*g2) += K_uu;
				tangentKTildeWKW(6*g1,6*g2 + 1) += K_uv;
				tangentKTildeWKW(6*g1,6*g2 + 2) += K_uw;//[2,0)
				tangentKTildeWKW(6*g1,6*g2 + 3) += (K_uw*y2 - K_uv*z2);
				tangentKTildeWKW(6*g1,6*g2 + 4) += (K_uu*z2 - K_uw*x2);
				tangentKTildeWKW(6*g1,6*g2 + 5) += (K_uv*x2 - K_uu*y2);
			
				tangentKTildeWKW(6*g1 + 1,6*g2) += K_vu;
				tangentKTildeWKW(6*g1 + 1,6*g2 + 1) += K_vv;
				tangentKTildeWKW(6*g1 + 1,6*g2 + 2) += K_vw;//[1,2)
				tangentKTildeWKW(6*g1 + 1,6*g2 + 3) += (K_vw*y2 - K_vv*z2);
				tangentKTildeWKW(6*g1 + 1,6*g2 + 4) += (K_vu*z2 - K_vw*x2);//[1,4)
				tangentKTildeWKW(6*g1 + 1,6*g2 + 5) += (K_vv*x2 - K_vu*y2);

				tangentKTildeWKW(6*g1 + 2,6*g2) += K_wu;//[0,2)
				tangentKTildeWKW(6*g1 + 2,6*g2 + 1) += K_wv;//[2,1)
				tangentKTildeWKW(6*g1 + 2,6*g2 + 2) += K_ww;
				tangentKTildeWKW(6*g1 + 2,6*g2 + 3) += (K_ww*y2 - K_wv*z2);
				tangentKTildeWKW(6*g1 + 2,6*g2 + 4) += (K_wu*z2 - K_ww*x2);
				tangentKTildeWKW(6*g1 + 2,6*g2 + 5) += (K_wv*x2 - K_wu*y2);//[2,5)
		
				tangentKTildeWKW(6*g1 + 3,6*g2) += (K_wu*y1 - K_vu*z1);//[3,0)
				tangentKTildeWKW(6*g1 + 3,6*g2 + 1) += (K_wv*y1 - K_vv*z1);
				tangentKTildeWKW(6*g1 + 3,6*g2 + 2) += (K_ww*y1 - K_vw*z1);
				tangentKTildeWKW(6*g1 + 3,6*g2 + 3) += (y2*(K_ww*y1 - K_vw*z1) - z2*(K_wv*y1 - K_vv*z1));
				tangentKTildeWKW(6*g1 + 3,6*g2 + 4) += (z2*(K_wu*y1 - K_vu*z1) - x2*(K_ww*y1 - K_vw*z1));
				tangentKTildeWKW(6*g1 + 3,6*g2 + 5) += (x2*(K_wv*y1 - K_vv*z1) - y2*(K_wu*y1 - K_vu*z1));
			
			
				tangentKTildeWKW(6*g1 + 4,6*g2) += (K_uu*z1 - K_wu*x1);
				tangentKTildeWKW(6*g1 + 4,6*g2 + 1) += (K_uv*z1 - K_wv*x1);//[4,1)
				tangentKTildeWKW(6*g1 + 4,6*g2 + 2) += (K_uw*z1 - K_ww*x1);
				tangentKTildeWKW(6*g1 + 4,6*g2 + 3) += (z2*(K_wv*x1 - K_uv*z1) - y2*(K_ww*x1 - K_uw*z1));
				tangentKTildeWKW(6*g1 + 4,6*g2 + 4) += (x2*(K_ww*x1 - K_uw*z1) - z2*(K_wu*x1 - K_uu*z1));
				tangentKTildeWKW(6*g1 + 4,6*g2 + 5) += (y2*(K_wu*x1 - K_uu*z1) - x2*(K_wv*x1 - K_uv*z1));

				tangentKTildeWKW(6*g1 + 5,6*g2) += (K_vu*x1 - K_uu*y1);
				tangentKTildeWKW(6*g1 + 5,6*g2 + 1) += (K_vv*x1 - K_uv*y1);
				tangentKTildeWKW(6*g1 + 5,6*g2 + 2) += (K_vw*x1 - K_uw*y1);//[5,2)
				tangentKTildeWKW(6*g1 + 5,6*g2 + 3) += (y2*(K_vw*x1 - K_uw*y1) - z2*(K_vv*x1 - K_uv*y1));
				tangentKTildeWKW(6*g1 + 5,6*g2 + 4) += (z2*(K_vu*x1 - K_uu*y1) - x2*(K_vw*x1 - K_uw*y1));
				tangentKTildeWKW(6*g1 + 5,6*g2 + 5) += (x2*(K_vv*x1 - K_uv*y1) - y2*(K_vu*x1 - K_uu*y1));
			}
			else
			{
				tangentKTildeWKW(9*g1,9*g2) += K_uu;
				tangentKTildeWKW(9*g1,9*g2 + 1) += K_uv;
				tangentKTildeWKW(9*g1,9*g2 + 2) += K_uw;
				tangentKTildeWKW(9*g1,9*g2 + 3) += (K_uw*y2 - K_uv*z2);
				tangentKTildeWKW(9*g1,9*g2 + 4) += (K_uu*z2 - K_uw*x2);
				tangentKTildeWKW(9*g1,9*g2 + 5) += (K_uv*x2 - K_uu*y2);
				tangentKTildeWKW(9*g1,9*g2 + 6) += (K_uw*x2*x2/2 - K_uu*x2*z2);
				tangentKTildeWKW(9*g1,9*g2 + 7) += (K_uw*y2*y2/2 - K_uv*y2*z2);
				tangentKTildeWKW(9*g1,9*g2 + 8) += (K_uw*x2*y2 - K_uu*y2*z2 - K_uv*x2*z2);

				tangentKTildeWKW(9*g1 + 1,9*g2) += K_vu;
				tangentKTildeWKW(9*g1 + 1,9*g2 + 1) += K_vv;
				tangentKTildeWKW(9*g1 + 1,9*g2 + 2) += K_vw;
				tangentKTildeWKW(9*g1 + 1,9*g2 + 3) += (K_vw*y2 - K_vv*z2);
				tangentKTildeWKW(9*g1 + 1,9*g2 + 4) += (K_vu*z2 - K_vw*x2);
				tangentKTildeWKW(9*g1 + 1,9*g2 + 5) += (K_vv*x2 - K_vu*y2);
				tangentKTildeWKW(9*g1 + 1,9*g2 + 6) += (K_vw*x2*x2/2 - K_vu*x2*z2);
				tangentKTildeWKW(9*g1 + 1,9*g2 + 7) += (K_vw*y2*y2/2 - K_vv*y2*z2);
				tangentKTildeWKW(9*g1 + 1,9*g2 + 8) += (K_vw*x2*y2 - K_vv*x2*z2 - K_vu*y2*z2);

				tangentKTildeWKW(9*g1 + 2,9*g2) += K_wu;
				tangentKTildeWKW(9*g1 + 2,9*g2 + 1) += K_wv;
				tangentKTildeWKW(9*g1 + 2,9*g2 + 2) += K_ww;
				tangentKTildeWKW(9*g1 + 2,9*g2 + 3) += (K_ww*y2 - K_wv*z2);
				tangentKTildeWKW(9*g1 + 2,9*g2 + 4) += (K_wu*z2 - K_ww*x2);
				tangentKTildeWKW(9*g1 + 2,9*g2 + 5) += (K_wv*x2 - K_wu*y2);
				tangentKTildeWKW(9*g1 + 2,9*g2 + 6) += (K_ww*x2*x2/2 - K_wu*x2*z2);
				tangentKTildeWKW(9*g1 + 2,9*g2 + 7) += (K_ww*y2*y2/2 - K_wv*y2*z2);
				tangentKTildeWKW(9*g1 + 2,9*g2 + 8) += (K_ww*x2*y2 - K_wv*x2*z2 - K_wu*y2*z2);
								
				tangentKTildeWKW(9*g1 + 3,9*g2) += (K_wu*y1 - K_vu*z1);//[3,0)
				tangentKTildeWKW(9*g1 + 3,9*g2 + 1) += (K_wv*y1 - K_vv*z1);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 2) += (K_ww*y1 - K_vw*z1);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 3) += (K_vv*z1*z2 + K_ww*y1*y2 - K_vw*z1*y2 - K_wv*y1*z2);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 4) += (K_wu*y1*z2 + K_vw*z1*x2 - K_vu*z1*z2 - K_ww*y1*x2);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 5) += (K_vu*z1*y2 + K_wv*y1*x2 - K_vv*z1*x2 - K_wu*y1*y2);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 6) += (K_vu*z1*x2*z2 + K_ww*y1*x2*x2/2 - K_vw*z1*x2*x2/2 - K_wu*y1*x2*z2);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 7) += (K_vv*z1*y2*z2 + K_ww*y1*y2*y2/2 - K_vw*z1*y2*y2/2 - K_wv*y1*y2*z2);
				tangentKTildeWKW(9*g1 + 3,9*g2 + 8) += (K_vu*z1*y2*z2 + K_vv*z1*x2*z2 + K_ww*y1*x2*y2 - K_vw*z1*x2*y2 - K_wu*y1*y2*z2 - K_wv*y1*x2*z2);
								
				tangentKTildeWKW(9*g1 + 4,9*g2) += (K_uu*z1 - K_wu*x1);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 1) += (K_uv*z1 - K_wv*x1);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 2) += (K_uw*z1 - K_ww*x1);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 3) += (K_uw*z1*y2 + K_wv*x1*z2 - K_uv*z1*z2 - K_ww*x1*y2);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 4) += (K_uu*z1*z2 + K_ww*x1*x2 - K_wu*x1*z2 - K_uw*z1*x2);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 5) += (K_uv*z1*x2 + K_wu*x1*y2 - K_uu*z1*y2 - K_wv*x1*x2);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 6) += (K_wu*x1*x2*z2 + K_uw*z1*x2*x2/2 - K_ww*x1*x2*x2/2 - K_uu*z1*x2*z2);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 7) += (K_uw*z1*y2*y2/2 + K_wv*x1*y2*z2 - K_uv*z1*y2*z2 - K_ww*x1*y2*y2/2);
				tangentKTildeWKW(9*g1 + 4,9*g2 + 8) += (K_uw*z1*x2*y2 + K_wu*x1*y2*z2 + K_wv*x1*x2*z2 - K_uu*z1*y2*z2 - K_uv*z1*x2*z2 - K_ww*x1*x2*y2);

				tangentKTildeWKW(9*g1 + 5,9*g2) += (K_vu*x1 - K_uu*y1);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 1) += (K_vv*x1 - K_uv*y1);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 2) += (K_vw*x1 - K_uw*y1);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 3) += (K_vw*x1*y2 + K_uv*y1*z2 - K_vv*x1*z2 - K_uw*y1*y2);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 4) += (K_uw*y1*x2 + K_vu*x1*z2 - K_uu*y1*z2 - K_vw*x1*x2);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 5) += (K_uu*y1*y2 + K_vv*x1*x2 - K_vu*x1*y2 - K_uv*y1*x2);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 6) += (K_uu*y1*x2*z2 + K_vw*x1*x2*x2/2 - K_vu*x1*x2*z2 - K_uw*y1*x2*x2/2);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 7) += (K_uv*y1*y2*z2 + K_vw*x1*y2*y2/2 - K_uw*y1*y2*y2/2 - K_vv*x1*y2*z2);
				tangentKTildeWKW(9*g1 + 5,9*g2 + 8) += (K_uu*y1*y2*z2 + K_uv*y1*x2*z2 + K_vw*x1*x2*y2 - K_uw*y1*x2*y2 - K_vu*x1*y2*z2 - K_vv*x1*x2*z2);
				
				tangentKTildeWKW(9*g1 + 6,9*g2) += (K_wu*x1*x1/2 - K_uu*x1*z1);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 1) += (K_wv*x1*x1/2 - K_uv*x1*z1);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 2) += (K_ww*x1*x1/2 - K_uw*x1*z1);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 3) += (K_uv*x1*z1*z2 + K_ww*x1*x1/2*y2 - K_uw*x1*z1*y2 - K_wv*x1*x1/2*z2);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 4) += (K_uw*x1*z1*x2 + K_wu*x1*x1/2*z2 - K_uu*x1*z1*z2 - K_ww*x1*x1/2*x2);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 5) += (K_uu*x1*z1*y2 + K_wv*x1*x1/2*x2 - K_uv*x1*z1*x2 - K_wu*x1*x1/2*y2);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 6) += (K_ww*x1*x1/2*x2*x2/2 + K_uu*x1*z1*x2*z2 - K_wu*x1*x1/2*x2*z2 - K_uw*x1*z1*x2*x2/2);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 7) += (K_ww*x1*x1/2*y2*y2/2 + K_uv*x1*z1*y2*z2 - K_wv*x1*x1/2*y2*z2 - K_uw*x1*z1*y2*y2/2);
				tangentKTildeWKW(9*g1 + 6,9*g2 + 8) += (K_uu*x1*z1*y2*z2 + K_uv*x1*z1*x2*z2 + K_ww*x1*x1/2*x2*y2 - K_wv*x1*x1/2*x2*z2 - K_wu*x1*x1/2*y2*z2 - K_uw*x1*z1*x2*y2);					

				tangentKTildeWKW(9*g1 + 7,9*g2) += (K_wu*y1*y1/2 - K_vu*y1*z1);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 1) += (K_wv*y1*y1/2 - K_vv*y1*z1);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 2) += (K_ww*y1*y1/2 - K_vw*y1*z1);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 3) += (K_ww*y1*y1/2*y2 + K_vv*y1*z1*z2 - K_wv*y1*y1/2*z2 - K_vw*y1*z1*y2);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 4) += (K_vw*y1*z1*x2 + K_wu*y1*y1/2*z2 - K_vu*y1*z1*z2 - K_ww*y1*y1/2*x2);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 5) += (K_wv*y1*y1/2*x2 + K_vu*y1*z1*y2 - K_wu*y1*y1/2*y2 - K_vv*y1*z1*x2);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 6) += (K_ww*y1*y1/2*x2*x2/2 + K_vu*y1*z1*x2*z2 - K_wu*y1*y1/2*x2*z2 - K_vw*y1*z1*x2*x2/2);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 7) += (K_ww*y1*y1/2*y2*y2/2 + K_vv*y1*z1*y2*z2 - K_wv*y1*y1/2*y2*z2 - K_vw*y1*z1*y2*y2/2);
				tangentKTildeWKW(9*g1 + 7,9*g2 + 8) += (K_vu*y1*z1*y2*z2 + K_vv*y1*z1*x2*z2 + K_ww*y1*y1/2*x2*y2 - K_vw*y1*z1*x2*y2 - K_wu*y1*y1/2*y2*z2 - K_wv*y1*y1/2*x2*z2);

				tangentKTildeWKW(9*g1 + 8,9*g2) += (K_wu*x1*y1 - K_vu*x1*z1 - K_uu*y1*z1);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 1) += (K_wv*x1*y1 - K_vv*x1*z1 - K_uv*y1*z1);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 2) += (K_ww*x1*y1 - K_vw*x1*z1 - K_uw*y1*z1);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 3) += (K_ww*x1*y1*y2 + K_uv*y1*z1*z2 + K_vv*x1*z1*z2 - K_wv*x1*y1*z2 - K_uw*y1*z1*y2 - K_vw*x1*z1*y2);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 4) += (K_vw*x1*z1*x2 + K_uw*y1*z1*x2 + K_wu*x1*y1*z2 - K_uu*y1*z1*z2 - K_vu*x1*z1*z2 - K_ww*x1*y1*x2);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 5) += (K_wv*x1*y1*x2 + K_uu*y1*z1*y2 + K_vu*x1*z1*y2 - K_wu*x1*y1*y2 - K_uv*y1*z1*x2 - K_vv*x1*z1*x2);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 6) += (K_uu*y1*z1*x2*z2 + K_vu*x1*z1*x2*z2 + K_ww*x1*y1*x2*x2/2 - K_wu*x1*y1*x2*z2 - K_uw*y1*z1*x2*x2/2 - K_vw*x1*z1*x2*x2/2);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 7) += (K_ww*x1*y1*y2*y2/2 + K_vv*x1*z1*y2*z2 + K_uv*y1*z1*y2*z2 - K_wv*x1*y1*y2*z2 - K_uw*y1*z1*y2*y2/2 - K_vw*x1*z1*y2*y2/2);
				tangentKTildeWKW(9*g1 + 8,9*g2 + 8) += (K_uu*y1*z1*y2*z2 + K_vu*x1*z1*y2*z2 + K_uv*y1*z1*x2*z2 + K_vv*x1*z1*x2*z2 + K_ww*x1*y1*x2*y2
													- K_wu*x1*y1*y2*z2 - K_wv*x1*y1*x2*z2 - K_uw*y1*z1*x2*y2 - K_vw*x1*z1*x2*y2);
			}
		}
	}
}
/**********************************************************************/
int AFElasticity::updateTangentialKtilde()// Neumann condition assumed, diriclet applied after K*u
{
	Timer timer;
	int g;

	timer.start();
	if (h_hexmesh_verbose) printf("Deflation: Updating sparseWKW for NonLinear \n");
	if (ws_deflationType == DEFLATION_RIGID) {
		g = 6*ws_nGroups;
		tangentKreductionRBM();
	}
	else {
		g = 9*ws_nGroups;
		tangentKreductionThinPlate();
	}

	if (h_hexmesh_verbose) printf("Deflation: LLT sparseWKW \n");
	KTildeWKW_LLT = tangentKTildeWKW.llt();// Decompose once, used in solving
	if (KTildeWKW_LLT.info() != 0) {
		if (h_hexmesh_verbose) printf("\nDeflation: LLT decomposition failed with %d groups \n",ws_nGroups);
		return -1;
	}
	KTildeWKW_Lower = KTildeWKW_LLT.matrixL();//store the Lower 
	if (h_hexmesh_verbose) printf("Deflation: SparseWKW constructed\n");
	return 1;
}
/**********************************************************************/
void AFElasticity::tangentKreductionRBM()
{

	tangentKTildeWKW.resize(6*ws_nGroups,6*ws_nGroups);// Initialize WKW (Here 'KTildeWKW')
	tangentKTildeWKW.setZero(6*ws_nGroups,6*ws_nGroups);

	for ( int k = 0; k < h_nElems; k++) {
		if (h_pseudoDensity[k] <0.5) continue; // element exists but is void (example: during topology optimization)
		tangentElementWKW(k, 1);
	}

	int rowStart,rowEnd;
	double maxValue = tangentKTildeWKW.maxCoeff();
	for (int g = 0; g <ws_nGroups;g++) {
		if (ws_groupCount[g] == 0){
			//printf("\n Empty group \n");
			rowStart = 6*g;
			rowEnd = 6*g + 6;
			for (int i = rowStart; i < rowEnd;i++) {
				//for (int j = rowStart; j < rowEnd;j++)
					//tangentKTildeWKW(i,j) = 0.0;
				tangentKTildeWKW(i,i) = maxValue;
			}
		}
	}
	
}
/********************************************************************/
void AFElasticity::tangentKreductionThinPlate()
{
	tangentKTildeWKW.resize(9*ws_nGroups,9*ws_nGroups);// Initialize WKW (Here 'KTildeWKW')
	tangentKTildeWKW.setZero(9*ws_nGroups,9*ws_nGroups);

	for (int k = 0; k < h_nElems; k++) {
		if (h_pseudoDensity[k] <0.5)// element exists but is void (example: during topology optimization)
				continue;
		tangentElementWKW(k,1);	
	}
	int rowStart,rowEnd;
	double maxValue = tangentKTildeWKW.maxCoeff();
	for (int g = 0; g <ws_nGroups;g++) {
		if (ws_groupCount[g] == 0){
			rowStart = 9*g;
			rowEnd = 9*g + 9;
			for (int i = rowStart; i < rowEnd;i++) {
				for (int j = rowStart; j < rowEnd;j++)
					tangentKTildeWKW(i,j) = 0.0;
				tangentKTildeWKW(i,i) = maxValue;
			}
		}
	}
}
/*********************************************************************/
int	AFElasticity::linearSolveCGWforLargeDeformation(double *f, double *u)
{
	// Solves Ku = f
	// Assumes K and f have been defined, and Dirichlet conditions have been set
	// Also assumes CG iterations and tolerances have been defined.
	// Solution is returned in u
	int  niter;
	Timer timer;
	// Update Ktilde for Tangential Stiffness
	timer.start();
	#ifdef COMPILE_GPUCODE
	/*if (usingGPU) {
		niter = performCGWOnGPU();
		return niter;
	}*/
	#endif

	double rho,alpha,beta,rho_new,normr,normr0;
	double *r,*p,*a,*d, *t;

	if (h_hexmesh_verbose)printf("Deflatiom: Performing deflated CG \n");

	// use temporary CPU variables
	r = (double *) malloc(3*h_nNodes*sizeof(double));
	p = (double *) malloc(3*h_nNodes*sizeof(double));
	a = (double *) malloc(3*h_nNodes*sizeof(double));
	d = (double *) malloc(3*h_nNodes*sizeof(double));
	t = (double *) malloc(3*h_nNodes*sizeof(double));

	safeFree(h_cgNorm);
	h_cgNorm = (double *) malloc(h_CGMaxIter*sizeof(double));
	
	VectorXd mu,result;
	if (ws_deflationType == DEFLATION_RIGID)	mu.resize(6*ws_nGroups);
	else mu.resize(9*ws_nGroups);
	// Performing conjugate gradient here with residual start
	computeNonLinearKtU(u,a);// a = K*sol
	zeaxpy(-1,1,r,a,f);// r = -a + f
	normr0 = dotprod(r,r);
	getWorkSpaceVector(r, mu); // mu = W'*r
	//Compute [W'*K*W]*result = W'*r = mu (from above)
	result = KTildeWKW_LLT.solve(mu);
	getSolutionSpaceVector(result, t);// t = W*result
	enforceDirichlet0(t);
	axpy(1,t,u);

	computeNonLinearKtU(u,a);// a = K*sol
	zeaxpy(-1,1,r,a,f);// r = -a + f
	
	normr = 10*normr0;
	//********************************************************	
	computeNonLinearKtU(r,t);
	getWorkSpaceVector(t, mu);	
	result = KTildeWKW_LLT.solve(mu);
	getSolutionSpaceVector(result, t);
	enforceDirichlet0(t);
	zeaxpy(-1,1,p,t,r);// p = r - t
	//********************************************
	rho = dotprod(r,r);
	niter = 1;     // Init counter for number of iterations

	while (normr > h_CGRelativeResErr*normr0)   {
		h_cgNorm[niter-1] = log10(normr/normr0);
		computeNonLinearKtU(p,a);// a = K*p;
		beta = dotprod(a,p);
		alpha = rho/beta;
		axpy(alpha, p,u);//u = u + alpha*p;
		if (niter%1000 == 0)	{ // restart CG with recomputing residual
			if (h_hexmesh_verbose) printf("Deflation: Restarting CG \n");
			computeNonLinearKtU(u,a);// a = K*sol
			zeaxpy(-1,1,r,a,f);// r = -a + f
		}
		else
			axpy(-alpha,a,r);//r = r - alpha*a;

		rho_new =  dotprod(r,r);//rho = a'*r
		normr = (rho_new);
		computeNonLinearKtU(r,t);
		getWorkSpaceVector(t, mu);		// mu = W'*t
		result = KTildeWKW_LLT.solve(mu);
		getSolutionSpaceVector(result, t); // t = W*result
		enforceDirichlet0(t);
		zeaxpy(1,(rho_new/rho),p,r,p);//p = a + rho_new/rho * p;
		axpy(-1,t,p);//p = -t + p
		rho = rho_new;
		niter++;
		h_nCGItersActual = niter;
		if ((h_hexmesh_verbose) && (niter%10 ==0)) printf("Deflation: %d: %e \n",niter,normr/normr0);
		
		if (niter >= ws_DCGMaxIter)  { 
			if (h_hexmesh_verbose) printf("Deflation: DCG did not converge in %d iter, resErr = %e \n",niter,normr/normr0);
			niter = -1;
			break;
		}
	}
	if (h_hexmesh_verbose) printf("Deflation: Time taken for iterations = %0.3f (s)\n",timer.getElapsedTimeInSec());timer.start();
	if (h_hexmesh_verbose)  printf("Deflation: CG on CPU; nIter = %d, resErr = %e \n",niter,normr/normr0);
	//printf("\nDeflation: CG on CPU; nIter = %d, resErr = %e \n",niter,normr/normr0);
	free(a);
	free(p);
	free(r);
	free(d);
	free(t); 
	return niter;
}
/**********************************************************************/
int	AFElasticity::linearSolveforLargeDeformation(double *f, double *u)
{
	int nIter;

	if (h_workSpaceInitialized)
		nIter = linearSolveCGWforLargeDeformation(f, u);
	else 
		nIter = linearSolveCGforLargeDeformation(f, u); 

	if (nIter < 0) 
		return FAILURE;

	if (_isnan(computeCompliance()))// likely to be disconnected body
		return FAILURE; 

	h_nCGIterations[h_nLinearSolves] = nIter;
	h_nLinearSolves++;

	return nIter;
}
/**********************************************************************/
int	AFElasticity::solveLargeDeformationElastic(bool recycleSolution)
{
	int nIter = 0;
	//enableHexmeshVerbose();
	h_gP = 2;
	if (h_hexmesh_verbose) 	printf("AFElasticity: Solving Large Deformation Non-Linear System\n");

	if ((h_voxelgrid) &&  (h_nodeBasedAssembly) && (h_KeNode == NULL)) {
		computeKeAndMeNodal(1);
	}

	nIter += solveElasticityStatic(dummyCallBack);
	
	// Initialize temporary variables: Internal Force, Residual Force, del_Displacement
	double *delU, *delF, *f_int, *fStep;
	delU = (double *) malloc(3*h_nNodes*sizeof(double));
	delF = (double *) malloc(3*h_nNodes*sizeof(double));
	f_int = (double *) malloc(3*h_nNodes*sizeof(double));
	fStep = (double *) malloc(3*h_nNodes*sizeof(double));

	if (h_gP == 1) {
		pk2Stress = (double *) malloc(6*h_nElems*sizeof(double));
		deformGrad = (double *) malloc(9*h_nElems*sizeof(double));
	}
	else {
		pk2Stress = (double *) malloc(48*h_nElems*sizeof(double));
		deformGrad = (double *) malloc(72*h_nElems*sizeof(double));
	}
	
	// Step size for large deformation problem
	int nLoadSteps, i, flag, innerIter;
	double normr, normr0;
	
	// Set initial valudes for delU and fStep
	nLoadSteps = 5;
	memset(delU, 0, 3*h_nNodes*sizeof(double));// Reset delta U
	copy(h_f,fStep);
	scale(1.0/nLoadSteps, fStep);
	scale(1.0/nLoadSteps, h_sol);

	// Compute Non Linear Ke Template
	computeNonLinearKeTemplates();

	normr0 = Norm(fStep);
	
	for (i = 0; i < nLoadSteps; i++) {
		normr = normr0;
		innerIter = 0;
		if (i > 0) {
			flag = linearSolveforLargeDeformation(fStep, delU);// Set initial guess for h_sol for next step
			if (flag < 0){ 
				if (h_hexmesh_verbose) printf("Deflation: Linear Solve did not converge in %d iter, resErr = %e \n",nIter,normr/normr0);
				nIter = -1;
				break;
			}
			nIter += flag;
			axpy(1, delU, h_sol);//u = delU + u
		}
		updateDeformGradAndStress();
		if (h_workSpaceInitialized)
			if (updateTangentialKtilde() == -1) {
				ReleaseWorkSpace();
				if (h_hexmesh_verbose) 	printf("\nAFElasticity: Deflation unsuccessful for Non-Linear\n");
			}
		while(normr > normr0/1000) {
			updateInternalForce(f_int);

			zeaxpy(-1,(i+1.0),delF,f_int,fStep);// delF = -f_int + loadScale*h_f
			normr = Norm(delF);
			
			if(innerIter%10 == 0) {
				printf("\nForce Step: %d, Count: %d, Relative Norm: %e\n",i, innerIter, (normr/normr0));
				enableHexmeshVerbose();
				computeDeformationMax();
				disableHexmeshVerbose();
			}
			
			flag = linearSolveforLargeDeformation(delF, delU);
			if (flag < 0){ 
				if (h_hexmesh_verbose) printf("Deflation: Linear Solve did not converge in %d iter, resErr = %e \n",nIter,normr/normr0);
				nIter = -1;
				break;
			}
			nIter += flag;
			axpy(1, delU, h_sol);//u = delU + u
			updateDeformGradAndStress();
			if (h_workSpaceInitialized)
				if (updateTangentialKtilde() == -1) {
					ReleaseWorkSpace();
					if (h_hexmesh_verbose) 	printf("\nAFElasticity: Deflation unsuccessful for Non-Linear\n");
				}
			innerIter++;
			if (innerIter >= 500)  { 
				if (h_hexmesh_verbose) printf("Deflation: Newton method did not converge for Force Step %d in %d iter, resErr = %e \n",i+1, innerIter, normr/normr0);
				nIter = -1;
				break;
			}
		}
		if (nIter < 0) 
			break;
	}
		
	

	safeFree(delU); safeFree(delF); safeFree(f_int); safeFree(fStep); 
	safeFree(deformGrad); deformGrad = NULL;
	safeFree(pk2Stress); pk2Stress = NULL;
	safeFree(h_KENLTemplates); h_KENLTemplates = NULL;
	return nIter;
}