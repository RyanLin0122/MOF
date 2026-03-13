#pragma once

#include <cstring>

#if defined(_WIN32)
#define STR_ICMP _stricmp
#else
#include <strings.h>
#define STR_ICMP strcasecmp
#endif

class cltCountrySystem {
public:
    cltCountrySystem();
    ~cltCountrySystem();

    void Initialize(int country);
    int Initialize(char* countryName);
    void Free();

    int IsKorea();
    int IsJapan();
    int IsTaiwan();
    int IsIndonesia();
    int IsHongKong();

    int GetCountry();

private:
    int m_country = 0;
};
