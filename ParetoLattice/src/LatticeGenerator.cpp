#include "LatticeGenerator.hpp"


using namespace std;
#pragma warning(disable:4996)

extern void setStatusMessage(const char str[]);
extern void updateProgressBar(const char title[], int percentComplete);
extern void resetProgressBar(const char title[]);

/**********************************************************************/
LatticeGenerator::LatticeGenerator()
{
    lg_latticeType = LATTICE_BOX;
    lg_LatticeGraph.nodes.clear();
    lg_LatticeGraph.parentBody.clear();
    lg_LatticeGraph.struts.clear();
    lg_LatticeGraph.strutExists.clear();
    lg_LatticeGraph.boxMin[0] = lg_LatticeGraph.boxMin[1] = lg_LatticeGraph.boxMin[2] = 0;
    lg_LatticeGraph.boxMax[0] = lg_LatticeGraph.boxMax[1] = lg_LatticeGraph.boxMax[2] = 1;
    lg_LatticeGraph.modelScale =  1;
}
/**********************************************************************/
double    LatticeGenerator::getLatticeModelScale()
{
    return lg_LatticeGraph.modelScale;
}
/************************************************************/
bool    LatticeGenerator::setLatticeGeneratorXYZOfTrussNode(int node,double xyz[3])
{
    if ( (node < 0) || (node >= lg_LatticeGraph.nodes.size()))
        return false;

    for (int i = 0; i < 3; i++)
        lg_LatticeGraph.nodes[node][i] = xyz[i];
    return true;
}
/************************************************************/
bool    LatticeGenerator::setLatticeGeneratorParentBodyOfTrussNode(int node,int parentBody)
{
    if ( (node < 0) || (node >= lg_LatticeGraph.parentBody.size()))
        return false;

    lg_LatticeGraph.parentBody[node] = parentBody;
    return true;
}
/************************************************************/
bool  LatticeGenerator::scaleLatticeGraph(double xScale,double yScale,double zScale)
{
    if (lg_LatticeGraph.struts.size() == 0) return false;

    for (int node = 0; node < lg_LatticeGraph.nodes.size(); node++){
        lg_LatticeGraph.nodes[node][0] *= xScale;
        lg_LatticeGraph.nodes[node][1] *= yScale;
        lg_LatticeGraph.nodes[node][2] *= zScale;
    }
    return true;
}
/************************************************************/
bool  LatticeGenerator::setLatticeGeneratorRadiusOfAllStruts(double radius)
{
    if (lg_LatticeGraph.struts.size() == 0) return false;
    lg_LatticeGraph.radiusOfTrunkStrut.clear();
    lg_LatticeGraph.strutExists.clear();
    for (int strut = 0;strut<lg_LatticeGraph.struts.size();strut++){
        lg_LatticeGraph.radiusOfTrunkStrut.push_back(radius);
        lg_LatticeGraph.strutExists.push_back(true);
    }
    return true;
}
/************************************************************/
bool  LatticeGenerator::setLatticeGeneratorradiusOfTrunkStrut(int strut,double radius)
{
    if (lg_LatticeGraph.struts.size() == 0) return false;
    lg_LatticeGraph.radiusOfTrunkStrut[strut] = radius;
    lg_LatticeGraph.strutExists[strut] = true;

    return true;
}
/************************************************************/
bool  LatticeGenerator::isStrutSuppressed(int strut)
{
    return (!lg_LatticeGraph.strutExists[strut]);
}
/************************************************************/
bool  LatticeGenerator::suppressLatticeGeneratorStrut(int strut)
{
    if (lg_LatticeGraph.strutExists.size() == 0) return false;
    lg_LatticeGraph.strutExists[strut] = false;

    return true;
}
/************************************************************/
bool  LatticeGenerator::unSuppressLatticeGeneratorStrut(int strut)
{
    if (lg_LatticeGraph.strutExists.size() == 0) return false;
    lg_LatticeGraph.strutExists[strut] = true;

    return true;
}
/************************************************************/
int  LatticeGenerator::getLatticeType(void)
{
    return lg_latticeType;
}
/************************************************************/
double  LatticeGenerator::getLatticeGeneratorRadiusOfTrunkStrut(int strut)
{
    if (lg_LatticeGraph.struts.size() == 0) return 0;
    return lg_LatticeGraph.radiusOfTrunkStrut[strut];
}
/************************************************************/
int     LatticeGenerator::getLatticeGeneratorNumberOfStruts(void)
{
    return lg_LatticeGraph.struts.size();
}
/************************************************************/
int     LatticeGenerator::getLatticeGeneratorNumberOfNodes(void)
{
    return lg_LatticeGraph.nodes.size();
}
/************************************************************/
bool    LatticeGenerator::getLatticeGeneratorNodesOfStrut(int strut,int nodes[2])
{
    if ( (strut < 0) || (strut >= lg_LatticeGraph.struts.size()))
        return false;

    nodes[0] = lg_LatticeGraph.struts[strut][0];
    nodes[1] = lg_LatticeGraph.struts[strut][1];
    return true;
}
/************************************************************/
bool    LatticeGenerator::getLatticeGeneratorXYZOfTrussNode(int node,double xyz[3])
{
    if ( (node < 0) || (node >= lg_LatticeGraph.nodes.size()))
        return false;

    for (int i = 0; i < 3; i++)
        xyz[i] = lg_LatticeGraph.nodes[node][i];
    return true;
}
/************************************************************/
int    LatticeGenerator::getLatticeGeneratorParentBodyOfTrussNode(int node)
{
    if ( (node < 0) || (node >= lg_LatticeGraph.parentBody.size()))
        return 0;

    return lg_LatticeGraph.parentBody[node];
}
/************************************************************/
bool LatticeGenerator::computeLatticeGraphFromVoxelMesh(LATTICETYPE latticeType)
{
    int elemNodes[8],nNeighborsOfNode0,nNeighborsOfNode1,node0,node1;
    int elemsNeighborsOfNode0[100],elemsNeighborsOfNode1[100],nCommonElems,commonElems[100],neighborElement;
    // Do not change the order of these edges, the first 8 are on XY plane, and the last 4 are Z
    int edgePairs[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    // Do not change the order of these faces
    int faceNodeIndex[6][4] = {{3,0,4,7},{0,1,5,4},{0,3,2,1},{1,2,6,5},{2,3,7,6},{7,4,5,6}};// see h_StructuringElems for ordering
    int *faceNodeId = NULL;// the 6 or 24 (for Voronoi) face node ids for each element
    Point3D pt;
    bool *elemsProcessed,insertEdge,*elemsToProcess;
    LineSegment strutElement;
    int nNodesCurrent;
    double elemCenter[3];
    bool addCornerNodes,addFaceCenterNodes,addCenterNodes, addVoronoiFaceNodes;
    bool addZEdges,addXYEdges,addFaceEdges,addNonZFaceEdges,addBCCEdges,addOctetEdges,addVoronoiFaceEdges;


    lg_latticeType = latticeType;
    lg_LatticeGraph.nodes.clear();
    lg_LatticeGraph.struts.clear();

    if (getNumElems() == 0){
        writeMessageToParetoLog("Please create a mesh first");
        return false;
    }
    InitializeTopOpt(3);//clears data structures
    elemsProcessed = (bool *)malloc(getNumElems()*sizeof(bool));
    memset(elemsProcessed,false,getNumElems()*sizeof(bool));

    elemsToProcess = (bool *)malloc(getNumElems()*sizeof(bool));
    memset(elemsToProcess,true,getNumElems()*sizeof(bool));

    for (int elem = 0; elem < getNumElems(); elem++)
        if(!shouldPartBeOptimized( getPartIdOfElem(elem)))
            elemsToProcess[elem] = false;


    addCornerNodes = false;
    addFaceCenterNodes = false;
    addCenterNodes = false;
    addVoronoiFaceNodes = false;

    // different cases for node addition
    switch (latticeType){
        case LATTICE_BOX:       addCornerNodes = true; break;
        case LATTICE_BCC:       addCornerNodes = true;addCenterNodes = true; break;
        case LATTICE_BCCz:      addCornerNodes = true;addCenterNodes = true; break;
        case LATTICE_BOX_BCC:   addCornerNodes = true;addCenterNodes = true; break;
        case LATTICE_FCC:       addCornerNodes = true;addFaceCenterNodes = true;  break;
        case LATTICE_FCCz:      addCornerNodes = true;addFaceCenterNodes = true; break;
        case LATTICE_FCCBCC:    addCornerNodes = true;addFaceCenterNodes = true;  break;
        case LATTICE_OCTET:     addCornerNodes = true;addFaceCenterNodes = true; break;
        case LATTICE_PILLARS:   addCornerNodes = true;break;
        case LATTICE_NO_OVERHANG: addCornerNodes = true;addFaceCenterNodes = true; addCenterNodes = true; break;
        case LATTICE_VORONOI:   addVoronoiFaceNodes = true; break;
    }


    addZEdges = addXYEdges = addFaceEdges = addBCCEdges = addNonZFaceEdges = addOctetEdges = addVoronoiFaceEdges =false;


    // different cases for edge addition
    switch (latticeType){
        case LATTICE_BOX:       addZEdges = true; addXYEdges = true; break;
        case LATTICE_BCC:       addBCCEdges = true; break;
        case LATTICE_BCCz:      addZEdges = true; addBCCEdges = true; break;
        case LATTICE_BOX_BCC:   addZEdges = true; addXYEdges = true; addBCCEdges = true; break;
        case LATTICE_FCC:       addFaceEdges = true;  break;
        case LATTICE_FCCz:      addZEdges = true; addFaceEdges = true; break;
        case LATTICE_FCCBCC:    addFaceEdges = true; addBCCEdges = true; break;
        case LATTICE_OCTET:     addFaceEdges = true;addOctetEdges = true;break;
        case LATTICE_PILLARS:   addZEdges = true; break;
        case LATTICE_NO_OVERHANG:   addZEdges = true; addBCCEdges = true;addNonZFaceEdges = true; break;
        case LATTICE_VORONOI:   addVoronoiFaceEdges = true;break;
    }

    /* Add required nodes */
    if (addCornerNodes){
        for (int node = 0; node < getNumNodes(); node++){
            pt[0] = h_x[node];
            pt[1] = h_y[node];
            pt[2] = h_z[node];
            lg_LatticeGraph.nodes.push_back(pt);
        }
    }

    if (addCenterNodes){//nodes numbers follow the voxel nodes
        // create one central node per element
        for (int elem = 0; elem < getNumElems(); elem++){
            getElemCenter(elem,elemCenter);
            for (int i = 0; i < 3; i++)
                pt[i] = elemCenter[i];
            lg_LatticeGraph.nodes.push_back(pt);// add a new point
        }
    }

    if (addFaceCenterNodes){
        faceNodeId = (int *)malloc(6*getNumElems()*sizeof(int));
        memset(faceNodeId,0,6*getNumElems()*sizeof(int));
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        nNodesCurrent = lg_LatticeGraph.nodes.size();
        // create face-nodes (shared by two elements),
        for (int elem = 0; elem < getNumElems(); elem++){
            getElemNodes(elem,elemNodes);
            for (int face = 0; face < 6; face++){
                // for each face, find the corresponding neighboring element
                neighborElement = h_StructuringElems[h_StructuringSize*elem+face+1];
                // if the neighborElement has already been processed
                if ((neighborElement >= 0) && (elemsProcessed[neighborElement]) ){
                    int neighborFaceId;//faceId of the neighboring element
                    switch (face){
                        case 0: neighborFaceId = 3;break;
                        case 1: neighborFaceId = 4;break;
                        case 2: neighborFaceId = 5;break;
                        case 3: neighborFaceId = 0;break;
                        case 4: neighborFaceId = 1;break;
                        case 5: neighborFaceId = 2;break;
                    }
                    faceNodeId[6*elem + face] = faceNodeId[6*neighborElement + neighborFaceId];
                    continue;
                }
                //else add a new point
                pt[0] = pt[1] = pt[2] = 0.0;
                int node;
                for (int i = 0; i < 4; i++){
                    node = elemNodes[faceNodeIndex[face][i]];
                    pt[0] += h_x[node];
                    pt[1] += h_y[node];
                    pt[2] += h_z[node];
                }
                pt[0] /=4;
                pt[1] /=4;
                pt[2] /=4;

                faceNodeId[6*elem + face] =  lg_LatticeGraph.nodes.size();
                lg_LatticeGraph.nodes.push_back(pt);// add a new point
            }
            elemsProcessed[elem] = true;
        }
    }
    else if (addVoronoiFaceNodes){// for VORONOI
        faceNodeId = (int *)malloc(24*getNumElems()*sizeof(int));
        memset(faceNodeId,0,24*getNumElems()*sizeof(int));
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        nNodesCurrent = lg_LatticeGraph.nodes.size();
        const double eta = 0.4;// controls the edge length
        double wts[4][4] = {
            {eta, eta, (1-2*eta)/2, (1-2*eta)/2},
            {(1-2*eta)/2, eta, eta, (1-2*eta)/2},
            {(1-2*eta)/2, (1-2*eta)/2, eta, eta},
            {eta, (1-2*eta)/2, (1-2*eta)/2, eta}
        };
        // create 4 face-nodes (shared by two elements),
        for (int elem = 0; elem < getNumElems(); elem++){
            getElemNodes(elem,elemNodes);
            for (int face = 0; face < 6; face++){
                // for each face, find the corresponding neighboring element
                neighborElement = h_StructuringElems[h_StructuringSize*elem+face+1];
                // if the neighborElement has already been processed
                if ((neighborElement >= 0) && (elemsProcessed[neighborElement]) ){
                    int neighborFaceId;//faceId of the neighboring element
                    switch (face){
                        case 0: neighborFaceId = 3;break;
                        case 1: neighborFaceId = 4;break;
                        case 2: neighborFaceId = 5;break;
                        case 3: neighborFaceId = 0;break;
                        case 4: neighborFaceId = 1;break;
                        case 5: neighborFaceId = 2;break;
                    }
                    for (int i = 0; i < 4; i++) //copy
                        faceNodeId[24*elem + 4*face+i] = faceNodeId[24*neighborElement + 4*neighborFaceId+i];
                    continue;
                }
                //else add 4 new points per face
                for (int n = 0; n < 4; n++){
                    pt[0] = pt[1] = pt[2] = 0.0;
                    int node;
                    for (int i = 0; i < 4; i++){
                        node = elemNodes[faceNodeIndex[face][i]];
                        pt[0] += wts[i][n]*h_x[node];
                        pt[1] += wts[i][n]*h_y[node];
                        pt[2] += wts[i][n]*h_z[node];
                    }

                    faceNodeId[24*elem + 4*face+n] =  lg_LatticeGraph.nodes.size();
                    lg_LatticeGraph.nodes.push_back(pt);// add a new point
                }
            }
            elemsProcessed[elem] = true;
        }

    }
    /* Add required edges */
    if (addXYEdges) {
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        for (int elem = 0; elem < getNumElems(); elem++){
            if (!elemsToProcess[elem]) continue;
            getElemNodes(elem,elemNodes);
            for (int edge = 0; edge < 8; edge++){// the first 8 are XY edges
                // for each edge, we need to check if it has been processed
                // we find all elements that share this edge
                // if any of the elements have been processed then skip
                node0 = elemNodes[edgePairs[edge][0]];
                node1 = elemNodes[edgePairs[edge][1]];

                nNeighborsOfNode0 = getNeighboringElemsOfNode(node0,elemsNeighborsOfNode0);
                nNeighborsOfNode1 = getNeighboringElemsOfNode(node1,elemsNeighborsOfNode1);
                nCommonElems = findCommonElements(nNeighborsOfNode0,elemsNeighborsOfNode0,nNeighborsOfNode1,elemsNeighborsOfNode1,commonElems);

                insertEdge = true;
                for (int i = 0; i < nCommonElems; i++){
                    if (elemsProcessed[commonElems[i]]){ // (need to avoid double counting)
                        insertEdge = false;
                        break;
                    }
                }
                if (insertEdge){// insert
                    strutElement[0] = node0;
                    strutElement[1] = node1;
                    lg_LatticeGraph.struts.push_back(strutElement);
                    lg_LatticeGraph.strutExists.push_back(true);
                }
            }
            elemsProcessed[elem] = true;
        }
    }
    if (addZEdges) {
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        for (int elem = 0; elem < getNumElems(); elem++){
            if (!elemsToProcess[elem]) continue;
            getElemNodes(elem,elemNodes);
            for (int edge = 8; edge < 12; edge++){// the last 4 are Z edges
                // for each edge, we need to check if it has been processed
                // we find all elements that share this edge
                // if any of the elements have been processed then skip
                node0 = elemNodes[edgePairs[edge][0]];
                node1 = elemNodes[edgePairs[edge][1]];
                nNeighborsOfNode0 = getNeighboringElemsOfNode(node0,elemsNeighborsOfNode0);
                nNeighborsOfNode1 = getNeighboringElemsOfNode(node1,elemsNeighborsOfNode1);
                nCommonElems = findCommonElements(nNeighborsOfNode0,elemsNeighborsOfNode0,nNeighborsOfNode1,elemsNeighborsOfNode1,commonElems);

                insertEdge = true;
                for (int i = 0; i < nCommonElems; i++){
                    if (elemsProcessed[commonElems[i]]){ // (need to avoid double counting)
                        insertEdge = false;
                        break;
                    }
                }
                if (insertEdge){// insert
                    strutElement[0] = node0;
                    strutElement[1] = node1;
                    lg_LatticeGraph.struts.push_back(strutElement);
                    lg_LatticeGraph.strutExists.push_back(true);
                }
            }
            elemsProcessed[elem] = true;
        }
    }
    if ((addFaceEdges) || (addNonZFaceEdges)) {// add edges on faces, connecting face node and corner nodes
        int faceNode,cornerNode;
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        for (int elem = 0; elem < getNumElems(); elem++){
            if (!elemsToProcess[elem]) continue;
            getElemNodes(elem,elemNodes);
            for (int face = 0; face < 6; face++){
                if ((addNonZFaceEdges) && ((face == 2) ||(face == 5)) ) continue;//skip faces perpendicular to Z
                //  find the corresponding neighboring element
                neighborElement = h_StructuringElems[h_StructuringSize*elem+face+1];
                if ((neighborElement >= 0) && (elemsProcessed[neighborElement]) ){
                    continue;// no need to add edges, the other face will take care
                }
                // add 4 edges
                faceNode = faceNodeId[6*elem + face];
                if (faceNode < 0) continue;
                for (int i = 0; i < 4; i++){
                    cornerNode = elemNodes[faceNodeIndex[face][i]];
                    strutElement[0] = faceNode;
                    strutElement[1] = cornerNode;
                    lg_LatticeGraph.struts.push_back(strutElement);
                    lg_LatticeGraph.strutExists.push_back(true);
                }
            }
            elemsProcessed[elem] = true;
        }
    }

    if (addBCCEdges) {
         for (int elem = 0; elem < getNumElems(); elem++){
            if (!elemsToProcess[elem]) continue;
            getElemNodes(elem,elemNodes);
            for (int i = 0; i < 8; i++){
                strutElement[0] = elemNodes[i];
                strutElement[1] = getNumNodes() + elem;// the center nodes start after the voxel nodes
                lg_LatticeGraph.struts.push_back(strutElement);
                lg_LatticeGraph.strutExists.push_back(true);
            }
         }
    }

    if (addOctetEdges) {// add edges of octets crossing various faces (total of 12 edges per elem)
        int faceNode1,faceNode2,face2;
        int faceConnectivity[5][4] = {{1,2,4,5},{2,3,5,-1},{3,4,-1,-1},{4,5,-1,-1},{5,-1,-1,-1}};
        memset(elemsProcessed, false, getNumElems() * sizeof(bool));
        for (int elem = 0; elem < getNumElems(); elem++) {
            for (int face1 = 0; face1 < 5; face1++){// no need to process other faces
                faceNode1 = faceNodeId[6 * elem + face1];
                for (int i = 0; i < 4; i++){//maximum of 4 edges
                    face2 = faceConnectivity[face1][i];
                    if (face2 < 0) continue;// skip
                    faceNode2 = faceNodeId[6 * elem + face2];
                    strutElement[0] = faceNode1;
                    strutElement[1] = faceNode2;
                    lg_LatticeGraph.struts.push_back(strutElement);
                    lg_LatticeGraph.strutExists.push_back(true);
                }
            }
        }
    }
    if (addVoronoiFaceEdges){
        int faceNode1,faceNode2;
        memset(elemsProcessed,false,getNumElems()*sizeof(bool));
        for (int elem = 0; elem < getNumElems(); elem++){
            if (!elemsToProcess[elem]) continue;
            for (int face = 0; face < 6; face++){
                neighborElement = h_StructuringElems[h_StructuringSize*elem+face+1];
                if ((neighborElement >= 0) && (elemsProcessed[neighborElement]) ){
                    continue;// no need to add edges, the other face will take care
                }
                // else add 4 edges
                for (int i = 0; i < 4; i++){
                    faceNode1 = faceNodeId[24*elem + 4*face+i];
                    if (i < 3)
                        faceNode2 = faceNodeId[24*elem + 4*face+i+1];
                    else
                        faceNode2 = faceNodeId[24*elem + 4*face+0];
                    if ((faceNode1 < 0) || (faceNode2 < 0))
                        continue;
                    strutElement[0] = faceNode1;
                    strutElement[1] = faceNode2;
                    lg_LatticeGraph.struts.push_back(strutElement);
                    lg_LatticeGraph.strutExists.push_back(true);
                }
            }

            // next add the cross-connections
            int facePairs[12][2] ={{0,1},{1,2},{2,3},{3,4},{0,4},{0,2},{0,5},{1,3},{1,5},{2,4},{3,5},{4,5}};
            int face1,face2,node1,node2,closestNodePairs[2];
            Point3D pt1,pt2;
            double dMin,d;

            for (int i = 0; i < 12; i++){
                face1 = facePairs[i][0];
                face2 = facePairs[i][1];
                // we need find the two nodes that are closest
                // brute-force method
                closestNodePairs[0] = 0;
                closestNodePairs[1] = 0;
                dMin= 1e12;
                for (int j = 0; j < 4; j++){
                    node1 = faceNodeId[24*elem + 4*face1+j];
                    if (node1 < 0)
                        continue;
                    pt1 = lg_LatticeGraph.nodes[node1];
                    for (int k = 0; k < 4; k++){
                        node2 = faceNodeId[24*elem + 4*face2+k];
                        if (node2 < 0)
                            continue;
                        pt2 = lg_LatticeGraph.nodes[node2];
                        d = DIST3D(pt1,pt2);
                        if (d < dMin){
                            dMin = d;
                            closestNodePairs[0] = node1;
                            closestNodePairs[1] = node2;
                        }
                    }

                }
                strutElement[0] = closestNodePairs[0];
                strutElement[1] = closestNodePairs[1];
                lg_LatticeGraph.struts.push_back(strutElement);
                lg_LatticeGraph.strutExists.push_back(true);
            }
            elemsProcessed[elem] = true;
        }
    }
    free(elemsToProcess);
    free(elemsProcessed);
    if (faceNodeId) free(faceNodeId);
    // add default parent body for all graph nodes
    for (int node = 0; node < lg_LatticeGraph.nodes.size(); node++)
        lg_LatticeGraph.parentBody.push_back(0);

    createLatticeLocalCoordinateSystem();
    return true;
}
/************************************************************/
void  LatticeGenerator::createLatticeLocalCoordinateSystem(void)
{
    int node0,node1;
    Point3D pt0,pt1;
    double length,nrm,dtPrd;

    // precompute length and local CS of each strut
    // useful for FEA
    lg_LatticeGraph.lengthOfStrut.clear();
    for (int strut = 0;strut<lg_LatticeGraph.struts.size();strut++){
        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];
        pt0 = lg_LatticeGraph.nodes[node0];
        pt1 = lg_LatticeGraph.nodes[node1];

        length = DIST3D(pt0,pt1);

        lg_LatticeGraph.lengthOfStrut.push_back(length);
        // local x-axis is along the strut
        lg_LatticeGraph.xAxis[0] = (lg_LatticeGraph.nodes[node1][0] - lg_LatticeGraph.nodes[node0][0])/length;
        lg_LatticeGraph.xAxis[1] = (lg_LatticeGraph.nodes[node1][1] - lg_LatticeGraph.nodes[node0][1])/length;
        lg_LatticeGraph.xAxis[2] = (lg_LatticeGraph.nodes[node1][2] - lg_LatticeGraph.nodes[node0][2])/length;

        // create a random y axis perpendicular to x axis
        lg_LatticeGraph.yAxis[0] = 0.2642376;
        lg_LatticeGraph.yAxis[1] = 0.36746373;
        lg_LatticeGraph.yAxis[2] = -0.65526526;
        dtPrd = DOTPROD3D(lg_LatticeGraph.xAxis,lg_LatticeGraph.yAxis);
        for (int i = 0; i < 3; i++)
            lg_LatticeGraph.yAxis[i] -= dtPrd*lg_LatticeGraph.xAxis[i];
        nrm = NORM3D(lg_LatticeGraph.yAxis);
        for (int i = 0; i < 3; i++)
            lg_LatticeGraph.yAxis[i] /= nrm;

        CROSS3D(lg_LatticeGraph.zAxis,lg_LatticeGraph.xAxis,lg_LatticeGraph.yAxis);
    }
}
/**********************************************************************/
double LatticeGenerator::computeLatticeVolume(void)
{
    double volume;
    int nStruts;
    double A,L,radius;

    nStruts = getLatticeGeneratorNumberOfStruts();
    if (nStruts == 0 ){
        return 0;
    }
    volume = 0;
    for (int strut = 0; strut < nStruts; strut++){
        radius =  lg_LatticeGraph.radiusOfTrunkStrut[strut];
        L = lg_LatticeGraph.lengthOfStrut[strut];
        A = 3.14159*radius*radius;
        volume += A*L;
    }
    return volume;
}
/************************************************************/
bool  LatticeGenerator::generateTriangleSoupFromLatticeGraph(const double halfWidth, const char*stlFile)
{
    // for every strut, generate 2 triangles in the XZ plane.
    int node0, node1;
    Point3D pt0,pt1;
    Point3D triPt0,triPt1,triPt2,triPt3;

    if ( lg_LatticeGraph.struts.size() == 0)
        return false;

    ofstream ofile(stlFile, ios::out| ios::binary);
    std::string header = "solid " + string(stlFile) + "-output";
    char head[80];
    strncpy(head,header.c_str(),sizeof(head)-1);

    int ntri;
    ntri = 2*lg_LatticeGraph.struts.size();// for every strut, generate 2 triangles in the XZ plane.

    ofile.write( head, 80*sizeof(char));
    ofile.write( (char*)&ntri, 4*sizeof(char));

    char attrib[2] = "0";
    float val;

    float normal[3];
    // for all triangles, the normal is along y plane
    normal[0] = 0;
    normal[1] = 1;
    normal[2] = 0;
    for (int strut = 0;strut<lg_LatticeGraph.struts.size();strut++){
        node0 = lg_LatticeGraph.struts[strut][0];
        node1 = lg_LatticeGraph.struts[strut][1];
        pt0 = lg_LatticeGraph.nodes[node0];
        pt1 = lg_LatticeGraph.nodes[node1];
        for (int i = 0; i < 3; i++){
            triPt0[i] = triPt3[i] = pt0[i];
            triPt1[i] = triPt2[i] = pt1[i];
        }
        // offset the x values
        triPt0[0] += halfWidth;
        triPt1[0] += halfWidth;
        triPt2[0] -= halfWidth;
        triPt3[0] -= halfWidth;

        // first triangle
        for( int j = 0; j < 3; j++) {
            val = normal[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt0[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt1[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt3[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        ofile.write( (char*)&attrib, 2);

        // second triangle
        for( int j = 0; j < 3; j++) {
            val = normal[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt1[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt2[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        for( int j = 0; j < 3; j++) {
            val = triPt3[j];
            ofile.write( reinterpret_cast<const char *>(&val), sizeof(float));
        }
        ofile.write( (char*)&attrib, 2);

    }
}
/************************************************************/
bool  LatticeGenerator::readLatticeGraphFromOffFile(const char *offFile)
{

    ifstream ifile(offFile, ios::in);
    if( ifile.fail() ) {
        cout << "Error: Can't open input file " << endl;
        return false;
    }
    lg_LatticeGraph.struts.clear();
    lg_LatticeGraph.nodes.clear();
    lg_LatticeGraph.strutExists.clear();

    lg_LatticeGraph.boxMin[0] = lg_LatticeGraph.boxMin[1] = lg_LatticeGraph.boxMin[2] = 1e8;
    lg_LatticeGraph.boxMax[0] = lg_LatticeGraph.boxMax[1] = lg_LatticeGraph.boxMax[2] = -1e8;

    string str;
    ifile >> str;
    if( str != "OFF") {
        cout << "Error: input file not in the off format" << endl;
        return false;
    }

    int numPoints, numFaces, numEdges;
    ifile >> numPoints >> numFaces >> numEdges;
    if( numPoints <= 0) {
        cout << "Error: numPoints must be positive in graph file" << endl;
        return false;
    }
    if( numFaces != 0) {
        cout << "Error: numFaces must be zero in graph file" << endl;
        return false;
    }
    if( numEdges <= 0) {
        cout << "Error: numEdges must be positive in graph file" << endl;
        return false;
    }

    double x, y, z;
    lg_LatticeGraph.nodes.resize(numPoints);
    for( size_t i = 0; i < numPoints; i++) {
        ifile >> x >> y >> z;
        lg_LatticeGraph.nodes[i][0] = x;
        lg_LatticeGraph.nodes[i][1] = y;
        lg_LatticeGraph.nodes[i][2] = z;
        lg_LatticeGraph.boxMin[0] = min(lg_LatticeGraph.boxMin[0],x);
        lg_LatticeGraph.boxMin[1] = min(lg_LatticeGraph.boxMin[1],y);
        lg_LatticeGraph.boxMin[2] = min(lg_LatticeGraph.boxMin[2],y);

        lg_LatticeGraph.boxMax[0] = max(lg_LatticeGraph.boxMax[0],x);
        lg_LatticeGraph.boxMax[1] = max(lg_LatticeGraph.boxMax[1],y);
        lg_LatticeGraph.boxMax[2] = max(lg_LatticeGraph.boxMax[2],y);

    }
    lg_LatticeGraph.modelScale = DIST3D(lg_LatticeGraph.boxMin,lg_LatticeGraph.boxMax);

    lg_LatticeGraph.struts.resize(numEdges);
    lg_LatticeGraph.strutExists.resize(numEdges);
    int n0,n1;
    for(size_t i = 0; i < numEdges; i++) {
        ifile >> n0 >> n1;
        lg_LatticeGraph.struts[i][0] = n0;
        lg_LatticeGraph.struts[i][1] = n1;
        lg_LatticeGraph.strutExists[i] = true;
    }

    // add default parent body for all graph nodes
    for (int node = 0; node < lg_LatticeGraph.nodes.size(); node++)
        lg_LatticeGraph.parentBody.push_back(0);

    ifile >> str;
    if( str != "#NodeParentBody") {// no parent body information
        ;
    }
    else {
        int parentBody;
        for( size_t i = 0; i < numPoints; i++) {
            ifile >> parentBody;
            lg_LatticeGraph.parentBody[i] = parentBody;
        }
    }
    ifile.close();

    createLatticeLocalCoordinateSystem();
    return true;
}
/************************************************************/
bool  LatticeGenerator::saveLatticeGraphToOffFile(const char*offFile)
{
    FILE *fptr = fopen(offFile, "wt");
    if (fptr== NULL){
        writeMessageToParetoLog("Unable to open file %s ",offFile);
        return false;
    }
    int number_of_struts;
    number_of_struts = 0;
    for (int strut = 0;strut<lg_LatticeGraph.struts.size();strut++)
         if (lg_LatticeGraph.strutExists[strut])
             number_of_struts++;

    int number_of_nodes = lg_LatticeGraph.nodes.size();

    fprintf(fptr,"OFF\n");
    fprintf(fptr,"%d 0 %d\n",number_of_nodes,number_of_struts);

    if (number_of_struts == 0){
        fclose(fptr);
        return false;
    }
    for (int node = 0;node<number_of_nodes;node++)
        fprintf(fptr,"%.3g %.3g %.3g \n",lg_LatticeGraph.nodes[node][0],lg_LatticeGraph.nodes[node][1],lg_LatticeGraph.nodes[node][2]);

    for (int strut = 0;strut<lg_LatticeGraph.struts.size();strut++)
        if (lg_LatticeGraph.strutExists[strut])
            fprintf(fptr,"%d %d \n",lg_LatticeGraph.struts[strut][0],lg_LatticeGraph.struts[strut][1]);

    fprintf(fptr,"#NodeParentBody\n");
    for (int node = 0;node<number_of_nodes;node++)
        fprintf(fptr,"%d \n",lg_LatticeGraph.parentBody[node]);

    fclose(fptr);
}
