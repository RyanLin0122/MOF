#ifndef CLT_SEX_SYSTEM_H
#define CLT_SEX_SYSTEM_H

#include <cstdint>

typedef int BOOL;

class cltSexSystem
{
public:
    cltSexSystem();

    void Initialize(uint8_t a2);
    BOOL IsMale();
    BOOL IsFemale();
    void SetSex_male();
    void SetSex_female();
    uint8_t GetSex();

private:
    uint8_t mSex;
};

#endif // CLT_SEX_SYSTEM_H