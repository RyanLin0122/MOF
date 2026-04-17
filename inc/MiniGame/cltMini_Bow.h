#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"
#include "UI/CControlAlphaBox.h"

class GameImage;

// mofclient.c 還原：cltMini_Bow — 弓箭手小遊戲（Bow 1）。
// 狀態機由全域 g_cGameBowState 管理：
//   0 = Wait, 1 = SelectDegree, 2 = Help, 3 = ShowPoint,
//   4 = Ready, 5 = Gamming, 6 = EndGame, 7 = Ranking, 100 = Exit
class cltMini_Bow : public cltMoF_BaseMiniGame {
public:
    cltMini_Bow();
    virtual ~cltMini_Bow();

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
    void EndGame();
    void Ranking();

    // 弓箭操作
    void MoveLargeArrow(std::uint8_t direction);
    void LoadArrow();
    void ShootArrow();
    void ResetArrow();
    int  CheckPoint();
    void MoveTarget();
    int  CreateTarget();

    // --- Button callbacks ---
    static void OnBtn_Start(cltMini_Bow* self);
    static void OnBtn_Ranking(cltMini_Bow* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Bow* self);
    static void OnBtn_RankingNext(cltMini_Bow* self);
    static void OnBtn_Help(cltMini_Bow* self);
    static void OnBtn_ShowPoint(cltMini_Bow* self);
    static void OnBtn_ExitPopUp(cltMini_Bow* self);
    static void OnBtn_DegreeEasy(cltMini_Bow* self);
    static void OnBtn_DegreeNormal(cltMini_Bow* self);
    static void OnBtn_DegreeHard(cltMini_Bow* self);

    // --- Timer callbacks ---
    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Bow* self);

public:
    // 5 欄位圖像 slot（每個 slot 20 bytes + GameImage*）
    struct ImageSlot {
        int           active;
        unsigned int  resID;
        std::uint16_t blockID;
        int           x;
        int           y;
        GameImage*    pImage;
    };

    static constexpr int kSlotCount  = 50;
    static constexpr int kArrowCount = 10;

    // --- 遊戲常數欄位（對齊 mofclient.c DWORD[4..10]）---
    std::uint32_t m_bgResID;             // DWORD[4]  背景資源 ID
    int           m_totalScore;          // DWORD[5]  贏/輸旗標（0/1）
    int           m_difficultyBaseScore; // DWORD[6]  勝判門檻（得分）
    int           m_winMark;             // DWORD[7]  baseScore（依難度）
    int           m_currentRoundScore;   // DWORD[8]  安慰分（固定 30）
    int           m_finalScore;          // DWORD[9]  計算結果分數
    int           m_displayScore;        // DWORD[10] 乘以訓練道具倍率顯示用

    // --- 難度參數 ---
    float         m_bonusMultiplier;     // DWORD[520] 超出 baseScore 的加分倍率
    int           m_scoreCap;            // DWORD[521] 結算分數上限

    // --- 50 個圖像 slot ---
    ImageSlot     m_slots[kSlotCount];

    // --- DrawNum ---
    cltMiniGame_DrawNum m_arrowScoreNums[kArrowCount]; // 每箭得分顯示
    cltMiniGame_DrawNum m_drawNumReady;    // 倒數顯示
    cltMiniGame_DrawNum m_drawNumPoint;    // 目前得分
    cltMiniGame_DrawNum m_drawNumFinal;    // 結算分數

    // --- AlphaBox ---
    CControlAlphaBox m_topBlackBox;
    CControlAlphaBox m_midBlackBox;
    CControlAlphaBox m_sideBoxL;           // 寬螢幕黑邊（左）
    CControlAlphaBox m_sideBoxR;           // 寬螢幕黑邊（右）

    // --- 弓箭狀態 ---
    std::uint8_t  m_arrowShotCount;        // 已射箭數 0..10
    std::uint8_t  m_arrowScores[kArrowCount]; // 每箭得分
    int           m_arrowLoaded;           // 箭已裝填
    int           m_arrowShooting;         // 箭正在飛行
    std::uint16_t m_arrowBlockID;          // 箭的動畫 blockID (=14)
    std::uint8_t  m_targetMoveSpeed;       // 靶移動速度
    std::uint16_t m_totalPoint;            // 總得分
    int           m_firstTimeEnd;          // 第一次進入 EndGame 需算分
    int           m_shootSpeed;            // 射箭速度分母（100→50→25→…）
    std::uint16_t m_arrowNumXPos[kArrowCount]; // 每箭得分數字 X 座標
    int           m_targetX;               // 靶的目前 X 座標
    std::uint16_t m_arrowX;                // 箭的目前 X
    std::uint16_t m_arrowY;                // 箭的目前 Y
    std::uint16_t m_startAreaX;            // 射出起始 X
    std::uint16_t m_startAreaY;            // 射出起始 Y
    int           m_hitTargetY;            // 箭命中的 Y 座標（靶中心）
    int           m_initArrowX;            // DWORD[1253] 箭初始 X（= screenX + 400）
    int           m_initArrowY;            // DWORD[1254] 箭初始 Y（= screenY + 367）
    std::uint8_t  m_shootCounter;          // 射擊動畫計數
    std::uint8_t  m_curArrowSlot;          // 目前箭的 slot 索引（初始 2）
    std::uint8_t  m_curTargetSlot;         // 目前靶的 slot 索引

    // --- UI slot 索引 ---
    std::uint8_t  m_slotRanking;           // 19：排行榜底板
    std::uint8_t  m_slotSelectDeg;         // 20：選擇難度
    std::uint8_t  m_slotHelp;              // 21：說明（同時作為 Draw 主 loop 上限）
    std::uint8_t  m_slotShowPoint;         // 22：得分表
    std::uint8_t  m_slotWin;               // 17：勝利 popup
    std::uint8_t  m_slotLose;              // 18：失敗 popup
    std::uint8_t  m_slotIdx_11;
    std::uint8_t  m_slotIdx_12;
    std::uint8_t  m_slotIdx_13;
    std::uint8_t  m_slotIdx_14;
    std::uint8_t  m_slotIdx_15;
    std::uint8_t  m_slotIdx_16;

    // --- Timer / 狀態 ---
    std::uint32_t m_timerHandle;
    std::uint32_t m_pollFrame;
    std::uint8_t  m_prevState;
    int           m_drawAlphaBox;
    std::uint8_t  m_showTime2;             // 主選單 Start 按鈕 state（4=已打完）
    std::uint8_t  m_difficulty;            // 1/2/4 = Easy/Normal/Hard

    // --- 與結算相關 ---
    std::uint32_t m_finalReady;
    std::uint32_t m_serverAck;
    std::uint32_t m_serverResult;
    std::uint32_t m_serverValid;
    std::uint32_t m_startTick;
    std::uint32_t m_serverTimeMs;
    std::uint32_t m_exitTick;

    GameImage*    m_pBgImage;
};
