#pragma once
#include <cstdint>
#include "global.h"

struct strMineKindInfo
{
    // 0x00
    std::uint16_t kind;                 // M代碼，TranslateKindCode("M0001")
    // 0x02
    std::uint16_t skill;                // 技能代碼，TranslateKindCodeToSkill("A01641")
    // 0x04
    std::uint32_t attack;               // 攻擊力
    // 0x08
    std::uint32_t attackRange;          // 攻擊範圍
    // 0x0C
    std::uint32_t detectRange;          // 認識/偵測範圍
    // 0x10
    std::uint32_t blockRatePermille;    // 封鎖率(千分率)  < 1000
    // 0x14
    std::uint32_t blockDurationMs;      // 封鎖持續時間(ms)
    // 0x18
    std::uint32_t stunRatePermille;     // 暈眩機率(千分率) < 1000
    // 0x1C
    std::uint32_t stunDurationMs;       // 暈眩時間(ms)
    // 0x20
    std::uint32_t freezeRatePermille;   // 冰結機率(千分率) < 1000
    // 0x24
    std::uint32_t freezeDurationMs;     // 冰結時間(ms)
    // 0x28
    std::uint32_t disappearMs;          // 地雷消滅時間(ms)
    // 0x2C
    std::uint8_t  airGround;            // 探測版本(空中/地上)，檔案為數字
    // 0x2D
    std::uint8_t  moveType;             // "FIX"=1, "MOVING"=2
    // 0x2E
    std::uint8_t  attackCountType;      // "ONE"=1, "MULTI"=2
    // 0x2F (保留對齊，對應反編譯中未用到的位元)
    std::uint8_t  _pad0;
    // 0x30
    std::uint32_t resourceIdHex;        // 16 進位字串以 %x 讀入
    // 0x34
    std::uint16_t totalFlame;           // 檔案最後一欄（小整數）

    // 結構大小對齊到 56 bytes（0x38）
};

class cltMineKindInfo
{
public:
    cltMineKindInfo();
    int  Initialize(const char* path);         // 讀檔成功回傳 1，否則 0（比照反編譯）
    void Free();

    strMineKindInfo* GetMineKindInfo(std::uint16_t code);

    static std::uint16_t TranslateKindCode(const char* s);        // "M0001" → 5 碼
    static std::uint16_t TranslateKindCodeToSkill(const char* s); // "A01641"/"Pxxxx" → 6 碼

private:
    int              m_count;   // 總筆數（反編譯：*(_DWORD*)this）
    strMineKindInfo* m_list;    // 陣列指標（反編譯：*((_DWORD*)this+1)）
};
