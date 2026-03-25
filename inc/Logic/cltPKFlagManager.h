#pragma once
#include <cstdint>
#include <cstring>
#include "UI/CControlAlphaBox.h"

class GameImage;

// stPKFlag — PK 旗幟房間資訊 (從 mofclient.c 反推)
struct stPKFlag {
    unsigned int handleID = 0;     // 0
    int      posX = 0;             // 4
    int      posY = 0;             // 8
    int      _reserved1 = 0;       // 12
    uint8_t  userNum = 0;          // 16
    uint8_t  maxUserNum = 0;       // 17
    uint8_t  roomLevel = 0;        // 18
    uint8_t  _pad1 = 0;            // 19
    int      textPosX = 0;         // 20
    int      textPosY = 0;         // 24
    int      nameScreenX = 0;      // 28
    int      nameScreenY = 0;      // 32
    char     masterName[64] = {};  // 36
    char     infoText[256] = {};   // 100
    uint16_t animFrame = 0;        // 356
    uint16_t _pad2 = 0;            // 358
    GameImage* pFlagImage = nullptr; // 360
    int      _pad3 = 0;            // 364

    // --- 嵌入 UI 控件 ---
    CControlAlphaBox alphaBox;     // 368

    stPKFlag();
    ~stPKFlag();
};

class cltPKFlagManager {
public:
    cltPKFlagManager();
    virtual ~cltPKFlagManager();

    void AddPKRoom(stPKFlag* pFlag);
    void DeletePKRoom(unsigned int handleID);
    void UpdatePKRoomInfo(unsigned int handleID, stPKFlag* pNewInfo);
    void SetMasterName(unsigned int handleID, char* name);
    void SetUserNum(unsigned int handleID, uint8_t num);
    void SetMaxUserNum(unsigned int handleID, uint8_t num);
    void SetRoomLevel(unsigned int handleID, uint8_t level);
    void UpdatePKRoomInfoText(int index);

    void Poll();
    void PrepareDrawing();
    void Draw(uint8_t index);

    int          IsCreateMatchRoom(int x, int y);
    unsigned int IsJoinMatchRoom(int mouseX, int mouseY);

    static constexpr int MAX_PK_ROOMS = 100;

private:
    int FindRoomIndex(unsigned int handleID);

    stPKFlag   m_rooms[MAX_PK_ROOMS];
    stPKFlag*  m_sortedPtrs[MAX_PK_ROOMS] = {};
    int        m_nRoomNum = 0;
};
