#include "ui_structuralLoadsWindow.hpp"
#include "CoreFunctions.hpp"

/***************************************************/
StructuralLoadsWindow::StructuralLoadsWindow(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    loadNumberSpinBox->setValue(0);
    selectionCombo->setCurrentIndex(2);//coarse cylinder

    structuralLoadsMap["Fixed"] = PARETO_BC_FIXED;
    structuralLoadsMap["Force"] = PARETO_BC_FORCE;
    structuralLoadsMap["Sliding"] = PARETO_BC_SLIDING;
    structuralLoadsMap["Retain"] = PARETO_BC_RETAIN;
    structuralLoadsMap["Pressure"] = PARETO_BC_PRESSURE;
    structuralLoadsMap["Normal Force"] = PARETO_BC_NORMAL_FORCE;
    structuralLoadsMap["Torque"] = PARETO_BC_TORQUE;

    structuralLoadsMap["XFixed"] = PARETO_BC_XFIXED;
    structuralLoadsMap["YFixed"] = PARETO_BC_YFIXED;
    structuralLoadsMap["ZFixed"] = PARETO_BC_ZFIXED;
    structuralLoadsMap["None"] = PARETO_BC_CLEAR;

    structualInputFieldLabel1->setText("") ;
    structualInputFieldLabel2->setText("") ;
    structualInputFieldLabel3->setText("") ;
    structuralLineEdit1->setDisabled(1);
    structuralLineEdit2->setDisabled(1);
    structuralLineEdit3->setDisabled(1);

    makeConnections();
}
/***************************************************/
void StructuralLoadsWindow::assignStructuralLoads()
{

    string propertyName;

    int paretoBCTypeSelected = structuralLoadsMap[StructuralLoadsComboBox->currentText().toStdString()] ;

    liveVar.XDisp = 0;
    liveVar.YDisp = 0;
    liveVar.ZDisp = 0;
    structualInputFieldLabel1->setText("") ;
    structualInputFieldLabel2->setText("") ;
    structualInputFieldLabel3->setText("") ;
    structuralLineEdit1->setDisabled(1);
    structuralLineEdit2->setDisabled(1);
    structuralLineEdit3->setDisabled(1);
    switch(paretoBCTypeSelected)
    {
    case PARETO_BC_CLEAR:
        break;
    case PARETO_BC_FIXED:
        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("X-Disp (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;

        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("Y-Disp (") + propertyName + string(")");
        structualInputFieldLabel2->setText(propertyName.c_str()) ;

        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("Z-Disp (") + propertyName + string(")");
        structualInputFieldLabel3->setText(propertyName.c_str()) ;

        structuralLineEdit1->setEnabled(1);
        structuralLineEdit2->setEnabled(1);
        structuralLineEdit3->setEnabled(1);
        liveVar.XDisp = structuralLineEdit1->text().toDouble();
        liveVar.YDisp = structuralLineEdit2->text().toDouble();
        liveVar.ZDisp = structuralLineEdit3->text().toDouble();

        break;
    case PARETO_BC_ZFIXED:
        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("Z-Disp (") + propertyName + string(")");
        structualInputFieldLabel3->setText(propertyName.c_str()) ;
        structuralLineEdit3->setEnabled(1);
        liveVar.ZDisp = structuralLineEdit3->text().toDouble();

        break;
    case PARETO_BC_YFIXED:
        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("Y-Disp (") + propertyName + string(")");
        structualInputFieldLabel2->setText(propertyName.c_str()) ;
        structuralLineEdit2->setEnabled(1);
        liveVar.YDisp = structuralLineEdit2->text().toDouble();
        break;
    case PARETO_BC_XFIXED:
        coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
        propertyName = string("X-Disp (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;
        structuralLineEdit1->setEnabled(1);
        liveVar.XDisp = structuralLineEdit1->text().toDouble();
        break;
    case PARETO_BC_TORQUE:
        coreFunctions.getPropertyUnits(PROPERTY_WORK,propertyName);
        propertyName = string("Torque (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;
        structualInputFieldLabel2->setText("") ;
        structualInputFieldLabel3->setText("") ;
        structuralLineEdit1->setEnabled(1);
        structuralLineEdit2->setDisabled(1);
        structuralLineEdit3->setDisabled(1);
        liveVar.Torque = structuralLineEdit1->text().toDouble();
        break;
    case PARETO_BC_NORMAL_FORCE:
        coreFunctions.getPropertyUnits(PROPERTY_FORCE,propertyName);
        propertyName = string("Normal Force (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;
        structualInputFieldLabel2->setText("") ;
        structualInputFieldLabel3->setText("") ;
        structuralLineEdit1->setEnabled(1);
        structuralLineEdit2->setDisabled(1);
        structuralLineEdit3->setDisabled(1);
        liveVar.NormalForce = structuralLineEdit1->text().toDouble();
        break;
    case PARETO_BC_PRESSURE:
        coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,propertyName);
        propertyName = string("Pressure (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;
        structualInputFieldLabel2->setText("") ;
        structualInputFieldLabel3->setText("") ;
        structuralLineEdit1->setEnabled(1);
        structuralLineEdit2->setDisabled(1);
        structuralLineEdit3->setDisabled(1);
        liveVar.Pressure = structuralLineEdit1->text().toDouble();
        break;
    case PARETO_BC_RETAIN:
        break;
    case PARETO_BC_SLIDING:
        break;
    case PARETO_BC_FORCE:
        coreFunctions.getPropertyUnits(PROPERTY_FORCE,propertyName);
        propertyName = string("X-Force (") + propertyName + string(")");
        structualInputFieldLabel1->setText(propertyName.c_str()) ;

        coreFunctions.getPropertyUnits(PROPERTY_FORCE,propertyName);
        propertyName = string("Y-Force (") + propertyName + string(")");
        structualInputFieldLabel2->setText(propertyName.c_str()) ;

        coreFunctions.getPropertyUnits(PROPERTY_FORCE,propertyName);
        propertyName = string("Z-Force (") + propertyName + string(")");
        structualInputFieldLabel3->setText(propertyName.c_str()) ;

        structuralLineEdit1->setEnabled(1);
        structuralLineEdit2->setEnabled(1);
        structuralLineEdit3->setEnabled(1);

        liveVar.XForce = structuralLineEdit1->text().toDouble();
        liveVar.YForce = structuralLineEdit2->text().toDouble();
        liveVar.ZForce = structuralLineEdit3->text().toDouble();
        break;
    default:
        break;
    }
    liveVar.ParetoBCType = paretoBCTypeSelected;
}
/*****************************************************************/
void StructuralLoadsWindow::showEvent(QShowEvent *)
{

    liveVar.currentBCSet = loadNumberSpinBox->value();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotThermalBC = false;

    viewer->updateGL();
    //loadMaterialDialog();
}
/***************************************************/
void StructuralLoadsWindow::applyStructuralLoad()
{
    assignStructuralLoads();
    coreFunctions.applyElasticityBC();

    viewer->updateGL();
    structuralLineEdit1->setText("0");
    structuralLineEdit2->setText("0") ;
    structuralLineEdit3->setText("0") ;

    if(liveVar.BC_STRUCTURAL_APPLIED)
        emit setStructuralLoadsComplete();
}
///////////////////////////////////////////////////////////////////////////////
void StructuralLoadsWindow::updateLoadNumber()
{
    liveVar.currentBCSet = loadNumberSpinBox->value();
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void StructuralLoadsWindow::updateSelectionType()
{
    double cutoffAngle[] = {0,2,10,15,20,180};// permitted angle between neighboring triangles for selection

    int selection = selectionCombo->currentIndex();
    liveVar.cutoffAngle = cutoffAngle[selection];
    viewer->updateGL();
}
/***************************************************/
void StructuralLoadsWindow::closeWindow()
{
    if(liveVar.BC_STRUCTURAL_APPLIED)
        emit setStructuralLoadsComplete();
    parentWidget()->show();
    close();
}
/***************************************************/
void StructuralLoadsWindow::makeConnections()
{
    SpinBoxi( loadNumberSpinBox, [=]{ updateLoadNumber();});
    ComboBox( selectionCombo, [=]{ updateSelectionType();});

    structuralLineEdit1->setValidator(new QDoubleValidator(this));
    structuralLineEdit2->setValidator(new QDoubleValidator(this));
    structuralLineEdit3->setValidator(new QDoubleValidator(this));

    ComboBox( StructuralLoadsComboBox, [=]{ assignStructuralLoads();});
    PushButton(applyStructuralLoads, [=] {applyStructuralLoad(); });
    PushButton(closePushButton,[=]{closeWindow();});
}
/***************************************************/
StructuralLoadsWindow::~StructuralLoadsWindow()
{
    ;
}
