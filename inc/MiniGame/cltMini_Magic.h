#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "UI/CControlAlphaBox.h"

class GameImage;

// mofclient.c 還原：cltMini_Magic — 「魔法拼圖」小遊戲。
// 4×4 格子的滑動拼圖，玩家移動游標並交換空格來完成指定圖案。
//
// 狀態機 g_cGameMagicState：
//   0   = Wait
//   1   = SelectDegree
//   2   = Help
//   3   = ShowPoint
//   4   = Ready (倒數)
//   5   = Gamming
//   6   = EndGame
//   7   = Ranking
//   100 = Exit
class cltMini_Magic : public cltMoF_BaseMiniGame {
public:
    cltMini_Magic();
    virtual ~cltMini_Magic();

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

    void StartGame();
    void Gamming();
    void EndGame();
    void Ranking();

    // 拼圖邏輯
    void CreateNewStage();
    void ShowMatrix();
    void ChangeRun();
    void MoveCursor(std::uint8_t direction);
    void Swap(std::uint8_t r1, std::uint8_t c1, std::uint8_t r2, std::uint8_t c2);
    BOOL CheckIndex(int idx);
    int  CheckSuccess();

    // 繪圖子函式
    void PrepareDrawingMatrix();
    void DrawMatrix();
    void PrepareDrawingCursor();
    void DrawCursor();
    void PrepareDrawingGoal();
    void DrawGoal();
    void PrepareDrawingBgPat();
    void DrawBgPat();

    // 時間/進度條
    void SetTimeBar();
    void SetStageBar();
    void ResetBar();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Magic* self);
    static void OnBtn_Ranking(cltMini_Magic* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Magic* self);
    static void OnBtn_RankingNext(cltMini_Magic* self);
    static void OnBtn_Help(cltMini_Magic* self);
    static void OnBtn_ShowPoint(cltMini_Magic* self);
    static void OnBtn_ExitPopUp(cltMini_Magic* self);
    static void OnBtn_DegreeEasy(cltMini_Magic* self);
    static void OnBtn_DegreeNormal(cltMini_Magic* self);
    static void OnBtn_DegreeHard(cltMini_Magic* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Magic* self);

public:
    // ======================== Member fields ========================
    // 以下偏移對齊 mofclient.c 中 cltMini_Magic 的 raw 佈局。
    // 基底 cltMoF_BaseMiniGame 結束於 +2100。

    // --- 背景 ---
    std::uint8_t    m_showTime2;        // BYTE[568]: Start 按鈕 state
    std::uint8_t    m_difficultyByte;   // BYTE[588]: 1/2/4

    // --- 結算用欄位 ---
    int             m_totalScore;          // DWORD[5]
    int             m_gameTime;            // DWORD[6]: 遊戲總時間(ms)
    int             m_winBaseScore;        // DWORD[7]: 勝利基礎分
    int             m_roundFixedScore;     // DWORD[8]: 固定每回合分
    int             m_baseScore;           // DWORD[9]: 結算基礎分
    int             m_displayScore;        // DWORD[10]: 最終得分

    // --- 控制旗標 ---
    // m_serverAck / m_serverResult / m_serverValid 由 base class 提供。
    int             m_gameActive;          // DWORD[138]
    std::uint32_t   m_startTick;           // DWORD[145]
    int             m_drawAlphaBox;        // DWORD[211]
    std::uint32_t   m_timerId;             // DWORD[149]
    std::uint32_t   m_endTick;             // DWORD[150]

    int             m_focusEnabledMagic;   // DWORD[517]
    float           m_incrementFactor;     // DWORD[520]: float
    std::uint32_t   m_maxScore;            // DWORD[521]

    // --- 背景影像 ---
    GameImage*      m_pBgImage;            // DWORD[2]

    // --- DrawNum 基底座標 ---
    std::int16_t    m_drawNumBaseX;        // WORD[1450]
    std::int16_t    m_drawNumBaseY;        // WORD[1451]

    // --- 拼圖矩陣 (4×4 shorts, row stride=8 bytes) ---
    static constexpr int kMaxRows = 4;
    static constexpr int kMaxCols = 4;
    std::int16_t    m_matrix[kMaxRows][kMaxCols];   // +2100

    // --- 目標圖案 ---
    GameImage*      m_goalImages[4];       // +2200

    // --- 時間條 alpha box ---
    CControlAlphaBox m_timeBarBox;         // +2232

    // --- 3 個 DrawNum ---
    cltMiniGame_DrawNum m_drawNumRemain;   // +2440
    cltMiniGame_DrawNum m_drawNumReady;    // +2500
    cltMiniGame_DrawNum m_drawNumScore;    // +2560

    // --- Slot 影像 ---
    static constexpr int kSlotCount = 11;
    struct ImageSlot {
        int           active;
        unsigned int  resID;
        std::uint16_t blockID;
        std::uint16_t pad;
        int           x;
        int           y;
    };
    GameImage*      m_slotImages[kSlotCount];       // +2620
    ImageSlot       m_slots[kSlotCount];            // +2664

    // --- 拼圖狀�� ---
    std::uint8_t    m_prevState;           // BYTE[2061]
    std::uint8_t    m_rows;                // BYTE[2894]: 格子行數
    std::uint8_t    m_cols;                // BYTE[2895]: 格子列數
    std::uint8_t    m_cursorRow;           // BYTE[2896]
    std::uint8_t    m_cursorCol;           // BYTE[2897]
    std::uint8_t    m_emptyRow;            // BYTE[2898]
    std::uint8_t    m_emptyCol;            // BYTE[2899]
    std::int16_t    m_goalValue;           // WORD[1452]
    std::uint8_t    m_caseIndex;           // BYTE[2906]
    std::uint8_t    m_bgAlpha;             // BYTE[2916]
    std::uint8_t    m_stageCompleted;      // BYTE[2917]
    std::uint8_t    m_totalStages;         // BYTE[2918]

    // --- UI slot 索引 ---
    std::uint8_t    m_uiRanking;           // BYTE[604]
    std::uint8_t    m_uiSelectDegree;      // BYTE[605]
    std::uint8_t    m_uiHelp;              // BYTE[606]
    std::uint8_t    m_uiShowPoint;         // BYTE[607]
    std::uint8_t    m_uiWin;              // BYTE[608]
    std::uint8_t    m_uiLose;             // BYTE[609]
    std::uint8_t    m_stageBarSlotBase;    // BYTE[2924]

    // --- 繪圖用指標 ---
    GameImage*      m_pGoalValueImage;     // DWORD[549]
    GameImage*      m_pCursorImage;        // DWORD[554]
    GameImage*      m_pBgPat1;             // DWORD[555]
    GameImage*      m_pBgPat2;             // DWORD[556]
    GameImage*      m_pTimeBarArrow;       // DWORD[557]
    GameImage*      m_matrixImages[kMaxRows][kMaxCols]; // DWORD[533..]

    // --- 座標 ---
    int             m_baseX2;              // DWORD[733]
    int             m_barY;                // DWORD[734]
    int             m_gridBaseX;           // DWORD[735]
    int             m_gridBaseY;           // DWORD[736]
    int             m_goalImgX;            // DWORD[737]
    int             m_goalImgY;            // DWORD[738]
    int             m_barEndX;             // DWORD[741]

    // --- 時間條值 ---
    std::int16_t    m_timeBarWidth;        // WORD[1132]

    // --- 其他 ---
    int             m_bgFadeDir;           // DWORD[728]: 背景閃爍方向
    int             m_timeLapseStarted;    // DWORD[730]
    std::uint32_t   m_gameElapsedTime;     // DWORD[727]: 遊戲經過時間(ms)
    std::uint32_t   m_pollFrame;           // DWORD[732]

    // --- 背景資源 ID ---
    std::uint32_t   m_bgResID;            // DWORD[4]
};
