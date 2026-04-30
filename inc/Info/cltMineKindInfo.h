#pragma once
#include <cstdint>
#include "global.h"

// =============================================================================
// strMineKindInfo
//
// 함정스킬(地雷技能) KindInfo — 由 cltMineKindInfo::Initialize 從 mine.txt 解析。
// 反編譯來源：mofclient.c:315277-315498
//
// mine.txt（EUC-KR）每筆有 18 欄；其中第 2 欄 「지뢰 명」 (Mine Name) 在反編譯
// 中只 strtok 取出檢查非 null，並未存入 struct，所以下方 struct 沒有對應欄位。
//
// 反編譯固定以 56 bytes 為一筆（operator new(56 * count)），對應下列欄位排列：
// =============================================================================
struct strMineKindInfo
{
    // 0x00 — ID (M코드 → kind)
    //        TranslateKindCode("M0001") = ((toupper('M')+31)<<11) | atoi("0001")
    std::uint16_t kind;                 // 地雷種類 ID（5 字元 M 編碼壓縮成 16-bit）

    // 0x02 — 스킬 아이디 (Skill ID)
    //        TranslateKindCodeToSkill("A01641") = 0x8000 | atoi("01641")
    std::uint16_t skill;                // 對應的技能代碼（A=Active 0x8000, P=Passive 0x0000）

    // 0x04 — 공격력 (Attack Power)
    std::uint32_t attack;               // 攻擊力

    // 0x08 — 공격범위 (Attack Range)
    std::uint32_t attackRange;          // 攻擊範圍

    // 0x0C — 인식 범위 (Detection Range)
    std::uint32_t detectRange;          // 偵測／認識範圍

    // 0x10 — 봉쇄율(천분율) (Block Rate, per-mille)
    std::uint32_t blockRatePermille;    // 封鎖率（千分率）；解析時必須 < 1000

    // 0x14 — 봉쇄 지속 시간(밀리세컨드) (Block Duration ms)
    std::uint32_t blockDurationMs;      // 封鎖持續時間（毫秒）

    // 0x18 — 기절확률(천분율) (Stun Rate, per-mille)
    //        ※ 反編譯先寫 disappear (+0x28) 再回頭寫 stunRate；最終 layout 同此。
    std::uint32_t stunRatePermille;     // 暈眩機率（千分率）；解析時必須 < 1000

    // 0x1C — 기절 지속시간(밀리세컨드) (Stun Duration ms)
    std::uint32_t stunDurationMs;       // 暈眩持續時間（毫秒）

    // 0x20 — 빙결확률(천분율) (Freeze Rate, per-mille)
    std::uint32_t freezeRatePermille;   // 冰結機率（千分率）；解析時必須 < 1000

    // 0x24 — 빙결 지속시간(밀리세컨드) (Freeze Duration ms)
    std::uint32_t freezeDurationMs;     // 冰結持續時間（毫秒）

    // 0x28 — 마인 소멸시간 (Mine Disappear Time, ms)
    //        反編譯把第 9 欄（檔內第 8 個數字欄）寫到 +40，故位於這裡。
    std::uint32_t disappearMs;          // 地雷消滅時間（毫秒）

    // 0x2C — 탐색 방벙 공중/지상 (Detect target Air/Ground)
    //        檔案內為整數（IsDigit 檢查 + atoi），存成單 byte。
    std::uint8_t  airGround;            // 偵測對象（空中/地上）

    // 0x2D — 공격 타입 (Attack Type)
    //        "FIX" -> 1, "MOVING" -> 2；其他值 → 解析失敗。
    std::uint8_t  moveType;             // 攻擊型態（固定式 / 移動式）

    // 0x2E — 공격 숫자 (Attack Count)
    //        "ONE" -> 1, "MULTI" -> 2；其他值 → 解析失敗。
    std::uint8_t  attackCountType;      // 攻擊數量（單一 / 多個）

    // 0x2F — (alignment padding)
    //        反編譯未寫入此 byte；存在純粹是因為 +0x30 的 DWORD 對齊。
    std::uint8_t  _pad0;

    // 0x30 — 리소스ID (Resource ID, hex)
    //        以 IsAlphaNumeric 通過後，sscanf("%x") 寫入這 4 bytes。
    std::uint32_t resourceIdHex;        // 資源 ID（檔案中為十六進位字串）

    // 0x34 — 토탈플레임 (Total Flame)
    //        反編譯只做 atoi，不做 IsDigit 檢查（檔內為小整數）。
    std::uint16_t totalFlame;           // Total Flame（影格／效果張數，小整數）

    // 0x36-0x37 — 結構整體對齊 padding（讓 sizeof == 56）
};

static_assert(sizeof(strMineKindInfo) == 56,
              "strMineKindInfo must be exactly 56 bytes (matches mofclient.c)");

// =============================================================================
// cltMineKindInfo
//
// 反編譯佈局（mofclient.c:315266-315274 建構子）：
//   *(_DWORD *)this       = m_count   (筆數)
//   *((_DWORD *)this + 1) = m_list    (連續 56-byte 陣列指標)
// 結構大小 = 8 bytes，對外行為由下列方法實作。
// =============================================================================
class cltMineKindInfo
{
public:
    cltMineKindInfo();
    int  Initialize(const char* path);              // 成功回傳 1，否則 0
    void Free();

    strMineKindInfo* GetMineKindInfo(std::uint16_t code);

    // "M0001" → ((toupper('M')+31)<<11) | atoi("0001"); v >= 0x800 視為失敗回 0。
    static std::uint16_t TranslateKindCode(const char* s);
    // "A01641" / "Pxxxx"：'A' → 0x8000, 'P' → 0x0000；v >= 0x8000 視為失敗回 0。
    static std::uint16_t TranslateKindCodeToSkill(const char* s);

private:
    int              m_count;   // *(this+0) 反編譯：筆數
    strMineKindInfo* m_list;    // *(this+1) 反編譯：陣列起點
};
