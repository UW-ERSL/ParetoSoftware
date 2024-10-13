#ifndef AFPLASTICITY_H
#define AFPLASTICITY_H

// Assembly Free Plasticity
#include <AFElasticity.h>
#include <AFPlasticityGPUDefines.h>


class AFPlasticity: public AFElasticity {
private:
    int         p_maxNewtonIteration;//max number of Newton iterations
    int         numPlasticGP;
    int         numPlasticElems;
    int         *p_elem;
    double      *KPlastic;
    bool        p_verbose;//controls messages
    double      p_E,p_nu,p_Y,p_G;// material properties for easy access  //p_G: Shear modulus
    bool        *p_PlasticState;// plastic state of each gauss point
    bool        *p_elemPlasticState;
    double      *p_sol_k; // the solution (displacement) during iteration k
    double      *p_sol_temp; // temporary solution during iteration k
    double      *p_delta_sol;// increment from Newton
    double      p_NewtonError;
    double      *(p_StressAtk[48]);// the 48 (8*6) stresses for each element having 8 Gausspoints (6 stress components), at iteration k
    double      *(p_PlasticStrains[48]);// the 6 plastic strains at each guass point. 8 GPs per element.
    double      *(p_DeviatoricStresses[48]);// the 6 deviatoric stresses at each guass point

    void        KPlasticAllPlasticElements(void);
    void        setElemPlasticState(int elem);
    void        setPlasticStatesOfAllElems(void);
    void        reInitializeAllAFPlasticityVariables(void);
    bool        allocateAFPlasticityMemory(void);
    void        releaseAFPlasticityMemory(void);
    bool        NewtonIteration(void);
    void        solveConstitutiveProblemPlasticity(void);
    void        getElemNodeDeformation_Plasticity(int elem,double u[8],double v[8],double w[8]);
    double      computeDeviatoricNormAtGP(int gp, int elem);
    void        computeDevStressesAtGP(int gp, int elem);
    void        solveConstitutiveProblemPlasticityPlasticStrainUpdate(void);
    void        Ktu(double *x,double *y);
    void        computeKPlastic(int elem, double KPlastic[576]);
    void        KdelCPUElemBased1(double *x,double *yPlastic);
    void        KdelCPUElemBased(double *x,double *yPlastic);
    void        computeKPlasticMatraixMul(int elem, double Ke[576]);
    void        computeDmatrixPlasticity(int gp,int elem, double D[6][6]);
    void        KPlasticDirectComputation(int elem, double Kdel1[576]);
    void        computeDeltaForce_Plasticity(double *deltaForce);
    bool        performTangentialCG(int *niter, double *f);
    void        computeFInternal(int elem, double fint[24]);
    void        computeFInternal_Ku_double(int elem, double fint[24]);
    bool        performTangentialCGWithDeflation(int *niter, double *f);
    bool        computeSparseWKWPlasticity(void);
    void        getKMatrixPlastic(int elem, double *KPlasticMatrix);
    void        ComputeReducedKMatrixPlasticity(void);
//GPU
    p_GPUData   p_gpuData;
    bool        usingGPU_plasticity;
    int         performCGOnGPU_plasticity(void);
    int         performCGWOnGPU_plasticity(void);
    p_WorkSpaceData p_gpuWS;

protected:// inherited classes can access these variables

public:
                AFPlasticity();  //constructor

     //bool       SolveElastoPlasticity(int nSteps,bool (*callBackFunction)(double fractionCompleted));
     bool       SolveElastoPlasticity(int nSteps);
     void       enableAFPlasticityVerbose(void);
     void       disableAFPlasticityVerbose(void);

                ~AFPlasticity();   //destructor

};

#endif
