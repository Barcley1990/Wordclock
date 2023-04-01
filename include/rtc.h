
#ifndef RTC_H
#define RTC_H

#include <time.h>

void Rtc_Setup(struct tm* timeinfo);
void Rtc_GetTime();
void Rtc_SetTime();


#endif