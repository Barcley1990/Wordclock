
#ifndef NETWORK_H
#define NETWROK_H

#include "time.h"


class Network
{
    public:
        Network();
        void GetNTPTime(struct tm* timeinfo);
        
    
    private:
        const char*           ssid     = "TARDIS";
        const char*           password = "82uqFnUSjUn7YL";  
        const char*           NTP_Server = "pool.ntp.org";
        const long            GTM_Offset_Sec = 3600; // +1h
        const int             DayLight_Offset_Sec = 0; // +0h winter time

};

#endif