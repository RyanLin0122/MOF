#pragma once

#include <cstdint>

// 動畫物件資訊結構 — 24 bytes，與反編譯佈局完全一致
struct stAniObjectInfo {
    std::uint16_t m_wKindCode;      // offset 0
    std::uint16_t _pad0;            // offset 2
    std::uint32_t m_dwResourceID;   // offset 4
    std::uint32_t m_dwMaxFrames;    // offset 8
    std::uint32_t m_dwHexParam;     // offset 12 (hex value, e.g. flip/color)
    std::uint16_t m_wScale;         // offset 16
    std::uint8_t  m_byVisible;      // offset 18
    std::uint8_t  m_byTransform;    // offset 19
    std::uint8_t  m_byExtra;        // offset 20
    std::uint8_t  _pad1[3];         // offset 21-23
}; // total = 24 bytes

class cltAnimationObjectManager {
public:
    cltAnimationObjectManager();
    virtual ~cltAnimationObjectManager();

    int Initialize(char* filename);

    // 取得指定地圖上的動畫物件數量，同時填充索引對照表
    int GetAniObjCntInMap(std::uint16_t mapKind);

    // 以索引取得動畫物件資訊
    stAniObjectInfo* GetAniObejctInfoByIndex(std::uint16_t index);

    // 取得索引對照表中第 n 筆的索引值
    std::uint16_t GetIndexInMap(std::uint16_t index);

    // 將5字元 KindCode 字串轉為16位元編碼 (static)
    static std::uint16_t TranslateKindCode(char* code);

private:
    stAniObjectInfo*  m_pAniObjectArray;  // DWORD offset 1
    std::uint16_t     m_wCount;           // WORD offset 4
    std::uint16_t     _pad;               // padding
    std::uint16_t*    m_pIndexMap;        // DWORD offset 3
};

extern cltAnimationObjectManager g_clAniObjectMgr;
