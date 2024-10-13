#include "supportStructure.hpp"
#include <iostream>
using namespace std;

/**************************************************/
void SupportStructures::identifyOverhangTriangles(double overhangAngle)
{
    allocateMemorySupportStructure();

    // walk thru all the triangles --> in future, make it parallel
    int numTriangles = vox.getNumTriangles();
    double triNormal[3];
    double zAxisNormal[] ={0,0,1};
    double boundingBoxMin[3],boundingBoxMax[3];
    double triX[3],triY[3],triZ[3];
    double ZmaxOfTriangle;
    vox.getBoundingBox(boundingBoxMin,boundingBoxMax);
    cout<<"bounding box min is :"<<boundingBoxMin[0]<<";"<<boundingBoxMin[1]<<";"<<boundingBoxMin[2]<<";"<<endl;
    overhangAngle = coreFunctions.convertAngleToRadians(overhangAngle);
    cout<<"Overhang angle : "<<overhangAngle<<endl;
    int numOverhangTriangles = 0;
    double suppportArea = 0;
    for(int triangle = 0 ; triangle < numTriangles ; triangle++)
    {
        vox.getTriPts(triangle,triX,triY,triZ);
        ZmaxOfTriangle = *std::max_element(std::begin(triZ), std::end(triZ));
        cout<<"max Z of elem : "<<ZmaxOfTriangle<<endl;
        vox.getNormalOfTriangle(triangle,triNormal);
        //cout<<"Tri :"<<triangle<<"Normal : "<<triNormal[0]<<";"<<triNormal[1]<<";"<<triNormal[2]<<";"<<endl;
        if( ( DOTPROD3D(triNormal,zAxisNormal)  < cos(overhangAngle) ) && (ZmaxOfTriangle != boundingBoxMin[2]))
        {
            ss_overhangTriangles[triangle] = true;
            numOverhangTriangles++;
            cout<<numOverhangTriangles<<endl;
            vox.highlightTrianglesRecursive(triangle,0,triNormal,0,&suppportArea);

        }

    }

    cout<<"NUm tri req support : "<<numOverhangTriangles<<"/"<<numTriangles<<endl;
    cout<<"I am here!"<<endl;
    renderer.plotBooleanFieldOverSTLMesh(ss_overhangTriangles);
    emit overhangTriangleIdentified();


    clearMemorySupportStructure();
}
/**************************************************/
void SupportStructures::allocateMemorySupportStructure()
{
    int numTriangles = vox.getNumTriangles();
    ss_overhangTriangles = (bool *)malloc(numTriangles*sizeof(bool));
    memset(ss_overhangTriangles,0,numTriangles*sizeof(bool));
    cout<<"Num Triangles"<<numTriangles<<endl;
}

/**************************************************/
void SupportStructures::clearMemorySupportStructure()
{
    free(ss_overhangTriangles );
    ss_overhangTriangles = NULL;

}
