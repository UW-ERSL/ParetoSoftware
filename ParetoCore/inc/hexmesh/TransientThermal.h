#ifndef TRANSIENT_THERMAL_H
#define TRANSIENT_THERMAL_H
#include "ThermoElasticity.h"
  

class TransientThermal:  public ThermoElasticity {

	protected:// inherited classes can access these variables
			bool   tt_verbose;
			double tt_beta ; //Trapezoidal rule factor
			double tt_deltaTime ; // Time step seconds
			double tt_time; //Total time of simutt_Latention in seconds
			double tt_max;// spatially max value of temp
			double tt_min;//spatially min value of temps
			double *tt_sol;//Solution at each time step
			double *tt_Keff;//Keffective TransientThermal
			double *tt_KPrimeEff;//Misnomer - it is Actually the K multilplied with Tn to obtain Reff
			double *tt_Feff;//Reffective TransientThermal Keff*Tn+1 = Reff 
			double *tt_Aeff;//tt_Aeff = tt_KPrimeEff*Tn
			double *tt_forcePrevious;// h thermal at previous time step
			double *tt_forceCurrent;// h thermal at current time step
			double *tt_forceCurrentWithRadiation;// h thermal at current time step

			//Phase Change Variables// 
			double *tt_x;//fraction of liquid at every node Used in isothermal
			double Cs,Cl,Hm,Hl,Linear; //specific heat of solid
			double tt_Tms;//solid to molten
			double tt_Tml;// molten to liquid
			double tt_Latent;//tt_Latentent hear


            void  allocateTransientThermalMemory(void);
			void  ComputeCE_TT(double dx, double dy, double dz, double p, double c, double Ce_TT[8][8]);
			bool  computeKEffKPrimeEffThermal(double deltaTime);
			
			void  releaseCPUMemoryTransientThermal(void);

			double computeThermalFluxAtTimeStep(int timeStep);
            void    ComputeTransientThermalFeff(int timeStep,bool Secondtime = false);
            void    ComputeTransientThermalFeffLinear(int timeStep);
            void    setTransientThermalSolution(double *K);

			
			bool  solveNonlinearThermalNewtonRaphson(int maxIter,int timeStep);

			//Phase Change Functions// 
			void PhaseChangeInitialization(void);
			void PhaseChangeCorrectionIsothermal(void);
			void PhaseChangeCorrectionNonIsothermal(void);



public:
			TransientThermal();
            void  setInitialTemperature(double T0);
			void  getMinMaxTempTransient(double *tmax, double *tmin);
			void  enableTransientThermalVerbose(void);
			void  disableTransientThermalVerbose(void);
            int   solveTransientThermoElasticProblem(double initialTemperature,double *initialTemperatureDistribution,double deltaTime, double totalTime,bool solveElastic,bool (*userFunction)(double) );
			 ~TransientThermal(void);
};

#endif
