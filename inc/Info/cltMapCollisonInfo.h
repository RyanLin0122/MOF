#pragma once
#include <cstdint>
#include <cstdio>

// ============================================================================
//  cltMapCollisonInfo — 地圖碰撞點陣（mofclient.c 0x5799B0..0x579C50）
// ----------------------------------------------------------------------------
//  本類別「不解析任何 .txt」。它從 .map 二進位檔尾或 CMofPacking 預載塊
//  讀入 (width * rowBytes * height) bytes 的位元陣列，由 Map::Initialize 在
//  解析完 tile/portal 資料後觸發：
//    *(DWORD)map == 11   → InitializeInPack(...)（CMofPacking 路徑）或
//                          Initialize(width,height,FILE*)（fopen 路徑）
//    其他版本           → Initialize(width,height)（僅配置全空點陣）
//  Map 將本物件以成員形式內嵌；mofclient.c 中位於 Map +19776（32-bit binary）。
//
//  資料佈局（與 32-bit 反編譯 1:1；x64 重建保留欄位 offset，pBits 因 8-byte
//  指標對齊而落在 +8）：
//    韓文：맵 충돌 비트맵 / 한 줄 바이트 / 비트 배열
//    中文：地圖碰撞點陣 / 每列位元組 / 位元陣列
//
//  位元編碼（IsCollison 反編譯）：
//    給定 (x, y)：byte = m_pBits[(x >> 3) + y * rowBytes]
//    依 (x & 7) 取位：0->0x80, 1->0x40, 2->0x20, 3->0x10,
//                     4->0x08, 5->0x04, 6->0x02, 7->0x01（MSB-first）
//    位元為 1 → 碰撞；x 或 y 越界亦視為碰撞。
// ============================================================================
class cltMapCollisonInfo {
public:
    cltMapCollisonInfo();
    ~cltMapCollisonInfo();

    // mofclient.c 0x5799D0：以 CMofPacking 提供的記憶體區塊（src）填入。
    int InitializeInPack(unsigned short width, unsigned short height, char* src);

    // mofclient.c 0x579A70：自開啟中的 FILE* 直接 fread。
    int Initialize(unsigned short width, unsigned short height, FILE* stream);

    // mofclient.c 0x579B10：僅配置並清零，不填入資料。
    int Initialize(unsigned short width, unsigned short height);

    // mofclient.c 0x579B90：等同 InitializeInPack（兩個位址，行為相同）。
    int Initialize(unsigned short width, unsigned short height, char* src);

    // mofclient.c 0x579C30
    void Free();

    // mofclient.c 0x579C50：座標越界回 1；位元為 1 回 1；否則回 0。
    int IsCollison(unsigned short x, unsigned short y);

private:
    // +0  (offset 0x00, 2 bytes)
    //   韓文：충돌 비트맵 너비 (타일/비트 단위)
    //   中文：碰撞點陣寬度（單位 = tile，亦即位元數）
    unsigned short m_wWidth;

    // +2  (offset 0x02, 2 bytes)
    //   韓文：충돌 비트맵 높이 (타일/비트 단위)
    //   中文：碰撞點陣高度（單位 = tile）
    unsigned short m_wHeight;

    // +4  (offset 0x04, 2 bytes)
    //   韓文：줄당 바이트 수 = ceil(width / 8)
    //   中文：每列位元組數 = ceil(width / 8)
    //   原始反編譯計算式： (width & 0x80000007) != 0 ? (width >> 3) + 1 : (width >> 3)
    unsigned short m_wRowBytes;

    // +6  (offset 0x06, 2 bytes padding for 32-bit pointer; x64 為 6 bytes 至 +8)
    //
    // +8  (offset 0x08, 32-bit binary：4 bytes / x64：8 bytes)
    //   韓文：비트 배열 포인터 (height × rowBytes 바이트)
    //   中文：位元陣列指標；分配大小 = height × rowBytes
    unsigned char* m_pBits;
};
