#ifndef MATERIALHANDLER_H
#define MATERIALHANDLER_H
#include <iostream>
#include <string>
#include <vector>
#include <array>
using namespace std;
#include "csv.h"

// This is hard coded
#define MH_NUM_MATERIAL_PROPERTIES  8

typedef std::array<double, MH_NUM_MATERIAL_PROPERTIES> MaterialProperties;


class MaterialHandler {
public:

    bool    readMaterialCSV(string fileName);
    int     getNumOfMaterials(void);
    int     getNumOfProperties(void);
    bool    getMaterialName(int materialId,string &materialName);
    bool    getPropertyName(int propertyId,string &propertyName);
    bool    getPropertyValues(int materialId,MaterialProperties &matProp);
    bool    addNewMaterial(string materialName,MaterialProperties matProp);
    bool    isMaterialInDatabase(MaterialProperties &matProp,int &matIndex);
private:

    string       mh_CSVFileName;
    vector<string> mh_materialNames;
    vector<MaterialProperties> mh_materialProperties;

    bool     areMaterialPropertiesSame(MaterialProperties &matProp1,MaterialProperties &matProp2);
    // hard coded column names
    string  mh_propertyNames[MH_NUM_MATERIAL_PROPERTIES] = {"Young's Modulus","Poisson ratio","Yield Strength",
                              "Density","Thermal Conductivity","Thermal Expansion","Spec. Heat Capacity",
                             "Price"};

    string  mh_propertyUnits[MH_NUM_MATERIAL_PROPERTIES];

};
#endif // MATERIALHANDLER_H
