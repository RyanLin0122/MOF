#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"

class GameImage;

// mofclient.c 還原：cltMini_Exorcist — 「驅魔」小遊戲（早期版本，
// 用三張卡 + LEFT/UP/RIGHT 按鍵應對）。和 cltMini_Sword 共用相同的
// cltMoF_BaseMiniGame 框架（按鈕焦點、排名分頁、ready/remain 倒數），
// 但實際玩法是「等卡片變亮 → 對應方向鍵 → CheckSuccess」。
//
// 狀態機 g_cGameExorcistState：
//   0   = Wait
//   1   = SelectDegree
//   2   = Help
//   3   = ShowPoint
//   4   = Ready (倒數)
//   5   = Gamming
//   6   = EndStage
//   7   = Ranking
//   100 = Exit
class cltMini_Exorcist : public cltMoF_BaseMiniGame {
public:
    cltMini_Exorcist();
    virtual ~cltMini_Exorcist();

    virtual int Poll() override;

    void PrepareDrawing();
    void PrepareDrawingCard();
    void Draw();
    int  ExitGame();
    void InitMiniGameImage();

    // 狀態切換
    void Init_Wait();
    void Init_SelectDegree();
    void Init_Ranking();
    void Init_PreRanking();
    void Init_NextRanking();
    void Init_Help();
    void Init_ShowPoint();
    void ShowPointText();
    void SetGameDegree(std::uint8_t degree);

    void Ready();
    void StartGame();
    void Gamming();
    void EndGame();
    void Ranking();

    // 卡片邏輯
    void InitCardTime(unsigned int openTimeMs, unsigned int hitTimeMs);
    void DecideCardViewValue(std::uint8_t cardIdx);
    int  KeyHit(std::uint16_t scancode);
    int  CheckSuccess(std::uint8_t cardIdx);
    void CreateNewCard();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Exorcist* self);
    static void OnBtn_Ranking(cltMini_Exorcist* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Exorcist* self);
    static void OnBtn_RankingNext(cltMini_Exorcist* self);
    static void OnBtn_Help(cltMini_Exorcist* self);
    static void OnBtn_ShowPoint(cltMini_Exorcist* self);
    static void OnBtn_ExitPopUp(cltMini_Exorcist* self);
    static void OnBtn_DegreeEasy(cltMini_Exorcist* self);
    static void OnBtn_DegreeNormal(cltMini_Exorcist* self);
    static void OnBtn_DegreeHard(cltMini_Exorcist* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Exorcist* self);
    static void OnTimer_DecreaseRemainTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_StageClear(unsigned int id, cltMini_Exorcist* self);
    static void OnTimer_TimeOutOpenCardTime();
    static void OnTimer_TimeOutHitCardTime();

public:
    // -----------------------------------------------------------------
    // 影像 slot — mofclient.c 中 50 個連續 5-int 條目；只有前 19 條
    // 真正被 InitMiniGameImage 寫入，其餘保持 active=0。
    // -----------------------------------------------------------------
    struct ImageSlot {
        int           active;   // +0
        unsigned int  resID;    // +4
        std::uint16_t blockID;  // +8 (低半字)
        std::uint16_t pad;      // +10
        int           x;        // +12
        int           y;        // +16
    };

    static constexpr int kSlotCount = 50;

    // m_slotImages[i] 與 m_slots[i] 一一對應（mofclient.c 把指標表
    // 放在 +2100，slot 條目放在 +2300）。
    GameImage*       m_slotImages[kSlotCount];
    ImageSlot        m_slots[kSlotCount];

    // --- 背景 ---
    GameImage*       m_pBgImage;        // DWORD[2]
    std::uint32_t    m_bgResID;         // DWORD[4]

    // --- 結算用欄位 ---
    int              m_totalScore;          // DWORD[5]：勝/負 (0/1)
    int              m_difficultyBaseScore; // DWORD[6]：勝判門檻分
    int              m_winMark;             // DWORD[7]：勝利基礎分
    int              m_currentRoundScore;   // DWORD[8]：固定 30
    int              m_finalScore;          // DWORD[9]
    int              m_displayScore;        // DWORD[10]

    // --- 4 個 DrawNum ---
    cltMiniGame_DrawNum m_drawNumRemain;    // +3300：剩餘秒
    cltMiniGame_DrawNum m_drawNumReady;     // +3360：倒數秒
    cltMiniGame_DrawNum m_drawNumScore;     // +3420：本局得分
    cltMiniGame_DrawNum m_drawNumFinal;     // +3480：最終得分

    // --- 卡片狀態 ---
    std::uint8_t     m_score;               // BYTE[3540]：累計成功擊
    std::uint8_t     m_targetCard[3];       // BYTE[3541..3543]：是否為目標
    std::uint8_t     m_cardImageIdx[3];     // BYTE[3544..3546]：實際畫的 slot 索引

    std::uint8_t     m_remainingHits;       // BYTE[3572]：本回合剩餘可擊次數
    std::uint8_t     m_inputCooldown;       // BYTE[3580]：輸入冷卻計數

    std::uint16_t    m_cardX[3];            // WORD[1800..1802]：卡片 x
    std::uint8_t     m_cardLightSlots[5];   // BYTE[3613..3617]：light 變體 slot 索引
    std::uint8_t     m_cardDarkSlots[5];    // BYTE[3618..3622]：dark 變體 slot 索引
    std::uint8_t     m_cardViewValue[3];    // BYTE[3623..3625]：動畫 0..4

    int              m_cardYPos;            // DWORD[908]：卡片 y

    // --- 各狀態介面對應的 slot 索引 ---
    std::uint8_t     m_uiRanking;       // BYTE[604] = 15
    std::uint8_t     m_uiSelectDegree;  // BYTE[605] = 16
    std::uint8_t     m_uiHelp;          // BYTE[606] = 17
    std::uint8_t     m_uiShowPoint;     // BYTE[607] = 18
    std::uint8_t     m_uiWin;           // BYTE[608] = 13
    std::uint8_t     m_uiLose;          // BYTE[609] = 14

    // --- 卡片/方向鍵的 slot 索引 ---
    std::uint8_t     m_cardSlots[7];        // BYTE[3606..3612] = {6,7,8,9,10,11,12}

    // --- per-class 控制旗標 ---
    int              m_inputReceivedThisFrame; // DWORD[894] = +3576
    std::uint32_t    m_openTimerId;            // DWORD[896]
    std::uint32_t    m_hitTimerId;             // DWORD[897]
    unsigned int     m_openTimeMs;             // DWORD[898]
    unsigned int     m_hitTimeMs;              // DWORD[899]
    std::uint32_t    m_pollFrame;              // DWORD[907]

    std::uint8_t     m_showTime2;       // BYTE[568]：Start 鈕 state
    std::uint8_t     m_difficultyByte;  // BYTE[588]：1/2/4
    std::uint8_t     m_prevState;       // BYTE[2061]

    std::uint32_t    m_finalReady;      // DWORD[138]
    std::uint32_t    m_serverAck;       // DWORD[139]
    std::uint32_t    m_serverResult;    // DWORD[140]
    std::uint32_t    m_serverValid;     // DWORD[141]
    std::uint32_t    m_startTick;       // DWORD[145]
    std::uint32_t    m_serverTimeMs;    // DWORD[146]
    std::uint32_t    m_exitTick;        // DWORD[150]

    float            m_incrementFactor; // DWORD[520]
    std::uint32_t    m_maxScore;        // DWORD[521]

    int              m_drawAlphaBox;    // DWORD[211]：是否繪製中央半透明遮罩
};
