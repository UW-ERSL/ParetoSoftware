#include "ui_thermalLoadsWindow.hpp"
#include "CoreFunctions.hpp"

/***************************************************/
void ThermalLoadsWindow::assignThermalLoads()
{

    string tempUnitsName,heatFluxName,heatCoefficientName;
    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,tempUnitsName);
    coreFunctions.getPropertyUnits(PROPERTY_HEAT_FLUX,heatFluxName);
    coreFunctions.getPropertyUnits(PROPERTY_HEAT_COEFFICIENT,heatCoefficientName);


    thermalInputFieldLabel1->setText("h ( " + QString::fromStdString(heatCoefficientName)+ "): ");
    thermalInputFieldLabel2->setText("T ( " + QString::fromStdString(tempUnitsName)+ "): ");
    thermalLineEdit1->setEnabled(1);
    thermalLineEdit2->setEnabled(1);
    liveVar.ThermalBCTypeGUI = PARETO_BC_CONVECTION;


    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_CONVECTION )
    {
        thermalInputFieldLabel1->setText("h ( " + QString::fromStdString(heatCoefficientName)+ "): ");
        thermalInputFieldLabel2->setText("T ( " + QString::fromStdString(tempUnitsName)+ "): ");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setEnabled(1);
        thermalLineEdit1->setText(QString::number(1000.0));
        thermalLineEdit2->setText(QString::number(300));
        liveVar.ThermalBCTypeGUI = PARETO_BC_CONVECTION;

    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_RADIATION )
    {
        thermalInputFieldLabel1->setText("Emissivity: ");
        thermalInputFieldLabel2->setText("T (" + QString::fromStdString(tempUnitsName)+ "): ");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setEnabled(1);
        thermalLineEdit1->setText(QString::number(1.0));
        thermalLineEdit2->setText(QString::number(300));
        liveVar.ThermalBCTypeGUI = PARETO_BC_RADIATION;
        if (liveVar.temperatureUnits != UNITS_TEMPERATURE_KELVIN){
            writeWarningToGUIWindow("To apply radiation, the temperature units must be set to Kelvin");
            return;
        }
    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_HEAT )
    {
        thermalInputFieldLabel1->setText("Q (" + QString::fromStdString(heatFluxName)+ "): ");
        thermalInputFieldLabel2->setText("");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setDisabled(1);
        thermalLineEdit1->setText(QString::number(1000.0));
        thermalLineEdit2->setText(QString::number(0));
        liveVar.ThermalBCTypeGUI = PARETO_BC_HEAT;

    }
    if(thermalLoadsMap[thermalLoadsComboBox->currentText().toStdString()] == PARETO_BC_TEMP )
    {
        thermalInputFieldLabel1->setText("T ("+  QString::fromStdString(tempUnitsName)+ "): ");
        thermalInputFieldLabel2->setText(" ");
        thermalLineEdit1->setEnabled(1);
        thermalLineEdit2->setDisabled(1);
        thermalLineEdit1->setText(QString::number(300.0));
        thermalLineEdit2->setText(QString::number(0));
        liveVar.ThermalBCTypeGUI = PARETO_BC_TEMP;
    }
}
/***************************************************/
ThermalLoadsWindow::ThermalLoadsWindow(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    selectionCombo->setCurrentIndex(2);//coarse cylinder

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
    assignThermalLoads();
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
    liveVar.ThermalBCValueGUI = thermalLineEdit1->text().toDouble();
    liveVar.ThermalBCReferenceGUI = thermalLineEdit2->text().toDouble();
    coreFunctions.applyThermalBC();
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void ThermalLoadsWindow::updateSelectionType()
{
    double cutoffAngle[] = {0,2,10,15,20,180};// permitted angle between neighboring triangles for selection

    int selection = selectionCombo->currentIndex();
    if (selection == 0)
        liveVar.displayEdges  = 1;
    else
        liveVar.displayEdges  = 0;
    liveVar.cutoffAngle = cutoffAngle[selection];
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
    ;
}
/*******************************************************/
void ThermalLoadsWindow::makeConnections()
{
    thermalLineEdit1->setValidator(new QDoubleValidator(this));
    thermalLineEdit2->setValidator(new QDoubleValidator(this));

    ComboBox( selectionCombo, [=]{ updateSelectionType();});
    ComboBox( thermalLoadsComboBox, [=]{ assignThermalLoads();});

    PushButton(applyThermalLoads, [=] {applyThermalLoad(); });
    PushButton(closePushButton,[=]{closeDialog();});

}
/***************************************************/
