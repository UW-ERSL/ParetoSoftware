#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include <fstream>
#define MMA_VERBOSE true
#define PI 3.14159265
using namespace std;
/************************************/
class MMA_optimizer
{


    int m,n,a0;
    double epsimin,raa0,raa0eps,f0app,z,zet;
    vector<int> a;
    vector<double> xval, xold1, xold2, xmin, xmax, xmami, low, upp, c, d, raa, raaeps;
	vector<double> x,y,s,lam,mu,xsi,eta,fapp;
	double obj;
	vector<double> sens,constraint,constrSens;

    int outerIter,maxOutIt;
    double kkttol,relativeFuncTol;

	int concheck(double,vector<double>);
	void raaupdate(double,vector<double>);
	double kktcheck(vector<double> fval, vector<double> df0dx, vector<double> dfdx);

public:


    void initialize(int numOfDesignVariables, int numOfConstraints, int maxOuterIterations = 100, double relFuncTol = 1e-3,double kktTolerance = 1e-3,double epsiminTilde = 1e-8, double cTilde = 1000,double dTilde = 1,double a0Tilde = 1,double aTilde = 0,double raa0Tilde = 1e-2,double raaepsTilde = 1e-6);
    void setDesignVariableInitialValuesAndBounds(vector<double> xInit,vector<double> xLow,vector<double> xUpp);
    void asymp(vector<double>df0dx, vector<double>dfdx);
	void gcmmasub(double f0val, vector<double> df0dx, vector<double> fval,vector<double> dfdx);
	void subsolv(vector<double> alfa,vector<double> beta,vector<double> p0,vector<double> q0,vector<double> P,vector<double> Q,vector<double> b);

 //  void optimize(double (*compObj)(vector<double>),bool (*compConstr)(vector<double> ,vector<double>& ),bool (*compObjSensi)(vector<double> ,vector<double>&),bool (*compConstrSensi)(vector<double> ,vector<double>&));
    void optimize(std::function<double(vector<double>)>compObj,std::function<bool(vector<double>,vector<double>&)>compConstr,std::function<bool(vector<double>,vector<double>&)>compObjSensi,std::function<bool(vector<double>,vector<double>&)>compConstrSensi,std::function< bool( vector<double>& ) >modifyDesignVar);

};
/************************************/
