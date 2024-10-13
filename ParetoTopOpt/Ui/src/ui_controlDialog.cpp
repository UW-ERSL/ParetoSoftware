#include "ui_controlDialog.hpp"
#include <iostream>

///////////////////////////////////////////////////////////////////////////////

ControlDialog :: ControlDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
    updatePushButtons();
    QFile HelpFile(":/resources/ParetoGuide.pdf");;
    HelpFile.copy(qApp->applicationDirPath().append("/ParetoGuide.pdf"));
}
///////////////////////////////////////////////////////////////////////////////

ControlDialog :: ~ControlDialog()
{
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openVerboseDialog()
{
    if( nullptr == verboseDialog )
        verboseDialog = new VerboseDialog( this );
    verboseDialog->setViewer( viewer );
    verboseDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openUnitDialog()
{

    if( nullptr == unitDialog )
        unitDialog = new UnitsDialog( this );
    QObject::connect( unitDialog, SIGNAL(setUnitsComplete()), this, SLOT(updatePushButtons()));
    unitDialog->setViewer( viewer );
    unitDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openGeometryDialog()
{
    if (!liveVar.UNITS_APPLIED){
        writeWarningToGUIWindow("Units are not set");
        return;
    }
    if( nullptr == geomDialog )
        geomDialog = new GeometryDialog( this );
    QObject::connect( geomDialog, SIGNAL(setGeometryComplete()), this, SLOT(updatePushButtons()));
    geomDialog->setViewer( viewer );
    geomDialog->show();

    //updatePushButtons();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openZernikeDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == zernikeDialog )
        zernikeDialog = new ZernikeDialog( this );

    zernikeDialog->setViewer( viewer );
    zernikeDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openMaterialDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == materialDialog )
        materialDialog = new MaterialDialog( this );
    QObject::connect( materialDialog, SIGNAL(setMaterialComplete()), this, SLOT(updatePushButtons()));
    materialDialog->setViewer( viewer );
    materialDialog->updatePropertyLineEdits();
    materialDialog->updateMaterialLabelText();
    materialDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openStructuralBoundCondDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == structuralLoadsWindow )
        structuralLoadsWindow = new StructuralLoadsWindow( this );
    QObject::connect( structuralLoadsWindow, SIGNAL(setStructuralLoadsComplete()), this, SLOT(updatePushButtons()));
    structuralLoadsWindow->setViewer( viewer );
    structuralLoadsWindow->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openThermalBoundCondDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == thermalLoadsWindow )
        thermalLoadsWindow = new ThermalLoadsWindow( this );
    QObject::connect( thermalLoadsWindow, SIGNAL(setThermalLoadsComplete()), this, SLOT(updatePushButtons()));
    thermalLoadsWindow->setViewer( viewer );
    thermalLoadsWindow->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openAccelerationDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == accelDialog )
        accelDialog = new AccelerationDialog( this );
    QObject::connect( accelDialog, SIGNAL(setAccelerationComplete()), this, SLOT(updatePushButtons()));
    accelDialog->setViewer( viewer );
    accelDialog->updateText();
    accelDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openDisplayOptionsDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == displayOptionsDialog )
        displayOptionsDialog = new DisplayOptionsDialog( this );
    displayOptionsDialog->setViewer( viewer );
    displayOptionsDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openStructuralAnalysisDialog()
{
    if ((!liveVar.MATERIAL_APPLIED) ){
        writeWarningToGUIWindow("Material has not been defined");
        return;
    }
    if ((!liveVar.BC_STRUCTURAL_APPLIED) && (!liveVar.BC_THERMAL_APPLIED) ){
        writeWarningToGUIWindow("Loads have not been applied");
        return;
    }
    if( nullptr == structuralAnalysisDialog )
        structuralAnalysisDialog = new StructuralAnalysisDialog( this );
    QObject::connect( structuralAnalysisDialog, SIGNAL(setFeaComplete()), this, SLOT(updatePushButtons()));
    structuralAnalysisDialog->setViewer( viewer );
    structuralAnalysisDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openFrequencyAnalysisDialog()
{
    if ((!liveVar.MATERIAL_APPLIED) ){
        writeWarningToGUIWindow("Material has not been defined");
        return;
    }
    if ((!liveVar.BC_STRUCTURAL_APPLIED) && (!liveVar.BC_THERMAL_APPLIED) ){
        writeWarningToGUIWindow("Loads have not been applied");
        return;
    }
    if( nullptr == frequencyAnalysisDialog )
        frequencyAnalysisDialog = new FrequencyAnalysisDialog( this );

    frequencyAnalysisDialog->setViewer( viewer );
    frequencyAnalysisDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog:: openElastoPlasticAnalysisDialog()
{

    if( nullptr == elastoPlasticAnalysisDialog )
        elastoPlasticAnalysisDialog = new ElastoPlasticAnalysisDialog( this );
    elastoPlasticAnalysisDialog->setViewer( viewer );
    elastoPlasticAnalysisDialog->show();

}

///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openConstraintsDialog()
{
    if ((!liveVar.MATERIAL_APPLIED) ){
        writeWarningToGUIWindow("Material has not been defined");
        return;
    }
    if ((!liveVar.BC_STRUCTURAL_APPLIED) && (!liveVar.BC_THERMAL_APPLIED) ){
        writeWarningToGUIWindow("Loads have not been applied");
        return;
    }
    if( nullptr == constraintsDialog )
        constraintsDialog = new ConstraintsDialog( this );
    QObject::connect( constraintsDialog, SIGNAL(setConstraintsComplete()), this, SLOT(updatePushButtons()));
    constraintsDialog->setViewer( viewer );
    constraintsDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openOptimizeTopologyDialog()
{
    if ((!liveVar.CONSTRAINTS_APPLIED) ){
        writeWarningToGUIWindow("Constraints have not been applied");
        return;
    }

    if( nullptr == topOptDialog )
        topOptDialog = new TopologyOptimizationDialog( this );
    QObject::connect( topOptDialog, SIGNAL(setTopOptComplete()), this, SLOT(updatePushButtons()));
    topOptDialog->setViewer( viewer );
    topOptDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openThermalOptimizationDialog()
{
    if ((!liveVar.CONSTRAINTS_APPLIED) ){
        writeWarningToGUIWindow("Constraints have not been applied");
        return;
    }

    cout << "here " << endl;
    if( nullptr == thermalOptDialog )
        thermalOptDialog = new ThermalOptimizationDialog( this );

    thermalOptDialog->setViewer( viewer );
    thermalOptDialog->show();

}

///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openAutoDesignDialog()
{

    if ((!liveVar.TOPOPT_SOLVED) ){
        writeWarningToGUIWindow("Model not yet optimized");
        return;
    }
    if( nullptr == autoDesignDialog )
        autoDesignDialog = new AutoDesignDialog( this );
    autoDesignDialog->setViewer( viewer );
    autoDesignDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openMeshMergingDialog()
{

    if ((!liveVar.TOPOPT_SOLVED) ){
        writeWarningToGUIWindow("Model not yet optimized");
        return;
    }
    if( nullptr == meshMergingDialog )
        meshMergingDialog = new MeshMergingDialog( this );
    meshMergingDialog->setViewer( viewer );
    meshMergingDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openMeshProjectionDialog()
{

    if ((!liveVar.TOPOPT_SOLVED) ){
        writeWarningToGUIWindow("Model not yet optimized");
        return;
    }
    if( nullptr == meshProjectionDialog )
        meshProjectionDialog = new MeshProjectionDialog( this );
    meshProjectionDialog->setViewer( viewer );
    meshProjectionDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openOptimizeMaterialDialog()
{
    if ((!liveVar.CONSTRAINTS_APPLIED) ){
        writeWarningToGUIWindow("Constraints have not been applied");
        return;
    }


    if( nullptr == materialOptimizeDialog )
        materialOptimizeDialog = new MaterialOptimizationDialog( this );

    materialOptimizeDialog->setViewer( viewer );
    materialOptimizeDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openResultsDialog()
{
    if ((!liveVar.TOPOPT_SOLVED) ){
        writeWarningToGUIWindow("Model not yet optimized");
        return;
    }
    if( nullptr == optResultDialog )
        optResultDialog = new OptimizationResultDialog( this );
    optResultDialog->setViewer( viewer );
    optResultDialog->show();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openLatticeStructureDialog()
{
    if (!liveVar.GEOMETRY_UPLOADED){
        writeWarningToGUIWindow("Geometry not loaded");
        return;
    }
    if( nullptr == latticeStructureDialog )
        latticeStructureDialog = new LatticeStructureDialog( this );
    latticeStructureDialog->setViewer( viewer );
    latticeStructureDialog->show();
}

///////////////////////////////////////////////////////////////////////////////
void ControlDialog::closeControlDialog()
{
    parentWidget()->show();
    close();
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openAMBuildDialog()
{
    if( nullptr == amBuildDialog )
        amBuildDialog = new AMBuildDialog( this );
    QObject::connect( amBuildDialog, SIGNAL(setAMBuildLoadingComplete()), this, SLOT(updatePushButtons()));
    amBuildDialog->setViewer( viewer );
    amBuildDialog->show();

}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog::openProjectsDialog()
{
    if( nullptr == projectDialog )
        projectDialog = new ProjectDialog( this );
    QObject::connect( projectDialog, SIGNAL(setProjectLoadingComplete()), this, SLOT(updatePushButtons()));
    projectDialog->setViewer( viewer );
    projectDialog->show();

}
/********************************************************/
void ControlDialog::updatePushButtons()
{

    const char *leftAlign ="Text-align:left";
    const char *notAllowed = ":/resources/notAllowed.png";
    const char *available = ":/resources/available.png";
    const char *done = ":/resources/done.png";


    verbosePushButton->setStyleSheet(leftAlign);
    unitPushButton->setStyleSheet(leftAlign);
    geometryPushButton->setStyleSheet(leftAlign);
    ZernikePushButton->setStyleSheet(leftAlign);
    materialPushButton->setStyleSheet(leftAlign);
    displayOptionsPushButton->setStyleSheet(leftAlign);
    structuralLoadsPushButton->setStyleSheet(leftAlign);
    thermalLoadsPushButton->setStyleSheet(leftAlign);
    accelerationPushButton->setStyleSheet(leftAlign);
    StructuralAnalysisPushButton->setStyleSheet(leftAlign);
    FrequencyAnalysisPushButton->setStyleSheet(leftAlign);
    ElastoPlasticAnalysisPushButton->setStyleSheet(leftAlign);
    constraintsPushButton->setStyleSheet(leftAlign);
    optimizeTopologyPushButton->setStyleSheet(leftAlign);
    optimizeThermalPushButton->setStyleSheet(leftAlign);
    optimizeMaterialPushButton->setStyleSheet(leftAlign);
    autoDesignPushButton->setStyleSheet(leftAlign);
    meshMergingPushButton->setStyleSheet(leftAlign);
    meshProjectionPushButton->setStyleSheet(leftAlign);
    resultPushButton->setStyleSheet(leftAlign);
    latticeStructurePushButton->setStyleSheet(leftAlign);
    AMBuildPushButton->setStyleSheet(leftAlign);
    projectsPushButton->setStyleSheet(leftAlign);
    helpPushButton->setStyleSheet(leftAlign);


    verbosePushButton->setIcon(QIcon(notAllowed));
    unitPushButton->setIcon(QIcon(notAllowed));
    geometryPushButton->setIcon(QIcon(notAllowed));
    ZernikePushButton->setIcon(QIcon(notAllowed));
    materialPushButton->setIcon(QIcon(notAllowed));
    displayOptionsPushButton->setIcon(QIcon(notAllowed));
    structuralLoadsPushButton->setIcon(QIcon(notAllowed));
    FrequencyAnalysisPushButton->setIcon(QIcon(notAllowed));
    thermalLoadsPushButton->setIcon(QIcon(notAllowed));
    accelerationPushButton->setIcon(QIcon(notAllowed));
    StructuralAnalysisPushButton->setIcon(QIcon(notAllowed));
    ElastoPlasticAnalysisPushButton->setIcon(QIcon(notAllowed));
    constraintsPushButton->setIcon(QIcon(notAllowed));
    optimizeTopologyPushButton->setIcon(QIcon(notAllowed));
    optimizeThermalPushButton->setIcon(QIcon(notAllowed));
    optimizeMaterialPushButton->setIcon(QIcon(notAllowed));
    autoDesignPushButton->setIcon(QIcon(notAllowed));
    meshMergingPushButton->setIcon(QIcon(notAllowed));
    meshProjectionPushButton->setIcon(QIcon(notAllowed));
    resultPushButton->setIcon(QIcon(notAllowed));
    latticeStructurePushButton->setIcon(QIcon(notAllowed));
    AMBuildPushButton->setIcon(QIcon(notAllowed));
    projectsPushButton->setIcon(QIcon(notAllowed));
    helpPushButton->setIcon(QIcon(notAllowed));

    verbosePushButton->setIcon(QIcon(available));
    unitPushButton->setIcon(QIcon(available));
    projectsPushButton->setIcon(QIcon(available));
    helpPushButton->setIcon(QIcon(available));

    unitPushButton->setIcon(QIcon(available));// to allow drag and drop
    displayOptionsPushButton->setIcon(QIcon(available)); // to allow drag and drop
    geometryPushButton->setIcon(QIcon(available)); // to allow drag and drop

    if (liveVar.UNITS_APPLIED){
        unitPushButton->setIcon(QIcon(done));
        geometryPushButton->setIcon(QIcon(available));
    }

    if (liveVar.GEOMETRY_UPLOADED){
        geometryPushButton->setIcon(QIcon(done));
        ZernikePushButton->setIcon(QIcon(available));
        materialPushButton->setIcon(QIcon(available));
        displayOptionsPushButton->setIcon(QIcon(available));
        structuralLoadsPushButton->setIcon(QIcon(available));
        thermalLoadsPushButton->setIcon(QIcon(available));
        accelerationPushButton->setIcon(QIcon(available));

        latticeStructurePushButton->setIcon(QIcon(available));
        AMBuildPushButton->setIcon(QIcon(available));
    }
    if (liveVar.MATERIAL_APPLIED){
        materialPushButton->setIcon(QIcon(done));
    }
    if (liveVar.BC_STRUCTURAL_APPLIED){
        structuralLoadsPushButton->setIcon(QIcon(done));
    }
    if (liveVar.BC_THERMAL_APPLIED){
        thermalLoadsPushButton->setIcon(QIcon(done));
    }
    if (liveVar.ACCELERATION_APPLIED){
        accelerationPushButton->setIcon(QIcon(done));
    }
    if ( (liveVar.MATERIAL_APPLIED) && ((liveVar.BC_STRUCTURAL_APPLIED) ||(liveVar.BC_THERMAL_APPLIED))  ){
        StructuralAnalysisPushButton->setIcon(QIcon(available));
        constraintsPushButton->setIcon(QIcon(available));
        ElastoPlasticAnalysisPushButton->setIcon(QIcon(available));
        FrequencyAnalysisPushButton->setIcon(QIcon(available));
    }

    if (liveVar.STATIC_FEA_SOLVED){
        StructuralAnalysisPushButton->setIcon(QIcon(done));
        ElastoPlasticAnalysisPushButton->setIcon(QIcon(available));
        optimizeThermalPushButton->setIcon(QIcon(available));
    }
    if ( (liveVar.CONSTRAINTS_APPLIED) ){
        constraintsPushButton->setIcon(QIcon(done));
        optimizeTopologyPushButton->setIcon(QIcon(available));
        optimizeThermalPushButton->setIcon(QIcon(available));
        optimizeMaterialPushButton->setIcon(QIcon(available));
    }
    if (liveVar.TOPOPT_SOLVED){
        autoDesignPushButton->setIcon(QIcon(available));
        meshMergingPushButton->setIcon(QIcon(available));
        meshProjectionPushButton->setIcon(QIcon(available));
        resultPushButton->setIcon(QIcon(available));
        optimizeTopologyPushButton->setIcon(QIcon(done));
    }
}
///////////////////////////////////////////////////////////////////////////////
void ControlDialog :: makeConnections()
{
    PushButton( verbosePushButton, [=]{ openVerboseDialog();});
    PushButton( unitPushButton, [=]{ openUnitDialog();});
    PushButton( geometryPushButton, [=]{ openGeometryDialog();});
    PushButton( ZernikePushButton, [=]{ openZernikeDialog();});
    PushButton( materialPushButton, [=]{ openMaterialDialog();});
    PushButton( structuralLoadsPushButton, [=]{ openStructuralBoundCondDialog();});
    PushButton( thermalLoadsPushButton, [=]{ openThermalBoundCondDialog();});
    PushButton( displayOptionsPushButton, [=]{ openDisplayOptionsDialog();});
    PushButton( StructuralAnalysisPushButton, [=]{ openStructuralAnalysisDialog();});
    PushButton( FrequencyAnalysisPushButton, [=]{ openFrequencyAnalysisDialog();});
    PushButton( ElastoPlasticAnalysisPushButton, [=]{ openElastoPlasticAnalysisDialog();});
    PushButton( accelerationPushButton, [=]{ openAccelerationDialog();});
    PushButton( constraintsPushButton,[=]{openConstraintsDialog();});
    PushButton( optimizeTopologyPushButton,[=]{openOptimizeTopologyDialog();});
    PushButton( optimizeThermalPushButton,[=]{openThermalOptimizationDialog();});
    PushButton( autoDesignPushButton,[=]{openAutoDesignDialog();});
    PushButton( meshMergingPushButton,[=]{openMeshMergingDialog();});
    PushButton( meshProjectionPushButton,[=]{openMeshProjectionDialog();});
    PushButton( optimizeMaterialPushButton,[=]{openOptimizeMaterialDialog();});
    PushButton( resultPushButton, [=]{ openResultsDialog();});
    PushButton( latticeStructurePushButton, [=]{ openLatticeStructureDialog();});
    PushButton( AMBuildPushButton, [=]{ openAMBuildDialog();});
    PushButton( projectsPushButton, [=]{ openProjectsDialog();});
    PushButton( helpPushButton, [=]{QDesktopServices::openUrl(QUrl::fromLocalFile(qApp->applicationDirPath().append("/ParetoGuide.pdf")));});
}
///////////////////////////////////////////////////////////////////////////////
