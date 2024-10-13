#include "MultiScale.h" // include the pertaining header files
#include<iostream>
#include<math.h>
#include<string>
#include<stdio.h>
#include<fstream>
#include<cmath>
#include <time.h>
#include<Timer.h>
#include<vector>
#include<algorithm>


/*****************************************************************************/
MultiScale::MultiScale()
{ 
    if(MULTISCALE_VERBOSE){
        std::cout<<"Initializing MultiScale Constructor";
	}
}
/****************************************************************/
MultiScale::~MultiScale()
{ 
    ;
}
/************************************************************/
void    MultiScale::enableMultiScaleVerbose()
{
    writeMessageToParetoLog("MultiScale: Disabling MultiScale Verbose");
    ms_verbose = false;
}
/************************************************************/
void    MultiScale::disableMultiScaleVerbose()
{
    writeMessageToParetoLog("MultiScale: Disabling MultiScale Verbose");
    ms_verbose = false;
}
/************************************************************/
bool MultiScale::computeMicrostructureEquivalentD(double *equivalentD,double *gradDelem)
{
    double *allDisplacements = (double*)malloc(3*h_nNodes*6*sizeof(double));

    for(int f = 0;f < 6;f++){
        assemblePeriodicForce(f);
        disableDeflation();
        enableDeflation();
        if (!solveElasticityStatic()){
            writeMessageToGUIWindow("Solution in periodic domain failed\n");
            return false;
        }
        getSolutionVector(&(allDisplacements[3*f*h_nNodes]));
    }

    double elemSize[3];
    getElemSize(elemSize);
    double a = (elemSize[0]+elemSize[1]+elemSize[2])/3;
    double dmatrixVector[36];
    getDMatrix(0,dmatrixVector);

    double x[8] = {0,a,a,0,0,a,a,0};
    double y[8] = {0,0,a,a,0,0,a,a};
    double z[8] = {0,0,0,0,a,a,a,a};

    computeEquivalentD(x,y,z,dmatrixVector,allDisplacements,equivalentD,gradDelem);
    free(allDisplacements);
    return true;
}
/************************************************************/
void MultiScale::generateStiffnessMatrixTemplates()
{
    char materialName[50];
    double materialProperties[7],tempDmatrix[36],tempKe[24][24];
    getMaterialPropertiesOfPart(0,materialProperties,materialName);
    int ctr = 0,i,j,k,l;
    for(i = 0;i<6;i++){
        for(j = i;j<6;j++){
            memset(tempDmatrix,0,36*sizeof(double));
            tempDmatrix[6*i+j] = 1;tempDmatrix[6*j+i] = 1;
            dMatrixTemplateIndices[ctr]=6*i+j;
            setDMatrix(0,tempDmatrix);
            createElementMatricesTemplates(false);
            getKeOfTemplateElem(0,tempKe);
            for(k = 0;k<24;k++){
                for(l=0;l<24;l++){
                    elemStiffMatrixTemplates[(k*24+l)*21+ctr] = tempKe[k][l];//There are 21 templates:storing first element of all templates first and so on...
                }
            }
            ctr++;
        }
    }
//Restore material properties
    removeAllMaterials();
    setIsotropicMaterialPropertiesOfPart(0,materialProperties,materialName);
    createElementMatricesTemplates(true);
}
/************************************************************/
void MultiScale::performSIMPpEquals1Optimization(double volFrac,double *density)
{
    setSIMPMinDensity(0.001);
    setSIMPTargetVolumeFraction(volFrac);
    setEexponent(1.0);
    disableDeflation();
    enableDeflation();
    solveSIMP();
    getSIMPDensity(density);
}
/************************************************************/
void MultiScale::addPlaceHolderMaterial(int nMaterials)
{
    char materialName[50];
    double materialProperties[7];
    int material;
    getMaterialPropertiesOfPart(0,materialProperties,materialName);
    removeAllMaterials();
    keepUnusedMaterial(true);
    double tempDouble = materialProperties[0];
    //Add material placeholders for different microstructure D matrices
    for(material = 0;material<nMaterials;material++){
        materialProperties[0] = tempDouble*(material+1)/nMaterials;
        setIsotropicMaterialPropertiesOfPart(0,materialProperties,materialName);
    }
}
bool MultiScale::perturbCuboidMeshDensityToVolFraction(double volFraction,double *density)
{
    int i;
    if (volFraction<getSIMPMinDensity()||volFraction>1){
        std::cout<<"Infeasible desired volume fraction";
        return false;
    }
    int elem,nodes[8];
    double elemSize[3],mi_elemSize[3],boundingBox[3][2];
    getElemSize(mi_elemSize);
    getBoundingBox(boundingBox);
    for(i = 0;i<3;i++)
        elemSize[i] = fabs(boundingBox[i][1]-boundingBox[i][0]);
    double elemCenter[3],cutoff = 0.5-pow(volFraction,2)/2,tempDouble;
    for(elem = 0;elem<h_nElems;elem++){
        setPseudoDensityOfElem(elem,0.8);

        getElemNodes(elem,nodes);
        getNodeXYZ(nodes[0],elemCenter);
        elemCenter[0] += mi_elemSize[0]/2;
        elemCenter[1] += mi_elemSize[0]/2;
        elemCenter[2] += mi_elemSize[0]/2;
        if(((elemCenter[0]>=-cutoff*elemSize[0] &&elemCenter[0]<=cutoff*elemSize[0])&&
            (elemCenter[1]>=-cutoff*elemSize[0] &&elemCenter[1]<=cutoff*elemSize[0]))
                ||((elemCenter[0]>=-cutoff*elemSize[0] &&elemCenter[0]<=cutoff*elemSize[0])&&
                   (elemCenter[2]>=-cutoff*elemSize[0] &&elemCenter[2]<=cutoff*elemSize[0]))
                ||((elemCenter[1]>=-cutoff*elemSize[1] &&elemCenter[1]<=cutoff*elemSize[0])&&
                   (elemCenter[2]>=-cutoff*elemSize[0] &&elemCenter[2]<=cutoff*elemSize[0]))){
            setPseudoDensityOfElem(elem,0.1);
        }
        tempDouble += getElemPseudoDensity(elem);
    }

    cout<<"volFrac = "<<tempDouble/h_nElems<<endl;
    getPseudoDensity(density);
    return true;
}
/************************************************************/
void MultiScale::initializeForMicrostructure(void)
{

    int elem;
    for(elem = 0;elem<h_nElems;elem++){
        setTemplateIdOfElem(0,elem);
        setMaterialIdOfElem(0,elem);
    }
    createElementMatricesTemplates(false);
    enableSimpMode();
    setCGRelativeResError(1e-12);
    setEexponent(3.0);
    setSIMPMinDensity(0.01);

    int BCType[3] = {0,0,0};
    double BCVal[3] = {0,0,0};
    setBCOnSingleNode(0,BCType,BCVal);

    disableNodalSpMV();
    enableDeflation();
    enablePreconditioner();
}
/************************************************************/
bool MultiScale::assignElemStiffMatBasedOnClusters(bool rotation,double *equivD,double *allEigenVectors,int *elemToClusterMapping)
{
    double R[6][6],tempDmatrix[36],tempKe[24][24];
    int elem,i,j,k,l;

#pragma omp parallel for private(elem,i,j,k,l,tempDmatrix,R,tempKe)
        for (elem = 0; elem < h_nElems; elem++){
            if(rotation){
                for(i =0;i<3;i++){
                    for(j=0;j<3;j++){
                        R[i][j] = allEigenVectors[elem*9+VoigtMap[j][0]*3+VoigtMap[i][0]]*allEigenVectors[elem*9+VoigtMap[j][1]*3+VoigtMap[i][1]];
                        R[i][j+3] = allEigenVectors[elem*9+VoigtMap[j+3][0]*3+VoigtMap[i][0]]*allEigenVectors[elem*9+VoigtMap[j+3][1]*3+VoigtMap[i][1]];
                        R[i+3][j] = 2*allEigenVectors[elem*9+3*VoigtMap[j][0]+VoigtMap[i+3][0]]*allEigenVectors[elem*9+3*VoigtMap[j][1]+VoigtMap[i+3][1]];
                        R[i+3][j+3] = allEigenVectors[elem*9+3*VoigtMap[j+3][0]+VoigtMap[i+3][0]]*allEigenVectors[9*elem+3*VoigtMap[j+3][1]+VoigtMap[i+3][1]]+allEigenVectors[9*elem+3*VoigtMap[j+3][0]+VoigtMap[i+3][1]]*allEigenVectors[9*elem+3*VoigtMap[j+3][1]+VoigtMap[i+3][0]];
                    }
                }
                for(i= 0;i<6;i++){
                    for(j= i;j<6;j++){
                        tempDmatrix[i*6+j] = 0;
                        for(k= 0;k<6;k++){
                            for(l=0;l<6;l++){
                                tempDmatrix[i*6+j] += R[i][k]*equivD[elemToClusterMapping[elem]*36+6*k+l]*R[j][l];
                            }
                        }
                    }
                }
            }
            else{
                for(i= 0;i<6;i++){
                    for(j= i;j<6;j++){
                        tempDmatrix[i*6+j] = equivD[elemToClusterMapping[elem]*36+6*i+j];
                    }
                }
            }
            for(k = 0;k<24;k++){
                for(l = 0;l<24;l++){
                    tempKe[k][l] = 0;
                    for(i = 0;i<21;i++){
                        tempKe[k][l] += tempDmatrix[dMatrixTemplateIndices[i]]*elemStiffMatrixTemplates[(k*24+l)*21+i];
                    }
                }
            }
            setKeOfTemplateElem(elem,tempKe);
        }
        return true;
}
/************************************************************/
bool MultiScale::computeMicroElemSensitivity(bool rotation,double *microSensitivities,double *allEigenVectors,int *elemToClusterMapping,double *gradD,int mi_nElems,int nClusters)
{
    int i,j,k,l,microElem,elem,cluster;
    double tempDmatrix[36],tempDouble,tempDouble2,R[6][6];
    double *microSensitivitiesAll;
    microSensitivitiesAll = (double*)malloc(mi_nElems*h_nElems*sizeof(double));
#pragma omp parallel for private(i,j,k,l,cluster,microElem,tempDmatrix,tempDouble,tempDouble2,R)
        for (elem = 0;elem<h_nElems;++elem){
            cluster = elemToClusterMapping[elem];
            if(rotation){
                for(i =0;i<3;i++){
                    for(j=0;j<3;j++){
                        R[i][j] = allEigenVectors[elem*9+j*3+i]*allEigenVectors[elem*9+j*3+i];
                        R[i][j+3] = allEigenVectors[elem*9+VoigtMap[j+3][0]*3+i]*allEigenVectors[elem*9+VoigtMap[j+3][1]*3+i];
                        R[i+3][j] = 2*allEigenVectors[elem*9+3*j+VoigtMap[i+3][0]]*allEigenVectors[elem*9+3*j+VoigtMap[i+3][1]];
                        R[i+3][j+3] = allEigenVectors[elem*9+3*VoigtMap[j+3][0]+VoigtMap[i+3][0]]*allEigenVectors[9*elem+3*VoigtMap[j+3][1]+VoigtMap[i+3][1]]+allEigenVectors[9*elem+3*VoigtMap[j+3][0]+VoigtMap[i+3][1]]*allEigenVectors[9*elem+3*VoigtMap[j+3][1]+VoigtMap[i+3][0]];
                    }
                }
            }
            double tempSolArray[24],elemSol[24];
            getElemSolution(elem,elemSol);

            for(microElem = 0;microElem<mi_nElems;microElem++){
                if(rotation){
                    for(i= 0;i<6;i++){
                        for(j= i;j<6;j++){
                            tempDmatrix[i*6+j] = 0;
                            for(k= 0;k<6;k++){
                                for(l=0;l<6;l++){
                                    tempDmatrix[i*6+j] += R[i][k]*gradD[36*mi_nElems*cluster+36*microElem+6*k+l]*R[j][l];
                                }
                            }
                        }
                    }
                }
                else{
                    for(i= 0;i<6;i++){
                        for(j= i;j<6;j++){
                            tempDmatrix[i*6+j] = gradD[36*mi_nElems*cluster+36*microElem+6*i+j];
                        }
                    }
                }
                tempDouble2 = 0;
                for (i = 0;i<24;i++){
                    tempSolArray[i] = 0;
                    for (j = 0; j < 24; j++){
                        tempDouble = 0;
                        for (k =0;k<21;k++){
                            tempDouble += elemStiffMatrixTemplates[(i*24+j)*21+k]*tempDmatrix[dMatrixTemplateIndices[k]];
                        }
                        tempSolArray[i] += tempDouble*elemSol[j];
                    }
                    tempDouble2 -= tempSolArray[i]*elemSol[i];
                }
                microSensitivitiesAll[elem*mi_nElems+microElem] = tempDouble2;
            }
        }
#pragma omp parallel for private(cluster,elem,microElem,tempDouble2)
        for(microElem = 0;microElem<mi_nElems;microElem++){
            for(cluster= 0;cluster<nClusters;cluster++){
                tempDouble2 = 0;
                for (elem = 0;elem<h_nElems;++elem){
                    if(elemToClusterMapping[elem]!=cluster)
                        continue;
                    tempDouble2 += microSensitivitiesAll[elem*mi_nElems+microElem];
                }
                microSensitivities[cluster*mi_nElems+microElem] = tempDouble2;
            }
        }
        safeFree(microSensitivitiesAll);
        return true;
}
/************************************************************/
bool MultiScale::getPrincipalStrainAndVectorOfElem(int elem,double principalStrains[3],double principalVectors[3][3])
{
    double strain[6], unsortedPrincipalStrains[3], maxDouble,tempVec[3];
    int i,j,k;

    TCLMatrix tclStrain(3,3,0.0),eigVect(3,3,0.0);
    TCLVector D(3,0.0);

    int tempInt, order[3] = {0,1,2};

    getElemStrains(elem,strain);
    for(i =0;i<3;i++){
        tclStrain[i][i] = strain[i];
    }
    tclStrain[0][1] = strain[3];tclStrain[1][0] = strain[3];
    tclStrain[0][2] = strain[4];tclStrain[2][0] = strain[4];
    tclStrain[1][2] = strain[5];tclStrain[2][1] = strain[5];

    tclStrain.eigen(D,eigVect);

    for(i =0;i<3;i++){
        for(j=0;j<3;j++){
            for(k=0;k<3;k++){
                unsortedPrincipalStrains[i] += eigVect[j][i]*tclStrain[j][k]*eigVect[k][i];
            }
        }
    }

    //Sort in descending order
    for(i =0;i<2;i++){
        maxDouble = fabs(unsortedPrincipalStrains[order[i]]);
        k = i;
        for(j=i+1;j<3;j++){
            if (fabs(unsortedPrincipalStrains[order[j]])>maxDouble){
                k = j;
                maxDouble = fabs(unsortedPrincipalStrains[order[j]]);
            }
        }
        if(k!=i){
            tempInt = order[i];
            order[i]= order[k];
            order[k] = tempInt;
        }

    }
    for(i =0;i<3;i++){
        principalStrains[i] = unsortedPrincipalStrains[order[i]];
        for(j=0;j<3;j++){
            principalVectors[i][j] = eigVect[order[i]][j];
        }

    }
    CROSS3D(tempVec,principalVectors[0],principalVectors[1]);
    maxDouble = DOTPROD3D(tempVec,principalVectors[2]);
    if(maxDouble<0){
        for(j=0;j<3;j++){
            principalVectors[2][j] *= -1;
        }
    }

    return true;
}
/****************************************************************/
void MultiScale::updateDensitySIMP_general(int nElems,double *simpDensity,
                                           double *simpSensitivity,double *volPerSimpDensity,
                                           bool *keepElem, double desiredVolFrac,
                                           double move,double damp, double minDensity)
{
    double lambda1 = 0, lambda2 = 1e4,lmid;
    double *newpseudoDensity = (double *)malloc((nElems)*sizeof(double));
    double sensitivityPerVolumeMean,currentVolume,totalVolume;

    sensitivityPerVolumeMean=0;
    for(int i=0;i<nElems;i++){
        sensitivityPerVolumeMean += simpDensity[i]*pow(fabs(simpSensitivity[i]/volPerSimpDensity[i]),damp)/minDensity;
        totalVolume += volPerSimpDensity[i];
    }

    sensitivityPerVolumeMean /=nElems;
    while (lambda2-lambda1 > 1e-7){
        lmid = 0.5*(lambda2+lambda1);
        currentVolume = 0;
        for(int i=0;i<nElems;i++){
            if (!keepElem[i]){
                newpseudoDensity[i] = MAX(simpDensity[i]-move,MIN(simpDensity[i]+move,simpDensity[i]*pow(-(simpSensitivity[i]/volPerSimpDensity[i])/(sensitivityPerVolumeMean*lmid),damp)));
                newpseudoDensity[i]  = MIN(1,MAX(minDensity,newpseudoDensity[i]));
            }
            else
                newpseudoDensity[i]  = 1.0;
            currentVolume += newpseudoDensity[i]*volPerSimpDensity[i];
        }
        if (currentVolume > desiredVolFrac*totalVolume)
            lambda1 = lmid;
        else
            lambda2 = lmid;
    }
    double maxDensity = -HUGE_VAL,minimumDensity = HUGE_VAL;
    for(int i=0;i<nElems;i++){
        if (maxDensity<newpseudoDensity[i]) maxDensity = newpseudoDensity[i];
        if (minimumDensity>newpseudoDensity[i]) minimumDensity = newpseudoDensity[i];
        simpDensity[i] = newpseudoDensity[i];
    }
    printf("maxDensity = %5.4f, minDensity = %5.4f, avgDensity = %5.4f, lambda = %8.6f\n",maxDensity,minimumDensity, currentVolume/totalVolume,lmid);
    free(newpseudoDensity);
}
/****************************************************************/
void MultiScale::printMicrostructures(double *microPseudoDensity,int nClusters,int iter)
{
    char filename[50];
    int cluster,elem,nodes[8];
    double elemCenter[3],mi_elemSize[3];
    getElemSize(mi_elemSize);
    for(cluster = 0;cluster<nClusters;cluster++){
        sprintf(filename,"DensityForCluster%dAtIteration%d.txt",cluster+1,iter+1);
        FILE *fptr = fopen(filename,"w");
        fprintf(fptr,"%d %f %f %f\n",h_nElems,mi_elemSize[0],mi_elemSize[0],mi_elemSize[0]);
        for(elem = 0;elem<h_nElems;elem++){
            getElemNodes(elem,nodes);
            getNodeXYZ(nodes[0],elemCenter);
            elemCenter[0] += mi_elemSize[0]/2;
            elemCenter[1] += mi_elemSize[0]/2;
            elemCenter[2] += mi_elemSize[0]/2;
            fprintf(fptr,"%f %f %f %f\n",elemCenter[0],elemCenter[1],elemCenter[2],microPseudoDensity[cluster*h_nElems+elem]);
        }
        fclose(fptr);
    }
}
/****************************************************************/
void MultiScale::freezeEdgesOfMicrostructure(bool *keepMicroElems,int nClusters,int mi_nElems)
{
    int nodes[8],tempInt,tempInt2,elem,cluster;
    double elemCenter[3],mi_elemSize[3];
    getElemSize(mi_elemSize);
    memset(keepMicroElems,0,nClusters*mi_nElems*sizeof(bool));
    int mi_elemPerSide = cbrt(mi_nElems);
    double cutoff = 1.1/(double)mi_elemPerSide;
    tempInt = 0;
    for(elem = 0;elem<mi_nElems;elem++){
        getElemNodes(elem,nodes);
        getNodeXYZ(nodes[0],elemCenter);
        elemCenter[0] += mi_elemSize[0]/2;
        elemCenter[1] += mi_elemSize[0]/2;
        elemCenter[2] += mi_elemSize[0]/2;
        tempInt2 = 0;
        if(elemCenter[0]<=mi_elemSize[0]*mi_elemPerSide*0.5*(cutoff-1))
            tempInt2++;
        if(elemCenter[1]<=mi_elemSize[0]*mi_elemPerSide*0.5*(cutoff-1))
            tempInt2++;
        if(elemCenter[2]<=mi_elemSize[0]*mi_elemPerSide*0.5*(cutoff-1))
            tempInt2++;
        if(elemCenter[0]>=mi_elemSize[0]*mi_elemPerSide*0.5*(1-cutoff))
            tempInt2++;
        if(elemCenter[1]>=mi_elemSize[0]*mi_elemPerSide*0.5*(1-cutoff))
            tempInt2++;
        if(elemCenter[2]>=mi_elemSize[0]*mi_elemPerSide*0.5*(1-cutoff))
            tempInt2++;
        if (tempInt2>1){
            for(cluster= 0;cluster<nClusters;cluster++)
                keepMicroElems[cluster*mi_nElems+elem] = true;
            tempInt++;
        }
     }
     cout<<"Number of frozen Elems = "<<tempInt<<endl;
}

/****************************************************************/
bool MultiScale::kMeans(double *data,int dataSize[2],int nClusters, int *elemToClusterMap, double *centers,int distanceType, bool initialized)
{
    int dim = dataSize[0],cluster,n,d;
    int noOfDataPts = dataSize[1];
    if(noOfDataPts==1){
        cout<<"kMeans: number of data point is ONE!"<<endl;
        memcpy(centers,data,dim*sizeof(double));
        return false;
    }
    if(nClusters>noOfDataPts){
        cout<<"kMeans: desired number of clusters exceed number of data points!"<<endl;
        return false;
    }
    if(nClusters == 1){
        memset(centers,0,dim*sizeof(double));
        for(n = 0;n<noOfDataPts;n++){
            for(int d = 0;d<dim;d++){
                centers[d] += data[n*dim+d];
            }
        }
        for(d = 0;d<dim;d++){
            centers[d] /= noOfDataPts;
        }
        return true;
    }
    double *tempDist = (double*)malloc(noOfDataPts*sizeof(double));
    double dataValue;

    if(!initialized){//seeding now if no initial guess provided
        int *centerIndices = (int*)malloc(nClusters*sizeof(int));
        centerIndices[0] = 0;
        centerIndices[1] = 0;
        double maxValue = -HUGE_VAL,minValue= HUGE_VAL;
        for( n = 0;n<noOfDataPts;n++){ //Seeding the first two centers as the farthest points;Alternatives are possible
            dataValue = 0;
            for(d = 0;d<dim;d++){
                dataValue+= data[n*dim+d];
            }
            minValue = (minValue>dataValue)?dataValue:minValue;
            centerIndices[0] = (minValue==dataValue)?n:centerIndices[0];
            maxValue = (maxValue<dataValue)?dataValue:maxValue;
            centerIndices[1] = (maxValue==dataValue)?n:centerIndices[1];
        }

        for(cluster = 2;cluster<nClusters;cluster++){
            for(n = 0;n<noOfDataPts;n++){
                tempDist[n] = HUGE_VAL;
            }
            for(int cluster2 = 0;cluster2<cluster;cluster2++){
                for(n = 0;n<noOfDataPts;n++){
                    dataValue = 0;
                    for(d = 0;d<dim;d++){
                        dataValue += (distanceType==1)?fabs(data[n*dim+d]-data[centerIndices[cluster2]*dim+d]):pow(data[n*dim+d]-data[centerIndices[cluster2]*dim+d],2);
                    }
                    tempDist[n] = (tempDist[n]>dataValue)?dataValue:tempDist[n];
                }
            }
            maxValue = -HUGE_VAL;
            for(n = 0;n<noOfDataPts;n++){
                if (tempDist[n]>maxValue){
                    centerIndices[cluster] = n;
                    maxValue = tempDist[n];
                }
            }
        }
        //Set initial seeds found as indices to the centers
        for(cluster =0;cluster<nClusters;cluster++){
            for (d = 0;d<dim;d++){
                centers[cluster*dim+d]= data[centerIndices[cluster]*dim+d];
            }
        }
        safeFree(centerIndices);
    }
    //Initially assign all elements to cluster 0
    memset(elemToClusterMap,0,noOfDataPts*sizeof(int));
    //Start iteration
    int* elemCountPerCluster = (int*)malloc(nClusters*sizeof(int));
    bool change = true;
    int iter = 0;
    for(n = 0;n<noOfDataPts;n++){
        tempDist[n] = HUGE_VAL;
    }
    while(change){
        change = false;
        for(cluster =0;cluster<nClusters;cluster++){
            for(n = 0;n<noOfDataPts;n++){
                dataValue = 0;
                for(d = 0;d<dim;d++){
                    dataValue += (distanceType==1)?fabs(data[n*dim+d]-centers[cluster*dim+d]):pow(data[n*dim+d]-centers[cluster*dim+d],2);
                }
                if(tempDist[n]>dataValue){
                    tempDist[n] = dataValue;
                    elemToClusterMap[n] = cluster;
                    change = true;
                }
            }
        }
        memset(centers,0,sizeof(double)*nClusters*dim);
        memset(elemCountPerCluster,0,sizeof(int)*nClusters);
        for(n = 0;n<noOfDataPts;n++){
            cluster = elemToClusterMap[n];
            for(d = 0;d<dim;d++){
                centers[cluster*dim+d] += data[n*dim+d];
            }
            elemCountPerCluster[cluster]++;
        }
        for(cluster =0;cluster<nClusters;cluster++){

            for(d = 0;d<dim;d++){
                centers[cluster*dim+d] /=max(1,elemCountPerCluster[cluster]);
            }

        }
        iter++;
    }
    safeFree(elemCountPerCluster);
    safeFree(tempDist);
    return true;
}
