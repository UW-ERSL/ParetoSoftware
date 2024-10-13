#ifndef LIVEVARIABLES_H
#define LIVEVARIABLES_H
#include <string>
#include "TopOpt.h"
#include "STLGeom.h"
// show or hide various guis & messages
#define ACADEMIC_LICENSE                true
#define ONLY_FEA                        false
#define ONLY_MESH                       false
#define PARETO_RESEARCH                 false
#define VERBOSE_GUI                     true
#define MATERIAL_OPTIMIZAION_GUI        false
#define THERMAL_OPTIMIZATION_GUI        false
#define AUTODESIGN_GUI                  false
#define LATTICE_GUI                     false
#define AMBUILD_GUI                     false
#define ZERNIKE_GUI                     false
#define ALLOW_INDEPENDENT_RESTRAINTS    true

using namespace std;
struct TopOptConstraints{
    bool isExtrudeActive;
    bool isAMBuildActive;
    bool isDrawDirectionActive;
    bool isxGridActive;
    bool isyGridActive;
    bool iszGridActive;
    bool isCyclicSymmetryActive;
    bool isMaxDisplacementActive;
    bool isStressSafetyActive;
    bool isMinFeatureSizeActive;
    bool isMaxFeatureSizeActive;
    bool keepTopologyConnected;
    bool isMinEigenActive;
    bool isTemperatureLimitActive;
    bool keepDirichlet;

    bool xSymmetry;
    bool ySymmetry;
    bool zSymmetry;

    int extrudeDirection;//  1: x, 2:y, 3: z
    int AMBuildDirection;//  1: x, 2:y, 3: z
    int drawDirection;//  1: x, 2:y, 3: z
    int xGridPattern; // 2,3,4,5...
    int yGridPattern;// 2,3,4,5...
    int zGridPattern;// 2,3,4,5...
    int cyclicSymmetry; // 2,3,4,5,...
    float maxDisplacementAllowed; // > 0
    float stressSafetyFactor; // > 1.0
    float minEigenValueAllowedHz;// > 0 in Hz
    float maxTemperatureAllowed;// > typically in K
    float minFeatureSize;// > 0
    float maxFeatureSize; // > 0
};

struct structuralDynamicsLiveVar{

    double alphaM;// damping
    double betaK;// damping
    double NewmarkGamma;
    double NewmarkBeta;
    double totalTime;
    double timeStep;
    double trackPoint[3];
    int    trackDOF; // 0, 1, 2
};

struct nonlinearLiveVar{
    int     nSteps;// how many steps
    int     nIterations;// per step
    double  tolerance; // termination convergence
};

struct LiveVariables
{

    // Names of files, etc
    string projectDirectory;
    string prjFileName;
    string meshFileName;
    string voxFileName;
    string modelName;
    string geomFileName[1000];
    string reportFolder;
    string  userName; // used in batch processing
    string stlFileName;
	string statusMessage;
    string graphicsText;
    FILE *logFilePtr;

    int DOSWindowCheck;
    bool DOSWindowOpen;

    // Verbose
    bool stlVerbose;
    bool voxVerbose;
    bool hexmeshVerbose;
    bool plasticityVerbose;
    bool deflationVerbose;
    bool thermalVerbose;
    bool topoptVerbose;

//Units, etc
    int paretoUnits;// 0 is meters, 1 is inches, 2 is mm
    int temperatureUnits; // added new live variable
    int timeUnits; //added new live variable to keep track of time units
    int angleUnits; // keep track of whether using radians or degrees
    double LScale; // for converting between meters and inches
    int usingGUI; // means use using gui, else command line (batch mode)

    bool assemblyMode;
    int nSTLFilesLoaded;
    int selectedPart;

// project settings variables;
    bool UNITS_APPLIED = false;
    bool MATERIAL_APPLIED= false;
    bool GEOMETRY_MODIFIED = false;
    bool GEOMETRY_UPLOADED= false;
    bool LATTICE_UPLOADED= false;
    bool BC_STRUCTURAL_APPLIED= false;
    bool BC_THERMAL_APPLIED= false;
    bool ACCELERATION_APPLIED= false;
    bool MESH_APPLIED= false;
    bool STATIC_FEA_SOLVED= false;
    bool THERMAL_FEA_SOLVED= false;
    bool MODAL_FEA_SOLVED= false;
    bool TRANSIENT_FEA_SOLVED= false;
    bool CONSTRAINTS_APPLIED= false;
    bool TOPOPT_SOLVED= false;
    bool CADIFIED_GEOMETRY_UPLOADED = false;// this is cleaned up geometry
    bool LATTICE_DESIGNED= false;
    bool GCODE_LOADED= false;
    int whichLoadSetElasticityFEASolved;

//Material
    int materialIndex;// from the CSV file
    float E;// Young's modulus
    float nu;//Poisson raio
    float Y; // Yield strength
    float rho;//density
    float alpha;// coefficient of expansion
    float K;// thermal conductivity
    float specificHeat;//
    float price;//
    bool nonDesignCheck;
    string materialName;
    //const char *materialNamesStr[7] = {"AlloySteel", "Ti6Al4V","ABSPlastic","Al1060","CastIronGr25","AMS5862","Other"};
    int numberOfMaterialIndicesStored; //for multimaterial optimization

//Boundary conditions Elastic
    int faceType;//triangle, plane, coarse cyl, ...
    int cutoffAngle;//degrees to find recursive mark neighboring triangles
    int currentBCSet;
    int nBCSet;
    int normalToSurface;
    int ParetoBCType;
    float XDisp,YDisp,ZDisp;
    float XForce,YForce,ZForce;
    float XAcceleration,YAcceleration,ZAcceleration;
    float radiansPerSecZAxis;// radians per seconds
    float Pressure;
    float BearingLoad;
    float NormalForce;
    float Torque;
    int CurrentElasticityBCLabel;
    int	CurrentThermalBCLabel;
    int nFacesSelected;

//Boundary conditions Thermal
    int ThermalBCTypeGUI;
    float ThermalBCValueGUI;// could be dirichlet value, flux value, heat transfer coefficient, radiation coefficient
    float  ThermalBCReferenceGUI;// for convection and radiation

// FEA
    int conformingMesh;
    int	meshQuality;
    int nElemsToMesh;
    float maxAspectRatio;
    bool includeThermalEffects;
    int partialvoxel;
    double densityCutoff;// for voxelization, typically 0.5
    int useBubbleFunctions;
    int elemSignatureTolPower;
    int useNonLinearThermal;


    int useGPU;
    int directSolver;
    int nodeBasedKU;
    int usePreconditioner;
    int nWorkSpaceGroups;
    int	workSpaceMethod;
    int cgResidualExponent;
    int cgMaxIter;
    int eigRelAccuracyExponent;
    int displayQuality;
    int forceRemesh;
    double timeForOneFEA;
    double timeForOneModalFEA;
    double estimatedTimeForTopOpt;

//Modal
    int EigenMethod;
    int nEigenModes;
    double *eigenValues;
    double **eigenModes;

    struct structuralDynamicsLiveVar structuralDynamics;

    struct nonlinearLiveVar nonlinear;
 // Elasto-plastic analysis
    int nElastoPlasticForceSteps;

// Display variables
    int displayGeom;
    int displayField;
    int viewMode;
    int cuttingPlane;
    float cuttingPlaneValue;
    int    xCuttingPlanePercent;
    int    yCuttingPlanePercent;
    int    zCuttingPlanePercent;
    bool   isDeformationScaled;// to scale deformation or not
    float   deformationScaled;//actual scale value
    vector<int> partsToHide;

    bool plotAnimation;
    bool plotAxis;
    bool plotElasticityBC;// with geometry
    bool plotThermalBC;// with geometry
    bool plotTopOptConstraints; //with geometry
    bool plotBoundingBox;
    bool geomTransparent;
    bool plotSummaryResults;
    int  displayEigenMode;
    bool displayEdges;
    bool plotNormals;
    bool plotSupportStructures;

    int     tiffDPI;
    bool    showNativeCADGeometry;// this is for SolidWorks, Creo, etc inter
    bool     showConcaveEdges;
    float   plotElementsWithDensityGreaterThan;
    bool     plotNonDesignGeometry;// during topology optimization, some of the bodies that are not optimized can be displayed


    float zeroStrainTemperature; // for thermo-elasticity
    float animationVariable;
    float staticAnimationScaling;
    float  modalAnimationScaling;
    int sensorNumber;

//TopOpt
    int useAllLoads;
    int topoptObjective;
    bool stopOptimization;
    bool useSIMPMethod;
    float pNorm;

    // all constraints are in one structure
    struct TopOptConstraints topoptConstraints;

    int maxFEA;
    bool isTopOptComplete;
    bool activeBCSets[100];// which of the bc sets are active for topology optimization

    float desiredVolFraction;
    float convergedVolFraction;
    float stepSize;//volume step size
    float displayVolFraction;
    float maxTimeAllowed;//Seconds
    float modelScale;

    bool saveIntermediate;

    bool  snapToGeom;// snap the topopt isosurface to original geometry
    float snapToGeomRelativeTol;
    int	imposedOverHangAngle;//degrees

 // Microstructures
    int noOfMicroStructureGroups;
    int numMicroElems;
    int typeOfClustering;

// iso surface extraction
    bool useTSFieldForIsoSurfaceExtraction;// if true, uses TS fueld mesh for isosurface; if false, uses voxel mesh

// autoDesign
    double autoDesignTopologyError; // for discarding small components and for checking symmetry between components
    double autoDesignSurfaceError; // for contour approximation
    int autoDesignContourOffset; // for offseting contours to include corners
    int autoDesignSmootheningIterations;// for smoothening contours

// Lattice structure
    int latticeType;
    int latticeUnitCellSizeInMM;
    int latticeResolution;
    float latticeFillRatio;
    int latticeSurfaceRetainType;
    int latticeSmoothening;
    int latticeSaveSTL;

    int BCType[MAX_BC_SET][STL_MAX_GROUPS][3];
    double BCValue[MAX_BC_SET][STL_MAX_GROUPS][6];//typically only the first 3 values are used except for torque
    double ElasticityAcceleration[MAX_BC_SET][4];// Linear acceleration (x,y,z) and //radians per second (omega) results in radial acceleration

    int ThermalBCType[MAX_BC_SET][STL_MAX_GROUPS];
    double ThermalBCValue[MAX_BC_SET][STL_MAX_GROUPS];// could be T (Dirichlet), q (flux), h (convection) or epsi (radiation) value
    double ThermalBCArea[MAX_BC_SET][STL_MAX_GROUPS];// keep track of all the areas
    double ThermalBCRef[MAX_BC_SET][STL_MAX_GROUPS];// for convection and radiation

    double ConstraintData[MAX_BC_SET][MAX_CONSTRAINTS][6]; //(upper/lower, hard/soft, limit value,x,y,z) // last three are optional
    int ConstraintType[MAX_BC_SET][MAX_CONSTRAINTS];
    int NumberOfConstraints[MAX_BC_SET]={0,0,0,0,0,0,0,0,0,0};

};

#define PARETO_BC_FIXED  0
#define PARETO_BC_FORCE  1
#define PARETO_BC_PRESSURE  2
#define PARETO_BC_SLIDING  3
#define PARETO_BC_RETAIN  4
#define PARETO_BC_NORMAL_FORCE  5
#define PARETO_BC_TORQUE  6
#define PARETO_BC_XFIXED  7
#define PARETO_BC_YFIXED  8
#define PARETO_BC_ZFIXED  9
#define PARETO_BC_CLEAR  10
#define PARETO_BC_BEARING_LOAD  11
#define PARETO_REMOTE_LOAD  12

#define PARETO_BC_TEMP 20
#define PARETO_BC_HEAT 21
#define PARETO_BC_CONVECTION 22
#define PARETO_BC_RADIATION 23



#define UNITS_METER 0
#define UNITS_INCH 1
#define UNITS_MM 2
#define UNITS_CM 3

// angles
#define UNITS_ANGLE_RADIAN 0
#define UNITS_ANGLE_DEGREE 1
 // temperature
#define UNITS_TEMPERATURE_CELCIUS 0
#define UNITS_TEMPERATURE_KELVIN 1
#define UNITS_TEMPERATURE_FAHRENHEIT 2

// time
#define TIME_UNIT_SECOND 0
#define TIME_UNIT_MINUTE 1
#define TIME_UNIT_HOUR 2
#define TIME_UNIT_DAY 3


enum display_geom {
    DISPLAY_GEOM_NONE = 0,
    DISPLAY_GEOM_INITIAL,
    DISPLAY_GEOM_TOPOLOGY,
    DISPLAY_GEOM_MESH,
    DISPLAY_GEOM_MESHCOMPONENTS,
    DISPLAY_GEOM_FINAL,
    DISPLAY_GEOM_LATTICE,
    DISPLAY_GEOM_CONTOURS,
    DISPLAY_GEOM_OTHER,
    DISPLAY_GEOM_GCODE,
    DISPLAY_GEOM_GCODE_FILAMENT,
    DISPLAY_GEOM_MESH_OTHER,

};

enum display_field {
    DISPLAY_FIELD_NONE = 0,
    DISPLAY_FIELD_DISPLACEMENT,
    DISPLAY_FIELD_STRESS,
    DISPLAY_FIELD_MODAL,
    DISPLAY_FIELD_TEMPERATURE,
    DISPLAY_FIELD_DISPLACEMENT_X,
    DISPLAY_FIELD_DISPLACEMENT_Y,
    DISPLAY_FIELD_DISPLACEMENT_Z,
    DISPLAY_FIELD_SIMP_DENSITY,
    DISPLAY_FIELD_TOPOLOGICAL_SENSITIVITY,
    DISPLAY_FIELD_MOISTURE,
};

#define MESH_VERY_COARSE 0
#define MESH_COARSE 1
#define MESH_MEDIUM 2
#define MESH_FINE 3
#define MESH_VERY_FINE 4


#define VIEW_ISO 0
#define VIEW_XYPLUS 1
#define VIEW_XYMINUS 2
#define VIEW_XZPLUS 3
#define VIEW_XZMINUS 4
#define VIEW_YZPLUS 5
#define VIEW_YZMINUS 6

enum CUTTING_PLANES {
    NO_CUTTINGPLANE = 0,
    XYPLUS_CUTTINGPLANE,
    XYMINUS_CUTTINGPLANE,
    XZPLUS_CUTTINGPLANE,
    XZMINUS_CUTTINGPLANE,
    YZPLUS_CUTTINGPLANE,
    YZMINUS_CUTTINGPLANE,
};

#define FACE_TRIANGLE 0
#define FACE_PLANE 1
#define FACE_FINE_CYL 2
#define FACE_COARSE_CYL 3
#define FACE_OTHER 4
#define FACE_ALL 5

#define DUMP_BASIC 0
#define DUMP_UVW 1
#define DUMP_VOXELMESH 2
#define DUMP_MODAL 3
#define DUMP_TOPOPT_ISO 4
#define DUMP_TEMP 5
#define DUMP_UVW_TEMP 6
#define DUMP_MESH 7
#define DUMP_GENESIS 8
#define DUMP_BND 9
#define DUMP_ABAQUS_RPT 10
#define DUMP_ISO_UVW 11
#define DUMP_UVW_NODES 12
#define DUMP_SIMP_DENSITY 13


#define STL_M 0
#define STL_MM 1
#define STL_INCH 2
#define STL_CM 3

#define PROPERTY_MASS 0
#define PROPERTY_TIME 1
#define PROPERTY_LENGTH 2
#define PROPERTY_YOUNG_MODULUS 3
#define PROPERTY_YIELD_STRENGTH 4
#define PROPERTY_ACCELERATION 5
#define PROPERTY_VELOCITY 6
#define PROPERTY_TEMPERATURE 7
#define PROPERTY_DENSITY 8
#define PROPERTY_SPECIFIC_HEAT 9
#define PROPERTY_THERMAL_EXPANSION 10
#define PROPERTY_THERMAL_CONDUCTIVITY 11
#define PROPERTY_FORCE 12
#define PROPERTY_PRESSURE 13
#define PROPERTY_WORK 14
#define PROPERTY_PRICE_PER_UNITMASS 15
#define PROPERTY_ANGLE 16
#define PROPERTY_VOLUME 17
#define PROPERTY_AREA 18
#define PROPERTY_HEAT_FLUX 19
#define PROPERTY_HEAT_COEFFICIENT 20

#define SPINBOX_MAX_WIDTH 75
#define GUI_MIN_WIDTH 250

#endif // LIVEVARIABLES_H
