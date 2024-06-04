#ifndef myHAS_Displays_h
#define myHAS_Displays_h

#include <time.h>
#include <thread>
#include <string>

using namespace std;

enum displayMode {dm_TIME, dm_ALARM, dm_EQUALIZER, dm_BLUETOOTH};

class myHAS_Displays
{
    public:
    myHAS_Displays(){}
    
    void setAlarmTime(long iAlarmTime){alarmTime = iAlarmTime;}

    virtual void startDisplay();
    virtual void stopDisplay();
    
    //status: 0 = OFF, 1 = ON, 2 = SNOOZE
    virtual void setAlarmStatus(int iStatus){}
    
    //0=OFF, 1=AUTO, 2=MANUAL
    void setAlarmMode(int iMode){alrmMode = iMode;}
    
    void setDisplayMode(displayMode iDispMode) {dispMode = iDispMode;}
    
    //Display an alternative text (max 3 chars) for 3 seconds
    void startTemporaryDisplay(string iText, int iDuration);
    
    protected:
    virtual void runDisplay() = 0;
    
    long alarmTime = -1;
    displayMode dispMode = dm_TIME;
    bool keepRunning = true;
    thread dispThread;
    int alrmMode=0;
    string temporaryText="";
    time_t tempTextExpires=0;
};

#endif
