#ifndef myHAS_Alarm_h
#define myHAS_Alarm_h
#include "myHAS_Displays.h"
#include "myHAS_SoundDriver.h"
#include "myHAS_Environment.h"
#include "../myHAS_Library/myHAS_SQLClient.h"
#include <thread>
#include <string>
#include "../include/ArrayCed.h"

using namespace std;

enum alarmMode {am_OFF, am_MANUAL, am_AUTO};
enum alarmState {as_OFF, as_ON, as_SNOOZE};

struct Rule
{
  int alarmTime = -1;

  string alarmSound;
  //Freq coded on 8 bits. higher bit means one time only
  // example: 10000000 = 128 => one time, no repeat
  //          01000001 = 65 => every saturday and sunday (2^0 = sunday, 2^1 = monday...)
  uint8_t frequency;

  //inactive rules are kept but not evaluated
  bool active = true;
};

class myHAS_Alarm
{
    public:
        myHAS_Alarm(int iID, myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Environment *iEnv, myHAS_SQLClient *iSQLClient);
        
        void startAlarmLoop();
        void stopAlarmLoop();
        
        void setAlarmMode(alarmMode iMode);
        alarmState getAlarmState(){return aState;}
        
        void stopAlarm();
        void snoozeAlarm();
        void setManualAlarmTime(long iTime);
        long getManualAlarmTime(){return manualAlarmTime;}
        //Define snooze time in seconds (default 30sec)
        void setSnoozeTime(int iSnoozeTime){snoozeTime = iSnoozeTime;}
        void saveAlarmTime();
        string getText(string iTextField);
        string voiceName="fr-FR-Wavenet-A";
    
    private:
        void alarmLoop();
        void ringAlarm(string iAlarmSound="");
        void importParameters();
        void computeNextAlarm(Rule iRule, long icurrTime);
        void checkSQLUpdate();
        void updateRulesInDB();
        
        thread *alarmThread = NULL;
        myHAS_Displays *pDisp = NULL;
        myHAS_SoundDriver *pSound = NULL;
        myHAS_Environment *pEnv = NULL;
        myHAS_SQLClient *pSQLClient = NULL;
        bool keepRunning = true;
                
        int ID = 0;
        
        int manualAlarmTime = -1;
        long alarmTime = -1;
        int nextAlarmTime= 99999;
        
        alarmState aState = as_OFF;
        alarmMode aMode=am_OFF;

        int snoozeTime = 1; //in minutes
        
        Array<Rule> listRules;
        string currentAlarmSound;
        
        string sqlFileCheck = "/temp/alarmclock.change";
};

#endif
