#pragma once
#include <cstdint>

class GameImage;

// mofclient.c 還原：cltMiniGame_Button — 小遊戲通用按鈕元件。
// 32-bit 大小 0x5C (92 bytes)，stride 用於 eh vector constructor iterator。
class cltMiniGame_Button {
public:
    cltMiniGame_Button();
    ~cltMiniGame_Button();

    void CreateBtn(int x, int y,
                   unsigned int imageType,
                   unsigned int resNormal,   uint16_t blockNormal,
                   unsigned int resOver,     uint16_t blockOver,
                   unsigned int resDown,     uint16_t blockDown,
                   unsigned int resDisabled, uint16_t blockDisabled,
                   void (*callback)(std::uintptr_t),
                   std::uintptr_t userData,
                   int initialActive);

    void SetActive(int active);
    void SetBtnState(uint8_t state);
    void SetPosition(int x, int y);
    void ButtonAction();
    int  Poll();
    void PrepareDrawing();
    void Draw();

    unsigned int GetImageType();
    unsigned int GetResourceID();
    uint16_t     GetBlockID();

    int     IsActive() const { return m_nActive; }
    uint8_t GetState() const { return m_nState; }

public:
    // +0: DWORD[0]
    int          m_nActive;
    // +4: BYTE[4]
    uint8_t      m_nState;

    // Hit-test rect: +8..+20 (left, top, right, bottom)
    int          m_left;
    int          m_top;
    int          m_right;
    int          m_bottom;

    // Block dimensions (from animation frame): +24, +28
    int          m_width;
    int          m_height;

    // Per-state image data
    // Normal: +32, +36, +40
    unsigned int m_normalImageType;
    unsigned int m_normalResID;
    uint16_t     m_normalBlockID;
    // Over (hover): +44, +48, +52
    unsigned int m_overImageType;
    unsigned int m_overResID;
    uint16_t     m_overBlockID;
    // Down (pressed): +56, +60, +64
    unsigned int m_downImageType;
    unsigned int m_downResID;
    uint16_t     m_downBlockID;
    // Disabled: +68, +72, +76
    unsigned int m_disabledImageType;
    unsigned int m_disabledResID;
    uint16_t     m_disabledBlockID;

    // +80: GameImage*
    GameImage*   m_pImage;
    // +84: callback
    void       (*m_pCallback)(std::uintptr_t);
    // +88: userData
    std::uintptr_t m_userData;
};
