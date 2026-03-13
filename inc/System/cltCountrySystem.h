#pragma once

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
