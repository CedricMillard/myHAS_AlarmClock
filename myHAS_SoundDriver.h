#ifndef myHAS_SoundDriver_h
#define myHAS_SoundDriver_h

#include <string>
#include <thread>
#include <time.h>
#include "../include/ArrayCed.h"
#include "../myHAS_Library/myHAS_SQLClient.h"

using namespace std;

class myHAS_SoundDriver
{
    public:
        myHAS_SoundDriver(myHAS_SQLClient *iSQLClient);
        static size_t curlCallback(void *data, size_t size, size_t nmemb, void *clientp);
        size_t manageCurlOutput(void *data, size_t size, size_t nmemb);
        void readText(string iText, string iVoiceName);
        void playRadio(string iRadioURI="");
        void stopRadio();
        //if index==1 => next station, if index = -1 prev station
        void changeRadio(int iIndex);
        bool isRadioON();
        bool check_url(char *url);

    private:
        void getMP3fromText(string iVoiceName);
        string getGoolgeCloudToken();
        
        int ttsPlayerPid = -1;
        int radioPlayerPid = -1;
        string ttsText;
        string pathToMP3File = "/home/admin/bonjour.mp3";
        string pathToMP3FileBackup = "/home/admin/bonjour_backup.mp3";
        string currentRadio;
        int currentRadioIndex = 0;
        Array<string> listRadio;
        string audioJson="";
        myHAS_SQLClient *pSQLClient = NULL;
        int pinMute = 9;
        string gCloudToken="";
        time_t tokenUpdateTime=0;
        
};

#endif
