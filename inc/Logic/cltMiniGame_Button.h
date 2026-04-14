#pragma once
#include <cstdint>

class GameImage;

// cltMiniGame_Button — 從 mofclient.c 反推的按鈕元件。
// cltHelpMessage 內含此物件 (offset 40, 92 bytes)。
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
                   void (*callback)(unsigned int),
                   unsigned int userData,
                   int reserved);

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

    // 供小遊戲焦點管理使用（對齊 mofclient.c 的欄位直接讀取）
    int     IsActive() const { return m_nActive; }
    uint8_t GetState() const { return m_nState; }

private:
    int          m_nActive = 0;
    uint8_t      m_nState = 0;
    int          m_x = 0;
    int          m_y = 0;
    unsigned int m_imageType = 0;
    unsigned int m_resIDs[4] = {};
    uint16_t     m_blockIDs[4] = {};
    void       (*m_pCallback)(unsigned int) = nullptr;
    unsigned int m_userData = 0;
    GameImage*   m_pImage = nullptr;
};
