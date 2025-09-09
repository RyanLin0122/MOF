#pragma once
#include <cstdint>

// 只提供轉碼函式；其餘資料由其它模組管理
class cltPetSkillKindInfo {
public:
    // 代碼格式：'A' 或 'P' + 5位十進位數字
    // 'A'：結果高位(0x8000) 置 1；'P'：高位為 0
    // 尾五碼必須 < 0x8000；否則回傳 0
    static uint16_t TranslateKindCode(char* a1);
};
