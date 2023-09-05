#include "myHAS_InputKeyboard.h"
#include <ncurses.h>

myHAS_InputKeyboard::myHAS_InputKeyboard(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm):myHAS_Inputs(iDisp, iSound, iAlarm)
{
    initscr();
}

myHAS_InputKeyboard::~myHAS_InputKeyboard()
{
    endwin();
}

void myHAS_InputKeyboard::runInput()
{
    char key;
    bool isAlarmPressed = false;
    while((key=getchar()) !=EOF)
    {
        if (key=='x' || key=='q')
			break;
            
		else if (key=='a') 
		{
			//if alarm already pressed, means we want to unpress it
            if (isAlarmPressed)
                unPressAlarm();
            else
                pressAlarm();
                
            isAlarmPressed = !isAlarmPressed;
            
		}
		else if (key==' ')
		{
			pressSnooze();
            unPressSnooze();
		}
		else if (key=='m')
		{
			pressMin();
            unPressMin();
			
		}
		else if (key=='h')
        {
			pressHour();
            unPressHour();
        }
        else if (key=='s')
        {
			pressSleep();
            unPressSleep();
        }
        else if (key=='t')
        {
			pressTime();
            unPressTime();
        }
		//OFF mode
		else if (key=='o')
		{
            setMode(m_OFF);
		}
		//radio mode
		else if(key=='i')
		{
            setMode(m_PLAYER);
		}
		//manual alarm mode
		else if(key=='u')
		{
            setMode(m_MAN_ALARM);
		}
		//auto alarm mode
		else if(key=='y')
		{
			setMode(m_AUTO_ALARM);
		}
		//next radio
		else if(key=='n')
		{
            pressMin();
		}
		//prev radio
		else if(key=='b')
		{
            pressHour();
		}
    }
}
