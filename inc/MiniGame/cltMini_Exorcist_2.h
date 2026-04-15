#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "MiniGame/CHospital.h"

class GameImage;

// mofclient.c 還原：cltMini_Exorcist_2 — 「驅魔（醫院）」小遊戲。
// 與 cltMini_Sword_2 共用 cltMoF_BaseMiniGame 的按鈕焦點 / 排名 / 計時器流程，
// 但實際玩法是由內建的 CHospital（3x3 病床 + 治療包選單）執行。
//
// 狀態機 g_cGameExorcist_2State：
//   0 = Wait, 1 = SelectDegree, 2 = Help, 3 = ShowPoint,
//   4 = Ready, 5 = Gamming, 6 = EndStage, 7 = Ranking, 100 = Exit
class cltMini_Exorcist_2 : public cltMoF_BaseMiniGame {
public:
    cltMini_Exorcist_2();
    virtual ~cltMini_Exorcist_2();

    virtual int Poll() override;

    void PrepareDrawing();
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
    void EndStage();
    void Ranking();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Exorcist_2* self);
    static void OnBtn_Ranking(cltMini_Exorcist_2* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Exorcist_2* self);
    static void OnBtn_RankingNext(cltMini_Exorcist_2* self);
    static void OnBtn_Help(cltMini_Exorcist_2* self);
    static void OnBtn_ShowPoint(cltMini_Exorcist_2* self);
    static void OnBtn_ExitPopUp(cltMini_Exorcist_2* self);
    static void OnBtn_DegreeEasy(cltMini_Exorcist_2* self);
    static void OnBtn_DegreeNormal(cltMini_Exorcist_2* self);
    static void OnBtn_DegreeHard(cltMini_Exorcist_2* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Exorcist_2* self);
    static void OnTimer_DecreaseRemainTime(unsigned int id, cltMoF_BaseMiniGame* self);

public:
    // -----------------------------------------------------------------
    // 6 個內嵌 GameImage slot（對齊 mofclient.c 的 +2100/+2124）
    //   每個 slot 含 { active, resID, blockID, x, y } 外加執行期的
    //   GameImage 指標，由 PrepareDrawing 更新。
    // -----------------------------------------------------------------
    struct ImageSlot {
        int           active;
        unsigned int  resID;
        std::uint16_t blockID;
        int           x;
        int           y;
        GameImage*    pImage;
    };

    static constexpr int kSlotCount = 6;

    // --- 背景影像（DWORD[2] / DWORD[4]）---
    GameImage*       m_pBgImage;
    std::uint32_t    m_bgResID;

    // --- 遊戲常數欄位（DWORD[5..10]）---
    int              m_totalScore;          // DWORD[5]：贏/輸 旗標 (0/1)
    int              m_difficultyBaseScore; // DWORD[6]：勝判門檻
    int              m_winMark;             // DWORD[7]：基礎勝利分
    int              m_currentRoundScore;   // DWORD[8]：固定 30
    int              m_finalScore;          // DWORD[9]
    int              m_displayScore;        // DWORD[10]

    // --- 6 個 slot ---
    ImageSlot        m_slots[kSlotCount];

    // --- 三個 DrawNum：倒數秒、結算分、即時得分 ---
    cltMiniGame_DrawNum m_drawNumReady;     // +2244
    cltMiniGame_DrawNum m_drawNumFinal;     // +2304
    cltMiniGame_DrawNum m_drawNumScore;     // +2364

    // --- 醫院場景 ---
    CHospital        m_hospital;            // +2432

    // --- per-class 欄位 ---
    int              m_difficulty;          // DWORD[1189]：0/1/2 = Easy/Normal/Hard
    int              m_score;               // DWORD[1190]：本局得分（鏡像 g_GAMESCORE）
    std::uint32_t    m_pollFrame;           // DWORD[1191]

    std::uint8_t     m_uiRanking;           // BYTE[604]
    std::uint8_t     m_uiSelectDegree;      // BYTE[2424]
    std::uint8_t     m_uiHelp;              // BYTE[2425]
    std::uint8_t     m_uiShowPoint;         // BYTE[2426]
    std::uint8_t     m_uiWin;               // BYTE[2427]
    std::uint8_t     m_uiLose;              // BYTE[2428]

    std::uint8_t     m_showTime2;           // BYTE[568]：Start 鈕 state
    std::uint8_t     m_difficultyByte;      // BYTE[588]：1/2/4
    std::uint8_t     m_prevState;           // BYTE[2061]

    std::uint32_t    m_finalReady;          // DWORD[138]
    std::uint32_t    m_serverAck;           // DWORD[139]
    std::uint32_t    m_serverResult;        // DWORD[140]
    std::uint32_t    m_serverValid;         // DWORD[141]
    std::uint32_t    m_startTick;           // DWORD[145]
    std::uint32_t    m_serverTimeMs;        // DWORD[146]
    std::uint32_t    m_exitTick;            // DWORD[150]

    float            m_incrementFactor;     // DWORD[520]
    std::uint32_t    m_maxScore;            // DWORD[521]

    int              m_drawAlphaBox;        // DWORD[211]：是否繪製中央半透明遮罩
};
