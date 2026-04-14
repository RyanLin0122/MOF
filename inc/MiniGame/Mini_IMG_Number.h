#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：Mini_IMG_Number — 以 GameImage blocks 繪出一個整數。
// SetNumber 將數字轉成每一位數的影格；Process 設定所有位數的 GameImage，
// Render 按序繪製。
class Mini_IMG_Number {
public:
    Mini_IMG_Number();
    ~Mini_IMG_Number();

    void InitMini_IMG_Number(unsigned int dwResourceID,
                             uint16_t     blockBase,
                             uint16_t     digitCapacity);
    void SetNumber(int value, float x, float y);
    void Process();
    void Render();

private:
    static constexpr int kMaxDigits = 10;

    GameImage*   m_pImages[kMaxDigits];     // +0..+36 : 每一位數的 GameImage 指標
    uint8_t      m_digitBlock[kMaxDigits];  // +40..+49: 每一位數對應的 block id offset
    uint8_t      m_digitCount;              // +50: 有效位數
    uint8_t      m_pad51[1];
    unsigned int m_dwResourceID;            // +52: 資源 ID
    uint16_t     m_blockBase;               // +56: 基礎 block id
    uint16_t     m_digitCapacity;           // +58: 總位數上限（未用）
    float        m_fX;                      // +60
    float        m_fY;                      // +64
    float        m_fHalfWidth;              // +68: 整個字串一半的寬度
};
