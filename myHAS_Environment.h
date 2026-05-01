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
#include <unordered_map>

using namespace std;

struct Weather
{
	float Tmax = -255;
	float Tmin = -255;
	float T6 = -255;
	short WeatherCode = -1;
	short Icon = 0;
	float Wind = -255;
	int Humidity = -1;
	time_t updateTime = 0;
};

bool operator==(const Weather& lhs, const Weather& rhs);

bool operator!=(const Weather& lhs, const Weather& rhs);

struct WeatherInfo {
    string description;
    int icon;
};

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
		string getWeatherDescription(int iWeatherCode);
	
	protected:
		void setTopics();
		void mqtt_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
	
	private:
		float TemperatureExt = 100;
		Weather weather_d[5];
		Weather weather_h[24];
		Array<float> sensorValues;
		Array<string> sensorUnits;
		unordered_map<int, WeatherInfo> weatherMap = {
    { 0,  {"ensoleillé",               W_SUN} },
    { 1,  {"principalement ensoleillé",W_SUN} },
    { 2,  {"partiellement nuageux",    W_PARTCLOUD} },
    { 3,  {"nuageux",                  W_CLOUD} },
    { 5,  {"brumeux",                  W_FOG} },
    { 10, {"brumeux",                  W_FOG} },
    { 40, {"brouillard",               W_FOG} },
    { 41, {"brouillard",               W_FOG} },
    { 42, {"brouillard",               W_FOG} },
    { 43, {"brouillard",               W_FOG} },
    { 44, {"brouillard",               W_FOG} },
    { 45, {"brouillard",               W_FOG} },
    { 46, {"brouillard",               W_FOG} },
    { 47, {"brouillard",               W_FOG} },
    { 48, {"brouillard givrant",       W_FOG} },
    { 49, {"brouillard givrant",       W_FOG} },
    { 50, {"légère bruine",            W_CLOUD} },
    { 51, {"légère bruine",            W_CLOUD} },
    { 52, {"bruine",                   W_CLOUD} },
    { 53, {"bruine",                   W_CLOUD} },
    { 54, {"forte bruine",             W_RAIN} },
    { 55, {"forte bruine",             W_RAIN} },
    { 56, {"légère bruine verglaçante",W_SLEET} },
    { 57, {"bruine verglaçante",       W_SLEET} },
    { 58, {"mélange de bruine et de pluie légère",   W_RAIN} },
    { 59, {"mélange de bruine et de pluie", W_RAIN} },
    { 60, {"légère pluie",      W_RAIN} },
    { 61, {"légère pluie",      W_RAIN} },
    { 62, {"pluvieux",                 W_RAIN} },
    { 63, {"pluvieux",                 W_RAIN} },
    { 64, {"forte pluie",              W_RAIN} },
    { 65, {"forte pluie",              W_RAIN} },
    { 66, {"pluie verglaçante légère", W_SLEET} },
    { 67, {"pluie verglaçante",        W_SLEET} },
    { 68, {"légères précipitations de mélange pluie neige", W_SLEET} },
    { 69, {"mélange pluie neige", W_SLEET} },
    { 70, {"légères chutes de neige",       W_SNOW} },
    { 71, {"légères chutes de neige",       W_SNOW} },
    { 72, {"légères chutes de neige",       W_SNOW} },
    { 73, {"chutes de neige",                  W_SNOW} },
    { 74, {"fortes chutes de neige",              W_SNOW} },
    { 75, {"fortes chutes de neige",              W_SNOW} },
    { 76, {"chutes de poudrin de glace",         W_SNOW} },
    { 77, {"chutes de neige en grains",          W_SNOW} },
    { 80, {"légères averses",          W_RAIN} },
    { 81, {"averses",                  W_RAIN} },
    { 82, {"fortes averses",           W_RAIN} },
    { 83, {"légères averse de mélange pluie neige", W_SLEET} },
    { 84, {"averses de mélange pluie neige", W_SLEET} },
    { 85, {"légères averses de neige", W_SNOW} },
    { 86, {"averses de neige",         W_SNOW} },
    { 89, {"légères averses de grêle", W_SNOW} },
    { 90, {"averses de grêle",         W_SNOW} },
    { 95, {"Orageux",                  W_THUNDER} },
    { 96, {"légèrement orageux avec chute de grêle",  W_THUNDER} },
    { 99, {"orageux avec chute de grêle", W_THUNDER} }
	};
};

#endif
