#ifndef myHAS_DisplayScreen_h
#define myHAS_DisplayScreen_h

#include "myHAS_Displays.h"
#include <ncurses.h>

class myHAS_DisplayScreen : public myHAS_Displays
{
	public:
		myHAS_DisplayScreen();
		//status: 0 = OFF, 1 = ON, 2 = SNOOZE
		void setAlarmStatus(int iStatus);
		void stopDisplay();
	
	private:
		void runDisplay();
		
		void printColon(int x, int y);
		void printDigit(int x, int y, int iDigit);
	
		WINDOW *win = NULL;
};

#endif
