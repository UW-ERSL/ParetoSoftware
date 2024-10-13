#pragma warning(disable:4996)
#include "VecMath.h"
#include "Simplify.h"

///////////////////////////////////////////
int test(int argc, char **argv) 
{ 
	// Setup Mesh for Simplification
	
	// Simplify::vertices[].p : vertex position
	// Simplify::triangles[].v[0..2] : vertex indices
	
	/*loopi(0,obj.objects[0].points.size())
	{
		Simplify::Vertex v;
		v.p=obj.objects[0].points[i];
		Simplify::vertices.push_back(v);
	}
	loopi(0,obj.objects[0].triangles.size())
	{
		Simplify::Triangle t;
		loopj(0,3)t.v[j]=obj.objects[0].triangles[i].points[j];
		Simplify::triangles.push_back(t);
	}*/
	printf("Input: %d triangles %d vertices\n",Simplify::triangles.size(),Simplify::vertices.size());
	
	// Simplification start
	
	Simplify::simplify_mesh(20000);
	Simplify::write_obj("../test_out.obj");
	
	// Simplification done;	
	// Visualize the Result
	
	printf("Output: %d triangles %d vertices\n",Simplify::triangles.size(),Simplify::vertices.size());
	
	/*loopi(0,Simplify::vertices.size())
	{
		obj.objects[0].points[i]=Simplify::vertices[i].p;
	}
	int ofs=0;
	loopi(0,Simplify::triangles.size())
	{
		if(!Simplify::triangles[i].deleted)
		{
			obj.objects[0].triangles[ofs] = obj.objects[0].triangles[i];
			obj.objects[0].triangles[ofs].material=0;
			loopj(0,3) obj.objects[0].triangles[ofs].points[j]=Simplify::triangles[i].v[j];
			ofs++;
		}
	}
	obj.objects[0].triangles.resize(ofs);*/

	return 1;

}