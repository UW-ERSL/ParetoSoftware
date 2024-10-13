#ifndef SIMP_H
#define SIMP_H // Ensure SIMP is included only once

#include "TopOptForMultiMaterial.h" // import Elasticity functions

#define SIMP_VERBOSE false // use this to print to console

class SIMP: virtual public TopOptForMultiMaterial {
	public:
			SIMP();
    bool	solveSIMP(bool (*callbackFunction)(float percentCompleted) = NULL);
    void	solveSIMPPareto(bool (*callbackFunction)(float percentCompleted));
	void	setSIMPTargetVolumeFraction(double volfrac);
	void	setSIMPMinDensity(double minDensity);

    double	getSIMPMinDensity(void);
    double  getSIMPTimerInSeconds(void);
    void    getSIMPDensity(double *density);
			~SIMP();

	private:
    Timer   timer;
		double *simp_pseudoDensity;
		double *simp_sensitivity;
		double simp_minDensity;
		double simp_targetVolumeFraction;
        double  simp_densityChangeTerminate;
        double simp_lambdaPrev;
		void  allocateMemory(void);

	protected:
		void updateDensitySIMP();
		void computeSIMPSensitivity();
		void updateSIMPDensity();

		
};

#endif // end code inclusion
