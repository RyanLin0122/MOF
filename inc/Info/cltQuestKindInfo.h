#pragma once
#include <cstdint>

struct cltQuestKindInfo
{
    // 依照反編譯：輸入形如 "Q1234"（總長 5），
    // 以 ((toupper(code[0]) + 31) << 11) | atoi(code+1) 編碼，超過 2047 回傳 0。
    static std::uint16_t TranslateKindCode(const char* code);
};
