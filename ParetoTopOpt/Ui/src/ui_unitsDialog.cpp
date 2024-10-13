#include "ui_unitsDialog.hpp"
#include "CoreFunctions.hpp"
#include <iostream>


///////////////////////////////////////////////////////////////////////////////

UnitsDialog :: UnitsDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    //map temp strings to ints
    unitsMap["Fahrenheit"] = UNITS_TEMPERATURE_FAHRENHEIT;
    unitsMap["Kelvin"] = UNITS_TEMPERATURE_KELVIN;
    unitsMap["Celcius"] = UNITS_TEMPERATURE_CELCIUS;

    //map angle strings to ints
    angleUnitsMap["Degree"] = UNITS_ANGLE_DEGREE;
    angleUnitsMap["Radian"] = UNITS_ANGLE_RADIAN;

}

///////////////////////////////////////////////////////////////////////////////

UnitsDialog :: ~UnitsDialog()
{
}

///////////////////////////////////////////////////////////////////////////////

void UnitsDialog :: applyUnitsOption()
{
    string unitsString = "";


    bool isMKS = MKSRadioButton->isChecked();
    if(isMKS) unitsString = "Units : meters";

    bool ismmKS = mmKSRadioButton->isChecked();
    if(ismmKS) unitsString = "Units : millimeters";


    bool isIPS = IPSRadioButton->isChecked();
    if(isIPS)   unitsString = "Units : inches";
    //bool isCGS = CGSRadioButton->isChecked();

    liveVar.paretoUnits = UNITS_METER*isMKS + UNITS_INCH*isIPS + UNITS_MM* ismmKS;

    setTemperatureUnits();

    liveVar.UNITS_APPLIED = true;
    liveVar.BC_STRUCTURAL_APPLIED = false;
    liveVar.BC_THERMAL_APPLIED = false;
    topopt.resetBC();
    topopt.resetBC_thermal();
    emit setUnitsComplete();
    parentWidget()->show();
    viewer->updateGL();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void UnitsDialog:: setTemperatureUnits()
{
    string tempUnitsName =  temperatureComboBox->currentText().toStdString(); // get the current text
    liveVar.temperatureUnits = unitsMap[tempUnitsName]; // map it to the integer
}
/*****************************************************************************/
void UnitsDialog:: setAngleUnits()
{
    string angleUnitsName =  angleUnitsComboBox->currentText().toStdString(); // get the current text
    liveVar.angleUnits= angleUnitsMap[angleUnitsName]; // map it to the integer
}
/*****************************************************************/
void UnitsDialog::showEvent(QShowEvent *)
{
    int tempUnits = liveVar.temperatureUnits;
    switch (liveVar.paretoUnits){
        case UNITS_METER: MKSRadioButton->setChecked(true);break;
        case UNITS_MM: mmKSRadioButton->setChecked(true);break;
        case UNITS_INCH: IPSRadioButton->setChecked(true);break;
    }
    switch (tempUnits){
        case UNITS_TEMPERATURE_CELCIUS: temperatureComboBox->setCurrentIndex(0); break;
        case UNITS_TEMPERATURE_KELVIN: temperatureComboBox->setCurrentIndex(1); break;
        case UNITS_TEMPERATURE_FAHRENHEIT: temperatureComboBox->setCurrentIndex(2); break;

    }
}
///////////////////////////////////////////////////////////////////////////////
void UnitsDialog :: makeConnections()
{
    int tempUnits = liveVar.temperatureUnits;
    ComboBox(temperatureComboBox,[=]{setTemperatureUnits();});
    ComboBox(angleUnitsComboBox,[=]{setAngleUnits();});
    PushButton( closePushButton,[=]{applyUnitsOption();});

    liveVar.temperatureUnits = tempUnits ;

}
///////////////////////////////////////////////////////////////////////////////
