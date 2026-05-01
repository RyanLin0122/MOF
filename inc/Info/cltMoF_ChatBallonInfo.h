#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)
// 每筆「角色對話氣泡」資訊（步距 = 18 WORD = 36 bytes；與反編譯一致）
// 對應 Character_ChatBallon.txt 欄位順序：
//   ChatBallon ID | 기획자이름(skip) | 이미지 ID |
//   LEFT_TOP | CENTER_TOP | RIGHT_TOP | LEFT_MIDDLE | RIGHT_MIDDLE |
//   CENTER_MIDDLE | LEFT_BOTTOM | RIGHT_BOTTOM |
//   CENTER_BOTTOM_L | CENTER_BOTTOM_C | CENTER_BOTTOM_R |
//   R(0~255) | G(0~255) | B(0~255)
struct strChatBallonInfo
{
    uint16_t ChatBallonKind;   // 韓: ChatBallon ID                中: 對話氣泡ID(B****→16位代碼)            offset: 0x00
    uint16_t _pad0;            // 韓: (예약, 미기록)                中: 保留(反編譯未寫入)                    offset: 0x02
    uint32_t ImageIDHex;       // 韓: 이미지 ID (%x)                中: 圖像ID(HEX 資源ID)                   offset: 0x04
    uint16_t Index[11];        // 韓: LEFT_TOP/CENTER_TOP/RIGHT_TOP/LEFT_MIDDLE/RIGHT_MIDDLE/CENTER_MIDDLE/LEFT_BOTTOM/RIGHT_BOTTOM/CENTER_BOTTOM_L/CENTER_BOTTOM_C/CENTER_BOTTOM_R
                               // 中: 11 個區塊影格索引(依上列順序)                                          offset: 0x08~0x1D
    uint8_t  R;                // 韓: R(0~255)                      中: 字色 R                                 offset: 0x1E
    uint8_t  G;                // 韓: G(0~255)                      中: 字色 G                                 offset: 0x1F
    uint8_t  B;                // 韓: B(0~255)                      中: 字色 B                                 offset: 0x20
    uint8_t  _pad1[3];         // 韓: (정렬용 패딩)                  中: 對齊填充至 36 bytes                    offset: 0x21~0x23
};
static_assert(sizeof(strChatBallonInfo) == 36, "strChatBallonInfo size must be 36 bytes");
#pragma pack(pop)

class cltMoF_ChatBallonInfo
{
public:
    cltMoF_ChatBallonInfo() : count_(0), table_(nullptr) {}
    virtual ~cltMoF_ChatBallonInfo() { Free(); } // 反編譯具有 vtable，這裡以虛析構模擬

    // 載入 Character_ChatBallon.txt；成功回傳 1，失敗回傳 0
    int Initialize(char* filename);

    // 釋放配置的表
    void Free();

    // 依 ChatBallonKind（B**** 轉碼後）取得記錄；找不到回傳 nullptr
    strChatBallonInfo* GetChatBallonInfoByKind(uint16_t kind);

    // 讀取後的陣列與筆數（除錯用）
    const strChatBallonInfo* data() const { return table_; }
    uint16_t count() const { return count_; }

private:
    // 反編譯中呼叫 cltItemKindInfo::TranslateKindCode；以同演算法提供等價實作
    static uint16_t TranslateItemKindCode(const char* s) {
        if (!s || std::strlen(s) != 5) return 0;
        int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
        int lo = std::atoi(s + 1);
        if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
        return 0;
    }

private:
    uint16_t            count_;   // 筆數（WORD）
    strChatBallonInfo* table_;   // 36 * count_ bytes
};