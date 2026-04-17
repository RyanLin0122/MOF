#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "MiniGame/cltMagic_Box.h"
#include "MiniGame/cltMagic_Target.h"
#include "UI/CControlAlphaBox.h"

class GameImage;

// mofclient.c 還原：cltMini_Magic_2 — 「魔法」小遊戲進階版。
// 32-bit object size = 0x1150 (4432 bytes)
//
// 狀態機 g_cGameMagic_2State：
//   0   = Wait
//   1   = SelectDegree
//   2   = Help
//   3   = ShowPoint
//   4   = Ready (倒數)
//   5   = Gamming (遊戲中)
//   6   = EndStage
//   7   = Ranking
//   100 = Exit
class cltMini_Magic_2 : public cltMoF_BaseMiniGame {
public:
    static constexpr int kMaxTargets = 10;
    static constexpr int kMaxTargetData = 70;
    static constexpr int kTimeLevels = 5;
    static constexpr int kSlotCount = 10;

    cltMini_Magic_2();
    virtual ~cltMini_Magic_2();

    virtual int  Poll() override;
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

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
    void SetBoxYPos();
    int  CheckCollision(std::uint8_t side);
    void CreateNewTarget();
    void IncreaseTimeLevelIndex();
    void EndStage();
    void Ranking();
    void DrawScreenEffect();

    // 繪圖子函式
    void PollTarget();
    void PrepareDrawingTarget();
    void DrawTarget();
    void PollBox();
    void PrepareDrawingBox();
    void DrawBox();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Magic_2* self);
    static void OnBtn_Ranking(cltMini_Magic_2* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Magic_2* self);
    static void OnBtn_RankingNext(cltMini_Magic_2* self);
    static void OnBtn_Help(cltMini_Magic_2* self);
    static void OnBtn_ShowPoint(cltMini_Magic_2* self);
    static void OnBtn_ExitPopUp(cltMini_Magic_2* self);
    static void OnBtn_DegreeEasy(cltMini_Magic_2* self);
    static void OnBtn_DegreeNormal(cltMini_Magic_2* self);
    static void OnBtn_DegreeHard(cltMini_Magic_2* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Magic_2* self);
    static void OnTimer_DecreaseRemainTime(unsigned int id, cltMini_Magic_2* self);
    static void OnTimer_StageClear(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_CreateNewTarget(unsigned int id, cltMini_Magic_2* self);

public:
    // ======================== Member fields ========================

    // --- 遊戲參數 ---
    std::uint8_t    m_difficulty;        // BYTE[588]: 1=easy, 2=normal, 4=hard
    std::uint8_t    m_showTime2;         // BYTE[568]: Start 按鈕 state (0 or 4)

    // --- 結算用欄位 ---
    int             m_totalScore;          // DWORD[5]
    int             m_passScore;           // DWORD[6]: 合格分數
    int             m_winBaseScore;        // DWORD[7]: 勝利基礎分
    int             m_roundFixedScore;     // DWORD[8]: 固定分
    int             m_baseScore;           // DWORD[9]: 結算基礎分
    int             m_displayScore;        // DWORD[10]: 最終得分
    float           m_incrementFactor;     // DWORD[520]: float
    std::uint32_t   m_maxScore;            // DWORD[521]

    // --- 遊戲分數 ---
    std::uint16_t   m_gameScore;           // WORD[1696]

    // --- 控制旗標 ---
    int             m_drawAlphaBox;        // DWORD[211]
    int             m_focusEnabledMagic;   // DWORD[517]

    // --- 背景影像 ---
    GameImage*      m_pBgImage;            // DWORD[2]
    std::uint32_t   m_bgResID;             // DWORD[4]
    std::uint8_t    m_lessonType;          // BYTE[20]

    // --- 時間等級 ---
    std::uint8_t    m_timeLevelIndex;      // BYTE[4416]
    std::uint32_t   m_timeCreateTable[kTimeLevels]; // DWORD[1099..1103]
    std::uint32_t   m_lastCreateTick;      // DWORD[847]: timeGetTime

    // --- Target 管理 ---
    std::uint8_t    m_totalTargetCount;    // BYTE[4394]
    std::uint8_t    m_currentTargetIndex;  // BYTE[4417]

    struct TargetData {
        std::uint8_t kind;       // 0=小妖, 1=中妖, 2=骷髏
        std::uint8_t type;       // speed type 0..3
        std::uint8_t direction;  // 0=右→左, 1=左→右
    };
    TargetData      m_targetData[kMaxTargetData];

    // --- 畫面震動效果 ---
    int             m_screenDamageActive;  // DWORD[1105]
    std::uint8_t    m_screenDamageCount;   // BYTE[4424]

    // --- UI slot 索引 ---
    std::uint8_t    m_uiSlotLeftBox;       // BYTE[4418]
    std::uint8_t    m_uiSlotRightBox;      // BYTE[4419]
    std::uint8_t    m_uiSlotRanking;       // BYTE[604]
    std::uint8_t    m_uiSlotPass;          // BYTE[3397]
    std::uint8_t    m_uiSlotFail;          // BYTE[3398]
    std::uint8_t    m_uiSlotDegreeSelect;  // BYTE[3394]
    std::uint8_t    m_uiSlotHelp;          // BYTE[3395]
    std::uint8_t    m_uiSlotShowPoint;     // BYTE[3396]

    // --- 每輪計數 ---
    int             m_pollFrameCounter;    // DWORD[1107]

    // --- 前一狀態 ---
    std::uint8_t    m_prevState;           // BYTE[2061]

    // --- DrawNum 物件 ---
    cltMiniGame_DrawNum m_drawNumReady;    // 倒數顯示
    cltMiniGame_DrawNum m_drawNumRemain;   // 剩餘時間
    cltMiniGame_DrawNum m_drawNumScore;    // 分數
    cltMiniGame_DrawNum m_drawNumFinal;    // 結算分數

    // --- Image slots (background overlays) ---
    GameImage*      m_slotImages[kSlotCount];
    struct ImageSlot {
        int           active;
        std::uint32_t resID;
        std::uint16_t blockID;
        int           x;
        int           y;
    };
    ImageSlot       m_slots[kSlotCount];

    // --- CControlAlphaBox 子物件 ---
    CControlAlphaBox m_screenEffectBox;    // 紅色畫面閃爍
    CControlAlphaBox m_sideBoxLeft;        // 寬螢幕左黑邊
    CControlAlphaBox m_sideBoxRight;       // 寬螢幕右黑邊

    // --- Magic Box & Target 子物件 ---
    cltMagic_Box    m_boxLeft;
    cltMagic_Box    m_boxRight;
    cltMagic_Target m_targets[kMaxTargets];
};
