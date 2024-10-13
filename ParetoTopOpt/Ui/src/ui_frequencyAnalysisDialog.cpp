#include "ui_frequencyAnalysisDialog.hpp"
#include "CoreFunctions.hpp"
#include "CoreFunctions.hpp"


extern bool getYesNoResponse(const char format[],...);
extern CoreFunctions           coreFunctions;
/////////////////////////////////////////////////////
FrequencyAnalysisDialog::FrequencyAnalysisDialog( QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    makeConnections();

    meshElementsMap[0] = 10000;
    meshElementsMap[1] = 25000;
    meshElementsMap[2] = 50000;
    meshElementsMap[3] = 100000;
    meshElementsMap[4] = 250000;

}
/////////////////////////////////////////////////////
FrequencyAnalysisDialog::~FrequencyAnalysisDialog()
{
    deltaPlotWindow.close();
    phasePlotWindow.close();

}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::showEvent(QShowEvent *e)
{
    //first store values before updating, else liveVar gets updated
    int nelems;
    double alphaM = liveVar.structuralDynamics.alphaM;
    double betaK = liveVar.structuralDynamics.betaK;
    double frequencyStart = 100;
    double frequencyEnd = 1000;
    double frequencyStep = 100;

    bool meshApplied = liveVar.MESH_APPLIED;
    bool nodeBased = liveVar.nodeBasedKU;


    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;

    nelems = liveVar.nElemsToMesh;
    numElemsSpinBox->setValue(nelems);


    elemQualityCombo->setCurrentIndex(0);
    for (int i = 1; i < 5; i++)
        if (nelems >= meshElementsMap[i])
            elemQualityCombo->setCurrentIndex(i);

    nodeBasedCheckBox->setChecked(nodeBased);

    alphaMSpinBox->setValue(alphaM);
    betaKSpinBox->setValue(betaK);
    frequencyStartSpinBox->setValue(frequencyStart);
    frequencyEndSpinBox->setValue(frequencyEnd);
    frequencyStepSpinBox->setValue(frequencyStep);

    liveVar.MESH_APPLIED = meshApplied;// save the previous value
    liveVar.nodeBasedKU = nodeBased;
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::updateNumberOfElements(void)
{
    int quality = elemQualityCombo->currentIndex();
    numElemsSpinBox->setValue(meshElementsMap[quality]);
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::updateLoadSetCallBack(void)
{

   liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
   liveVar.displayField = DISPLAY_FIELD_NONE;
   liveVar.plotElasticityBC = true;
   viewer->updateGL();
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::updateLiveVariables(void)
{
    liveVar.MESH_APPLIED = false;
    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.nElemsToMesh = numElemsSpinBox->value();
    liveVar.nWorkSpaceGroups = coreFunctions.getRecommendedDeflationGroups(liveVar.nElemsToMesh);
    liveVar.nodeBasedKU = (int)nodeBasedCheckBox->isChecked();

    liveVar.displayGeom = DISPLAY_GEOM_INITIAL;
    liveVar.displayField = DISPLAY_FIELD_NONE;
    liveVar.plotElasticityBC = true;
    liveVar.plotTopOptConstraints = true;

    viewer->updateGL();
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::onFEAFinished()
{
    viewer->updateGL();
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::solveFrequencyResponse()
{
    liveVar.nElemsToMesh = numElemsSpinBox->value();

    if (liveVar.nElemsToMesh  > 10000){
        if (!getYesNoResponse("Large number of elements ... analysis will be slow; continue?"))
            return;
    }
    if (frequencyStartSpinBox->value() > frequencyEndSpinBox->value()){
        writeWarningToGUIWindow("Start Frequency must be less than End Frequency");
        return;
    }

    liveVar.structuralDynamics.alphaM = alphaMSpinBox->value();
    liveVar.structuralDynamics.betaK = betaKSpinBox->value();

    coreFunctions.solveFrequencyResponse();

    plotFrequencyResponse();
    viewer->updateGL();
}
/************************************************************/
void FrequencyAnalysisDialog :: plotData(int nSamples, double *xData,double *yData,
                                         const char *Title, const char *XLabel,
                                         const char *YLabel,QMainWindow &dataWindow)
{
    QFont font;
    QLineSeries *spline = new QLineSeries();
    QScatterSeries *scatter = new QScatterSeries();
    font.setPointSize(16);

    char title[256];
    sprintf(title,Title);
    spline->setName(title);
    spline->setPointLabelsFont(font);
    QPen Pen;
    Pen.setColor("blue");
    Pen.setStyle(Qt::SolidLine);
    Pen.setWidth(4);
    spline->setPen(Pen);

    double maxValue = -HUGE_VAL,minValue = HUGE_VAL;
    double maxXValue = -HUGE_VAL,minXValue = HUGE_VAL;

    for (int i =nSamples-1; i >=0;i--){	// There are N+1 samples
        maxXValue = MAX(maxXValue,xData[i]);
        minXValue = MIN(minXValue,xData[i]);
        maxValue = MAX(maxValue,yData[i]);
        minValue = MIN(minValue,yData[i]);
        spline->append(xData[i], yData[i]);
        scatter->append(xData[i], yData[i]);
    }

    QChart *chart = new QChart();

    chart->addSeries(spline);
    chart->addSeries(scatter);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText(XLabel);
    font.setPointSize(14);
    axisX->setLabelsFont(font);
    axisX->setTitleFont(font);
    axisX->setRange(minXValue,maxXValue);
    chart->addAxis(axisX, Qt::AlignBottom);

    spline->attachAxis(axisX);
    scatter->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText(YLabel);
    font.setPointSize(14);
    axisY->setLabelsFont(font);
    axisY->setTitleFont(font);
    axisY->setLabelFormat("%.3g");
    axisY->setRange(minValue -0.1 *(maxValue-minValue),maxValue +0.1 *(maxValue-minValue));
    chart->addAxis(axisY, Qt::AlignLeft);

    QPen axisPen;
    axisPen.setWidth(2);
    axisX->setGridLinePen(axisPen);
    axisY->setGridLinePen(axisPen);

    spline->attachAxis(axisY);
    scatter->attachAxis(axisY);

    font.setPointSize(16);
    chart->setTitleFont(font);
    chart->setTitle(title);
    chart->legend()->hide();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    dataWindow.setCentralWidget(chartView);
    dataWindow.resize(800, 550);
    dataWindow.show();

}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::plotFrequencyResponse()
{
    int nFrequencyPts;
    double omegaHz[1000],deltaMax[1000], phaseAngleRadians[1000],maxDeformation;
    nFrequencyPts = topopt.getFrequencyData(omegaHz,deltaMax,phaseAngleRadians);
    if (nFrequencyPts == 0) return;

    deltaPlotWindow.close();
    phasePlotWindow.close();
    string title1;
    maxDeformation = 0;
    for (int i = 0; i < nFrequencyPts; i++)
        maxDeformation = MAX(maxDeformation,deltaMax[i]);

    title1 = string("Pareto Deformation; max: ") + to_string(maxDeformation);
    plotData(nFrequencyPts,omegaHz,deltaMax,title1.c_str(),"Freq (Hz)","Displacement",deltaPlotWindow);
    plotData(nFrequencyPts,omegaHz,phaseAngleRadians,"Pareto Phase Plot","Freq (Hz)","Phase (degrees)",phasePlotWindow);

}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::stopFrequencyResponse()
{
    ;
}
/////////////////////////////////////////////////////
void FrequencyAnalysisDialog::closeFrequencyAnalysisDialog()
{
    viewer->updateGL();
    parentWidget()->show();
    close();
}
/**********************************************************/
void FrequencyAnalysisDialog::makeConnections()
{
    ComboBox( elemQualityCombo, [=]{updateNumberOfElements();});
    SpinBoxi( numElemsSpinBox, [=]{ updateLiveVariables();});
    CheckBox( nodeBasedCheckBox, [=]{ updateLiveVariables();});
    PushButton( frequencyResponseSolvePushButton, [=]{ solveFrequencyResponse();});
    PushButton( frequencyResponseStopPushButton, [=]{ stopFrequencyResponse();});
}
/////////////////////////////////////////////////////
