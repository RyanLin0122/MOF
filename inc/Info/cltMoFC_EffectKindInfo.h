#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <new>
#include "global.h"

//---------------------------------------------------------------------
// stEffectKindInfo
//   每筆「스킬 이펙트(技能特效) 종류 정보(種別資訊)」節點。
//   GT (mofclient.c:259619) 由 `operator new(0x84u)` 配置，總長 0x84 = 132 bytes
//   後立即 `memset 0`，所以未初始化欄位皆為 0。
//
// 偏移表（offsets）：
//   +0x00  uint16_t  kindCode      종류 코드 / 種別碼
//                                   GT: TranslateKindCode("Exxxx") 之 16-bit 結果
//                                   寫入路徑：mofclient.c:259623
//                                       *(_WORD *)*v9 = v7;
//   +0x02  char      eaFile[128]   EA 파일명 / EA 檔案名稱（含末端 NUL）
//                                   寫入路徑：mofclient.c:259629
//                                       strcpy(v11 + 2, v12);
//                                   讀取點：CEAManager::GetEAData(...,(char *)v + 2)
//                                          (mofclient.c:255395 等)
//   +0x82  uint8_t   skillType     스킬 타입 / 技能類型
//                                   1 = ONCE         (한번 / 單次)
//                                   2 = DIRECT       (지향형 / 指向型)
//                                   3 = SUSTAIN      (지속형 / 持續型)
//                                   4 = SHOOTUNIT    (단위 발사 / 單位投射)
//                                   5 = SHOOTNOTEA   (EA 미사용 발사 / 無 EA 投射)
//                                   6 = ITEMONCE     (아이템 한번 / 道具單次)
//                                   0 = 그 외 / 其他（GT 不終止解析、保持 0）
//                                   寫入路徑：mofclient.c:259634-259657
//                                       v11[130] = N;
//   +0x83  uint8_t   _pad          정렬용 패딩 / 對齊填充（恆為 0）
//---------------------------------------------------------------------
#pragma pack(push, 1)
struct stEffectKindInfo {
    uint16_t kindCode;      // +0x00 (0)   16-bit 종류 코드 / 種別碼
    char     eaFile[128];   // +0x02 (2)   EA 파일명 / EA 檔案名稱
    uint8_t  skillType;     // +0x82 (130) 스킬 타입 / 技能類型 (1..6, 其他=0)
    uint8_t  _pad;          // +0x83 (131) 정렬 패딩 / 對齊填充
};
#pragma pack(pop)

static_assert(sizeof(stEffectKindInfo) == 0x84,
              "stEffectKindInfo size must be 0x84 (132) bytes");
static_assert(offsetof(stEffectKindInfo, kindCode)  == 0x00, "kindCode offset");
static_assert(offsetof(stEffectKindInfo, eaFile)    == 0x02, "eaFile offset");
static_assert(offsetof(stEffectKindInfo, skillType) == 0x82, "skillType offset");
static_assert(offsetof(stEffectKindInfo, _pad)      == 0x83, "_pad offset");


//---------------------------------------------------------------------
// cltMoFC_EffectKindInfo
//   「스킬 이펙트 종류 정보」(技能特效種別資訊) 表的全域單例。
//   全域實例：g_clEffectKindInfo  (mofclient.c)
//
// 反編譯記憶體佈局 (32-bit MSVC, mofclient.c:259539-259544)：
//   +0x00000  void*               vftable
//   +0x00004  stEffectKindInfo*   table[0xFFFF]   ; 65535 個指標槽 (4 bytes 各)
//   total memset 區域 = 0x3FFFC bytes (= 65535 * 4 bytes，即整張 table)
//
// 此處還原使用獨立 `table_[]` 陣列；行為等價（記憶體絕對位移因 32→64 bit
// 改變，唯邏輯/觀察行為相同）。
//
// 由 g_clEffectKindInfo 主呼叫位置：
//   - mofclient.c:209613  cltMoFC_EffectKindInfo::Initialize("SkillEffect.txt")
//   - 多處 GetEffectKindInfo (例 31442, 35450, 195734, 255388, 258793...)
//---------------------------------------------------------------------
class cltMoFC_EffectKindInfo {
public:
    cltMoFC_EffectKindInfo();

    // GT (mofclient.c:259557): 唯一動作為寫回 vftable，未釋放 table_ 中任一節點。
    // 此處保持「不釋放」以維持與 GT 相同的記憶體生命週期語意。
    virtual ~cltMoFC_EffectKindInfo();

    // SkillEffect.txt (EUC-KR) 載入。
    // 回傳值（mirrors v19 in GT, mofclient.c:259596）：
    //   1 — 開檔成功且：
    //       (a) 三行 header 後第 4 次 fgets 為 NULL（無資料列），或
    //       (b) 至少一列成功解析後再次 fgets 為 NULL（乾淨 EOF）。
    //   0 — 開檔失敗、header 不足、TranslateKindCode 失敗、重複種別碼，
    //       或欄位 strtok 中途回 NULL。
    int Initialize(char* fileName);

    // GT mofclient.c:259682：直接以 (this+1+code) 索引取回 table 指標。
    stEffectKindInfo* GetEffectKindInfo(uint16_t code);

    // GT mofclient.c:259688：先 TranslateKindCode 再走 16-bit 路徑。
    stEffectKindInfo* GetEffectKindInfo(char* codeStr);

    // GT mofclient.c:259697：
    //   if (strlen(s) != 5) return 0;
    //   v3 = (toupper(s[0]) + 31) << 11;          // 32-bit 計算
    //   v4 = (uint16_t)atoi(s + 1);                // 結果先 truncate 至 16-bit
    //   if (v4 < 0x800) result = (uint16_t)(v3|v4); else result = 0;
    //   return (uint16_t)result;                   // 高 16-bit 被截斷
    // 對應字母群組（低 16-bit 值）：
    //   'A'→0x0000  'B'→0x0800  'C'→0x1000  'D'→0x1800  'E'→0x2000
    //   'F'→0x2800  'G'→0x3000  ...          'Z'→0x9800
    uint16_t TranslateKindCode(char* codeStr);

private:
    // table_[i] 對應到 GT 的 *((_DWORD*)this + i + 1)。
    // 索引值即 TranslateKindCode 的 16-bit 結果，1..0xFFFE 為有效範圍
    // （0 不會被寫入：GT 在 `if (!v6) break;` 處中止）。
    static constexpr size_t TABLE_SIZE = 0xFFFF;  // 65535 槽，與 GT 相同
    stEffectKindInfo* table_[TABLE_SIZE];
};
