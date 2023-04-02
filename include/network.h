/*-------------------------------------------------------------------------
Network library

Written by Tobias Nuss

-------------------------------------------------------------------------


-------------------------------------------------------------------------*/
#ifndef NETWORK_H
#define NETWROK_H

#include "time.h"



void Wifi_Setup(const char *ssid, const char *pwd);
void Wifi_Get_NtpTime(struct tm* timeinfo);
void Wifi_StartAP();
void Wifi_StopAP();
void Wifi_ServerExec();

#endif