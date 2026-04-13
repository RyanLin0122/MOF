#pragma once
#include <cstdint>
#include "Info/cltPortalInfo.h"

class Map;

// cltClientPortalInfo 還原自 mofclient.c (0x004E1350 .. 0x004E16F0)
//
// Layout (來自反編譯):
//   offset 0x00 (DWORD):         stPortalInfo* m_pPortalBuffer
//   offset 0x04..0x2C (10×INT):  暫存 portal index 陣列 (由 GetPortalInfo 寫入)
//   offset 0x2C (INT):           m_nPortalCount
//   offset 0x30 (WORD):          m_wMapKind (目前地圖 kind)
//
// 靜態成員：
//   cltPortalInfo* m_pclPortalInfo
//   Map*           m_pMap
class cltClientPortalInfo {
public:
    cltClientPortalInfo();
    // 注意：ground truth 沒有使用者定義的解構子 (dtor 主體未出現在 mofclient.c)，
    // 由 compiler 產生 trivial dtor。本還原遵循該行為，不提供顯式 dtor。

    // 靜態初始化：儲存 cltPortalInfo/Map 指標
    static void InitializeStaticVariable(cltPortalInfo* portalInfo, Map* map);

    // 依 mapKind 載入該地圖內所有 portal 記錄 (配置 buffer 並複製 40 bytes/筆)
    void Init(uint16_t mapKind);

    // GetPortalInfo: 供 Init 呼叫，a3 為 index 暫存區 (通常指向 this+4)
    void GetPortalInfo(uint16_t mapKind, int* a3);

    // 依 PortalID 取得記錄 (委派給 cltPortalInfo)
    stPortalInfo* GetPortalInfoByPortalID(uint16_t portalID);

    // 釋放目前快取的 portal buffer
    void Free();

    // Portal action 判定：回傳值對應反編譯 switch case
    //   1 = 無 portal / 未命中 / 不在範圍
    //   2 = 等級限制不足
    //   3 = 一般傳送 (a5=目標 mapID, a7=portalType)
    //   4 = 傳送門類型 1
    //   5 = 傳送門類型 6
    unsigned char IsPortalAction(int ptX, int ptY, uint16_t a4,
                                 uint16_t* a5, unsigned char* a6,
                                 uint16_t* a7, int a8);

    // 直接存取 buffer 中第 a3 筆 portal 的欄位
    int       GetPosX(uint16_t a2, uint16_t a3);
    int       GetPosY(uint16_t a2, uint16_t a3);
    int       GetPortalID(uint16_t a2, uint16_t a3);
    stPortalInfo* GetPortalInfoInMap(uint16_t a2);
    uint16_t  GetPortalType(uint16_t a2, uint16_t a3);

public:
    // 靜態成員 (對齊反編譯)
    static cltPortalInfo* m_pclPortalInfo;
    static Map*           m_pMap;

private:
    // Layout 對齊反編譯
    stPortalInfo* m_pPortalBuffer;  // offset 0x00
    int           m_nIndexBuffer[10]; // offset 0x04..0x2C (10 個暫存 index)
    int           m_nPortalCount;   // offset 0x2C
    uint16_t      m_wMapKind;       // offset 0x30
    uint16_t      _pad32;           // offset 0x32
};

extern cltClientPortalInfo g_clClientPortalInfo;
extern cltPortalInfo       g_clPortalInfo;
