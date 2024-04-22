/**
 * TODO: watchdog to allow reboot with key combination (as separate process to reboot even in case of code crash)
 */

#include "myHAS_DisplaySegments.h"
#include "myHAS_DisplayScreen.h"
#include "myHAS_SoundDriver.h"
#include "myHAS_Alarm.h"
#include "myHAS_Environment.h"
#include "myHAS_InputKeyboard.h"
#include "myHAS_InputButtons.h"
#include "../myHAS_Library/myHAS_SQLClient.h"
#include "../myHAS_Library/myHAS_Utilities.h"

#define _ID_ 90010
//#define _SCREEN_
//#define _KEYBOARD_

#ifndef _KEYBOARD_
myHAS_InputButtons* myHAS_InputButtons::pInputButtons = NULL;
#endif

int main()
{	
	myHAS_Environment * pEnv = new myHAS_Environment();

	myHAS_Displays *pDisp = NULL;
#ifdef _SCREEN_
	myHAS_DisplayScreen *pDispScreen = new myHAS_DisplayScreen();
	pDisp = (myHAS_Displays *)pDispScreen;
#else
	myHAS_DisplaySegments *pDispSeg = new myHAS_DisplaySegments();
	pDisp = (myHAS_Displays *)pDispSeg;
#endif
	pDisp->startDisplay();
	
	myHAS_SQLClient * pSQLClient = new myHAS_SQLClient();
	
	myHAS_SoundDriver* pSound = new myHAS_SoundDriver(pSQLClient);
	pSound->startTokenUpdateLoop();
		
	myHAS_Alarm *pAlarm = new myHAS_Alarm(_ID_, pDisp, pSound, pEnv, pSQLClient);
	//pAlarm->setManualAlarmTime((int)(getCurrentTimeSec()/60)+1);


	myHAS_Inputs *pInput = NULL;
#ifdef _KEYBOARD_	
	myHAS_InputKeyboard *pInputKeyboard = new myHAS_InputKeyboard(pDisp, pSound, pAlarm);
	pInput = (myHAS_Inputs *)pInputKeyboard;
#else
	myHAS_InputButtons *pInpuButtons = myHAS_InputButtons::getInputButtons(pDisp, pSound, pAlarm);
	pInput = (myHAS_Inputs *)pInpuButtons;
#endif

	//Blocking loop
	pInput->runInput();

	pDisp->stopDisplay();
	pAlarm->stopAlarmLoop();
	pSound->stopRadio();
	pSound->stopTokenUpdateLoop();
	delete pSQLClient;
	delete pEnv;
	delete pAlarm;
	delete pSound;
	delete pDisp;
	delete pInput;

	return 0;
}
