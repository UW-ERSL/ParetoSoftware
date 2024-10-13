#pragma once

#include <array>
#include <vector>
#include <memory>
#include <string>

#include "veclib.hpp"

class DelaunaySurface
{
public:
    void setPoints( const std::vector<Array3D> &coords);
    void setPoints( const std::vector<Array3D> &coords, const std::vector<Array3D> &normals);
    void readTetmesh(const char *nodeFile,const char *eleFile);
    void extractSurface(double featureSize);
    void getSurface(std::vector<Array3D> &coords, std::vector<Array3I> &faces);
    void saveSurfaceAsOffFile( const std::string &f);
    void buildMeshTopology(void);
    void saveAllTriangles( const std::string &filename, double edgeLengthCutoff);
private:

    class Node;
    using NodePtr = std::shared_ptr<Node>;

    class Face;
    using FacePtr = std::shared_ptr<Face>;

    class Cell;
    using CellPtr = std::shared_ptr<Cell>;

    struct Node
    {
        Array3D  xyz;
        int      id;
        std::vector<FacePtr> faces;
        std::vector<CellPtr> cells;
    };

    struct Face
    {
        std::array<NodePtr,3> nodes;
        std::array<CellPtr,2> cells;

        bool hasNode( const NodePtr &vquery) const
        {
            for( int i = 0; i < 3; i++) if( nodes[i] == vquery) return 1;
            return 0;
        }

        bool isSame( const NodePtr &v0, const NodePtr &v1, const NodePtr &v2) const
        {
            if( !hasNode(v0) ) return 0;
            if( !hasNode(v1) ) return 0;
            if( !hasNode(v2) ) return 0;
            return 1;
        }

        bool isBoundary() const {
            if( cells[1] == nullptr) return 1;
            if( !cells[1]->active )  return 1;
            return 0;
        }

        int addCell( const CellPtr &c)
        {
            if( cells[0] == nullptr) {
                cells[0] = c;
                return 0;
            }
            if( cells[1] == nullptr) {
                cells[1] = c;
                return 0;
            }
            return 1;
        }
    };

    struct Cell
    {
        std::array<NodePtr,4> nodes;

        FacePtr getFaceAt(int iface)
        {
            auto getFace = [] (const NodePtr &v0, const NodePtr &v1, const NodePtr &v2)
            {
                auto vmin = JMath::min_value(v0, v1, v2 );

                for( auto f : vmin->faces)
                    if( f->isSame(v0, v1, v2)) return f;

                auto newface = std::make_shared<Face>();
                newface->nodes = {v0, v1, v2};
                vmin->faces.push_back(newface);
                return newface;
            };

            switch(iface)
            {
            case 0:
                return getFace( nodes[0], nodes[1], nodes[2]);
            case 1:
                return getFace( nodes[0], nodes[1], nodes[3]);
            case 2:
                return getFace( nodes[0], nodes[2], nodes[3]);
            case 3:
                return getFace( nodes[1], nodes[2], nodes[3]);
            }
        }

        bool active = 1;
	int  orient = 0;
    };

    struct Mesh
    {
        std::vector<NodePtr> nodes;
        std::vector<FacePtr> faces;
        std::vector<CellPtr> cells;

    };

    std::vector<Array3D> inPoints;
    std::vector<Array3D> pNormals;
    std::vector<Array3I> newTriangles;

    Mesh tetmesh;
    bool isOutside(const CellPtr &tet) const;

};

