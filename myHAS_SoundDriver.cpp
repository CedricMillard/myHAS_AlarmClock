/**
 * TODO: 
 *  - Add function to check if radio is ON or OFF
 *  - Manage error in the json received from google in case it is incorrect 
 *  - Split WakeUpMsg Preparation from playing it to stop it in case alarm is stopped before it is ready...
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

string myHAS_SoundDriver::getGoolgeCloudToken()
{
	string token = "";
	char buffer[128];
	FILE * pipe = popen("gcloud auth application-default print-access-token","r");
	if (!pipe) return "";
	while (fgets(buffer, sizeof(buffer), pipe) !=NULL)
		token += buffer;
	pclose(pipe);
	return token;
}

void myHAS_SoundDriver::readText(string iText, string iVoiceName)
{
	ttsText = iText;
	getMP3fromText(iVoiceName);
	
	if (access(pathToMP3File.c_str(), F_OK) == 0)
	{
		string command = "cvlc " + pathToMP3File + " vlc://quit";
		system(command.c_str());
        //delete the file to not have an outdated message
        if(remove(pathToMP3File.c_str()))
            cout<<"ERROR deleting "<<pathToMP3File<<endl;
	}
	else
		cout<<"ERROR: FILE "<<pathToMP3File<<" not found"<<endl;
}

void myHAS_SoundDriver::playRadio(string iRadioURI)
{	
	if(radioPlayerPid>0)
		return;
	
	if(iRadioURI.length()==0)
		iRadioURI = currentRadio;
	int pid = fork();
	if (pid==0)
	{
		if(execlp("mplayer","-afm", "-nocache", "-noconsolecontrols", "-really-quiet", iRadioURI.c_str())==-1)
        {
            cout<<"ERROR LAUNCHING mplayer "<<strerror(errno)<<" "<<iRadioURI<<endl;
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
	if(radioPlayerPid>0)
	{
		kill(radioPlayerPid,SIGKILL);
		sleep(0.2);
		radioPlayerPid = -1;
	}
	
}

void myHAS_SoundDriver::getMP3fromText(string iVoiceName)
{
	string languageCode = iVoiceName.substr(0,5);
	
    string ttsRequest="{'input':{'text':";
	ttsRequest += "'" + ttsText + "'";
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
	string token = "Authorization: Bearer ";
	token += getGoolgeCloudToken();
	
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
	istringstream encodedString (conSettings["audioContent"].GetString());
	ostringstream decodedString;
	base64::decoder D;
	D.decode(encodedString, decodedString);
	FILE * fBonjour = fopen(pathToMP3File.c_str(), "w"); 
	fwrite(decodedString.str().c_str(), sizeof(char), decodedString.str().length(), fBonjour);
	fclose(fBonjour);
	audioJson = "";
    ttsText="";
}

void myHAS_SoundDriver::changeRadio(int iIndex)
{
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
