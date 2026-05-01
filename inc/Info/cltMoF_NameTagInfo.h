#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

// =============================================================================
// strNameTagInfo : 단일 이름표 정보 (single name-tag record)
// 反編譯 (mofclient.c:316913 cltMoF_NameTagInfo::Initialize) 中 v7 為 _WORD*,
// 寫入索引為 v7[0], v7+2, v7[4], v7+6, v7[8]，每筆推進 v7 += 10 (= 20 bytes)。
// 內存 layout 嚴格對應反編譯讀寫之 word 索引 0..9 (共 10 個 WORD = 20 bytes)。
// =============================================================================
#pragma pack(push, 1)
struct strNameTagInfo
{
    // [WORD 0]   offset 0x00 (2 bytes)
    // 韓: NameTag ID  (원본 "N****" 5 글자 → cltItemKindInfo::TranslateKindCode 로 16-bit 코드 변환)
    // 中: 名稱標籤識別碼 (原始檔以 "N****" 5 字串表示，經 TranslateKindCode 轉成 16-bit)
    uint16_t NameTagKind;

    // [WORD 1]   offset 0x02 (2 bytes)
    // 韓: 미사용 패딩 (반편역 코드는 이 워드를 쓰지 않음, memset 으로 0 채움)
    // 中: 對齊用保留欄位（反編譯未寫入；memset 已歸零）
    uint16_t _padWord1;

    // [WORD 2..3] offset 0x04 (4 bytes, DWORD)
    // 韓: 이미지 ID  (HEX 텍스트, sscanf("%x", v7+2) 로 32-bit 워드 두 칸에 기록)
    // 中: 圖像ID（十六進位字串，反編譯以 sscanf("%x", v7+2) 寫入 32-bit）
    uint32_t ImageIDHex;

    // [WORD 4]   offset 0x08 (2 bytes)
    // 韓: 이미지 블록 ID  (atoi → v7[4] = 16-bit)
    // 中: 圖像Block ID（atoi 結果存入 16-bit）
    uint16_t ImageBlockID;

    // [WORD 5]   offset 0x0A (2 bytes)
    // 韓: 미사용 패딩 (반편역 코드는 이 워드를 쓰지 않음)
    // 中: 對齊用保留欄位（反編譯未寫入）
    uint16_t _padWord5;

    // [WORD 6..7] offset 0x0C (4 bytes, DWORD)
    // 韓: PC방 사용자 이미지 ID  (HEX, sscanf("%x", v7+6) → 32-bit)
    // 中: 網咖(PC房)用戶圖像ID（十六進位字串，sscanf 寫入 32-bit）
    uint32_t PCImageIDHex;

    // [WORD 8]   offset 0x10 (2 bytes)
    // 韓: PC방 사용자 블록 ID  (atoi → v7[8] = 16-bit)
    // 中: 網咖用戶圖像Block ID（atoi 結果存入 16-bit）
    uint16_t PCImageBlockID;

    // [WORD 9]   offset 0x12 (2 bytes)
    // 韓: 미사용 패딩 (반편역 코드는 이 워드를 쓰지 않음)
    // 中: 對齊用保留欄位（反編譯未寫入）
    uint16_t _padWord9;
};
static_assert(sizeof(strNameTagInfo) == 20, "strNameTagInfo must be exactly 20 bytes (10 WORDs)");
#pragma pack(pop)

// =============================================================================
// cltMoF_NameTagInfo : character_nametag.txt 解析器
// 對應反編譯：mofclient.c:316876..317014
// 物件配置 (32-bit 反編譯視角)：
//   *(_DWORD*)this + 0  = vtable
//   *((_WORD*)this + 2) = m_count   (WORD)
//   *((_DWORD*)this + 2)= m_table   (strNameTagInfo*)
// 在 x64 編譯下指標寬度雖為 8，但對外觀察的語意（count / table 兩成員）保持一致。
// =============================================================================
class cltMoF_NameTagInfo
{
public:
    cltMoF_NameTagInfo() : m_count(0), m_table(nullptr) {}

    // 反編譯類別具 vtable (有虛擬解構子)，這裡以虛擬解構子保留多型語意。
    virtual ~cltMoF_NameTagInfo()
    {
        // 對應 mofclient.c:316898 反編譯解構子：
        //   v2 = this->m_table; if (v2) { operator delete(v2); this->m_table = 0; }
        // 注意：反編譯解構子並未把 m_count 歸零，但物件即將消滅故無觀察差異。
        if (m_table)
        {
            ::operator delete(m_table);
            m_table = nullptr;
        }
    }

    // 解析 character_nametag.txt；成功回傳 1，否則回傳 0。
    // 對應 mofclient.c:316913
    int Initialize(char* filename);

    // 線性搜尋；對應 mofclient.c:316996
    strNameTagInfo* GetNameTagInfoByKind(unsigned __int16 kind);

    // 純取存（非反編譯介面，僅供 C++ 端方便存取）
    const strNameTagInfo* data() const { return m_table; }
    unsigned __int16 count() const { return m_count; }

private:
    // 反編譯 mofclient.c:316961 直接呼叫 cltItemKindInfo::TranslateKindCode；
    // 此處不再保留私有副本，改於 .cpp 透過 cltItemKindInfo:: 取用，與 GT 1:1。

    // 對應 *((_WORD*)this + 2)：實際資料筆數
    unsigned __int16 m_count;

    // 對應 *((_DWORD*)this + 2)：strNameTagInfo 陣列起點 (operator new 配置)
    strNameTagInfo*  m_table;
};
