/*-------------------------------------------------------------------------
Network library

Written by Tobias Nuss

-------------------------------------------------------------------------


-------------------------------------------------------------------------*/
#ifndef NETWORK_H
#define NETWROK_H

#include "time.h"



void Wifi_Setup();
void Wifi_Get_NtpTime(struct tm* timeinfo);
void Wifi_ServerExec();

#endif