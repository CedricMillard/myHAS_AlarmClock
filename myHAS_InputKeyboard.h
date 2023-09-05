#ifndef myHAS_InputKeyboard_h
#define myHAS_InputKeyboard_h

#include "myHAS_Inputs.h"

class myHAS_InputKeyboard : public myHAS_Inputs
{
    public:
        myHAS_InputKeyboard(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm);
        ~myHAS_InputKeyboard();
        
    protected:
        void runInput();
    
    private:    
        
};

#endif
