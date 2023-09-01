/**
 * 
 */

#include "myHAS_DisplaySegments.h"
#include "myHAS_DisplayScreen.h"
#include "myHAS_SoundDriver.h"
#include "myHAS_Alarm.h"
#include "myHAS_Environment.h"
#include "../myHAS_Library/myHAS_SQLClient.h"

#include <iostream>
#include <string>
#include <fstream>

#include <unistd.h> 

#define _ID_ 90010
//#define _DEBUG_

using namespace std;

enum MODE {m_OFF, m_PLAYER, m_MAN_ALARM, m_AUTO_ALARM};

int main()
{	
	MODE mode = m_OFF;
	
	myHAS_Environment * pEnv = new myHAS_Environment();

	myHAS_Displays *pDisp = NULL;
#ifdef _DEBUG_
	myHAS_DisplayScreen *pDispScreen = new myHAS_DisplayScreen();
	pDisp = (myHAS_Displays *)pDispScreen;
#else
	myHAS_DisplaySegments *pDispSeg = new myHAS_DisplaySegments();
	pDisp = (myHAS_Displays *)pDispSeg;
	//used in dev mode to use keyboard inputs instead of GPIO inputs
	//iniscr allows getchar() to return for each pressed character and not wait for pressing enter
    initscr();
#endif
	pDisp->startDisplay();
	
	myHAS_SQLClient * pSQLClient = new myHAS_SQLClient();
	
	myHAS_SoundDriver* pSound = new myHAS_SoundDriver(pSQLClient);
		
	myHAS_Alarm *pAlarm = new myHAS_Alarm(_ID_, pDisp, pSound, pEnv, pSQLClient);
	pAlarm->setManualAlarmTime((int)(getCurrentTimeSec()/60)+1);
	
	bool showTime = true;

	//Main loop
	char key;
	while((key=getchar()) !=EOF)
	{
		if (key=='x' || key=='q')
			break;
		else if (key=='a') 
		{
			showTime=false;
			pDisp->displayAlarm(true);
		}
		else if (key==' ')
		{
			//Add that in other state, pressing snooze will display temperature
			if (pAlarm->getAlarmState()==as_ON)
				pAlarm->snoozeAlarm();
		}
		else if (!showTime && key=='m')
		{
			//Make sure to not increase hour when minutes reach 60
			long alarmTime = pAlarm->getManualAlarmTime();
			int alarmHour = alarmTime/60;
			int alarmMin = (alarmTime%60);
			alarmMin=(alarmMin+1)%60;
			alarmTime=alarmHour*60 + alarmMin;
			pAlarm->setManualAlarmTime(alarmTime);
			
		}
		else if (!showTime && key=='h')
			pAlarm->setManualAlarmTime((pAlarm->getManualAlarmTime()+60)%(24*60));
		//OFF mode
		else if (key=='o')
		{
			mode = m_OFF;
			pAlarm->stopAlarm();
			pSound->stopRadio();
			pAlarm->setAlarmMode(am_OFF);
			pDisp->displayEqualizer(false);
		}
		//radio mode
		else if(key=='i')
		{
			mode = m_PLAYER;
			pAlarm->setAlarmMode(am_OFF);
			pSound->playRadio();
			pDisp->displayEqualizer(true);
		}
		//manual alarm mode
		else if(key=='u')
		{
			mode = m_MAN_ALARM;
			pSound->stopRadio();
			pAlarm->setAlarmMode(am_MANUAL);
			pDisp->displayEqualizer(false);
		}
		//auto alarm mode
		else if(key=='y')
		{
			mode = m_AUTO_ALARM;
			pSound->stopRadio();
			pAlarm->setAlarmMode(am_AUTO);
			pDisp->displayEqualizer(false);
		}
		//next radio
		else if(key=='n')
		{
			if (mode == m_PLAYER)
				pSound->changeRadio(1);
		}
		//prev radio
		else if(key=='b')
		{
			if (mode == m_PLAYER)
				pSound->changeRadio(-1);
		}
		else
		{
			if(mode == m_PLAYER)
				pDisp->displayEqualizer(true);
			else
			{
				pDisp->displayEqualizer(false);
				pDisp->displayAlarm(false);
			}
			showTime = true;
		}
	}
	pDisp->stopDisplay();
	pAlarm->stopAlarmLoop();
	pSound->stopRadio();
	delete pSQLClient;
	delete pEnv;
#ifndef _DEBUG_
	//to be removed when GPIOs will be used instead of keyboard
	endwin();
#endif

	return 0;
}
