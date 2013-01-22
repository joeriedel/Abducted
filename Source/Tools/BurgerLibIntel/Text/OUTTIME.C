#include "Text.h"
#include <Time.h>

/****************************************

	Print the time using a time conversion enum

****************************************/

void OutTime(OutTime_t Flag)
{
	time_t MyTime;
	struct tm *MyTime2;
	Word AM;
	Word Hour;

	MyTime = time(0);				/* Read the time */
	MyTime2 = localtime(&MyTime);
	Hour = MyTime2->tm_hour;

	if (!(Flag&1)) {
		if (Hour>=12) {
			Hour -=12;
			AM = 'P';
		} else {
			AM = 'A';
		}
		if (!Hour) {
			Hour = 12;
		}
	}
	OutLongWord(Hour);
	OutChar(':');
	OutLongWord2(MyTime2->tm_min,2,'0');
	if (Flag&2) {
		OutChar(':');
		OutLongWord2(MyTime2->tm_sec,2,'0');
	}
	if (!(Flag&1)) {
		OutChar(' ');
		OutChar(AM);
		OutChar('M');
	}
}

