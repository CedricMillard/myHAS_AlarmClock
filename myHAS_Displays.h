#ifndef myHAS_Displays_h
#define myHAS_Displays_h

#include <thread>

using namespace std;

class myHAS_Displays
{
    public:
    myHAS_Displays(){}
    //void setAlarm(long* iAlarmTime);
    void setAlarmTime(long iAlarmTime){alarmTime = iAlarmTime;}
    void displayAlarm(bool idispAlarm);
    void displayEqualizer(bool idispEq);

    virtual void startDisplay();
    virtual void stopDisplay();
    
    //status: 0 = OFF, 1 = ON, 2 = SNOOZE
    virtual void setAlarmStatus(int iStatus){}
    
    protected:
    virtual void runDisplay() = 0;
    
    //long* alarmTime = NULL;
    long alarmTime = -1;
    bool dispAlarm = false;
    bool dispEqualizer = false;
    bool keepRunning = true;
    thread dispThread;
};

#endif
