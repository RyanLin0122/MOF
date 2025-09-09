#pragma once
#include <cstdint>
#include "global.h"
#include "Font/FileCrypt.h"


// 單筆前端伺服器資訊：20-byte 名稱/IP + 4-byte 連接埠 = 24 bytes
struct stFrontServerInfo {
    char Host[20]; // 以 strcpy 寫入（無長度檢查，忠實還原）
    int  Port;     // atoi 後寫入
};

// 前端伺服器列表（固定容量 10 筆，佈局與反編譯碼一致）
class cltFrontServerInfo {
public:
    cltFrontServerInfo();

    // 忠實還原的 Initialize：依 NationCode 與 FileCrypt 邏輯讀檔並解析
    // 成功回傳 1，否則 0
    int Initialize();

    // 便利介面：指定檔名（保持與原始行為一致的流程）
    int InitializeFromFiles(const char* datPath, const char* tmpPath = "tmp.dat");

    // 與反編譯一致：若旗標為 0 則回傳 nullptr；否則回傳下一筆，並將索引循環
    const stFrontServerInfo* GetRandomServerInfo();

    // 輔助查詢
    uint16_t GetCount() const { return m_wCount; }
    const stFrontServerInfo* GetArray() const { return m_aInfos; }

private:
    // 內部工具
    int  ParseFromFile(const char* filePath);
    void ShuffleLikeDecompiled();
    static bool DeleteFileCompat(const char* path);

private:
    // 佈局需對齊反編譯碼：
    // [0..239]   : 10 筆 * 24 bytes = 240 bytes
    stFrontServerInfo m_aInfos[10]; // 240 bytes

    // offset 240: WORD 計數
    uint16_t m_wCount;              // 2 bytes

    // offset 242: WORD 目前索引（會先 ++ 再取值）
    uint16_t m_wIndex;              // 2 bytes

    // offset 244: DWORD 單次保護旗標（初始化為 1，呼叫 GetRandomServerInfo 後置 0）
    uint32_t m_dwFirstCall;         // 4 bytes
};


// 與反編譯碼對應的除錯層級（2 時顯示錯誤）
extern unsigned int g_DebugLevel;
