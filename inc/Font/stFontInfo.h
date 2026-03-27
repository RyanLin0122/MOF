#pragma once

/**
 * @struct stFontInfo
 * @brief 儲存從字型設定檔中讀取的單一字型設定。
 *
 * 這個結構的定義是根據 MoFFont::InitFontInfo (0x0051B660) 的解析邏輯推斷出來的。
 * Ground truth (win32) 大小 264 bytes (char[128]+char[128]+int+int)。
 * Win64 版本使用 wchar_t[128] 以配合 CreateFontW，大小為 392 bytes。
 */
struct stFontInfo {
    char szKeyName[128];      // 用於查找的鍵名，例如 "Normal"
    //char szFaceName[128];     // ground truth (win32) 使用 char[128]
    wchar_t wszFaceName[128];   // win64 使用 wchar_t，在 InitFontInfo 以 CP949 轉換
    int  nHeight;             // 字型高度
    int  nWeight;             // 字型粗細 (e.g., 400 for FW_NORMAL)
};