#pragma once
#include <cstdint>
#include <cstddef>

// =============================================================================
// stTransportKindInfo  —  마판 자동차(교통수단) 정보 / 載具(交通工具) 資訊
// 對應檔案：text_dump/transportkindinfo.txt
// 由 clTransportKindInfo::Initialize() 解析；每筆 84 bytes（mofclient.c 339985）
// =============================================================================
//
// 反編譯佐證：
//   *(_WORD *)v7         = TranslateKindCode(token0)        // offset 0
//   *(_WORD *)(v7 +  2)  = atoi(token2)                     // offset 2
//   *(_WORD *)(v7 +  4)  = atoi(token3)                     // offset 4
//   strcpy(v7 +  6, token4)                                 // offset 6 (.txt up)
//   strcpy(v7 + 38, token5)                                 // offset 38 (.txt down)
//   sscanf(token6, "%x", v7 + 72)                           // offset 72 (uint32)
//   *(_WORD *)(v7 + 76)  = atoi(token7)                     // offset 76 (uint16)
//   sscanf(token8, "%x", v7 + 80)                           // offset 80 (uint32)
//
// 結構為自然對齊（無 #pragma pack）：strcpy 區塊以 1-byte 對齊，
// 但 70~71 與 78~79 是 4-byte 對齊插入的 padding。
// =============================================================================
struct stTransportKindInfo
{
    // 자동차 ID (TranslateKindCode 後的 16-bit code)
    // 자동차 ID  / 載具種類碼  /  offset 0
    std::uint16_t kindCode;

    // 자동차 이름(text code)  / 載具名稱（文字代碼）
    // 자동차 이름(text code)  / 名稱文字碼  /  offset 2
    std::uint16_t nameTextCode;

    // 자동차 설명(text code)  / 載具說明（文字代碼）
    // 자동차 설명(text code)  / 說明文字碼  /  offset 4
    std::uint16_t descTextCode;

    // 애니메이션 정보 파일(gi) 앞쪽  / 動畫資訊檔（前/上半身）
    // 애니메이션 정보 파일(gi) 앞쪽  / 動畫檔(上)  /  offset 6 .. 37
    char aniFileUp[32];

    // 애니메이션 정보 파일(gi) 뒤쪽  / 動畫資訊檔（後/下半身）
    // 애니메이션 정보 파일(gi) 뒤쪽  / 動畫檔(下)  /  offset 38 .. 69
    char aniFileDown[32];

    // 對齊 padding (offset 70..71)；自然對齊讓 iconResourceID 落在 4-byte 邊界。
    std::uint16_t _pad0;

    // 아이콘 리소스 ID (16進)  / 圖示資源 ID（hex）
    // 아이콘 리소스 ID  / 圖示資源 ID  /  offset 72
    std::uint32_t iconResourceID;

    // 아이콘 블럭 ID  / 圖示 block ID
    // 아이콘 블럭 ID  / 圖示區塊 ID  /  offset 76
    std::uint16_t iconBlockID;

    // 對齊 padding (offset 78..79)。
    std::uint16_t _pad1;

    // 자동차 이펙트 (16進)  / 載具特效 ID（hex）
    // 자동차 이펙트  / 載具特效 ID  /  offset 80
    std::uint32_t carEffectID;
};

static_assert(sizeof(stTransportKindInfo) == 84, "stTransportKindInfo must be 84 bytes (mofclient.c 339985)");
static_assert(offsetof(stTransportKindInfo, kindCode)        ==  0, "offset(kindCode)        != 0");
static_assert(offsetof(stTransportKindInfo, nameTextCode)    ==  2, "offset(nameTextCode)    != 2");
static_assert(offsetof(stTransportKindInfo, descTextCode)    ==  4, "offset(descTextCode)    != 4");
static_assert(offsetof(stTransportKindInfo, aniFileUp)       ==  6, "offset(aniFileUp)       != 6");
static_assert(offsetof(stTransportKindInfo, aniFileDown)     == 38, "offset(aniFileDown)     != 38");
static_assert(offsetof(stTransportKindInfo, iconResourceID)  == 72, "offset(iconResourceID)  != 72");
static_assert(offsetof(stTransportKindInfo, iconBlockID)     == 76, "offset(iconBlockID)     != 76");
static_assert(offsetof(stTransportKindInfo, carEffectID)     == 80, "offset(carEffectID)     != 80");


// =============================================================================
// clTransportKindInfo  —  載具種類資訊解析器
// mofclient.c：339958 (ctor) / 339985 (Initialize) / 340100 (GetTransportKindInfo)
//                340121 (TranslateKindCode)
// 大小 12 bytes：vftable(4) + m_pList(4) + m_nNum(4)
// =============================================================================
class clTransportKindInfo
{
public:
    clTransportKindInfo();
    virtual ~clTransportKindInfo();

    // 解析 TransportKindInfo.txt → 填入 m_pList / m_nNum
    // 成功回傳 1，失敗回傳 0；對齊 mofclient.c 339985 的回傳語意。
    int Initialize(char* filename);

    // 依 16-bit kindCode 線性搜尋；找不到回傳 nullptr。
    // 對齊 mofclient.c 340100：步進 42 個 _WORD（= 84 bytes / 筆）。
    stTransportKindInfo* GetTransportKindInfo(std::uint16_t transportKind);

    // "I1024" 等 5-char 字串 → 16-bit kindCode。
    // 對齊 mofclient.c 340121：(_toupper(c0)+31)<<11 | atoi(c1..c4)；
    // atoi 結果若 ≥ 0x800 則回傳 0。注意：(_toupper(c)+31)<<11 在數學上會
    // 溢位 16-bit，最終以 _WORD 截斷儲存（store 時自動只取低 16 bit）。
    static std::uint16_t TranslateKindCode(char* a1);

protected:
    // mofclient.c (32-bit GT)：*((_DWORD *)this + 1)  / GT offset 4
    // x64 build：vftable 為 8 bytes，本欄位實際 offset 為 8。Parser 全程使用
    // 成員存取（m_pList[i].field），byte arithmetic 僅在 entry 內部使用，所以
    // class layout 改變不影響行為。
    stTransportKindInfo* m_pList;

    // mofclient.c (32-bit GT)：*((_DWORD *)this + 2)  / GT offset 8
    int m_nNum;
};
