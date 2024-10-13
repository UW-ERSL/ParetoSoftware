#include "SurfVoxels.h"
#include <fstream>
#include <algorithm>
#include <cassert>

using namespace std;


template<typename T>
std::istream & read_binary(std::istream& stream, T& value) {
    return stream.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template<typename T>
std::istream & read_binary(std::istream& stream, std::vector<T>& buf) {
    return stream.read(reinterpret_cast<char*>(&buf[0]), sizeof(T)*buf.size());
}

////////////////////////////////////////////////////////////////////////////////

void SurfVoxels::readSTL(const string &filename)
{
    ifstream ifile(filename.c_str(), ios::in);
    if( ifile.fail() ) {
	    cout << "Warning: Could not open stl file " << filename << endl;
	    return;
    }

    string str;
    ifile >> str;
    ifile.close();

    if( str == "solid")
        read_Ascii_STL( filename);
    else
        read_Binary_STL( filename);

    size_t numTriangles  = vCoords.size()/3;
    faces.resize( numTriangles );
    size_t index = 0;
    for( size_t i = 0; i < numTriangles; i++) {
        faces[i][0] = index++;
        faces[i][1] = index++;
        faces[i][2] = index++;
    }
}

////////////////////////////////////////////////////////////////////////////////
void SurfVoxels::read_Binary_STL(const string &filename)
{
    ifstream ifile(filename.c_str(), ios::binary);
    if( ifile.fail() ) return ;

    ifile.seekg(0, ios::end);
    size_t fileSize = ifile.tellg();
    ifile.seekg(0, ios::beg);

    size_t numTriangles = (fileSize-84)/50.0;

    vector<char> header(80);
    read_binary( ifile, header);

    uint32_t nsize;
    read_binary( ifile, nsize);

    if( nsize != numTriangles) cout << "Warning: Corrupted STL file" << endl;

    vCoords.reserve(3*numTriangles);

    vector<float> buf(12);
    vector<char>  count(2);

    std::array<double,3> xyz;
    std::array<float,3>  nr;
    for( size_t i = 0; i < numTriangles; i++) {
        read_binary( ifile, buf);
        read_binary( ifile, count);
        size_t index = 0;
        for(int j = 0; j < 3; j++)
            nr[j] = buf[index++];
        for(int j = 0; j < 3; j++) {
            xyz[0] = buf[index++];
            xyz[1] = buf[index++];
            xyz[2] = buf[index++];
            vCoords.push_back(xyz);
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void SurfVoxels::read_Ascii_STL(const string &filename)
{
    ifstream ifile(filename.c_str());
    if( ifile.fail() ) return;
    string str;

    ifile >> str;
    assert( str == "solid");

    char stlname[255];
    ifile.getline(stlname, 255);

    std::array<double,3> xyz;
    std::array<float,3>  nr;
    while(1) {
        if( ifile.eof() ) break;
        ifile >> str;
        if( str == "facet") {
            ifile >> str;
            assert( str == "normal");
            ifile >> nr[0] >> nr[1] >> nr[2];
            ifile >> str;
            assert( str == "outer");
            ifile >> str;
            assert( str == "loop");
            for( int i = 0; i < 3; i++) {
                ifile >> str >> xyz[0] >> xyz[1] >> xyz[2];
                assert( str == "vertex");
                vCoords.push_back(xyz);
            }
            ifile >> str;
            assert( str == "endloop");
            ifile >> str;
            assert( str == "endfacet");
        }
    }
}

void SurfVoxels::readOFF(const string &filename)
{
    ifstream ifile(filename.c_str(), ios::in);
    if( ifile.fail() ) {
        cout << "Error: Can't open input file " << filename << endl;
    }

    string str;
    ifile >> str;

    int val = str.compare("OFF");

    if(val) {
        cout << "Error: input file not in the off format" << endl;
    }

    int numPoints, numFaces, numEdges;
    ifile >> numPoints >> numFaces >> numEdges;

    if( numPoints < 1) {
        cout << "Warning: Input file has no points " << endl;
    }

    vCoords.clear();

    Array3D xyz;
    if( numPoints ) {
        vCoords.resize(numPoints);
        for( size_t i = 0; i < numPoints; i++) {
            ifile >> xyz[0] >> xyz[1] >> xyz[2];
            vCoords[i] = xyz;
        }
    }

    Array3I tri;
    if( numFaces) {
        int dummy;
        faces.resize(numFaces);
        for( size_t i = 0; i < numFaces; i++) {
            ifile >> dummy >> tri[0] >> tri[1] >> tri[2];
            faces[i] = tri;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
int SurfVoxels::readMesh(const string &filename)
{

   if( filename.rfind(".off") != string::npos) readOFF(filename);
   if( filename.rfind(".stl") != string::npos) readSTL(filename);
   if( filename.rfind(".STL") != string::npos) readSTL(filename);

   voxData.empty();
   return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool SurfVoxels::checkOverlap( double triVerts[3][3], int i, int j, int k)
{
    double boxCenter[3];

    boxCenter[0] = origin[0] + (i+0.5)*voxelSize;
    boxCenter[1] = origin[1] + (j+0.5)*voxelSize;
    boxCenter[2] = origin[2] + (k+0.5)*voxelSize;

    double boxHalfSize[3];
    boxHalfSize[0] = 0.5*voxelSize;
    boxHalfSize[1] = 0.5*voxelSize;
    boxHalfSize[2] = 0.5*voxelSize;

    bool result = triBoxOverlap(boxCenter, boxHalfSize, triVerts);
    return result;
}
////////////////////////////////////////////////////////////////////////////////

void SurfVoxels::checkOverlap(size_t faceid)
{
    double triVerts[3][3];

    // Construct the Axis-Aligned Bounding Box of the triangle ...
    for( int i = 0; i < 3; i++) {
        int  vid = faces[faceid][i];
        Array3D xyz    = vCoords[vid];
        triVerts[i][0] = xyz[0];
        triVerts[i][1] = xyz[1];
        triVerts[i][2] = xyz[2];
    }

    Array3D minCorner, maxCorner;
    minCorner[0] = numeric_limits<double>::max();
    minCorner[1] = numeric_limits<double>::max();
    minCorner[2] = numeric_limits<double>::max();

    maxCorner[0] = -0.99*minCorner[0];
    maxCorner[1] = -0.99*minCorner[1];
    maxCorner[2] = -0.99*minCorner[2];

    for( int i = 0; i < 3; i++) {
        minCorner[0] = min( minCorner[0], triVerts[i][0] );
        minCorner[1] = min( minCorner[1], triVerts[i][1] );
        minCorner[2] = min( minCorner[2], triVerts[i][2] );

        maxCorner[0] = max( maxCorner[0], triVerts[i][0] );
        maxCorner[1] = max( maxCorner[1], triVerts[i][1] );
        maxCorner[2] = max( maxCorner[2], triVerts[i][2] );
    }

    Array3I ijk0, ijk1;
    getIJK( minCorner, ijk0);
    getIJK( maxCorner, ijk1);

    int i0 = ijk0[0] - 1;
    int j0 = ijk0[1] - 1;
    int k0 = ijk0[2] - 1;

    int i1 = ijk1[0] + 1;
    int j1 = ijk1[1] + 1;
    int k1 = ijk1[2] + 1;

    for( int k = k0; k < k1; k++) {
        for( int j = j0; j < j1; j++) {
            for( int i = i0; i < i1; i++) {
                if(checkOverlap(triVerts, i,j,k) ) {
                    size_t index = k*voxDim*voxDim + j*voxDim + i;
                    voxData[index] = 1;
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void SurfVoxels:: initMesh()
{
    Array3D  minval, maxval;
    minval[0] = minval[1] = minval[2] = std::numeric_limits<double>::max();
    maxval[0] = maxval[1] = maxval[2] = -0.99*minval[0];

    size_t numPoints = vCoords.size();
    for( size_t i = 0; i < numPoints; i++) {
        auto xyz = vCoords[i];
        for( int j = 0; j < 3; j++) {
            minval[j] = std::min( minval[j], xyz[j] );
            maxval[j] = std::max( maxval[j], xyz[j] );
        }
    }

    double xlen = maxval[0] - minval[0];
    double ylen = maxval[1] - minval[1];
    double zlen = maxval[2] - minval[2];

    double maxlen = std::max({xlen,ylen,zlen});

    voxelSize = maxlen/(double)voxDim;
    for( int j = 0; j < 3; j++)
        origin[j] = minval[j] - 2*voxelSize;

    voxDim += 4;
}

///////////////////////////////////////////////////////////////////////////////

vector<bool> SurfVoxels::getDenseBuffer()
{
    if( voxData.empty() ) {
        initMesh();
        size_t numVoxels = voxDim*voxDim*voxDim;
        voxData.resize(numVoxels, 0);
        size_t numTriangles = faces.size();
        for( size_t i = 0; i < numTriangles; i++) {
            checkOverlap(i);
        }
    }

    return voxData;
}

///////////////////////////////////////////////////////////////////////////////

