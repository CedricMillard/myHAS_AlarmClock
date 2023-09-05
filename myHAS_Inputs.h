#ifndef myHAS_Inputs_h
#define myHAS_Inputs_h

#include "myHAS_Displays.h"
#include "myHAS_SoundDriver.h"
#include "myHAS_Alarm.h"

using namespace std;

enum MODE {m_OFF, m_PLAYER, m_MAN_ALARM, m_AUTO_ALARM};

class myHAS_Inputs
{
    public:
        myHAS_Inputs(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm);
        virtual ~myHAS_Inputs(){}
        virtual void runInput() = 0;
        
    protected:
    
        void pressSnooze();
        void unPressSnooze();
        void pressSleep();
        void unPressSleep();
        void pressHour();
        void unPressHour();
        void pressMin();
        void unPressMin();
        void pressAlarm();
        void unPressAlarm();
        void pressTime();
        void unPressTime();
        void setMode(MODE iMode);
        void audioSourceRadio(bool iIsRadio);
        
        
        myHAS_Displays *pDisp = NULL;
        myHAS_SoundDriver *pSound = NULL;
        myHAS_Alarm *pAlarm = NULL;
        MODE mode = m_OFF;
        bool isRadio = true;
        bool showAlarm = false;
	bool dispEqualizer = false;
};

#endif
