#ifndef myHAS_InputButtons_h
#define myHAS_InputButtons_h

#include "myHAS_Inputs.h"

class myHAS_InputButtons : public myHAS_Inputs
{
    public:
        myHAS_InputButtons(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm);
        ~myHAS_InputButtons();
        
    protected:
        void runInput();
    
    private:    
        void readInputs(int &oButtons, int& oSelector);
        
        //Time, Hour, Min, Alarm, Sleep, Snooze
        int gpioButtons[6]={13,12,3,2,0,4};
        //Radio, AutoAlarm, ManAlarm, Bluetooth
        int gpioSelectors[4]={7,9,8,16};
};

#endif
