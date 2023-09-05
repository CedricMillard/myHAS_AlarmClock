#include "myHAS_DisplaySegments.h"
#include <wiringPi.h>
#include <cmath>
#include <time.h>
#include <unistd.h> 

myHAS_DisplaySegments::myHAS_DisplaySegments()
{
    //Initialize GPIOs for display
    wiringPiSetup();

    for (int i=0; i<4;i++)
    {
	pinMode(gpioTable4[i], OUTPUT);
	pinMode(gpioTable3[i], OUTPUT);
	pinMode(gpioTable2[i], OUTPUT);
	pinMode(gpioTable1[i], OUTPUT);
    }
    pinMode(catGPIO[0], OUTPUT);
    pinMode(catGPIO[1], OUTPUT);
}

//main loop to be run in a separate thread
void myHAS_DisplaySegments::runDisplay()
{
    bool cat = true;
    int digit1 = 0;
    int digit2 = 0;
    int digit3 = 0;
    int digit4 = 0;
    bool semiColon = true;
    time_t lastTime = 0;
    int loopCount = 0;

    while(keepRunning)
    {
	semiColon = true;
	if(dispAlarm)
	{
	    if(alarmTime>=0)
	    {
		long bufAl = alarmTime;
		int alarmHour = bufAl/60;
		bufAl = bufAl%60;
		int alarmMin = bufAl;
		
		digit1 = (int)(alarmHour/10);
		digit2 = alarmHour%10;
		digit3 = (int)(alarmMin/10);
		digit4 = alarmMin%10;
	    }
	    else
	    {
		//10 is a dash
		digit1 = 0;
		digit2 = 10;
		digit3 = 10;
		digit4 = 10;
	    }
	}
	else if(dispEqualizer)
	{
	    //To directly display graph rather than time until loopCount reaches 0
	    if(digit2<11)
	    {
		digit2 = 11 + rand()%3;
		digit3 = 11 + rand()%3;
		digit4 = 11 + rand()%3;
	    }
		
	    if(loopCount==0)
	    {
		digit2 = 11 + rand()%3;
		digit3 = 11 + rand()%3;
		digit4 = 11 + rand()%3;
	    }
	    digit1 = 0;
	    semiColon = false;
	}
	else
	{
	    time_t rawtime;
	    struct tm *timeinfo;

	    time(&rawtime);
	    timeinfo = localtime(&rawtime);
	    digit1 = (int)(timeinfo->tm_hour /10);
	    digit2 = timeinfo->tm_hour %10;
	    digit3 = (int)(timeinfo->tm_min /10);
	    digit4= timeinfo->tm_min %10;
	}						
	displaySegments(1, digit1, cat, semiColon);
	displaySegments(2, digit2, cat, semiColon);
	displaySegments(3, digit3, cat, semiColon);
	displaySegments(4, digit4, cat, semiColon);
	usleep(10000);
	loopCount = (loopCount+1)%40;
	cat = !cat;
    }
}

void myHAS_DisplaySegments::stopDisplay()
{
    myHAS_Displays::stopDisplay();
	
    //turn off the segments
    for (int i=0;i<4;i++)
    {
	digitalWrite (gpioTable4[i], LOW);
	digitalWrite (gpioTable3[i], LOW);
	digitalWrite (gpioTable2[i], LOW);
	digitalWrite (gpioTable1[i], LOW);
    }
    digitalWrite (catGPIO[0], LOW);
    digitalWrite (catGPIO[1], LOW);
}

/*  Display a number on the specified digit
 *  iDigit: digit to display
 *  iNumber = number to display
 *  cat = which cathode is active
*/
void myHAS_DisplaySegments::displaySegments (int iDigit, int iNumber, bool cat, bool iSemiColon)
{
    //0~9 = digits, 10="-", 11 = "_". 12 = "=", 13 = "///"  
    int maskTable [14] = {123,96,94,118,101,55,63,112,127,119,4,2,6,22};
    int maskTableDig1 [14] = {0,3,70,0,0,0,0,0,0,0,0,0,0,0};

    int mask = maskTable[iNumber];
    int maskDig1 = maskTableDig1[iNumber];

    if((iDigit%2==1 && cat) || (iDigit%2==0 && !cat)) mask = mask>>4;
    
    digitalWrite (catGPIO[0], cat);
    digitalWrite (catGPIO[1], !cat);
    
    for (int i=0;i<4;i++)
    {
	if (iDigit==3) digitalWrite (gpioTable3[i], mask & int(pow(2,i)));
	else if (iDigit==1) digitalWrite (gpioTable1[i], maskDig1 & int(pow(2,i)));
	else if (iDigit==4) 
	{
	    //Prevents to change the segment common with digit 3
	    if(!(!cat && i==3))
	    {
		digitalWrite (gpioTable4[i], mask & int(pow(2,i)));
	    }
	}
	else if (iDigit==2) 
	{
	    //Prevents to change the segment common with digit 3
	    if(!(!cat && i==3))
	    {
		digitalWrite (gpioTable2[i], mask & int(pow(2,i)));
	    }
	}
    }
    //semicolon
    if(iSemiColon)
	digitalWrite (gpioTable1[3], 1);
    else
	digitalWrite (gpioTable1[3], 0);
}
	
