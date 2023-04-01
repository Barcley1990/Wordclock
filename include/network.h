/*-------------------------------------------------------------------------
Network library

Written by Tobias Nuss

-------------------------------------------------------------------------
This file is part of the Makuna/Rtc library.

-------------------------------------------------------------------------*/
#ifndef NETWORK_H
#define NETWROK_H

#include "time.h"

void Wifi_Setup(const char *ssid, const char *pwd);
void Wifi_Get_NtpTime(struct tm* timeinfo);

#endif