/**
 * TODO: 
 *  - trigger dB update to change radio list on the fly
 * 
 */
#include "myHAS_SoundDriver.h"
#include <unistd.h> 
#include <csignal>
#include <sstream>
#include <errno.h>
#include <curl/curl.h>
#include <b64/decode.h>
#include "../include/rapidjson/document.h"
#include <wiringPi.h>
#include "../myHAS_Library/myHAS_Utilities.h"

using namespace std;
using namespace rapidjson;

myHAS_SoundDriver::myHAS_SoundDriver(myHAS_SQLClient *iSQLClient)
{
	if(!iSQLClient)
		return;
	pSQLClient = iSQLClient;
	listRadio = iSQLClient->getStringList("RadioStation","URI");
	if(listRadio.size()==0)
	{
		cout<<"List of radio stations is empty"<<endl;
		return;
	}
	currentRadio = listRadio[0];
	
	wiringPiSetup();
	pinMode(pinMute, OUTPUT);
    pullUpDnControl(pinMute, PUD_DOWN);
    digitalWrite(pinMute,0);
    //update the token for faster speech
    getGoolgeCloudToken();
}

size_t myHAS_SoundDriver::curlCallback(void *data, size_t size, size_t nmemb, void *clientp)
{
	return ((myHAS_SoundDriver*)clientp)->manageCurlOutput(data, size, nmemb);
}

size_t myHAS_SoundDriver::manageCurlOutput(void *data, size_t size, size_t nmemb)
{
	size_t realsize = size*nmemb;

	char *ptr = (char*)malloc((realsize+1)*sizeof(char));
	strncpy(ptr, (const char*)data,realsize);
	ptr[realsize] = '\0';
	
	audioJson+=string(ptr);

	return realsize;
}

string myHAS_SoundDriver::getGoolgeCloudToken(bool iForce)
{
	//update token if empty or older than a day
	if (gCloudToken.length()==0 || difftime(getUnixTime(), tokenUpdateTime)>=3000 || iForce)
	{
		gCloudToken = "";
		char buffer[128];
		FILE * pipe = popen("gcloud auth application-default print-access-token","r");
		if (!pipe) return "";
		while (fgets(buffer, sizeof(buffer), pipe) !=NULL)
			gCloudToken += buffer;
		pclose(pipe);
		tokenUpdateTime = getUnixTime();
		cout<<"Google token updated"<<endl;
	}
	return gCloudToken;
}

void myHAS_SoundDriver::readText(string iText, string iVoiceName)
{
	ttsText = iText;
	getMP3fromText(iVoiceName);
	
	if (access(pathToMP3File.c_str(), F_OK) == 0)
	{
		digitalWrite(pinMute,1);
		//string command = "cvlc " + pathToMP3File + " --gain 2 vlc://quit";
		string command = "mplayer -af volume=5:0 -nocache -noconsolecontrols -really-quiet "+pathToMP3File;
		system(command.c_str());
		digitalWrite(pinMute,0);
        //delete the file to not have an outdated message
        if(remove(pathToMP3File.c_str()))
            cout<<"ERROR deleting "<<pathToMP3File<<endl;
	}
	else if (access(pathToMP3FileBackup.c_str(), F_OK) == 0)
	{
		cout<<"ERROR audio file not generated, playing back-up "<<pathToMP3File<<endl;
		digitalWrite(pinMute,1);
		string command = "mplayer -af volume=5:0 -nocache -noconsolecontrols -really-quiet "+pathToMP3FileBackup;
		system(command.c_str());
		digitalWrite(pinMute,0);
	}
	else
		cout<<"ERROR: FILE "<<pathToMP3File<<" not found"<<endl;
}

void myHAS_SoundDriver::playRadio(string iRadioURI)
{	
	if(isRadioON())
		return;
	
	stopBluetooth();
	
	if(iRadioURI.length()==0)
		iRadioURI = currentRadio;
	int pid = fork();
	digitalWrite(pinMute,1);
	if (pid==0)
	{
		
		if(execlp("mplayer", "-nocache", "-noconsolecontrols", "-really-quiet", iRadioURI.c_str())==-1)
        {
            cout<<"ERROR LAUNCHING mplayer "<<strerror(errno)<<" "<<iRadioURI<<endl;
            digitalWrite(pinMute,0);
            exit(-1);
        }
	}
	else 
	{ 
		radioPlayerPid = pid;
	}
}

void myHAS_SoundDriver::stopRadio()
{
	digitalWrite(pinMute,0);
	if(isRadioON())
	{
		kill(radioPlayerPid,SIGKILL);
		sleep(0.2);
		radioPlayerPid = -1;
	}	
}

void myHAS_SoundDriver::getMP3fromText(string iVoiceName)
{
	string languageCode = iVoiceName.substr(0,5);
	cout<<"getMP3fromText()"<<endl;
    string ttsRequest="{'input':{'text':";
	ttsRequest += "\"" + ttsText + "\"";
	ttsRequest+="},'voice':{'languageCode':";
    ttsRequest=ttsRequest+"'"+languageCode+"'";
    ttsRequest+=",'name':";
    ttsRequest=ttsRequest+"'" + iVoiceName + "'";
    ttsRequest+="},'audioConfig':{'audioEncoding':'MP3'}}";
	
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *handle = curl_easy_init();
	//curl -X POST 
	//     -H "Authorization: Bearer $(gcloud auth application-default print-access-token)"     
	//     -H "Content-Type: application/json; charset=utf-8"     
	//     -d @ttsReq.json
	//     "https://texttospeech.googleapis.com/v1/text:synthesize" >>  bonjour.json
	struct curl_slist *list = NULL;
	curl_easy_setopt(handle, CURLOPT_URL, "https://texttospeech.googleapis.com/v1/text:synthesize");
	string token = getGoolgeCloudToken();
	if (token.length()==0)
	{
		cout<<"Failed to retrieve gCloud token"<<endl;
		return;
	}
	token = "Authorization: Bearer "+token;
	
	list = curl_slist_append(list, strndup(token.c_str(),token.length()-1));
	if(list==NULL)
	{
		cout<<"ERROR adding in list"<<endl;
		return;
	}
	list = curl_slist_append(list, "Content-Type: application/json; charset=utf-8");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);
	
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, ttsRequest.c_str());
	
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlCallback);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)this);
	CURLcode res = curl_easy_perform(handle);
	curl_slist_free_all(list);
	if(res!=CURLE_OK)
		cout<<"ERROR when requesting  TTS: "<<curl_easy_strerror(res)<<endl;		
	if(audioJson.length()==0)
		cout<<"ERROR, TTS output is empty"<<endl;
	else
	{
		//cout<<"RESULT JSON: "<<audioJson<<endl;
	}
	curl_easy_cleanup(handle);
	curl_global_cleanup();
	
	//Converti en mp3 avec libbase64 et enregistre un fichier mp3
	Document conSettings;
	conSettings.Parse(audioJson.c_str());
	if(!conSettings.IsObject())
	{
		cout<<"Json is corrupted: "<<audioJson<<endl;
		return;
	}
	if(!conSettings.HasMember("audioContent"))
	{
		cout<<"Json does not contain audio: "<<audioJson<<endl;
		return;
	}
	istringstream encodedString (conSettings["audioContent"].GetString());
	ostringstream decodedString;
	base64::decoder D;
	D.decode(encodedString, decodedString);
	FILE * fBonjour = fopen(pathToMP3File.c_str(), "w"); 
	fwrite(decodedString.str().c_str(), sizeof(char), decodedString.str().length(), fBonjour);
	fclose(fBonjour);
	audioJson = "";
    ttsText="";
    cout<<"done with creating mp3 file"<<endl;
}

void myHAS_SoundDriver::changeRadio(int iIndex)
{
	if(muzicMode != mm_RADIO)
		return;
		
	stopRadio();
	currentRadioIndex = currentRadioIndex + iIndex;
	if(currentRadioIndex >= listRadio.size())
		currentRadioIndex = 0;
	else if(currentRadioIndex <0)
		currentRadioIndex = listRadio.size() - 1;
		
	currentRadio = listRadio[currentRadioIndex];
	playRadio();
}

bool myHAS_SoundDriver::isRadioON()
{
	return radioPlayerPid>0;
}

bool myHAS_SoundDriver::check_url(char *url)
{
    CURL *curl;
    CURLcode response;

    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        /* don't write output to stdout */
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

        /* Perform the request */
        response = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return (response == CURLE_OK);
}

void myHAS_SoundDriver::setMusicMode(musicMode iMode) 
{
	muzicMode = iMode;
}

void myHAS_SoundDriver::startBluetooth()
{
	cout<<"Start Bluetooth"<<endl;
	string command = "systemctl --user unmask pulseaudio.socket && systemctl --user start pulseaudio.service";
	system(command.c_str());
	digitalWrite(pinMute,1);
}

void myHAS_SoundDriver::stopBluetooth()
{
	cout<<"Stop Bluetooth"<<endl;
	string command = "systemctl --user mask pulseaudio.socket && systemctl --user stop pulseaudio.service";
	system(command.c_str());
	digitalWrite(pinMute,0);
}

void myHAS_SoundDriver::playSound()
{
	if(muzicMode==mm_BLUETOOTH)
		startBluetooth();
	else
		playRadio();
}

void myHAS_SoundDriver::stopSound()
{
	stopBluetooth();
	stopRadio();
}

void myHAS_SoundDriver::startTokenUpdateLoop()
{
    stopTokenUpdateLoop();
    keepRunning = true;
    tokenUpdateThread = new thread(&myHAS_SoundDriver::tokenUpdateLoop, this);
}

void myHAS_SoundDriver::stopTokenUpdateLoop()
{
	keepRunning = false;
	
    //Kill the thread
    if(tokenUpdateThread)
    {
        tokenUpdateThread->join();
        delete tokenUpdateThread;
        tokenUpdateThread = NULL;
    }
}

void myHAS_SoundDriver::tokenUpdateLoop()
{
	while(keepRunning)
    {
		getGoolgeCloudToken();
		sleep(500);
	}
}
