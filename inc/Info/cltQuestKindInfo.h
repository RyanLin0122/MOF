#pragma once
#include <cstdint>

struct cltQuestKindInfo
{
    // 依照反編譯：輸入形如 "Q1234"（總長 5），
    // 以 ((toupper(code[0]) + 31) << 11) | atoi(code+1) 編碼，超過 2047 回傳 0。
    static std::uint16_t TranslateKindCode(const char* code);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetQuestKindInfo(std::uint16_t kindCode);
};
