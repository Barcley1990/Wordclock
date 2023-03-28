/*-------------------------------------------------------------------------
Network library

Written by Tobias Nuss

-------------------------------------------------------------------------
This file is part of the Makuna/Rtc library.

-------------------------------------------------------------------------*/
#ifndef NETWORK_H
#define NETWROK_H

#include "time.h"


class Network
{
    public:
        Network(const char *ssid, const char *pwd)
        {
            this->_ssid = ssid;
            this->_password = pwd;
        }
        void GetNTPTime(struct tm* timeinfo);
        
    
    private:
        const char*           _ssid     = "";
        const char*           _password = "";  
        const char*           NTP_Server = "pool.ntp.org";
        const long            GTM_Offset_Sec = 3600; // +1h
        const int             DayLight_Offset_Sec = 0; // +0h winter time

};

#endif