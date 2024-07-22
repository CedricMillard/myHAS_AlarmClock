#ifndef myHAS_InputButtons_h
#define myHAS_InputButtons_h

#include "myHAS_Inputs.h"

class myHAS_InputButtons : public myHAS_Inputs
{
    public:
        static myHAS_InputButtons * getInputButtons(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm);
        ~myHAS_InputButtons();
        
        myHAS_InputButtons(myHAS_InputButtons &other) = delete;
        void operator=(const myHAS_InputButtons &) = delete;
        
    protected:
        void runInput();
    
    private:    
    
        myHAS_InputButtons(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm);
        
        static myHAS_InputButtons *pInputButtons;
        static void buttonCallback();
        
        void readInputs(int &oButtons, int& oSelector);
        
        //Time, Hour, Min, Alarm, Sleep, Snooze
        //int gpioButtons[6]={13,12,3,2,0,4};
        //Radio, AutoAlarm, ManAlarm, Bluetooth
        //int gpioSelectors[4]={7,9,8,16};
        
        //Time, Hour, Min, Alarm, Sleep, Snooze
        int gpioButtons[6]={21,30,14,13,12,22};
        //Off, Radio, AutoAlarm, ManAlarm, Bluetooth
        int gpioSelectors[5]={7,0,3,2,8};
        
        int prevButtonState = 0;
        int prevSelectorState = 0;
        unsigned int lastInterrupt = 0; 
        
        bool callbackIsActive = false;
        
        bool keepRunning = true;
};

#endif
