#include "myHAS_DisplayScreen.h"
#include <time.h>
#include <unistd.h> 
#include <iostream>
#include <string.h>

myHAS_DisplayScreen::myHAS_DisplayScreen()
{
    int ch;

    win = initscr();
    if(win == NULL)
    {
        cout<<"Error creating window"<<endl;
        return;
    }
    
    curs_set(0);
    mvwaddstr(win, 0, 0, "CLOCK");
    mvwaddstr(win, 0, 6, "ALARM OFF");
}

//main loop to be run in a separate thread
void myHAS_DisplayScreen::runDisplay()
{
    char prevTime[19]="      ";
    while(keepRunning)
    {
        time_t rawtime;
        struct tm *timeinfo;
        char sTime[10];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if (dispMode==dm_TIME)
            strftime(sTime, 10, "%H%M%S",timeinfo);
        else
        {
	    int alarmHour = 88;
	    int alarmMin = 88;
	    int alarmSec = 88;
		
	    if(alarmTime>=0)
	    {
		long bufAl = alarmTime;
		alarmHour = bufAl/60;
		bufAl = bufAl%60;
		alarmMin = bufAl;
		alarmSec = 0;
	    }

            sprintf(sTime, "%02d%02d%02d",alarmHour, alarmMin, alarmSec);
        }

        int yOffset = 1;
        //clear();
        printColon(2, 17);
        printColon(2, 36);	
        for (int i=0; i<6;i++)
        {
            if (i==2 || i==4)
                yOffset+=3;
                
            if(prevTime[i]!=sTime[i])
                printDigit(2, yOffset, int(sTime[i])-48);
            
            yOffset+=8;
        }
        move(11,0);
        strcpy(prevTime, sTime);
        refresh();
        usleep(100000);
    }
    wclear(win);
    delwin(win);
    endwin();
    refresh();
}
	
void myHAS_DisplayScreen::printColon(int x, int y)
{
	mvwaddstr(win, x+2, y, "O");
	mvwaddstr(win, x+4, y, "O");
}

void myHAS_DisplayScreen::printDigit(int x, int y, int iDigit)
{
	if (iDigit<0 || iDigit>9) return;
	
	uint8_t numbers[10];
	numbers[0] = 0b00111111;
	numbers[1] = 0b00000110; 
	numbers[2] = 0b01011011;
	numbers[3] = 0b01001111;
	numbers[4] = 0b01100110;
	numbers[5] = 0b01101101;
	numbers[6] = 0b01111101;
	numbers[7] = 0b00000111;
	numbers[8] = 0b01111111;
	numbers[9] = 0b01101111;
	
	//Clear digit
	for (int i=0;i<7;i++)
		mvwaddstr(win, x+i, y, "      ");
	
	if (numbers[iDigit] & 0b01) mvwaddstr(win, x, y+1, "####");
	if (numbers[iDigit]>>1 & 0b01) { mvwaddstr(win, x+1, y+5, "#"); mvwaddstr(win, x+2, y+5, "#"); }
	if (numbers[iDigit]>>2 & 0b01) { mvwaddstr(win, x+4, y+5, "#"); mvwaddstr(win, x+5, y+5, "#"); }
	if (numbers[iDigit]>>3 & 0b01) { mvwaddstr(win, x+6, y+1, "####");}
	if (numbers[iDigit]>>4 & 0b01) { mvwaddstr(win, x+4, y, "#"); mvwaddstr(win, x+5, y, "#"); }
	if (numbers[iDigit]>>5 & 0b01) { mvwaddstr(win, x+1, y, "#"); mvwaddstr(win, x+2, y, "#"); }
	if (numbers[iDigit]>>6 & 0b01) { mvwaddstr(win, x+3, y+1, "####");}
}

//status: 0 = OFF, 1 = ON, 2 = SNOOZE
void myHAS_DisplayScreen::setAlarmStatus(int iStatus)
{
    switch(iStatus)
    {
        case 0:	mvwaddstr(win, 0, 6, "ALARM OFF   ");break;
        case 1: mvwaddstr(win, 0, 6, "ALARM ON    ");break;
        case 2: mvwaddstr(win, 0, 6, "ALARM SNOOZE");break;
        default:mvwaddstr(win, 0, 6, "ALARM ERROR ");break;
    }
    refresh();
}

void myHAS_DisplayScreen::stopDisplay()
{
    myHAS_Displays::stopDisplay();
    endwin();
}
