#include "ui_displayOptionsDialog.hpp"
#include "CoreFunctions.hpp"
#include <iostream>

/*****************************************************************/
DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();
    geomComboBox->setCurrentIndex(2);
    geomMap["Geometry"] =DISPLAY_GEOM_INITIAL;
    geomMap["FinalTopology"] =DISPLAY_GEOM_TOPOLOGY;
    geomMap["Mesh"] = DISPLAY_GEOM_MESH;
    geomMap["MeshComponent"] = DISPLAY_GEOM_MESHCOMPONENTS;
    geomMap["FinalDesign"] = DISPLAY_GEOM_FINAL;
    geomMap["GCode"] = DISPLAY_GEOM_OTHER;

    fieldMap["None"] = DISPLAY_FIELD_NONE;
    fieldMap["Displacement"] = DISPLAY_FIELD_DISPLACEMENT;
    fieldMap["Disp-X"] = DISPLAY_FIELD_DISPLACEMENT_X;
    fieldMap["Disp-Y"] = DISPLAY_FIELD_DISPLACEMENT_Y;
    fieldMap["Disp-Z"] = DISPLAY_FIELD_DISPLACEMENT_Z;
    fieldMap["Stress"] = DISPLAY_FIELD_STRESS;
    fieldMap["EigenMode"] = DISPLAY_FIELD_MODAL;
    fieldMap["Moisture"] = DISPLAY_FIELD_MOISTURE; // In this context, moisture relates to temperature

    orientationMap["Iso"] = VIEW_ISO ;
    orientationMap["XY+"] = VIEW_XYPLUS;
    orientationMap["XY-"] = VIEW_XYMINUS;
    orientationMap["ZY+"] = VIEW_YZPLUS;
    orientationMap["ZY-"] = VIEW_YZMINUS;
    orientationMap["ZX+"] = VIEW_XZPLUS;
    orientationMap["ZX-"] = VIEW_XZMINUS;

}
/*****************************************************************/
void DisplayOptionsDialog::updateCuttingPlane()
{
    liveVar.xCuttingPlanePercent = (int) XCuttingPlaneSpinBox->value();
    liveVar.yCuttingPlanePercent = (int) YCuttingPlaneSpinBox->value();
    liveVar.zCuttingPlanePercent = (int) ZCuttingPlaneSpinBox->value();
    updateDisplay();
}


/*****************************************************************/
void DisplayOptionsDialog::updateDisplay()
{
    string geomToPlot =geomComboBox->currentText().toStdString();

    if ((geomMap[geomToPlot] == DISPLAY_GEOM_TOPOLOGY) && (!liveVar.TOPOPT_SOLVED)){
        writeWarningToLog("Solve topology optimization for this display option.");
        return;
    }
    liveVar.displayGeom = geomMap[geomToPlot];

    string fieldToPlot = fieldComboBox->currentText().toStdString();

    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_DISPLACEMENT) && (!liveVar.STATIC_FEA_SOLVED) ){
        writeWarningToLog("Solve static FEA for this display option.");
        return;
    }
    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_STRESS) && (!liveVar.STATIC_FEA_SOLVED)){
        writeWarningToLog("Solve static FEA for this display option.");
        return;
    }
    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_MODAL) && (!liveVar.MODAL_FEA_SOLVED) ){
        writeWarningToLog("Solve modal FEA for this display option.");
        return;
    }

    /*
    string view = viewComboBox->currentText().toStdString();

    int orientation = orientationMap[view];

    qglviewer::Vec dirVec;
    dirVec.x = 1;
    dirVec.y = 1;
    dirVec.z = 1;
    switch (orientation){
        case VIEW_ISO: dirVec.x = 1; dirVec.y = 1; dirVec.z = 1; break;
        case VIEW_XYPLUS: dirVec.x = 1; dirVec.y = 0; dirVec.z = 0; break;
        case VIEW_XYMINUS: dirVec.x = -1; dirVec.y = 0; dirVec.z = 0; break;
    }
    viewer->camera()->setViewDirection(dirVec);
    */

    liveVar.displayField = fieldMap[fieldToPlot];
    liveVar.isDeformationScaled = deformationScaleCheckBox->isChecked();
    liveVar.displayEdges = showTrianglesCheckBox->isChecked();
    liveVar.geomTransparent = transparentGeomCheckBox->isChecked();
    liveVar.plotBoundingBox =  boundingBoxCheckBox->isChecked();
    liveVar.plotAxis = axisCheckBox->isChecked();
    liveVar.plotElasticityBC = elasticityBCCheckBox->isChecked();
    liveVar.plotThermalBC =  thermalBCCheckBox->isChecked();
    liveVar.plotNonDesignGeometry = nonDesignGeomCheckBox->isChecked();
    viewer->updateGL();
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::resetView()
{
    viewer->camera()->showEntireScene();
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::animate()
{
    if (liveVar.plotAnimation) return;
    if ( (!liveVar.STATIC_FEA_SOLVED) && (!liveVar.MODAL_FEA_SOLVED)) {
        writeWarningToLog("Please carry out static FEA or modal FEA for animation");
        return;
    }

    if ( (liveVar.displayField != DISPLAY_FIELD_DISPLACEMENT) &&
         (liveVar.displayField != DISPLAY_FIELD_STRESS) &&
         (liveVar.displayField != DISPLAY_FIELD_MODAL) ){
        writeWarningToLog("Please select displacement or stress or Eigen for animation");
        return;
    }
    liveVar.plotAnimation = true;
    liveVar.animationVariable  = 0;
    Timer timer;
    timer.start();
    while ((timer.getElapsedTimeInSec() < 100.0) && (liveVar.animationVariable  < 3*2*3.1416)) // cycles
        viewer->updateGL();

    liveVar.plotAnimation = false;
    timer.stop();
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::closeDialog()
{
    parentWidget()->show();
    close();
}
/*****************************************************************/
void DisplayOptionsDialog::showEvent(QShowEvent *)
{
    // first store the liveVar data, else when updated they get overwritten
    int displayGeom = liveVar.displayGeom;
    int displayField = liveVar.displayField;
    bool isDeformationScaled = liveVar.isDeformationScaled;
    bool geomTransparent = liveVar.geomTransparent;
    bool plotAxis = liveVar.plotAxis;
    bool showTriangles = liveVar.displayEdges;
    bool plotBoundingBox = liveVar.plotBoundingBox;
    bool plotElasticityBC = liveVar.plotElasticityBC;
    bool plotThermalBC = liveVar.plotThermalBC;
    bool plotNonDesignGeometry = liveVar.plotNonDesignGeometry;


    // update the combo box using liveVar
    geomComboBox->setCurrentIndex(displayGeom);
    fieldComboBox->setCurrentIndex(displayField);

    deformationScaleCheckBox->setChecked(isDeformationScaled);
    transparentGeomCheckBox->setChecked(geomTransparent);
    boundingBoxCheckBox->setChecked(plotBoundingBox);
    showTrianglesCheckBox->setChecked(showTriangles);
    axisCheckBox->setChecked(plotAxis);
    elasticityBCCheckBox->setChecked(plotElasticityBC);
    thermalBCCheckBox->setChecked(plotThermalBC);
    nonDesignGeomCheckBox->setChecked(plotNonDesignGeometry);

}
/*****************************************************************/
DisplayOptionsDialog::~DisplayOptionsDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void DisplayOptionsDialog :: makeConnections()
{
    ComboBox( geomComboBox, [=]{ updateDisplay();});
    ComboBox( fieldComboBox, [=]{ updateDisplay();});
    SpinBoxi( XCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});
    SpinBoxi( YCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});
    SpinBoxi( ZCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});
    CheckBox( deformationScaleCheckBox, [=]{ updateDisplay();});
    CheckBox( boundingBoxCheckBox, [=]{ updateDisplay();});
    CheckBox( showTrianglesCheckBox, [=]{ updateDisplay();});
    CheckBox( transparentGeomCheckBox, [=]{ updateDisplay();});
    CheckBox( axisCheckBox, [=]{ updateDisplay();});
    CheckBox( elasticityBCCheckBox, [=]{ updateDisplay();});
    CheckBox( thermalBCCheckBox, [=]{ updateDisplay();});
    CheckBox( symmetryCheckBox, [=]{ updateDisplay();});
    CheckBox( nonDesignGeomCheckBox, [=]{ updateDisplay();});
    PushButton( animatePushButton, [=]{ animate();});
    PushButton( resetViewPushButton, [=]{ resetView();});
    PushButton( closePushButton, [=]{ closeDialog();});

}
