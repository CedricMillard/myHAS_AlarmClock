#ifndef myHAS_DisplaySegments_h
#define myHAS_DisplaySegments_h

#include "myHAS_Displays.h"

//using namespace std;

class myHAS_DisplaySegments : public myHAS_Displays
{
    public:
	myHAS_DisplaySegments();
	//main loop to be run in a separate thread
	void runDisplay();
	void stopDisplay();

    private:
	// Display a number on the specified digit
	void displaySegments (int iDigit, int iNumber, bool cat, bool iSemiColon);

	int catGPIO[2] = {26,5};
	int gpioTable4[4]={31, 27, 28, 29};
	int gpioTable3[4]={11, 6, 10, 29};
	int gpioTable2[4]={30, 21, 22, 23};
	int gpioTable1[4]={23, 24, 25, 14};
};


#endif
