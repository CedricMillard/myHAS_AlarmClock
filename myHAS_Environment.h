#ifndef myHAS_Environment_h
#define myHAS_Environment_h

#define W_SUN 1
#define W_PARTCLOUD 2
#define W_CLOUD 3
#define W_RAIN 4
#define W_SLEET 5
#define W_SNOW 6
#define W_WIND 7
#define W_FOG 8
#define W_THUNDER 9

#include <string>
#include "../include/ArrayCed.h"
#include "../myHAS_Library/myHAS_MQTTClient.h"

using namespace std;

struct Weather
{
	float Tmax = -255;
	float Tmin = -255;
	float T6 = -255;
	short Weather = 0;
	float Wind = -255;
	time_t updateTime = 0;
};

bool operator==(const Weather& lhs, const Weather& rhs);

bool operator!=(const Weather& lhs, const Weather& rhs);

class myHAS_Environment : public myHAS_MQTTClient
{
	public:
		//Allow to specify a different client ID
		myHAS_Environment(const char * iMQTTClientID = "myHAS_AC_Env");
		float getTemperatureExtFromWeather();
		float getSensorValue(int iSensorId);
		void setSensorValue(int iSensorId, float iValue);
		void setWeatherDaily(string iWeatherJson);
		void setWeatherHourly(string iWeatherJson);
		string getSensorUnit(int iSensorId);
		float getTemperatureMorning();
		Weather getTodayWeather();
		Weather getTomorrowWeather();
		Weather getWeatherDay(int day);
		Weather getWeatherHour(long hour);
	
	protected:
		void setTopics();
		void mqtt_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
	
	private:
		float TemperatureExt = 100;
		Weather weather_d[5];
		Weather weather_h[24];
		Array<float> sensorValues;
		Array<string> sensorUnits;
};

#endif
