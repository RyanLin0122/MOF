#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// =============================================================================
// cltPKRankKindInfo  ── PK 계급 정보 (PK 階級資料)
// 對應反編譯：mofclient.c:322889-323061
// 來源資料：text_dump/pkrank.txt（EUC-KR 編碼，3 行表頭 + N 筆資料）
//
// 表頭欄位（韓文 / 中文 / 用途）：
//   [1] 계급 ID            階級 ID            "R0001"…透過 TranslateKindCode 轉 16-bit
//   [2] 계급                階級（純數字）     反編譯內 strtok 取得後直接丟棄
//   [3] 계급명 텍스트 ID    階級名稱文字 ID    對應 MoFTexts 的 WORD 索引
//   [4] 포인트              所需點數           達到此階級所需戰績點數
//   [5] 계급장 리소스       階級徽章資源       8 位 16 進位 (sscanf "%x")
//   [6] 계급장 블록 ID      階級徽章區塊 ID    UI Atlas 區塊
//   [7] 패배시 감소 포인트  敗北時扣減點數     可為負，PK 失敗時扣
//   [8] 전공 포인트 지급    戰功點數給予       PK 勝利時的戰功獎勵
//   [9] 공적 포인트 지급(X) 公績點數給予(X)    舊版欄位（廢棄但仍解析）
// =============================================================================

// 由反編譯還原的記錄結構（每筆固定 28 bytes，反編譯以指針 +28 步進、+offset 直接寫值）
#pragma pack(push, 1)
struct strPKRankKindInfo {
    uint16_t kind;        // +0  (WORD)  계급 ID            階級 ID（TranslateKindCode 結果）
    uint16_t textId;      // +2  (WORD)  계급명 텍스트 ID  名稱文字 ID（atoi）
    int32_t  needPoint;   // +4  (DWORD) 포인트            所需點數（atoi，可大值）
    uint32_t resIdHex;    // +8  (DWORD) 계급장 리소스    徽章資源 ID（sscanf "%x"）
    uint16_t blockId;     // +12 (WORD)  계급장 블록 ID    徽章 Atlas Block（atoi）
    uint16_t _pad;        // +14 (WORD)  ── 反編譯未寫入；memset(0) 後保持 0
    int32_t  losePenalty; // +16 (DWORD) 패배시 감소 포인트  敗北扣分（atoi，可為負）
    int32_t  meritPoint;  // +20 (DWORD) 전공 포인트 지급   戰功給予（atoi）
    int32_t  publicPoint; // +24 (DWORD) 공적 포인트 지급   公績給予 X（atoi）
};                         // sizeof == 28
#pragma pack(pop)
static_assert(sizeof(strPKRankKindInfo) == 28, "strPKRankKindInfo must be 28 bytes");

// 反編譯類別佈局（this 指向 8 bytes：m_items 指標 + m_count）
//   this[0] (DWORD): strPKRankKindInfo*  m_items   ── 連續陣列
//   this[1] (DWORD): int                 m_count   ── 陣列筆數
class cltPKRankKindInfo {
public:
    cltPKRankKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltPKRankKindInfo() { Free(); }

    // 對應反編譯：int __thiscall Initialize(char* filename)（mofclient.c:322889）
    int Initialize(char* filename);

    // 對應反編譯：void __thiscall Free()（mofclient.c:322991）
    void Free();

    // 對應反編譯：strPKRankKindInfo* GetPKRankKindInfo(WORD code)（mofclient.c:323002）
    strPKRankKindInfo* GetPKRankKindInfo(uint16_t code);

    // 對應反編譯：WORD GetPKRankKindByPoint(int point)（mofclient.c:323023）
    uint16_t GetPKRankKindByPoint(int point);

    // 對應反編譯：static WORD TranslateKindCode(char* s)（mofclient.c:323046）
    //   '長度必須為 5；result = ((toupper(s[0])+31) << 11) | atoi(s+1)，atoi 須 < 0x800'
    static uint16_t TranslateKindCode(char* s);

    // 便利存取（非反編譯介面）
    inline const strPKRankKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strPKRankKindInfo* m_items; // this[0]：連續記憶體，每筆 28 bytes
    int                m_count; // this[1]：總筆數
};
