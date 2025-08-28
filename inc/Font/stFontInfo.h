#pragma once

/**
 * @struct stFontInfo
 * @brief 儲存從字型設定檔中讀取的單一字型設定。
 *
 * 這個結構的定義是根據 MoFFont::InitFontInfo (0x0051B660) 的解析邏輯推斷出來的。
 * 它的大小是 264 位元組，與原始碼中分配的步長(stride)相符。
 */
struct stFontInfo {
    char szKeyName[128];      // 用於查找的鍵名，例如 "Normal"
    //char szFaceName[128];     // 字型名稱，例如 "Tahoma"
    wchar_t wszFaceName[128];   // 字型名稱，例如 L"Gulim"
    int  nHeight;             // 字型高度
    int  nWeight;             // 字型粗細 (e.g., 400 for FW_NORMAL)
};