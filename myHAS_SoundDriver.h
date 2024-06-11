#ifndef myHAS_SoundDriver_h
#define myHAS_SoundDriver_h

#include <string>
#include <thread>
#include <time.h>
#include "myHAS_Displays.h"
#include "../include/ArrayCed.h"
#include "../myHAS_Library/myHAS_SQLClient.h"

using namespace std;
enum musicMode {mm_RADIO, mm_BLUETOOTH};

class myHAS_SoundDriver
{
    public:
        myHAS_SoundDriver(myHAS_SQLClient *iSQLClient, myHAS_Displays *iDisp);
        static size_t curlCallback(void *data, size_t size, size_t nmemb, void *clientp);
        size_t manageCurlOutput(void *data, size_t size, size_t nmemb);
        void readText(string iText, string iVoiceName);
        void playRadio(string iRadioURI);
        void stopRadio();
        //if index==1 => next station, if index = -1 prev station
        void changeRadio(int iIndex);
        bool isRadioON();
        bool check_url(char *url);
        string getGoolgeCloudToken(bool iForce=false);
        void setMusicMode(musicMode iMode);
        musicMode getMusicMode() {return muzicMode;}
        void startBluetooth();
        void stopBluetooth();
        void playSound();
        void stopSound();
        
        void tokenUpdateLoop();
        void startTokenUpdateLoop();
        void stopTokenUpdateLoop();

    private:
        void getMP3fromText(string iVoiceName);
                
        int ttsPlayerPid = -1;
        int radioPlayerPid = -1;
        string ttsText;
        string pathToMP3File = "/home/admin/bonjour.mp3";
        string pathToMP3FileBackup = "/home/admin/bonjour_backup.mp3";
        string currentRadio;
        string curRadioDisp="   ";
        int currentRadioIndex = 0;
        Array<string> listRadio;
        Array<string> listRadioDisplayText;
        string audioJson="";
        myHAS_Displays *pDisp = NULL;
        myHAS_SQLClient *pSQLClient = NULL;
        int pinMute = 9;
        string gCloudToken="";
        time_t tokenUpdateTime=0;
        musicMode muzicMode=mm_RADIO;
        thread *tokenUpdateThread = NULL;
        bool keepRunning = true;
        
};

#endif
