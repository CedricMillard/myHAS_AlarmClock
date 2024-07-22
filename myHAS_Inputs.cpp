#include "myHAS_Inputs.h"

myHAS_Inputs::myHAS_Inputs(myHAS_Displays *iDisp, myHAS_SoundDriver *iSound, myHAS_Alarm *iAlarm)
{
    pDisp = iDisp;
    pSound = iSound;
    pAlarm = iAlarm;
}

void myHAS_Inputs::pressSnooze()
{
    if(mode==m_PLAYER)
    {
        pDisp->setDisplayMode(dm_TIME);
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
    if(mode==m_PLAYER)
    {
        if(pSound->getMusicMode()==mm_RADIO)
            pDisp->setDisplayMode(dm_EQUALIZER);
        else
            pDisp->setDisplayMode(dm_BLUETOOTH);
    }
}

void myHAS_Inputs::pressSleep()
{
    if(mode==m_AUTO_ALARM || mode==m_MAN_ALARM)
    {
        if(pAlarm->getAlarmState()!=as_OFF)
            pAlarm->stopAlarm();
        else
            pSound->changeRadio(0);
    }
    else if (mode==m_PLAYER)
        pSound->changeRadio(0);
}

void myHAS_Inputs::unPressSleep()
{
    
}

void myHAS_Inputs::pressHour()
{
    if(isAlarmPressed || isTimePressed)
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
    if(isAlarmPressed || isTimePressed)
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
    switch(mode)
    {
        case m_OFF:
            pDisp->setDisplayMode(dm_ALARM);
            isAlarmPressed = true;
            break;
        case m_PLAYER:
            break;
        case m_AUTO_ALARM:
            pDisp->setDisplayMode(dm_ALARM);
            break;
        case m_MAN_ALARM:
            pDisp->setDisplayMode(dm_ALARM);
            isAlarmPressed = true;
            break;
        default:
            break;
    }
}

void myHAS_Inputs::unPressAlarm()
{
    switch(mode)
    {
        case m_OFF:
            pDisp->setDisplayMode(dm_TIME);
            isAlarmPressed = false;
            pAlarm->saveAlarmTime();
            break;
        case m_PLAYER:
            break;
        case m_AUTO_ALARM:
            pDisp->setDisplayMode(dm_TIME);
            break;
        case m_MAN_ALARM:
            pDisp->setDisplayMode(dm_TIME);
            isAlarmPressed = false;
            pAlarm->saveAlarmTime();
            break;
        default:
            break;
    }
}

void myHAS_Inputs::pressTime()
{
    switch(mode)
    {
        case m_OFF:
            pDisp->setDisplayMode(dm_ALARM);
            isTimePressed = true;
            break;
        case m_PLAYER:
            break;
        case m_AUTO_ALARM:
            pDisp->setDisplayMode(dm_ALARM);
            break;
        case m_MAN_ALARM:
            pDisp->setDisplayMode(dm_ALARM);
            isTimePressed = true;
            break;
        default:
            break;
    }
}

void myHAS_Inputs::unPressTime()
{
    switch(mode)
    {
        case m_OFF:
            pDisp->setDisplayMode(dm_TIME);
            isTimePressed = false;
            pAlarm->saveAlarmTime();
            break;
        case m_PLAYER:
            break;
        case m_AUTO_ALARM:
            pDisp->setDisplayMode(dm_TIME);
            break;
        case m_MAN_ALARM:
            pDisp->setDisplayMode(dm_TIME);
            isTimePressed = false;
            pAlarm->saveAlarmTime();
            break;
        default:
            break;
    }
}

void myHAS_Inputs::setMode(MODE iMode)
{
    if(iMode == mode)
    {
        return;
    }
    mode = iMode;
    switch(mode)
    {
        case m_OFF:
			pAlarm->stopAlarm();
			pSound->stopSound();
			pAlarm->setAlarmMode(am_OFF);
			pDisp->setDisplayMode(dm_TIME);
            break;
        
        case m_PLAYER:
            pAlarm->setAlarmMode(am_OFF);
            //Stop the radio if it was started with sleep button
            pSound->stopSound();
            if(mode!=m_PLAYER)
                break;
            pSound->playSound();
            //in case mode has already changed as play sound is a bit slow
            if(mode==m_PLAYER)
            {
                if(pSound->getMusicMode()==mm_RADIO)
                    pDisp->setDisplayMode(dm_EQUALIZER);
                else
                    pDisp->setDisplayMode(dm_BLUETOOTH);
            }
            else
                pSound->stopSound();
            break;
            
        case m_AUTO_ALARM:
            pSound->stopSound();
			pAlarm->setAlarmMode(am_AUTO);
			pDisp->setDisplayMode(dm_TIME);
            break;
            
        case m_MAN_ALARM:
            pSound->stopSound();
			pAlarm->setAlarmMode(am_MANUAL);
			pDisp->setDisplayMode(dm_TIME);
            break;
        
        default:
            break;
    }
}

void myHAS_Inputs::audioSourceRadio(musicMode iRadioMode)
{
    if(iRadioMode == pSound->getMusicMode())
        return;
        
    pSound->setMusicMode(iRadioMode);
    if(mode== m_PLAYER)
    {
        if (iRadioMode == mm_BLUETOOTH)
        {
            pSound->stopRadio();
            pSound->startBluetooth();
            pDisp->setDisplayMode(dm_BLUETOOTH);
        }
        else
        {
            pSound->stopBluetooth();
            pSound->changeRadio(0);
            pDisp->setDisplayMode(dm_EQUALIZER);
        }
    }
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
