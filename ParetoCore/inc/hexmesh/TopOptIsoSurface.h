#ifndef TOPOPTISOSURFACE_H
#define TOPOPTISOSURFACE_H
#include "LatticeStructure.h"
#include <list>
#include <stdio.h>
class TopOptIsoSurface:  public LatticeStructure {
private:

	int		 findDuplicateIsoPt(int isoCounter,double pt[3]);

protected:// inherited classes can access these variables

	//for iso surfaces
	int			ts_isoNPoints[MAX_ISO_SAVES];
	int			ts_isoNTriangles[MAX_ISO_SAVES];
	double		*(ts_isoXYZ[MAX_ISO_SAVES]);//3*npoints
	double		*(ts_isoVertexNormals[MAX_ISO_SAVES]);//3*npoints
	double		*(ts_isoNormals[MAX_ISO_SAVES]);//3*nTriangles
	int			*(ts_isoTriangles[MAX_ISO_SAVES]);//3*nTriangles
	int			*(ts_isoTrianglesNeighbors[MAX_ISO_SAVES]);//3*nTriangles
	int			*(ts_isoPartId[MAX_ISO_SAVES]);//nTriangles

	//final static solution
	double			*(ts_isoUVWFinal);//9*nTriangles
	double			*(ts_isoVonMisesFinal);//3*nTriangles

    //final thermal solution
    double			*(ts_isoTemperatureFinal);//3*nTriangles

    //final modal solution
    double			*(ts_isoModalFinal);//9*nTriangles

	void	 assignPartNumberIsoSurfaceTriangles(int isoCounter);

public:
			TopOptIsoSurface();
	

	double   computeIsoSurfaceVolume(int isoCounter);
	bool	 dumpExtendedOffFile(const char *fileName, int isoCounter = -1);
    bool	 readExtendedOffFile(const char *fileName, int isoCounter =-1);
    bool	 readAndAppendFromExtendedOffFile(const char *fileName, int isoCounter =-1);
	void	 laplaceSmoothIsoSurface(const float scale,const int isoCounter);
	bool	 readIsoBinary(const char *fileName,int isoCounter = -1);
	bool	 dumpSolidWorksVRML(int isoCounter,const char *fileName,double scale = 1);
    int      dumpMeshWithTSfield(const char *fileName);
	bool	 dumpIsoOffFormat(int isoCounter,const char *fileName,double scale = 1);
	bool	 dumpIsoBinary(int isoCounter,const char *fileName,double scale = 1);
    bool	 createAndDumpIsoSurfaceFromTSfield(const char *fileName,int isoCounter =-1,bool invert = false);
	bool	 createAndDumpIsoSurfaceFromMesh(const char *fileName,int isoCounter=-1);
    bool	 createAndDumpNegativeIsoSurfaceFromMesh(const char *fileName,int isoCounter=-1);
    bool     createAndDumpIsoSurfaceFromPseudoDensity(const char *fileName,double cutoff= 0.5);
    bool	 createAndDumpIsoSurfaceFromMesh_MultiMaterial(const char *fileName,int materialToSave,int isoCounter=-1);
	bool	 pushFinalStaticSolutionToIsoSurface(void);
    bool	 pushFinalTemperatureToIsoSurface(void);
    bool	 pushFinalModalSolutionToIsoSurface(void);

    bool	setIsoNodeXYZ(int isocounter, int node, double xyz[3]);

    int	getNumIsoTriangles(int isoCounter);
    int	getNumIsoNodes(int isocounter);
	bool	getIsoTriNodes(int isocounter,int tri, int nodes[3]);
	bool	getIsoNodeXYZ(int isocounter, int node, double xyz[3]);
	bool	getIsoNodeNormal(int isocounter, int node, double normal[3]);
	void	getIsoTriPts(int isoCounter,int tri,double xTri[3],double yTri[3],double zTri[3],double triNormal[3],
    double  vertexNormals0[3], double vertexNormals1[3], double vertexNormals2[3]);
	void	getIsoTriNeighbors(int isoCounter,int tri,int triNeighbors[3]);
    int     getIsoTriPartNum(int isoCounter,int tri);

	bool	 getFinalIsoElasticitySolutionOfTriangle(int tri,double u[3],double v[3],double w[3],double vonMises[3]);
    bool     getFinalIsoTemperatureOfTriangle(int tri,double temperature[3]);
    bool	 getFinalModalSolutionOfTriangle(int tri,double u[3],double v[3],double w[3]);

    ~TopOptIsoSurface();
};

#endif
