#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

#pragma pack(push, 1)
// 單筆名稱標籤資訊（依反編譯配置：10 個 WORD = 20 bytes）
struct strNameTagInfo
{
    // [0] NameTag ID（5碼 → 16-bit 代碼）
    uint16_t NameTagKind;            // "NameTag ID"（N****）

    // [1] 保留（反編譯未寫入，僅佔位）
    uint16_t _pad1;

    // [2..3] 이미지 ID（HEX，%x 以 32-bit 寫入）
    uint32_t ImageIDHex;             // 圖像ID(十六進位)

    // [4] 이미지 블록ID
    uint16_t ImageBlockID;           // 圖像Block ID

    // [5] 保留（反編譯未寫入）
    uint16_t _pad2;

    // [6..7] PC방 사용자 이미지 ID（HEX，%x 以 32-bit 寫入）
    uint32_t PCImageIDHex;           // PC房用圖像ID(十六進位)

    // [8] PC방 사용자 블록ID
    uint16_t PCImageBlockID;         // PC房用圖像Block ID

    // [9] 保留（反編譯未寫入）
    uint16_t _pad3;
};
static_assert(sizeof(strNameTagInfo) == 20, "strNameTagInfo must be 20 bytes");
#pragma pack(pop)

class cltMoF_NameTagInfo
{
public:
    cltMoF_NameTagInfo() : count_(0), table_(nullptr) {}
    virtual ~cltMoF_NameTagInfo() { Free(); }  // 反編譯類別具 vtable，這裡以虛析構模擬

    // 讀取 character_nametag.txt；成功回傳 1，失敗回傳 0
    int Initialize(char* filename);

    // 釋放配置
    void Free();

    // 依 NameTagKind (N**** 轉碼後) 取得記錄
    strNameTagInfo* GetNameTagInfoByKind(uint16_t kind);

    // 取用資料
    const strNameTagInfo* data() const { return table_; }
    uint16_t count() const { return count_; }

private:
    // 反編譯中呼叫 cltItemKindInfo::TranslateKindCode；這裡以等價實作提供
    static uint16_t TranslateItemKindCode(const char* s) {
        if (!s || std::strlen(s) != 5) return 0;
        int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
        int lo = std::atoi(s + 1);
        if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
        return 0;
    }

private:
    uint16_t        count_;   // 筆數（WORD）
    strNameTagInfo* table_;   // 指向 20*count_ bytes 的區塊
};
