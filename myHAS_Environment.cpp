#include "myHAS_Environment.h"
#include <iostream>
#include <string>
#include <time.h>
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

myHAS_Environment::myHAS_Environment(const char * iMQTTClientID)
{
  start_MQTT(iMQTTClientID);
}

void myHAS_Environment::setTopics()
{
  addTopic("/sensor/#");
	addTopic("/weather/daily");
	addTopic("/weather/hourly");
}

void myHAS_Environment::mqtt_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	string topic = msg->topic;
  string sPayload = (char*)msg->payload;
  
  //cout<<topic<<" : "<<sPayload<<endl; 

  if(topic.find("/sensor/")!=string::npos)
  {
    //retrieve ID of the prise
    string sID = topic.substr(topic.find("/sensor/")+8, topic.find("/", topic.find("/sensor/")+8));
    short iID = stoi(sID);
    if(topic.find("/value")!=string::npos)
    {
      if(sPayload.length()>0)
        setSensorValue(iID, stof(sPayload));
      else
        setSensorValue(iID, -255);
    }
    if(topic.find("/type")!=string::npos)
    {
      if(!sensorUnits.exists(iID))
      {
        switch(stoi(sPayload))
        {
          case 0:sensorUnits.add("*C", iID);break;
          case 2:sensorUnits.add("V", iID);break;
          case 3:sensorUnits.add("*C", iID);break;
          default: sensorUnits.add("", iID);break;
        }
      }
    }
  }

  if(topic=="/weather/daily")
  {
    setWeatherDaily(sPayload);
  }

  if(topic=="/weather/hourly")
  {
    setWeatherHourly(sPayload);
  }
}

float myHAS_Environment::getTemperatureExtFromWeather()
{
  return getWeatherHour(time(nullptr)).Tmax;
}

float myHAS_Environment::getSensorValue(int iSensorId)
{
  if(sensorValues.exists(iSensorId))
    return (float) sensorValues.getItem(iSensorId);
  else
    return -255;
}

void myHAS_Environment::setSensorValue(int iSensorId, float iValue)
{
  if(!sensorValues.exists(iSensorId))
  {
    sensorValues.add(iValue, iSensorId);
  }
  else sensorValues.getItem(iSensorId) = iValue;
}

void myHAS_Environment::setWeatherDaily(string iWeatherJson)
{
    //Decode json
	Document weatherJsonDoc;
	weatherJsonDoc.Parse(iWeatherJson.c_str());
    Value::ConstValueIterator itr; 
    int i = 0;
    for(itr = weatherJsonDoc["daily"].Begin(); itr!=weatherJsonDoc["daily"].End(); ++itr)
    {
        weather_d[i].Weather = (short) itr->GetObject()["weather"].GetInt();
        weather_d[i].Tmax = (float)itr->GetObject()["Tmax"].GetFloat();
        weather_d[i].Tmin = (float)itr->GetObject()["Tmin"].GetFloat();
        weather_d[i].Wind = (float)itr->GetObject()["wind"].GetFloat();
        weather_d[i].T6 = (float)itr->GetObject()["T6"].GetFloat();
        weather_d[i].updateTime = (long)itr->GetObject()["time"].GetInt64();
        i++;
        if(i>4) break;
    }
}

void myHAS_Environment::setWeatherHourly(string iWeatherJson)
{
    Document weatherJsonDoc;
	weatherJsonDoc.Parse(iWeatherJson.c_str());
    Value::ConstValueIterator itr; 
    int i = 0;
    for(itr = weatherJsonDoc["hourly"].Begin(); itr!=weatherJsonDoc["hourly"].End(); ++itr)
    {
        weather_h[i].Weather = (short) itr->GetObject()["weather"].GetInt();
        weather_h[i].Tmax = weather_h[i].Tmin = (float)itr->GetObject()["Temp"].GetFloat();
        weather_h[i].Wind = (float)itr->GetObject()["wind"].GetFloat();
        weather_h[i].updateTime = (long)itr->GetObject()["time"].GetInt64();
        i++;
        if(i>23) break;
    }
}

string myHAS_Environment::getSensorUnit(int iSensorId)
{
  if(sensorUnits.exists(iSensorId))
    return sensorUnits.getItem(iSensorId);
  else
    return "";
}

float myHAS_Environment::getTemperatureMorning()
{
  return getTodayWeather().Tmin;
}

Weather myHAS_Environment::getTodayWeather()
{
  return getWeatherDay(0);
}

Weather myHAS_Environment::getTomorrowWeather()
{
  return getWeatherDay(1);
}

Weather myHAS_Environment::getWeatherDay(int day)
{
  Weather emptyWeather;
  if (day>=4 || weather_d[0].updateTime == 0)
  {
    return emptyWeather;
  }

  struct tm updatedDay = *localtime(&(weather_d[0].updateTime));
  updatedDay.tm_hour = 0; updatedDay.tm_min = 0; updatedDay.tm_sec = 0;

  long delta = difftime(time(nullptr), mktime(&updatedDay));

  //Weather updated today
  if (delta < 86400) return weather_d[day];
  //Weather was updated yesterday
  if (delta < 172800 && day<3) return weather_d[day+1];
  if (delta < 259200 && day<2) return weather_d[day+2];
  if (delta < 345600 && day<1) return weather_d[day+3];
  return emptyWeather;
}

Weather myHAS_Environment::getWeatherHour(long hour)
{
  Weather emptyWeather;
  
  //Compute time index
  int index = (hour - weather_h[0].updateTime)/3600;

  if (index<0 || index > 13)
  {
    return emptyWeather;
  }

  return  weather_h[index];
}


bool operator==(const Weather& lhs, const Weather& rhs)
{
    if(lhs.Tmax!=rhs.Tmax) return false;
    if(lhs.Tmin!=rhs.Tmin) return false;
    if(lhs.T6!=rhs.T6) return false;
    if(lhs.Weather!=rhs.Weather) return false;
    if(lhs.Wind!=rhs.Wind) return false;
    
    return true;
}

bool operator!=(const Weather& lhs, const Weather& rhs)
{
    return !(lhs==rhs);
}
