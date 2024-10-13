#include <stdio.h>
#include <stdlib.h>
#include <ElasticPolynomial.h>


#define MAX(a,b) (a>b?a:b)
#define MIN(a,b) (a<b?a:b)
/**********************************************************************/
ElasticPolynomial::ElasticPolynomial(int dummy)
{ 
	ep_order = 0;
	ep_nConstraints = 0;
	ep_nCoefficients = 0;
	ep_verbose = false;
}
/**********************************************************************/
void ElasticPolynomial::initialize(int order, double E, double nu,double modelScale)
{
	// uses polynomial representation of elastic fields to solve for simple boundary value problems
	// Reference: General representations of Polynomial Elastic Fields
	// Wang, Xu and Zhao, Journal of Applied Mechanics, 2012, Vol 79

	int nCoefficients;
	ep_order = 0;
	ep_nConstraints = 0;
	ep_nCoefficients = 0;

	if ((order <=0) || (order > 5)){
		printf("ElasticPolynomial: order must be 1,2, 3, 4 or 5 \n");
		return;
	}
	ep_order = order;

	ep_E = E;// Young's modulus
	ep_nu = nu;// Poisson ratio
	ep_L = modelScale;

	ep_lambda = E*nu/((1+nu)*(1-2*nu));
	ep_mu = E/(2*(1+nu));

	ep_solved = false;
	
	ep_dirichletConstraintApplied = false;
	ep_XYZMin[0] = 1e32;ep_XYZMin[1] = 1e32;ep_XYZMin[2] = 1e32;
	ep_XYZMax[0] = -1e32;ep_XYZMax[1] = -1e32;ep_XYZMax[2] = -1e32;
	ep_origin[0] =0;ep_origin[1] =0;ep_origin[2] =0;

	nCoefficients = 0;
	for (int order = 0; order <= ep_order; order++)
		nCoefficients += 3*(2*order+1);

	ep_nCoefficients = nCoefficients;
	if (ep_verbose) printf("ElasticPolynomial: nCoefficients = %d \n",nCoefficients);

	ep_u.resize(ep_nCoefficients);//displacements
	ep_v.resize(ep_nCoefficients);
	ep_w.resize(ep_nCoefficients);


	ep_s11.resize(ep_nCoefficients);// stress
	ep_s12.resize(ep_nCoefficients);
	ep_s13.resize(ep_nCoefficients);
	ep_s22.resize(ep_nCoefficients);
	ep_s23.resize(ep_nCoefficients);
	ep_s33.resize(ep_nCoefficients);

	for (int i =0; i < ep_nCoefficients; i++) {
		ep_u(i) = 0;
		ep_v(i) = 0;
		ep_w(i) = 0;
		ep_s11(i) = 0;
		ep_s22(i) = 0;
		ep_s33(i) = 0;
		ep_s12(i) = 0;
		ep_s13(i) = 0;
		ep_s23(i) = 0;
	}
	ep_A.resize(ep_nCoefficients,ep_nCoefficients);
	ep_rhs.resize(ep_nCoefficients);
}
/**********************************************************************/
void	ElasticPolynomial::setOrigin(double origin[3])
{
	for (int i =0; i < 3; i++)
		ep_origin[i] = origin[i];
}
/**********************************************************************/
void ElasticPolynomial::enableVerbose(void)
{
	ep_verbose = true;
}
/**********************************************************************/
void ElasticPolynomial::disableVerbose(void)
{
	ep_verbose = false;
}
/**********************************************************************/
void ElasticPolynomial::computeDisplacementCoefficients(double pt[3])
{
	//Generated via Matlab
	double x,y,z,nu,L;

	x = pt[0] - ep_origin[0];
	y = pt[1] - ep_origin[1];
	z = pt[2] - ep_origin[2];
	nu = ep_nu;
	L = ep_L;

	for (int i =0; i < ep_nCoefficients; i++) {
		ep_u(i) = 0;
		ep_v(i) = 0;
		ep_w(i) = 0;
	}
	int order;

	order = ep_order;

	if (order == 0){
		ep_u[0] = nu*-4.0+3.0;
		ep_v[1] = nu*-4.0+3.0;
		ep_w[2] = nu*-4.0+3.0;
	}

	else if (order == 1) {
		ep_u[0] = nu*-4.0+3.0;
		ep_u[3] = (x*(nu*2.0-1.0)*-2.0)/L;
		ep_u[4] = -y/L;
		ep_u[5] = -z/L;
		ep_u[6] = -(y*(nu*4.0-3.0))/L;
		ep_u[9] = -(z*(nu*4.0-3.0))/L;

		ep_v[1] = nu*-4.0+3.0;
		ep_v[4] = -(x*(nu*4.0-3.0))/L;
		ep_v[6] = -x/L;
		ep_v[7] = (y*(nu*2.0-1.0)*-2.0)/L;
		ep_v[8] = -z/L;
		ep_v[10] = -(z*(nu*4.0-3.0))/L;

		ep_w[2] = nu*-4.0+3.0;
		ep_w[5] = -(x*(nu*4.0-3.0))/L;
		ep_w[8] = -(y*(nu*4.0-3.0))/L;
		ep_w[9] = -x/L;
		ep_w[10] = -y/L;
		ep_w[11] = (z*(nu*2.0-1.0)*-2.0)/L;
	}
	else if (order == 2) {
		ep_u[0] = nu*-4.0+3.0;
		ep_u[3] = (x*(nu*2.0-1.0)*-2.0)/L;
		ep_u[4] = -y/L;
		ep_u[5] = -z/L;
		ep_u[6] = -(y*(nu*4.0-3.0))/L;
		ep_u[9] = -(z*(nu*4.0-3.0))/L;
		ep_u[12] = -1.0/(L*L)*(nu*(x*x)*4.0-nu*(y*y)*4.0-x*x+(y*y)*3.0);
		ep_u[13] = -1.0/(L*L)*(nu*4.0-3.0)*(y*y-z*z);
		ep_u[14] = -1.0/(L*L)*y*z*(nu*4.0-3.0);
		ep_u[15] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
		ep_u[16] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_u[18] = 1.0/(L*L)*x*y*2.0;
		ep_u[19] = -1.0/(L*L)*y*z;
		ep_u[20] = -1.0/(L*L)*(y*y);
		ep_u[22] = 1.0/(L*L)*x*z*2.0;
		ep_u[23] = 1.0/(L*L)*x*z*-2.0;
		ep_u[24] = -1.0/(L*L)*y*z;
		ep_u[26] = -1.0/(L*L)*(z*z);
	
		ep_v[1] = nu*-4.0+3.0;
		ep_v[4] = -(x*(nu*4.0-3.0))/L;
		ep_v[6] = -x/L;
		ep_v[7] = (y*(nu*2.0-1.0)*-2.0)/L;
		ep_v[8] = -z/L;
		ep_v[10] = -(z*(nu*4.0-3.0))/L;
		ep_v[12] = 1.0/(L*L)*x*y*2.0;
		ep_v[13] = 1.0/(L*L)*x*y*-2.0;
		ep_v[14] = -1.0/(L*L)*x*z;
		ep_v[16] = -1.0/(L*L)*(x*x);
		ep_v[17] = -1.0/(L*L)*(nu*(y*y)*4.0-nu*(z*z)*4.0-y*y+(z*z)*3.0);
		ep_v[18] = 1.0/(L*L)*(nu*4.0-3.0)*(x*x-z*z);
		ep_v[19] = -1.0/(L*L)*x*z*(nu*4.0-3.0);
		ep_v[20] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_v[21] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_v[23] = 1.0/(L*L)*y*z*2.0;
		ep_v[24] = -1.0/(L*L)*x*z;
		ep_v[25] = -1.0/(L*L)*(z*z);
	
		ep_w[2] = nu*-4.0+3.0;
		ep_w[5] = -(x*(nu*4.0-3.0))/L;
		ep_w[8] = -(y*(nu*4.0-3.0))/L;
		ep_w[9] = -x/L;
		ep_w[10] = -y/L;
		ep_w[11] = (z*(nu*2.0-1.0)*-2.0)/L;
		ep_w[13] = 1.0/(L*L)*x*z*2.0;
		ep_w[14] = -1.0/(L*L)*x*y;
		ep_w[15] = -1.0/(L*L)*(x*x);
		ep_w[17] = 1.0/(L*L)*y*z*2.0;
		ep_w[18] = 1.0/(L*L)*y*z*-2.0;
		ep_w[19] = -1.0/(L*L)*x*y;
		ep_w[21] = -1.0/(L*L)*(y*y);
		ep_w[22] = 1.0/(L*L)*(nu*(x*x)*4.0-nu*(z*z)*4.0-(x*x)*3.0+z*z);
		ep_w[23] = -1.0/(L*L)*(nu*4.0-3.0)*(x*x-y*y);
		ep_w[24] = -1.0/(L*L)*x*y*(nu*4.0-3.0);
		ep_w[25] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_w[26] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
	}
	else if (order == 3) {
		ep_u[0] = nu*-4.0+3.0;
		ep_u[3] = (x*(nu*2.0-1.0)*-2.0)/L;
		ep_u[4] = -y/L;
		ep_u[5] = -z/L;
		ep_u[6] = -(y*(nu*4.0-3.0))/L;
		ep_u[9] = -(z*(nu*4.0-3.0))/L;
		ep_u[12] = -1.0/(L*L)*(nu*(x*x)*4.0-nu*(y*y)*4.0-x*x+(y*y)*3.0);
		ep_u[13] = -1.0/(L*L)*(nu*4.0-3.0)*(y*y-z*z);
		ep_u[14] = -1.0/(L*L)*y*z*(nu*4.0-3.0);
		ep_u[15] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
		ep_u[16] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_u[18] = 1.0/(L*L)*x*y*2.0;
		ep_u[19] = -1.0/(L*L)*y*z;
		ep_u[20] = -1.0/(L*L)*(y*y);
		ep_u[22] = 1.0/(L*L)*x*z*2.0;
		ep_u[23] = 1.0/(L*L)*x*z*-2.0;
		ep_u[24] = -1.0/(L*L)*y*z;
		ep_u[26] = -1.0/(L*L)*(z*z);
		ep_u[27] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*-2.0;
		ep_u[28] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
		ep_u[29] = 1.0/(L*L*L)*y*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0);
		ep_u[30] = -1.0/(L*L*L)*y*(nu*4.0-3.0)*(y*y-(z*z)*3.0);
		ep_u[31] = 1.0/(L*L*L)*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0);
		ep_u[32] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((y*y)*3.0-z*z);
		ep_u[33] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
		ep_u[35] = 1.0/(L*L*L)*x*(y*y)*6.0;
		ep_u[37] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_u[38] = 1.0/(L*L*L)*y*(x*x-y*y)*-3.0;
		ep_u[39] = 1.0/(L*L*L)*y*(x*x-z*z)*-3.0;
		ep_u[40] = -1.0/(L*L*L)*(y*y)*z;
		ep_u[41] = 1.0/(L*L*L)*x*(z*z)*6.0;
		ep_u[43] = 1.0/(L*L*L)*z*(x*x-z*z)*-3.0;
		ep_u[44] = 1.0/(L*L*L)*z*(x*x-y*y)*-3.0;
		ep_u[46] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_u[47] = -1.0/(L*L*L)*y*(z*z);

		ep_v[1] = nu*-4.0+3.0;
		ep_v[4] = -(x*(nu*4.0-3.0))/L;
		ep_v[6] = -x/L;
		ep_v[7] = (y*(nu*2.0-1.0)*-2.0)/L;
		ep_v[8] = -z/L;
		ep_v[10] = -(z*(nu*4.0-3.0))/L;
		ep_v[12] = 1.0/(L*L)*x*y*2.0;
		ep_v[13] = 1.0/(L*L)*x*y*-2.0;
		ep_v[14] = -1.0/(L*L)*x*z;
		ep_v[16] = -1.0/(L*L)*(x*x);
		ep_v[17] = -1.0/(L*L)*(nu*(y*y)*4.0-nu*(z*z)*4.0-y*y+(z*z)*3.0);
		ep_v[18] = 1.0/(L*L)*(nu*4.0-3.0)*(x*x-z*z);
		ep_v[19] = -1.0/(L*L)*x*z*(nu*4.0-3.0);
		ep_v[20] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_v[21] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_v[23] = 1.0/(L*L)*y*z*2.0;
		ep_v[24] = -1.0/(L*L)*x*z;
		ep_v[25] = -1.0/(L*L)*(z*z);
		ep_v[27] = 1.0/(L*L*L)*(x*x)*y*6.0;
		ep_v[29] = 1.0/(L*L*L)*x*(x*x-y*y)*3.0;
		ep_v[30] = 1.0/(L*L*L)*x*(y*y-z*z)*-3.0;
		ep_v[32] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_v[33] = -1.0/(L*L*L)*(x*x)*z;
		ep_v[34] = 1.0/(L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
		ep_v[35] = 1.0/(L*L*L)*y*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*-2.0;
		ep_v[36] = 1.0/(L*L*L)*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0);
		ep_v[37] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((x*x)*3.0-z*z);
		ep_v[38] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0);
		ep_v[39] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(z*z)*3.0);
		ep_v[40] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
		ep_v[42] = 1.0/(L*L*L)*y*(z*z)*6.0;
		ep_v[44] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_v[45] = 1.0/(L*L*L)*z*(y*y-z*z)*-3.0;
		ep_v[46] = 1.0/(L*L*L)*z*(x*x-y*y)*3.0;
		ep_v[47] = -1.0/(L*L*L)*x*(z*z);

		ep_w[2] = nu*-4.0+3.0;
		ep_w[5] = -(x*(nu*4.0-3.0))/L;
		ep_w[8] = -(y*(nu*4.0-3.0))/L;
		ep_w[9] = -x/L;
		ep_w[10] = -y/L;
		ep_w[11] = (z*(nu*2.0-1.0)*-2.0)/L;
		ep_w[13] = 1.0/(L*L)*x*z*2.0;
		ep_w[14] = -1.0/(L*L)*x*y;
		ep_w[15] = -1.0/(L*L)*(x*x);
		ep_w[17] = 1.0/(L*L)*y*z*2.0;
		ep_w[18] = 1.0/(L*L)*y*z*-2.0;
		ep_w[19] = -1.0/(L*L)*x*y;
		ep_w[21] = -1.0/(L*L)*(y*y);
		ep_w[22] = 1.0/(L*L)*(nu*(x*x)*4.0-nu*(z*z)*4.0-(x*x)*3.0+z*z);
		ep_w[23] = -1.0/(L*L)*(nu*4.0-3.0)*(x*x-y*y);
		ep_w[24] = -1.0/(L*L)*x*y*(nu*4.0-3.0);
		ep_w[25] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_w[26] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
		ep_w[28] = 1.0/(L*L*L)*(x*x)*z*6.0;
		ep_w[30] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_w[31] = 1.0/(L*L*L)*x*(x*x-z*z)*3.0;
		ep_w[32] = 1.0/(L*L*L)*x*(y*y-z*z)*3.0;
		ep_w[33] = -1.0/(L*L*L)*(x*x)*y;
		ep_w[34] = 1.0/(L*L*L)*(y*y)*z*6.0;
		ep_w[36] = 1.0/(L*L*L)*y*(y*y-z*z)*3.0;
		ep_w[37] = 1.0/(L*L*L)*y*(x*x-z*z)*3.0;
		ep_w[39] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_w[40] = -1.0/(L*L*L)*x*(y*y);
		ep_w[41] = 1.0/(L*L*L)*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*-2.0;
		ep_w[42] = 1.0/(L*L*L)*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*-2.0;
		ep_w[43] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0);
		ep_w[44] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(y*y)*3.0);
		ep_w[45] = -1.0/(L*L*L)*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0);
		ep_w[46] = 1.0/(L*L*L)*y*(nu*4.0-3.0)*((x*x)*3.0-y*y);
		ep_w[47] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
	}
	else if (order == 4){

		ep_u[0] = nu*-4.0+3.0;
		ep_u[3] = (x*(nu*2.0-1.0)*-2.0)/L;
		ep_u[4] = -y/L;
		ep_u[5] = -z/L;
		ep_u[6] = -(y*(nu*4.0-3.0))/L;
		ep_u[9] = -(z*(nu*4.0-3.0))/L;
		ep_u[12] = -1.0/(L*L)*(nu*(x*x)*4.0-nu*(y*y)*4.0-x*x+(y*y)*3.0);
		ep_u[13] = -1.0/(L*L)*(nu*4.0-3.0)*(y*y-z*z);
		ep_u[14] = -1.0/(L*L)*y*z*(nu*4.0-3.0);
		ep_u[15] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
		ep_u[16] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_u[18] = 1.0/(L*L)*x*y*2.0;
		ep_u[19] = -1.0/(L*L)*y*z;
		ep_u[20] = -1.0/(L*L)*(y*y);
		ep_u[22] = 1.0/(L*L)*x*z*2.0;
		ep_u[23] = 1.0/(L*L)*x*z*-2.0;
		ep_u[24] = -1.0/(L*L)*y*z;
		ep_u[26] = -1.0/(L*L)*(z*z);
		ep_u[27] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*-2.0;
		ep_u[28] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
		ep_u[29] = 1.0/(L*L*L)*y*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0);
		ep_u[30] = -1.0/(L*L*L)*y*(nu*4.0-3.0)*(y*y-(z*z)*3.0);
		ep_u[31] = 1.0/(L*L*L)*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0);
		ep_u[32] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((y*y)*3.0-z*z);
		ep_u[33] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
		ep_u[35] = 1.0/(L*L*L)*x*(y*y)*6.0;
		ep_u[37] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_u[38] = -1.0/(L*L*L)*y*((x*x)*3.0-(y*y)*3.0);
		ep_u[39] = -1.0/(L*L*L)*y*((x*x)*3.0-(z*z)*3.0);
		ep_u[40] = -1.0/(L*L*L)*(y*y)*z;
		ep_u[41] = 1.0/(L*L*L)*x*(z*z)*6.0;
		ep_u[43] = -1.0/(L*L*L)*z*((x*x)*3.0-(z*z)*3.0);
		ep_u[44] = -1.0/(L*L*L)*z*((x*x)*3.0-(y*y)*3.0);
		ep_u[46] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_u[47] = -1.0/(L*L*L)*y*(z*z);
		ep_u[48] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0;
		ep_u[49] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0;
		ep_u[50] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0;
		ep_u[51] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0;
		ep_u[52] = -1.0/(L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0+x*x*x*x-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*2.4E1);
		ep_u[53] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0);
		ep_u[54] = -1.0/(L*L*L*L)*((x*x)*(y*y)*3.0+(x*x)*(z*z)*3.0-(y*y)*(z*z)*9.0+nu*(x*x*x*x)*4.0+x*x*x*x-nu*(x*x)*(y*y)*1.2E1-nu*(x*x)*(z*z)*1.2E1+nu*(y*y)*(z*z)*1.2E1);
		ep_u[55] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0);
		ep_u[56] = -1.0/(L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0+x*x*x*x-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*2.4E1);
		ep_u[58] = -1.0/(L*L*L*L)*y*(y*(z*z)*3.0-y*y*y);
		ep_u[59] = 1.0/(L*L*L*L)*x*(y*y)*z*-6.0;
		ep_u[60] = -1.0/(L*L*L*L)*y*((x*x)*y*3.0-y*y*y);
		ep_u[62] = 1.0/(L*L*L*L)*y*((y*y)*z*3.0-z*z*z);
		ep_u[63] = 1.0/(L*L*L*L)*x*y*(y*y-z*z)*6.0;
		ep_u[64] = 1.0/(L*L*L*L)*y*z*(x*x-y*y)*-3.0;
		ep_u[65] = 1.0/(L*L*L*L)*x*y*(x*x-(y*y)*3.0)*-4.0;
		ep_u[66] = -1.0/(L*L*L*L)*z*((x*x)*z*3.0-z*z*z);
		ep_u[67] = 1.0/(L*L*L*L)*x*y*(z*z)*-6.0;
		ep_u[68] = -1.0/(L*L*L*L)*z*((y*y)*z*3.0-z*z*z);
		ep_u[70] = 1.0/(L*L*L*L)*x*z*(x*x-(z*z)*3.0)*-4.0;
		ep_u[71] = 1.0/(L*L*L*L)*y*z*(x*x-z*z)*-3.0;
		ep_u[72] = 1.0/(L*L*L*L)*x*z*(y*y-z*z)*-6.0;
		ep_u[73] = 1.0/(L*L*L*L)*z*(y*(z*z)*3.0-y*y*y);

		ep_v[1] = nu*-4.0+3.0;
		ep_v[4] = -(x*(nu*4.0-3.0))/L;
		ep_v[6] = -x/L;
		ep_v[7] = (y*(nu*2.0-1.0)*-2.0)/L;
		ep_v[8] = -z/L;
		ep_v[10] = -(z*(nu*4.0-3.0))/L;
		ep_v[12] = 1.0/(L*L)*x*y*2.0;
		ep_v[13] = 1.0/(L*L)*x*y*-2.0;
		ep_v[14] = -1.0/(L*L)*x*z;
		ep_v[16] = -1.0/(L*L)*(x*x);
		ep_v[17] = -1.0/(L*L)*(nu*(y*y)*4.0-nu*(z*z)*4.0-y*y+(z*z)*3.0);
		ep_v[18] = 1.0/(L*L)*(nu*4.0-3.0)*(x*x-z*z);
		ep_v[19] = -1.0/(L*L)*x*z*(nu*4.0-3.0);
		ep_v[20] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
		ep_v[21] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_v[23] = 1.0/(L*L)*y*z*2.0;
		ep_v[24] = -1.0/(L*L)*x*z;
		ep_v[25] = -1.0/(L*L)*(z*z);
		ep_v[27] = 1.0/(L*L*L)*(x*x)*y*6.0;
		ep_v[29] = 1.0/(L*L*L)*x*((x*x)*3.0-(y*y)*3.0);
		ep_v[30] = -1.0/(L*L*L)*x*((y*y)*3.0-(z*z)*3.0);
		ep_v[32] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_v[33] = -1.0/(L*L*L)*(x*x)*z;
		ep_v[34] = 1.0/(L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
		ep_v[35] = 1.0/(L*L*L)*y*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*-2.0;
		ep_v[36] = 1.0/(L*L*L)*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0);
		ep_v[37] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((x*x)*3.0-z*z);
		ep_v[38] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0);
		ep_v[39] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(z*z)*3.0);
		ep_v[40] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
		ep_v[42] = 1.0/(L*L*L)*y*(z*z)*6.0;
		ep_v[44] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_v[45] = -1.0/(L*L*L)*z*((y*y)*3.0-(z*z)*3.0);
		ep_v[46] = 1.0/(L*L*L)*z*((x*x)*3.0-(y*y)*3.0);
		ep_v[47] = -1.0/(L*L*L)*x*(z*z);
		ep_v[48] = -1.0/(L*L*L*L)*x*(x*(y*y)*3.0-x*x*x);
		ep_v[49] = 1.0/(L*L*L*L)*(x*x)*y*z*-6.0;
		ep_v[50] = -1.0/(L*L*L*L)*x*(x*(z*z)*3.0-x*x*x);
		ep_v[52] = 1.0/(L*L*L*L)*x*((x*x)*y*1.2E1-(y*y*y)*4.0);
		ep_v[53] = 1.0/(L*L*L*L)*x*z*(x*x-y*y)*3.0;
		ep_v[54] = 1.0/(L*L*L*L)*x*y*(x*x-z*z)*6.0;
		ep_v[55] = 1.0/(L*L*L*L)*x*((x*x)*z*3.0-z*z*z);
		ep_v[57] = 1.0/(L*L*L*L)*y*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0;
		ep_v[58] = 1.0/(L*L*L*L)*x*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0;
		ep_v[59] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0;
		ep_v[60] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0;
		ep_v[61] = -1.0/(L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0+y*y*y*y-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*2.4E1);
		ep_v[62] = 1.0/(L*L*L*L)*x*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0);
		ep_v[63] = -1.0/(L*L*L*L)*((x*x)*(y*y)*3.0-(x*x)*(z*z)*9.0+(y*y)*(z*z)*3.0+nu*(y*y*y*y)*4.0+y*y*y*y-nu*(x*x)*(y*y)*1.2E1+nu*(x*x)*(z*z)*1.2E1-nu*(y*y)*(z*z)*1.2E1);
		ep_v[64] = -1.0/(L*L*L*L)*x*z*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0);
		ep_v[65] = -1.0/(L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0-(x*x*x*x)*3.0+y*y*y*y-nu*(x*x)*(y*y)*2.4E1);
		ep_v[67] = -1.0/(L*L*L*L)*z*((x*x)*z*3.0-z*z*z);
		ep_v[68] = 1.0/(L*L*L*L)*x*y*(z*z)*-6.0;
		ep_v[69] = -1.0/(L*L*L*L)*z*((y*y)*z*3.0-z*z*z);
		ep_v[71] = 1.0/(L*L*L*L)*z*(x*(z*z)*3.0-x*x*x);
		ep_v[72] = 1.0/(L*L*L*L)*y*z*(x*x-z*z)*-6.0;
		ep_v[73] = 1.0/(L*L*L*L)*x*z*(y*y-z*z)*-3.0;
		ep_v[74] = 1.0/(L*L*L*L)*y*z*(y*y-(z*z)*3.0)*-4.0;

		ep_w[2] = nu*-4.0+3.0;
		ep_w[5] = -(x*(nu*4.0-3.0))/L;
		ep_w[8] = -(y*(nu*4.0-3.0))/L;
		ep_w[9] = -x/L;
		ep_w[10]= -y/L;
		ep_w[11] = (z*(nu*2.0-1.0)*-2.0)/L;
		ep_w[13] = 1.0/(L*L)*x*z*2.0;
		ep_w[14] = -1.0/(L*L)*x*y;
		ep_w[15] = -1.0/(L*L)*(x*x);
		ep_w[17] = 1.0/(L*L)*y*z*2.0;
		ep_w[18] = 1.0/(L*L)*y*z*-2.0;
		ep_w[19] = -1.0/(L*L)*x*y;
		ep_w[21] = -1.0/(L*L)*(y*y);
		ep_w[22] = 1.0/(L*L)*(nu*(x*x)*4.0-nu*(z*z)*4.0-(x*x)*3.0+z*z);
		ep_w[23] = -1.0/(L*L)*(nu*4.0-3.0)*(x*x-y*y);
		ep_w[24] = -1.0/(L*L)*x*y*(nu*4.0-3.0);
		ep_w[25] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
		ep_w[26] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
		ep_w[28] = 1.0/(L*L*L)*(x*x)*z*6.0;
		ep_w[30] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_w[31] = 1.0/(L*L*L)*x*((x*x)*3.0-(z*z)*3.0);
		ep_w[32] = 1.0/(L*L*L)*x*((y*y)*3.0-(z*z)*3.0);
		ep_w[33] = -1.0/(L*L*L)*(x*x)*y;
		ep_w[34] = 1.0/(L*L*L)*(y*y)*z*6.0;
		ep_w[36] = 1.0/(L*L*L)*y*((y*y)*3.0-(z*z)*3.0);
		ep_w[37] = 1.0/(L*L*L)*y*((x*x)*3.0-(z*z)*3.0);
		ep_w[39] = 1.0/(L*L*L)*x*y*z*6.0;
		ep_w[40] = -1.0/(L*L*L)*x*(y*y);
		ep_w[41] = 1.0/(L*L*L)*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*-2.0;
		ep_w[42] = 1.0/(L*L*L)*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*-2.0;
		ep_w[43] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0);
		ep_w[44] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(y*y)*3.0);
		ep_w[45] = -1.0/(L*L*L)*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0);
		ep_w[46] = 1.0/(L*L*L)*y*(nu*4.0-3.0)*((x*x)*3.0-y*y);
		ep_w[47] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
		ep_w[49] = -1.0/(L*L*L*L)*x*(x*(y*y)*3.0-x*x*x);
		ep_w[50] = 1.0/(L*L*L*L)*(x*x)*y*z*-6.0;
		ep_w[51] = -1.0/(L*L*L*L)*x*(x*(z*z)*3.0-x*x*x);
		ep_w[53] = 1.0/(L*L*L*L)*x*((x*x)*y*3.0-y*y*y);
		ep_w[54] = 1.0/(L*L*L*L)*x*z*(x*x-y*y)*6.0;
		ep_w[55] = 1.0/(L*L*L*L)*x*y*(x*x-z*z)*3.0;
		ep_w[56] = 1.0/(L*L*L*L)*x*((x*x)*z*1.2E1-(z*z*z)*4.0);
		ep_w[57] = -1.0/(L*L*L*L)*y*(y*(z*z)*3.0-y*y*y);
		ep_w[58] = 1.0/(L*L*L*L)*x*(y*y)*z*-6.0;
		ep_w[59] = -1.0/(L*L*L*L)*y*((x*x)*y*3.0-y*y*y);
		ep_w[61] = 1.0/(L*L*L*L)*y*((y*y)*z*1.2E1-(z*z*z)*4.0);
		ep_w[62] = 1.0/(L*L*L*L)*x*y*(y*y-z*z)*3.0;
		ep_w[63] = 1.0/(L*L*L*L)*y*z*(x*x-y*y)*-6.0;
		ep_w[64] = 1.0/(L*L*L*L)*y*(x*(y*y)*3.0-x*x*x);
		ep_w[66] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0;
		ep_w[67] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0;
		ep_w[68] = 1.0/(L*L*L*L)*x*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0;
		ep_w[69] = 1.0/(L*L*L*L)*y*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0;
		ep_w[70] = -1.0/(L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0-(x*x*x*x)*3.0+z*z*z*z-nu*(x*x)*(z*z)*2.4E1);
		ep_w[71] = -1.0/(L*L*L*L)*x*y*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0);
		ep_w[72] = -1.0/(L*L*L*L)*((x*x)*(y*y)*-9.0+(x*x)*(z*z)*3.0+(y*y)*(z*z)*3.0+nu*(z*z*z*z)*4.0+z*z*z*z+nu*(x*x)*(y*y)*1.2E1-nu*(x*x)*(z*z)*1.2E1-nu*(y*y)*(z*z)*1.2E1);
		ep_w[73] = -1.0/(L*L*L*L)*x*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0);
		ep_w[74] = -1.0/(L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0-(y*y*y*y)*3.0+z*z*z*z-nu*(y*y)*(z*z)*2.4E1);
	}

	else if (order == 5){
  ep_u[0] = nu*-4.0+3.0;
  ep_u[3] = (x*(nu*2.0-1.0)*-2.0)/L;
  ep_u[4] = -y/L;
  ep_u[5] = -z/L;
  ep_u[6] = -(y*(nu*4.0-3.0))/L;
  ep_u[9] = -(z*(nu*4.0-3.0))/L;
  ep_u[12] = -1.0/(L*L)*(nu*(x*x)*4.0-nu*(y*y)*4.0-x*x+(y*y)*3.0);
  ep_u[13] = -1.0/(L*L)*(nu*4.0-3.0)*(y*y-z*z);
  ep_u[14] = -1.0/(L*L)*y*z*(nu*4.0-3.0);
  ep_u[15] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
  ep_u[16] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
  ep_u[18] = 1.0/(L*L)*x*y*2.0;
  ep_u[19] = -1.0/(L*L)*y*z;
  ep_u[20] = -1.0/(L*L)*(y*y);
  ep_u[22] = 1.0/(L*L)*x*z*2.0;
  ep_u[23] = 1.0/(L*L)*x*z*-2.0;
  ep_u[24] = -1.0/(L*L)*y*z;
  ep_u[26] = -1.0/(L*L)*(z*z);
  ep_u[27] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*-2.0;
  ep_u[28] = 1.0/(L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
  ep_u[29] = 1.0/(L*L*L)*y*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0);
  ep_u[30] = -1.0/(L*L*L)*y*(nu*4.0-3.0)*(y*y-(z*z)*3.0);
  ep_u[31] = 1.0/(L*L*L)*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0);
  ep_u[32] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((y*y)*3.0-z*z);
  ep_u[33] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
  ep_u[35] = 1.0/(L*L*L)*x*(y*y)*6.0;
  ep_u[37] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_u[38] = -1.0/(L*L*L)*y*((x*x)*3.0-(y*y)*3.0);
  ep_u[39] = -1.0/(L*L*L)*y*((x*x)*3.0-(z*z)*3.0);
  ep_u[40] = -1.0/(L*L*L)*(y*y)*z;
  ep_u[41] = 1.0/(L*L*L)*x*(z*z)*6.0;
  ep_u[43] = -1.0/(L*L*L)*z*((x*x)*3.0-(z*z)*3.0);
  ep_u[44] = -1.0/(L*L*L)*z*((x*x)*3.0-(y*y)*3.0);
  ep_u[46] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_u[47] = -1.0/(L*L*L)*y*(z*z);
  ep_u[48] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0;
  ep_u[49] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0;
  ep_u[50] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0;
  ep_u[51] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0;
  ep_u[52] = -1.0/(L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0+x*x*x*x-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*2.4E1);
  ep_u[53] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0);
  ep_u[54] = -1.0/(L*L*L*L)*((x*x)*(y*y)*3.0+(x*x)*(z*z)*3.0-(y*y)*(z*z)*9.0+nu*(x*x*x*x)*4.0+x*x*x*x-nu*(x*x)*(y*y)*1.2E1-nu*(x*x)*(z*z)*1.2E1+nu*(y*y)*(z*z)*1.2E1);
  ep_u[55] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0);
  ep_u[56] = -1.0/(L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0+x*x*x*x-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*2.4E1);
  ep_u[58] = -1.0/(L*L*L*L)*y*(y*(z*z)*3.0-y*y*y);
  ep_u[59] = 1.0/(L*L*L*L)*x*(y*y)*z*-6.0;
  ep_u[60] = -1.0/(L*L*L*L)*y*((x*x)*y*3.0-y*y*y);
  ep_u[62] = 1.0/(L*L*L*L)*y*((y*y)*z*3.0-z*z*z);
  ep_u[63] = 1.0/(L*L*L*L)*x*y*(y*y-z*z)*6.0;
  ep_u[64] = 1.0/(L*L*L*L)*y*z*(x*x-y*y)*-3.0;
  ep_u[65] = 1.0/(L*L*L*L)*x*y*(x*x-(y*y)*3.0)*-4.0;
  ep_u[66] = -1.0/(L*L*L*L)*z*((x*x)*z*3.0-z*z*z);
  ep_u[67] = 1.0/(L*L*L*L)*x*y*(z*z)*-6.0;
  ep_u[68] = -1.0/(L*L*L*L)*z*((y*y)*z*3.0-z*z*z);
  ep_u[70] = 1.0/(L*L*L*L)*x*z*(x*x-(z*z)*3.0)*-4.0;
  ep_u[71] = 1.0/(L*L*L*L)*y*z*(x*x-z*z)*-3.0;
  ep_u[72] = 1.0/(L*L*L*L)*x*z*(y*y-z*z)*-6.0;
  ep_u[73] = 1.0/(L*L*L*L)*z*(y*(z*z)*3.0-y*y*y);
  ep_u[75] = 1.0/(L*L*L*L*L)*x*(nu*(x*x*x*x)*2.0+nu*(y*y*y*y)*1.0E1+x*x*x*x-(y*y*y*y)*5.0-nu*(x*x)*(y*y)*2.0E1)*2.0;
  ep_u[76] = 1.0/(L*L*L*L*L)*x*(nu*(x*x*x*x)*2.0+nu*(z*z*z*z)*1.0E1+x*x*x*x-(z*z*z*z)*5.0-nu*(x*x)*(z*z)*2.0E1)*2.0;
  ep_u[77] = 1.0/(L*L*L*L*L)*y*((x*x)*(y*y)*1.0E1+nu*(x*x*x*x)*2.0E1+nu*(y*y*y*y)*4.0+(x*x*x*x)*5.0-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*4.0E1);
  ep_u[78] = 1.0/(L*L*L*L*L)*z*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0+x*x*x*x-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*2.4E1);
  ep_u[79] = 1.0/(L*L*L*L*L)*y*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0+x*x*x*x-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*2.4E1);
  ep_u[80] = 1.0/(L*L*L*L*L)*z*((x*x)*(z*z)*1.0E1+nu*(x*x*x*x)*2.0E1+nu*(z*z*z*z)*4.0+(x*x*x*x)*5.0-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*4.0E1);
  ep_u[81] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0;
  ep_u[82] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0;
  ep_u[83] = 1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*((y*y)*(z*z)*-1.0E1+y*y*y*y+(z*z*z*z)*5.0);
  ep_u[84] = 1.0/(L*L*L*L*L)*x*(nu*2.0-1.0)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*2.0;
  ep_u[85] = 1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*((y*y)*(z*z)*-1.0E1+(y*y*y*y)*5.0+z*z*z*z);
  ep_u[87] = 1.0/(L*L*L*L*L)*x*(y*y)*(x*x-y*y)*2.0E1;
  ep_u[89] = 1.0/(L*L*L*L*L)*y*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z);
  ep_u[90] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-(y*y)*3.0)*4.0;
  ep_u[91] = 1.0/(L*L*L*L*L)*y*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0);
  ep_u[92] = 1.0/(L*L*L*L*L)*(y*y)*z*(y*y-z*z);
  ep_u[93] = -1.0/(L*L*L*L*L)*(y*y)*z*((x*x)*3.0-y*y);
  ep_u[94] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-z*z)*2.0E1;
  ep_u[95] = 1.0/(L*L*L*L*L)*x*(y*y)*(x*x-(z*z)*3.0)*4.0;
  ep_u[96] = 1.0/(L*L*L*L*L)*y*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0);
  ep_u[97] = 1.0/(L*L*L*L*L)*x*(z*z)*(x*x-z*z)*2.0E1;
  ep_u[99] = 1.0/(L*L*L*L*L)*z*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0);
  ep_u[100] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-(z*z)*3.0)*4.0;
  ep_u[101] = 1.0/(L*L*L*L*L)*z*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z);
  ep_u[103] = -1.0/(L*L*L*L*L)*y*(z*z)*((x*x)*3.0-z*z);
  ep_u[104] = -1.0/(L*L*L*L*L)*y*(z*z)*(y*y-z*z);
  ep_u[105] = 1.0/(L*L*L*L*L)*z*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0);
  ep_u[106] = 1.0/(L*L*L*L*L)*x*(z*z)*(x*x-(y*y)*3.0)*4.0;
  ep_u[107] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-y*y)*2.0E1;

  ep_v[1] = nu*-4.0+3.0;
  ep_v[4] = -(x*(nu*4.0-3.0))/L;
  ep_v[6] = -x/L;
  ep_v[7] = (y*(nu*2.0-1.0)*-2.0)/L;
  ep_v[8] = -z/L;
  ep_v[10] = -(z*(nu*4.0-3.0))/L;
  ep_v[12] = 1.0/(L*L)*x*y*2.0;
  ep_v[13] = 1.0/(L*L)*x*y*-2.0;
  ep_v[14] = -1.0/(L*L)*x*z;
  ep_v[16] = -1.0/(L*L)*(x*x);
  ep_v[17] = -1.0/(L*L)*(nu*(y*y)*4.0-nu*(z*z)*4.0-y*y+(z*z)*3.0);
  ep_v[18] = 1.0/(L*L)*(nu*4.0-3.0)*(x*x-z*z);
  ep_v[19] = -1.0/(L*L)*x*z*(nu*4.0-3.0);
  ep_v[20] = 1.0/(L*L)*x*y*(nu*2.0-1.0)*-2.0;
  ep_v[21] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
  ep_v[23] = 1.0/(L*L)*y*z*2.0;
  ep_v[24] = -1.0/(L*L)*x*z;
  ep_v[25] = -1.0/(L*L)*(z*z);
  ep_v[27] = 1.0/(L*L*L)*(x*x)*y*6.0;
  ep_v[29] = 1.0/(L*L*L)*x*((x*x)*3.0-(y*y)*3.0);
  ep_v[30] = -1.0/(L*L*L)*x*((y*y)*3.0-(z*z)*3.0);
  ep_v[32] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_v[33] = -1.0/(L*L*L)*(x*x)*z;
  ep_v[34] = 1.0/(L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*-2.0;
  ep_v[35] = 1.0/(L*L*L)*y*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*-2.0;
  ep_v[36] = 1.0/(L*L*L)*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0);
  ep_v[37] = 1.0/(L*L*L)*z*(nu*4.0-3.0)*((x*x)*3.0-z*z);
  ep_v[38] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0);
  ep_v[39] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(z*z)*3.0);
  ep_v[40] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
  ep_v[42] = 1.0/(L*L*L)*y*(z*z)*6.0;
  ep_v[44] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_v[45] = -1.0/(L*L*L)*z*((y*y)*3.0-(z*z)*3.0);
  ep_v[46] = 1.0/(L*L*L)*z*((x*x)*3.0-(y*y)*3.0);
  ep_v[47] = -1.0/(L*L*L)*x*(z*z);
  ep_v[48] = -1.0/(L*L*L*L)*x*(x*(y*y)*3.0-x*x*x);
  ep_v[49] = 1.0/(L*L*L*L)*(x*x)*y*z*-6.0;
  ep_v[50] = -1.0/(L*L*L*L)*x*(x*(z*z)*3.0-x*x*x);
  ep_v[52] = 1.0/(L*L*L*L)*x*((x*x)*y*1.2E1-(y*y*y)*4.0);
  ep_v[53] = 1.0/(L*L*L*L)*x*z*(x*x-y*y)*3.0;
  ep_v[54] = 1.0/(L*L*L*L)*x*y*(x*x-z*z)*6.0;
  ep_v[55] = 1.0/(L*L*L*L)*x*((x*x)*z*3.0-z*z*z);
  ep_v[57] = 1.0/(L*L*L*L)*y*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0;
  ep_v[58] = 1.0/(L*L*L*L)*x*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0;
  ep_v[59] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0;
  ep_v[60] = 1.0/(L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0;
  ep_v[61] = -1.0/(L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0+y*y*y*y-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*2.4E1);
  ep_v[62] = 1.0/(L*L*L*L)*x*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0);
  ep_v[63] = -1.0/(L*L*L*L)*((x*x)*(y*y)*3.0-(x*x)*(z*z)*9.0+(y*y)*(z*z)*3.0+nu*(y*y*y*y)*4.0+y*y*y*y-nu*(x*x)*(y*y)*1.2E1+nu*(x*x)*(z*z)*1.2E1-nu*(y*y)*(z*z)*1.2E1);
  ep_v[64] = -1.0/(L*L*L*L)*x*z*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0);
  ep_v[65] = -1.0/(L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0-(x*x*x*x)*3.0+y*y*y*y-nu*(x*x)*(y*y)*2.4E1);
  ep_v[67] = -1.0/(L*L*L*L)*z*((x*x)*z*3.0-z*z*z);
  ep_v[68] = 1.0/(L*L*L*L)*x*y*(z*z)*-6.0;
  ep_v[69] = -1.0/(L*L*L*L)*z*((y*y)*z*3.0-z*z*z);
  ep_v[71] = 1.0/(L*L*L*L)*z*(x*(z*z)*3.0-x*x*x);
  ep_v[72] = 1.0/(L*L*L*L)*y*z*(x*x-z*z)*-6.0;
  ep_v[73] = 1.0/(L*L*L*L)*x*z*(y*y-z*z)*-3.0;
  ep_v[74] = 1.0/(L*L*L*L)*y*z*(y*y-(z*z)*3.0)*-4.0;
  ep_v[75] = 1.0/(L*L*L*L*L)*(x*x)*y*(x*x-y*y)*-2.0E1;
  ep_v[77] = 1.0/(L*L*L*L*L)*x*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0);
  ep_v[78] = 1.0/(L*L*L*L*L)*x*y*z*((x*x)*3.0-y*y)*-4.0;
  ep_v[79] = 1.0/(L*L*L*L*L)*x*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z);
  ep_v[81] = 1.0/(L*L*L*L*L)*(x*x)*z*(x*x-(y*y)*3.0);
  ep_v[82] = 1.0/(L*L*L*L*L)*(x*x)*z*(x*x-z*z);
  ep_v[83] = 1.0/(L*L*L*L*L)*x*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0);
  ep_v[84] = 1.0/(L*L*L*L*L)*(x*x)*y*(y*y-(z*z)*3.0)*4.0;
  ep_v[85] = 1.0/(L*L*L*L*L)*x*y*z*(y*y-z*z)*2.0E1;
  ep_v[86] = 1.0/(L*L*L*L*L)*y*(nu*(y*y*y*y)*2.0+nu*(z*z*z*z)*1.0E1+y*y*y*y-(z*z*z*z)*5.0-nu*(y*y)*(z*z)*2.0E1)*2.0;
  ep_v[87] = 1.0/(L*L*L*L*L)*y*(nu*(x*x*x*x)*1.0E1+nu*(y*y*y*y)*2.0-(x*x*x*x)*5.0+y*y*y*y-nu*(x*x)*(y*y)*2.0E1)*2.0;
  ep_v[88] = 1.0/(L*L*L*L*L)*z*((y*y)*(z*z)*1.0E1+nu*(y*y*y*y)*2.0E1+nu*(z*z*z*z)*4.0+(y*y*y*y)*5.0-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*4.0E1);
  ep_v[89] = 1.0/(L*L*L*L*L)*x*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0+y*y*y*y-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*2.4E1);
  ep_v[90] = 1.0/(L*L*L*L*L)*z*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0-(x*x*x*x)*3.0+y*y*y*y-nu*(x*x)*(y*y)*2.4E1);
  ep_v[91] = 1.0/(L*L*L*L*L)*x*((x*x)*(y*y)*1.0E1+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*2.0E1-(x*x*x*x)*3.0+(y*y*y*y)*5.0-nu*(x*x)*(y*y)*4.0E1);
  ep_v[92] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0;
  ep_v[93] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0;
  ep_v[94] =  1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*((x*x)*(z*z)*-1.0E1+(x*x*x*x)*5.0+z*z*z*z);
  ep_v[95] = 1.0/(L*L*L*L*L)*y*(nu*2.0-1.0)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*2.0;
  ep_v[96] = 1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*((x*x)*(z*z)*-1.0E1+x*x*x*x+(z*z*z*z)*5.0);
  ep_v[98] = 1.0/(L*L*L*L*L)*y*(z*z)*(y*y-z*z)*2.0E1;
  ep_v[100] = 1.0/(L*L*L*L*L)*z*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z);
  ep_v[101] = 1.0/(L*L*L*L*L)*x*y*z*(y*y-(z*z)*3.0)*4.0;
  ep_v[102] = 1.0/(L*L*L*L*L)*z*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0);
  ep_v[103] = -1.0/(L*L*L*L*L)*x*(z*z)*(x*x-z*z);
  ep_v[104] = -1.0/(L*L*L*L*L)*x*(z*z)*((y*y)*3.0-z*z);
  ep_v[105] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-y*y)*-2.0E1;
  ep_v[106] = 1.0/(L*L*L*L*L)*y*(z*z)*((x*x)*3.0-y*y)*-4.0;
  ep_v[107] = 1.0/(L*L*L*L*L)*z*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0);

  ep_w[2] = nu*-4.0+3.0;
  ep_w[5] = -(x*(nu*4.0-3.0))/L;
  ep_w[8] = -(y*(nu*4.0-3.0))/L;
  ep_w[9] = -x/L;
  ep_w[10] = -y/L;
  ep_w[11] = (z*(nu*2.0-1.0)*-2.0)/L;
  ep_w[13] = 1.0/(L*L)*x*z*2.0;
  ep_w[14] = -1.0/(L*L)*x*y;
  ep_w[15] = -1.0/(L*L)*(x*x);
  ep_w[17] = 1.0/(L*L)*y*z*2.0;
  ep_w[18] = 1.0/(L*L)*y*z*-2.0;
  ep_w[19] = -1.0/(L*L)*x*y;
  ep_w[21] = -1.0/(L*L)*(y*y);
  ep_w[22] = 1.0/(L*L)*(nu*(x*x)*4.0-nu*(z*z)*4.0-(x*x)*3.0+z*z);
  ep_w[23] = -1.0/(L*L)*(nu*4.0-3.0)*(x*x-y*y);
  ep_w[24] = -1.0/(L*L)*x*y*(nu*4.0-3.0);
  ep_w[25] = 1.0/(L*L)*y*z*(nu*2.0-1.0)*-2.0;
  ep_w[26] = 1.0/(L*L)*x*z*(nu*2.0-1.0)*-2.0;
  ep_w[28] = 1.0/(L*L*L)*(x*x)*z*6.0;
  ep_w[30] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_w[31] = 1.0/(L*L*L)*x*((x*x)*3.0-(z*z)*3.0);
  ep_w[32] = 1.0/(L*L*L)*x*((y*y)*3.0-(z*z)*3.0);
  ep_w[33] = -1.0/(L*L*L)*(x*x)*y;
  ep_w[34] = 1.0/(L*L*L)*(y*y)*z*6.0;
  ep_w[36] = 1.0/(L*L*L)*y*((y*y)*3.0-(z*z)*3.0);
  ep_w[37] = 1.0/(L*L*L)*y*((x*x)*3.0-(z*z)*3.0);
  ep_w[39] = 1.0/(L*L*L)*x*y*z*6.0;
  ep_w[40] = -1.0/(L*L*L)*x*(y*y);
  ep_w[41] = 1.0/(L*L*L)*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*-2.0;
  ep_w[42] = 1.0/(L*L*L)*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*-2.0;
  ep_w[43] = -1.0/(L*L*L)*x*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0);
  ep_w[44] = -1.0/(L*L*L)*x*(nu*4.0-3.0)*(x*x-(y*y)*3.0);
  ep_w[45] = -1.0/(L*L*L)*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0);
  ep_w[46] = 1.0/(L*L*L)*y*(nu*4.0-3.0)*((x*x)*3.0-y*y);
  ep_w[47] = 1.0/(L*L*L)*x*y*z*(nu*2.0-1.0)*-2.0;
  ep_w[49] = -1.0/(L*L*L*L)*x*(x*(y*y)*3.0-x*x*x);
  ep_w[50] = 1.0/(L*L*L*L)*(x*x)*y*z*-6.0;
  ep_w[51] = -1.0/(L*L*L*L)*x*(x*(z*z)*3.0-x*x*x);
  ep_w[53] = 1.0/(L*L*L*L)*x*((x*x)*y*3.0-y*y*y);
  ep_w[54] = 1.0/(L*L*L*L)*x*z*(x*x-y*y)*6.0;
  ep_w[55] = 1.0/(L*L*L*L)*x*y*(x*x-z*z)*3.0;
  ep_w[56] = 1.0/(L*L*L*L)*x*((x*x)*z*1.2E1-(z*z*z)*4.0);
  ep_w[57] = -1.0/(L*L*L*L)*y*(y*(z*z)*3.0-y*y*y);
  ep_w[58] = 1.0/(L*L*L*L)*x*(y*y)*z*-6.0;
  ep_w[59] = -1.0/(L*L*L*L)*y*((x*x)*y*3.0-y*y*y);
  ep_w[61] = 1.0/(L*L*L*L)*y*((y*y)*z*1.2E1-(z*z*z)*4.0);
  ep_w[62] = 1.0/(L*L*L*L)*x*y*(y*y-z*z)*3.0;
  ep_w[63] = 1.0/(L*L*L*L)*y*z*(x*x-y*y)*-6.0;
  ep_w[64] = 1.0/(L*L*L*L)*y*(x*(y*y)*3.0-x*x*x);
  ep_w[66] = 1.0/(L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0;
  ep_w[67] = 1.0/(L*L*L*L)*y*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0;
  ep_w[68] = 1.0/(L*L*L*L)*x*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0;
  ep_w[69] = 1.0/(L*L*L*L)*y*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0;
  ep_w[70] = -1.0/(L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0-(x*x*x*x)*3.0+z*z*z*z-nu*(x*x)*(z*z)*2.4E1);
  ep_w[71] = -1.0/(L*L*L*L)*x*y*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0);
  ep_w[72] = -1.0/(L*L*L*L)*((x*x)*(y*y)*-9.0+(x*x)*(z*z)*3.0+(y*y)*(z*z)*3.0+nu*(z*z*z*z)*4.0+z*z*z*z+nu*(x*x)*(y*y)*1.2E1-nu*(x*x)*(z*z)*1.2E1-nu*(y*y)*(z*z)*1.2E1);
  ep_w[73] = -1.0/(L*L*L*L)*x*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0);
  ep_w[74] = -1.0/(L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0-(y*y*y*y)*3.0+z*z*z*z-nu*(y*y)*(z*z)*2.4E1);
  ep_w[76] = 1.0/(L*L*L*L*L)*(x*x)*z*(x*x-z*z)*-2.0E1;
  ep_w[78] = 1.0/(L*L*L*L*L)*x*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y);
  ep_w[79] = 1.0/(L*L*L*L*L)*x*y*z*((x*x)*3.0-z*z)*-4.0;
  ep_w[80] = 1.0/(L*L*L*L*L)*x*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0);
  ep_w[81] = 1.0/(L*L*L*L*L)*(x*x)*y*(x*x-y*y);
  ep_w[82] = 1.0/(L*L*L*L*L)*(x*x)*y*(x*x-(z*z)*3.0);
  ep_w[83] = 1.0/(L*L*L*L*L)*x*y*z*(y*y-z*z)*-2.0E1;
  ep_w[84] = 1.0/(L*L*L*L*L)*(x*x)*z*((y*y)*3.0-z*z)*-4.0;
  ep_w[85] = 1.0/(L*L*L*L*L)*x*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0);
  ep_w[86] = 1.0/(L*L*L*L*L)*(y*y)*z*(y*y-z*z)*-2.0E1;
  ep_w[88] = 1.0/(L*L*L*L*L)*y*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0);
  ep_w[89] = 1.0/(L*L*L*L*L)*x*y*z*((y*y)*3.0-z*z)*-4.0;
  ep_w[90] = 1.0/(L*L*L*L*L)*y*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y);
  ep_w[92] = 1.0/(L*L*L*L*L)*x*(y*y)*(y*y-(z*z)*3.0);
  ep_w[93] = -1.0/(L*L*L*L*L)*x*(y*y)*(x*x-y*y);
  ep_w[94] = 1.0/(L*L*L*L*L)*y*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0);
  ep_w[95] = 1.0/(L*L*L*L*L)*(y*y)*z*((x*x)*3.0-z*z)*-4.0;
  ep_w[96] = 1.0/(L*L*L*L*L)*x*y*z*(x*x-z*z)*-2.0E1;
  ep_w[97] = 1.0/(L*L*L*L*L)*z*(nu*(x*x*x*x)*1.0E1+nu*(z*z*z*z)*2.0-(x*x*x*x)*5.0+z*z*z*z-nu*(x*x)*(z*z)*2.0E1)*2.0;
  ep_w[98] = 1.0/(L*L*L*L*L)*z*(nu*(y*y*y*y)*1.0E1+nu*(z*z*z*z)*2.0-(y*y*y*y)*5.0+z*z*z*z-nu*(y*y)*(z*z)*2.0E1)*2.0;
  ep_w[99] = 1.0/(L*L*L*L*L)*x*((x*x)*(z*z)*1.0E1+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*2.0E1-(x*x*x*x)*3.0+(z*z*z*z)*5.0-nu*(x*x)*(z*z)*4.0E1);
  ep_w[100] = 1.0/(L*L*L*L*L)*y*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0-(x*x*x*x)*3.0+z*z*z*z-nu*(x*x)*(z*z)*2.4E1);
  ep_w[101] = 1.0/(L*L*L*L*L)*x*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0-(y*y*y*y)*3.0+z*z*z*z-nu*(y*y)*(z*z)*2.4E1);
  ep_w[102] = 1.0/(L*L*L*L*L)*y*((y*y)*(z*z)*1.0E1+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*2.0E1-(y*y*y*y)*3.0+(z*z*z*z)*5.0-nu*(y*y)*(z*z)*4.0E1);
  ep_w[103] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0;
  ep_w[104] = 1.0/(L*L*L*L*L)*x*y*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0;
  ep_w[105] = 1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*((x*x)*(y*y)*-1.0E1+x*x*x*x+(y*y*y*y)*5.0);
  ep_w[106] = 1.0/(L*L*L*L*L)*z*(nu*2.0-1.0)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*2.0;
  ep_w[107] = 1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*((x*x)*(y*y)*-1.0E1+(x*x*x*x)*5.0+y*y*y*y);

	}
}
/**********************************************************************/
void ElasticPolynomial::computeStressCoefficients(double pt[3])
{
	double x,y,z,nu,lambda,mu,L;

	if (ep_verbose) printf("ElasticPolynomial: Computing stress coefficients\n");
	// generate constraints
	x = pt[0] - ep_origin[0];
	y = pt[1] - ep_origin[1];
	z = pt[2] - ep_origin[2];

	nu = ep_nu;
	lambda = ep_lambda;
	mu = ep_mu;
	L = ep_L;

	for (int i =0; i < ep_nCoefficients; i++) {
		ep_s11(i) = 0;
		ep_s22(i) = 0;
		ep_s33(i) = 0;
		ep_s12(i) = 0;
		ep_s13(i) = 0;
		ep_s23(i) = 0;
	}

	int order;

	order = ep_order;

	if (order == 0){
		// nothing to do.
	}

	else if (order == 1) {
		ep_s11[3] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s11[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;

		ep_s12[4] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[6] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;

		ep_s13[5] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[9] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;

		ep_s22[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[7] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s22[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;

		ep_s23[8] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[10] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;

		ep_s33[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[11] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;

	}
	else if (order == 2) {
		ep_s11[3] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s11[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*(x*2.0-nu*x*8.0)*2.0;
		ep_s11[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s11[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s11[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0));
		ep_s11[18] = 1.0/(L*L)*mu*y*4.0;
		ep_s11[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s11[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s11[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*z*4.0;
		ep_s11[23] = 1.0/(L*L)*mu*z*-4.0;
		ep_s11[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s11[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;

		ep_s12[4] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[6] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[12] = mu*(1.0/(L*L)*y-1.0/(L*L)*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[13] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*-2.0;
		ep_s12[14] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[16] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s12[18] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*2.0;
		ep_s12[19] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[20] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s12[24] = 1.0/(L*L)*mu*z*-2.0;

		ep_s13[5] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[9] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[13] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*2.0;
		ep_s13[14] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[15] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s13[19] = 1.0/(L*L)*mu*y*-2.0;
		ep_s13[22] = mu*(1.0/(L*L)*x-1.0/(L*L)*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
		ep_s13[23] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*-2.0;
		ep_s13[24] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[26] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
	
		ep_s22[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[7] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s22[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*x*4.0;
		ep_s22[13] = 1.0/(L*L)*mu*x*-4.0;
		ep_s22[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s22[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*(y*2.0-nu*y*8.0)*2.0;
		ep_s22[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
		ep_s22[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s22[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0));
		ep_s22[23] = 1.0/(L*L)*mu*z*4.0;
		ep_s22[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;

		ep_s23[8] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[10] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[14] = 1.0/(L*L)*mu*x*-2.0;
		ep_s23[17] = mu*(1.0/(L*L)*z-1.0/(L*L)*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[18] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*-2.0;
		ep_s23[19] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[21] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s23[23] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*2.0;
		ep_s23[24] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[25] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;

		ep_s33[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[11] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s33[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0));
		ep_s33[13] = 1.0/(L*L)*mu*x*4.0;
		ep_s33[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s33[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*y*4.0;
		ep_s33[18] = 1.0/(L*L)*mu*y*-4.0;
		ep_s33[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s33[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*(z*2.0-nu*z*8.0)*2.0;
		ep_s33[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s33[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;

	}
	else if (order == 3) {
		ep_s11[3] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s11[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*(x*2.0-nu*x*8.0)*2.0;
		ep_s11[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s11[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s11[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0));
		ep_s11[18] = 1.0/(L*L)*mu*y*4.0;
		ep_s11[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s11[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s11[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*z*4.0;
		ep_s11[23] = 1.0/(L*L)*mu*z*-4.0;
		ep_s11[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s11[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s11[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
		ep_s11[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
		ep_s11[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*y*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*z*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*y*z*(nu*2.0-1.0)*4.0;
		ep_s11[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
		ep_s11[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
		ep_s11[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0);
		ep_s11[37] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s11[38]= -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s11[39] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
		ep_s11[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
		ep_s11[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
		ep_s11[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
		ep_s11[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s11[44] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
		ep_s11[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0);
		ep_s11[46]= 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s11[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
		
		ep_s12[4] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[6] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[12] = mu*(1.0/(L*L)*y-1.0/(L*L)*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[13] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*-2.0;
		ep_s12[14] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[16] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s12[18] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*2.0;
		ep_s12[19] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[20] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s12[24] = 1.0/(L*L)*mu*z*-2.0;
		ep_s12[27] = mu*(1.0/(L*L*L)*x*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
		ep_s12[29] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[30] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(y*y-(z*z)*3.0)*(1.0/2.0))*-2.0;
		ep_s12[32] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s12[33] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
		ep_s12[35] = mu*(1.0/(L*L*L)*y*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
		ep_s12[37] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s12[38] = mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
		ep_s12[39] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(z*z)*3.0)*(1.0/2.0))*-2.0;
		ep_s12[40] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
		ep_s12[44] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s12[46] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s12[47] = 1.0/(L*L*L)*mu*(z*z)*-2.0;
		
		ep_s13[5] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[9] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[13] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*2.0;
		ep_s13[14] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[15] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s13[19] = 1.0/(L*L)*mu*y*-2.0;
		ep_s13[22] = mu*(1.0/(L*L)*x-1.0/(L*L)*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
		ep_s13[23] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*-2.0;
		ep_s13[24] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[26] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
		ep_s13[28] = mu*(1.0/(L*L*L)*x*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
		ep_s13[30] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s13[31] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s13[32] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((y*y)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
		ep_s13[33] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
		ep_s13[37] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s13[39] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s13[40] = 1.0/(L*L*L)*mu*(y*y)*-2.0;
		ep_s13[41] = mu*(1.0/(L*L*L)*z*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
		ep_s13[43] = mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
		ep_s13[44] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(y*y)*3.0)*(1.0/2.0))*-2.0;
		ep_s13[46] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
		ep_s13[47] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
		
		ep_s22[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[7] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s22[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*x*4.0;
		ep_s22[13] = 1.0/(L*L)*mu*x*-4.0;
		ep_s22[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s22[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*(y*2.0-nu*y*8.0)*2.0;
		ep_s22[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
		ep_s22[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s22[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0));
		ep_s22[23] = 1.0/(L*L)*mu*z*4.0;
		ep_s22[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s22[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
		ep_s22[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
		ep_s22[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s22[30] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
		ep_s22[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0);
		ep_s22[32] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s22[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
		ep_s22[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
		ep_s22[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
		ep_s22[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*z*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*z*(nu*2.0-1.0)*4.0;
		ep_s22[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
		ep_s22[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
		ep_s22[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0);
		ep_s22[44] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s22[45] =  -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s22[46] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
		ep_s22[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
		
		ep_s23[8] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[10] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[14] = 1.0/(L*L)*mu*x*-2.0;
		ep_s23[17] = mu*(1.0/(L*L)*z-1.0/(L*L)*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[18] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*-2.0;
		ep_s23[19] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[21] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s23[23]= mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*2.0;
		ep_s23[24] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[25] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
		ep_s23[30] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s23[32] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s23[33] = 1.0/(L*L*L)*mu*(x*x)*-2.0;
		ep_s23[34] = mu*(1.0/(L*L*L)*y*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
		ep_s23[36] = mu*(1.0/(L*L*L)*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[37] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
		ep_s23[39] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s23[40] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
		ep_s23[42] = mu*(1.0/(L*L*L)*z*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
		ep_s23[44] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
		ep_s23[45] = mu*(1.0/(L*L*L)*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*-2.0;
		ep_s23[46] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-y*y)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*(nu*4.0-3.0))*2.0;
		ep_s23[47] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
		
		ep_s33[3]= (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[11] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s33[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0));
		ep_s33[13] = 1.0/(L*L)*mu*x*4.0;
		ep_s33[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s33[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*y*4.0;
		ep_s33[18] = 1.0/(L*L)*mu*y*-4.0;
		ep_s33[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s33[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*(z*2.0-nu*z*8.0)*2.0;
		ep_s33[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s33[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
		ep_s33[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
		ep_s33[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
		ep_s33[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0);
		ep_s33[30] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s33[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s33[32] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
		ep_s33[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
		ep_s33[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
		ep_s33[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
		ep_s33[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s33[37] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
		ep_s33[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0);
		ep_s33[39] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s33[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
		ep_s33[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
		ep_s33[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
		ep_s33[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*y*(nu*2.0-1.0)*4.0;
	}
	else if (order == 4){
		ep_s11[3] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s11[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s11[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*(x*2.0-nu*x*8.0)*2.0;
		ep_s11[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s11[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s11[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0));
		ep_s11[18] = 1.0/(L*L)*mu*y*4.0;
		ep_s11[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s11[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s11[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*z*4.0;
		ep_s11[23] = 1.0/(L*L)*mu*z*-4.0;
		ep_s11[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s11[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s11[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
		ep_s11[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
		ep_s11[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*y*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*z*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*y*z*(nu*2.0-1.0)*4.0;
		ep_s11[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
		ep_s11[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
		ep_s11[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0);
		ep_s11[37] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s11[38]= -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s11[39] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
		ep_s11[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
		ep_s11[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
		ep_s11[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
		ep_s11[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s11[44] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
		ep_s11[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0);
		ep_s11[46]= 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s11[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
		ep_s11[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)+mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)*2.0;
		ep_s11[49] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
		ep_s11[50] = mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
		ep_s11[51] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
		ep_s11[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)*2.0;
		ep_s11[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*y*z*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)*2.0;
		ep_s11[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*y*z*(x*6.0-nu*x*2.4E1)*2.0;
		ep_s11[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)*2.0;
		ep_s11[57] = lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
		ep_s11[58] = lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
		ep_s11[59] = lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)-1.0/(L*L*L*L)*mu*(y*y)*z*1.2E1;
		ep_s11[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)-1.0/(L*L*L*L)*mu*x*(y*y)*1.2E1;
		ep_s11[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1));
		ep_s11[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1));
		ep_s11[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)+1.0/(L*L*L*L)*mu*y*(y*y-z*z)*1.2E1;
		ep_s11[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s11[65] = mu*(1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0);
		ep_s11[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)-1.0/(L*L*L*L)*mu*x*(z*z)*1.2E1;
		ep_s11[67] = lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)-1.0/(L*L*L*L)*mu*y*(z*z)*1.2E1;
		ep_s11[68] = lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
		ep_s11[69] = lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
		ep_s11[70] = mu*(1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0);
		ep_s11[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s11[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*z*(y*y-z*z)*1.2E1;
		ep_s11[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1));
		ep_s11[74] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0);
		
		ep_s12[4] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[6] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s12[12] = mu*(1.0/(L*L)*y-1.0/(L*L)*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[13] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*-2.0;
		ep_s12[14] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[16] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s12[18] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*2.0;
		ep_s12[19] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s12[20] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s12[24] = 1.0/(L*L)*mu*z*-2.0;
		ep_s12[27] = mu*(1.0/(L*L*L)*x*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
		ep_s12[29] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[30] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(y*y-(z*z)*3.0)*(1.0/2.0))*-2.0;
		ep_s12[32] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s12[33] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
		ep_s12[35] = mu*(1.0/(L*L*L)*y*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
		ep_s12[37] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s12[38] = mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
		ep_s12[39] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(z*z)*3.0)*(1.0/2.0))*-2.0;
		ep_s12[40] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
		ep_s12[44] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s12[46] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s12[47] = 1.0/(L*L*L)*mu*(z*z)*-2.0;
		ep_s12[48] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)+1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*y*(y*2.0-nu*y*4.0))*2.0;
		ep_s12[49] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*1.2E1))*-2.0;
		ep_s12[50] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0))*2.0;
		ep_s12[52] = mu*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1-(y*y*y)*1.2E1-nu*(x*x)*y*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((x*x)*y*1.2E1-(y*y*y)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*1.2E1)*2.0;
		ep_s12[53] = mu*(1.0/(L*L*L*L)*(x*x)*z*3.0+1.0/(L*L*L*L)*z*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(x*x-y*y)*(3.0/2.0)+1.0/(L*L*L*L)*y*z*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s12[54] = mu*(1.0/(L*L*L*L)*((x*x)*y*6.0-y*(z*z)*1.8E1-nu*(x*x)*y*2.4E1+nu*y*(z*z)*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*y*(x*x-z*z)*3.0)*2.0;
		ep_s12[55] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*3.0+1.0/(L*L*L*L)*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
		ep_s12[58] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(-1.0/2.0)+1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0))*2.0;
		ep_s12[59] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*1.2E1))*-2.0;
		ep_s12[60] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)-1.0/(L*L*L*L)*y*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)+1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*y*(x*2.0-nu*x*4.0))*-2.0;
		ep_s12[62] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*3.0+1.0/(L*L*L*L)*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
		ep_s12[63] = mu*(1.0/(L*L*L*L)*(x*(y*y)*6.0-x*(z*z)*1.8E1-nu*x*(y*y)*2.4E1+nu*x*(z*z)*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*x*(y*y-z*z)*3.0)*2.0;
		ep_s12[64] = mu*(1.0/(L*L*L*L)*(y*y)*z*3.0-1.0/(L*L*L*L)*z*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(x*x-y*y)*(3.0/2.0)+1.0/(L*L*L*L)*x*z*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
		ep_s12[65] = mu*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1-(x*x*x)*1.2E1-nu*x*(y*y)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*x*(y*y)*1.2E1+1.0/(L*L*L*L)*x*(x*x-(y*y)*3.0)*2.0)*-2.0;
		ep_s12[67] = 1.0/(L*L*L*L)*mu*x*(z*z)*-1.2E1;
		ep_s12[68] = 1.0/(L*L*L*L)*mu*y*(z*z)*-1.2E1;
		ep_s12[71] = mu*(1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(x*x-z*z)*(3.0/2.0))*-2.0;
		ep_s12[72] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
		ep_s12[73] = mu*(1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(y*y-z*z)*(3.0/2.0))*-2.0;
		
		ep_s13[5] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[9] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s13[13] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*2.0;
		ep_s13[14] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[15] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
		ep_s13[19] = 1.0/(L*L)*mu*y*-2.0;
		ep_s13[22] = mu*(1.0/(L*L)*x-1.0/(L*L)*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
		ep_s13[23] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*-2.0;
		ep_s13[24] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s13[26] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
		ep_s13[28] = mu*(1.0/(L*L*L)*x*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
		ep_s13[30] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s13[31] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s13[32] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((y*y)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
		ep_s13[33] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
		ep_s13[37] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s13[39] = 1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s13[40] = 1.0/(L*L*L)*mu*(y*y)*-2.0;
		ep_s13[41] = mu*(1.0/(L*L*L)*z*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
		ep_s13[43] = mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
		ep_s13[44] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(y*y)*3.0)*(1.0/2.0))*-2.0;
		ep_s13[46] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
		ep_s13[47] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
		ep_s13[49] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0))*2.0;
		ep_s13[50] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*1.2E1))*-2.0;
		ep_s13[51] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*z*(z*2.0-nu*z*4.0))*2.0;
		ep_s13[53] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*3.0+1.0/(L*L*L*L)*y*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0))*2.0;
		ep_s13[54] = mu*(1.0/(L*L*L*L)*((x*x)*z*6.0-(y*y)*z*1.8E1-nu*(x*x)*z*2.4E1+nu*(y*y)*z*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*z*(x*x-y*y)*3.0)*2.0;
		ep_s13[55] = mu*(1.0/(L*L*L*L)*(x*x)*y*3.0+1.0/(L*L*L*L)*y*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(x*x-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*y*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s13[56] = mu*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1-(z*z*z)*1.2E1-nu*(x*x)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((x*x)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*1.2E1)*2.0;
		ep_s13[58] = 1.0/(L*L*L*L)*mu*(y*y)*z*-1.2E1;
		ep_s13[59] = 1.0/(L*L*L*L)*mu*x*(y*y)*-1.2E1;
		ep_s13[62] = mu*(1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(y*y-z*z)*(3.0/2.0))*2.0;
		ep_s13[63] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
		ep_s13[64] = mu*(1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(x*x-y*y)*(3.0/2.0))*-2.0;
		ep_s13[66] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)+1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*z*(x*2.0-nu*x*4.0))*-2.0;
		ep_s13[67] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*1.2E1))*-2.0;
		ep_s13[68] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0))*-2.0;
		ep_s13[70] = mu*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1-(x*x*x)*1.2E1-nu*x*(z*z)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*x*(z*z)*1.2E1+1.0/(L*L*L*L)*x*(x*x-(z*z)*3.0)*2.0)*-2.0;
		ep_s13[71] = mu*(1.0/(L*L*L*L)*y*(z*z)*3.0-1.0/(L*L*L*L)*y*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*(x*x-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*y*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
		ep_s13[72] = mu*(1.0/(L*L*L*L)*(x*(y*y)*1.8E1-x*(z*z)*6.0-nu*x*(y*y)*2.4E1+nu*x*(z*z)*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*x*(z*z)*6.0-1.0/(L*L*L*L)*x*(y*y-z*z)*3.0)*2.0;
		ep_s13[73] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*y*(z*z)*3.0-1.0/(L*L*L*L)*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
		
		ep_s22[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[7] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s22[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s22[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*x*4.0;
		ep_s22[13] = 1.0/(L*L)*mu*x*-4.0;
		ep_s22[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s22[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*(y*2.0-nu*y*8.0)*2.0;
		ep_s22[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
		ep_s22[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
		ep_s22[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0));
		ep_s22[23] = 1.0/(L*L)*mu*z*4.0;
		ep_s22[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s22[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s22[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
		ep_s22[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
		ep_s22[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s22[30] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
		ep_s22[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0);
		ep_s22[32] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s22[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
		ep_s22[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
		ep_s22[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
		ep_s22[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*z*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*z*(nu*2.0-1.0)*4.0;
		ep_s22[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
		ep_s22[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
		ep_s22[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0);
		ep_s22[44] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s22[45] =  -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s22[46] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
		ep_s22[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
		ep_s22[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)-1.0/(L*L*L*L)*mu*(x*x)*y*1.2E1;
		ep_s22[49] = lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)-1.0/(L*L*L*L)*mu*(x*x)*z*1.2E1;
		ep_s22[50] = lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
		ep_s22[51] = lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
		ep_s22[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1))+1.0/(L*L*L*L)*mu*x*((x*x)*1.2E1-(y*y)*1.2E1)*2.0;
		ep_s22[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s22[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)+1.0/(L*L*L*L)*mu*x*(x*x-z*z)*1.2E1;
		ep_s22[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1));
		ep_s22[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1));
		ep_s22[57] = mu*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
		ep_s22[58] = mu*(1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
		ep_s22[59] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
		ep_s22[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)+mu*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)*2.0;
		ep_s22[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)*2.0;
		ep_s22[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*z*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)*2.0;
		ep_s22[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*z*(y*6.0-nu*y*2.4E1)*2.0;
		ep_s22[65] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)*2.0;
		ep_s22[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
		ep_s22[67] = lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
		ep_s22[68] = lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)-1.0/(L*L*L*L)*mu*x*(z*z)*1.2E1;
		ep_s22[69] = lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)-1.0/(L*L*L*L)*mu*y*(z*z)*1.2E1;
		ep_s22[70] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0);
		ep_s22[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1));
		ep_s22[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*z*(x*x-z*z)*1.2E1;
		ep_s22[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s22[74] = mu*(1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0);
		
		ep_s23[8] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[10] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
		ep_s23[14] = 1.0/(L*L)*mu*x*-2.0;
		ep_s23[17] = mu*(1.0/(L*L)*z-1.0/(L*L)*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[18] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*-2.0;
		ep_s23[19] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[21] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
		ep_s23[23]= mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*2.0;
		ep_s23[24] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
		ep_s23[25] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
		ep_s23[30] = 1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s23[32] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s23[33] = 1.0/(L*L*L)*mu*(x*x)*-2.0;
		ep_s23[34] = mu*(1.0/(L*L*L)*y*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
		ep_s23[36] = mu*(1.0/(L*L*L)*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[37] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
		ep_s23[39] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
		ep_s23[40] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
		ep_s23[42] = mu*(1.0/(L*L*L)*z*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
		ep_s23[44] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
		ep_s23[45] = mu*(1.0/(L*L*L)*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*-2.0;
		ep_s23[46] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-y*y)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*(nu*4.0-3.0))*2.0;
		ep_s23[47] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
		ep_s23[49] = 1.0/(L*L*L*L)*mu*(x*x)*y*-1.2E1;
		ep_s23[50] = 1.0/(L*L*L*L)*mu*(x*x)*z*-1.2E1;
		ep_s23[53] = mu*(1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(x*x-y*y)*(3.0/2.0))*2.0;
		ep_s23[54] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
		ep_s23[55] = mu*(1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(x*x-z*z)*(3.0/2.0))*2.0;
		ep_s23[57] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(-1.0/2.0)+1.0/(L*L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*z*(z*2.0-nu*z*4.0))*2.0;
		ep_s23[58]= mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*1.2E1))*-2.0;
		ep_s23[59] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0))*-2.0;
		ep_s23[61] = mu*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1-(z*z*z)*1.2E1-nu*(y*y)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((y*y)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*1.2E1)*2.0;
		ep_s23[62] = mu*(1.0/(L*L*L*L)*x*(y*y)*3.0+1.0/(L*L*L*L)*x*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(y*y-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
		ep_s23[63] = mu*(1.0/(L*L*L*L)*((x*x)*z*1.8E1-(y*y)*z*6.0-nu*(x*x)*z*2.4E1+nu*(y*y)*z*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*6.0-1.0/(L*L*L*L)*z*(x*x-y*y)*3.0)*2.0;
		ep_s23[64] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(1.0/2.0)+1.0/(L*L*L*L)*x*(y*y)*3.0-1.0/(L*L*L*L)*x*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0))*2.0;
		ep_s23[67] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0))*-2.0;
		ep_s23[68] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*1.2E1))*-2.0;
		ep_s23[69] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)+1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*z*(y*2.0-nu*y*4.0))*-2.0;
		ep_s23[71] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(1.0/2.0)+1.0/(L*L*L*L)*x*(z*z)*3.0-1.0/(L*L*L*L)*x*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
		ep_s23[72] = mu*(1.0/(L*L*L*L)*((x*x)*y*1.8E1-y*(z*z)*6.0-nu*(x*x)*y*2.4E1+nu*y*(z*z)*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*y*(z*z)*6.0-1.0/(L*L*L*L)*y*(x*x-z*z)*3.0)*2.0;
		ep_s23[73] = mu*(1.0/(L*L*L*L)*x*(z*z)*3.0-1.0/(L*L*L*L)*x*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*(y*y-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
		ep_s23[74] = mu*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1-(y*y*y)*1.2E1-nu*y*(z*z)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*y*(z*z)*1.2E1+1.0/(L*L*L*L)*y*(y*y-(z*z)*3.0)*2.0)*-2.0;
		
		ep_s33[3]= (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
		ep_s33[11] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
		ep_s33[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0));
		ep_s33[13] = 1.0/(L*L)*mu*x*4.0;
		ep_s33[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
		ep_s33[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*y*4.0;
		ep_s33[18] = 1.0/(L*L)*mu*y*-4.0;
		ep_s33[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
		ep_s33[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
		ep_s33[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*(z*2.0-nu*z*8.0)*2.0;
		ep_s33[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
		ep_s33[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
		ep_s33[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
		ep_s33[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
		ep_s33[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0);
		ep_s33[30] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s33[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
		ep_s33[32] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
		ep_s33[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
		ep_s33[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
		ep_s33[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
		ep_s33[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
		ep_s33[37] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
		ep_s33[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0);
		ep_s33[39] = 1.0/(L*L*L)*mu*x*y*1.2E1;
		ep_s33[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
		ep_s33[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
		ep_s33[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
		ep_s33[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*y*(nu*2.0-1.0)*4.0;
		ep_s33[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
		ep_s33[49] = lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
		ep_s33[50] = lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)-1.0/(L*L*L*L)*mu*(x*x)*y*1.2E1;
		ep_s33[51] = lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)-1.0/(L*L*L*L)*mu*(x*x)*z*1.2E1;
		ep_s33[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1));
		ep_s33[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1));
		ep_s33[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)+1.0/(L*L*L*L)*mu*x*(x*x-y*y)*1.2E1;
		ep_s33[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s33[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1))+1.0/(L*L*L*L)*mu*x*((x*x)*1.2E1-(z*z)*1.2E1)*2.0;
		ep_s33[57] = lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)-1.0/(L*L*L*L)*mu*(y*y)*z*1.2E1;
		ep_s33[58] = lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)-1.0/(L*L*L*L)*mu*x*(y*y)*1.2E1;
		ep_s33[59] = lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
		ep_s33[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
		ep_s33[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1))+1.0/(L*L*L*L)*mu*y*((y*y)*1.2E1-(z*z)*1.2E1)*2.0;
		ep_s33[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
		ep_s33[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*y*(x*x-y*y)*1.2E1;
		ep_s33[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1));
		ep_s33[65] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0);
		ep_s33[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)+mu*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)*2.0;
		ep_s33[67] = mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
		ep_s33[68] = mu*(1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
		ep_s33[69] = mu*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
		ep_s33[70] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)*2.0;
		ep_s33[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)*2.0;
		ep_s33[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*(z*6.0-nu*z*2.4E1)*2.0;
		ep_s33[74] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)*2.0;
}
else if (order == 5){
  ep_s11[3] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
  ep_s11[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s11[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s11[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*(x*2.0-nu*x*8.0)*2.0;
  ep_s11[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
  ep_s11[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
  ep_s11[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0));
  ep_s11[18] = 1.0/(L*L)*mu*y*4.0;
  ep_s11[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
  ep_s11[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
  ep_s11[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*z*4.0;
  ep_s11[23] = 1.0/(L*L)*mu*z*-4.0;
  ep_s11[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
  ep_s11[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
  ep_s11[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
  ep_s11[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)*2.0;
  ep_s11[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*y*(x*6.0-nu*x*2.4E1)*2.0;
  ep_s11[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*z*(x*6.0-nu*x*2.4E1)*2.0;
  ep_s11[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*y*z*(nu*2.0-1.0)*4.0;
  ep_s11[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
  ep_s11[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
  ep_s11[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0);
  ep_s11[37] = 1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s11[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s11[39] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
  ep_s11[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
  ep_s11[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
  ep_s11[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
  ep_s11[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s11[44] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
  ep_s11[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0);
  ep_s11[46] = 1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s11[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
  ep_s11[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)+mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)*2.0;
  ep_s11[49] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
  ep_s11[50] = mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
  ep_s11[51] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
  ep_s11[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)*2.0;
  ep_s11[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*y*z*(x*6.0-nu*x*2.4E1)*2.0;
  ep_s11[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)*2.0;
  ep_s11[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*y*z*(x*6.0-nu*x*2.4E1)*2.0;
  ep_s11[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)*2.0;
  ep_s11[57] = lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
  ep_s11[58] = lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
  ep_s11[59] = lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)-1.0/(L*L*L*L)*mu*(y*y)*z*1.2E1;
  ep_s11[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)-1.0/(L*L*L*L)*mu*x*(y*y)*1.2E1;
  ep_s11[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1));
  ep_s11[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1));
  ep_s11[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)+1.0/(L*L*L*L)*mu*y*(y*y-z*z)*1.2E1;
  ep_s11[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s11[65] = mu*(1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0);
  ep_s11[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)-1.0/(L*L*L*L)*mu*x*(z*z)*1.2E1;
  ep_s11[67] = lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)-1.0/(L*L*L*L)*mu*y*(z*z)*1.2E1;
  ep_s11[68] = lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
  ep_s11[69] = lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
  ep_s11[70] = mu*(1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0);
  ep_s11[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s11[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*z*(y*y-z*z)*1.2E1;
  ep_s11[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1));
  ep_s11[74] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0);
  ep_s11[75] = mu*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(y*y*y*y)*1.0E1+x*x*x*x-(y*y*y*y)*5.0-nu*(x*x)*(y*y)*2.0E1)*2.0+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(y*y)*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(y*y*y*y)*1.0E1+x*x*x*x-(y*y*y*y)*5.0-nu*(x*x)*(y*y)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(y*y)*4.0E1)*2.0);
  ep_s11[76] = mu*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(z*z*z*z)*1.0E1+x*x*x*x-(z*z*z*z)*5.0-nu*(x*x)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(z*z)*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(z*z*z*z)*1.0E1+x*x*x*x-(z*z*z*z)*5.0-nu*(x*x)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(z*z)*4.0E1)*2.0);
  ep_s11[77] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*8.0E1+x*(y*y)*2.0E1+(x*x*x)*2.0E1-nu*x*(y*y)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*y*6.0E1-(y*y*y)*2.0E1))+1.0/(L*L*L*L*L)*mu*y*(nu*(x*x*x)*8.0E1+x*(y*y)*2.0E1+(x*x*x)*2.0E1-nu*x*(y*y)*8.0E1)*2.0;
  ep_s11[78] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L*L)*x*z*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)+1.0/(L*L*L*L*L)*mu*z*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)*2.0;
  ep_s11[79] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)+1.0/(L*L*L*L*L)*mu*y*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)*2.0;
  ep_s11[80] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*8.0E1+x*(z*z)*2.0E1+(x*x*x)*2.0E1-nu*x*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*z*6.0E1-(z*z*z)*2.0E1))+1.0/(L*L*L*L*L)*mu*z*(nu*(x*x*x)*8.0E1+x*(z*z)*2.0E1+(x*x*x)*2.0E1-nu*x*(z*z)*8.0E1)*2.0;
  ep_s11[81] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)+mu*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)*2.0;
  ep_s11[82] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)+mu*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)*2.0;
  ep_s11[83] = -lambda*(1.0/(L*L*L*L*L)*x*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(y*y-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1);
  ep_s11[84] = lambda*(1.0/(L*L*L*L*L)*(x*x)*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0)+1.0/(L*L*L*L*L)*mu*(nu*2.0-1.0)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*4.0;
  ep_s11[85] = lambda*(-1.0/(L*L*L*L*L)*x*((y*y)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1);
  ep_s11[86] = lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*2.0+nu*(z*z*z*z)*1.0E1+y*y*y*y-(z*z*z*z)*5.0-nu*(y*y)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(y*y)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*y*(z*z)*4.0E1)*2.0);
  ep_s11[87] = mu*(1.0/(L*L*L*L*L)*(y*y)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(y*y*y*y)*2.0-(x*x*x*x)*5.0+y*y*y*y-nu*(x*x)*(y*y)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(y*y)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*(x*x)*y*4.0E1)*2.0);
  ep_s11[88] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*8.0E1+y*(z*z)*2.0E1+(y*y*y)*2.0E1-nu*y*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*y*((y*y)*z*6.0E1-(z*z*z)*2.0E1));
  ep_s11[89] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0);
  ep_s11[90] = mu*(1.0/(L*L*L*L*L)*y*z*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0)*2.0+lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0);
  ep_s11[91] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*8.0E1+(x*x)*y*2.0E1+(y*y*y)*2.0E1-nu*(x*x)*y*8.0E1)-1.0/(L*L*L*L*L)*y*(x*(y*y)*6.0E1-(x*x*x)*2.0E1))-1.0/(L*L*L*L*L)*mu*y*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)*2.0;
  ep_s11[92] = lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0);
  ep_s11[93] = lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)-1.0/(L*L*L*L*L)*mu*x*(y*y)*z*1.2E1;
  ep_s11[94] = lambda*(-1.0/(L*L*L*L*L)*y*((x*x)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)+mu*(1.0/(L*L*L*L*L)*y*z*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)*2.0;
  ep_s11[95] = lambda*(1.0/(L*L*L*L*L)*(y*y)*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(y*y)*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)+mu*(1.0/(L*L*L*L*L)*(y*y)*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0)*2.0;
  ep_s11[96] = -lambda*(1.0/(L*L*L*L*L)*y*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(x*x-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)-1.0/(L*L*L*L*L)*mu*y*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)*2.0;
  ep_s11[97] = mu*(1.0/(L*L*L*L*L)*(z*z)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(z*z*z*z)*2.0-(x*x*x*x)*5.0+z*z*z*z-nu*(x*x)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(x*x)*z*4.0E1)*2.0);
  ep_s11[98] = lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*1.0E1+nu*(z*z*z*z)*2.0-(y*y*y*y)*5.0+z*z*z*z-nu*(y*y)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(y*y)*z*4.0E1)*2.0);
  ep_s11[99] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*8.0E1+(x*x)*z*2.0E1+(z*z*z)*2.0E1-nu*(x*x)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(x*(z*z)*6.0E1-(x*x*x)*2.0E1))-1.0/(L*L*L*L*L)*mu*z*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)*2.0;
  ep_s11[100] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0)+mu*(1.0/(L*L*L*L*L)*y*z*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0)*2.0;
  ep_s11[101] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L*L)*x*z*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0);
  ep_s11[102] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*8.0E1+(y*y)*z*2.0E1+(z*z*z)*2.0E1-nu*(y*y)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(y*(z*z)*6.0E1-(y*y*y)*2.0E1));
  ep_s11[103] = lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)-1.0/(L*L*L*L*L)*mu*x*y*(z*z)*1.2E1;
  ep_s11[104] = lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0);
  ep_s11[105] = -lambda*(1.0/(L*L*L*L*L)*z*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(x*x-y*y)*2.0E1-1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)-1.0/(L*L*L*L*L)*mu*z*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)*2.0;
  ep_s11[106] = lambda*(1.0/(L*L*L*L*L)*(z*z)*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*2.0-1.0/(L*L*L*L*L)*(z*z)*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)+mu*(1.0/(L*L*L*L*L)*(z*z)*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0)*2.0;
  ep_s11[107] = lambda*(-1.0/(L*L*L*L*L)*z*((x*x)*y*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)+mu*(1.0/(L*L*L*L*L)*y*z*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)*2.0;

  ep_s12[4] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s12[6] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s12[12] = mu*(1.0/(L*L)*y-1.0/(L*L)*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
  ep_s12[13] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*-2.0;
  ep_s12[14] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s12[16] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
  ep_s12[18] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*2.0;
  ep_s12[19] = mu*(1.0/(L*L)*z*(1.0/2.0)+1.0/(L*L)*z*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s12[20] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
  ep_s12[24] = 1.0/(L*L)*mu*z*-2.0;
  ep_s12[27] = mu*(1.0/(L*L*L)*x*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
  ep_s12[29] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
  ep_s12[30] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(y*y-(z*z)*3.0)*(1.0/2.0))*-2.0;
  ep_s12[32] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
  ep_s12[33] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
  ep_s12[35] = mu*(1.0/(L*L*L)*y*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*y*6.0)*-2.0;
  ep_s12[37] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
  ep_s12[38] =  mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
  ep_s12[39] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(z*z)*3.0)*(1.0/2.0))*-2.0;
  ep_s12[40] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
  ep_s12[44] = 1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s12[46] = 1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s12[47] = 1.0/(L*L*L)*mu*(z*z)*-2.0;
  ep_s12[48] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)+1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*y*(y*2.0-nu*y*4.0))*2.0;
  ep_s12[49] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*1.2E1))*-2.0;
  ep_s12[50] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0))*2.0;
  ep_s12[52] = mu*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1-(y*y*y)*1.2E1-nu*(x*x)*y*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((x*x)*y*1.2E1-(y*y*y)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*1.2E1)*2.0;
  ep_s12[53] = mu*(1.0/(L*L*L*L)*(x*x)*z*3.0+1.0/(L*L*L*L)*z*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(x*x-y*y)*(3.0/2.0)+1.0/(L*L*L*L)*y*z*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
  ep_s12[54] = mu*(1.0/(L*L*L*L)*((x*x)*y*6.0-y*(z*z)*1.8E1-nu*(x*x)*y*2.4E1+nu*y*(z*z)*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*y*(x*x-z*z)*3.0)*2.0;
  ep_s12[55] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*3.0+1.0/(L*L*L*L)*z*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
  ep_s12[58] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(-1.0/2.0)+1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0))*2.0;
  ep_s12[59] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*1.2E1))*-2.0;
  ep_s12[60] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)-1.0/(L*L*L*L)*y*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)+1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*y*(x*2.0-nu*x*4.0))*-2.0;
  ep_s12[62] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*3.0+1.0/(L*L*L*L)*z*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
  ep_s12[63] = mu*(1.0/(L*L*L*L)*(x*(y*y)*6.0-x*(z*z)*1.8E1-nu*x*(y*y)*2.4E1+nu*x*(z*z)*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*x*(y*y-z*z)*3.0)*2.0;
  ep_s12[64] = mu*(1.0/(L*L*L*L)*(y*y)*z*3.0-1.0/(L*L*L*L)*z*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(x*x-y*y)*(3.0/2.0)+1.0/(L*L*L*L)*x*z*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
  ep_s12[65] = mu*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1-(x*x*x)*1.2E1-nu*x*(y*y)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*x*(y*y)*1.2E1+1.0/(L*L*L*L)*x*(x*x-(y*y)*3.0)*2.0)*-2.0;
  ep_s12[67] = 1.0/(L*L*L*L)*mu*x*(z*z)*-1.2E1;
  ep_s12[68] = 1.0/(L*L*L*L)*mu*y*(z*z)*-1.2E1;
  ep_s12[71] = mu*(1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(x*x-z*z)*(3.0/2.0))*-2.0;
  ep_s12[72] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
  ep_s12[73] = mu*(1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*z*(y*y-z*z)*(3.0/2.0))*-2.0;
  ep_s12[75] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*y*2.0E1+1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*-4.0E1+(y*y*y)*2.0E1+nu*(x*x)*y*4.0E1)+1.0/(L*L*L*L*L)*x*y*(x*x-y*y)*2.0E1)*-2.0;
  ep_s12[77] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*1.0E1+nu*(x*x*x*x)*2.0E1+nu*(y*y*y*y)*4.0+(x*x*x*x)*5.0-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*1.6E1+(x*x)*y*2.0E1-(y*y*y)*1.2E1-nu*(x*x)*y*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)*(1.0/2.0))*2.0;
  ep_s12[78] = mu*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1-(y*y*y)*1.2E1-nu*(x*x)*y*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-y*y)*2.0+1.0/(L*L*L*L*L)*(x*x)*y*z*1.2E1)*-2.0;
  ep_s12[79] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0+x*x*x*x-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(x*(z*z)*1.2E1-(x*x*x)*4.0)*(1.0/2.0))*2.0;
  ep_s12[81] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*z+1.0/(L*L*L*L*L)*x*z*(x*x-(y*y)*3.0)+1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)+1.0/(L*L*L*L*L)*x*y*z*(y*2.0-nu*y*4.0))*2.0;
  ep_s12[82] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*z+1.0/(L*L*L*L*L)*x*z*(x*x-z*z)+1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z))*2.0;
  ep_s12[83] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((y*y)*(z*z)*-1.0E1+y*y*y*y+(z*z*z*z)*5.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*(y*(z*z)*2.0E1-(y*y*y)*4.0)*(1.0/2.0))*2.0;
  ep_s12[84] = mu*(1.0/(L*L*L*L*L)*x*y*(y*y-(z*z)*3.0)*4.0-1.0/(L*L*L*L*L)*x*(nu*2.0-1.0)*(y*(z*z)*1.2E1-(y*y*y)*4.0))*2.0;
  ep_s12[85] = mu*(1.0/(L*L*L*L*L)*y*z*(y*y-z*z)*1.0E1-1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*(y*(z*z)*2.0E1-(y*y*y)*2.0E1)*(1.0/2.0))*2.0;
  ep_s12[87] = mu*(1.0/(L*L*L*L*L)*x*(y*y*y)*2.0E1+1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*-4.0E1+(x*x*x)*2.0E1+nu*x*(y*y)*4.0E1)-1.0/(L*L*L*L*L)*x*y*(x*x-y*y)*2.0E1)*-2.0;
  ep_s12[89] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0+y*y*y*y-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*(y*(z*z)*1.2E1-(y*y*y)*4.0)*(1.0/2.0))*2.0;
  ep_s12[90] = mu*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1-(x*x*x)*1.2E1-nu*x*(y*y)*4.8E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*z*(x*x-(y*y)*3.0)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*1.2E1)*2.0;
  ep_s12[91] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*1.0E1+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*2.0E1-(x*x*x*x)*3.0+(y*y*y*y)*5.0-nu*(x*x)*(y*y)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*1.6E1+x*(y*y)*2.0E1-(x*x*x)*1.2E1-nu*x*(y*y)*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*((x*x)*y*6.0E1-(y*y*y)*2.0E1)*(1.0/2.0))*2.0;
  ep_s12[92] = mu*(1.0/(L*L*L*L*L)*(y*y*y)*z+1.0/(L*L*L*L*L)*y*z*(y*y-z*z)+1.0/(L*L*L*L*L)*y*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z))*2.0;
  ep_s12[93] = mu*(1.0/(L*L*L*L*L)*(y*y*y)*z+1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)-1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-y*y)+1.0/(L*L*L*L*L)*x*y*z*(x*2.0-nu*x*4.0))*2.0;
  ep_s12[94] = mu*(1.0/(L*L*L*L*L)*x*z*(x*x-z*z)*1.0E1-1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*(x*(z*z)*2.0E1-(x*x*x)*2.0E1)*(1.0/2.0))*2.0;
  ep_s12[95] = mu*(1.0/(L*L*L*L*L)*x*y*(x*x-(z*z)*3.0)*4.0-1.0/(L*L*L*L*L)*y*(nu*2.0-1.0)*(x*(z*z)*1.2E1-(x*x*x)*4.0))*2.0;
  ep_s12[96] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((x*x)*(z*z)*-1.0E1+x*x*x*x+(z*z*z*z)*5.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*(x*(z*z)*2.0E1-(x*x*x)*4.0)*(1.0/2.0))*2.0;
  ep_s12[100] = mu*(1.0/(L*L*L*L*L)*z*(x*(z*z)*1.2E1-(x*x*x)*4.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*z*(x*x-(z*z)*3.0)*2.0)*-2.0;
  ep_s12[101] = mu*(1.0/(L*L*L*L*L)*z*(y*(z*z)*1.2E1-(y*y*y)*4.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*z*(y*y-(z*z)*3.0)*2.0)*-2.0;
  ep_s12[103] = mu*(1.0/(L*L*L*L*L)*(z*z)*(x*x-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(z*z)*((x*x)*3.0-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(x*x)*(z*z))*-2.0;
  ep_s12[104] = mu*(1.0/(L*L*L*L*L)*(z*z)*(y*y-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(z*z)*((y*y)*3.0-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(y*y)*(z*z))*-2.0;
  ep_s12[105] = mu*(1.0/(L*L*L*L*L)*z*((x*x)*y*6.0E1-(y*y*y)*2.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*y*z*(x*x-y*y)*1.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*2.0E1)*-2.0;
  ep_s12[106] = 1.0/(L*L*L*L*L)*mu*x*y*(z*z)*-4.8E1;
  ep_s12[107] = mu*(1.0/(L*L*L*L*L)*z*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*z*(x*x-y*y)*1.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*2.0E1)*-2.0;

  ep_s13[5] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s13[9] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s13[13] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*2.0;
  ep_s13[14] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s13[15] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*2.0-1.0))*-2.0;
  ep_s13[19] = 1.0/(L*L)*mu*y*-2.0;
  ep_s13[22] = mu*(1.0/(L*L)*x-1.0/(L*L)*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
  ep_s13[23] = mu*(1.0/(L*L)*x+1.0/(L*L)*x*(nu*4.0-3.0))*-2.0;
  ep_s13[24] = mu*(1.0/(L*L)*y*(1.0/2.0)+1.0/(L*L)*y*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s13[26] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
  ep_s13[28] = mu*(1.0/(L*L*L)*x*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
  ep_s13[30] = mu*(1.0/(L*L*L)*y*z*3.0+1.0/(L*L*L)*y*z*(nu*4.0-3.0)*3.0)*2.0;
  ep_s13[31] = mu*(1.0/(L*L*L)*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
  ep_s13[32] = mu*(1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((y*y)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
  ep_s13[33] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
  ep_s13[37] = 1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s13[39] = 1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s13[40] = 1.0/(L*L*L)*mu*(y*y)*-2.0;
  ep_s13[41] = mu*(1.0/(L*L*L)*z*(x*6.0-nu*x*1.2E1)-1.0/(L*L*L)*x*z*6.0)*-2.0;
  ep_s13[43] = mu*(1.0/(L*L*L)*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*x*(x*6.0-nu*x*8.0)*(1.0/2.0))*-2.0;
  ep_s13[44] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(x*x)*(nu*4.0-3.0)+1.0/(L*L*L)*(nu*4.0-3.0)*(x*x-(y*y)*3.0)*(1.0/2.0))*-2.0;
  ep_s13[46] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
  ep_s13[47] = mu*(1.0/(L*L*L)*y*z+1.0/(L*L*L)*y*z*(nu*2.0-1.0))*-2.0;
  ep_s13[49] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0))*2.0;
  ep_s13[50] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*1.2E1))*-2.0;
  ep_s13[51] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(-1.0/2.0)+1.0/(L*L*L*L)*x*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*z*(z*2.0-nu*z*4.0))*2.0;
  ep_s13[53] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*y*3.0+1.0/(L*L*L*L)*y*(nu*(x*x)*1.2E1-nu*(y*y)*4.0-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0))*2.0;
  ep_s13[54] = mu*(1.0/(L*L*L*L)*((x*x)*z*6.0-(y*y)*z*1.8E1-nu*(x*x)*z*2.4E1+nu*(y*y)*z*2.4E1)*(-1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*z*(x*x-y*y)*3.0)*2.0;
  ep_s13[55] = mu*(1.0/(L*L*L*L)*(x*x)*y*3.0+1.0/(L*L*L*L)*y*(nu*(x*x)*1.2E1-nu*(z*z)*4.0-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(x*x-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*y*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
  ep_s13[56] = mu*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1-(z*z*z)*1.2E1-nu*(x*x)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((x*x)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(x*x)*z*1.2E1)*2.0;
  ep_s13[58] = 1.0/(L*L*L*L)*mu*(y*y)*z*-1.2E1;
  ep_s13[59] = 1.0/(L*L*L*L)*mu*x*(y*y)*-1.2E1;
  ep_s13[62] = mu*(1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(y*y-z*z)*(3.0/2.0))*2.0;
  ep_s13[63] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
  ep_s13[64] = mu*(1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*(x*x-y*y)*(3.0/2.0))*-2.0;
  ep_s13[66] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)+1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*z*(x*2.0-nu*x*4.0))*-2.0;
  ep_s13[67] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*1.2E1))*-2.0;
  ep_s13[68] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0))*-2.0;
  ep_s13[70] = mu*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1-(x*x*x)*1.2E1-nu*x*(z*z)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*x*(z*z)*1.2E1+1.0/(L*L*L*L)*x*(x*x-(z*z)*3.0)*2.0)*-2.0;
  ep_s13[71] = mu*(1.0/(L*L*L*L)*y*(z*z)*3.0-1.0/(L*L*L*L)*y*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*(x*x-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*y*(x*6.0-nu*x*8.0)*(1.0/2.0))*2.0;
  ep_s13[72] = mu*(1.0/(L*L*L*L)*(x*(y*y)*1.8E1-x*(z*z)*6.0-nu*x*(y*y)*2.4E1+nu*x*(z*z)*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*x*(z*z)*6.0-1.0/(L*L*L*L)*x*(y*y-z*z)*3.0)*2.0;
  ep_s13[73] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*y*(z*z)*3.0-1.0/(L*L*L*L)*y*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
  ep_s13[76] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*z*2.0E1+1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*-4.0E1+(z*z*z)*2.0E1+nu*(x*x)*z*4.0E1)+1.0/(L*L*L*L*L)*x*z*(x*x-z*z)*2.0E1)*-2.0;
  ep_s13[78] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0+x*x*x*x-(y*y*y*y)*3.0-nu*(x*x)*(y*y)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(x*(y*y)*1.2E1-(x*x*x)*4.0)*(1.0/2.0))*2.0;
  ep_s13[79] = mu*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1-(z*z*z)*1.2E1-nu*(x*x)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-z*z)*2.0+1.0/(L*L*L*L*L)*(x*x)*y*z*1.2E1)*-2.0;
  ep_s13[80] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*1.0E1+nu*(x*x*x*x)*2.0E1+nu*(z*z*z*z)*4.0+(x*x*x*x)*5.0-(z*z*z*z)*3.0-nu*(x*x)*(z*z)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*1.6E1+(x*x)*z*2.0E1-(z*z*z)*1.2E1-nu*(x*x)*z*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)*(1.0/2.0))*2.0;
  ep_s13[81] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*y+1.0/(L*L*L*L*L)*x*y*(x*x-y*y)+1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y))*2.0;
  ep_s13[82] = mu*(1.0/(L*L*L*L*L)*(x*x*x)*y+1.0/(L*L*L*L*L)*x*y*(x*x-(z*z)*3.0)+1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L*L)*x*y*z*(z*2.0-nu*z*4.0))*2.0;
  ep_s13[83] = mu*(1.0/(L*L*L*L*L)*y*z*(y*y-z*z)*1.0E1+1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*((y*y)*z*2.0E1-(z*z*z)*2.0E1)*(1.0/2.0))*-2.0;
  ep_s13[84] = mu*(1.0/(L*L*L*L*L)*x*z*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*(nu*2.0-1.0)*((y*y)*z*1.2E1-(z*z*z)*4.0))*-2.0;
  ep_s13[85] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((y*y)*(z*z)*-1.0E1+(y*y*y*y)*5.0+z*z*z*z)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*((y*y)*z*2.0E1-(z*z*z)*4.0)*(1.0/2.0))*2.0;
  ep_s13[89] = mu*(1.0/(L*L*L*L*L)*y*((y*y)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*y*z*((y*y)*3.0-z*z)*2.0)*-2.0;
  ep_s13[90] = mu*(1.0/(L*L*L*L*L)*y*(x*(y*y)*1.2E1-(x*x*x)*4.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*y*(x*x-(y*y)*3.0)*2.0)*-2.0;
  ep_s13[92] = mu*(1.0/(L*L*L*L*L)*(y*y)*(y*y-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(y*y)*(y*y-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*(y*y)*(z*z))*2.0;
  ep_s13[93] = mu*(1.0/(L*L*L*L*L)*(y*y)*(x*x-y*y)*(1.0/2.0)+1.0/(L*L*L*L*L)*(y*y)*((x*x)*3.0-y*y)*(1.0/2.0)+1.0/(L*L*L*L*L)*(x*x)*(y*y))*-2.0;
  ep_s13[94] = mu*(1.0/(L*L*L*L*L)*y*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*y*(x*x-z*z)*1.0E1+1.0/(L*L*L*L*L)*x*y*(z*z)*2.0E1)*-2.0;
  ep_s13[95] = 1.0/(L*L*L*L*L)*mu*x*(y*y)*z*-4.8E1;
  ep_s13[96] = mu*(1.0/(L*L*L*L*L)*y*((x*x)*z*6.0E1-(z*z*z)*2.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*y*z*(x*x-z*z)*1.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*2.0E1)*-2.0;
  ep_s13[97] = mu*(1.0/(L*L*L*L*L)*x*(z*z*z)*2.0E1+1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*-4.0E1+(x*x*x)*2.0E1+nu*x*(z*z)*4.0E1)-1.0/(L*L*L*L*L)*x*z*(x*x-z*z)*2.0E1)*-2.0;
  ep_s13[99] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*1.0E1+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*2.0E1-(x*x*x*x)*3.0+(z*z*z*z)*5.0-nu*(x*x)*(z*z)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*1.6E1+x*(z*z)*2.0E1-(x*x*x)*1.2E1-nu*x*(z*z)*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*((x*x)*z*6.0E1-(z*z*z)*2.0E1)*(1.0/2.0))*2.0;
  ep_s13[100] = mu*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1-(x*x*x)*1.2E1-nu*x*(z*z)*4.8E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*y*(x*x-(z*z)*3.0)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*1.2E1)*2.0;
  ep_s13[101] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*((y*y)*(z*z)*6.0+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*4.0-(y*y*y*y)*3.0+z*z*z*z-nu*(y*y)*(z*z)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*((y*y)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0))*2.0;
  ep_s13[103] = mu*(1.0/(L*L*L*L*L)*y*(z*z*z)+1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)-1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-z*z)+1.0/(L*L*L*L*L)*x*y*z*(x*2.0-nu*x*4.0))*2.0;
  ep_s13[104] = mu*(1.0/(L*L*L*L*L)*y*(z*z*z)-1.0/(L*L*L*L*L)*y*z*(y*y-z*z)+1.0/(L*L*L*L*L)*y*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y))*2.0;
  ep_s13[105] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((x*x)*(y*y)*-1.0E1+x*x*x*x+(y*y*y*y)*5.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*(x*(y*y)*2.0E1-(x*x*x)*4.0)*(1.0/2.0))*2.0;
  ep_s13[106] = mu*(1.0/(L*L*L*L*L)*x*z*(x*x-(y*y)*3.0)*4.0-1.0/(L*L*L*L*L)*z*(nu*2.0-1.0)*(x*(y*y)*1.2E1-(x*x*x)*4.0))*2.0;
  ep_s13[107] = mu*(1.0/(L*L*L*L*L)*x*y*(x*x-y*y)*1.0E1-1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*(x*(y*y)*2.0E1-(x*x*x)*2.0E1)*(1.0/2.0))*2.0;

  ep_s22[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s22[7] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
  ep_s22[11] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s22[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0))+1.0/(L*L)*mu*x*4.0;
  ep_s22[13] = 1.0/(L*L)*mu*x*-4.0;
  ep_s22[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
  ep_s22[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
  ep_s22[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*(y*2.0-nu*y*8.0)*2.0;
  ep_s22[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
  ep_s22[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*z*(nu*2.0-1.0)*4.0;
  ep_s22[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0));
  ep_s22[23] = 1.0/(L*L)*mu*z*4.0;
  ep_s22[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
  ep_s22[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
  ep_s22[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
  ep_s22[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
  ep_s22[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s22[30] = 1.0/(L*L*L)*mu*x*y*-1.2E1;
  ep_s22[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0);
  ep_s22[32] = 1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s22[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
  ep_s22[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
  ep_s22[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)*2.0;
  ep_s22[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*z*(y*6.0-nu*y*2.4E1)*2.0;
  ep_s22[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0)-1.0/(L*L*L)*mu*x*(y*6.0-nu*y*2.4E1)*2.0;
  ep_s22[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*z*(nu*2.0-1.0)*4.0;
  ep_s22[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0);
  ep_s22[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)+1.0/(L*L*L)*mu*(z*z)*1.2E1;
  ep_s22[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0);
  ep_s22[44] = 1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s22[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s22[46] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
  ep_s22[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0;
  ep_s22[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)-1.0/(L*L*L*L)*mu*(x*x)*y*1.2E1;
  ep_s22[49] = lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)-1.0/(L*L*L*L)*mu*(x*x)*z*1.2E1;
  ep_s22[50] = lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
  ep_s22[51] = lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
  ep_s22[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1))+1.0/(L*L*L*L)*mu*x*((x*x)*1.2E1-(y*y)*1.2E1)*2.0;
  ep_s22[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s22[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)+1.0/(L*L*L*L)*mu*x*(x*x-z*z)*1.2E1;
  ep_s22[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1));
  ep_s22[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1));
  ep_s22[57] = mu*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
  ep_s22[58] = mu*(1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
  ep_s22[59] = mu*(1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)*2.0+lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
  ep_s22[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)+mu*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)*2.0;
  ep_s22[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1))-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)*2.0;
  ep_s22[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*z*(y*6.0-nu*y*2.4E1)*2.0;
  ep_s22[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)*2.0;
  ep_s22[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*z*(y*6.0-nu*y*2.4E1)*2.0;
  ep_s22[65] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)*2.0;
  ep_s22[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
  ep_s22[67] = lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
  ep_s22[68] = lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)-1.0/(L*L*L*L)*mu*x*(z*z)*1.2E1;
  ep_s22[69] = lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)-1.0/(L*L*L*L)*mu*y*(z*z)*1.2E1;
  ep_s22[70] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0);
  ep_s22[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1));
  ep_s22[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*z*(x*x-z*z)*1.2E1;
  ep_s22[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s22[74] = mu*(1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0)*-2.0-lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0);
  ep_s22[75] = mu*(1.0/(L*L*L*L*L)*(x*x)*(x*x-y*y)*2.0E1-1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1)*-2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(y*y*y*y)*1.0E1+x*x*x*x-(y*y*y*y)*5.0-nu*(x*x)*(y*y)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(y*y)*4.0E1)*2.0);
  ep_s22[76] = lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(z*z*z*z)*1.0E1+x*x*x*x-(z*z*z*z)*5.0-nu*(x*x)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(z*z)*4.0E1)*2.0);
  ep_s22[77] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*8.0E1+x*(y*y)*2.0E1+(x*x*x)*2.0E1-nu*x*(y*y)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*y*6.0E1-(y*y*y)*2.0E1))-1.0/(L*L*L*L*L)*mu*x*((x*x)*y*6.0E1-(y*y*y)*2.0E1)*2.0;
  ep_s22[78] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L*L)*x*z*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)-mu*(1.0/(L*L*L*L*L)*x*z*((x*x)*3.0-y*y)*4.0-1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)*2.0;
  ep_s22[79] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0);
  ep_s22[80] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*8.0E1+x*(z*z)*2.0E1+(x*x*x)*2.0E1-nu*x*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*z*6.0E1-(z*z*z)*2.0E1));
  ep_s22[81] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)-1.0/(L*L*L*L*L)*mu*(x*x)*y*z*1.2E1;
  ep_s22[82] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0);
  ep_s22[83] = -lambda*(1.0/(L*L*L*L*L)*x*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(y*y-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)-1.0/(L*L*L*L*L)*mu*x*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)*2.0;
  ep_s22[84] = lambda*(1.0/(L*L*L*L*L)*(x*x)*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0)+mu*(1.0/(L*L*L*L*L)*(x*x)*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0)*2.0;
  ep_s22[85] = lambda*(-1.0/(L*L*L*L*L)*x*((y*y)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)+mu*(1.0/(L*L*L*L*L)*x*z*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)*2.0;
  ep_s22[86] = mu*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*2.0+nu*(z*z*z*z)*1.0E1+y*y*y*y-(z*z*z*z)*5.0-nu*(y*y)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*y*(z*z)*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*2.0+nu*(z*z*z*z)*1.0E1+y*y*y*y-(z*z*z*z)*5.0-nu*(y*y)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(y*y)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*y*(z*z)*4.0E1)*2.0);
  ep_s22[87] = mu*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(y*y*y*y)*2.0-(x*x*x*x)*5.0+y*y*y*y-nu*(x*x)*(y*y)*2.0E1)*2.0+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*(x*x)*y*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(y*y*y*y)*2.0-(x*x*x*x)*5.0+y*y*y*y-nu*(x*x)*(y*y)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(y*y)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*(x*x)*y*4.0E1)*2.0);
  ep_s22[88] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*8.0E1+y*(z*z)*2.0E1+(y*y*y)*2.0E1-nu*y*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*y*((y*y)*z*6.0E1-(z*z*z)*2.0E1))+1.0/(L*L*L*L*L)*mu*z*(nu*(y*y*y)*8.0E1+y*(z*z)*2.0E1+(y*y*y)*2.0E1-nu*y*(z*z)*8.0E1)*2.0;
  ep_s22[89] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)+1.0/(L*L*L*L*L)*mu*x*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)*2.0;
  ep_s22[90] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0)+1.0/(L*L*L*L*L)*mu*z*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)*2.0;
  ep_s22[91] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*8.0E1+(x*x)*y*2.0E1+(y*y*y)*2.0E1-nu*(x*x)*y*8.0E1)-1.0/(L*L*L*L*L)*y*(x*(y*y)*6.0E1-(x*x*x)*2.0E1))+1.0/(L*L*L*L*L)*mu*x*(nu*(y*y*y)*8.0E1+(x*x)*y*2.0E1+(y*y*y)*2.0E1-nu*(x*x)*y*8.0E1)*2.0;
  ep_s22[92] = lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)+mu*(1.0/(L*L*L*L*L)*x*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)*2.0;
  ep_s22[93] = lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)+mu*(1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)*2.0;
  ep_s22[94] = lambda*(-1.0/(L*L*L*L*L)*y*((x*x)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1);
  ep_s22[95] = lambda*(1.0/(L*L*L*L*L)*(y*y)*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(y*y)*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)+1.0/(L*L*L*L*L)*mu*(nu*2.0-1.0)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*4.0;
  ep_s22[96] = -lambda*(1.0/(L*L*L*L*L)*y*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(x*x-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1);
  ep_s22[97] = lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(z*z*z*z)*2.0-(x*x*x*x)*5.0+z*z*z*z-nu*(x*x)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(x*x)*z*4.0E1)*2.0);
  ep_s22[98] = mu*(1.0/(L*L*L*L*L)*(z*z)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*1.0E1+nu*(z*z*z*z)*2.0-(y*y*y*y)*5.0+z*z*z*z-nu*(y*y)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(y*y)*z*4.0E1)*2.0);
  ep_s22[99] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*8.0E1+(x*x)*z*2.0E1+(z*z*z)*2.0E1-nu*(x*x)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(x*(z*z)*6.0E1-(x*x*x)*2.0E1));
  ep_s22[100] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0);
  ep_s22[101] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L*L)*x*z*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)+mu*(1.0/(L*L*L*L*L)*x*z*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)*2.0;
  ep_s22[102] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*8.0E1+(y*y)*z*2.0E1+(z*z*z)*2.0E1-nu*(y*y)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(y*(z*z)*6.0E1-(y*y*y)*2.0E1))-1.0/(L*L*L*L*L)*mu*z*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)*2.0;
  ep_s22[103] = lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0);
  ep_s22[104] = lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)-1.0/(L*L*L*L*L)*mu*x*y*(z*z)*1.2E1;
  ep_s22[105] = -lambda*(1.0/(L*L*L*L*L)*z*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(x*x-y*y)*2.0E1-1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)-mu*(1.0/(L*L*L*L*L)*x*z*(x*x-y*y)*2.0E1-1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)*2.0;
  ep_s22[106] = lambda*(1.0/(L*L*L*L*L)*(z*z)*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*2.0-1.0/(L*L*L*L*L)*(z*z)*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)-mu*(1.0/(L*L*L*L*L)*(z*z)*((x*x)*3.0-y*y)*4.0-1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)*2.0;
  ep_s22[107] = lambda*(-1.0/(L*L*L*L*L)*z*((x*x)*y*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)-1.0/(L*L*L*L*L)*mu*z*((x*x)*y*6.0E1-(y*y*y)*2.0E1)*2.0;

  ep_s23[8] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s23[10] = mu*((nu*2.0-3.0/2.0)/L+(1.0/2.0)/L)*-2.0;
  ep_s23[14] = 1.0/(L*L)*mu*x*-2.0;
  ep_s23[17] = mu*(1.0/(L*L)*z-1.0/(L*L)*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
  ep_s23[18] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*4.0-3.0))*-2.0;
  ep_s23[19] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s23[21] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*2.0-1.0))*-2.0;
  ep_s23[23] = mu*(1.0/(L*L)*y+1.0/(L*L)*y*(nu*4.0-3.0))*2.0;
  ep_s23[24] = mu*(1.0/(L*L)*x*(1.0/2.0)+1.0/(L*L)*x*(nu*4.0-3.0)*(1.0/2.0))*-2.0;
  ep_s23[25] = mu*(1.0/(L*L)*z+1.0/(L*L)*z*(nu*2.0-1.0))*-2.0;
  ep_s23[30] = 1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s23[32] = 1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s23[33] = 1.0/(L*L*L)*mu*(x*x)*-2.0;
  ep_s23[34] = mu*(1.0/(L*L*L)*y*(z*6.0-nu*z*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
  ep_s23[36] = mu*(1.0/(L*L*L)*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(y*y)*3.0+1.0/(L*L*L)*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
  ep_s23[37] = mu*(1.0/(L*L*L)*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-z*z)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*(nu*4.0-3.0))*2.0;
  ep_s23[39] = mu*(1.0/(L*L*L)*x*z*3.0+1.0/(L*L*L)*x*z*(nu*4.0-3.0)*3.0)*2.0;
  ep_s23[40] = mu*(1.0/(L*L*L)*x*y+1.0/(L*L*L)*x*y*(nu*2.0-1.0))*-2.0;
  ep_s23[42] = mu*(1.0/(L*L*L)*z*(y*6.0-nu*y*1.2E1)-1.0/(L*L*L)*y*z*6.0)*-2.0;
  ep_s23[44] = mu*(1.0/(L*L*L)*x*y*3.0+1.0/(L*L*L)*x*y*(nu*4.0-3.0)*3.0)*2.0;
  ep_s23[45] = mu*(1.0/(L*L*L)*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L)*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L)*(z*z)*3.0-1.0/(L*L*L)*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*-2.0;
  ep_s23[46] = mu*(1.0/(L*L*L)*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L)*(nu*4.0-3.0)*((x*x)*3.0-y*y)*(1.0/2.0)-1.0/(L*L*L)*(y*y)*(nu*4.0-3.0))*2.0;
  ep_s23[47] = mu*(1.0/(L*L*L)*x*z+1.0/(L*L*L)*x*z*(nu*2.0-1.0))*-2.0;
  ep_s23[49] = 1.0/(L*L*L*L)*mu*(x*x)*y*-1.2E1;
  ep_s23[50] = 1.0/(L*L*L*L)*mu*(x*x)*z*-1.2E1;
  ep_s23[53] = mu*(1.0/(L*L*L*L)*x*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(x*x-y*y)*(3.0/2.0))*2.0;
  ep_s23[54] = 1.0/(L*L*L*L)*mu*x*y*z*-2.4E1;
  ep_s23[55] = mu*(1.0/(L*L*L*L)*x*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(x*x-z*z)*(3.0/2.0))*2.0;
  ep_s23[57] = mu*(1.0/(L*L*L*L)*(y*(z*z)*3.0-y*y*y)*(-1.0/2.0)+1.0/(L*L*L*L)*y*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L)*y*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*y*z*(z*2.0-nu*z*4.0))*2.0;
  ep_s23[58] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*1.2E1))*-2.0;
  ep_s23[59] = mu*(1.0/(L*L*L*L)*((x*x)*y*3.0-y*y*y)*(1.0/2.0)+1.0/(L*L*L*L)*y*((x*x)*3.0-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0))*-2.0;
  ep_s23[61] = mu*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1-(z*z*z)*1.2E1-nu*(y*y)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L)*((y*y)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*1.2E1)*2.0;
  ep_s23[62] = mu*(1.0/(L*L*L*L)*x*(y*y)*3.0+1.0/(L*L*L*L)*x*(nu*(y*y)*1.2E1-nu*(z*z)*4.0-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)+1.0/(L*L*L*L)*x*(y*y-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*z*(z*6.0-nu*z*8.0)*(1.0/2.0))*2.0;
  ep_s23[63] = mu*(1.0/(L*L*L*L)*((x*x)*z*1.8E1-(y*y)*z*6.0-nu*(x*x)*z*2.4E1+nu*(y*y)*z*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*(y*y)*z*6.0-1.0/(L*L*L*L)*z*(x*x-y*y)*3.0)*2.0;
  ep_s23[64] = mu*(1.0/(L*L*L*L)*(x*(y*y)*3.0-x*x*x)*(1.0/2.0)+1.0/(L*L*L*L)*x*(y*y)*3.0-1.0/(L*L*L*L)*x*(nu*(x*x)*4.0-nu*(y*y)*1.2E1-(x*x)*3.0+(y*y)*3.0)*(1.0/2.0))*2.0;
  ep_s23[67] = mu*(1.0/(L*L*L*L)*((x*x)*z*3.0-z*z*z)*(1.0/2.0)+1.0/(L*L*L*L)*z*((x*x)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0))*-2.0;
  ep_s23[68] = mu*(1.0/(L*L*L*L)*x*y*z*6.0-1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*1.2E1))*-2.0;
  ep_s23[69] = mu*(1.0/(L*L*L*L)*((y*y)*z*3.0-z*z*z)*(1.0/2.0)-1.0/(L*L*L*L)*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)+1.0/(L*L*L*L)*z*((y*y)*3.0-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*y*z*(y*2.0-nu*y*4.0))*-2.0;
  ep_s23[71] = mu*(1.0/(L*L*L*L)*(x*(z*z)*3.0-x*x*x)*(1.0/2.0)+1.0/(L*L*L*L)*x*(z*z)*3.0-1.0/(L*L*L*L)*x*(nu*(x*x)*4.0-nu*(z*z)*1.2E1-(x*x)*3.0+(z*z)*3.0)*(1.0/2.0))*2.0;
  ep_s23[72] = mu*(1.0/(L*L*L*L)*((x*x)*y*1.8E1-y*(z*z)*6.0-nu*(x*x)*y*2.4E1+nu*y*(z*z)*2.4E1)*(1.0/2.0)+1.0/(L*L*L*L)*y*(z*z)*6.0-1.0/(L*L*L*L)*y*(x*x-z*z)*3.0)*2.0;
  ep_s23[73] = mu*(1.0/(L*L*L*L)*x*(z*z)*3.0-1.0/(L*L*L*L)*x*(nu*(y*y)*4.0-nu*(z*z)*1.2E1-(y*y)*3.0+(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L)*x*(y*y-z*z)*(3.0/2.0)+1.0/(L*L*L*L)*x*y*(y*6.0-nu*y*8.0)*(1.0/2.0))*2.0;
  ep_s23[74] = mu*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1-(y*y*y)*1.2E1-nu*y*(z*z)*4.8E1)*(1.0/2.0)-1.0/(L*L*L*L)*y*(z*z)*1.2E1+1.0/(L*L*L*L)*y*(y*y-(z*z)*3.0)*2.0)*-2.0;
  ep_s23[78] = mu*(1.0/(L*L*L*L*L)*x*((x*x)*y*1.2E1-(y*y*y)*4.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*y*((x*x)*3.0-y*y)*2.0)*-2.0;
  ep_s23[79] = mu*(1.0/(L*L*L*L*L)*x*((x*x)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*z*((x*x)*3.0-z*z)*2.0)*-2.0;
  ep_s23[81] = mu*(1.0/(L*L*L*L*L)*(x*x)*(x*x-y*y)*(1.0/2.0)+1.0/(L*L*L*L*L)*(x*x)*(x*x-(y*y)*3.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*(x*x)*(y*y))*2.0;
  ep_s23[82] = mu*(1.0/(L*L*L*L*L)*(x*x)*(x*x-z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*(x*x)*(x*x-(z*z)*3.0)*(1.0/2.0)-1.0/(L*L*L*L*L)*(x*x)*(z*z))*2.0;
  ep_s23[83] = mu*(1.0/(L*L*L*L*L)*x*((y*y)*z*6.0E1-(z*z*z)*2.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*z*(y*y-z*z)*1.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*2.0E1)*-2.0;
  ep_s23[84] = 1.0/(L*L*L*L*L)*mu*(x*x)*y*z*-4.8E1;
  ep_s23[85] = mu*(1.0/(L*L*L*L*L)*x*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*x*y*(y*y-z*z)*1.0E1+1.0/(L*L*L*L*L)*x*y*(z*z)*2.0E1)*-2.0;
  ep_s23[86] = mu*(1.0/(L*L*L*L*L)*(y*y*y)*z*2.0E1+1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*-4.0E1+(z*z*z)*2.0E1+nu*(y*y)*z*4.0E1)+1.0/(L*L*L*L*L)*y*z*(y*y-z*z)*2.0E1)*-2.0;
  ep_s23[88] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*1.0E1+nu*(y*y*y*y)*2.0E1+nu*(z*z*z*z)*4.0+(y*y*y*y)*5.0-(z*z*z*z)*3.0-nu*(y*y)*(z*z)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*1.6E1+(y*y)*z*2.0E1-(z*z*z)*1.2E1-nu*(y*y)*z*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)*(1.0/2.0))*2.0;
  ep_s23[89] = mu*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1-(z*z*z)*1.2E1-nu*(y*y)*z*4.8E1)*(-1.0/2.0)+1.0/(L*L*L*L*L)*x*z*((y*y)*3.0-z*z)*2.0+1.0/(L*L*L*L*L)*x*(y*y)*z*1.2E1)*-2.0;
  ep_s23[90] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(y*y)*6.0+nu*(x*x*x*x)*4.0+nu*(y*y*y*y)*4.0-(x*x*x*x)*3.0+y*y*y*y-nu*(x*x)*(y*y)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*((x*x)*y*1.2E1-(y*y*y)*4.0)*(1.0/2.0))*2.0;
  ep_s23[92] = mu*(1.0/(L*L*L*L*L)*x*(y*y*y)+1.0/(L*L*L*L*L)*x*y*(y*y-(z*z)*3.0)+1.0/(L*L*L*L*L)*x*y*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)+1.0/(L*L*L*L*L)*x*y*z*(z*2.0-nu*z*4.0))*2.0;
  ep_s23[93] = mu*(1.0/(L*L*L*L*L)*x*(y*y*y)-1.0/(L*L*L*L*L)*x*y*(x*x-y*y)+1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x))*2.0;
  ep_s23[94] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*-3.0E1+(x*x*x*x)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((x*x)*(z*z)*-1.0E1+(x*x*x*x)*5.0+z*z*z*z)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*(nu*4.0-3.0)*((x*x)*z*2.0E1-(z*z*z)*4.0)*(1.0/2.0))*2.0;
  ep_s23[95] = mu*(1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*y*(nu*2.0-1.0)*((x*x)*z*1.2E1-(z*z*z)*4.0))*-2.0;
  ep_s23[96] = mu*(1.0/(L*L*L*L*L)*x*z*(x*x-z*z)*1.0E1+1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*((x*x)*z*2.0E1-(z*z*z)*2.0E1)*(1.0/2.0))*-2.0;
  ep_s23[98] = mu*(1.0/(L*L*L*L*L)*y*(z*z*z)*2.0E1+1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*-4.0E1+(y*y*y)*2.0E1+nu*y*(z*z)*4.0E1)-1.0/(L*L*L*L*L)*y*z*(y*y-z*z)*2.0E1)*-2.0;
  ep_s23[100] = mu*(1.0/(L*L*L*L*L)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*(1.0/2.0)+1.0/(L*L*L*L*L)*((x*x)*(z*z)*6.0+nu*(x*x*x*x)*4.0+nu*(z*z*z*z)*4.0-(x*x*x*x)*3.0+z*z*z*z-nu*(x*x)*(z*z)*2.4E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*((x*x)*z*1.2E1-(z*z*z)*4.0)*(1.0/2.0))*2.0;
  ep_s23[101] = mu*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1-(y*y*y)*1.2E1-nu*y*(z*z)*4.8E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*x*y*(y*y-(z*z)*3.0)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*1.2E1)*2.0;
  ep_s23[102] = mu*(1.0/(L*L*L*L*L)*((y*y)*(z*z)*1.0E1+nu*(y*y*y*y)*4.0+nu*(z*z*z*z)*2.0E1-(y*y*y*y)*3.0+(z*z*z*z)*5.0-nu*(y*y)*(z*z)*4.0E1)*(1.0/2.0)+1.0/(L*L*L*L*L)*((y*y)*(z*z)*-3.0E1+(y*y*y*y)*5.0+(z*z*z*z)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*1.6E1+y*(z*z)*2.0E1-(y*y*y)*1.2E1-nu*y*(z*z)*8.0E1)*(1.0/2.0)-1.0/(L*L*L*L*L)*z*((y*y)*z*6.0E1-(z*z*z)*2.0E1)*(1.0/2.0))*2.0;
  ep_s23[103] = mu*(1.0/(L*L*L*L*L)*x*(z*z*z)-1.0/(L*L*L*L*L)*x*z*(x*x-z*z)+1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x))*2.0;
  ep_s23[104] = mu*(1.0/(L*L*L*L*L)*x*(z*z*z)+1.0/(L*L*L*L*L)*x*z*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)-1.0/(L*L*L*L*L)*x*z*((y*y)*3.0-z*z)+1.0/(L*L*L*L*L)*x*y*z*(y*2.0-nu*y*4.0))*2.0;
  ep_s23[105] = mu*(1.0/(L*L*L*L*L)*x*y*(x*x-y*y)*1.0E1+1.0/(L*L*L*L*L)*x*(nu*4.0-3.0)*((x*x)*y*2.0E1-(y*y*y)*2.0E1)*(1.0/2.0))*-2.0;
  ep_s23[106] = mu*(1.0/(L*L*L*L*L)*y*z*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*z*(nu*2.0-1.0)*((x*x)*y*1.2E1-(y*y*y)*4.0))*-2.0;
  ep_s23[107] = mu*(1.0/(L*L*L*L*L)*((x*x)*(y*y)*-3.0E1+(x*x*x*x)*5.0+(y*y*y*y)*5.0)*(1.0/2.0)+1.0/(L*L*L*L*L)*(nu*4.0-3.0)*((x*x)*(y*y)*-1.0E1+(x*x*x*x)*5.0+y*y*y*y)*(1.0/2.0)-1.0/(L*L*L*L*L)*y*(nu*4.0-3.0)*((x*x)*y*2.0E1-(y*y*y)*4.0)*(1.0/2.0))*2.0;

  ep_s33[3] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s33[7] = (lambda*(nu*2.0-1.0)*-2.0)/L;
  ep_s33[11] = (lambda*(nu*2.0-1.0)*-2.0)/L-(mu*(nu*2.0-1.0)*4.0)/L;
  ep_s33[12] = lambda*(1.0/(L*L)*x*2.0+1.0/(L*L)*(x*2.0-nu*x*8.0));
  ep_s33[13] = 1.0/(L*L)*mu*x*4.0;
  ep_s33[15] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
  ep_s33[16] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0;
  ep_s33[17] = lambda*(1.0/(L*L)*y*2.0+1.0/(L*L)*(y*2.0-nu*y*8.0))+1.0/(L*L)*mu*y*4.0;
  ep_s33[18] = 1.0/(L*L)*mu*y*-4.0;
  ep_s33[20] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0;
  ep_s33[21] = 1.0/(L*L)*lambda*z*(nu*2.0-1.0)*-2.0;
  ep_s33[22] = lambda*(1.0/(L*L)*z*2.0+1.0/(L*L)*(z*2.0-nu*z*8.0))+1.0/(L*L)*mu*(z*2.0-nu*z*8.0)*2.0;
  ep_s33[25] = 1.0/(L*L)*lambda*y*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*y*(nu*2.0-1.0)*4.0;
  ep_s33[26] = 1.0/(L*L)*lambda*x*(nu*2.0-1.0)*-2.0-1.0/(L*L)*mu*x*(nu*2.0-1.0)*4.0;
  ep_s33[27] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0);
  ep_s33[28] = -lambda*(1.0/(L*L*L)*(x*x)*-6.0+1.0/(L*L*L)*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(x*x)*8.0)+1.0/(L*L*L)*mu*(x*x)*1.2E1;
  ep_s33[29] = -lambda*(1.0/(L*L*L)*y*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*y*6.0);
  ep_s33[30] = 1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s33[31] = -lambda*(1.0/(L*L*L)*z*(x*6.0-nu*x*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*z*1.2E1;
  ep_s33[32] = 1.0/(L*L*L)*mu*x*z*-1.2E1;
  ep_s33[33] = 1.0/(L*L*L)*lambda*y*z*(nu*2.0-1.0)*-2.0;
  ep_s33[34] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0)+1.0/(L*L*L)*mu*(y*y)*1.2E1;
  ep_s33[35] = -lambda*(1.0/(L*L*L)*(y*y)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(y*y)*8.0);
  ep_s33[36] = -lambda*(1.0/(L*L*L)*z*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*z*1.2E1;
  ep_s33[37] = 1.0/(L*L*L)*mu*y*z*-1.2E1;
  ep_s33[38] = -lambda*(1.0/(L*L*L)*x*(y*6.0-nu*y*2.4E1)+1.0/(L*L*L)*x*y*6.0);
  ep_s33[39] = 1.0/(L*L*L)*mu*x*y*1.2E1;
  ep_s33[40] = 1.0/(L*L*L)*lambda*x*z*(nu*2.0-1.0)*-2.0;
  ep_s33[41] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
  ep_s33[42] = -lambda*(1.0/(L*L*L)*(z*z)*-6.0+1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)-mu*(1.0/(L*L*L)*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L)*nu*(z*z)*8.0)*2.0;
  ep_s33[43] = -lambda*(1.0/(L*L*L)*x*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*x*z*6.0)-1.0/(L*L*L)*mu*x*(z*6.0-nu*z*2.4E1)*2.0;
  ep_s33[45] = -lambda*(1.0/(L*L*L)*y*(z*6.0-nu*z*2.4E1)+1.0/(L*L*L)*y*z*6.0)-1.0/(L*L*L)*mu*y*(z*6.0-nu*z*2.4E1)*2.0;
  ep_s33[47] = 1.0/(L*L*L)*lambda*x*y*(nu*2.0-1.0)*-2.0-1.0/(L*L*L)*mu*x*y*(nu*2.0-1.0)*4.0;
  ep_s33[48] = lambda*(1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L)*(x*x)*y*6.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0);
  ep_s33[49] = lambda*(1.0/(L*L*L*L)*(x*x)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(y*y)*6.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0);
  ep_s33[50] = lambda*(1.0/(L*L*L*L)*(x*x)*y*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*(x*x)*y*8.0)-1.0/(L*L*L*L)*mu*(x*x)*y*1.2E1;
  ep_s33[51] = lambda*(1.0/(L*L*L*L)*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(x*x)*z*6.0+1.0/(L*L*L*L)*nu*(x*x)*z*8.0)-1.0/(L*L*L*L)*mu*(x*x)*z*1.2E1;
  ep_s33[52] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(y*y)*1.2E1));
  ep_s33[53] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1));
  ep_s33[54] = lambda*(-1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(y*y)*6.0+x*(z*z)*6.0+(x*x*x)*4.0-nu*x*(y*y)*2.4E1-nu*x*(z*z)*2.4E1)+1.0/(L*L*L*L)*x*(x*x-y*y)*6.0+1.0/(L*L*L*L)*x*(x*x-z*z)*6.0)+1.0/(L*L*L*L)*mu*x*(x*x-y*y)*1.2E1;
  ep_s33[55] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*y*z*(x*6.0-nu*x*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s33[56] = -lambda*(1.0/(L*L*L*L)*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L)*x*((x*x)*1.2E1-(z*z)*1.2E1))+1.0/(L*L*L*L)*mu*x*((x*x)*1.2E1-(z*z)*1.2E1)*2.0;
  ep_s33[57] = lambda*(1.0/(L*L*L*L)*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L)*(y*y)*z*6.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0)-1.0/(L*L*L*L)*mu*(y*y)*z*1.2E1;
  ep_s33[58] = lambda*(1.0/(L*L*L*L)*x*(y*y)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*2.0-nu*(z*z)*6.0+(z*z)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0)-1.0/(L*L*L*L)*mu*x*(y*y)*1.2E1;
  ep_s33[59] = lambda*(1.0/(L*L*L*L)*(y*y)*z*-6.0+1.0/(L*L*L*L)*z*(nu*(x*x)*-6.0+nu*(y*y)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*(y*y)*z*8.0);
  ep_s33[60] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(y*y)*6.0+1.0/(L*L*L*L)*nu*x*(y*y)*8.0);
  ep_s33[61] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L)*y*((y*y)*1.2E1-(z*z)*1.2E1))+1.0/(L*L*L*L)*mu*y*((y*y)*1.2E1-(z*z)*1.2E1)*2.0;
  ep_s33[62] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1))-1.0/(L*L*L*L)*mu*x*y*z*1.2E1;
  ep_s33[63] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*6.0+y*(z*z)*6.0+(y*y*y)*4.0-nu*(x*x)*y*2.4E1-nu*y*(z*z)*2.4E1)+1.0/(L*L*L*L)*y*(x*x-y*y)*6.0-1.0/(L*L*L*L)*y*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*y*(x*x-y*y)*1.2E1;
  ep_s33[64] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*z*(y*6.0-nu*y*2.4E1));
  ep_s33[65] = -lambda*(1.0/(L*L*L*L)*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L)*(x*x)*y*8.0+1.0/(L*L*L*L)*y*(x*x-(y*y)*3.0)*4.0);
  ep_s33[66] = lambda*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L)*x*(z*z)*6.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)+mu*(1.0/(L*L*L*L)*x*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)*2.0;
  ep_s33[67] = mu*(1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*y*(z*z)*-6.0+1.0/(L*L*L*L)*y*(nu*(x*x)*-6.0+nu*(z*z)*2.0+(x*x)*3.0)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
  ep_s33[68] = mu*(1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*x*(z*z)*-6.0+1.0/(L*L*L*L)*x*(nu*(y*y)*-6.0+nu*(z*z)*2.0+(y*y)*3.0)*2.0+1.0/(L*L*L*L)*nu*x*(z*z)*8.0);
  ep_s33[69] = mu*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0)*2.0+lambda*(1.0/(L*L*L*L)*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L)*y*(z*z)*6.0+1.0/(L*L*L*L)*nu*y*(z*z)*8.0);
  ep_s33[70] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L)*(x*x)*z*8.0+1.0/(L*L*L*L)*z*(x*x-(z*z)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)*2.0;
  ep_s33[71] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*(z*6.0-nu*z*2.4E1)*2.0;
  ep_s33[72] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)+1.0/(L*L*L*L)*z*(x*x-z*z)*6.0+1.0/(L*L*L*L)*z*(y*y-z*z)*6.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(x*x)*z*6.0+(y*y)*z*6.0+(z*z*z)*4.0-nu*(x*x)*z*2.4E1-nu*(y*y)*z*2.4E1)*2.0;
  ep_s33[73] = -lambda*(1.0/(L*L*L*L)*x*y*z*6.0+1.0/(L*L*L*L)*x*y*(z*6.0-nu*z*2.4E1))-1.0/(L*L*L*L)*mu*x*y*(z*6.0-nu*z*2.4E1)*2.0;
  ep_s33[74] = -lambda*(1.0/(L*L*L*L)*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L)*(y*y)*z*8.0+1.0/(L*L*L*L)*z*(y*y-(z*z)*3.0)*4.0)-1.0/(L*L*L*L)*mu*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)*2.0;
  ep_s33[75] = lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(y*y*y*y)*1.0E1+x*x*x*x-(y*y*y*y)*5.0-nu*(x*x)*(y*y)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(y*y)*4.0E1)*2.0);
  ep_s33[76] = mu*(1.0/(L*L*L*L*L)*(x*x)*(x*x-z*z)*2.0E1-1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1)*-2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*2.0+nu*(z*z*z*z)*1.0E1+x*x*x*x-(z*z*z*z)*5.0-nu*(x*x)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(x*x)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*x*(nu*(x*x*x)*8.0+(x*x*x)*4.0-nu*x*(z*z)*4.0E1)*2.0);
  ep_s33[77] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*8.0E1+x*(y*y)*2.0E1+(x*x*x)*2.0E1-nu*x*(y*y)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*y*6.0E1-(y*y*y)*2.0E1));
  ep_s33[78] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*1.6E1+x*(y*y)*1.2E1+(x*x*x)*4.0-nu*x*(y*y)*4.8E1)-1.0/(L*L*L*L*L)*x*z*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0);
  ep_s33[79] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(x*x*x)*1.6E1+x*(z*z)*1.2E1+(x*x*x)*4.0-nu*x*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)-mu*(1.0/(L*L*L*L*L)*x*y*((x*x)*3.0-z*z)*4.0-1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)*2.0;
  ep_s33[80] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(x*x*x)*8.0E1+x*(z*z)*2.0E1+(x*x*x)*2.0E1-nu*x*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*x*((x*x)*z*6.0E1-(z*z*z)*2.0E1))-1.0/(L*L*L*L*L)*mu*x*((x*x)*z*6.0E1-(z*z*z)*2.0E1)*2.0;
  ep_s33[81] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(y*y)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0);
  ep_s33[82] = lambda*(1.0/(L*L*L*L*L)*y*z*(nu*(x*x)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*y*z*6.0+1.0/(L*L*L*L*L)*nu*(x*x)*y*z*8.0)-1.0/(L*L*L*L*L)*mu*(x*x)*y*z*1.2E1;
  ep_s33[83] = -lambda*(1.0/(L*L*L*L*L)*x*(y*(z*z)*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(y*y-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)-mu*(1.0/(L*L*L*L*L)*x*y*(y*y-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)*2.0;
  ep_s33[84] = lambda*(1.0/(L*L*L*L*L)*(x*x)*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((y*y)*(z*z)*-6.0+y*y*y*y+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(x*x)*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0)-mu*(1.0/(L*L*L*L*L)*(x*x)*((y*y)*3.0-z*z)*4.0-1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0)*2.0;
  ep_s33[85] = lambda*(-1.0/(L*L*L*L*L)*x*((y*y)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1)-1.0/(L*L*L*L*L)*mu*x*((y*y)*z*6.0E1-(z*z*z)*2.0E1)*2.0;
  ep_s33[86] = mu*(1.0/(L*L*L*L*L)*(y*y)*(y*y-z*z)*2.0E1-1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1)*-2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*2.0+nu*(z*z*z*z)*1.0E1+y*y*y*y-(z*z*z*z)*5.0-nu*(y*y)*(z*z)*2.0E1)*2.0-1.0/(L*L*L*L*L)*(y*y)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*y*(z*z)*4.0E1)*2.0);
  ep_s33[87] = lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(y*y*y*y)*2.0-(x*x*x*x)*5.0+y*y*y*y-nu*(x*x)*(y*y)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(y*y)*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(y*y)*4.0E1+1.0/(L*L*L*L*L)*y*(nu*(y*y*y)*8.0+(y*y*y)*4.0-nu*(x*x)*y*4.0E1)*2.0);
  ep_s33[88] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*8.0E1+y*(z*z)*2.0E1+(y*y*y)*2.0E1-nu*y*(z*z)*8.0E1)-1.0/(L*L*L*L*L)*y*((y*y)*z*6.0E1-(z*z*z)*2.0E1))-1.0/(L*L*L*L*L)*mu*y*((y*y)*z*6.0E1-(z*z*z)*2.0E1)*2.0;
  ep_s33[89] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*1.6E1+y*(z*z)*1.2E1+(y*y*y)*4.0-nu*y*(z*z)*4.8E1)-1.0/(L*L*L*L*L)*x*y*((y*y)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)-mu*(1.0/(L*L*L*L*L)*x*y*((y*y)*3.0-z*z)*4.0-1.0/(L*L*L*L*L)*x*y*(z*z)*8.0)*2.0;
  ep_s33[90] = lambda*(1.0/(L*L*L*L*L)*z*(nu*(y*y*y)*1.6E1+(x*x)*y*1.2E1+(y*y*y)*4.0-nu*(x*x)*y*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0);
  ep_s33[91] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(y*y*y)*8.0E1+(x*x)*y*2.0E1+(y*y*y)*2.0E1-nu*(x*x)*y*8.0E1)-1.0/(L*L*L*L*L)*y*(x*(y*y)*6.0E1-(x*x*x)*2.0E1));
  ep_s33[92] = lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(y*y)*2.0-nu*(z*z)*2.0+z*z)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0)-1.0/(L*L*L*L*L)*mu*x*(y*y)*z*1.2E1;
  ep_s33[93]= lambda*(1.0/(L*L*L*L*L)*x*z*(nu*(x*x)*-2.0+nu*(y*y)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*(y*y)*z*6.0+1.0/(L*L*L*L*L)*nu*x*(y*y)*z*8.0);
  ep_s33[94] = lambda*(-1.0/(L*L*L*L*L)*y*((x*x)*z*6.0E1-(z*z*z)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1)-1.0/(L*L*L*L*L)*mu*y*((x*x)*z*6.0E1-(z*z*z)*2.0E1)*2.0;
  ep_s33[95] = lambda*(1.0/(L*L*L*L*L)*(y*y)*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(z*z)*-6.0+x*x*x*x+z*z*z*z)*2.0-1.0/(L*L*L*L*L)*(y*y)*((x*x)*3.0-z*z)*4.0+1.0/(L*L*L*L*L)*(x*x)*(y*y)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)-mu*(1.0/(L*L*L*L*L)*(y*y)*((x*x)*3.0-z*z)*4.0-1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)*2.0;
  ep_s33[96] = -lambda*(1.0/(L*L*L*L*L)*y*(x*(z*z)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*y*(x*x-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)-mu*(1.0/(L*L*L*L*L)*x*y*(x*x-z*z)*2.0E1-1.0/(L*L*L*L*L)*x*y*(z*z)*4.0E1)*2.0;
  ep_s33[97] = mu*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(z*z*z*z)*2.0-(x*x*x*x)*5.0+z*z*z*z-nu*(x*x)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(x*x)*z*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(x*x*x*x)*1.0E1+nu*(z*z*z*z)*2.0-(x*x*x*x)*5.0+z*z*z*z-nu*(x*x)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(x*x-z*z)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(x*x)*z*4.0E1)*2.0);
  ep_s33[98] = mu*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*1.0E1+nu*(z*z*z*z)*2.0-(y*y*y*y)*5.0+z*z*z*z-nu*(y*y)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(y*y)*z*4.0E1)*2.0)*2.0+lambda*(1.0/(L*L*L*L*L)*(nu*(y*y*y*y)*1.0E1+nu*(z*z*z*z)*2.0-(y*y*y*y)*5.0+z*z*z*z-nu*(y*y)*(z*z)*2.0E1)*2.0+1.0/(L*L*L*L*L)*(z*z)*(y*y-z*z)*2.0E1+1.0/(L*L*L*L*L)*(y*y)*(z*z)*4.0E1+1.0/(L*L*L*L*L)*z*(nu*(z*z*z)*8.0+(z*z*z)*4.0-nu*(y*y)*z*4.0E1)*2.0);
  ep_s33[99] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*8.0E1+(x*x)*z*2.0E1+(z*z*z)*2.0E1-nu*(x*x)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(x*(z*z)*6.0E1-(x*x*x)*2.0E1))+1.0/(L*L*L*L*L)*mu*x*(nu*(z*z*z)*8.0E1+(x*x)*z*2.0E1+(z*z*z)*2.0E1-nu*(x*x)*z*8.0E1)*2.0;
  ep_s33[100] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)+1.0/(L*L*L*L*L)*y*z*(x*x-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*(x*x)*y*z*8.0)+1.0/(L*L*L*L*L)*mu*y*(nu*(z*z*z)*1.6E1+(x*x)*z*1.2E1+(z*z*z)*4.0-nu*(x*x)*z*4.8E1)*2.0;
  ep_s33[101] = lambda*(1.0/(L*L*L*L*L)*x*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)+1.0/(L*L*L*L*L)*x*z*(y*y-(z*z)*3.0)*4.0+1.0/(L*L*L*L*L)*x*(y*y)*z*8.0)+1.0/(L*L*L*L*L)*mu*x*(nu*(z*z*z)*1.6E1+(y*y)*z*1.2E1+(z*z*z)*4.0-nu*(y*y)*z*4.8E1)*2.0;
  ep_s33[102] = lambda*(1.0/(L*L*L*L*L)*y*(nu*(z*z*z)*8.0E1+(y*y)*z*2.0E1+(z*z*z)*2.0E1-nu*(y*y)*z*8.0E1)-1.0/(L*L*L*L*L)*z*(y*(z*z)*6.0E1-(y*y*y)*2.0E1))+1.0/(L*L*L*L*L)*mu*y*(nu*(z*z*z)*8.0E1+(y*y)*z*2.0E1+(z*z*z)*2.0E1-nu*(y*y)*z*8.0E1)*2.0;
  ep_s33[103] = lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)+mu*(1.0/(L*L*L*L*L)*x*y*(nu*(x*x)*-2.0+nu*(z*z)*2.0+x*x)*2.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)*2.0;
  ep_s33[104]= lambda*(1.0/(L*L*L*L*L)*x*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0-1.0/(L*L*L*L*L)*x*y*(z*z)*6.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)+mu*(1.0/(L*L*L*L*L)*x*y*(nu*(y*y)*-2.0+nu*(z*z)*2.0+y*y)*2.0+1.0/(L*L*L*L*L)*nu*x*y*(z*z)*8.0)*2.0;
  ep_s33[105] = -lambda*(1.0/(L*L*L*L*L)*z*(x*(y*y)*6.0E1-(x*x*x)*2.0E1)+1.0/(L*L*L*L*L)*x*z*(x*x-y*y)*2.0E1-1.0/(L*L*L*L*L)*x*(y*y)*z*4.0E1);
  ep_s33[106] = lambda*(1.0/(L*L*L*L*L)*(z*z)*(x*x-(y*y)*3.0)*4.0+1.0/(L*L*L*L*L)*(nu*2.0-1.0)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*2.0-1.0/(L*L*L*L*L)*(z*z)*((x*x)*3.0-y*y)*4.0+1.0/(L*L*L*L*L)*(x*x)*(z*z)*8.0+1.0/(L*L*L*L*L)*(y*y)*(z*z)*8.0)+1.0/(L*L*L*L*L)*mu*(nu*2.0-1.0)*((x*x)*(y*y)*-6.0+x*x*x*x+y*y*y*y)*4.0;
  ep_s33[107] = lambda*(-1.0/(L*L*L*L*L)*z*((x*x)*y*6.0E1-(y*y*y)*2.0E1)+1.0/(L*L*L*L*L)*y*z*(x*x-y*y)*2.0E1+1.0/(L*L*L*L*L)*(x*x)*y*z*4.0E1);
}
}
/**********************************************************************/
void ElasticPolynomial::applyDirichletConstraint(double pt[3],double uvw[3])
{
	int j,nOldConstraints;
	double x,y,z,nu;

	// generate constraints
	x = pt[0] - ep_origin[0];
	y = pt[1] - ep_origin[1];
	z = pt[2] - ep_origin[2];

	for (int i = 0; i < 3;i++) {
		ep_XYZMin[i] = MIN(ep_XYZMin[i],pt[i]);
		ep_XYZMax[i] = MAX(ep_XYZMax[i],pt[i]);
	}
	nu = ep_nu;
	nOldConstraints = ep_nConstraints;

	ep_nConstraints += 3;
	ep_A.conservativeResize(ep_nConstraints,ep_nCoefficients);//keep previous rows
	ep_rhs.conservativeResize(ep_nConstraints);//keep previous rows

	computeDisplacementCoefficients(pt);
	for (j = 0; j < ep_nCoefficients; j++){
		ep_A(nOldConstraints,j) = ep_u(j);
		ep_A(nOldConstraints+1,j) = ep_v(j);
		ep_A(nOldConstraints+2,j) = ep_w(j);
	}
	ep_rhs(nOldConstraints) = uvw[0];
	ep_rhs(nOldConstraints+1) = uvw[1];
	ep_rhs(nOldConstraints+2) = uvw[2];
	ep_dirichletConstraintApplied = true;
}
/**********************************************************************/
void ElasticPolynomial::applyTractionConstraint(double pt[3],double outwardNormal[3],double traction[3])
{
	int j,nOldConstraints;
	double x,y,z;

	if (ep_verbose) printf("ElasticPolynomial: Appling traction constraint\n");
	// generate constraints
	x = pt[0] - ep_origin[0];
	y = pt[1] - ep_origin[1];
	z = pt[2] - ep_origin[2];

	for (int i = 0; i < 3;i++) {
		ep_XYZMin[i] = MIN(ep_XYZMin[i],pt[i]);
		ep_XYZMax[i] = MAX(ep_XYZMax[i],pt[i]);
	}

	nOldConstraints = ep_nConstraints;

	ep_nConstraints += 3;
	ep_A.conservativeResize(ep_nConstraints,ep_nCoefficients);//keep previous rows
	ep_rhs.conservativeResize(ep_nConstraints);//keep previous rows

	computeStressCoefficients(pt);
	double scale = ep_L/ep_E;// for proper scaling

	for (j = 0; j < ep_nCoefficients; j++){
		ep_A(nOldConstraints,j) = (scale)*(ep_s11(j)*outwardNormal[0] + ep_s12(j)*outwardNormal[1] + ep_s13(j)*outwardNormal[2]);
		ep_A(nOldConstraints+1,j) = (scale)*(ep_s12(j)*outwardNormal[0] + ep_s22(j)*outwardNormal[1] + ep_s23(j)*outwardNormal[2]);
		ep_A(nOldConstraints+2,j) = (scale)*(ep_s13(j)*outwardNormal[0] + ep_s23(j)*outwardNormal[1] + ep_s33(j)*outwardNormal[2]);
	}
	ep_rhs(nOldConstraints) = (scale)*traction[0];
	ep_rhs(nOldConstraints+1) = (scale)*traction[1];
	ep_rhs(nOldConstraints+2) = (scale)*traction[2];
}
/**********************************************************************/
int ElasticPolynomial::getNumberOfUnknowns(void)
{
	return ep_nCoefficients;
}
/**********************************************************************/
bool ElasticPolynomial::solve(void)
{
	
	if (ep_nConstraints < ep_nCoefficients){
		printf("ElasticPolynomial: Insufficient constraints\n");
		return false;
	}
	if (!ep_dirichletConstraintApplied){
		printf("ElasticPolynomial: Dirichlet constraint has not been applied\n");
		return false;
	}
	
	if (ep_verbose) printf("ElasticPolynomial: ep_nConstraints = %d \n",ep_nConstraints);
 
	if (1) 
		ep_sol = ep_A.jacobiSvd(ComputeThinU | ComputeThinV).solve(ep_rhs);

	else {
		MatrixXd At,B;
		VectorXd c;

		At = ep_A.transpose();
		B = At*ep_A;
		c = At*ep_rhs;

		ep_sol = B.ldlt().solve(c);
	}

	ep_solved = true;
	return true;
}
/**********************************************************************/
void ElasticPolynomial::printRHS(void)
{
	double val;
	for (int i = 0; i < ep_nConstraints; i++) {
		val = 1.0*ep_rhs(i);
		printf("%.2e \n",val);
	}
}
/**********************************************************************/
void ElasticPolynomial::printAMatrix(void)
{
	double val;
	for (int i = 0; i < ep_nConstraints; i++) {//ep_nConstraints
		for (int j = 0; j < 7; j++) {//ep_nCoefficients
			val = 1.0*ep_A(i,j);
			printf("%.2e ",val);
		}
		printf("\n");
	}
}
/**********************************************************************/
void ElasticPolynomial::evaluateDisplacementAt(double pt[3],double uvw[3])
{
	if (!ep_solved){
		printf("ElasticPolynomial: solution not available\n");
		return;
	}
	computeDisplacementCoefficients(pt);
	uvw[0] =0; uvw[1] = 0; uvw[2] = 0;
	for (int i = 0; i < ep_nCoefficients; i++){
		uvw[0] += ep_u[i]*ep_sol[i];
		uvw[1] += ep_v[i]*ep_sol[i];
		uvw[2] += ep_w[i]*ep_sol[i];
	}
}
/**********************************************************************/
void ElasticPolynomial::evaluateStressTensorAt(double pt[3],double s[3][3])
{
	if (!ep_solved){
		printf("ElasticPolynomial: solution not available\n");
		return;
	}
	int i,j;
	computeStressCoefficients(pt);
	for ( i =0; i < 3; i++)
		for ( j =0; j < 3; j++)
			s[i][j] = 0.0;

	for (int i = 0; i < ep_nCoefficients; i++){
		s[0][0] += ep_s11[i]*ep_sol[i];
		s[1][1] += ep_s22[i]*ep_sol[i];
		s[2][2] += ep_s33[i]*ep_sol[i];
		s[0][1] += ep_s12[i]*ep_sol[i];
		s[0][2] += ep_s13[i]*ep_sol[i];
		s[1][2] += ep_s23[i]*ep_sol[i];
	}
	s[1][0] = s[0][1];
	s[2][0] = s[0][2];
	s[2][1] = s[1][2];
}
/**********************************************************************/
void ElasticPolynomial::evaluateVonMisesStressAt(double pt[3],double *vonMises)
{
	if (!ep_solved){
		printf("ElasticPolynomial: solution not available\n");
		return;
	}
	double s[3][3];
	evaluateStressTensorAt(pt,s);

	*vonMises =  1.0/sqrt(2.0)*sqrt( pow(s[0][0]-s[1][1],2.0)+ pow(s[0][0]-s[2][2],2.0) +pow(s[1][1]-s[2][2],2.0)+
		6*(s[0][1]*s[0][1] + s[0][2]*s[0][2] + s[1][2]*s[1][2]));
}
/**********************************************************************/
void ElasticPolynomial::printSolution(void)
{
	if (!ep_solved){
		printf("ElasticPolynomial: solution not available\n");
		return;
	}
	for (int i = 0; i < ep_nCoefficients; i++) 
		printf("%.2e \n",ep_sol(i));
}
/**********************************************************************/
ElasticPolynomial::~ElasticPolynomial()
{
	if (ep_nCoefficients == 0) return;

	ep_u.resize(1);
	ep_v.resize(1);
	ep_w.resize(1);

	ep_A.resize(1,1);
	return;
}
