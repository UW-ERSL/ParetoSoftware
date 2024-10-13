#include "MaterialHandler.hpp"
#include <iostream>
#include <fstream>
using namespace std;


/**********************************************************************/
bool    MaterialHandler::readMaterialCSV(string fileName)
{
    mh_CSVFileName = fileName;
    io::CSVReader<MH_NUM_MATERIAL_PROPERTIES+1> in(fileName);
    std::string dummyStr,name("Name");
    // header
    in.read_header(io::ignore_extra_column, name,mh_propertyNames[0],mh_propertyNames[1], mh_propertyNames[2],
            mh_propertyNames[3],mh_propertyNames[4],mh_propertyNames[5],mh_propertyNames[6],mh_propertyNames[7]);

    // units
    in.read_row(dummyStr, mh_propertyUnits[0], mh_propertyUnits[1],mh_propertyUnits[2],
                           mh_propertyUnits[3],mh_propertyUnits[4],mh_propertyUnits[5],
                           mh_propertyUnits[6],mh_propertyUnits[7]);

    // read properties
    std::string materialName;
    MaterialProperties matProp;

    mh_materialNames.clear();
    mh_materialProperties.clear();
    while (in.read_row(materialName, matProp[0], matProp[1],matProp[2],
                       matProp[3],matProp[4],matProp[5],
                       matProp[6],matProp[7])) {
        mh_materialNames.push_back(materialName);
        mh_materialProperties.push_back(matProp);
    }
    return true;
}
/**********************************************************************/
int     MaterialHandler::getNumOfMaterials(void)
{
    return mh_materialNames.size();
}
/**********************************************************************/
int     MaterialHandler::getNumOfProperties(void)
{
    return MH_NUM_MATERIAL_PROPERTIES;
}
/**********************************************************************/
bool    MaterialHandler::getMaterialName(int materialId,string &materialName)
{
    if ( (materialId < 0) || (materialId >= mh_materialNames.size()))  return false;
    materialName = mh_materialNames.at(materialId);
    return true;
}
/**********************************************************************/
bool     MaterialHandler::getPropertyName(int propertyId,string &propertyName)
{
    if ( (propertyId < 0) || (propertyId > MH_NUM_MATERIAL_PROPERTIES))  return false;

    propertyName = mh_propertyNames[propertyId];
    return true;
}
/**********************************************************************/
bool     MaterialHandler::areMaterialPropertiesSame(MaterialProperties &matProp1,MaterialProperties &matProp2)
{
    double relativeError;

    relativeError = 0;
    for (int i = 0; i < MH_NUM_MATERIAL_PROPERTIES; i++)
       relativeError += (matProp1[i] - matProp2[i])/(matProp1[i]+1e-32);// avoid division by zero

    if (relativeError < 1e-8) return true;
    else return false;
}
/**********************************************************************/
bool     MaterialHandler::isMaterialInDatabase(MaterialProperties &matProp,int &matIndex)
{
    MaterialProperties matPropTemp;

    matIndex = -1;
    for (int index = 0; index < mh_materialNames.size(); index++){
        matPropTemp = mh_materialProperties.at(index);
        if (areMaterialPropertiesSame(matProp,matPropTemp)){
            matIndex = index;
            return true;
        }
    }
    return false;
}
/**********************************************************************/
bool    MaterialHandler::getPropertyValues(int materialId,MaterialProperties &matProp)
{
    if ( (materialId < 0) || (materialId >= mh_materialNames.size()))  return false;
    matProp = mh_materialProperties.at(materialId);
    return true;
}
/**********************************************************************/
bool    MaterialHandler::addNewMaterial(string materialName,MaterialProperties matProp)
{
    mh_materialNames.push_back(materialName);
    mh_materialProperties.push_back(matProp);

    remove("./ParetoMaterials.csv");
    FILE *fptr = fopen("./ParetoMaterials.csv","w");
    if (fptr){
        fprintf(fptr,"Name ");
        for (int i = 0; i < MH_NUM_MATERIAL_PROPERTIES; i++)
            fprintf(fptr,", %s ",mh_propertyNames[i].c_str());

        fprintf(fptr,"\n");

        fprintf(fptr,"Units ");
        for (int i = 0; i < MH_NUM_MATERIAL_PROPERTIES; i++)
            fprintf(fptr,", %s ",mh_propertyUnits[i].c_str());

        fprintf(fptr,"\n");

        MaterialProperties matProp;
        for (int j = 0; j < mh_materialNames.size(); j++){
            matProp = mh_materialProperties.at(j);
            fprintf(fptr,"%s ",mh_materialNames[j].c_str());
            for (int i = 0; i < MH_NUM_MATERIAL_PROPERTIES; i++)
                fprintf(fptr,", %g ",matProp[i]);

            fprintf(fptr,"\n");
        }
        fclose(fptr);
    }
    else
        std::cout << "Could not open ParetoMaterials.csv for writing" << std::endl;

    return true;
}
