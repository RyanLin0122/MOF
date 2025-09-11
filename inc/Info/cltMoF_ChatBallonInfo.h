#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)
// 每筆「角色對話氣泡」資訊（步距 = 18 WORD = 36 bytes；與反編譯一致）
struct strChatBallonInfo
{
    // 0x00
    uint16_t ChatBallonKind;     // ChatBallon ID（B**** 5碼→16位代碼）

    // 0x02（保留：反編譯未寫入）
    uint16_t _pad0;

    // 0x04
    uint32_t ImageIDHex;         // 이미지 ID（十六進位 %x）// 圖像ID(HEX)

    // 0x08 ~ 0x1D（共 11 個 WORD）
    // 依序：LEFT_TOP, CENTER_TOP, RIGHT_TOP, LEFT_MIDDLE, RIGHT_MIDDLE,
    // CENTER_MIDDLE, LEFT_BOTTOM, RIGHT_BOTTOM, CENTER_BOTTOM_L, CENTER_BOTTOM_C, CENTER_BOTTOM_R
    uint16_t Index[11];          // 區塊索引 // 對應各部位影格索引

    // 0x1E ~ 0x20：RGB（1 byte 各自）
    uint8_t  R;                  // R(0~255) // 字色R
    uint8_t  G;                  // G(0~255) // 字色G
    uint8_t  B;                  // B(0~255) // 字色B

    // 0x21 ~ 0x23（保留，對齊到 36 bytes）
    uint8_t  _pad1[3];
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