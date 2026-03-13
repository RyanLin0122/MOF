#include "System/cltCountrySystem.h"

#include <cstring>

#if defined(_WIN32)
#define STR_ICMP _stricmp
#else
#include <strings.h>
#define STR_ICMP strcasecmp
#endif

cltCountrySystem::cltCountrySystem() = default;
cltCountrySystem::~cltCountrySystem() { m_country = 0; }

void cltCountrySystem::Initialize(int country) { m_country = country; }

int cltCountrySystem::Initialize(char* countryName) {
    if (!countryName) {
        return 0;
    }

    if (STR_ICMP("KOREA", countryName) == 0) {
        Initialize(0);
        return 1;
    }
    if (STR_ICMP("JAPAN", countryName) == 0) {
        Initialize(1);
        return 1;
    }
    if (STR_ICMP("TAIWAN", countryName) == 0) {
        Initialize(2);
        return 1;
    }
    if (STR_ICMP("INDONESIA", countryName) == 0) {
        Initialize(3);
        return 1;
    }
    if (STR_ICMP("HONGKONG", countryName) == 0) {
        Initialize(4);
        return 1;
    }
    return 0;
}

void cltCountrySystem::Free() { m_country = 0; }

int cltCountrySystem::IsKorea() { return m_country == 0; }
int cltCountrySystem::IsJapan() { return m_country == 1; }
int cltCountrySystem::IsTaiwan() { return m_country == 2; }
int cltCountrySystem::IsIndonesia() { return m_country == 3; }
int cltCountrySystem::IsHongKong() { return m_country == 4; }

int cltCountrySystem::GetCountry() { return m_country; }
