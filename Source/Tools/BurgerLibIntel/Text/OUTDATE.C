#include "Text.h"
#include <time.h>

/****************************************

	Print the time using a time conversion enum

****************************************/

void OutDate(OutDate_t Flag)
{
	time_t MyTime;
	struct tm *MyTime2;
	Word Month,Day;

	MyTime = time(0);				/* Read the time */
	MyTime2 = localtime(&MyTime);
	Month = MyTime2->tm_mon+1;
	Day = MyTime2->tm_mday;
	if (Flag&1) {
		Word Temp;
		Temp = Day;
		Day = Month;
		Month = Temp;
	}
	OutLongWord(Month);
	OutChar('/');
	OutLongWord2(Day,2,'0');
	OutChar('/');
	OutLongWord2(MyTime2->tm_year%100,2,'0');
}
