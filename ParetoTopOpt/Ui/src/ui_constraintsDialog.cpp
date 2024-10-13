#include "ui_constraintsDialog.hpp"
#include "CoreFunctions.hpp"
#include <iostream>

extern LiveVariables liveVar;
extern ParetoApplication topopt;
extern Voxelizer vox;
extern CoreFunctions coreFunctions;

/*****************************************************************/
ConstraintsDialog::ConstraintsDialog(QWidget *parent): QDialog(parent)
{
    setupUi(this);

    makeConnections();
}
/*****************************************************************/
void ConstraintsDialog::updateCheckbox()
{
    // enables/disables the combo box or spinbox, when checkbox is selected/deselected

     if (drawDirectionCheckBox->isChecked())
         drawDirectionComboBox->setEnabled(true);
     else
         drawDirectionComboBox->setEnabled(false);

     if (extrudeCheckBox->isChecked())
         extrudeComboBox->setEnabled(true);
     else
         extrudeComboBox->setEnabled(false);

     if (AMBuildCheckBox->isChecked())
         AMBuildComboBox->setEnabled(true);
     else
         AMBuildComboBox->setEnabled(false);

     if (cyclicSymmetryCheckBox->isChecked())
         cyclicSymmetryComboBox->setEnabled(true);
     else
         cyclicSymmetryComboBox->setEnabled(false);

     if (xGridPatternCheckBox->isChecked())
         xGridPatternSpinBox->setEnabled(true);
     else
         xGridPatternSpinBox->setEnabled(false);

     if (yGridPatternCheckBox->isChecked())
         yGridPatternSpinBox->setEnabled(true);
     else
         yGridPatternSpinBox->setEnabled(false);

     if (zGridPatternCheckBox->isChecked())
         zGridPatternSpinBox->setEnabled(true);
     else
         zGridPatternSpinBox->setEnabled(false);

     if (minFeatureSizeCheckBox->isChecked())
         minFeatureSizeSpinBox->setEnabled(true);
     else
         minFeatureSizeSpinBox->setEnabled(false);

     if (maxFeatureSizeCheckBox->isChecked())
         maxFeatureSizeSpinBox->setEnabled(true);
     else
         maxFeatureSizeSpinBox->setEnabled(false);

     if (stressSafetyFactorCheckBox->isChecked())
         stressSafetyFactorSpinBox->setEnabled(true);
     else
         stressSafetyFactorSpinBox->setEnabled(false);


     if (displacementLimitCheckBox->isChecked())
         displacementLimitSpinBox->setEnabled(true);
     else
         displacementLimitSpinBox->setEnabled(false);

     if (modalLimitCheckBox->isChecked())
         modalLimitSpinBox->setEnabled(true);
     else
         modalLimitSpinBox->setEnabled(false);

     if (temperatureLimitCheckBox->isChecked())
         temperatureLimitSpinBox->setEnabled(true);
     else
         temperatureLimitSpinBox->setEnabled(false);


     updateConstraints();
}
/*****************************************************************/
void ConstraintsDialog::updateConstraints()
{
    // here we transfer the data from GUI to liveVar

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;

    liveVar.topoptConstraints.isExtrudeActive = false;
    if (extrudeCheckBox->isChecked()){
        liveVar.topoptConstraints.isExtrudeActive = true;
        liveVar.topoptConstraints.extrudeDirection = extrudeComboBox->currentIndex()+1;//offset by 1

    }
    liveVar.topoptConstraints.isAMBuildActive = false;
    if (AMBuildCheckBox->isChecked()){
        liveVar.topoptConstraints.isAMBuildActive = true;
        switch (AMBuildComboBox->currentIndex()){
            case 0: liveVar.topoptConstraints.AMBuildDirection = 1; break; // +X
            case 1: liveVar.topoptConstraints.AMBuildDirection = -1; break; // -X
            case 2: liveVar.topoptConstraints.AMBuildDirection = 2; break; // +Y
            case 3: liveVar.topoptConstraints.AMBuildDirection = -2; break; // -Y
            case 4: liveVar.topoptConstraints.AMBuildDirection = 3; break; // +Z
            case 5: liveVar.topoptConstraints.AMBuildDirection = -3; break; // -Z
        }

    }
    liveVar.topoptConstraints.isDrawDirectionActive = false;
    if (drawDirectionCheckBox->isChecked()){
        liveVar.topoptConstraints.isDrawDirectionActive = true;
        liveVar.topoptConstraints.drawDirection = drawDirectionComboBox->currentIndex()+1;// offset by 1
    }

    liveVar.topoptConstraints.isCyclicSymmetryActive = false;
    if (cyclicSymmetryCheckBox->isChecked()){
        liveVar.topoptConstraints.isCyclicSymmetryActive = true;
        liveVar.topoptConstraints.cyclicSymmetry = cyclicSymmetryComboBox->currentIndex() + 2;
    }
    liveVar.topoptConstraints.isxGridActive = false;
    liveVar.topoptConstraints.isyGridActive = false;
    liveVar.topoptConstraints.iszGridActive = false;

    liveVar.topoptConstraints.xGridPattern = 1;
    liveVar.topoptConstraints.yGridPattern = 1;
    liveVar.topoptConstraints.zGridPattern = 1;
    if (xGridPatternCheckBox->isChecked()){
        liveVar.topoptConstraints.isxGridActive = true;
        liveVar.topoptConstraints.xGridPattern = xGridPatternSpinBox->value();
    }
    if (yGridPatternCheckBox->isChecked()){
        liveVar.topoptConstraints.isyGridActive = true;
        liveVar.topoptConstraints.yGridPattern = yGridPatternSpinBox->value();
    }
    if (zGridPatternCheckBox->isChecked()){
        liveVar.topoptConstraints.iszGridActive = true;
        liveVar.topoptConstraints.zGridPattern = zGridPatternSpinBox->value();
    }

    liveVar.topoptConstraints.isMinFeatureSizeActive = false;
    if (minFeatureSizeCheckBox->isChecked()){
        double meshSize = topopt.getMeshElementSize();
        double minFeatureSize = minFeatureSizeSpinBox->value();
        int smoothingIterations = floor(minFeatureSize/meshSize);
        if (smoothingIterations < 1){
            writeWarningToGUIWindow("Warning: Min feature size %g must be at least equal to mesh size %g",minFeatureSize,meshSize);
            minFeatureSize = meshSize;
        }
        liveVar.topoptConstraints.isMinFeatureSizeActive = true;
        liveVar.topoptConstraints.minFeatureSize = 1.1*minFeatureSize;
    }

    liveVar.topoptConstraints.isMaxFeatureSizeActive = false;
    if (maxFeatureSizeCheckBox->isChecked()){
        liveVar.topoptConstraints.isMaxFeatureSizeActive = true;
        liveVar.topoptConstraints.maxFeatureSize = maxFeatureSizeSpinBox->value();
    }

    liveVar.topoptConstraints.isStressSafetyActive = false;
    if (stressSafetyFactorCheckBox->isChecked()){
        liveVar.topoptConstraints.isStressSafetyActive = true;
        liveVar.topoptConstraints.stressSafetyFactor = stressSafetyFactorSpinBox->value();
    }
    liveVar.topoptConstraints.isMaxDisplacementActive = false;
    if (displacementLimitCheckBox->isChecked()){
        liveVar.topoptConstraints.isMaxDisplacementActive = true;
        liveVar.topoptConstraints.maxDisplacementAllowed = displacementLimitSpinBox->value();
    }

    liveVar.topoptConstraints.isMinEigenActive = false;
    if (modalLimitCheckBox->isChecked()){
        liveVar.topoptConstraints.isMinEigenActive = true;
        liveVar.topoptConstraints.minEigenValueAllowedHz = modalLimitSpinBox->value();
    }

    if (temperatureLimitCheckBox->isChecked()){
        liveVar.topoptConstraints.isTemperatureLimitActive = true;
        liveVar.topoptConstraints.maxTemperatureAllowed = temperatureLimitSpinBox->value();
    }
    liveVar.topoptConstraints.xSymmetry = xSymmetryCheckBox->isChecked();
    liveVar.topoptConstraints.ySymmetry = ySymmetryCheckBox->isChecked();
    liveVar.topoptConstraints.zSymmetry = zSymmetryCheckBox->isChecked();

    liveVar.topoptConstraints.keepTopologyConnected = keepTopologyConnected->isChecked();
    liveVar.topoptConstraints.keepDirichlet = keepFixedFacesCheckBox->isChecked();
    liveVar.CONSTRAINTS_APPLIED = true;
    viewer->updateGL();
}
/*****************************************************************/
void ConstraintsDialog::AMBuildMessage()
{
    if (AMBuildCheckBox->isChecked())
        ;//writeWarningToGUIWindow("The AM Build constraint is under development");
}
/*****************************************************************/
void ConstraintsDialog::closeDialog()
{
    emit setConstraintsComplete();
    updateConstraints();

    parentWidget()->show();
    close();
}
/*****************************************************************/
void ConstraintsDialog::showEvent(QShowEvent *e)
{

    // here, we go from liveVar (that might be initialized via project files) to update the GUI
    string propertyName;
    coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
    propertyName = string("MinFeat(") + propertyName + string(")");
    minFeatureSizeLabel->setText(propertyName.c_str());

    coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
    propertyName = string("MaxFeat(") + propertyName + string(")");
    maxFeatureSizeLabel->setText(propertyName.c_str());

    coreFunctions.getPropertyUnits(PROPERTY_LENGTH,propertyName);
    propertyName = string("MaxDisp(") + propertyName + string(")");
    displacementLimitLabel->setText(propertyName.c_str());

    //first store values before updating, else gets liveVar overwritten with default values
    int drawDirection,extrude,cylicSymmetry,xGridPattern,yGridPattern,zGridPattern,AMBuildDirection;
    double stressSafety,maxDisp,minEig,minFeatureSize,maxFeatureSize,temperatureLimit;
    bool keepDir,xSymmetry,ySymmetry,zSymmetry,keepConnected;
    bool isExtrudeActive, isDrawDirectionActive,isCyclicSymmetryActive,isAMBuildActive;
    bool isMaxDisplacementActive,isMaxFeatureSizeActive,isMinFeatureSizeActive;
    bool isMinEigenActive,isStressSafetyActive;
    bool isxGridActive,isyGridActive,iszGridActive;
    bool isTemperatureLimitActive;

    isExtrudeActive = liveVar.topoptConstraints.isExtrudeActive;
    isAMBuildActive = liveVar.topoptConstraints.isAMBuildActive;
    isDrawDirectionActive = liveVar.topoptConstraints.isDrawDirectionActive;
    isCyclicSymmetryActive = liveVar.topoptConstraints.isCyclicSymmetryActive;
    isMaxDisplacementActive = liveVar.topoptConstraints.isMaxDisplacementActive;
    isMaxFeatureSizeActive = liveVar.topoptConstraints.isMaxFeatureSizeActive;
    isMinFeatureSizeActive = liveVar.topoptConstraints.isMinFeatureSizeActive;
    isMinEigenActive = liveVar.topoptConstraints.isMinEigenActive;
    isTemperatureLimitActive = liveVar.topoptConstraints.isTemperatureLimitActive;
    isStressSafetyActive = liveVar.topoptConstraints.isStressSafetyActive;
    isxGridActive = liveVar.topoptConstraints.isxGridActive;
    isyGridActive = liveVar.topoptConstraints.isyGridActive;
    iszGridActive = liveVar.topoptConstraints.iszGridActive;

    // WARNING: Before you enable/disable, save all the liveVar, else
    // the liveVar gets overwritten to default values

    xSymmetry = liveVar.topoptConstraints.xSymmetry;
    ySymmetry = liveVar.topoptConstraints.ySymmetry;
    zSymmetry = liveVar.topoptConstraints.zSymmetry;
    keepConnected = liveVar.topoptConstraints.keepTopologyConnected;
    keepDir = liveVar.topoptConstraints.keepDirichlet;
    drawDirection = liveVar.topoptConstraints.drawDirection-1;
    extrude = liveVar.topoptConstraints.extrudeDirection-1;
    AMBuildDirection = liveVar.topoptConstraints.AMBuildDirection;
    cylicSymmetry = liveVar.topoptConstraints.cyclicSymmetry-2;
    minFeatureSize = liveVar.topoptConstraints.minFeatureSize;;
    maxFeatureSize = liveVar.topoptConstraints.maxFeatureSize;
    xGridPattern = liveVar.topoptConstraints.xGridPattern;
    yGridPattern = liveVar.topoptConstraints.yGridPattern;
    zGridPattern = liveVar.topoptConstraints.zGridPattern;
    maxDisp = liveVar.topoptConstraints.maxDisplacementAllowed;
    stressSafety = liveVar.topoptConstraints.stressSafetyFactor;
    minEig = liveVar.topoptConstraints.minEigenValueAllowedHz;
    temperatureLimit = liveVar.topoptConstraints.maxTemperatureAllowed;

    if (isDrawDirectionActive){
        drawDirectionComboBox->setEnabled(true);
        drawDirectionCheckBox->setChecked(true); 
    }
    else {
        drawDirectionComboBox->setEnabled(false);
        drawDirectionCheckBox->setChecked(false);
    }
    if (isExtrudeActive){
        extrudeComboBox->setEnabled(true);
        extrudeCheckBox->setChecked(true);
    }
    else {
        extrudeComboBox->setEnabled(false);
        extrudeCheckBox->setChecked(false);
    }

    if (isAMBuildActive){
        AMBuildComboBox->setEnabled(true);
        AMBuildCheckBox->setChecked(true);
    }
    else {
        AMBuildComboBox->setEnabled(false);
        AMBuildCheckBox->setChecked(false);
    }

    if (isCyclicSymmetryActive){
        cyclicSymmetryComboBox->setEnabled(true);
        cyclicSymmetryCheckBox->setChecked(true);
    }
    else {
        cyclicSymmetryComboBox->setEnabled(false);
        cyclicSymmetryCheckBox->setChecked(false);
    }
    if (isMinFeatureSizeActive){
        minFeatureSizeSpinBox->setEnabled(true);   
        minFeatureSizeCheckBox->setChecked(true); 
    }
    else{ 
        minFeatureSizeSpinBox->setEnabled(false);
        minFeatureSizeCheckBox->setChecked(false);
    }

    if (isMaxFeatureSizeActive){
        maxFeatureSizeSpinBox->setEnabled(true);
        maxFeatureSizeCheckBox->setChecked(true);
    }
    else{
        maxFeatureSizeSpinBox->setEnabled(false);
        maxFeatureSizeCheckBox->setChecked(false);
    }
    if (isxGridActive){
        xGridPatternCheckBox->setChecked(true);
        xGridPatternSpinBox->setEnabled(true);
    }
    else{
        xGridPatternCheckBox->setChecked(false);
        xGridPatternSpinBox->setEnabled(false);
    }
    if (isyGridActive){
        yGridPatternCheckBox->setChecked(true);
        yGridPatternSpinBox->setEnabled(true);
    }
    else{
        yGridPatternCheckBox->setChecked(false);
        yGridPatternSpinBox->setEnabled(false);
    }
    if (iszGridActive){
        zGridPatternCheckBox->setChecked(true);
        zGridPatternSpinBox->setEnabled(true);
    }
    else{
        zGridPatternCheckBox->setChecked(false);
        zGridPatternSpinBox->setEnabled(false);
    }
    if (isMaxDisplacementActive){
        displacementLimitCheckBox->setChecked(true);
        displacementLimitSpinBox->setEnabled(true);
    }
    else{
        displacementLimitCheckBox->setChecked(false);
        displacementLimitSpinBox->setEnabled(false);
    }
    if (isStressSafetyActive){
        stressSafetyFactorCheckBox->setChecked(true);
        stressSafetyFactorSpinBox->setEnabled(true);
    }
    else{
        stressSafetyFactorCheckBox->setChecked(false);
        stressSafetyFactorSpinBox->setEnabled(false);
    }
    if (isMinEigenActive){
        modalLimitCheckBox->setChecked(true);
        modalLimitSpinBox->setEnabled(true);
    }
    else{
        modalLimitCheckBox->setChecked(false);
        modalLimitSpinBox->setEnabled(false);
    }

    if (isTemperatureLimitActive){
        temperatureLimitCheckBox->setChecked(true);
        temperatureLimitSpinBox->setEnabled(true);
    }
    else{
        temperatureLimitCheckBox->setChecked(false);
        temperatureLimitSpinBox->setEnabled(false);
    }

    drawDirectionComboBox->setCurrentIndex(drawDirection);
    extrudeComboBox->setCurrentIndex(extrude);
    switch (AMBuildDirection){
        case 1: AMBuildComboBox->setCurrentIndex(0); break;// +X
        case -1: AMBuildComboBox->setCurrentIndex(1); break;// -X
        case 2: AMBuildComboBox->setCurrentIndex(2); break;// +Y
        case -2: AMBuildComboBox->setCurrentIndex(3); break;// -Y
        case 3: AMBuildComboBox->setCurrentIndex(4); break;// +Z
        case -3: AMBuildComboBox->setCurrentIndex(5); break;// -Z
    }
    cyclicSymmetryComboBox->setCurrentIndex(cylicSymmetry);
    xGridPatternSpinBox->setValue(xGridPattern);
    yGridPatternSpinBox->setValue(yGridPattern);
    zGridPatternSpinBox->setValue(zGridPattern);
    // feature size cannot be smaller than mesh size
    minFeatureSize = MAX(minFeatureSize,topopt.getMeshElementSize());
    minFeatureSizeSpinBox->setValue(minFeatureSize);
    maxFeatureSizeSpinBox->setValue(maxFeatureSize);
    stressSafetyFactorSpinBox->setValue(stressSafety);

    displacementLimitSpinBox->setValue(maxDisp);
    modalLimitSpinBox->setValue(minEig);
    temperatureLimitSpinBox->setValue(temperatureLimit);
    xSymmetryCheckBox->setChecked(xSymmetry);
    ySymmetryCheckBox->setChecked(ySymmetry);
    zSymmetryCheckBox->setChecked(zSymmetry);
    keepFixedFacesCheckBox->setChecked(keepDir);

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;
    viewer->updateGL();
}
/*****************************************************************/
ConstraintsDialog::~ConstraintsDialog()
{
    ;
}
///////////////////////////////////////////////////////////////////////////////
void ConstraintsDialog :: makeConnections()
{
    CheckBox( drawDirectionCheckBox, [=]{ updateCheckbox();});
    CheckBox( extrudeCheckBox, [=]{ updateCheckbox();});
    CheckBox( AMBuildCheckBox, [=]{ AMBuildMessage();updateCheckbox();});
    CheckBox( cyclicSymmetryCheckBox, [=]{ updateCheckbox();});
    CheckBox( xGridPatternCheckBox, [=]{ updateCheckbox();});
    CheckBox( yGridPatternCheckBox, [=]{ updateCheckbox();});
    CheckBox( zGridPatternCheckBox, [=]{ updateCheckbox();});
    CheckBox( minFeatureSizeCheckBox, [=]{ updateCheckbox();});
    CheckBox( maxFeatureSizeCheckBox, [=]{ updateCheckbox();});
    CheckBox( stressSafetyFactorCheckBox, [=]{ updateCheckbox();});
    CheckBox( displacementLimitCheckBox, [=]{ updateCheckbox();});
    CheckBox( modalLimitCheckBox, [=]{ updateCheckbox();});
    CheckBox( temperatureLimitCheckBox, [=]{ updateCheckbox();});
    ComboBox( drawDirectionComboBox, [=]{ updateConstraints();});
    ComboBox( extrudeComboBox, [=]{ updateConstraints();});
    ComboBox( AMBuildComboBox, [=]{ updateConstraints();});
    ComboBox( cyclicSymmetryComboBox, [=]{ updateConstraints();});
    SpinBoxd( minFeatureSizeSpinBox, [=]{ updateConstraints();});
    SpinBoxd( maxFeatureSizeSpinBox, [=]{ updateConstraints();});
    SpinBoxi( xGridPatternSpinBox, [=]{ updateConstraints();});
    SpinBoxi( yGridPatternSpinBox, [=]{ updateConstraints();});
    SpinBoxi( zGridPatternSpinBox, [=]{ updateConstraints();});
    SpinBoxd( stressSafetyFactorSpinBox, [=]{ updateConstraints();});
    SpinBoxd( displacementLimitSpinBox, [=]{ updateConstraints();});
    CheckBox( xSymmetryCheckBox, [=]{ updateConstraints();});
    CheckBox( ySymmetryCheckBox, [=]{ updateConstraints();});
    CheckBox( zSymmetryCheckBox, [=]{ updateConstraints();});
    CheckBox( keepTopologyConnected, [=]{ updateConstraints();});
    CheckBox( keepFixedFacesCheckBox, [=]{ updateConstraints();});
    PushButton( applyPushButton, [=]{ closeDialog();});
}
