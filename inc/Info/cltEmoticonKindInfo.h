#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// ----------------------------------------------------------------------------
// stEmoticonWordInfo  (188 bytes, packed)
//   反編譯位置：mofclient.c:299191（Initialize）寫入；299339（InitEmoticonItem）qmemcpy(.., 0xBCu)
//   韓文標頭：「설명 / 이모티콘 이름 / 이모티콘 종류 / 아이템 아이디 / 대표 아이템 아이디 /
//              리소스 ID / 블록 ID / 툴팁 ID / 표현 낱말 리스트」
//   中文用途：每一列「表情」資料；descript 欄會被當成 strtok 第一個 token 直接丟棄。
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct stEmoticonWordInfo {
    uint16_t nameId;        // +0    이모티콘 이름        表情名稱 ID（atoi）
    uint16_t repItemKind;   // +2    대표 아이템 아이디  「群組鍵」道具碼；"0"→0，否則 TranslateKindCode
    uint16_t itemKind;      // +4    아이템 아이디        本列道具碼；"0"→0，否則 TranslateKindCode
    uint16_t _padA;         // +6    (對齊；反編譯從未存取)
    uint32_t kind;          // +8    이모티콘 종류        表情種類（atoi）— GetEmoticonItemInfoByKind 比對欄
    uint32_t resIdHex;      // +12   리소스 ID            資源 ID（sscanf "%x"；含 '+' 視為 parse error）
    uint16_t blockId;       // +16   블록 ID              block ID（atoi）
    uint16_t tooltipId;     // +18   툴팁 ID              tooltip 文字 ID（atoi）
    uint16_t wordsTextId;   // +20   표현 낱말 리스트     詞列 MoFTexts ID（atoi；==0 視為 parse error）
    char     word0[16];     // +22   表現詞 #1            DCTTextManager.GetText(wordsTextId) 以 '/' 切分後的詞
    char     word1[16];     // +38   表現詞 #2
    char     word2[16];     // +54   表現詞 #3
    char     word3[16];     // +70   表現詞 #4
    char     word4[16];     // +86   表現詞 #5
    char     word5[16];     // +102  表現詞 #6
    char     word6[16];     // +118  表現詞 #7
    char     word7[16];     // +134  表現詞 #8
    char     word8[16];     // +150  表現詞 #9
    char     word9[16];     // +166  表現詞 #10
    uint16_t _padB;         // +182  (對齊；反編譯從未存取)
    uint32_t wordCount;     // +184  유효 낱말 수          有效詞數（包含 word0；GT 在 >10 詞時會位移寫入此欄）
};
#pragma pack(pop)

// ----------------------------------------------------------------------------
// stEmoticonItemInfo  (984 bytes, packed)
//   反編譯位置：mofclient.c:299401  v9 = operator new(984 * (v6 + 1));
//   構成：頭部 44 bytes（idList[20] + repItemKind + _pad）+ 五個 stEmoticonWordInfo（5 × 188 = 940）
//   韓文意義：「대표 아이템 아이디」相同的 5 個表情（slot[0..4]）+ 此群組所擁有的道具 ID 清單（idList[20]）
//
//   InitEmoticonItem 三段流程（嚴格反編譯，注意 Phase 2 / Phase 3 的累計副作用）：
//
//     Phase 0：依 m_rows[i].repItemKind 的「變更次數」估出 m_itemCnt = changes + 1
//     Phase 1：把所有 repItemKind==0 的列依序 memcpy 到 m_items + copiedZero*188 + 44，
//              連續 5 列正好填滿 m_items[0] 的 slot[0..4]（典型「rep==0 的 5 個表情」）
//     Phase 2：對每個 g >= 1，掃描 baseIndex..rc-1 的所有列，把 row.repItemKind 寫進
//              m_items[g] +40。因為迴圈無 break，所有 g >= 1 的 +40 最終都會等於
//              m_rows[m_rowCnt-1].repItemKind（GT 反編譯之既有行為，故意保留）。
//     Phase 3：對每個 g >= 1，從 baseIndex 起逐列掃描；當 row.repItemKind == m_items[g]+40 時：
//                (a) 把 row.itemKind 寫到 m_items[g].idList[writeCountTotal] WORD 處
//                    — writeCountTotal 為 v34，跨群組累計，整個 InitEmoticonItem 只在進入第三段前歸 0；
//                (b) 連續拷 5 列（row..row+4，每筆 188 bytes）到 m_items[g].slot[0..4]，
//                    後續若同群組再次匹配會覆寫這 5 個 slot（GT 反編譯既有行為，故意保留）；
//                (c) r += 5（跳過已處理的 5 列）。
// ----------------------------------------------------------------------------
#pragma pack(push, 1)
struct stEmoticonItemInfo {
    uint16_t idList[20];         // +0    이 그룹이 보유한 아이템 ID 목록（最多 20 個）
                                 //       — GetEmoticonItemInfoByID 比對的欄位
    uint16_t repItemKind;        // +40   대표 아이템 아이디（群組鍵；Phase 2 / Phase 3 的比對基準）
    uint16_t _pad;               // +42   (對齊；反編譯從未存取)
    stEmoticonWordInfo slot[5];  // +44   슬롯 0..4：5 筆 188 bytes（每筆代表一個表情）
};
#pragma pack(pop)

static_assert(sizeof(stEmoticonWordInfo) == 188, "stEmoticonWordInfo size must be 188 bytes");
static_assert(sizeof(stEmoticonItemInfo) == 984, "stEmoticonItemInfo size must be 984 bytes");

// ----------------------------------------------------------------------------
// cltEmoticonKindInfo  — 反編譯成員配置（mofclient.c:299162 / 299647）
//   *(this + 0)   : vftable
//   *(this + 4)   : m_rows   (stEmoticonWordInfo*  — 188B/列 連續緩衝)
//   *(this + 8)   : m_rowCnt (int                  — 原始列數，由 fgets 預掃決定)
//   *(this + 12)  : m_items  (stEmoticonItemInfo* — 984B/組 連續緩衝)
//   *(this + 16)  : m_itemCnt(int                  — 群組數)
// ----------------------------------------------------------------------------
class cltEmoticonKindInfo {
public:
    cltEmoticonKindInfo();
    virtual ~cltEmoticonKindInfo();

    // 解析 EmoticonList.txt（跳過前三行表頭）
    int  Initialize(char* filename);

    // 釋放 m_rows / m_items（不影響 m_rowCnt / m_itemCnt）
    void Free();

    // ---- 查詢介面（對應 mofclient.c:299505..299636） ----

    // 以「組索引（0-based）」取群組頁面（m_items[idx]）
    stEmoticonItemInfo* GetEmoticonItemInfoByIndex(uint8_t idx);

    // 以 row.kind（DWORD）逐組逐 slot 比對，回傳第一個命中的群組
    stEmoticonItemInfo* GetEmoticonItemInfoByKind(int kind);

    // 以「轉碼後的 itemId」掃 idList[0..19]，回傳擁有此 ID 的群組；id==0 → m_items[0]
    stEmoticonItemInfo* GetEmoticonItemInfoByID(uint16_t itemIdCode);

    // 以 row.kind 取得「該 slot 188B 結構」（對應 mofclient.c:299598）
    stEmoticonWordInfo* GetEmoticonWordInfoByKind(int kind);

    // 群組數（反編譯：return *((this)+4)）
    int  GetEmoticonItemCnt();

    // idList 中是否存在指定的 itemIdCode
    int  IsEmoticonItem(uint16_t itemIdCode);

    // 與其它 *KindInfo 相同的 5-char 轉碼：(toupper(c0)+31)<<11 | atoi(c1..c4)
    uint16_t TranslateKindCode(char* s);

private:
    // Phase 0..3：把「188B 列陣列」打包成「984B 組陣列」（mofclient.c:299339）
    int InitEmoticonItem();

private:
    stEmoticonWordInfo* m_rows = nullptr;   // +4   188B 連續緩衝（每列一筆 stEmoticonWordInfo）
    int                 m_rowCnt = 0;       // +8   m_rows 的列數
    stEmoticonItemInfo* m_items = nullptr;  // +12  984B 連續緩衝（每組一筆 stEmoticonItemInfo）
    int                 m_itemCnt = 0;      // +16  m_items 的群組數
};
