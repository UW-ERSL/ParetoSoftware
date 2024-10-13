#include <stdio.h>
#include <ParetoSensor.h>
#include <list>
#include <vector>
#include <CommonGUI.h>
#include <fstream>
#ifdef CHARTLIB
#include <tchar.h>
#include <chartdir.h>
#endif

/**********************************************************************/
ParetoSensor::ParetoSensor(){
    s_numSensors = 0; // no sensors at start
}
/**********************************************************************/
void ParetoSensor::initializeParetoSensor(int sensorNumber,PARETOSENSOR_TYPE sensorType,double defaultValue, 
                                          const char *xLabel,const char *yLabel,const char *title)
{
    s_ParetoSensorType[sensorNumber] = sensorType;

    s_ParetoSensorDefaultValue[sensorNumber] = defaultValue;
    s_ParetoSensorLocation[sensorNumber][0] = 0;
    s_ParetoSensorLocation[sensorNumber][1] = 0;
    s_ParetoSensorLocation[sensorNumber][2] = 0;
    s_data[sensorNumber].clear();
    s_time[sensorNumber].clear();
    strcpy(s_xLabel[sensorNumber],xLabel);
    strcpy(s_yLabel[sensorNumber],yLabel);
    strcpy(s_title[sensorNumber],title);
    s_numSensors++;
}
/**********************************************************************/
void ParetoSensor::setParetoSensorDefaultValue(int sensorNumber,double defaultValue)
{
    s_ParetoSensorDefaultValue[sensorNumber] = defaultValue;
}
/**********************************************************************/
void ParetoSensor::setParetoSensorLocation(int sensorNumber,double pt[3])
{
    s_ParetoSensorLocation[sensorNumber][0] = pt[0];
    s_ParetoSensorLocation[sensorNumber][1] = pt[1];
    s_ParetoSensorLocation[sensorNumber][2] = pt[2];
}
/**********************************************************************/
bool ParetoSensor::sampleParetoSensorNumber(int sensorNumber,double time)
{
    double uvw[3],stresses[6],delta,vonMises,temperature,defaultValue;

    if (time == 0)
        s_time[sensorNumber].push_back(s_data[sensorNumber].size());
    else
        s_time[sensorNumber].push_back(time);

    defaultValue = s_ParetoSensorDefaultValue[sensorNumber];
    switch (s_ParetoSensorType[sensorNumber]) {
    case TEMPERATURE:
        if (!(getTemperatureAt(s_ParetoSensorLocation[sensorNumber],&temperature))){
            s_data[sensorNumber].push_back(defaultValue);
            // printf("Could not capture temperature sensor data at (%.2e,%.2e,%.2e) \n",s_ParetoSensorLocation[sensorNumber][0],s_ParetoSensorLocation[sensorNumber][1],s_ParetoSensorLocation[sensorNumber][2]);
            return false;
        }
        break;
    default:
        if (!(getElasticitySolutionAt(s_ParetoSensorLocation[sensorNumber],uvw,stresses,&vonMises))){
            s_data[sensorNumber].push_back(defaultValue);
            //printf("Could not capture elasticity sensor data at (%.2e,%.2e,%.2e) \n",s_ParetoSensorLocation[sensorNumber][0],s_ParetoSensorLocation[sensorNumber][1],s_ParetoSensorLocation[sensorNumber][2]);
            return false;
        }
        break;
    }
    switch (s_ParetoSensorType[sensorNumber]) {
    case U_DISPLACMENT:
        s_data[sensorNumber].push_back(uvw[0]);
        break;
    case V_DISPLACMENT:
        s_data[sensorNumber].push_back(uvw[1]);
        break;
    case W_DISPLACMENT:
        s_data[sensorNumber].push_back(uvw[2]);
        break;
    case TOTAL_DISPLACEMENT:
        delta = sqrt (uvw[0]*uvw[0] + uvw[1]*uvw[1] + uvw[2]*uvw[2]);
        s_data[sensorNumber].push_back(delta);
        break;
    case TEMPERATURE:
        s_data[sensorNumber].push_back(temperature);
        break;
    case SXX_STRESS:
        s_data[sensorNumber].push_back(stresses[0]);
        break;
    case SYY_STRESS:
        s_data[sensorNumber].push_back(stresses[1]);
        break;
    case SZZ_STRESS:
        s_data[sensorNumber].push_back(stresses[2]);
        break;
    case SXY_STRESS:
        s_data[sensorNumber].push_back(stresses[3]);
        break;
    case SXZ_STRESS:
        s_data[sensorNumber].push_back(stresses[4]);
        break;
    case SYZ_STRESS:
        s_data[sensorNumber].push_back(stresses[5]);
        break;
    case VONMISES_STRESS:
        s_data[sensorNumber].push_back(vonMises);
        break;
    }
    return true;
}
#ifdef CHARTLIB
/**********************************************************************/
void ParetoSensor::plotParetoSensorData(int sensorNumber)
{
    int width = 400, height = 300;
    int nXSamples;
    double *xValues,*yValues,yMax,yMin;

    if (s_data[sensorNumber].size() == 0){
        printf("ParetoSensor: No sensor data \n");
        return;
    }
    print(sensorNumber);// save in .csv format
    nXSamples = s_data[sensorNumber].size();

    xValues = (double *)malloc(nXSamples*sizeof(double ));
    yValues = (double *)malloc(nXSamples*sizeof(double ));
    for (int j = 0; j < nXSamples; j++) {
        xValues[j] = s_time[sensorNumber][j];
        yValues[j] = s_data[sensorNumber][j];
    }

    XYChart *c = new XYChart(width, height, 0xcce6ff, 0x000000, 1);
    c->setPlotArea(70, 40, width-100, height-90, 0xffffff, -1, -1, 0xc0c0c0, -1);
    c->addLegend(70, 15, false, "", 8)->setBackground(Chart::Transparent);
    c->addTitle(s_title[sensorNumber], "arialbd.ttf", 12, 0x000000);

    yMax = *max_element(s_data[sensorNumber].begin(), s_data[sensorNumber].end());
    yMin = *min_element(s_data[sensorNumber].begin(), s_data[sensorNumber].end());

    if ( (fabs(yMax) > 0.1) && (fabs(yMax) < 1000) && (fabs(yMin) > 0.1) && (fabs(yMin) < 1000) )
        c->yAxis()->setLabelFormat("{value|P4}");// precision
    else
        c->yAxis()->setLabelFormat("{value|E2}");

    c->xAxis()->setLabelStep(nXSamples/10.0);
    c->xAxis()->setTitle(s_xLabel[sensorNumber], "arialbd.ttf", 10, 0x000000);
    c->yAxis()->setTitle(s_yLabel[sensorNumber], "arialbd.ttf", 10, 0x000000);

    c->xAxis()->setLabelStep(nXSamples/10.0);

    LineLayer *layer = c->addLineLayer();
    layer->setXData(DoubleArray(xValues, nXSamples));
    layer->addDataSet(DoubleArray(yValues, nXSamples), 0xcf4040, "");

    // Enable data label on the data points. Set the label format to nn%.
    //layer->setDataLabelFormat("{value|0}%");
    char *imageName = "ParetoSensor.svg";
    c->makeChart(imageName);

    free(xValues);
    free(yValues);
    delete c;

    // open the image file
    int nRet= (int)ShellExecute( NULL, _T("open"), _T("ParetoSensor.svg"), NULL, NULL, SW_SHOWNORMAL );
    if ( nRet <= 32 ) {
        printf("Could not open file %s \n",imageName);
    }
}

/**********************************************************************/
#else
void ParetoSensor::plotParetoSensorData(int sensorNumber)
{
    printf("ParetoSensor: Plotting has been disabled in Batch mode \n");
    ;//no plotting in batch mode
}
#endif
/**********************************************************************/
void ParetoSensor:: scaleData(int sensorNumber,double scale,double offset)
{

    for (int i = 0; i < s_data[sensorNumber].size(); i++)
        s_data[sensorNumber].at(i) = scale*s_data[sensorNumber].at(i) + offset;
}
/**********************************************************************/
void ParetoSensor::print(int sensorNumber)
{
    if (s_data[sensorNumber].size() == 0){
        printf("ParetoSensor: No sensor data \n");
        return;
    }
    char fileName[256];
    sprintf(fileName,"Sensor%d.csv",sensorNumber);
    FILE *fptr = fopen(fileName,"w");
    for (int i = 0; i < s_data[sensorNumber].size(); i++){
        fprintf(fptr,"%.3e, %.3e \n",s_time[sensorNumber].at(i),s_data[sensorNumber].at(i));
    }
    fprintf(fptr,"\n");
    fclose(fptr);
}
/**********************************************************************/
void ParetoSensor::getSensorData(int ParetoSensorNumber, std::vector<double> &sensorData , std::vector<double> &sensorTime)
{
    if(ParetoSensorNumber > MAX_PARETOSENSORS)
        return;
    sensorData.clear();
    sensorTime.clear();
    for(int i = 0 ; i < s_data[ParetoSensorNumber].size() ; i++ )
    {
        sensorData.push_back(s_data[ParetoSensorNumber].at(i));
        sensorTime.push_back(s_time[ParetoSensorNumber].at(i));
    }
}
/**********************************************************************/
double ParetoSensor::getSensorDataAtInstance(int sensorNumber,int instance)
{
    if(sensorNumber > MAX_PARETOSENSORS)
        return 0;
    if(instance > s_data[sensorNumber].size())
        return 0;
    return s_data[sensorNumber].at(instance);
}
/**********************************************************************/
int  ParetoSensor::getNumTimeStepsOfSensor(int sensorNumber)
{
    return s_time[sensorNumber].size();
}
/**********************************************************************/
ParetoSensor::~ParetoSensor(void)
{
    for (int sensorNumber = 0; sensorNumber < MAX_PARETOSENSORS; sensorNumber++)
        s_data[sensorNumber].clear();
    return;
}
/**********************************************************************/
void    ParetoSensor::getSensorLocation(int sensorNumber, double location[3])
{
    for(int i = 0 ; i < 3 ; i++)
        location[i] = s_ParetoSensorLocation[sensorNumber][i]; // default to zero
}
/**********************************************************************/
void ParetoSensor::saveSensorsToFile(string fileName)
{
    fstream outFile;
    outFile.open(fileName,ios::out);
    outFile<<s_numSensors<<endl;
    double location[3];
    for(int sensor = 0 ; sensor < s_numSensors ; sensor++)
    {
        getSensorLocation(sensor,location);

        outFile<<sensor<<"\t"<<s_ParetoSensorType[sensor]<<"\t"<<location[0]<<"\t"<<location[1]<<"\t"<<location[2]<<endl;
    }
}
/**********************************************************************/
void ParetoSensor::loadSensorsFromFile(string fileName)
{
    fstream inFile;
    inFile.open(fileName, ios::in);
    int v1,v2;
    PARETOSENSOR_TYPE sType;
    double pt[3];
    inFile>>v1;
    string line;
    int numSensors = v1;
    cout<<" Num sensors -: "<<numSensors<<endl;
    int numLinesRead = 0;
    while(!inFile.eof())
    {
        if(numLinesRead > numSensors)
            return;
        getline(inFile,line);
        istringstream iss(line);
        iss>>v1>>v2>>pt[0]>>pt[1]>>pt[2];
        if(numLinesRead == 0)
        {
            numLinesRead++;
            continue; // the first line has just the numsensors info so dont process
        }

        cout<<" sensor : "<<v1<<" type "<<v2<<" location -: "<<pt[0]<<";"<<pt[1]<<";"<<pt[2]<<endl;
        sType =  static_cast<PARETOSENSOR_TYPE>(v2);
        initializeParetoSensor(v1,sType);
        setParetoSensorLocation(v1,pt);

    }

}
/**********************************************************************/
int ParetoSensor::getSensorType(int sensorNumber)
{
    int sensorType = s_ParetoSensorType[sensorNumber];
    return sensorType;
}
/**********************************************************************/
int ParetoSensor::getNumberOfSensorsInitialized(void)
{
    return s_numSensors;
}
/**********************************************************************/
void ParetoSensor::saveSensorDataToFile(std::string fileName)
{
    fstream outFile;
    outFile.open(fileName,ios::out);
    for(int sens = 0 ; sens < s_numSensors ; sens++)
    {
        outFile<<endl;
        for(int i = 0 ; i < s_data[sens].size() ; i++)
        {
            outFile<<s_data[sens].at(i)<<"\t";
        }
    }
}

/**********************************************************************/
