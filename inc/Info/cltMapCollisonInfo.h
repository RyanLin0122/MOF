#pragma once
#include <cstdint>
#include <cstdio>

// 地圖碰撞點陣資訊（mofclient.c 0x5799B0..0x579C50）。
//
// 用途：
//   Map::Initialize 在解析 .map 檔案時，若 *(DWORD)map == 11（紙包格式版本），
//   會把檔尾的碰撞點陣以 width/height（單位：tile）載入此物件；非 11 則只配置
//   一張全空的點陣。Map 將本物件以成員形式內嵌（mofclient.c：偏移 +19776）。
//
// 資料佈局（與反編譯 1:1）：
//   +0  uint16   m_wWidth        碰撞圖寬（tile 數，亦即位元數）
//   +2  uint16   m_wHeight       碰撞圖高
//   +4  uint16   m_wRowBytes     每列位元組數 = ceil(width / 8)
//   +8  uint8*   m_pBits         位元陣列；大小 = height * rowBytes
//
// 位元編碼（IsCollison 反編譯）：
//   給定 (x, y)：byte = m_pBits[(x >> 3) + y * rowBytes]
//   依 (x & 7) 取位：0->0x80, 1->0x40, ..., 7->0x01（MSB-first）
//   位元為 1 -> 碰撞；x 或 y 越界亦視為碰撞。
class cltMapCollisonInfo {
public:
    cltMapCollisonInfo();
    ~cltMapCollisonInfo();

    // mofclient.c 0x5799D0：以 packing 提供的記憶體區塊（src）填入。
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
    unsigned short m_wWidth;
    unsigned short m_wHeight;
    unsigned short m_wRowBytes;
    unsigned char* m_pBits;
};
