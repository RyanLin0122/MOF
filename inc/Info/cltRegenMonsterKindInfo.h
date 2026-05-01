#pragma once
#include <cstdint>
#include <cstdio>
#include "Info/cltCharKindInfo.h"
#include "Info/cltMapInfo.h"
#include "global.h"

// ===========================================================================
//  cltRegenMonsterKindInfo  ── 對應檔案：RegenMonsterInfo.txt
//
//  맵 상 몬스터 정보 (Field-Map Regen Monster Info)
//  / 地圖怪物重生資訊 (記載每一塊地圖區域的怪物重生種類與生成框)
//
//  反編譯位置：mofclient.c:329239-329494
//      Initialize / Free / GetRegenMonsterNum / GetRegenMonsterKindInfoByIndex
//      / TranslateKindCode / GetRegenMapIDByMonsterKind / GetRegenMonsterKindByMapID
//
//  text 檔欄位：// 종류 : 맵 상 몬스터 정보 // 버전 : 19.8 // 작성일 : 20090817
//      ID  이름(기획자용)  종류ID  맵 이름(기획자용)  맵ID  x  y  width  height
//      최대수  리젠 간격(초)
// ===========================================================================

// ---------------------------------------------------------------------------
//  strRegenMonsterKindInfo ── 한 행 데이터 / 單行資料 (32 bytes)
//
//  반편역 (mofclient.c:329275-329320)：每行 32 bytes，由 operator new 連續配置。
//  寫入順序與位移逐字對齊反編譯：
//     v5 += 32; *(v5 - 4) = 1000 * atoi(...)  ⇒ 上一筆 +28 是最後寫入。
//
//  注意：'이름(기획자용)' 與 '맵 이름(기획자용)' 兩欄解析時直接 strtok 丟棄，
//        不存於本結構中。
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct strRegenMonsterKindInfo {
    // ----- col 0 --------------------------------------------------------
    // 韓: ID                      中: 主鍵 ID (Axxxx)
    // 解: TranslateKindCode → +0 (WORD)；翻譯為 0 即中止解析
    uint16_t kind;        // +0   (offset 0x00)

    // ----- col 2 --------------------------------------------------------
    // 韓: 종류 ID                 中: 怪物種類碼 (Jxxxx)
    // 解: cltCharKindInfo::TranslateKindCode → +2 (WORD)
    uint16_t charKind;    // +2   (offset 0x02)

    // ----- col 4 --------------------------------------------------------
    // 韓: 맵 ID                   中: 地圖 ID (Mxxxx)
    // 解: cltMapInfo::TranslateKindCode → +4 (WORD)
    uint16_t mapID;       // +4   (offset 0x04)

    // ----- pad ----------------------------------------------------------
    // 韓: 패딩                    中: 對齊填充 (永遠為 0)
    // 解: 反編譯不寫入此 2 bytes (operator new 後未 memset)，內容未定義
    uint16_t _pad;        // +6   (offset 0x06)

    // ----- col 5 --------------------------------------------------------
    // 韓: x                       中: 矩形左上 X
    // 解: atoi → +8 (DWORD)
    int32_t  x1;          // +8   (offset 0x08)

    // ----- col 6 --------------------------------------------------------
    // 韓: y                       中: 矩形左上 Y
    // 解: atoi → +12 (DWORD)
    int32_t  y1;          // +12  (offset 0x0C)

    // ----- col 7 --------------------------------------------------------
    // 韓: width                   中: 矩形右邊 X (= x + width)
    // 解: atoi(width) + x1 → +16 (DWORD)
    //     ※ 反編譯實際寫入 *(v5+16) = *(v5+8) + atoi(token)，故此欄為「右邊界絕對座標」。
    int32_t  x2;          // +16  (offset 0x10)

    // ----- col 8 --------------------------------------------------------
    // 韓: height                  中: 矩形下邊 Y (= y + height)
    // 解: atoi(height) + y1 → +20 (DWORD)
    //     ※ 同上，存的是「下邊界絕對座標」。
    int32_t  y2;          // +20  (offset 0x14)

    // ----- col 9 --------------------------------------------------------
    // 韓: 최대수                  中: 最大同時存在數
    // 解: atoi → +24 (DWORD)
    int32_t  maxCount;    // +24  (offset 0x18)

    // ----- col 10 -------------------------------------------------------
    // 韓: 리젠 간격(초)           中: 重生間隔 (毫秒；存檔為秒)
    // 解: 1000 * atoi → +28 (DWORD)
    //     ※ 反編譯：v5 += 32; *(v5 - 4) = 1000 * atoi(token)。
    int32_t  intervalMs;  // +28  (offset 0x1C)
};
#pragma pack(pop)
static_assert(sizeof(strRegenMonsterKindInfo) == 32,
    "strRegenMonsterKindInfo must be 32 bytes");

// ---------------------------------------------------------------------------
//  cltRegenMonsterKindInfo ── 載入器 / 載入容器類別
//
//  반편역 데이터 멤버:
//    *(_DWORD *)this        ── m_items（連續陣列起點，operator new 配置；
//                                每筆 32 bytes，總長度 = 32 * m_count）
//    *((_DWORD *)this + 1)  ── m_count（== 預掃資料行數）
//
//  반편역 메서드:
//    Initialize                 ── mofclient.c:329239
//    Free                       ── mofclient.c:329336
//    GetRegenMonsterNum         ── mofclient.c:329347
//    GetRegenMonsterKindInfoByIndex ── mofclient.c:329353
//    TranslateKindCode (static) ── mofclient.c:329365 (與 CharKindInfo 同 body)
//    GetRegenMapIDByMonsterKind ── mofclient.c:329383
//    GetRegenMonsterKindByMapID ── mofclient.c:329431
//
//  ※ Get* 兩個 lookup 方法將「結果 buffer」直接寫在 this 物件的尾端：
//      this+8        : 1 個 WORD     ← GetRegenMapIDByMonsterKind 寫入點
//      this+10..+18  : 5 個 WORD     ← GetRegenMonsterKindByMapID 寫入點
//      this+18..+20  : 1 個 WORD 0   ← 其零終止
//     兩個 buffer 不重疊；呼叫者 (CToolTip) 直接以該指標作迭代。
// ---------------------------------------------------------------------------
class cltRegenMonsterKindInfo {
public:
    cltRegenMonsterKindInfo() : m_items(nullptr), m_count(0) {
        m_mapIDOut = 0;
        for (int i = 0; i < 6; ++i) m_kindsOut[i] = 0;
    }
    ~cltRegenMonsterKindInfo() { Free(); }

    // 반편역：int __thiscall Initialize(this, char* filename)
    int  Initialize(char* filename);

    // 반편역：void __thiscall Free(this)
    void Free();

    // 반편역：int __thiscall GetRegenMonsterNum(this)
    //   ※ 回傳預掃行數，非實際成功解析筆數。
    int  GetRegenMonsterNum() const { return m_count; }

    // 반편역：str* __thiscall GetRegenMonsterKindInfoByIndex(this, int idx)
    strRegenMonsterKindInfo* GetRegenMonsterKindInfoByIndex(int idx);

    // 반편역：static uint16_t __cdecl TranslateKindCode(char* s)
    //   장 5 글자, 첫 글자 알파벳 + 4 자리 십진수, atoi(s+1) < 0x800.
    static uint16_t TranslateKindCode(char* s);

    // 반편역：char* __thiscall GetRegenMapIDByMonsterKind(this, uint16_t monKind)
    //   回傳指向 this+8 的 1 槽 buffer (uint16_t)；同種怪只記第一張地圖。
    uint16_t* GetRegenMapIDByMonsterKind(uint16_t monsterKind);

    // 반편역：uint16_t* __thiscall GetRegenMonsterKindByMapID(this, uint16_t mapID)
    //   回傳指向 this+10 的 5 槽 buffer (uint16_t)；以 cltCharKindInfo
    //   nameTextCode (+2) 去重，寫入 monsterRegistryKind (+6)。
    //   尾端有零終止 (this+18..+20)。
    uint16_t* GetRegenMonsterKindByMapID(uint16_t mapID);

    inline const strRegenMonsterKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strRegenMonsterKindInfo* m_items;     // +0  연속 배열，32 bytes/筆
    int                      m_count;     // +4  파일 자료 행 수
    uint16_t                 m_mapIDOut;  // +8  GetRegenMapIDByMonsterKind 結果緩衝 (1 槽)
    uint16_t                 m_kindsOut[6]; // +10..+22  GetRegenMonsterKindByMapID 結果緩衝
                                            //          (5 槽 + 1 槽零終止；this+18..+20 為零)
};
