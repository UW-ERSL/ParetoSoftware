#ifndef ELASTIC_POLYNOMIAL_H
#define ELASTIC_POLYNOMIAL_H

#include <Eigen/Dense>

using namespace Eigen;

class ElasticPolynomial {
private:
	int		ep_order;// n, the order of polynomial
	int		ep_nCoefficients;// N, number of unknown coefficients
	int		ep_nConstraints; // how many active constraints (must be >= ep_nCoefficients) to solve
	double	ep_nu; // Poisson's ratio
	double	ep_E; //Young's modulus
	double	ep_lambda;// Lame parameter
	double	ep_mu; // Lame parameter
	double	ep_L;// Model scale
	bool	ep_verbose; // for printing messages
	double	ep_XYZMin[3],ep_XYZMax[3];
	double	ep_origin[3];
	bool			ep_dirichletConstraintApplied;
	bool			ep_solved;
	VectorXd		ep_u,ep_v,ep_w;
	VectorXd		ep_s11,ep_s12,ep_s13,ep_s22,ep_s23,ep_s33;

	MatrixXd		ep_A;// Coefficient Matrix
	VectorXd		ep_rhs;
	VectorXd		ep_sol;// coefficients

	void	 computeDisplacementCoefficients(double pt[3]);
	void	 computeStressCoefficients(double pt[3]);

public:
	
			 ElasticPolynomial(int dummy);	//Constructor
	void	 initialize(int order, double E, double nu,double modelScale);
	void	 setOrigin(double origin[3]);
	void	 applyDirichletConstraint(double pt[3],double uvw[3]);
	void	 applyTractionConstraint(double pt[3],double outwardNormal[3],double traction[3]);
	bool	 solve(void);
	void	 enableVerbose(void);
	void	 disableVerbose(void);
	int		 getNumberOfUnknowns(void);
	void	 evaluateDisplacementAt(double pt[3],double uvw[3]);
	void	 evaluateStressTensorAt(double pt[3],double s[3][3]);
	void	 evaluateVonMisesStressAt(double pt[3],double *vonMises);
	void	 printSolution(void);	
	void	 printAMatrix(void);
	void	 printRHS(void);
			 ~ElasticPolynomial();//Destructor

};
#endif