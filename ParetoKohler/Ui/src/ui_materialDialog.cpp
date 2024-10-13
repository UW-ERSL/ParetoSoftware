#include "ui_materialDialog.hpp"
#include "CoreFunctions.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>


/*************************************************************************/
MaterialDialog :: MaterialDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
///////////////////////////////////////////////////////////////////////////////
MaterialDialog :: ~MaterialDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog :: applyMaterialDialog()
{

    // first we validated the values in lineEdits
    for (int i = 0 ; i < 8; i++)
        propertyLineEdit[i]->setValidator( new QDoubleValidator(this) );

    // when material is chosen from Combo box, we find the index
    int matIndex =  materialComboBox->currentIndex();

    // if matIndex  == #materials (last in Combobox) in database, new material is being added
    if (matIndex >= materialHandler.getNumOfMaterials()){
        // convert into SI before storing
        MaterialProperties matProp;
        for (int i = 0 ; i < 8; i++)
            matProp[i] = propertyLineEdit[i]->text().toDouble();

        bool ok;
        QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                                 tr("Material Name:"), QLineEdit::Normal,
                                                 "", &ok);
        if (ok && !text.isEmpty()){
            string materialName = text.toStdString();
            materialHandler.addNewMaterial(materialName,matProp);
            liveVar.materialName = materialName;
        }
        else
            return;
    }
    else
        liveVar.materialName = materialComboBox->currentText().toStdString();

    liveVar.E = propertyLineEdit[0]->text().toDouble();
    liveVar.nu = propertyLineEdit[1]->text().toDouble();
    liveVar.Y = propertyLineEdit[2]->text().toDouble();
    liveVar.rho = propertyLineEdit[3]->text().toDouble();
    liveVar.K = propertyLineEdit[4]->text().toDouble();
    liveVar.alpha = propertyLineEdit[5]->text().toDouble();
    liveVar.specificHeat = propertyLineEdit[6]->text().toDouble();
    liveVar.price = propertyLineEdit[7]->text().toDouble();

    liveVar.nonDesignCheck = optimizeCheckBox->isChecked();
    coreFunctions.applyMaterial();
    if (liveVar.MATERIAL_APPLIED)
         closeDialog(); // apply and close
    emit setMaterialComplete();

    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog :: closeDialog()
{

    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog::updatePropertyLineEdits()
{
    materialComboBox->setCurrentIndex(liveVar.materialIndex );

    string propertyUnits ;
    string propertyName;

    materialHandler.getPropertyName(0,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,propertyUnits);
    propertyLabel[0]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");

    materialHandler.getPropertyName(1,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,propertyUnits);
    propertyLabel[1]->setText(QString::fromStdString(propertyName) +  " ( - ):");

    materialHandler.getPropertyName(2,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,propertyUnits);
    propertyLabel[2]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");

    materialHandler.getPropertyName(3,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_DENSITY,propertyUnits);
    propertyLabel[3]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");

    materialHandler.getPropertyName(4,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_THERMAL_CONDUCTIVITY,propertyUnits);
    propertyLabel[4]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");


    materialHandler.getPropertyName(5,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_THERMAL_EXPANSION,propertyUnits);
    cout << propertyName << endl;
    cout << propertyUnits << endl;
    propertyLabel[5]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");

    materialHandler.getPropertyName(6,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_SPECIFIC_HEAT,propertyUnits);
    propertyLabel[6]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");


    materialHandler.getPropertyName(7,propertyName);
    coreFunctions.getPropertyUnits(PROPERTY_PRICE_PER_UNITMASS,propertyUnits);
    propertyLabel[7]->setText(QString::fromStdString(propertyName) +  " (" + QString::fromStdString(propertyUnits) + "):");

    propertyLineEdit[0]->setText(QString::number(liveVar.E));
    propertyLineEdit[1]->setText(QString::number(liveVar.nu));
    propertyLineEdit[2]->setText(QString::number(liveVar.Y));
    propertyLineEdit[3]->setText(QString::number(liveVar.rho));
    propertyLineEdit[4]->setText(QString::number(liveVar.K));
    propertyLineEdit[5]->setText(QString::number(liveVar.alpha));
    propertyLineEdit[6]->setText(QString::number(liveVar.specificHeat));
    propertyLineEdit[7]->setText(QString::number(liveVar.price));
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog::updateComboBox()
{
    string materialName;
    materialComboBox->clear();
    for (int i = 0; i < materialHandler.getNumOfMaterials(); i++){
        materialHandler.getMaterialName(i,materialName);
        materialComboBox->addItem(materialName.c_str());
    }

    materialComboBox->addItem("New material");
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog::loadMaterialDialog()
{
    // when material is chosen from Combo box, we find the index
    int matIndex =  materialComboBox->currentIndex();

    string materialName = materialComboBox->currentText().toStdString();
    MaterialProperties matProp;
    materialHandler.getPropertyValues(matIndex,matProp);

    // By default all materials properties are in SI (m,kg,s)
    // the SI material values are then scaled according to units selected
    //The liveVar are then filled in
    double EScale = 1.0,rhoScale = 1.0,alphaScale =1.0,KScale = 1.0,priceScale = 1.0,specificHeatScale = 1.0;

    /*if (liveVar.paretoUnits == UNITS_MM) {// convert into mm,kg,s, K
        EScale = 1e-3;// Convert from N/m^2, i.e., kg/(m-sec^2) to kg/(mm-sec^2)
        rhoScale = 1e-9;// Convert from kg/m^3 to kg/mm^3
        alphaScale = 1.0;// thermal expansion
        KScale = 1e9;// Convert from W-m/K, i.e., kg m^3/(K-sec^2) to kg mm^3/(K-sec^2)
        specificHeatScale = 1e6;// Convert from J/(kg-K), i.e., m^2/(K-sec^2) to mm^2/(K-sec^2)
        priceScale = 1;// Convert from $/kg to $/kg
    }*/

    if (liveVar.paretoUnits == UNITS_INCH) {// convert into in,lb,s
        EScale = 0.2248/(39.36*39.36);//0.0001450377;
        rhoScale = 3.612729e-005;// Convert from kg/m^3 to lbm/in^3
        alphaScale = 1.0;// thermal expansion
        KScale = 1.0;
        specificHeatScale = 1.0;
        priceScale = 0.454;
    }

    liveVar.materialIndex = matIndex;
    liveVar.materialName = materialName;
    liveVar.E = EScale * matProp[0];
    liveVar.nu = matProp[1];
    liveVar.Y = EScale *matProp[2];
    liveVar.rho = rhoScale * matProp[3];
    liveVar.K = KScale * matProp[4];
    liveVar.alpha = alphaScale *matProp[5];
    liveVar.specificHeat = specificHeatScale * matProp[6];
    liveVar.price = priceScale * matProp[7];


    // finally the line edits are filled in via liveVar.
    updatePropertyLineEdits();
}
/*****************************************************************/
void MaterialDialog::showEvent(QShowEvent *)
{
    //updateComboBox();
    loadMaterialDialog();
}
/********************************************************************************/
void MaterialDialog::updateMaterialLabelText()
{
    string strUnits ;
}
///////////////////////////////////////////////////////////////////////////////
void MaterialDialog :: makeConnections()
{
    ComboBox( materialComboBox, [=]{ loadMaterialDialog();});
    PushButton(applyPushButton,[=]{applyMaterialDialog();});
}
///////////////////////////////////////////////////////////////////////////////
