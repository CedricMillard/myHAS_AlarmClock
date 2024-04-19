#include "myHAS_Inputs.h"

myHAS_Inputs::myHAS_Inputs(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm)
{
    pDisp = iDisp;
    pSound = iSound;
    pAlarm = iAlarm;
}

void myHAS_Inputs::pressSnooze()
{
    //Add that in other state, pressing snooze will display temperature
    if(mode==m_PLAYER)
    {
        dispEqualizer = !dispEqualizer;
        pDisp->displayEqualizer(dispEqualizer);
    }
    else if (mode==m_AUTO_ALARM || mode==m_MAN_ALARM)
    {
        if (pAlarm->getAlarmState()==as_ON)
            pAlarm->snoozeAlarm();
        else if (pSound->isRadioON())
            pSound->stopRadio();
        else
            pSound->readText(pAlarm->getText("SnoozeText"), pAlarm->voiceName);
    }
    else
    {        
        pSound->readText(pAlarm->getText("SnoozeText"), pAlarm->voiceName);
    }
}

void myHAS_Inputs::unPressSnooze()
{
    
}

void myHAS_Inputs::pressSleep()
{
    if(mode==m_AUTO_ALARM || mode==m_MAN_ALARM)
    {
        if(pAlarm->getAlarmState()!=as_OFF)
            pAlarm->stopAlarm();
        else
            pSound->playRadio();
    }
}

void myHAS_Inputs::unPressSleep()
{
    
}

void myHAS_Inputs::pressHour()
{
    if(showAlarm)
    {
        if(mode!=m_AUTO_ALARM)
        {
            if(isAlarmPressed)
                changeAlarmMin(-1);
            else
                changeAlarmHour(-1);
        }
    }
    else if (pSound->isRadioON())
        pSound->changeRadio(-1);
}

void myHAS_Inputs::unPressHour()
{
    
}

void myHAS_Inputs::pressMin()
{
    if(showAlarm)
    {
        if(mode!=m_AUTO_ALARM)
        {
            if(isAlarmPressed)
                changeAlarmMin(1);
            else
                changeAlarmHour(1);
        }
    }
    else if (pSound->isRadioON())
        pSound->changeRadio(1);
}

void myHAS_Inputs::unPressMin()
{
    
}

void myHAS_Inputs::pressAlarm()
{
    showAlarm=true;
    pDisp->displayAlarm(true);
    isAlarmPressed = true;
}

void myHAS_Inputs::unPressAlarm()
{
    showAlarm=false;
    pDisp->displayAlarm(false);
    isAlarmPressed = false;
    pAlarm->saveAlarmTime();
}

void myHAS_Inputs::pressTime()
{
    showAlarm=true;
    pDisp->displayAlarm(true);
    isTimePressed = true;
}

void myHAS_Inputs::unPressTime()
{
    showAlarm=false;
    pDisp->displayAlarm(false);
    isTimePressed = false;
    pAlarm->saveAlarmTime();
}

void myHAS_Inputs::setMode(MODE iMode)
{
    if(iMode == mode)
        return;
    mode = iMode;
    switch(mode)
    {
        case m_OFF:
			pAlarm->stopAlarm();
			pSound->stopRadio();
			pAlarm->setAlarmMode(am_OFF);
			pDisp->displayEqualizer(false);
            break;
        
        case m_PLAYER:
            pAlarm->setAlarmMode(am_OFF);
			pSound->playRadio();
			dispEqualizer = true;
			pDisp->displayEqualizer(dispEqualizer);
            break;
            
        case m_AUTO_ALARM:
            pSound->stopRadio();
			pAlarm->setAlarmMode(am_AUTO);
			pDisp->displayEqualizer(false);
            break;
            
        case m_MAN_ALARM:
            pSound->stopRadio();
			pAlarm->setAlarmMode(am_MANUAL);
			pDisp->displayEqualizer(false);
            break;
        
        default:
            break;
    }
}

void myHAS_Inputs::audioSourceRadio(bool iIsRadio)
{
    
}

void myHAS_Inputs::changeAlarmMin(int iIncrement)
{
    long alarmTime = pAlarm->getManualAlarmTime();
    int alarmHour = alarmTime/60;
    int alarmMin = (alarmTime%60);
    alarmMin=alarmMin+iIncrement;
    if(alarmMin>59) alarmMin = 0;
    else if(alarmMin<0) alarmMin = 59; 
    alarmTime=alarmHour*60 + alarmMin;
    pAlarm->setManualAlarmTime(alarmTime);
}

void myHAS_Inputs::changeAlarmHour(int iIncrement)
{
    long alarmTime = pAlarm->getManualAlarmTime();
    int alarmHour = alarmTime/60;
    int alarmMin = (alarmTime%60);
    alarmHour=alarmHour+iIncrement;
    if(alarmHour>23) alarmHour = alarmHour - 24;
    else if(alarmHour<0) alarmHour = 24 + alarmHour; 
    alarmTime=alarmHour*60 + alarmMin;
    pAlarm->setManualAlarmTime(alarmTime);
}
