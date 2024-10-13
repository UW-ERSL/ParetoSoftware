#include "ui_displayOptionsDialog.hpp"
#include <iostream>

extern void writeWarningToGUIWindow(const char format[],...);
/*****************************************************************/
DisplayOptionsDialog::DisplayOptionsDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);
    makeConnections();

    geomMap["None"] = DISPLAY_GEOM_NONE;
    geomMap["InitialDesign"] =DISPLAY_GEOM_INITIAL;
    geomMap["FinalTopology"] =DISPLAY_GEOM_TOPOLOGY;
    geomMap["Mesh"] = DISPLAY_GEOM_MESH;
    geomMap["MeshComponent"] = DISPLAY_GEOM_MESHCOMPONENTS;
    geomMap["FinalDesign"] = DISPLAY_GEOM_FINAL;
    geomMap["Contours"] = DISPLAY_GEOM_CONTOURS;
    geomMap["TempGeom"] = DISPLAY_GEOM_OTHER;

    fieldMap["None"] = DISPLAY_FIELD_NONE;
    fieldMap["Displacement"] = DISPLAY_FIELD_DISPLACEMENT;
    fieldMap["Stress"] = DISPLAY_FIELD_STRESS;
    fieldMap["EigenMode"] = DISPLAY_FIELD_MODAL;
    fieldMap["Temperature"] = DISPLAY_FIELD_TEMPERATURE;
    fieldMap["SIMP"] = DISPLAY_FIELD_SIMP_DENSITY;
    fieldMap["TSField"] = DISPLAY_FIELD_TOPOLOGICAL_SENSITIVITY;

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
void DisplayOptionsDialog::updateEigenModeToDisplay()
{
    liveVar.displayEigenMode = (int) EigenNumberSpinBox->value();
    if (liveVar.displayEigenMode > liveVar.nEigenModes){
        writeWarningToGUIWindow("Only %d eigenmodes were computed.",liveVar.nEigenModes);
        return;
    }

    topopt.pushInitialModalSolutionToTessellation(liveVar.eigenModes[liveVar.displayEigenMode-1]);//push eigenmode

    updateDisplay();
}

/*****************************************************************/
void DisplayOptionsDialog::updateDisplay()
{
    string geomToPlot =geomComboBox->currentText().toStdString();

    if ((geomMap[geomToPlot] == DISPLAY_GEOM_TOPOLOGY) && (!liveVar.TOPOPT_SOLVED)){
        writeWarningToGUIWindow("Solve topology optimization for this display option.");
        return;
    }
    liveVar.displayGeom = geomMap[geomToPlot];

    string fieldToPlot = fieldComboBox->currentText().toStdString();

    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_DISPLACEMENT) && (!liveVar.STATIC_FEA_SOLVED) ){
        writeWarningToGUIWindow("Solve structural FEA for this display option.");
        return;
    }
    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_STRESS) && (!liveVar.STATIC_FEA_SOLVED)){
        writeWarningToGUIWindow("Solve structural FEA for this display option.");
        return;
    }
    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_TEMPERATURE) && (!liveVar.THERMAL_FEA_SOLVED)){
        writeWarningToGUIWindow("Solve static thermal FEA for this display option.");
        return;
    }
    if ((fieldMap[fieldToPlot] == DISPLAY_FIELD_MODAL) && (!liveVar.MODAL_FEA_SOLVED) ){
        writeWarningToGUIWindow("Solve modal FEA for this display option.");
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
    liveVar.plotSummaryResults = showTextCheckBox->isChecked();
    liveVar.geomTransparent = transparentGeomCheckBox->isChecked();
    liveVar.plotBoundingBox =  boundingBoxCheckBox->isChecked();
    liveVar.plotAxis = axisCheckBox->isChecked();
    liveVar.plotElasticityBC = elasticityBCCheckBox->isChecked();
    liveVar.plotThermalBC =  thermalBCCheckBox->isChecked();
    liveVar.plotTopOptConstraints = topoptConstraintsCheckBox->isChecked();
    liveVar.plotNonDesignGeometry = nonDesignGeomCheckBox->isChecked();
    liveVar.displayEigenMode = (int) EigenNumberSpinBox->value();
    liveVar.plotSupportStructures = showAMSupportsCheckBox->isChecked();
    viewer->updateGL();
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::saveImage()
{
    static QString lastSelectedDirectory;
    QString qstr = QFileDialog::getSaveFileName(this,
                                                *new QString("Select Image File "),
                                                lastSelectedDirectory,
                                                *new QString( "Format ( *.png )"));

    viewer->setSnapshotQuality(100);
    viewer->saveSnapshot(qstr);
    cout << "Saving image as " << qstr.toUtf8().constData() << endl;
}

/*****************************************************************/
void DisplayOptionsDialog::resetView()
{

    liveVar.selectedPart = -1;
    liveVar.partsToHide.clear();
    vox.resetHighlight(); // unselect all highlighted triangles
    viewer->camera()->showEntireScene();
    viewer->updateGL();
}

/*****************************************************************/
void DisplayOptionsDialog::animate()
{
    if (liveVar.plotAnimation) return;
    if ( (!liveVar.STATIC_FEA_SOLVED) && (!liveVar.MODAL_FEA_SOLVED)) {
        writeWarningToGUIWindow("Please carry out static FEA or modal FEA for animation");
        return;
    }

    if ( (liveVar.displayField != DISPLAY_FIELD_DISPLACEMENT) &&
         (liveVar.displayField != DISPLAY_FIELD_STRESS) &&
         (liveVar.displayField != DISPLAY_FIELD_MODAL) ){
        writeWarningToGUIWindow("Please select displacement or stress or Eigen for animation");
        return;
    }
    if ( topopt.getNumElems() > 100000){
        writeWarningToGUIWindow("Too large a model to animate");
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
void DisplayOptionsDialog::hideSelectedPart()
{
    writeMessageToGUIWindow("Hiding selected part %d",liveVar.selectedPart );
    if (liveVar.selectedPart >= 0)
        liveVar.partsToHide.push_back(liveVar.selectedPart);

    liveVar.selectedPart = -1;
    vox.resetHighlight(); // unselect all highlighted triangles
    viewer->updateGL();
}
/*****************************************************************/
void DisplayOptionsDialog::AMSupportMessage()
{
    writeWarningToGUIWindow("You must select AM Build under TopOpt Constraints to display support.");
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
    bool showText = liveVar.plotSummaryResults;
    bool showAMSupports = liveVar.plotSupportStructures;
    bool plotBoundingBox = liveVar.plotBoundingBox;
    bool plotElasticityBC = liveVar.plotElasticityBC;
    bool plotThermalBC = liveVar.plotThermalBC;
    bool plotTopOptConstraints = liveVar.plotTopOptConstraints;
    bool plotNonDesignGeometry = liveVar.plotNonDesignGeometry;


    // update the combo box using liveVar
    geomComboBox->setCurrentIndex(displayGeom);
    fieldComboBox->setCurrentIndex(displayField);


    deformationScaleCheckBox->setChecked(isDeformationScaled);
    transparentGeomCheckBox->setChecked(geomTransparent);
    boundingBoxCheckBox->setChecked(plotBoundingBox);
    showTrianglesCheckBox->setChecked(showTriangles);
    showTextCheckBox->setChecked(showText);
    axisCheckBox->setChecked(plotAxis);
    elasticityBCCheckBox->setChecked(plotElasticityBC);
    thermalBCCheckBox->setChecked(plotThermalBC);
    topoptConstraintsCheckBox->setChecked(plotTopOptConstraints);
    showAMSupportsCheckBox->setChecked(showAMSupports);
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
    //ComboBox( viewComboBox, [=]{ updateDisplay();});
    CheckBox( deformationScaleCheckBox, [=]{ updateDisplay();});
    CheckBox( boundingBoxCheckBox, [=]{ updateDisplay();});
    CheckBox( showTrianglesCheckBox, [=]{ updateDisplay();});
    CheckBox( showTextCheckBox, [=]{ updateDisplay();});
    CheckBox( transparentGeomCheckBox, [=]{ updateDisplay();});
    CheckBox( axisCheckBox, [=]{ updateDisplay();});
    CheckBox( elasticityBCCheckBox, [=]{ updateDisplay();});
    CheckBox( thermalBCCheckBox, [=]{ updateDisplay();});
    CheckBox( topoptConstraintsCheckBox, [=]{ updateDisplay();});
    CheckBox( nonDesignGeomCheckBox, [=]{ updateDisplay();});
    CheckBox( showAMSupportsCheckBox, [=]{AMSupportMessage(); updateDisplay();});

    SpinBoxi( XCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});
    SpinBoxi( YCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});
    SpinBoxi( ZCuttingPlaneSpinBox, [=]{ updateCuttingPlane();});

    SpinBoxi( EigenNumberSpinBox, [=]{ updateEigenModeToDisplay();});

    PushButton( animatePushButton, [=]{ animate();});
    PushButton( hideSelectedPartPushButton, [=]{ hideSelectedPart();});
    PushButton( saveImagePushButton, [=]{ saveImage();});
    PushButton( resetViewPushButton, [=]{ resetView();});
    PushButton( closePushButton, [=]{ closeDialog();});

}
