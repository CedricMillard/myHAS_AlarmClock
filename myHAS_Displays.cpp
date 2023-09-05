#include "myHAS_DisplaySegments.h"
#include <wiringPi.h>
#include <cmath>
#include <time.h>
#include <unistd.h> 

void myHAS_Displays::displayAlarm(bool idispAlarm)
{
	dispAlarm = idispAlarm;
	//if(idispAlarm) dispEqualizer = false;
}

void myHAS_Displays::displayEqualizer(bool idispEq)
{
	dispEqualizer = idispEq;
	//if(idispEq) dispAlarm = false;
}

void myHAS_Displays::stopDisplay()
{
    keepRunning = false;
	
    //Kill the thread
    dispThread.join();
}

void myHAS_Displays::startDisplay()
{
    keepRunning = true;
    dispThread = thread(&myHAS_Displays::runDisplay, this);
}
