#include "myHAS_Displays.h"
#include <wiringPi.h>
#include <cmath>
#include <time.h>
#include <unistd.h> 
#include "../myHAS_Library/myHAS_Utilities.h"

void myHAS_Displays::stopDisplay()
{
    keepRunning = false;
	
    //Kill the threads
    dispThread.join();
}

void myHAS_Displays::startDisplay()
{
    keepRunning = true;
    dispThread = thread(&myHAS_Displays::runDisplay, this);
}

void myHAS_Displays::startTemporaryDisplay(string iText, int iDuration)
{
    temporaryText = iText;
    tempTextExpires = getUnixTime() + iDuration;
}
