
/**
 * TODO: try to implement interrupts to see if it reduces the load
 * see wiringPiISR but would need statuc function...
 * Idea would be to declare the callbacks as in Arduino in the main and use global variable to access the class member...
 * Worth a try in a separate code branch !
 * Or OK to run on 20% CPU load...
 */
#include "myHAS_InputButtons.h"
#include <unistd.h> 
#include <wiringPi.h>

myHAS_InputButtons::myHAS_InputButtons(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm):myHAS_Inputs(iDisp, iSound, iAlarm)
{
    wiringPiSetup();

    for (int i=0; i<6;i++)
    {
        pinMode(gpioButtons[i], INPUT);
        pullUpDnControl(gpioButtons[i], PUD_UP);
    }
    
    for (int i=0; i<4;i++)
    {
        pinMode(gpioSelectors[i], INPUT);
        pullUpDnControl(gpioSelectors[i], PUD_UP);
    }
}

myHAS_InputButtons::~myHAS_InputButtons()
{
    
}

void myHAS_InputButtons::readInputs(int &oButtons, int& oSelector)
{
    for (int i=0; i<6;i++)
    {
        oButtons = 2 * oButtons + digitalRead (gpioButtons[i]);
    } 
    
    for (int i=0; i<4;i++)
    {
        oSelector = 2 * oSelector + digitalRead (gpioSelectors[i]);
    } 
}

void myHAS_InputButtons::runInput()
{
    //Read all input states
    int currButtonState = 0;
    int currSelectorState = 0;
    int prevButtonState = 0;
    int prevSelectorState = 0;
    
    while(true)
    {
        sleep(0.25);
        //save previous button state
        prevButtonState = currButtonState;
        prevSelectorState = currSelectorState;
        
        currButtonState = 0;
        currSelectorState = 0;
        readInputs(currButtonState, currSelectorState);
        
        //Compute the difference with previous state
        int deltaB = currButtonState ^ prevButtonState;
        int deltaS = currSelectorState ^ prevSelectorState;
        
        //No change, skip
        if(!deltaB && !deltaS)
            continue;
        
        //A button has been pressed or released
        //order : Time, Hour, Min, Alarm, Sleep, Snooze
        if(deltaB)
        {
            //Stop loop if press on snooze + alarm + time    
            if (currButtonState==0b011010)
                break;
            
            //alarm button was changed
            if (deltaB & 0b000100) 
            {
                if (currButtonState & 0b000100)
                    unPressAlarm();
                else
                    pressAlarm();
            }
            //Snooze
            if (deltaB & 0b000001)
            {
                if(currButtonState & 0b000001)
                    unPressSnooze();
                else
                    pressSnooze();
            }
            //minutes
            if (deltaB & 0b001000)
            {
                if(currButtonState & 0b001000)
                    unPressMin();
                else
                    pressMin();
            }
            //hours
            if (deltaB & 0b010000)
            {
                if(currButtonState & 0b010000)
                    unPressHour();
                else
                    pressHour();
            }
            //time
            if (deltaB & 0b100000)
            {
                if(currButtonState & 0b100000)
                    unPressTime();
                else
                    pressTime();
            }
            //sleep
            if (deltaB & 0b000010)
            {
                if(currButtonState & 0b000010)
                    unPressSleep();
                else
                    pressSleep();
            }
        }
        
        //A selector has changed
        //order Radio, AutoAlarm, ManAlarm, Bluetooth
        int modeChanged = deltaS >> 1;
        if(modeChanged)
        {
            switch (currSelectorState>>1)
            {
                case 0b111:
                    setMode(m_OFF);
                    break;
                
                //radio mode
                case 0b011:
                    setMode(m_PLAYER);
                    break;
            
                //auto alarm mode
                case 0b101:
                    setMode(m_AUTO_ALARM);
                    break;
                
                //manual alarm mode
                case 0b110:
                    setMode(m_MAN_ALARM);
                    break;
                    
                default:
                    setMode(m_OFF);
                    break;
            }
        }
        int soundChanged = deltaS %2;
        if(soundChanged)
        {
            //If currSelectorState = 0 it means we have selected radio
            audioSourceRadio(!(currSelectorState%2));
        }
    }
}
