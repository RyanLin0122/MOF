#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "MiniGame/cltBow2_Char.h"
#include "MiniGame/cltBow2_Spear.h"
#include "UI/CControlAlphaBox.h"
#include "Image/Image.h"

class GameImage;

// mofclient.c 還原：cltMini_Bow_2 — 弓箭手小遊戲 2（躲飛矛）。
// 狀態機由全域 g_cGameBow_2State 管理：
//   0 = Wait, 1 = SelectDegree, 2 = Help, 3 = ShowPoint,
//   4 = Ready, 5 = Gamming, 6 = EndStage, 7 = Ranking, 100 = Exit
class cltMini_Bow_2 : public cltMoF_BaseMiniGame {
public:
    cltMini_Bow_2();
    virtual ~cltMini_Bow_2();

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

    // 飛矛系統
    void CheckCollision();
    void ReleaseSpearTimer();
    void AddSpear(std::uint8_t posIndex, std::uint8_t direction, float moveType, float speed);
    void CreateSpear();
    void CreateNormalSpear();
    void CreateSnipeSpear();
    void CreateOctetSpear();
    void Create2VolleySpear();
    void CreateRainSpear();
    void CreateHorizonSpear();
    void PollSpear();

    // 生存時間
    void IncreaseLiveTime();
    unsigned int GetLiveTime();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Bow_2* self);
    static void OnBtn_Ranking(cltMini_Bow_2* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Bow_2* self);
    static void OnBtn_RankingNext(cltMini_Bow_2* self);
    static void OnBtn_Help(cltMini_Bow_2* self);
    static void OnBtn_ShowPoint(cltMini_Bow_2* self);
    static void OnBtn_ExitPopUp(cltMini_Bow_2* self);
    static void OnBtn_DegreeEasy(cltMini_Bow_2* self);
    static void OnBtn_DegreeNormal(cltMini_Bow_2* self);
    static void OnBtn_DegreeHard(cltMini_Bow_2* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_IncreaseLiveTime(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_CreateNormalSpear(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_CreateSnipeSpear(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_CreateOctetSpear(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_Create2VolleySpear(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_CreateRainSpear(unsigned int id, cltMini_Bow_2* self);
    static void OnTimer_CreateHorizonSpear(unsigned int id, cltMini_Bow_2* self);

public:
    // 6 欄位圖像 slot
    struct ImageSlot {
        int           active;
        unsigned int  resID;
        std::uint16_t blockID;
        int           x;
        int           y;
        GameImage*    pImage;
    };

    static constexpr int kSlotCount  = 6;
    static constexpr int kSpearCount = 150;

    // --- 遊戲常數欄位（對齊 mofclient.c DWORD[4..10]）---
    std::uint32_t m_bgResID;             // DWORD[4]  背景資源 ID
    int           m_totalScore;          // DWORD[5]  勝/敗旗標（0/1）
    int           m_difficultyBaseScore; // DWORD[6]  存活時間門檻（ms 為單位內部值）
    int           m_winMark;             // DWORD[7]  baseScore（依難度）
    int           m_currentRoundScore;   // DWORD[8]  安慰分（固定 30）
    int           m_finalScore;          // DWORD[9]  計算結果分數
    int           m_displayScore;        // DWORD[10] 乘以訓練道具倍率顯示用

    // --- 難度參數 ---
    float         m_bonusMultiplier;     // DWORD[520] 超出 baseScore 的加分倍率
    int           m_scoreCap;            // DWORD[521] 結算分數上限

    // --- Image 子物件（建構/解構用，無直接遊戲邏輯） ---
    Image         m_image;

    // --- 生命條圖示 ---
    GameImage*    m_pLifeBarImage;       // DWORD[570]

    // --- 6 個圖像 slot ---
    GameImage*    m_slotImages[kSlotCount];
    ImageSlot     m_slots[kSlotCount];

    // --- DrawNum ---
    cltMiniGame_DrawNum m_drawNumReady;    // 倒數顯示
    cltMiniGame_DrawNum m_drawNumLiveTime; // 存活時間顯示
    cltMiniGame_DrawNum m_drawNumTarget;   // 目標分數顯示
    cltMiniGame_DrawNum m_drawNumFinal;    // 結算分數

    // --- AlphaBox ---
    CControlAlphaBox m_whiteFlash;         // 白色全螢幕閃光
    CControlAlphaBox m_redFlash;           // 紅色全螢幕閃光

    // --- 角色 ---
    cltBow2_Char  m_char;

    // --- 飛矛容器 ---
    stBowSpear    m_spears[kSpearCount];

    // --- Timer handles ---
    std::uint32_t m_timerLiveTime;         // DWORD[3934]
    std::uint32_t m_timerNormalSpear;       // DWORD[3935]
    std::uint32_t m_timerSnipeSpear;        // DWORD[3936]
    std::uint32_t m_timerOctetSpear;        // DWORD[3937]
    std::uint32_t m_timer2VolleySpear;      // DWORD[3938]
    std::uint32_t m_timerRainSpear;         // DWORD[3939]
    std::uint32_t m_timerHorizonSpear;      // DWORD[3940]

    // --- 遊戲狀態 ---
    std::uint32_t m_liveTime;              // DWORD[3941] 存活時間累計
    int           m_liveTimeChanged;       // DWORD[3942] 是否有新的飛矛模式觸發

    // --- Slot 索引 ---
    std::uint8_t  m_slotRanking;           // 0: 排行榜底板 slot
    std::uint8_t  m_slotSelectDeg;         // 3: 選擇難度 slot
    std::uint8_t  m_slotHelp;              // 4: 說明 slot
    std::uint8_t  m_slotShowPoint;         // 5: 得分表 slot
    std::uint8_t  m_slotWin;               // 1: 勝利 popup slot
    std::uint8_t  m_slotLose;              // 2: 失敗 popup slot

    // --- 命中動畫 ---
    int           m_alphaWhiteValue;       // DWORD[3945] 白閃 alpha 值 (0..175)
    int           m_alphaRedValue;         // DWORD[3946] 紅閃 alpha 值 (0..175)
    std::uint8_t  m_hitPhase;              // 命中動畫階段 (0..3)
    int           m_isHit;                 // DWORD[3948] 碰撞旗標
    std::uint32_t m_pollFrame;             // DWORD[3949] poll frame 計數

    // --- 與結算相關 ---
    // m_serverAck / m_serverResult / m_serverValid 由 base class 提供。
    int           m_firstTimeEnd;          // DWORD[138]
    std::uint32_t m_startTick;             // DWORD[145]
    std::uint32_t m_exitTick;              // DWORD[150]

    // --- UI 狀態 ---
    std::uint8_t  m_prevState;             // BYTE[2061]
    int           m_drawAlphaBox;          // DWORD[211]
    std::uint8_t  m_showTime2;             // BYTE[568] 主選單 Start 按鈕 state（4=已打完）
    std::uint8_t  m_difficulty;            // BYTE[588]

    GameImage*    m_pBgImage;
};
