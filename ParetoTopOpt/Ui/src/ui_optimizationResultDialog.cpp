#include "ui_optimizationResultDialog.hpp"
#include "CoreFunctions.hpp"
using namespace QtCharts;
extern void grabImage(const char *fileName);
extern void setStatusMessage(const char str[]);
extern void writeMessageToGUIWindow(const char format[],...);

extern void updateProgressBar(const char title[], int percentComplete);
extern void resetProgressBar(const char title[]);
/////////////////////////////////////////////////////
OptimizationResultDialog::OptimizationResultDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();
}
/*****************************************************/
OptimizationResultDialog::~OptimizationResultDialog()
{

}
/////////////////////////////////////////////////////
void OptimizationResultDialog::saveTopology()
{
    char temp[256];
    string resultFile;
    double volfrac,actualFrac,time,isoCutoff;
    int isoCounter;

    liveVar.displayVolFraction = topologyAtVolSpinBox->value();

    volfrac = liveVar.displayVolFraction;
    isoCounter = topopt.getClosestCounterAtVolFraction(volfrac,&actualFrac,&isoCutoff,&time);
    volfrac = MAX(volfrac,topopt.getCurrentVolumeFraction());
    sprintf(temp,"_%.2f.stl",volfrac);
    resultFile = liveVar.projectDirectory + liveVar.modelName + string(temp);

    writeMessageToGUIWindow("Saving STL to %s", resultFile.c_str());
    topopt.dumpIsoBinary(isoCounter,resultFile.c_str());

    sprintf(temp,"_simplified_%.2f.stl",volfrac);
    resultFile = liveVar.projectDirectory + liveVar.modelName + string(temp);
    writeMessageToGUIWindow("Saving Simplfied STL to %s", resultFile.c_str());
    coreFunctions.saveSimplifiedIsoSurface(isoCounter,2,resultFile.c_str());

    viewer->updateGL();
    setStatusMessage("STL Saved");
}
/////////////////////////////////////////////////////
void OptimizationResultDialog::saveCleanTopologyAtVol()
{
    //coreFunctions.mergeCADandIsosurfaceMesh();
    char temp[256];
    string resultFile;
    double volfrac,actualFrac,time,isoCutoff;
    int isoCounter;

    liveVar.displayVolFraction = topologyAtVolSpinBox->value();

    volfrac = liveVar.displayVolFraction;
    isoCounter = topopt.getClosestCounterAtVolFraction(volfrac,&actualFrac,&isoCutoff,&time);
    volfrac = MAX(volfrac,topopt.getCurrentVolumeFraction());
    sprintf(temp,"_%.2f.simpMesh",volfrac);
    resultFile = liveVar.projectDirectory + liveVar.modelName + string(temp);

    writeMessageToGUIWindow("Saving STL to %s", resultFile.c_str());
    topopt.dumpMeshWithTSfield(resultFile.c_str());

    viewer->updateGL();
    setStatusMessage("STL Saved");


    viewer->updateGL();
}
/*******************************************/
void OptimizationResultDialog::setTopologyAtVol()
{
    liveVar.displayVolFraction = topologyAtVolSpinBox->value();
    viewer->updateGL();
}
///////////////////////////////////////////////////////////////////////////////
void OptimizationResultDialog ::showEvent(QShowEvent *e)
{

    double volfrac;
    volfrac = topopt.getCurrentVolumeFraction();

    topologyAtVolSpinBox->setValue(volfrac);
    topologyAtVolSpinBox->setRange(volfrac,1.0);

    loadSpinBox->setRange(0,liveVar.nBCSet-1);
}
///////////////////////////////////////////////////////////////////////////////
void OptimizationResultDialog :: closeDialog()
{
    parentWidget()->show();
    close();
}
/************************************************************/
void OptimizationResultDialog :: loadMesh()
{

}
/************************************************************/
void OptimizationResultDialog :: saveMesh()
{

}
/************************************************************/
void OptimizationResultDialog :: saveMeshSurface()
{

}
/************************************************************/
void OptimizationResultDialog :: plotParetoCurves()
{
    plotComplianceCurve();
    plotStressCurve();
    plotMaxTempCurve();
    plotEigenCurve();
}

/************************************************************/
void OptimizationResultDialog :: plotComplianceCurve()
{
    int N;
    string strUnits;
    double volFractions[MAX_ISO_SAVES],complianceData[MAX_BC_SET][MAX_ISO_SAVES],relmaxDisp[MAX_ISO_SAVES],stressData[MAX_BC_SET][MAX_ISO_SAVES],timeTaken[MAX_ISO_SAVES];

    N = topopt.getCurrentIsoCounter();
    // There are N+1 samples
    topopt.getResultsAtAllVolFracs(volFractions,relmaxDisp,stressData,complianceData,timeTaken);


    QFont font;
    QSplineSeries *compliance = new QSplineSeries();
    QScatterSeries *complianceScatter = new QScatterSeries();
    font.setPointSize(16);
    int loadNum;
    loadNum = loadSpinBox->value();
    char title[256];
    sprintf(title,"Compliance; loadset %d",loadNum);
    compliance->setName(title);

    compliance->setPointLabelsFont(font);
    QPen compliancePen;
    compliancePen.setColor("blue");
    compliancePen.setStyle(Qt::SolidLine);
    compliancePen.setWidth(4);
    compliance->setPen(compliancePen);

    double maxValue = -HUGE_VAL,minValue = HUGE_VAL;
    double maxXValue = -HUGE_VAL,minXValue = HUGE_VAL;
    for (int i =N; i >=0;i--){	// There are N+1 samples
        maxXValue = MAX(maxXValue,volFractions[i]);
        minXValue = MIN(minXValue,volFractions[i]);
        maxValue = MAX(maxValue,complianceData[loadNum][i]);
        minValue = MIN(minValue,complianceData[loadNum][i]);
        compliance->append(volFractions[i], complianceData[loadNum][i]);
        complianceScatter->append(volFractions[i], complianceData[loadNum][i]);
    }

    QChart *chart = new QChart();

    chart->addSeries(compliance);
    chart->addSeries(complianceScatter);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Volume fraction");
    font.setPointSize(14);
    axisX->setLabelsFont(font);
    axisX->setTitleFont(font);
    axisX->setRange(MAX(0,minXValue -0.1 *(maxXValue-minXValue)),maxXValue);
    chart->addAxis(axisX, Qt::AlignBottom);

    compliance->attachAxis(axisX);
    complianceScatter->attachAxis(axisX);

    coreFunctions.getPropertyUnits(PROPERTY_WORK,strUnits);
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Compliance(" + QString::fromStdString(strUnits) + ")");
    font.setPointSize(14);
    axisY->setLabelsFont(font);
    axisY->setTitleFont(font);
    axisY->setLabelFormat("%.3g");
    axisY->setRange(MAX(0,minValue -0.1 *(maxValue-minValue)),maxValue +0.1 *(maxValue-minValue));
    chart->addAxis(axisY, Qt::AlignLeft);

    QPen axisPen;
    axisPen.setWidth(2);
    axisX->setGridLinePen(axisPen);
    axisY->setGridLinePen(axisPen);

    compliance->attachAxis(axisY);
    complianceScatter->attachAxis(axisY);

    font.setPointSize(16);
    chart->setTitleFont(font);
    chart->setTitle(title);
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    complianceWindow.setCentralWidget(chartView);
    complianceWindow.resize(800, 550);
    complianceWindow.show();

}
/************************************************************/
void OptimizationResultDialog :: plotStressCurve()
{
    int N;
    string strUnits ;
    double volFractions[MAX_ISO_SAVES],complianceData[MAX_BC_SET][MAX_ISO_SAVES],relmaxDisp[MAX_ISO_SAVES],stressData[MAX_BC_SET][MAX_ISO_SAVES],timeTaken[MAX_ISO_SAVES];

    N = topopt.getCurrentIsoCounter();
    // There are N+1 samples
    topopt.getResultsAtAllVolFracs(volFractions,relmaxDisp,stressData,complianceData,timeTaken);


    QFont font;
    QSplineSeries *stress = new QSplineSeries();
    QScatterSeries *stressScatter = new QScatterSeries();
    font.setPointSize(16);
    stress->setName("Stress");

    stress->setPointLabelsFont(font);
    QPen stressPen;
    stressPen.setColor("blue");
    stressPen.setStyle(Qt::SolidLine);
    stressPen.setWidth(4);
    stress->setPen(stressPen);

    int loadNum;
    loadNum = loadSpinBox->value();
    char title[256];
    sprintf(title,"Max Stress; loadset %d",loadNum);


    double maxValue = -HUGE_VAL,minValue = HUGE_VAL;
    double maxXValue = -HUGE_VAL,minXValue = HUGE_VAL;
    for (int i =N; i >=0;i--){	// There are N+1 samples
        maxXValue = MAX(maxXValue,volFractions[i]);
        minXValue = MIN(minXValue,volFractions[i]);
        maxValue = MAX(maxValue,stressData[loadNum][i]);
        minValue = MIN(minValue,stressData[loadNum][i]);
        stress->append(volFractions[i], stressData[loadNum][i]);
        stressScatter->append(volFractions[i], stressData[loadNum][i]);
    }

    QChart *chart = new QChart();

    chart->addSeries(stress);
    chart->addSeries(stressScatter);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Volume fraction");
    font.setPointSize(14);
    axisX->setLabelsFont(font);
    axisX->setTitleFont(font);
    axisX->setRange(MAX(0,minXValue -0.1 *(maxXValue-minXValue)),maxXValue);
    chart->addAxis(axisX, Qt::AlignBottom);


    stress->attachAxis(axisX);
    stressScatter->attachAxis(axisX);

    QValueAxis *axisY= new QValueAxis();

    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,strUnits);
    axisY->setTitleText(" Max. Stress (" + QString::fromStdString(strUnits) + ")");
    axisY->setRange(MAX(0,minValue -0.1 *(maxValue-minValue)),maxValue +0.1 *(maxValue-minValue));
    font.setPointSize(14);
    axisY->setLabelFormat("%.3g");
    axisY->setLabelsFont(font);
    axisY->setTitleFont(font);

    QPen axisPen;
    axisPen.setWidth(2);
    axisX->setGridLinePen(axisPen);
    axisY->setGridLinePen(axisPen);

    chart->addAxis(axisY, Qt::AlignLeft);
    stress->attachAxis(axisY);
    stressScatter->attachAxis(axisY);
    font.setPointSize(16);
    chart->setTitleFont(font);
    chart->setTitle(title);
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    stressWindow.setCentralWidget(chartView);
    stressWindow.resize(800, 550);
    stressWindow.show();

}
/************************************************************/
void OptimizationResultDialog :: plotMaxTempCurve()
{
    int N;
    string strUnits;
    double volFractions[MAX_ISO_SAVES],maxTempData[MAX_BC_SET][MAX_ISO_SAVES];

    N = topopt.getCurrentIsoCounter();
    // There are N+1 samples
    topopt.getMaxTemperatureAtAllVolFracs(volFractions,maxTempData);

    QFont font;
    QSplineSeries *maxTemp = new QSplineSeries();
    QScatterSeries *maxTempScatter = new QScatterSeries();
    font.setPointSize(16);
    int loadNum;
    loadNum = loadSpinBox->value();
    char title[256];
    sprintf(title,"Max Temperature; loadset %d",loadNum);
    maxTemp->setName(title);

    maxTemp->setPointLabelsFont(font);
    QPen Pen;
    Pen.setColor("blue");
    Pen.setStyle(Qt::SolidLine);
    Pen.setWidth(4);
    maxTemp->setPen(Pen);

    double maxValue = -HUGE_VAL,minValue = HUGE_VAL;
    double maxXValue = -HUGE_VAL,minXValue = HUGE_VAL;
    for (int i =N; i >=0;i--){	// There are N+1 samples
        maxXValue = MAX(maxXValue,volFractions[i]);
        minXValue = MIN(minXValue,volFractions[i]);
        maxValue = MAX(maxValue,maxTempData[loadNum][i]);
        minValue = MIN(minValue,maxTempData[loadNum][i]);
        maxTemp->append(volFractions[i], maxTempData[loadNum][i]);
        maxTempScatter->append(volFractions[i], maxTempData[loadNum][i]);
    }

    QChart *chart = new QChart();

    chart->addSeries(maxTemp);
    chart->addSeries(maxTempScatter);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Volume fraction");
    font.setPointSize(14);
    axisX->setLabelsFont(font);
    axisX->setTitleFont(font);
    axisX->setRange(MAX(0,minXValue -0.1 *(maxXValue-minXValue)),maxXValue);
    chart->addAxis(axisX, Qt::AlignBottom);

    maxTemp->attachAxis(axisX);
    maxTempScatter->attachAxis(axisX);

    coreFunctions.getPropertyUnits(PROPERTY_TEMPERATURE,strUnits);
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Temperature(" + QString::fromStdString(strUnits) + ")");
    font.setPointSize(14);
    axisY->setLabelsFont(font);
    axisY->setTitleFont(font);
    axisY->setLabelFormat("%.3g");
    axisY->setRange(MAX(0,minValue -0.1 *(maxValue-minValue)),maxValue +0.1 *(maxValue-minValue));
    chart->addAxis(axisY, Qt::AlignLeft);

    QPen axisPen;
    axisPen.setWidth(2);
    axisX->setGridLinePen(axisPen);
    axisY->setGridLinePen(axisPen);

    maxTemp->attachAxis(axisY);
    maxTempScatter->attachAxis(axisY);

    font.setPointSize(16);
    chart->setTitleFont(font);
    chart->setTitle(title);
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    maxTempWindow.setCentralWidget(chartView);
    maxTempWindow.resize(800, 550);
    maxTempWindow.show();

}
/************************************************************/
void OptimizationResultDialog :: plotEigenCurve()
{
    int N;
    double volFractions[MAX_ISO_SAVES],eigenData[MAX_ISO_SAVES];

    if (!topopt.isEigenConstraintActive()){
        return;
        QMessageBox msgBox;

        msgBox.setText("Modal constraint not imposed during optimization.\n \
                        No modal data available.");
        msgBox.addButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::NonModal);
        msgBox.show();
        msgBox.exec();
        return;
    }
    N = topopt.getCurrentIsoCounter();
    // There are N+1 samples
    topopt.getEigenResultsAtAllVolFracs(volFractions,eigenData);

    QFont font;
    QSplineSeries *eigen = new QSplineSeries();
    QScatterSeries *eigenScatter = new QScatterSeries();
    font.setPointSize(16);
    eigen->setName("First mode frequency");

    eigen->setPointLabelsFont(font);
    QPen pen;
    pen.setColor("blue");
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(4);
    eigen->setPen(pen);

    double maxValue = -HUGE_VAL,minValue = HUGE_VAL;
    double maxXValue = -HUGE_VAL,minXValue = HUGE_VAL;
    for (int i =N; i >=0;i--){	// There are N+1 samples
        maxXValue = MAX(maxXValue,volFractions[i]);
        minXValue = MIN(minXValue,volFractions[i]);
        maxValue = MAX(maxValue,eigenData[i]);
        minValue = MIN(minValue,eigenData[i]);
        eigen->append(volFractions[i], eigenData[i]);
        eigenScatter->append(volFractions[i], eigenData[i]);
    }

    QChart *chart = new QChart();

    chart->addSeries(eigen);
    chart->addSeries(eigenScatter);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Volume fraction");
    font.setPointSize(14);
    axisX->setLabelsFont(font);
    axisX->setTitleFont(font);
    axisX->setRange(MAX(0,minXValue -0.1 *(maxXValue-minXValue)),maxXValue);
    chart->addAxis(axisX, Qt::AlignBottom);


    eigen->attachAxis(axisX);
    eigenScatter->attachAxis(axisX);

    QValueAxis *axisY= new QValueAxis();
    axisY->setTitleText("First mode (Hz)");

    font.setPointSize(14);
    axisY->setLabelFormat("%.3g");
    axisY->setLabelsFont(font);
    axisY->setTitleFont(font);

    QPen axisPen;
    axisPen.setWidth(2);
    axisX->setGridLinePen(axisPen);
    axisY->setGridLinePen(axisPen);

    axisY->setRange(minValue -0.1 *(maxValue-minValue),maxValue +0.1 *(maxValue-minValue));
    chart->addAxis(axisY, Qt::AlignLeft);
    eigen->attachAxis(axisY);
    eigenScatter->attachAxis(axisY);

    font.setPointSize(16);
    chart->setTitleFont(font);
    chart->setTitle("First Modal Curve");
    chart->legend()->hide();


    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    eigenWindow.setCentralWidget(chartView);
    eigenWindow.resize(800, 550);
    eigenWindow.show();

}

/************************************************************/
void OptimizationResultDialog :: plotCGIter()
{

}
/************************************************************/
void OptimizationResultDialog :: createHTMLReport()
{

    string reportFile;
    double xyzMin[3], xyzMax[3];
    double vSize[3],STLVolume,STLArea,center[3];
    int gridSize[3];
    char *dimless = " ";
    string geometryFile,BCFile,FEADispFile,FEAStressFile;
    string MeshFile,TopologyFile,TopologyDispFile,TopologyStressFile;
    string ComplianceCurveFile,StressCurveFile;
    string pressureUnits,massUnits,volumeUnits,areaUnits,lengthUnits,densityUnits;
    coreFunctions.getPropertyUnits(PROPERTY_MASS,massUnits);
    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,pressureUnits);
    coreFunctions.getPropertyUnits(PROPERTY_VOLUME,volumeUnits);
    coreFunctions.getPropertyUnits(PROPERTY_AREA,areaUnits);
    coreFunctions.getPropertyUnits(PROPERTY_PRESSURE,pressureUnits);
    coreFunctions.getPropertyUnits(PROPERTY_LENGTH,lengthUnits);
    coreFunctions.getPropertyUnits(PROPERTY_DENSITY,densityUnits);

    vox.getBoundingBox(xyzMin, xyzMax);
    STLVolume = vox.computeVolumeOfSTLPart(0,&STLArea,center);
    vox.getVoxelSize(vSize);
    vox.getNumGridElems(gridSize);


    //create report folders
    liveVar.reportFolder = liveVar.projectDirectory + liveVar.modelName + string("Reports");
#ifdef _WIN32
    CreateDirectoryA(liveVar.reportFolder.c_str(), NULL);// if does not exists
#endif
    reportFile = liveVar.reportFolder + string("\\") + liveVar.modelName + string("Report.html");

    FILE *fptr = fopen(reportFile.c_str(),"w");
    if (fptr == NULL){
        writeWarningToGUIWindow("Unsuccessful in creating %s",reportFile.c_str());
        return;
    }

    writeMessageToGUIWindow("Creating report");
    geometryFile = liveVar.reportFolder + string("\\") + string("Geometry.png");

    MeshFile = liveVar.reportFolder + string("\\") + string("Mesh.png");
    FEADispFile = liveVar.reportFolder + string("\\") + string("FEADisp.png");
    FEAStressFile = liveVar.reportFolder + string("\\") + string("FEAStress.png");
    TopologyFile = liveVar.reportFolder + string("\\") + string("Topology.png");
    TopologyDispFile = liveVar.reportFolder + string("\\") + string("TopologyDisp.png");
    TopologyStressFile = liveVar.reportFolder + string("\\") + string("TopologyStress.png");

    resetProgressBar("Grabbing images");
    updateProgressBar("Grabbing images",0.0);

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.geomTransparent = false;
    liveVar.plotElasticityBC = false;
    grabImage(geometryFile.c_str());
    updateProgressBar("Grabbing images",10.0);

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.geomTransparent = true;
    liveVar.plotElasticityBC = true;
    for (int bc = 0; bc < liveVar.nBCSet; bc++){
        liveVar.currentBCSet = bc;
        BCFile = liveVar.reportFolder + string("\\") + string("BC") + to_string(bc) + string(".png");
        grabImage(BCFile.c_str());
    }
    liveVar.currentBCSet = 0;
    updateProgressBar("Grabbing images",20.0);

    liveVar.currentBCSet = 0;
    liveVar.displayGeom = DISPLAY_GEOM_MESH;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.geomTransparent = true;
    grabImage(MeshFile.c_str());
    updateProgressBar("Grabbing images",30.0);

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
    liveVar.geomTransparent = true;
    grabImage(FEADispFile.c_str());
    updateProgressBar("Grabbing images",40.0);

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_STRESS;
    liveVar.geomTransparent = true;
    grabImage(FEAStressFile.c_str());
    updateProgressBar("Grabbing images",50.0);

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.geomTransparent = false;
    liveVar.plotElasticityBC = false;
    grabImage(TopologyFile.c_str());
    updateProgressBar("Grabbing images",60.0);

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_DISPLACEMENT;
    liveVar.geomTransparent = false;
    liveVar.plotElasticityBC = false;
    grabImage(TopologyDispFile.c_str());
    updateProgressBar("Grabbing images",70.0);

    liveVar.displayGeom = DISPLAY_GEOM_TOPOLOGY;
    liveVar.displayField = DISPLAY_FIELD_STRESS;
    liveVar.geomTransparent = false;
    liveVar.plotElasticityBC = false;
    grabImage(TopologyStressFile.c_str());
    updateProgressBar("Grabbing images",80.0);

    for (int bc = 0; bc < liveVar.nBCSet; bc++){
        loadSpinBox->setValue(bc);
        ComplianceCurveFile = liveVar.reportFolder + string("\\") + string("ComplianceCurve") + to_string(bc) + string(".png");
        StressCurveFile = liveVar.reportFolder + string("\\") + string("StressCurve") + to_string(bc) + string(".png");

        plotComplianceCurve();// need to fill in the data structure first
        saveComplianceCurve(ComplianceCurveFile.c_str());
        plotStressCurve();
        saveStressCurve(StressCurveFile.c_str());

    }

    updateProgressBar("Grabbing images",100.0);
    resetProgressBar("Creating HTML");

    fprintf(fptr,"<!DOCTYPE html>\n");
    fprintf(fptr,"<html>\n");
    fprintf(fptr,"<head>\n");
    fprintf(fptr,"<meta charset=\"UTF-8\" />");
    fprintf(fptr,"<title>%s Report</title>\n", liveVar.modelName.c_str());
    //fprintf(fptr,"<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" />\n");
    fprintf(fptr,"</head>\n");
    fprintf(fptr,"<body>\n");
    fprintf(fptr,"<h1>1. Geometry</h1>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"Geometry.png\" ALT=\"Geometry\" width=600></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 1: Initial Geometry</strong></p>\n</h1>");

    // Geometry information
    fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
    fprintf(fptr,"<tr>\n");
    fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\"> Quantity </div></th>\n");
    fprintf(fptr,"<th width=\"300\" scope=\"col\"><div align=\"center\">Value</div></th>\n");
    fprintf(fptr,"<th width=\"50\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Volume</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",STLVolume);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",volumeUnits.c_str());
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Surface Area</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",STLArea);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",areaUnits.c_str());
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">#Triangles</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%d</div></td>\n",vox.getNumTriPoints());
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">#Points</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%d</div></td>\n",vox.getNumTriangles());
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Bounding box</div></td>\n");
    //bounding box
    fprintf(fptr,"<td><div align=\"center\">Min: (%f,%f,%f)</div>",xyzMin[0],xyzMin[1],xyzMin[2]);
    fprintf(fptr,"<div align=\"center\">Max: (%f,%f,%f)</div>",xyzMax[0],xyzMax[1],xyzMax[2]);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthUnits.c_str());
    fprintf(fptr,"</tr>\n</table>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Table 1: Geometric Information</strong></p>\n</h2>");
    //material properties
    fprintf(fptr,"<h1>2. Material</h1>\n");
    fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
    fprintf(fptr,"<tr>\n");
    fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Material Property</div></th>\n");
    fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Value</div></th>\n");
    fprintf(fptr,"<th width=\"150\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">E</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.E);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",pressureUnits.c_str());
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">&nu;</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.nu);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">&rho;</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",liveVar.rho);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",densityUnits.c_str());
    fprintf(fptr,"</tr>\n</table>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Table 2: Material Properties</strong></p>\n</h2>");

    //Loading


    fprintf(fptr,"<h1>3. Loading </h1>\n");
    for (int bc = 0; bc < liveVar.nBCSet; bc++){
        string tempStr;
        tempStr = string("<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"BC") + to_string(bc) + string(".png\" ALT=\"BoundaryConditions\" width=\"600 \"></div>\n");
        fprintf(fptr,tempStr.c_str());
        fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 2: Boundary Conditions</strong></p>\n</h2>");
    }
    //FEA
    fprintf(fptr,"<h1>4. Static Analysis </h1>\n");

    fprintf(fptr,"<h2>4.1 Mesh</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"Mesh.png\" ALT=\"Mesh\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 2: Grid Mesh</strong></p>\n</h2>");

    // FEA results
    fprintf(fptr,"<h2>4.2 Stress</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"FEAStress.png\" ALT=\"Stress\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 3: Stress Analysis</strong></p>\n</h2>");

    fprintf(fptr,"<h2>4.3 Displacement</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"FEADisp.png\" ALT=\"Displacement\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 4: Displacement Analysis</strong></p>\n</h2>");

    // FEA TABLE
    fprintf(fptr,"<table align=\"center\" width=\"550\" height=\"150\" border=\"1\">\n");
    fprintf(fptr,"<tr>\n");
    fprintf(fptr,"<th width=\"200\" scope=\"col\"><div align=\"center\">Parameter</div></th>\n");
    fprintf(fptr,"<th width=\"250\" scope=\"col\"><div align=\"center\">Values</div></th>\n");
    fprintf(fptr,"<th width=\"100\" scope=\"col\"><div align=\"center\">Unit</div></th>\n");
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Voxel Size</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">(%f, %f, %f)</div>",vSize[0],vSize[1],vSize[2]);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthUnits.c_str());
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Background Grid</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">(%d, %d, %d)</div>",gridSize[0],gridSize[1],gridSize[2]);
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",dimless);
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Max displacement</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",topopt.getDeformationMax());
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",lengthUnits.c_str());
    fprintf(fptr,"</tr>\n<tr>\n");
    fprintf(fptr,"<td><div align=\"center\">Max Von Mises Stress</div></td>\n");
    fprintf(fptr,"<td><div align=\"center\">%e</div></td>\n",topopt.getMaxVonMisesStress());
    fprintf(fptr,"<td><div align=\"center\">%s</div></td>\n",pressureUnits.c_str());
    fprintf(fptr,"</tr>\n</table>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Table 3: FEA Summary </strong></p>\n</h2>");


    //Modal Analysis
    fprintf(fptr,"<h1>5. Modal Analysis </h1>\n");

    //Optimization
    fprintf(fptr,"<h1>6. Optimization </h1>\n");

    fprintf(fptr,"<h2>6.1 Final Topology</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"Topology.png\" ALT=\"Mesh\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 5: Isosurface</strong></p>\n</h2>");

    fprintf(fptr,"<h2>6.2 Final Stress</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"TopologyStress.png\" ALT=\"Stress\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 6: Final Stress </strong></p>\n</h2>");

    fprintf(fptr,"<h2>6.3 Final Displacement</h2>\n");
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"TopologyDisp.png\" ALT=\"Displacement\" width=\"600\"></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 7: Final Displacement </strong></p>\n</h2>");


    fprintf(fptr,"<h2>6.4 Pareto Curves</h2>\n");
    for (int bc = 0; bc < liveVar.nBCSet; bc++){
        string tempStr;
        tempStr = string("<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"ComplianceCurve") + to_string(bc) + string(".png\" ALT=\"Pareto\" width=\"600\" ></div>\n");
        fprintf(fptr,tempStr.c_str());
        fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 5: Compliance Curve</strong></p>\n</h2>");
    }
    fprintf(fptr,"<div align=\"center\"><IMG style=\"border:2px solid black\" SRC=\"StressCurve.png\" ALT=\"Pareto\" width=\"600\" ></div>\n");
    fprintf(fptr,"<h2><p align=\"center\"><strong>Figure 5: Stress Curve</strong></p>\n</h2>");


    // The END
    fprintf(fptr,"</body>\n");
    fprintf(fptr,"</html>\n");
    fclose(fptr);
}
////////////////////////////////////////////////////////////
void OptimizationResultDialog::saveComplianceCurve(const char *fileName)
{
    Timer timer;
    viewer->updateGL();
    timer.pause(100);
    viewer->updateGL();
    QPixmap p =complianceWindow.grab();
    p.save(QString(fileName));
}
////////////////////////////////////////////////////////////
void OptimizationResultDialog::saveStressCurve(const char *fileName)
{
    Timer timer;
    viewer->updateGL();
    timer.pause(100);
    viewer->updateGL();
    QPixmap p = stressWindow.grab();
    p.save(QString(fileName));
}
////////////////////////////////////////////////////////////
void OptimizationResultDialog::makeConnections()
{

    PushButton(loadMeshPushButton,[=]{loadMesh();});
    PushButton(plotCGIterPushButton,[=]{plotCGIter();});
    PushButton(saveMeshPushButton,[=]{saveMesh();});
    PushButton(createHTMLReportPushButton,[=]{createHTMLReport();});
    PushButton(plotParetoCurvesPushButton,[=]{plotParetoCurves();});
    PushButton(saveMeshSurfacePushButton,[=]{saveMeshSurface();});
    PushButton(saveCleanTopologyPushButton,[=]{saveCleanTopologyAtVol();});
    PushButton(saveTopologyPushButton,[=]{saveTopology();});
    PushButton(closePushButton,[=]{closeDialog();});
    SpinBoxd( topologyAtVolSpinBox, [=]{ setTopologyAtVol();});
}
