#ifndef COMMONGUI_H
#define COMMONGUI_H

#define UNITS_METER 0
#define UNITS_INCH 1
#define UNITS_MM 2
#define UNITS_CM 3

#define ALLOY_STEEL 0
#define Ti6Al4V 1
#define ABS_PLASTIC 2
#define AL_1060 3
#define CASTIRON_GR25 4
#define AMS5862 5
#define OTHER_MATL 6

// The code is compiled in one of the following modes
// Don't change the number of BATCH_MODE number; it affects linux compilation
#define RESEARCH_MODE 1
#define BATCH_MODE 5

#define COMPILE_MODE RESEARCH_MODE

#define ROTATE 1
#define TRANSLATE 2
#define ZOOM 3


enum PARETO_GUI_MODE
{
	PARETO,
	MICROTO,
	THERMOELASTICITY,
	MULTI_MATERIAL,
	TO_FOR_AM,
};

struct DOEFiber {
	int fiberFillPercent;
	float fiberLengthMicrons;
	float fiberDiaMicrons;
	float  coneAngleDegrees;
};
struct LiveVariables {
// Names of files, etc
	char *sciartHomeDirectory;
	char *projectDirectory;
	char *prjFileName;
	char *voxFileName;
	char *modelName;
	char *(geomFileName[1000]);
	char *reportFolder;
	char  userName[256]; // used in batch processing
	char *userMessage;

	int DOSWindowCheck;
	bool DOSWindowOpen;
	
	// Verbose
	int stlVerbose;
	int voxVerbose;
	int hexmeshVerbose;
	int deflationVerbose;
	int thermalVerbose;
	int topoptVerbose;
	

//Units, etc
	int paretoUnits;// 0 is meters, 1 is inches, 2 is mm
	double LScale; // for converting between meters and inches
	int usingGUI; // means use using gui, else command line (batch mode)
	PARETO_GUI_MODE usingGUIMode;// which GUI mode
	int assemblyMode;
	int nSTLFilesLoaded;
	int selectedPart;

//Material
	int material;
	float E;// Young's modulus
	float nu;//Poisson raio
	float Y; // Yield strength
	float rho;//density
	float alpha;// coefficient of expansion
	float K;// thermal conductivity
	float specificHeat;//

	int nonDesignCheck;

//Boundary conditions Elastic
	int faceType;//triangle, plane, coarse cyl, ...
	int cutoffAngle;///degrees to find recursive mark neighboring triangles
	int currentBCSet;
	int nBCSet;
	int normalToSurface;
	int ParetoBCType;
	float XForce,YForce,ZForce;
	float XAcceleration,YAcceleration,ZAcceleration;
	float radiansPerSecZAxis;// radians per seconds
	float Pressure;
	float NormalForce;
	float Torque;
	int CurrentElasticityBCLabel;
	int	CurrentThermalBCLabel;
	int nFacesSelected;

//Boundary conditions Thermal
	int ThermalBCType;
	float ThermalBCValue;// could be dirichlet value, flux value, heat transfer coefficient, radiation coefficient
	float  ThermalBCReference;// for convection and radiation

// FEA
	int conformingMesh;
	int	meshQuality;
	int nElemsToMesh; 
	float maxAspectRatio;
	int xSym,ySym,zSym;
	int partialvoxel;
	double densityCutoff;// for voxelization, typically 0.5
	int useBubbleFunctions;
	int elemSignatureTolPower;
	int useNonLinearThermal;

	int useGPU;
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
	double estimatedTimeForTopOpt;
	
//Modal
	int EigenMethod;
	int useEigenStatic;
	int nEigenModes;
	double *eigenValues;
	double **eigenModes;


// Display variables
	int displayGeom;
	int displayField;
	int displayMode;
	int viewMode;
	int cuttingPlane;
	float cuttingPlaneValue;
	int plotAxis;
	int showTriangles;
	int plotAnimation;
	int plotElasticityBC;// with geometry
	int plotThermalBC;// with geometry
	int plotSymmetryPlanes; //with geometry
	int displayEigenMode;
	int plotOverGeom; // if 0, plot stresses, etc over mesh
	int boundingBox;
	int displayEdges;
	int geomTransparent;
	int plotPrimaryMesh;
	int plotTitle;
	int plotContours;
	int plotNormals;
	int plotSupportStructures;
	int tiffDPI;
	bool showGeometry;
	int showConcaveEdges;
	float plotElementsWithDensityGreaterThan;
	int plotNonDesignGeometry;// during topology optimization, some of the bodies that are not optimized can be displayed
	int text3DFontType; // 0 is GLUT RASTER, 1 is text3d and 2 wglUseFontOutlines

// Transient
	float newmarkGamma;
	float newmarkBeta;
	float dampingM;
	float dampingK;
	int		transientTolerance;
	float  initialTemperature;
	float  transientDeltaTime;
	float  transientTotalTime;
	char *transientTrackPtStr;

	int sensorNumber;

//TopOpt
	int useAllLoads;
	int topoptObjective;
	float pNorm;
	int drawDirection;
	int imposedBuildDirection;// for additive manufacturing constraint
	int throughCutDirection;
	int cyclicSymmetry;
	float maxDisplacementAllowed;
	float stressSafetyFactor;
	int	nSmoothingIterations;
	int maxFEA;

	bool isTopOptComplete;
	bool activeBCSets[100];// which of the bc sets are active for topology optimization

	float desiredVolFraction;
	float stepSize;
	float displayVolFraction;
	float	maxTimeAllowed;//Seconds
	float modelScale;
	int keepDirichlet;
	int saveIntermediate;
	char *stlFileName;
	float snapToGeomRelativeTol;
	int	invertedIsoApproach;
	int	imposedOverHangAngle;//degrees
	int	saveImagesForReport;

// iso surface extraction
	int isoSurfaceExtractionMethod;

// Lattice structure
	int latticeType;
	int latticeUnitCellSizeInMM; //mm
	int latticeResolution;
	float latticeFillRatio;
	int latticeSurfaceRetainType;
	int latticeSmoothening;
	int latticeSaveSTL;

// contour extraction
	int smootheningMethod;// 0 is non area preserving, 1 is area preserving
	int contourFeatureTolerance; // to get rid of entire contours
	int contourSimplificationTolerance;// to get rid of points on contour, relative to mesh size, 0 means don't simplify, 
	int contourSmoothening;// how many times
	int contourOffset;// relative to mesh size,
	
	DOEFiber doeFiber;
};


enum PARETO_GUI_STATE
{
    GROUND, 
	GEOM_SUCCESFUL,
	MATERIAL_SUCCESFUL,
	BC_SUCCESFUL,
	HEX_SUCCESFUL,
	THERMALFEA_SUCCESFUL,
	STATICFEA_SUCCESFUL,
	MODALFEA_SUCCESFUL,
	BUCKLINGFEA_SUCCESFUL,
	TRANSIENT_SUCCESFUL,
	TOPOPT_SUCCESFUL
};



#define DUMMY_BUTTON 0
#define LOADBREP_BUTTON 1
#define LOADGEOM_BUTTON 2
#define BCSET_SPINNER 3
#define MATERIAL_BUTTON 5
#define READVOXEL_BUTTON 6
#define SAVEPROJECT_BUTTON 7
#define LOADPROJECT_BUTTON 8
#define SOLVE_ELASTOSTATICS_BUTTON 9
#define DISPLAYFE_BUTTON 11
#define SOLVEMODAL_BUTTON 12
#define PLOT_TS_BUTTON 13
#define solveElasticTransient_BUTTON 14
#define OPTIMIZE_BUTTON 15
#define STL_QUALITY_SPINNER 16
#define SAVE_STL_BUTTON 17
#define PLOT_PARETO_BUTTON 18
#define HELP_BUTTON 19
#define LOADABAQUSINP_BUTTON 20
#define FIELDCORRECTION_BUTTON 21
#define ANALYZE_MICROSTRUCTURE_BUTTON 22
#define DELETECONGRUENCE_BUTTON 23
#define CHECKCONGRUENCE_BUTTON 24
#define IMPROVECONGRUENCE_BUTTON 25
#define SHOW_INACCESSIBLE 26
#define CREATEREPORT_BUTTON 27
#define SAVE_BMP_BUTTON 28
#define EIGENSTATIC_BUTTON 29
#define PLOT_CGITERATIONS_BUTTON 30
#define PLOT_TRANSIENT_DISP_BUTTON 31
#define PLOT_TRANSIENT_STRESS_BUTTON 32
#define COMPARESPEED_BUTTON 33
#define SHIFTCELL_BUTTON 34
#define STACKCELL_BUTTON 35
#define RESETCELL_BUTTON 36
#define CONVERGENCE_BUTTON 37
#define DROPTEST_BUTTON 39
#define COMPUTEDAMPING_BUTTON 40
#define PLOT_CGCONVERGENCE_BUTTON 41
#define SOLVEEXACT_BUTTON 43
#define RECOMPUTEBC_BUTTON 44
#define APPLY_THERMAL_BC_BUTTON 45
#define SOLVE_THERMOSTATICS_BUTTON 46
#define CREATEMESH_BUTTON 49
#define OPTIMIZE_MICROSTRUCTURE_BUTTON 50
#define LDC_BUTTON 51
#define CONFORMMESH_BUTTON 53
#define LOCALCONFORMING_BUTTON 54
#define ELASTIC_POLYNOMIAL_BUTTON 55
#define SAVEMESH_BUTTON 56
#define SOLVE_THERMOELASTIC_BUTTON 57
#define RESET_DISPLAY_BUTTON 58
#define SOLVE_TRANSIENT_THERMAL_BUTTON 59
#define SOLVE_TRANSIENT_THERMOELASTICITY_BUTTON 60
#define MULTIMATERIAL_OPTIMIZE_BUTTON 61
#define LOAD_DMATRICES_BUTTON 62
#define SOLVEBUCKLING_BUTTON 63
#define RELOADGEOM_BUTTON 64
#define PLOT_TRANSIENT_TEMPERATURE_BUTTON 65
#define THERMAL_OPTIMIZE_BUTTON 66
#define EXTRACT_CONTOURS_BUTTON 67
#define SAVE_TIF_BUTTON 68
#define PLOT_SENSORDATA_BUTTON 69
#define OPTIMIZE_SUPPORT_BUTTON 70
#define LOADMESH_BUTTON 71
#define APPLY_NEWELASTICITY_BC_BUTTON 72
#define ADAPTIVE_FEA_BUTTON 73
#define SAVEGEOM_AS_OBJ_BUTTON 74
#define APPLY_ACCELERATION_BUTTON 75
#define CREATE_SKELETON_BUTTON 76
#define SAVE_SURFACE_MESH_BUTTON 77
#define REMOVE_NONMANIFOLD_ELEMENTS_BUTTON 78
#define SAVE_MESH_BUTTON 79
#define CREATE_LATTICE_MESH_BUTTON 80
#define SOLVE_TOOLPATH_BUTTON 81
#define CREATE_TEMPLATE_LATTICE 82
#define SIMP_OPTIMIZE_BUTTON 83
#define DOE_FIBER_THERMAL_ANALYSIS 84


#define DISPLAY_NOTHING -1
#define DISPLAY_GEOM 0
#define DISPLAY_BC 1
#define DISPLAY_MESH 2
#define DISPLAY_INITIAL_DELTA 3
#define DISPLAY_INITIAL_STRESS 4
#define DISPLAY_TEMPERATURE 5
#define DISPLAY_MODAL 6

#define DISPLAY_ISO 7
#define DISPLAY_TSFIELD 8
#define DISPLAY_CONTOUR 9
#define DISPLAY_DELTA_OVER_MESH 10
#define DISPLAY_MESHERROR 14
#define DISPLAY_ISO_INVERTED 15
#define DISPLAY_SPHEREMAP 16
#define DISPLAY_MESH_COMPONENTS 17
#define DISPLAY_TEMP_ON_MESH 18
#define DISPLAY_STRESS_OVER_MESH 19
#define DISPLAY_PSEUDODENSITY_MESH 20
#define DISPLAY_SUPPORTSTRUCTURE_MESH 21
#define DISPLAY_FINAL_STL 22
#define DISPLAY_LATTICESTRUCTURE 23
#define DISPLAY_DEFLATION_GROUPS 24
#define DISPLAY_MESH_SURFACE 25
#define DISPLAY_MESH_SURFACE_PSEUDODENSITY 26
#define DISPLAY_FINAL_DELTA 27
#define DISPLAY_FINAL_STRESS 28


#define DISPLAY_GEOM_INITIALSTL 0
#define DISPLAY_GEOM_INITIALMESH 1
#define DISPLAY_GEOM_FINALSTL 2
#define DISPLAY_GEOM_FINALMESH 3

#define DISPLAY_FIELD_NONE 0
#define DISPLAY_FIELD_TOTALDEFORMATION 1
#define DISPLAY_FIELD_VONMISES 2
#define DISPLAY_FIELD_TEMPERATURE 3


#define MESH_VERY_COARSE 0
#define MESH_COARSE 1
#define MESH_MEDIUM 2
#define MESH_FINE 3
#define MESH_VERY_FINE 4

#define VIEW_YZMINUS 6

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


#define PARETO_BC_TEMP 20
#define PARETO_BC_HEAT 21
#define PARETO_BC_CONVECTION 22
#define PARETO_BC_RADIATION 23


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
#endif
