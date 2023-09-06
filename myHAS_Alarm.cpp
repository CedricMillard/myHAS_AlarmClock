/**
 * TODO: 
 *  - Split WakeUpMsg Preparation from playing it to stop it in case alarm is stopped before it is ready...
 *  - Move weather text to config file to manage several language
 * 
 */

#include "myHAS_Alarm.h"
#include <time.h>
#include <unistd.h> 
#include <stdio.h>
#include <cmath>
#include <iostream>
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/stringbuffer.h"
#include "../include/rapidjson/writer.h"

using namespace rapidjson;
using namespace std;

long getCurrentTimeSec()
{
	time_t rawtime;
	struct tm *timeinfo;
	char sTime[10];
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	long currentSec = timeinfo->tm_sec + 60*timeinfo->tm_min + 3600*timeinfo->tm_hour;
	return currentSec;
}

uint8_t getDay()
{
    uint8_t day;
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char sTime[2];
    strftime(sTime, 2, "%w", timeinfo);
    day = atoi(sTime);
    return day;
}

myHAS_Alarm::myHAS_Alarm(int iID, myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Environment *iEnv, myHAS_SQLClient *iSQLClient)
{
    ID = iID;
    pDisp = iDisp; 
    pSound = iSound; 
    pEnv = iEnv;
    pSQLClient = iSQLClient;
    
    importParameters();
}
void myHAS_Alarm::importParameters()
{
    snoozeTime = pSQLClient->getIntValue("AlarmClock", ID, "SnoozeTime");
    voiceName = pSQLClient->getStringValue("TTSVoices",pSQLClient->getIntValue("AlarmClock", ID, "VoiceId"),"VoiceName");
    
    listRules.removeAll();
    nextAlarmTime= 99999;
    pDisp->setAlarmTime(-1);
    
    setManualAlarmTime(pSQLClient->getIntValue("AlarmClock", ID, "ManualAlarmTime"));
        
    string alarmRules = pSQLClient->getStringValue("AlarmClock", ID, "Rules");
    
    //Replace by reading from dB (need a dB class)
    Document rulesJsonDoc;
    rulesJsonDoc.Parse(alarmRules.c_str());
    Value::ConstValueIterator itr; 
    int i = 0;
    for(itr = rulesJsonDoc["rules"].Begin(); itr!=rulesJsonDoc["rules"].End(); ++itr)
    {
        Rule newRule;
        newRule.frequency = (uint8_t) itr->GetObject()["freq"].GetInt();
        newRule.alarmTime = itr->GetObject()["time"].GetInt();
        newRule.active = itr->GetObject()["active"].GetBool();
        
        newRule.alarmSound = pSQLClient->getStringValue("RadioStation", itr->GetObject()["alarm"].GetInt(), "URI");
        
        listRules.add(newRule);
    }
}

string myHAS_Alarm::getWakeUptext()
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	string request = pSQLClient->getStringValue("AlarmClock", ID, "WakeUpPhrase");
    string sTime = to_string(timeinfo->tm_hour) + " heures ";
    if(timeinfo->tm_min>0) 
        sTime += to_string(timeinfo->tm_min);
	
    string sWeather = "";
    switch(pEnv->getTodayWeather().Weather)
	{
	    case W_SUN:
		sWeather+="le temps est ensoleillé";
		break;
	    case W_PARTCLOUD:
		sWeather+="le temps est nuageux";
		break;
	    case W_CLOUD:
		sWeather+="le temps est couvert";
		break;
	    case W_RAIN:
		sWeather+="le temps est pluvieux";
		break;
	    case W_SLEET:
		sWeather+="attention au risque de pluie verglaçante";
		break;
	    case W_SNOW:
		sWeather+="le temps est neigeux";
		break;
	    case W_WIND:
		sWeather+="le temps est venteux";
		break;
	    case W_FOG:
		sWeather+="le temps est brumeux";
		break;
	    case W_THUNDER:
		sWeather+="le temps est orageux";
		break;
	}
    
    int index=-1;
    if((index=request.find("#TIME#"))!=string::npos)
        request.replace(index, 6, sTime);
    
    index=-1;
    if((index=request.find("#WEATHER#"))!=string::npos)
        request.replace(index, 9, sWeather);
        
    index=-1;
    if((index=request.find("#SENSOR_"))!=string::npos)
    {
        string sensorID = request.substr(index+8, 5);
        string sensorValue = to_string((int)pEnv->getSensorValue(stoi(sensorID)));
        request.replace(index, 14, sensorValue);
    }
	
	return request;
}

void myHAS_Alarm::startAlarmLoop()
{
    stopAlarmLoop();
	keepRunning = true;
    alarmThread = new thread(&myHAS_Alarm::alarmLoop, this);
}

void myHAS_Alarm::stopAlarmLoop()
{
	keepRunning = false;
	
    //Kill the thread
    if(alarmThread)
    {
        alarmThread->join();
        delete alarmThread;
        alarmThread = NULL;
    }
}

void myHAS_Alarm::alarmLoop()
{
	//bool a=true;
    
    while(keepRunning)
	{
        long timeNow = getCurrentTimeSec();
        switch(aMode)
        {
            case am_OFF:
                break;
            
            case am_MANUAL:
                //Start Alarm
                if((timeNow - alarmTime) >= 0 && (timeNow - alarmTime) <= 2 && aState!=as_ON)
                {
                    aState = as_ON;
                    ringAlarm();
                }
                break;
            
            case am_AUTO:
                for(int i=0; i<listRules.size(); i++)
                {
                    //cout<<"Rule "<<i<<" active "<<(int)listRules[i].active<<endl;
                    if( listRules[i].active) 
                    {
                        computeNextAlarm(listRules[i], timeNow);
                        if(((uint8_t)(pow(2,getDay())+128) & listRules[i].frequency) /*||  listRules[i].frequency==128*/)
                        {
                            if((timeNow - listRules[i].alarmTime*60)<=2 && (timeNow - listRules[i].alarmTime*60)>=0)
                            {
                                if(aState!=as_ON)
                                {
                                    aState = as_ON;
                                    alarmTime = listRules[i].alarmTime*60;
                                    nextAlarmTime= 99999;
                                    pDisp->setAlarmTime(-1);
                                    currentAlarmSound = listRules[i].alarmSound;
                                    ringAlarm(currentAlarmSound);
                                }
                                if(listRules[i].frequency==128)
                                {
                                    listRules[i].active = false;
                                    //update rule in SQL dB
                                    updateRulesInDB();
                                }
                            }
                        }
                    }
                }
		
                //Start in case of snooze 
                if((timeNow - alarmTime) >= 0 && (timeNow - alarmTime) <= 2 && aState==as_SNOOZE)
                {
                    aState = as_ON;
                    ringAlarm(currentAlarmSound);
                }
                break;
                
            default:
                break;
        }
        /*if(a) cout<<"ALARM LOOP /"<<endl;
        else cout<<"ALARM LOOP \\"<<endl;
        a=!a;*/
        checkSQLUpdate();
        sleep(1);
	}
	
	stopAlarm();
}

void myHAS_Alarm::computeNextAlarm(Rule iRule, long icurrTime)
{
    int today = getDay();
    int tomorrow = getDay()+1;
    if(tomorrow>6) tomorrow = 0;
    bool tryMore = true;
    
    if(((uint8_t)(pow(2,today)+128) & iRule.frequency))
    {
        if(icurrTime < iRule.alarmTime*60 && iRule.alarmTime < nextAlarmTime)
        {
            nextAlarmTime= iRule.alarmTime;
            tryMore = false;
        }
    }
    if (tryMore && ((uint8_t)(pow(2,tomorrow)+128) & iRule.frequency))
    {
        if(icurrTime > iRule.alarmTime*60 && iRule.alarmTime+10000 < nextAlarmTime)
        {
            nextAlarmTime= iRule.alarmTime+10000;
            
        }
    }
    if(nextAlarmTime<99999)
        pDisp->setAlarmTime(nextAlarmTime%10000);
}

void myHAS_Alarm::stopAlarm() 
{
    if(aState != as_OFF)
    {
        pDisp->setAlarmStatus(0);
        pSound->stopRadio();
        aState = as_OFF;
    }
    switch (aMode)
    {
        case am_MANUAL:
            alarmTime = manualAlarmTime*60;
            break;
        
        case am_AUTO:
            alarmTime = -1;
            break;
        
        default:
            alarmTime = -1;
            break;
    }
}

void myHAS_Alarm::snoozeAlarm() 
{
    if(aState==as_ON)
    {
        stopAlarm();
        aState = as_SNOOZE;
        alarmTime = (getCurrentTimeSec() + snoozeTime*60)%(24*3600);
        if(pDisp)
            pDisp->setAlarmStatus(2);
    }
}

void myHAS_Alarm::setAlarmMode(alarmMode iMode)
{
    if (iMode!=aMode)
    {
        aMode = iMode;
        stopAlarm();
        if (iMode==am_AUTO)
        {
            nextAlarmTime= 99999;
            pDisp->setAlarmTime(-1);
            startAlarmLoop();
            return;
        }
        
        pDisp->setAlarmTime(manualAlarmTime);
        if (iMode==am_MANUAL)
            startAlarmLoop();
        else
            stopAlarmLoop();
    }
}

void myHAS_Alarm::setManualAlarmTime(long iTime)
{
    //Cannot set manual alarm time in auto mode, as it will display the next scheduled alarm
    if(aMode==am_AUTO)
	return;

    if(manualAlarmTime != iTime && iTime!=-1)
    {
        manualAlarmTime = iTime;
        alarmTime = manualAlarmTime*60;
    }
    
    if(pDisp)
        pDisp->setAlarmTime(manualAlarmTime);
}

void myHAS_Alarm::ringAlarm(string iAlarmSound)
{
    if(pDisp)
        pDisp->setAlarmStatus(1);

    if(pSound)
    {
        pSound->readText(getWakeUptext(), voiceName);
	
        //Allow to not start radio if snooze or off is pressed during wake-up phrase
        if(aState==as_ON)
            pSound->playRadio(iAlarmSound);
    }
}

void myHAS_Alarm::checkSQLUpdate()
{
    //file exists in folder, means sql table has changed
    if(!access(sqlFileCheck.c_str(),F_OK))
    {
        remove(sqlFileCheck.c_str());
        importParameters();        
    }
}

void myHAS_Alarm::updateRulesInDB()
{
    //Create json from list rules
    Document newJson;
    newJson.SetObject();
    Value arrayRules(kArrayType);
    Document::AllocatorType &allocator = newJson.GetAllocator();
    for (int i=0;i<listRules.size();i++)
    {
        Value rule(kObjectType);
        rule.AddMember("freq",listRules[i].frequency,allocator);
        rule.AddMember("time",listRules[i].alarmTime,allocator);
        rule.AddMember("active",listRules[i].active,allocator);
        rule.AddMember("alarm", pSQLClient->getIntValue("RadioStation", "URI", listRules[i].alarmSound, "Id"),allocator);
        
        arrayRules.PushBack(rule, allocator);
    }
    newJson.AddMember("rules",arrayRules,allocator);
    
    StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	newJson.Accept(writer);

    //update attribute in dB
   	pSQLClient->updateObject("AlarmClock", ID, "Rules", strbuf.GetString());
}

void myHAS_Alarm::saveAlarmTime()
{
    if(aMode==am_AUTO)
        return;
    
    pSQLClient->updateObject("AlarmClock", ID, "ManualAlarmTime", to_string(manualAlarmTime));
}
