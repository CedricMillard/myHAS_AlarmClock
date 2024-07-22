/**
 * 
 */
#include "myHAS_InputButtons.h"
#include <unistd.h> 
#include <iostream> 
#include <wiringPi.h>
#include <sys/time.h>
#include "../myHAS_Library/myHAS_Utilities.h"

myHAS_InputButtons * myHAS_InputButtons::getInputButtons(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm)
{
    if(!pInputButtons)
    {
        pInputButtons = new myHAS_InputButtons(iDisp, iSound, iAlarm);
        for (int i=0; i<6; i++)
        {
            wiringPiISR(pInputButtons->gpioButtons[i],INT_EDGE_BOTH, & myHAS_InputButtons::buttonCallback);
        }
        for (int i=0; i<5; i++)
        {
            wiringPiISR(pInputButtons->gpioSelectors[i],INT_EDGE_BOTH, & myHAS_InputButtons::buttonCallback);
        }
        //To define initial mode
        buttonCallback();
    }
    
    return pInputButtons;
}

void myHAS_InputButtons::buttonCallback()
{
    pInputButtons->callbackIsActive = true;
    //debouncing buttons => filter out event coming less than 20ms from last interrupt
    //Does not work well, better to try with a capacitor
    unsigned int deltaT = millis() - pInputButtons->lastInterrupt;
    //if(deltaT<10) //was 20 before
      //  return;
    pInputButtons->lastInterrupt = millis();
    
    int currButtonState = 0;
    int currSelectorState = 0;
    pInputButtons->readInputs(currButtonState, currSelectorState);
    
    //Compute the difference with previous state
    int deltaB = currButtonState ^ pInputButtons->prevButtonState;
    int deltaS = currSelectorState ^ pInputButtons->prevSelectorState;
    
    //save previous button state
    pInputButtons->prevButtonState = currButtonState;
    pInputButtons->prevSelectorState = currSelectorState;
    
    //No change, skip
    if(!deltaB && !deltaS)
    {
        pInputButtons->callbackIsActive = false;
        return;
    }
    
    //A button has been pressed or released
    //order : Time, Hour, Min, Alarm, Sleep, Snooze
    if(deltaB)
    {
        //Stop loop if press on snooze + alarm + time    
        if (currButtonState==0b011010)
            pInputButtons->keepRunning = false;
        
        //alarm button was changed
        if (deltaB & 0b000100) 
        {
            if (currButtonState & 0b000100)
                pInputButtons->unPressAlarm();
            else
                pInputButtons->pressAlarm();
        }
        //Snooze
        if (deltaB & 0b000001)
        {
            if(currButtonState & 0b000001)
                pInputButtons->unPressSnooze();
            else
                pInputButtons->pressSnooze();
        }
        //minutes
        if (deltaB & 0b001000)
        {
            if(currButtonState & 0b001000)
                pInputButtons->unPressMin();
            else
                pInputButtons->pressMin();
        }
        //hours
        if (deltaB & 0b010000)
        {
            if(currButtonState & 0b010000)
                pInputButtons->unPressHour();
            else
                pInputButtons->pressHour();
        }
        //time
        if (deltaB & 0b100000)
        {
            if(currButtonState & 0b100000)
                pInputButtons->unPressTime();
            else
                pInputButtons->pressTime();
        }
        //sleep
        if (deltaB & 0b000010)
        {
            if(currButtonState & 0b000010)
                pInputButtons->unPressSleep();
            else
                pInputButtons->pressSleep();
        }
    }
    
    //A selector has changed
    //order Off, Radio, AutoAlarm, ManAlarm, Bluetooth
    int modeChanged = deltaS >> 1;
    if(modeChanged)
    {
        switch (currSelectorState>>1)
        {
            case 0b1111:
                break;
                
            case 0b0111:
                cout<<getTimeStamp()<<" Change mode to OFF"<<endl;
                pInputButtons->setMode(m_OFF);
                break;
            
            //radio mode
            case 0b1011:
                cout<<getTimeStamp()<<" Change mode to PLAYER"<<endl;
                pInputButtons->setMode(m_PLAYER);
                break;
        
            //auto alarm mode
            case 0b1101:
                cout<<getTimeStamp()<<" Change mode to AUTO_ALARM"<<endl;
                pInputButtons->setMode(m_AUTO_ALARM);
                break;
            
            //manual alarm mode
            case 0b1110:
                cout<<getTimeStamp()<<" Change mode to MAN_ALARM"<<endl;   
                pInputButtons->setMode(m_MAN_ALARM);
                break;
                
            default:
                //pInputButtons->setMode(m_OFF);
                break;
        }
    }
    int soundChanged = deltaS %2;
    if(soundChanged)
    {
        //If currSelectorState = 0 it means we have selected radio
        pInputButtons->audioSourceRadio(currSelectorState%2 ? mm_BLUETOOTH : mm_RADIO);
    }
    
    pInputButtons->callbackIsActive = false;
}

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
    
    for (int i=0; i<5;i++)
    {
        oSelector = 2 * oSelector + digitalRead (gpioSelectors[i]);
    } 
}

void myHAS_InputButtons::runInput()
{
    while(keepRunning)
    {
        sleep(1);
    }
}
