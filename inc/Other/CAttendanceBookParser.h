#ifndef C_ATTENDANCE_BOOK_PARSER_H
#define C_ATTENDANCE_BOOK_PARSER_H

#include <cstdint>
#include <cstdio>
#include "global.h"


// 反編譯於 GetDate 回傳的資料結構（每筆佔 4 bytes）
struct stAttendance {
    uint16_t v0; // 對應寫入的第一個 WORD（v8）
    uint16_t v1; // 對應寫入的第二個 WORD（v10）
};

class CAttendanceBookParser {
public:
    // 0055F1F0：建構子：只把計數（*(WORD*)(this + 200)）設為 0
    CAttendanceBookParser();

    // 0055F210：Initialize(String2)
    // 回傳：讀取到 EOF（正常結束）=> 1；其他（開檔失敗/格式不符/中途退出）=> 0
    int Initialize(char* String2);

    // 0055F3A0：GetIndex(a2, a3) 逐筆比對（v0==a2 && v1==a3），回傳索引；找不到回 -1（int16）
    int16_t GetIndex(uint16_t a2, uint16_t a3);

    // 0055F3E0：GetDate(a2) 回傳位於 (this + 4 * a2) 的資料指標
    stAttendance* GetDate(uint16_t a2);

private:
    // 內存佈局需與反編譯一致：
    // [0..199]  : 50 筆 * 每筆 4 bytes = 200 bytes 的資料區（兩個 WORD）
    // [200..201]: 計數（WORD）=> *((WORD*)this + 100)
    // 其餘未見使用
    stAttendance mRecords[50]; // 起始即為資料區（支撐以 this+4*idx 取址）
    uint16_t     mCount;       // 位移 200 bytes，對應 *((_WORD*)this + 100)
};

#endif // C_ATTENDANCE_BOOK_PARSER_H
