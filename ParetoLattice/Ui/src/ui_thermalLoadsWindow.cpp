#include "ui_thermalLoadsWindow.hpp"
#include "CoreFunctions.hpp"

/***************************************************/
void ThermalLoadsWindow::assignThermalLoads()
{

    string tempUnitsName;
    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);

    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_CONVECTION )
    {
        thermalInputFieldLabel1->setText("h_ref");
        thermalInputFieldLabel2->setText("T : " + QString::fromStdString(tempUnitsName));
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setEnabled(1);
        liveVar.ThermalBCTypeGUI = PARETO_BC_CONVECTION;
        liveVar.ThermalBCValueGUI = thermalLineEdit1->text().toDouble();
        liveVar.ThermalBCReferenceGUI = thermalLineEdit2->text().toDouble();


    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_RADIATION )
    {
        thermalInputFieldLabel1->setText("emissivity ");
        thermalInputFieldLabel2->setText("T (" + QString::fromStdString(tempUnitsName)+ "): ");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setEnabled(1);
        liveVar.ThermalBCTypeGUI = PARETO_BC_RADIATION;
        liveVar.ThermalBCValueGUI = thermalLineEdit1->text().toDouble();
        liveVar.ThermalBCReferenceGUI = thermalLineEdit2->text().toDouble();

    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_HEAT )
    {
        thermalInputFieldLabel1->setText("Q ");
        thermalInputFieldLabel2->setText("");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setDisabled(1);
        liveVar.ThermalBCTypeGUI = PARETO_BC_HEAT;
        liveVar.ThermalBCValueGUI = thermalLineEdit1->text().toDouble();

    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_TEMP )
    {
        thermalInputFieldLabel1->setText("T "+  QString::fromStdString(tempUnitsName)+ "): ");
        thermalInputFieldLabel2->setText(" ");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setDisabled(1);
        liveVar.ThermalBCTypeGUI = PARETO_BC_TEMP;
        liveVar.ThermalBCValueGUI = thermalLineEdit1->text().toDouble();


    }

}
/***************************************************/
ThermalLoadsWindow::ThermalLoadsWindow(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    thermalLoadsMap["Convection"] = PARETO_BC_CONVECTION;
    thermalLoadsMap["Radiation"] = PARETO_BC_RADIATION;
    thermalLoadsMap["Temperature"] = PARETO_BC_TEMP;
    thermalLoadsMap["Heat Flux"] = PARETO_BC_HEAT;

    string tempUnitsName;
    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);
    thermalInputFieldLabel1->setText("T ("+  QString::fromStdString(tempUnitsName) + ")");
    thermalInputFieldLabel2->setText(" ");
    thermalLineEdit1->setEnabled(1);
    thermalLineEdit2->setDisabled(1);

    makeConnections();


}
/////////////////////////////////////////////////////
void ThermalLoadsWindow::showEvent(QShowEvent *e)
{
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = false;
    liveVar.plotThermalBC = true;
    liveVar.plotTopOptConstraints = true;
    viewer->updateGL();
}
/***************************************************/
void ThermalLoadsWindow::applyThermalLoad()
{

    assignThermalLoads();
    coreFunctions.applyThermalBC();
    viewer->updateGL();
}


/***************************************************/
void ThermalLoadsWindow::closeDialog()
{
    if(liveVar.BC_THERMAL_APPLIED){
        emit setThermalLoadsComplete();
    }
    parentWidget()->show();
    close();
}
/***************************************************/
ThermalLoadsWindow::~ThermalLoadsWindow()
{

}
/*******************************************************/
void ThermalLoadsWindow::makeConnections()
{
    thermalLineEdit1->setValidator(new QDoubleValidator(this));
    thermalLineEdit2->setValidator(new QDoubleValidator(this));

    ComboBox( thermalLoadsComboBox, [=]{ assignThermalLoads();});

    PushButton(applyThermalLoads, [=] {applyThermalLoad(); });
    PushButton(closePushButton,[=]{closeDialog();});

}
/***************************************************/
