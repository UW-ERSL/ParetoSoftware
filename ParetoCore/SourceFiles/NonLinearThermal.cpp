#include "NonLinearThermal.h"

#pragma warning( disable : 4101 )//disable warning with openmp

/*********************************************************************************/
NonLinearThermal::NonLinearThermal()
{
    nlt_isTransient = false;
    nlt_CETemplates_thermal = NULL;
    tt_beta = 1;
}
/*********************************************************************************/
void NonLinearThermal::initializeKEnonLinearThermal(void)
{
    nlt_numNeighbouringElemsOfNodeOnEvolvingSurface = (int *)malloc(getNumNodes()*sizeof(double));
    int nTemplateIDs;
    double Ke_thermal[8][8],thermalConductivity,Ce_thermal[8][8],specificHeat,density;

    if (!h_voxelgrid)
        return;

    safeFree(h_KETemplates_thermal);
    nTemplateIDs = getNumTemplateIDs();
    h_KETemplates_thermal =	(double *)malloc((nTemplateIDs)*8*8*sizeof(double));
    nlt_CETemplates_thermal = (double *)malloc((nTemplateIDs)*8*8*sizeof(double));

    tt_sol_thermalElemPrev = (double *)malloc((h_nElems)*sizeof(double));
    memset(tt_sol_thermalElemPrev,0,h_nElems*sizeof(double));

    // must generalize later to multiple materials
    thermalConductivity = getThermalConductivityOfElem(0);
    specificHeat = getSpecificHeatOfElem(0);
    density = getDensityOfElem(0);
    ComputeKe_thermal(h_dx,h_dy,h_dz,1,Ke_thermal);// unit value

    if(nlt_isTransient)
        ComputeCE_TT(h_dx,h_dy,h_dz,density,1,Ce_thermal);

    int k =0;
    for (int i =0; i < 8; i++) {
        for(int j = 0; j < 8;j++) {
            h_KETemplates_thermal[k*(nTemplateIDs) ] = (double) Ke_thermal[i][j];
            nlt_CETemplates_thermal[k*(nTemplateIDs)] = (double) Ce_thermal[i][j];
            k++;
        }
    }
}
/*********************************************************************************/
void NonLinearThermal::setNonLinearThermalCoefficients(float k[3], float c[3])
{
    for(int i = 0 ; i< 3; i++){
        nlt_k[i] = k[i];
        nlt_c[i] = c[i];
    }
    tt_beta = 1;
}
/*********************************************************************************/
int NonLinearThermal::performCG_NLThermal(double *f,double *u)
{
    int nCGIter = 0;
    if (ws_deflationType ==  DEFLATION_NONE) {//no deflation
        nCGIter = performSimpleCG_NLThermal(f,u);
    }
    else {
        nCGIter = performDeflatedCG_NLThermal(f,u);
    }
    return nCGIter;
}
/**********************************************************************/
int	NonLinearThermal::performSimpleCG_NLThermal(double *f,double *u)
{
    // Solves Kx = f
    int  niter;
    Timer timer;
    double rho,alpha,beta,rho_new,normr,normr0;
    double *r,*p,*a;


    r = (double *) malloc(h_nNodes*sizeof(double));
    p = (double *) malloc(h_nNodes*sizeof(double));
    a = (double *) malloc(h_nNodes*sizeof(double));

    timer.start();
    KuCPU_NLThermal(u,a);

    zeaxpy_thermal(-1,1,r,a,f);
    normr = sqrt(dotprod_thermal(r,r));
    normr0 = normr;
    copy_thermal(r,a);
    copy_thermal(a,p);
    rho = dotprod_thermal(a,r);
    niter = 1;

    while ( (fabs(normr0) > 0) && ((niter < 10) || (normr > h_relAccuracyThermal*normr0)))  {
        KuCPU_NLThermal(p,a);// a = K*p;//
        beta = dotprod_thermal(a,p);
        alpha = rho/beta;

        axpy_thermal(alpha,p,u);
        axpy_thermal(-alpha,a,r);
        normr = sqrt(dotprod_thermal(r,r));
        if (niter == 1) normr0 = normr;
        copy_thermal(r,a);// Set a = r
        rho_new =  dotprod_thermal(a,r);
        zeaxpy_thermal(1,(rho_new/(rho)),p,a,p);
        rho = rho_new;
        niter++;

        if (normr >1000*normr0)  {
            writeMessageToParetoLog("ThermoElasticity: Diverging at %d iter, resErr = %e \n",niter,normr);
            niter = -1;
            break;
        }
        if (niter == h_CGMaxIter)  {
            writeMessageToParetoLog("ThermoElasticity: Did not converge in %d iter, resErr = %e \n",niter,normr/normr0);
            niter = -1;
            break;
        }
    }
    safeFree(a);
    safeFree(p);
    safeFree(r);
    return niter;
}
/*********************************************************************************/
int	NonLinearThermal::performDeflatedCG_NLThermal(double *f,double *u){
    // Solves Ku = f
    // Assumes K and f have been defined, and Dirichlet conditions have been set
    // Also assumes CG iterations and tolerances have been defined.
    // Solution is returned in h_sol_thermal
    bool success;

    double rho,alpha,beta,rho_new,normr,normr0;
    double *r,*p,*a,*d, *t;

    int nIter;

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
    KuCPU_NLThermal(u,a);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
    getWorkSpaceVectorThermal(r, mu); // mu = W'*h_f
    //Compute [W'*K*W]*result = W'*r = mu (from above)
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);// h_sol = W*result
    enforceDirichletZero_thermal(t);//Anirudh
    axpy_thermal(1,t,u);
    KuCPU_NLThermal(u,a);// a = K*sol
    zeaxpy_thermal(-1,1,r,a,f);// r = -a + f//Anirudh
    normr0 = dotprod_thermal(r,r);
    normr = 10*normr0;
    //********************************************************
    KuCPU_NLThermal(r,t);
    getWorkSpaceVectorThermal(t, mu);
    result = sparseSolverThermal.solve(mu);
    getSolutionSpaceVectorThermal(result, t);
    enforceDirichletZero_thermal(t);//Anirudh
    zeaxpy_thermal(-1,1,p,t,r);// p = r - t
    //********************************************
    rho = dotprod_thermal(r,r);
    nIter = 1;     // Init counter for number of iterations

    success = true;
    while (normr > h_relAccuracyThermal*normr0)   {
        KuCPU_NLThermal(p,a);// a = K*p;
        beta = dotprod_thermal(a,p);
        alpha = rho/beta;
        axpy_thermal(alpha, p,u);//u = u + alpha*p;
        if (nIter%1000 == 0)	{ // restart CG with recomputing residual
            KuCPU_NLThermal(u,a);// a = K*sol
            zeaxpy_thermal(-1,1,r,a,f);// r = -a + f
        }
        else
            axpy_thermal(-alpha,a,r);//r = r - alpha*a;
        if (nIter == 1) normr0 = normr;
        h_cgNorm[nIter-1] = log10(normr/normr0);

        rho_new =  dotprod_thermal(r,r);//rho = a'*r
        normr = (rho_new);
        KuCPU_NLThermal(r,t);

        getWorkSpaceVectorThermal(t, mu);		// mu = W'*t
        result = sparseSolverThermal.solve(mu);
        getSolutionSpaceVectorThermal(result, t); // t = W*result
        enforceDirichletZero_thermal(t);//Anirudh

        zeaxpy_thermal(1,(rho_new/rho),p,r,p);//p = r + rho_new/rho * p;
        zeaxpy_thermal(-1,1,p,t,p);//p = -t + p
        rho = rho_new;
        h_nCGItersActual = nIter;
        nIter++;

        if (nIter >= ws_DCGMaxIter)  {
            writeMessageToParetoLog("NonlinearThermal: Could not converge in performDeflatedCG_NLThermal");
            return nIter;
        }
    }

    //computeTemperatureMax();
    free(a);
    free(p);
    free(r);
    free(d);
    free(t);
    return nIter;
}
/*********************************************************************************/
void NonLinearThermal::KuCPU_NLThermal(double *x,double *y)
{
    // Carry out K*x on CPU, also enforce Dirichlet boundary conditions Element based
    register int i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,row,nTemplateIDs;
    register double KE,CE, TResult=0.0,TVal,T;
    register double elemConductivity,elemSpecificHeat;
    register double transientFactor,T_prev;
    nTemplateIDs = getNumTemplateIDs();
    transientFactor = tt_beta;


#pragma omp parallel for private(i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,KE,CE,TResult,T,T_prev,TVal,row,elemConductivity,elemSpecificHeat)
    for ( node = 0; node < h_nNodes; node++) {
        TResult=0.0;
        if ((doesNodeExist(node)) &&  (h_free_thermal[node])) {// process only if one of the dof  is free
            maxElem = h_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElem - nodeIndex*MAXELEMS;
                if (getElemPseudoDensity(elem) < 0.5)// element exists but is void (during topology optimization)
                    continue;

                T = getTemperatureInElem(elem);
                T_prev = getPreviousTemperatureInElem(elem);

                if(nlt_isTransient){
                    elemConductivity = getNonLinearConductivityAtTemp(T,T_prev);
                    elemSpecificHeat = getNonLinearSpecificHeatAtTemp(T,T_prev);
                }
                else {
                    elemConductivity = getNonLinearConductivityAtTemp(T,T_prev);
                }
                elemID = getElemTemplateID(elem);
                row = 8*nodeIndex;
                for (i = 0; i < 8; i++) {
                    node2 = h_elems[(h_nElems)*i + elem];
                    TVal = x[node2];
                    KEindex = (row + i)*(nTemplateIDs) + elemID;
                    KE = h_KETemplates_thermal[KEindex];
                    if(nlt_isTransient){
                        CE = nlt_CETemplates_thermal[KEindex]; // transient thermal
                        TResult +=(tt_beta*elemConductivity*KE*TVal )+ (elemSpecificHeat*CE*TVal/tt_deltaTime);

                    }
                    else
                        TResult +=(elemConductivity*KE*TVal );
                }
            }
        }
        y[node] = TResult;
        y[node] += transientFactor*h_convection[node]*x[node]; //h_convection component
        y[node] +=  transientFactor*h_radiationNonLinear[node]*x[node];// h_radiation component;
        y[node] = (h_free_thermal[node])*y[node] ;// Dirichlet condition
    }
}
/**********************************************************************/
void NonLinearThermal::setTransientNonLinearThermalData(bool isNonLinearTransient,float initialTemperature, float timeStepSize,float totalTime)
{
    nlt_isTransient = isNonLinearTransient;
    tt_deltaTime = timeStepSize;//time step, second time
    tt_time = totalTime;// seconds transient analysis
    tt_beta = 1;
}
/**********************************************************************/
bool	NonLinearThermal::solveNLStaticThermalProblem(bool solveElastic,bool (*callBackFunction)(double nrmError))
{
    h_nodeBasedAssembly = false;
    nlt_isTransient = false;
    double* h_fSave;


    initializeKEnonLinearThermal(); //initilize h_KETemplates_thermal
    computeElementTemperature();

    if (ws_deflationType !=  DEFLATION_NONE) {// deflation
        if(h_thermo_elasticity_verbose)
            writeMessageToParetoLog("Computing thermal deflation ststic non linear");
        CreateDeflationGroups();
    }
    int nNodes = getNumNodes();
    if(solveElastic == true){
        h_fSave = (double *)malloc(3*nNodes*sizeof(double));
        computeNLThermalForceTemplate();
        getForceVector(h_fSave);
    }
    //double emissivity = 0.5;
    //applyConvectionOnExposedSurface(20,600);

    //applyRadiationOnAllExposedNodes(emissivity) ;

    if(h_isThereRadiation)
        computeRadiationNonLinear();

    //addVolumetricHeatSource();

    solveNLThermalProblemPicard(50,0);

    computeTemperatureMax();
    // elastic
    //compute the thermal forces and then solve elastic
    if(solveElastic == true) {
        if(h_thermo_elasticity_verbose)cout <<"Solving Structural problem with thermal loads \n ---------------------- \n";
        computeElementTemperature();
        addNonLinearThermalForce();
        solveElasticityStatic();
        postprocess();
        pushInitialStaticSolutionToTessellation();
        setForceVector(h_fSave);
        safeFree(h_fSave);h_fSave = NULL;
    }
    return true;
}
/**********************************************************************/
bool  NonLinearThermal::solveNLThermalProblemPicard(int maxIter,int timeStep)
{
    int iter = 0,nCGIter;
    double *solOld,*diff,normErr;
    bool solveSuccess = false;

    solOld = (double *)malloc(h_nNodes*sizeof(double));
    diff = (double *)malloc(h_nNodes*sizeof(double));
    memset(solOld,0,h_nNodes*sizeof(double));


    normErr = 1;
    for (iter =1; iter < maxIter; iter++){
        if(h_isThereRadiation)
            computeRadiationNonLinear();// compute the nonlinear h_radiation term and nonlinear thermal force
        if ((normErr > 0.0) && (ws_deflationType !=  DEFLATION_NONE)) {// deflation
            sparseWKW_NLThermal();
        }
        if(nlt_isTransient)
            ComputeNLThermalEffectiveForce(timeStep);


        if(h_isThereRadiation)
            nCGIter = performCG_NLThermal(h_f_thermalWithRadiation,h_sol_thermal);
        else if(nlt_isTransient)
            nCGIter = performCG_NLThermal(tt_Feff,h_sol_thermal);
        else
            nCGIter = performCG_NLThermal(h_f_thermal,h_sol_thermal);

        zeaxpy_thermal(-1,1,diff,h_sol_thermal,solOld);
        normErr = Norm_thermal(diff)/(Norm_thermal(h_sol_thermal)+1e-16);

        writeMessageToParetoLog("NonLinearThermal: Picard iter %d, normErr = %e", iter,normErr);

        if (normErr < h_relAccuracyThermal){
            solveSuccess =  true;
            break;
        }
        copy_thermal(h_sol_thermal,solOld);
        computeElementTemperature();
        savePreviousTransientTemperature();
    }
    free(solOld);
    free(diff);
    return solveSuccess;
}
/**********************************************************************/
double NonLinearThermal::getPreviousTemperatureInElem(int elem)
{ 
    if (tt_sol == NULL) return 0; // thermal problem has not been solved or it is a static problem
    if ((elem < 0) || (elem >= h_nElems)) return 0;
    return tt_sol_thermalElemPrev[elem];
}
/**********************************************************************/
void NonLinearThermal::savePreviousTransientTemperature(void)
{
    // now compute temperature at center of each element
    if(tt_sol == NULL)
        return; // it might be solving a static problem
    int node,counter;
    double TElem;

    for (int elem = 0; elem < h_nElems; elem++) {
        if( getElemPseudoDensity(elem) < 0.05 )
            continue;

        TElem = 0;
        counter = 0;
        tt_sol_thermalElemPrev[elem]  = 0;
        for (int m = 0;m < 8; m++) {
            node = h_elems[(h_nElems)*m + elem];
            if (!doesNodeExist(node)) continue;
            TElem += tt_sol[node];
            counter++;
        }
        if (counter > 0)
            tt_sol_thermalElemPrev[elem] = (TElem/counter);
    }
}
/*
/************************************************************/
void NonLinearThermal::ComputeNLThermalEffectiveForce(int timeStep)
{
    KuCPUForceElemBased_NLThermal(tt_sol,tt_Aeff);

    double Tref;
    int nodeLabel=0;
    //	Computing Force Previous
    if(timeStep > 1)
        if(h_isThereRadiation)
            memcpy(tt_forcePrevious,tt_forceCurrentWithRadiation,h_nNodes*sizeof(double));
        else
            memcpy(tt_forcePrevious,tt_forceCurrent,h_nNodes*sizeof(double));

    //  Computing Force Current

    if(timeStep ==1 && h_isThereRadiation){
        memcpy(tt_forceCurrent,h_f_thermal,h_nNodes*sizeof(double));
        memcpy(tt_forceCurrentWithRadiation,h_f_thermalWithRadiation,h_nNodes*sizeof(double));
    }
    else if(timeStep == 1){
        memcpy(tt_forceCurrent,h_f_thermal,h_nNodes*sizeof(double));
    }
    else if(h_isThereRadiation) {
        for (int i = 0; i<h_nNodes; i++) {
            nodeLabel = h_nodeLabels[i];
            Tref =  h_radiationTRef[nodeLabel];
            tt_forceCurrentWithRadiation[i] = tt_forceCurrent[i] + h_radiationNonLinear[i]*Tref;
        }
    }
    if (timeStep > 1){
#pragma omp parallel for
        for(int i = 0; i<h_nNodes; i++)
            tt_Feff[i] = tt_Aeff[i] + (1-tt_beta)*tt_forcePrevious[i] + tt_beta*tt_forceCurrent[i];
    }
}
/**********************************************************************/
void NonLinearThermal::KuCPUForceElemBased_NLThermal(double *x,double *y) {
    // y = K*x{
    // Carry out K*x on CPU, also enforce Dirichlet boundary conditions
    register int i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,row,nTemplateIDs;
    register double KE,CE, TResult=0.0,TVal,T;

    double elemConductivity, elemSpecificHeat,T_prev;
    nTemplateIDs = getNumTemplateIDs();
#pragma omp parallel for private(i,j,node,node2,codedElem,elem,KEindex,maxElem,nodeIndex,elemID,KE,CE,T,T_prev,TResult,TVal,row,elemConductivity,elemSpecificHeat)
    for ( node = 0; node < h_nNodes; node++) {

        TResult=0.0;
        if ((doesNodeExist(node)) &&  (h_free_thermal[node])) {// process only if one of the dof  is free
            maxElem = h_nNeighboringElems[node];// how many elements connected to node
            for (j =0; j < maxElem; j++) {//
                codedElem = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
                nodeIndex = codedElem/MAXELEMS;//ranges from 0 to 7, position of this node within this elem
                elem = codedElem - nodeIndex*MAXELEMS;
                if (getElemPseudoDensity(elem) < 0.5)// element exists but is void (during topology optimization)
                    continue;
                elemID = getElemTemplateID(elem);

                T = getTemperatureInElem(elem);
                T_prev = getPreviousTemperatureInElem(elem);
                elemConductivity = getNonLinearConductivityAtTemp(T ,T_prev);
                elemSpecificHeat = getNonLinearSpecificHeatAtTemp(T,T_prev);

                row = 8*nodeIndex; //row index in KE matrix corresponding to each node (dofs)
                for (i = 0; i < 8; i++) {// now accumulate for each of the 8 nodes
                    node2 = h_elems[(h_nElems)*i + elem];
                    TVal = x[node2];

                    KEindex = (row + i)*(nTemplateIDs) + elemID; //calc expr here using TVal
                    KE = h_KETemplates_thermal[KEindex]; //NLThermal : Ensure that the conductivity value is set to one here
                    CE = nlt_CETemplates_thermal[KEindex];

                    TResult +=((tt_beta-1)*elemConductivity*KE*TVal )+ (elemSpecificHeat*CE*TVal/tt_deltaTime);   //
                }
            }
        }
        y[node] = TResult;
        y[node] += (tt_beta-1)*h_convection[node]*x[node];
        y[node] +=  (tt_beta-1)*h_radiationNonLinear[node]*x[node];
        y[node] = (h_free_thermal[node])*y[node] ;// Dirichlet condition is enforced
    }
}
/*************************************************************/
double NonLinearThermal::getNonLinearConductivityAtTemp(double T ,double T_old)
{
    double elemConductivity;

    if(!nlt_isTransient){
        elemConductivity = nlt_k[0] + nlt_k[1]*(T) + nlt_k[2]*(T)*(T);
    }
    else {//transient
        elemConductivity =( nlt_k[0] + nlt_k[1]*(T ) + nlt_k[2]*(T)*(T));
    }
    return elemConductivity;
}
/*************************************************************/
double NonLinearThermal::getNonLinearSpecificHeatAtTemp(double T, double T_old)
{
    double C = 0;
    if(nlt_isTransient == false)
        return 0; // no specific heat for steady state ...

    C = nlt_c[0] + nlt_c[1]*T + nlt_c[2]*T*T;
    return C;

}
/**********************************************************************/
bool NonLinearThermal::solveNLTransientThermalProblem(double initialTemperature,double timeStepSize,double simulationTime,bool solveElastic,bool (*callBackFunction)(double nrmError))
{
    int totalTimeSteps;
    double tempMaxPrev;
    double minVonMises,maxVonMises;
    double *forceVecTemp;

    forceVecTemp = NULL;
    h_nodeBasedAssembly = false;
    nlt_isTransient = true;

    allocateTransientThermalMemory();
    setInitialTemperature(initialTemperature);
    set_thermalStaticSol(tt_sol);
    computeElementTemperature();// copy this over to element temperature

    tt_deltaTime = timeStepSize;//time step
    tt_time = simulationTime;//  total time
    initializeKEnonLinearThermal();
    totalTimeSteps = tt_time/tt_deltaTime;

    //addInternalHeatGenerationToThermalForce(); // Add internal heat source if present
    tempMaxPrev = 0;

    int nNodes = getNumNodes();
    if(solveElastic){
        forceVecTemp = (double *)malloc(3*nNodes*sizeof(double));
        computeNLThermalForceTemplate();
        getForceVector(forceVecTemp);
    }

    if (ws_deflationType !=  DEFLATION_NONE) {// deflation
        writeMessageToParetoLog("Computing thermal deflation transient non linear\n");
        CreateDeflationGroups();
    }

    //addVolumetricHeatSource();


    for(int timeStep = 1; timeStep<= totalTimeSteps;timeStep++) {
        if (!callBackFunction(1.*timeStep/totalTimeSteps)){// can place other data in user function
            writeMessageToParetoLog("TransientThermal: Terminating on user request\n");
            return false;
        }
        writeMessageToParetoLog("******Non linear Transient: Step %d/%d",timeStep,totalTimeSteps);
        ComputeNLThermalEffectiveForce(timeStep);

        solveNLThermalProblemPicard(50,timeStep);

        computeTemperatureMax();
        tt_min = h_temperatureMin;
        tt_max = h_temperatureMax;


        setTransientThermalSolution(h_sol_thermal);
        //updateThermalPhaseOfElements();
        //if ((tempMaxPrev - tt_max) < 0.001) break;
        tempMaxPrev = tt_max;

        // elasticity
        if(solveElastic){
            addNonLinearThermalForce();
            solveElasticityStatic();
            postprocess();
            minVonMises = getMinVonMisesStress();
            maxVonMises = getMaxVonMisesStress();
            cout <<"Min VonMises : " <<minVonMises << "  ; maxVonMises : "<<maxVonMises<<endl<<"------------------------------- \n";
            setForceVector(forceVecTemp);
        }
    }
    if(solveElastic){
        pushInitialStaticSolutionToTessellation();
        safeFree(forceVecTemp);
    }
    safeFree(nlt_numNeighbouringElemsOfNodeOnEvolvingSurface);
    releaseCPUMemoryTransientThermal();
    return true;
}
/******************************************************************************/
bool NonLinearThermal::Kreduction_NLThermal()
{
    int  group2, group1, node1, node2;
    double T,T_old,conductivityOfElem,specificHeatOfElem;
    int node_i, node_j, elemID,KEindex,nTemplateIDs;
    double K_val,xnode1,ynode1,znode1,xnode2,ynode2,znode2,additionalTerm;
    bool atLeastOneElementExists;

    if (ws_nGroups <= 0) return false;
    KDeflatedThermal.resize(4*ws_nGroups,4*ws_nGroups);
    KDeflatedThermal.setZero(4*ws_nGroups,4*ws_nGroups);

    nTemplateIDs = getNumTemplateElements();
    atLeastOneElementExists = false;

    for (int elem = 0; elem < h_nElems; elem++) {
        T = getTemperatureInElem(elem);
        T_old = getPreviousTemperatureInElem(elem);
        if(nlt_isTransient){
            conductivityOfElem = getNonLinearConductivityAtTemp(T,T_old);
            specificHeatOfElem = getNonLinearSpecificHeatAtTemp(T,T_old);
        }
        else {
            conductivityOfElem = getNonLinearConductivityAtTemp(T,T_old);
        }
        if (getElemPseudoDensity(elem) <0.5) continue; // element exists but is void (example: during topology optimization)
        atLeastOneElementExists = true;
        elemID = getTemplateId(elem);
        for (node_i = 0; node_i < 8; node_i++) {
            //node of a single voxel
            node1 = h_elems[node_i*h_nElems + elem];
            if (!h_free_thermal[node1]) continue;// node is fixed (thermally)
            if (!doesNodeExist(node1)){
                writeMessageToParetoLog("Error in Kreduction\n");
                return false;
            }
            group1 = ws_nodeGroupNumber[node1]; // Group number on mesh

            xnode1 = (h_x[node1] - h_CGPatch[3*group1 + 0]);
            ynode1 = (h_y[node1] - h_CGPatch[3*group1 + 1]);
            znode1 = (h_z[node1] - h_CGPatch[3*group1 + 2]);

            if ((group1 < 0) || (group1 >= ws_nGroups)){
                writeMessageToParetoLog("Kreduction_NLThermal: invalid group %d for node %d\n",group1,node1);
                continue;
            }
            for (node_j = 0; node_j < 8; node_j++) {
                node2 = h_elems[node_j*h_nElems + elem];
                if (!h_free_thermal[node2]) continue;// node is fixed (thermally)
                if (!doesNodeExist(node2)){
                    writeMessageToParetoLog("Error in Kreduction\n");
                    continue;//should not come here
                }
                group2 = ws_nodeGroupNumber[node2];

                if ((group2 < 0) || (group2 >= ws_nGroups)){
                    writeMessageToParetoLog("Kreduction_NLThermal: invalid group %d for node %d\n",group2,node2);
                    continue;
                }
                xnode2 = (h_x[node2] - h_CGPatch[3*group2 + 0]);
                ynode2 = (h_y[node2] - h_CGPatch[3*group2 + 1]);
                znode2 = (h_z[node2] - h_CGPatch[3*group2 + 2]);

                KEindex = (8*node_i + node_j)*(nTemplateIDs) + elemID;

                if(nlt_isTransient)
                    K_val =(tt_beta*conductivityOfElem*h_KETemplates_thermal[KEindex] )+ (specificHeatOfElem*nlt_CETemplates_thermal[KEindex]/tt_deltaTime);
                else
                    K_val =(conductivityOfElem*h_KETemplates_thermal[KEindex] );

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
    if (!atLeastOneElementExists) {
        writeMessageToParetoLog("Kreduction_NLThermal: All elements are void");
        return false;
    }
    /// add the lumped (diagonal) convection terms
    for (node1 = 0; node1 < h_nNodes; node1++) {
        if (!h_free_thermal[node1]) continue;// node is fixed (thermally)
        if (!doesNodeExist(node1)){
            continue;
        }
        group1 = ws_nodeGroupNumber[node1]; // Group number on mesh

        xnode1 = (h_x[node1] - h_CGPatch[3*group1 + 0]);
        ynode1 = (h_y[node1] - h_CGPatch[3*group1 + 1]);
        znode1 = (h_z[node1] - h_CGPatch[3*group1 + 2]);

        additionalTerm = tt_beta*h_convection[node1] + tt_beta*h_radiationNonLinear[node1];

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
    if (h_thermo_elasticity_verbose)
        writeMessageToParetoLog("NonLinearThermal: Size of KDeflatedThermal: (%d, %d)",KDeflatedThermal.rows(),KDeflatedThermal.cols());

    if (h_thermo_elasticity_verbose)
        writeMessageToParetoLog("NonLinearThermal: Max value in KDeflatedThermal: (%g)",maxValue);

    return true;
}
/**********************************************************************/
//Allocate double* WKW, and check whether ws_nodeGroupNumber is in place
bool NonLinearThermal::sparseWKW_NLThermal()// Neumann condition assumed, diriclet applied after K*u
{
    if (h_thermo_elasticity_verbose)
        writeMessageToParetoLog("NonLinearThermal: In sparseWKW_NLThermal");

    if (!Kreduction_NLThermal()) return false;

    Eigen::SparseMatrix<double>	 sparseWKWThermal = KDeflatedThermal.sparseView();
    sparseSolverThermal.compute(sparseWKWThermal);
    if (sparseSolverThermal.info() != 0) {
        return false;
    }
    return true;
}
/**********************************************************************/
bool NonLinearThermal::InitializeNonLinearPhaseChangeData(int isPhaseChange,double latentHeat, double Tms, double Tml){

    nlt_latentHeat = latentHeat;
    nlt_isPhaseChange = isPhaseChange;
    nlt_solidusTemp = Tms;
    nlt_liquidusTemp = Tml;


    if(nlt_isPhaseChange == 1){
        nlt_phaseStateOfElem = (double*)malloc(sizeof(double)*(h_nElems));
        memset(nlt_phaseStateOfElem,0,h_nElems*sizeof(double));
        return true;
    }
    return false;
}
/**********************************************************************/
double NonLinearThermal::getThermalPhaseOfElem(int elem){
    if(nlt_isPhaseChange == 0)
        return 0;
    else
        return nlt_phaseStateOfElem[elem];

}
/**********************************************************************/
int NonLinearThermal::isElementChangingPhase(double T){
    if(nlt_isPhaseChange == 0)
        return 0;
    if( (T > nlt_solidusTemp) && (T < nlt_liquidusTemp) )
        return 1;
    return 0;
}
/**********************************************************************/
void NonLinearThermal::updateThermalPhaseOfElem(int elem){
    // update the phase of the elements based on the temperature
    if(nlt_isPhaseChange == 0)
        return;
    double T;


    T = getTemperatureInElem(elem);
    if(T > nlt_liquidusTemp)
        nlt_phaseStateOfElem[elem ] = 1;
    else if ( T < nlt_solidusTemp)
        nlt_phaseStateOfElem[elem] = 0;
    else
        nlt_phaseStateOfElem[elem] = (T - nlt_solidusTemp)/(nlt_liquidusTemp - nlt_solidusTemp);

    return;
}
/**********************************************************/
void NonLinearThermal::updateThermalPhaseOfElements(){
    // update the phase of the elements based on the temperature
    if(nlt_isPhaseChange == 0)
        return;
    double T;

    for(int elem = 0 ; elem < h_nElems ; elem ++){
        T = getTemperatureInElem(elem);
        if(T > nlt_liquidusTemp)
            nlt_phaseStateOfElem[elem ] = 1;
        else if ( T < nlt_solidusTemp)
            nlt_phaseStateOfElem[elem] = 0;
        else
            nlt_phaseStateOfElem[elem] = (T - nlt_solidusTemp)/(nlt_liquidusTemp - nlt_solidusTemp);
    }
    return;
}
/**********************************************************/
void NonLinearThermal::computeNLThermalForceTemplate(){
    double dx,dy,dz,Y,nu,rho,alpha,specificHeat,K;
    char materialName[256];
    dx = h_dx;
    dy = h_dy;
    dz = h_dz;


    getMaterialOfPart(0,&Y,&nu,&rho,&K,&alpha,&specificHeat,materialName);

    alpha = 1.0; // for nonlinear dependence of thermal expn coeff on temperature
    //obtained from MatLab by doing triple integration of (dJ*B'*D*alpha*dummyvector')
    nlt_thermalForceTemplate[0] = (Y*alpha*dy*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[1] = (Y*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[2] = (Y*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[3] = (Y*alpha*dy*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[4] = (Y*alpha*dy*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[5] = (Y*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[6] = (Y*alpha*dy*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[7] = (Y*alpha*dy*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[8] = (Y*alpha*dx*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[9] = (Y*alpha*dx*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[10] = (Y*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[11] = (Y*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[12] = (Y*alpha*dx*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[13] = (Y*alpha*dx*dz)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[14] = (Y*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[15] = (Y*alpha*dx*dz*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[16] = (Y*alpha*dx*dy)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[17] = (Y*alpha*dx*dy)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[18] = (Y*alpha*dx*dy)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[19] = (Y*alpha*dx*dy)/(nu*8.0-4.0);
    nlt_thermalForceTemplate[20] = (Y*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[21] = (Y*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[22] = (Y*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);
    nlt_thermalForceTemplate[23] = (Y*alpha*dx*dy*(-1.0/4.0))/(nu*2.0-1.0);

}
/**********************************************************/
void	NonLinearThermal::setNonLinearThermalExpansionCoeffs(float alpha[3]){
    for(int i = 0 ; i < 3 ; i++)
        nlt_thermalExpansionCoeffs[i] = alpha[i];

    return;
}
/**********************************************************/
double NonLinearThermal::getThermalExpansionCoeffAtTemperature(double T){
    double alpha;
    alpha = nlt_thermalExpansionCoeffs[0] + nlt_thermalExpansionCoeffs[1]*(T) + nlt_thermalExpansionCoeffs[2]*(T)*(T);

    return alpha;
}
/**********************************************************/
void NonLinearThermal::addNonLinearThermalForce(void)
{
    double elemTemp,T0,alpha;
    int elemnodes[8];

    if (!h_voxelgrid){
        writeMessageToParetoLog("ThermoElasticity::addThermalForce only implemented for voxel grid");
        return;
    }
    T0 = h_thermoElasticReferenceTemperature;// reference temperature
    for(int elem = 0; elem<h_nElems ; elem++){
        if (getElemPseudoDensity(elem) < 0.5)
            continue;
        getElemNodes(elem,elemnodes);
        elemTemp = h_sol_thermalElem[elem];
        alpha = getThermalExpansionCoeffAtTemperature(elemTemp);
        for(int j = 0; j<8 ; j++) {
            if (isNodeFree(elemnodes[j])) {
                h_f[3*elemnodes[j]]   += nlt_thermalForceTemplate[j]*alpha*(elemTemp-T0);
                h_f[3*elemnodes[j]+1] += nlt_thermalForceTemplate[j+8]*alpha*(elemTemp-T0);
                h_f[3*elemnodes[j]+2] += nlt_thermalForceTemplate[j+16]*alpha*(elemTemp-T0);
            }
        }
    }
}
/**********************************************************/
bool NonLinearThermal::setSolveThermoElasticity(bool solveThermoElasticity){
    nlt_solveThermoElastic = solveThermoElasticity;
    return solveThermoElasticity;
}
/**********************************************************/
bool NonLinearThermal::setInternalHeatGenerationVolumetric(double Q){
    if(fabs(Q) > 0 ){
        nlt_isVolumetricHeatSourcePresent = true;
        nlt_volumetricHeatSource = Q;
        return true;
    }
    nlt_volumetricHeatSource = 0;
    return false;
}
/**********************************************************/
void NonLinearThermal::addVolumetricHeatSource(){
    if(nlt_isVolumetricHeatSourcePresent == false)
        return;
    // get the net heat in terms of W
    double Q;
    Q = nlt_volumetricHeatSource*getNumElems()*h_dx*h_dy*h_dz; // Watts
    // divide the heat among all nodes
    for(int node = 0 ; node <h_nNodes; node++){
        if(h_free_thermal[node])
            h_f_thermal[node] += Q/h_nNodes;
    }
}
/**********************************************************************/
bool	NonLinearThermal::findNumNeighbouringElemsOfNodeOnEvolvingBoundary(int node)

{	
    // intially set that it is at its intial stage as geom mesh was loaded
    int numActiveNeigbors = 0;
    int  codedElemNum,nodeIndex,elem;
    // then process every neighbor of the node
    int maxElem = 8; // it could be connected to a max of 8 elems
    for (int j =0; j < maxElem; j++) {
        codedElemNum = h_neighborElems[(h_nNodes)*j + node];// process elem-j neighbor
        if(codedElemNum == -1) continue; // the mesh neighbor doesn't exist
        nodeIndex = codedElemNum/MAXELEMS;
        elem =codedElemNum - nodeIndex*MAXELEMS;
        if(getElemPseudoDensity(elem) < 0.5)
            // if the pseudodensity of a sorrounding elem is 0, decrease the num of neigbors by 1
            numActiveNeigbors--;
        if(getElemPseudoDensity(elem) >= 0.5)
            numActiveNeigbors++;
        // if it has only 1 neighbor -> corner , 2 neighbors -> edge (3-7) -> face 8 -> interna;

    }
    nlt_numNeighbouringElemsOfNodeOnEvolvingSurface[node] = numActiveNeigbors;
    if(numActiveNeigbors == 8)
        return false; // return false if it is in the interior
    else
        return true;

}
/**********************************************************************/
double NonLinearThermal::getExposedNodeAreaFraction(int node)
{
    if(nlt_numNeighbouringElemsOfNodeOnEvolvingSurface[node] == 1) // if node is on the corner
        return 3;
    else if(nlt_numNeighbouringElemsOfNodeOnEvolvingSurface[node] == 2)  //edge
        return 4;

    else
        return 4;
}
/**********************************************************************/
void NonLinearThermal::applyConvectionOnExposedSurface(double h, double TRef) 
{
    double cornerArea,a;
    a = (h_dx+h_dy+h_dz)/3.0;// average edge length
    cornerArea = 0.25*(a*a);

    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if ( findNumNeighbouringElemsOfNodeOnEvolvingBoundary(node)  == false ) continue;// node is neither on actual boundary nor on evolving boundary, skip
        h_f_thermal[node] += (double) getExposedNodeAreaFraction( node)*cornerArea*h*TRef;
        h_convection[node] = (double) getExposedNodeAreaFraction( node)*cornerArea*h;
    }
}
/**********************************************************************/
void NonLinearThermal::applyRadiationOnAllExposedNodes(double emissivity) 
{
    h_isThereRadiation = true;
    double stefanBoltzmannConstant = 5.67e-8;// W/m2-K4
    double cornerArea,a;
    a = (h_dx+h_dy+h_dz)/3.0;// average edge length
    cornerArea = 0.25*(a*a);

    for (int node = 0; node < h_nNodes; node++) {
        if (!doesNodeExist(node)) continue;
        if (!h_free_thermal[node]) continue; // Dirichlet has been applied, skip
        if ( findNumNeighbouringElemsOfNodeOnEvolvingBoundary(node)  == false ) continue;// node is neither on actual boundary nor on evolving boundary, skip
        h_radiation[node] = (double)  getExposedNodeAreaFraction( node)*cornerArea*emissivity*stefanBoltzmannConstant;
    }
}
/**********************************************************************/
