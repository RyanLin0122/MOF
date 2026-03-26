#pragma once
#include <cstdint>
#include "UI/CControlAlphaBox.h"
#include "UI/CControlChatBallon.h"

class GameImage;

// stMatchInfo — 對戰房間資訊 (從 mofclient.c 反推)
struct stMatchInfo {
    // --- 基本欄位 ---
    uint8_t  roomID = 0;           // 0
    uint8_t  userNum = 0;          // 1
    uint8_t  maxUserNum = 0;       // 2
    uint8_t  roomLevel = 0;        // 3
    char     masterName[15] = {};  // 4  (15 bytes, GT copy loop < 0xF)
    char     roomTitle[41] = {};   // 19 (41 bytes, GT copy loop < 0x29)
    int      posX = 0;             // 60
    int      posY = 0;             // 64
    char     tag[13] = {};         // 68 (13 bytes, GT copy loop < 0xD)
    uint16_t betMoney = 0;         // 82
    char     infoText[128] = {};   // 84
    int      textPosX = 0;         // 212
    int      textPosY = 0;         // 216
    int      nameScreenX = 0;      // 220
    int      nameScreenY = 0;      // 224
    uint16_t animFrame = 0;        // 228
    GameImage* pFlagImage = nullptr;   // 232
    GameImage* pSubImage = nullptr;    // 236

    // --- 嵌入 UI 控件 ---
    CControlAlphaBox   alphaBox;       // 240
    CControlChatBallon chatBallon;     // 448

    stMatchInfo();
    ~stMatchInfo();
};

class cltMatchManager {
public:
    cltMatchManager();
    virtual ~cltMatchManager();

    void Poll();
    void PrepareDrawing();
    void Draw(uint8_t index);

    int     IsCreateMatchRoom(int x, int y);
    uint8_t IsJoinMatchRoom(int mouseX, int mouseY);

    void InitMatchInfo();
    void AddMatchInfo(stMatchInfo* pInfo);
    void DelMatchInfo(uint8_t index);
    void UpdateMatchUserSize(uint8_t roomID, uint8_t newUserNum);

    stMatchInfo* GetMatchInfo(uint8_t roomID);
    stMatchInfo* GetMatchInfoByIndex(uint8_t index);
    int          GetMatchRoomNum();

    static constexpr int MAX_ROOMS = 21;

private:
    stMatchInfo  m_matches[MAX_ROOMS];
    stMatchInfo* m_sortedPtrs[MAX_ROOMS] = {};
    uint8_t      m_nRoomNum = 0;
};
