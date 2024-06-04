#ifndef myHAS_DisplaySegments_h
#define myHAS_DisplaySegments_h

#include "myHAS_Displays.h"

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
	int getCharValue(char iChar);

	int catGPIO[2] = {15,16};
	int gpioTable4[4]={4, 5, 6, 10};
	int gpioTable3[4]={26, 11, 31, 10};
	int gpioTable2[4]={23, 24, 27, 28};
	int gpioTable1[4]={28, 25, 29, 1};
};


#endif
