#include "myHAS_DisplaySegments.h"
#include <wiringPi.h>
#include <cmath>
#include <time.h>
#include <unistd.h> 

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
