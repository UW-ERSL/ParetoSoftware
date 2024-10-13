#ifndef TAUBINSMOOTH_H
#define TAUBINSMOOTH_H

#include "primitives.h"

#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <iomanip>
using std::setiosflags;

#include <ios>
using std::ios_base;
using std::ios;

#include <set>
using std::set;

#include <vector>
using std::vector;

#include <limits>
using std::numeric_limits;

#include <cstring> // for memcpy()
#include <cctype>


class indexed_mesh
{
public:
	void clear(void)
	{
		triangles.clear();
		vertices.clear();
		vertex_to_triangle_indices.clear();
		vertex_to_vertex_indices.clear();
		vertex_normals.clear();
		triangle_normals.clear();
	}

	vector<indexed_triangle> triangles;
	vector<vertex_3> vertices;
	vector< vector<size_t> > vertex_to_triangle_indices;
	vector< vector<size_t> > vertex_to_vertex_indices;
	vector<vertex_3> vertex_normals;
	vector<vertex_3> triangle_normals;

	bool load_from_binary_stereo_lithography_file(const char *const file_name, const bool generate_normals = true, const size_t buffer_width = 65536);
	bool save_to_binary_stereo_lithography_file(const char *const file_name, const size_t buffer_width = 65536);
	bool save_to_povray_mesh2_file(const char *const file_name, const bool write_vertex_normals = false);
	bool save_to_obj_file(const char *const file_name);
	bool save_to_extended_off_file(const char *const file_name);
	void scaleModel(double scale);

	int getNumberOfTriangles(void);
	void getTriangleNodes(int t,int tri[3]);
	int getNumberOfVertices(void);
	void getVertex(int v,double *xyz);
	void setVertex(int v,double *xyz);
	bool moveVertexIfValid(int v,double *xyz,double normalChangeTolerance = 0.9);
	void getVertexNormal(int v,double *normal);

	void set_max_extent(float max_extent);


	void expand_surface(const float scale);
    void expand_surface_at_vertices(const float scale, bool *whichVertices);
	void laplace_smooth(const float scale);
	void taubin_smooth(const float lambda, const float mu, const size_t steps);

	void fix_cracks(void);

private:
	void generate_vertex_normals(void);
	void generate_single_vertex_normal(int v,double normal[3]);
	void generate_triangle_normals(void);
	void generate_vertex_and_triangle_normals(void);
	void regenerate_vertex_and_triangle_normals_if_exists(void);
	template<typename T> void eliminate_vector_duplicates(vector<T> &v);
	bool merge_vertex_pair(const size_t keeper, const size_t goner);
};


#endif
