#include "myHAS_DisplaySegments.h"
#include <wiringPi.h>
#include <cmath>
#include <time.h>
#include <unistd.h> 
#include "../myHAS_Library/myHAS_Utilities.h"

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
    //refresh rate of the equalizer
    int loopCount = 0;

    while(keepRunning)
    {
	if(difftime(getUnixTime(),tempTextExpires)<=0)
	{
	    digit1 = 0;
	    digit2 = getCharValue(temporaryText[0]);
	    digit3 = getCharValue(temporaryText[1]);
	    digit4 = getCharValue(temporaryText[2]);
	    semiColon = false;
	}
	else switch (dispMode)
	{
	    case dm_ALARM:
		semiColon = true;
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
		break;
	    case dm_EQUALIZER:
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
		break;
	    
	    case dm_BLUETOOTH:
		digit1 = 0;
		digit2 = 14;
		digit3 = 15;
		digit4 = 18;
		
		semiColon = false;
		break;
	    
	    case dm_TIME:
		time_t rawtime;
		struct tm *timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);
		digit1 = (int)(timeinfo->tm_hour /10);
		digit2 = timeinfo->tm_hour %10;
		digit3 = (int)(timeinfo->tm_min /10);
		digit4= timeinfo->tm_min %10;
		
		//To manage blinking the dots when there is an active alarm
		if(rawtime%2 && alrmMode && alarmTime>=0)
		    semiColon = false;
		else
		    semiColon = true;
		break;
		
	    default:
		//Err
		digit1 = 0;
		digit2 = 16;
		digit3 = 17;
		digit4 = 17;
		semiColon = false;
		break;
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
    //0~9 = digits, 10="-", 11 = "_". 12 = "=", 13 = "///", 14="b", 15="t", 16="E", 17="r", 18 = " ", 
    // 19 = "L", 20="i", 21="n", 22="J", 23="o", 24="u", 25="C", 26="A", 27="d", 28"H", 29"h"
    int maskTable [30] = {123,96,94,118,101,55,63,112,127,119,4,2,6,22,47,15,23,12,0,11,32,44,98,46,42,27,125,110,109,45};
    //0=OFF, 1="1" 2="2", rest is off
    int maskTableDig1 [30] = {0,3,70,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

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
	
int myHAS_DisplaySegments::getCharValue(char iChar)
{
    switch (iChar)
    {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case '-': return 10;
	case '_': return 11;
	case '=': return 12;
	case 'b': return 14;
	case 't': return 15;
	case 'E': return 16;
	case 'r': return 17;
	case ' ': return 18;
	case 'L': return 19;
	case 'i': return 20;
	case 'n': return 21;
	case 'J': return 22;
	case 'o': return 23;
	case 'u': return 24;
	case 'C': return 25;
	case 'A': return 26;
	case 'd': return 27;
	case 'H': return 28;
	case 'h': return 29;
	default: return 18;
    }
}
