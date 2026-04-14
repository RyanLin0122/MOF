#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "MiniGame/Sword2.h"
#include "MiniGame/BackGroundMgr.h"
#include "UI/CControlAlphaBox.h"

class GameImage;

// mofclient.c 還原：cltMini_Sword_2 — 「擊劍（進階）」小遊戲。
// 與 cltMini_Sword 使用相同的 State 機制，但以獨立的 g_cGameSword_2State
// 與子物件 Sword2 執行實際玩法。對齊 mofclient.c 的所有函式呼叫。
//
// 狀態機：
//   0 = Wait, 1 = SelectDegree, 2 = Help, 3 = ShowPoint,
//   4 = Ready, 5 = Gamming, 6 = EndStage, 7 = Ranking, 100 = Exit
class cltMini_Sword_2 : public cltMoF_BaseMiniGame {
public:
    cltMini_Sword_2();
    virtual ~cltMini_Sword_2();

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
    static void OnBtn_Start(cltMini_Sword_2* self);
    static void OnBtn_Ranking(cltMini_Sword_2* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Sword_2* self);
    static void OnBtn_RankingNext(cltMini_Sword_2* self);
    static void OnBtn_Help(cltMini_Sword_2* self);
    static void OnBtn_ShowPoint(cltMini_Sword_2* self);
    static void OnBtn_ExitPopUp(cltMini_Sword_2* self);
    static void OnBtn_DegreeEasy(cltMini_Sword_2* self);
    static void OnBtn_DegreeNormal(cltMini_Sword_2* self);
    static void OnBtn_DegreeHard(cltMini_Sword_2* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Sword_2* self);
    static void OnTimer_DecreaseRemainTime(unsigned int id, cltMoF_BaseMiniGame* self);

public:
    // -----------------------------------------------------------------
    //  內嵌的 6 個 GameImage slot（對齊 mofclient.c 的 +2100/+2124）
    //    每個 slot 含 { active, resID, blockID, x, y } 外加執行期的
    //    GameImage 指標，由 PrepareDrawing() 更新。
    // -----------------------------------------------------------------
    struct ImageSlot
    {
        int          active;
        unsigned int resID;
        std::uint16_t blockID;
        int          x;
        int          y;
        GameImage*   pImage;
    };

    static constexpr int kSlotCount = 6;

    // --- 遊戲常數欄位（對齊 mofclient.c DWORD[4..10]）---
    std::uint32_t m_bgResID;                 // DWORD[4]   背景資源
    int           m_totalScore;              // DWORD[5]   贏/輸 旗標 (0/1)
    int           m_difficultyBaseScore;     // DWORD[6]   勝判門檻（點數）
    int           m_winMark;                 // DWORD[7]   baseScore，依難度
    int           m_currentRoundScore;       // DWORD[8]   安慰分（固定 30）
    int           m_finalScore;              // DWORD[9]   即時算出的本局分數
    int           m_displayScore;            // DWORD[10]  乘以訓練道具倍率後顯示用

    // --- 6 個內嵌 slot ---
    ImageSlot m_slots[kSlotCount];           // +2100 / +2124

    // --- DrawNum / AlphaBox ---
    cltMiniGame_DrawNum m_drawNumReady;      // +2244：倒數顯示
    cltMiniGame_DrawNum m_drawNumFinal;      // +2304：結算分數
    CControlAlphaBox    m_topBlackBox;       // +2364
    CControlAlphaBox    m_botBlackBox;       // +2572

    // --- UI slot index 對照表（原始 byte 2780..2784 / 604）---
    std::uint8_t m_uiRanking;                // byte 604  : slot idx 0（排行榜底板）
    std::uint8_t m_uiSelectDegree;           // byte 2780 : slot idx 3
    std::uint8_t m_uiHelp;                   // byte 2781 : slot idx 4
    std::uint8_t m_uiShowPoint;              // byte 2782 : slot idx 5
    std::uint8_t m_uiWin;                    // byte 2783 : slot idx 1
    std::uint8_t m_uiLose;                   // byte 2784 : slot idx 2

    // --- Sword2 子物件 + 捲動背景 ---
    Sword2        m_sword2;                  // +2788
    BackGroundMgr m_bgMgr;                   // +4064

    // --- 其他 per-class 欄位 ---
    std::uint8_t  m_showTime2;               // byte 568 : 主選單 Start 按鈕 state（4=已打完）
    std::uint8_t  m_difficulty;              // byte 588 : 1/2/4 = easy/normal/hard

    std::uint32_t m_finalReady;              // DWORD[138]：首次進 EndStage 需算分
    std::uint32_t m_serverAck;               // DWORD[139]
    std::uint32_t m_serverResult;            // DWORD[140]
    std::uint32_t m_serverValid;             // DWORD[141]
    std::uint32_t m_startTick;               // DWORD[145]：開始時間
    std::uint32_t m_serverTimeMs;            // DWORD[146]：本局耗時
    std::uint32_t m_exitTick;                // DWORD[150]：結算開始 tick

    float         m_incrementFactor;         // DWORD[520] (float)：超出 base 的分數線性加成
    std::uint32_t m_maxScore;                // DWORD[521]：最終分數上限
    std::uint32_t m_sword2Degree;            // DWORD[1196]：0/1/2 Sword2 內使用的難度
    std::uint32_t m_finalPoint;              // DWORD[1197]：Sword2 回傳的總得分
    std::uint32_t m_pollFrame;               // DWORD[1198]：Poll frame skip 計數

    std::uint8_t  m_prevState;               // byte 2061 : 上一 tick 的狀態
};
