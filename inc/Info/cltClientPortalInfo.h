#pragma once
#include <cstdint>
#include "Info/cltPortalInfo.h"

class Map;

// cltClientPortalInfo 還原自 mofclient.c (0x004E1350 .. 0x004E16F0)
//
// === GT (x86) layout — 全長 0x34 (52) bytes ============================
//   offset 0x00 (DWORD):         stPortalInfo* m_pPortalBuffer
//   offset 0x04..0x2B (10×INT):  暫存 portal index 陣列 (40 bytes;
//                                由 cltPortalInfo::GetPortalCntInMap 填入)
//   offset 0x2C (INT):           m_nPortalCount
//   offset 0x30 (WORD):          m_wMapKind (目前地圖 kind)
//   offset 0x32 (WORD):          (padding)
//
// 反編譯參考索引:
//   *(_DWORD *)this           → m_pPortalBuffer    (0x00)
//   (int *)((char*)this + 4)  → &m_nIndexBuffer[0] (0x04)
//   *((_DWORD *)this + 11)    → m_nPortalCount     (0x2C)
//   *((_WORD  *)this + 24)    → m_wMapKind         (0x30)
//
// 注意：本專案 build 為 x64，pointer 為 8 bytes，因此實際物件 layout
// 不再與 32-bit GT 完全相同；上述 offset 為 GT(x86) 反編譯參照值，
// 僅作為對照文件用。語意（成員相對順序與型別）與 GT 等價。
//
// 韓 / 中 對照：
//   m_pPortalBuffer  포털 정보 버퍼   傳送門資訊緩衝（指向當前地圖的記錄）
//   m_nIndexBuffer   포털 인덱스 큐   傳送門索引佇列（GetPortalCntInMap 填）
//   m_nPortalCount   포털 개수        當前地圖傳送門數量
//   m_wMapKind       맵 종류          目前地圖 kind（IsPortalAction 寫入）
//
// 靜態成員：
//   cltPortalInfo* m_pclPortalInfo   // 全域 PortalList table（mofclient.c:24220）
//   Map*           m_pMap            // 主地圖物件（mofclient.c:24221）
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

    // 取得目前快取的 portal 數量（mofclient.c：dword_AF4244 等位置會直接讀
    // m_nPortalCount，本還原以 getter 暴露）。
    int GetPortalCount() const { return m_nPortalCount; }

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
    // Layout（offset 為 GT/x86 反編譯參照；x64 build 因 pointer 為 8 bytes，
    // 實際 byte-offset 會位移，但成員順序與型別不變）
    //
    //  韓: 포털 정보 버퍼          中: 傳送門記錄緩衝               GT off: 0x00
    stPortalInfo* m_pPortalBuffer;
    //  韓: 인덱스 임시 큐 (10)     中: 索引暫存佇列 (10 ints = 40B) GT off: 0x04..0x2B
    int           m_nIndexBuffer[10];
    //  韓: 포털 개수               中: 傳送門數                     GT off: 0x2C
    int           m_nPortalCount;
    //  韓: 맵 종류 (현재)          中: 目前地圖 kind                GT off: 0x30
    uint16_t      m_wMapKind;
    //  韓: (정렬 패딩)             中: 對齊填充                     GT off: 0x32
    uint16_t      _pad32;
};

extern cltClientPortalInfo g_clClientPortalInfo;
extern cltPortalInfo       g_clPortalInfo;
