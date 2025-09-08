#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include "global.h"

// 每筆 20 bytes，與反編譯位移完全對齊
#pragma pack(push, 1)
struct strLessonKindInfo
{
    // +0~+2：基本三欄
    uint8_t  kind;          // GetLessonKind()
    uint8_t  ranking_kind;  // GetLessonKindOfRanking()
    uint8_t  type;          // GetLessonType(): 0..3 合法，4 非法

    // +3：未使用（反編譯未賦值）
    uint8_t  _rsv3{ 0 };

    // +4~+8：WORD 欄位
    uint16_t name_code;     // 이름 코드
    uint16_t desc_code;     // 설명 코드
    uint16_t place_code;    // PlaceCode（注意：在檔案最後一欄，但反編譯於最後寫入 +8）

    // +10：未使用（保持 0，以對齊 20 bytes）
    uint16_t _rsv10{ 0 };

    // +12：DWORD 十六進位
    uint32_t icon_res_id;   // 아이콘 리소스 아이디（"%x" 讀入）

    // +16：WORD
    uint16_t block_id;      // 블록 아이디

    // +18：未使用
    uint16_t _rsv18{ 0 };
};
#pragma pack(pop)

static_assert(sizeof(strLessonKindInfo) == 20, "strLessonKindInfo must be 20 bytes");

class cltLessonKindInfo
{
public:
    cltLessonKindInfo() = default;
    ~cltLessonKindInfo() { Free(); }

    // 成功完整讀畢：回傳 1；否則 0
    int Initialize(char* filename);
    void Free();

    // 查詢
    strLessonKindInfo* GetLessonKindInfo(uint8_t kind);
    strLessonKindInfo* GetLessonKindInfoByIndex(int index);

    // 工具（與反編譯行為一致）
    int  GetLessonKind(char* s);              // SWORD_1.. / BOW_1.. / MAGIC_1.. / THEOLOGY_1..
    int  GetLessonKindOfRanking(char* s);     // 同上，獨立函式但內容相同
    uint8_t GetLessonType(char* s);           // SWORD=0, BOW=1, MAGIC=2, THEOLOGY=3, 其它=4
    int  IsValidLessonKind(uint8_t kind);     // 走訪表確認是否存在

private:
    // 小工具
    static bool IsDigitStr(const char* t) {
        if (!t || !*t) return false;
        for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
            if (!std::isdigit(*p)) return false;
        return true;
    }
    static bool IsAlphaNumStr(const char* t) {
        if (!t || !*t) return false;
        for (const unsigned char* p = (const unsigned char*)t; *p; ++p)
            if (!std::isalnum(*p)) return false; // 十六進位字串允許 0-9a-zA-Z
        return true;
    }

private:
    strLessonKindInfo* m_list{ nullptr };
    int                m_count{ 0 };
};
