#ifndef ParetoSensor_H
#define ParetoSensor_H
#include <vector>
#include "Timer.h"
#include "NonLinearThermal.h"


#define MAX_PARETOSENSORS 100

using namespace std;

// commment if the library is not available
//#define CHARTLIB
enum PARETOSENSOR_TYPE
{
	U_DISPLACMENT,
	V_DISPLACMENT,
	W_DISPLACMENT,
	TOTAL_DISPLACEMENT,
	TEMPERATURE,
	SXX_STRESS,
	SYY_STRESS,
	SZZ_STRESS,
	SXY_STRESS,
	SXZ_STRESS,
	SYZ_STRESS,
    VONMISES_STRESS
};

class ParetoSensor: public NonLinearThermal {


private:
	PARETOSENSOR_TYPE	s_ParetoSensorType[MAX_PARETOSENSORS];
	std::vector<double>	s_data[MAX_PARETOSENSORS];// sensor data (temperature, deformation, stress, ...) dependoing on type
	std::vector<double>	s_time[MAX_PARETOSENSORS];//the time at which the data was captured (default: 0,1,2,...)
	double				s_ParetoSensorDefaultValue[MAX_PARETOSENSORS];// in case the sensor values cannot read (missing mesh element)
	double				s_ParetoSensorLocation[MAX_PARETOSENSORS][3];// where to sense ... this can change dynamically
	char				s_xLabel[MAX_PARETOSENSORS][256];//for plotting
	char				s_yLabel[MAX_PARETOSENSORS][256];//for plotting
	char				s_title[MAX_PARETOSENSORS][256];//for plotting
    int                 s_numSensors; // keep track as new sensors are init
protected:
	
public:
			ParetoSensor(void);// 

		void initializeParetoSensor(int ParetoSensorNumber,PARETOSENSOR_TYPE ParetoSensorType,double defaultValue = 0, const char *xLabel="",const char *yLabel="",const char *title="");//what type of ParetoSensor and where, with pointer to pareto
		void setParetoSensorDefaultValue(int ParetoSensorNumber,double defaultValue);
		void setParetoSensorLocation(int ParetoSensorNumber,double pt[3]);
		bool sampleParetoSensorNumber(int ParetoSensorNumber,double time = 0);//optinally pass the time of sampling
		void plotParetoSensorData(int ParetoSensorNumber);
		void scaleData(int ParetoSensorNumber,double scale,double offset);
        void getSensorData(int ParetoSensorNumber, std::vector<double> &sensorData, std::vector<double> &sensorTime); // Function to get sensor value
        double getSensorDataAtInstance(int sensorNumber,int instance);
        void    getSensorLocation(int sensorNumber, double location[3]);
        int     getSensorType(int sensorNumber);
        int     getNumTimeStepsOfSensor(int sensorNumber);
        void print(int ParetoSensorNumber);
        void saveSensorsToFile(std::string fileName);
        void loadSensorsFromFile(std::string fileName);
        int    getNumberOfSensorsInitialized();
        void saveSensorDataToFile(std::string fileName);
			
			~ParetoSensor();
};

#endif
