#ifndef MULTISCALE_H
#define MULTISCALE_H

#include "SIMP.h" // import Elasticity functions

#define MULTISCALE_VERBOSE false // use this to print to console

class MultiScale: virtual public SIMP {
	public:
                    MultiScale();
                    ~MultiScale();

            bool    solveMultiScale(void);
            void    updateDensitySIMP_general(int nElems,double *simpDensity,double *simpSensitivity,double *volPerSimpDensity,bool *keepElem, double desiredVolFrac,double move,double damp, double minDensity);
            bool    getPrincipalStrainAndVectorOfElem(int elem,double principalStrains[3],double principalVectors[3][3]);
            bool    kMeans(double *data,int dataSize[2],int nClusters, int *elemToClusterMap, double *centers,int distanceType, bool initialized);
            void    initializeForMicrostructure(void);
            bool    computeMicrostructureEquivalentD(double *equivalentD,double *gradDelem);
            void    enableMultiScaleVerbose();
            void    disableMultiScaleVerbose();
            void    generateStiffnessMatrixTemplates();
            void    performSIMPpEquals1Optimization(double volFrac, double *density);
            bool    perturbCuboidMeshDensityToVolFraction(double volFraction, double *density);
            bool    assignElemStiffMatBasedOnClusters(bool rotation,double *equivD,double *allEigenVectors,int *elemToClusterMapping);
            bool    computeMicroElemSensitivity(bool rotation,double *microSensitivities,double *allEigenVectors,int *elemToClusterMapping,double *gradD,int mi_nElems,int nClusters);
            void    addPlaceHolderMaterial(int nMaterials);
            void    freezeEdgesOfMicrostructure(bool *keepMicroElems,int nClusters,int mi_nElems);
            void    printMicrostructures(double *microPseudoDensity,int nClusters,int iter);
    private:
      const int     VoigtMap[6][2] = {{0,0},{1,1},{2,2},{0,1},{0,2},{1,2}};
            bool    ms_verbose;
            int     dMatrixTemplateIndices[21];
            double  elemStiffMatrixTemplates[576*21];

	protected:



		
};

#endif // end code inclusion
