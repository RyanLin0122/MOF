#include "Other/CAttendanceBookParser.h"
#include <cstring>
#include <cstdlib>

// -------------------- Constructor --------------------
CAttendanceBookParser::CAttendanceBookParser() {
    // 0055F1F0：*((_WORD*)this + 100) = 0;
    mCount = 0;
    // 反編譯沒有清資料區，為保持一致，不額外 memset。
}

// -------------------- Initialize --------------------
int CAttendanceBookParser::Initialize(char* String2) {
    // 對應反編譯中的區域變數
    char Delimiter[2 + 1] = { 0 }; // 實際用到 "\t\n"
    FILE* Stream = nullptr;
    int v17 = 0;                 // 預設 0；只有在 while 成功解析並以 EOF 結束時設為 1

    // 準備緩衝（反編譯是 Buffer(1) + v19(1020) 總長 1021? 但 _fgets 用 1023）
    // 這裡使用 1024，符合 _fgets(&Buffer, 1023, ...)
    char buf[1024] = { 0 };

    // 對應：
    // Buffer = `string';  // 無實際意義，忽略
    // memset(v19, 0, sizeof(v19));
    // v20 = 0; v21 = 0;   // 無實際意義，忽略
    std::strcpy(Delimiter, "\t\n");

    // 開檔
    Stream = g_clTextFileManager.fopen(String2);
    if (!Stream) {
        return 0; // 開檔失敗 => 0
    }

    // 跳過前三行
    std::fgets(buf, 1023, Stream);
    std::fgets(buf, 1023, Stream);
    std::fgets(buf, 1023, Stream);

    // 第四行起開始處理；若讀不到第四行，直接走到 LABEL_7（回 1）
    if (std::fgets(buf, 1023, Stream)) {
        while (true) {
            // v5 = _strtok(&Buffer, Delimiter);
            char* p0 = std::strtok(buf, Delimiter);
            if (!p0) {
                // 無第一欄 => 離開 while，回傳 v17（維持 0）
                break;
            }
            uint16_t idx = static_cast<uint16_t>(std::atoi(p0));

            // v7 = _strtok(0, Delimiter); if (!v7) break（回 0）
            char* p1 = std::strtok(nullptr, Delimiter);
            if (!p1) {
                // 對齊反編譯：break 後關檔並回 v17(0)
                break;
            }
            int16_t v8 = static_cast<int16_t>(std::atoi(p1));

            // v9 = _strtok(0, Delimiter); if (!v9) break（回 0）
            char* p2 = std::strtok(nullptr, Delimiter);
            if (!p2) {
                break; // 關檔並回 0
            }
            int16_t v10 = static_cast<int16_t>(std::atoi(p2));

            // v11 = (_WORD *)((char *)this + 4 * v6);
            // *v11     = v8;
            // v11[1]   = v10;
            // ++*((_WORD *)this + 100);
            //
            // 注意：反編譯未做 idx 邊界檢查；此處也不加保護以保持行為一致。
            mRecords[idx].v0 = static_cast<uint16_t>(v8);
            mRecords[idx].v1 = static_cast<uint16_t>(v10);
            ++mCount;

            // 讀下一行；若讀不到 => LABEL_7：v17=1; 關檔；return 1;
            if (!std::fgets(buf, 1023, Stream)) {
                v17 = 1;
                g_clTextFileManager.fclose(Stream);
                return v17;
            }

            // strtok 使用方式需在每次迴圈開始對新的 buf 呼叫 strtok(buf, ...)
            // 目前流程符合
        }

        // while 因格式中斷/欄位不足而跳出 => 關檔並回 v17(0)
        g_clTextFileManager.fclose(Stream);
        return v17;
    }
    else {
        // LABEL_7 的另一條路徑：第四行就讀不到 => v17=1
        v17 = 1;
        g_clTextFileManager.fclose(Stream);
        return v17;
    }
}

// -------------------- GetIndex --------------------
int16_t CAttendanceBookParser::GetIndex(uint16_t a2, uint16_t a3) {
    // 0055F3A0 的語義：掃描 this 起始記憶體的 4-byte 條目，共 mCount 筆
    // 找到第一筆 (v0==a2 && v1==a3) 就回其索引；否則回 -1
    int v3 = 0;
    uint16_t cnt = mCount;

    if (cnt) {
        while (!(mRecords[v3].v0 == a2 && mRecords[v3].v1 == a3)) {
            ++v3;
            if (v3 >= cnt) {
                v3 = -1; // LOWORD(v3) = -1
                break;
            }
        }
    }
    else {
        v3 = -1;
    }
    return static_cast<int16_t>(v3);
}

// -------------------- GetDate --------------------
stAttendance* CAttendanceBookParser::GetDate(uint16_t a2) {
    // 0055F3E0：return (this + 4 * a2)
    // 直接回傳指向內部資料的指標；未做邊界檢查以符合反編譯行為。
    return &mRecords[a2];
}
