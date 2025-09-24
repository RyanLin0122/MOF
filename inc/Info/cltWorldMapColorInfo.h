#pragma once
#include <cstdint>
#include <cstddef>

class cltWorldMapColorInfo {
public:
    cltWorldMapColorInfo();
    ~cltWorldMapColorInfo();

    // 與反編譯一致：成功回傳 1，失敗回傳 0
    int InitializeInPack(char* pathInPack);
    int Initialize(const char* fileName);
    void Free();

    // 取得顏色：成功回傳像素(0–255)，越界或未初始化回傳 0
    int GetColor(int x, int y) const;

private:
    // 直接模擬 operator new / operator delete 配對（非 new[]/delete[]）
    uint8_t* m_pData;

    static constexpr int    kWidth = 409;     // 與索引式 409*a3 + a2 一致
    static constexpr int    kHeight = 331;
    static constexpr size_t kSize = 0x210D3; // 反編譯中的配置大小
};
