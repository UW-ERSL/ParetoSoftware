#ifndef COREFUNCTIONS_H
#define COREFUNCTIONS_H

#include "ParetoApplication.h"
#include "Voxelizer.h"
#include "LiveVariables.hpp"
#include "taubinSmooth.h"

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <omp.h>
#include <conio.h>
#include <process.h>
#include <string>

#define QKEY 81
#define strcmpCaseInsenstive strcmpi

#define CLIENTFILES "clientFiles\\%s"
#define CLIENTFILES_STL "clientFiles\\%s_STL.%.3f.stl"

#else
#define strcmpCaseInsenstive strcasecmp
#define CLIENTFILES "clientFiles/%s"
#define CLIENTFILES_STL "clientFiles/%s_STL.%.3f.stl"

#endif

using Array3D = std::array<double,3>;
using Array3I = std::array<int,3>;
class CoreFunctions {
public:

    bool    loadGeom(bool assembly = false);
    bool    loadTemporaryGeom(bool assembly = false);// typically for final (cleaned-up) geometry
    bool    updateGeom(bool assembly = false);
    bool    distanceFunctionIsosurface(void);
    void    resetLiveVariables();
    void    resetLiveVariablesTopOptConstraints();
    void    oneTimeInitialization();
    void    initializeLiveVariables();
    //void    changeMaterial(void);
    void    saveMesh(void);
    void    savePRJ(bool withResults = false);
    int     getRecommendedDeflationGroups(int nElems);
    void    applyMaterial(void);
    void    applyElasticityBC(void);
    void    applyThermalBC(void);
    void    createVoxelMesh(const char *voxFile);
    bool    createTopOptMesh(void);
	bool    readMeshAndAllocate(const char *voxFileBinary);
    bool    computeZernikeCoefficients(int gridSize, int ZernikeOrder, string outFile);
    bool    generateResultsFromFinalMesh(string finalMeshName);
    void    createLatticeStructureMesh(void);
    bool    solveModal(void);
    bool    identifyCandidateBuildDirections(void);
    bool    solveStatic(void);
    bool    solveStaticWithMaxTemplates(void);

    bool    solveElastoPlastic(void);
    bool    solveThermoElastic(void);
    bool    solveThermoTransient(void);
    bool    optimizeThermoStatic(void);
    bool    solveThermoStatic(void);
    bool    solveFrequencyResponse(void);
    bool    approximateConvexDecompositionOfTempGeometry(void);
    bool    approximateConvexDecomposition(void);
    bool    singleLoadOptimize(void);
    bool    singleLoadOptimizeSIMP(void);
    bool    autoDesign( string offFile);
    bool    projectIsosurfaceMeshOntoCAD(double relativeProjectionDistance, double relaticeRemeshDensity);
    bool    mergeCADandIsosurfaceMesh(double CADControlParam = 0.9,double IsosurfaceControlParam = 1.5);
    bool    simplifyNegativeSubtractAndSaveResult(void);
    void    saveSimplifiedIsoSurface(int isoCounter, int simplificationLevel, string stlFile);

    bool    uniformRemeshing(vector<Array3D> &coords, vector<Array3I> &triangles, int nTrianglesDesired = -1.0, double creaseAngle = 10);

    bool    singleLoadOptimize_multiscale(void);
    bool    singleLoadThermalOptimize(void);

    bool    multiLoadOptimize(void);
    bool    createTetmesh(void);
    bool    singleLoadOptimize_MultiMaterial(void);
    void    loadPRJ(const char *extension);
    void    getMaterialDataAsString(string &materialData);
    void    getPropertyUnits(int propertyID, string &propertyName);
    void    setTemperatureUnits(int tempUnits);
    void    setAngleUnits(int angleUnits);
    void    dumpPointCloud(string fileName);
    double  convertAngleToRadians(double angle);
    void    changeTopOptBC(void);
    bool    verifyNeumannConditions(void);
    bool    verifyDirichletConditions(void);
    bool    verifySymmetryConditions(void);
    void    SaveAsTIFF(const char *fileName);
    void    SaveAsBMP(const char *fileName);
    bool    checkMeshingError(void);
    bool    unBond(void);

private:
    Timer   timer;


    bool    checkForFiniteForce(void);
    void    freeEigenModesMemory(void);
    void    safeFree(void *ptr);
    void    allocateMemoryForEigenModes(void);
    void    eliminateEmptyGroups(void);

    bool    hasMaterialBeenApplied(void);


    void    initializeTopOptVariables(void);
    void    setTopOptConstraints(void);
    void    getMaxDeformationAndStressOverMesh(double *maxDeformation,double *maxStress);
    void    retainSurfacesForMultiload(void);
    int     find_word(FILE *filein,const char *word);
    void    setTopOptConstraints_Multimaterial(void);
};
#endif // COREFUNCTIONS_H
