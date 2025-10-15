#ifndef CLT_SEX_SYSTEM_H
#define CLT_SEX_SYSTEM_H

#include <cstdint>

typedef int BOOL; // 對齊反編譯中的 BOOL（非必要不引入 windows.h）

class cltSexSystem
{
public:
    // 00590880：建構子（僅回傳 this，不做初始化）
    cltSexSystem();

    // 005908A0：Initialize(this, a2) -> *this = a2
    void Initialize(uint8_t a2);

    // 005908B0：IsMale -> toupper(*this) == 'M'(77)
    BOOL IsMale();

    // 005908D0：IsFemale -> toupper(*this) == 'F'(70)
    BOOL IsFemale();

    // 005908F0：SetSex_male -> *this = 'M'(77)
    void SetSex_male();

    // 00590900：SetSex_female -> *this = 'F'(70)
    void SetSex_female();

    // 00590910：GetSex -> 回傳首位元組
    uint8_t GetSex();

private:
    // 物件首位元組即性別字元（ASCII 'M'/'F' 或小寫）
    uint8_t mSex; // 對應 *(_BYTE*)this
};

#endif // CLT_SEX_SYSTEM_H
