// Copyright (C) Pelikan Software Kft.	All rights reserved.
// http://www.pelikansoftware.com/

#ifndef _WIN32
#define __stdcall
#endif
#ifndef _0A44DED7_40D4_484F_8469_5849CF3D2E4B_
#define _0A44DED7_40D4_484F_8469_5849CF3D2E4B_

#pragma pack(push,1)

/*
 3D boolean API Interface,
PREFACE
 This API is wrapper around FinalMesh SDK. Purpose of this wrapper is in providing simplified and more stable version of internal API.

Basic concept.

*/

namespace ivbool3d{

struct Point3f
{
	float x,y,z;
        Point3f(){};
	Point3f(float X,float Y,float Z){x=X;y=Y;z=Z;}
};

struct Point3d
{
	double x,y,z;
	Point3d(){};
	Point3d(double X,double Y,double Z){x=X;y=Y;z=Z;}
};

struct p2d
{
	float x,y;
	p2d(){};
	p2d(float X,float Y){x=X;y=Y;}
};

// output trinagle
struct Tri
{
  unsigned int v[3];
};

struct Face
{
	unsigned *v;
	unsigned *n;
	unsigned *uv0;
	unsigned *uv1;
	unsigned *uv2;
	unsigned *uv3;
	unsigned *holes;
	unsigned size;
	unsigned mtlId;
	unsigned numHoles;
};

class Object{
public:
	void*lParam;
	virtual int __stdcall Release()=0;// returns value after decrement
	virtual int __stdcall AddRef()=0;// returns value after increement

	virtual bool __stdcall SetTM(const double*,int size=16)=0;
	// Set object global transformation matrix. Size may be 12 or 16
	virtual bool __stdcall SetPoints(const Point3f*points,unsigned numPoints)=0;
	virtual bool __stdcall SetNormals(const Point3f*n,unsigned count)=0;
	virtual bool __stdcall SetUV(const p2d*uv,unsigned count)=0;

	virtual bool __stdcall	SetNumFaces(unsigned count)=0;
	virtual unsigned __stdcall NewFace(unsigned faceSize,const unsigned *ids)=0;
	virtual bool __stdcall SetHole(unsigned iFace,unsigned holeIndex)=0;

	virtual bool __stdcall SetMtlID(unsigned iFace,unsigned mdlId)=0;
	virtual bool __stdcall SetNormalFace(unsigned iFace,const unsigned *v)=0;
	virtual bool __stdcall SetUVFace0(unsigned iFace,const unsigned *v)=0;
	virtual bool __stdcall SetUVFace1(unsigned iFace,const unsigned *v)=0;
	virtual bool __stdcall SetUVFace2(unsigned iFace,const unsigned *v)=0;
	virtual bool __stdcall SetUVFace3(unsigned iFace,const unsigned *v)=0;


	//get only
	virtual bool __stdcall GetPoints(const Point3f**v,unsigned &count)=0;
	virtual bool __stdcall GetNormals(const Point3f**n,unsigned&count)=0;
	virtual bool __stdcall GetUV(const p2d**uv,unsigned&count)=0;
	virtual bool __stdcall GetFace(unsigned iFace,Face*)=0;
	
	virtual unsigned __stdcall GetNumFaces()=0;
	virtual bool __stdcall SetPointsD(const Point3d*points,unsigned numPoints)=0;
	virtual bool __stdcall GetPointsD(const Point3d**v,unsigned &count)=0;
};


enum boolean_mode{
	M_MERGE, // merge triangles in one object, no intersection, no clipping - nothing. May be used for test purposes.
	M_OR,M_AND,M_SUB_AB,M_SUB_BA,
	M_CUT_AB,// used when first argument is non closed surface.
	M_CUT_BA,// used when second argument is non closed surface.
	M_TEST,	// test combination of internal flags
	M_CUT_AB_neg,// negative result of M_CUT_AB
	M_UNION=M_OR// just different name
};

enum BOOL_FLAGS
{
	EXTRA_INFO=1,		// Obsolete
	SINGLE_THREAD=2,	// Obsolete. Used for debugging reasons
	REDUCE=4,			// Execute reducer modifier.
	DEBUG_LOG=8,		// Log debug information to console of attached debugger
	POLYGONISE=16,		// Execute polygonization modifier.
	POLYGONISE_HOLES=32,// If set holes in polygons will be detected and converted to holes. may be used if POLYGONISE flag is set. 
	NORMALS=64,			// If set, nomrals for source object will be generated. Default crease angle will be used. If custom angle is required, please contact support.
	TRIANGULATE=128,	// Triangulate source mesh. This flag works independently and may be used if you want to just triangulat source object.
};

typedef bool ( __stdcall*KEEPGOINGPROC)(void*);

class Processor
{
public:
	virtual bool __stdcall GetPrecision()=0;// true for double version, false - for float. Reserved for future
	virtual Object* __stdcall AddSource(int iChannel,Object*obj=NULL)=0;// add source object. where channel is 0 or 1. Each channel may have many objects. obj is optional, may be result of previous operation
	virtual bool __stdcall Process(boolean_mode mode,unsigned int flags)=0;
	// where flags is OR combination of BOOL_FLAGS
	virtual bool __stdcall Interrupt()=0;
	// Reserved for future version. Must be called from another thread.
	virtual Object* __stdcall GetResult()=0;
	// retrieve result object.
	virtual bool __stdcall SetCallback(KEEPGOINGPROC,void*lParam)=0;//reserved for future
	virtual bool __stdcall SetReduceProps(double vJoin,double angleTreshold,double edgeTreshold,double maxEdgeSize=1e6)=0;// absolute values
	// where vJoin  - join points closer than specified distance - this is to remove almost the same point - because of math error - not a part of reducer itself
	// angleTreshold  && edgeTreshold are the main parameters
	// angleTreshold - radians, should be very small about 0.1 of deg. 
	// edgeTreshold - collapse edges, smaller than required details
	// maxEdgeSize - protect large edges and do not combine them with each other.
	virtual bool __stdcall SetMaxPolySize(unsigned maxPoly)=0;
	// Used by polygonizer in order to limit maximem number of points in polygon.
	virtual bool __stdcall dbgSetSaveFileName(const wchar_t*)=0;
	// Used for debugging purposes. Call this function before ->Process and .fmesh file will be saved. This file may be loaded into finalmesh.

	virtual void __stdcall Reset()=0;// remove source and result objects. Prepare Processor for next operation.
	virtual void __stdcall Destroy()=0;// destroy Processor object

	inline Object*NewObject(){return AddSource(-1,0);}
};

};


typedef void ( __stdcall*IV3DINIT ) ();
typedef void ( __stdcall*IV3DDONE ) ();
typedef ivbool3d::Processor* ( __stdcall*IVBOOLCREATEPROC )();

#pragma pack(pop)
#endif
