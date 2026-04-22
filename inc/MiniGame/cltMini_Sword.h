#pragma once

#include <cstdint>

#include "MiniGame/cltMoF_BaseMiniGame.h"
#include "MiniGame/cltMiniGame_DrawNum.h"

class GameImage;

// mofclient.c 還原：cltMini_Sword — 劍士小遊戲。
// 狀態機由全域 g_cGameSwordState 管理：
//   0 = Wait, 1 = SelectDegree, 2 = Help, 3 = ShowPoint,
//   4 = Ready, 5 = Gamming, 6 = EndStage, 7 = Ranking, 100 = Exit
class cltMini_Sword : public cltMoF_BaseMiniGame {
public:
    cltMini_Sword();
    virtual ~cltMini_Sword();

    virtual int  Poll() override;
    void PrepareDrawing();
    void Draw();
    void ExitGame();

    unsigned int   GetExp();
    void           SetPoint(uint16_t v);
    void           IncreasePoint();
    void           IncLessonPt_Sword(unsigned int v);
    uint16_t       GetPoint();

    // 狀態切換
    void Ready();
    void Init_Wait();
    void Init_SelectDegree();
    void Init_Ranking();
    void Ranking();
    void Init_PreRanking();
    void Init_NextRanking();
    void Init_Help();
    void Init_ShowPoint();
    void ShowPointText();
    void SetGameDegree(uint8_t degree);
    void StartGame();
    void Gamming();
    void EndStage();
    void KeyHit(uint8_t direction);
    void DrawEffect(uint8_t dummy);
    void CheckGameDegree();
    void InitMiniGameImage();

    // --- Button / Timer callbacks ---
    static void OnBtn_Start(cltMini_Sword* self);
    static void OnBtn_Ranking(cltMini_Sword* self);
    static void OnBtn_Exit();
    static void OnBtn_RankingPre(cltMini_Sword* self);
    static void OnBtn_RankingNext(cltMini_Sword* self);
    static void OnBtn_Help(cltMini_Sword* self);
    static void OnBtn_ShowPoint(cltMini_Sword* self);
    static void OnBtn_ExitPopUp(cltMini_Sword* self);
    static void OnBtn_DegreeEasy(cltMini_Sword* self);
    static void OnBtn_DegreeNormal(cltMini_Sword* self);
    static void OnBtn_DegreeHard(cltMini_Sword* self);

    static void OnTimer_DecreaseReadyTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_TimeOutReadyTime(unsigned int id, cltMini_Sword*      self);
    static void OnTimer_DecreaseRemainTime(unsigned int id, cltMoF_BaseMiniGame* self);
    static void OnTimer_StageClear(unsigned int id, cltMini_Sword* self);

public:
    // mofclient.c 的 5 欄位圖像 slot（每個 slot 20 bytes + GameImage*）
    struct ImageSlot
    {
        int           active;
        unsigned int  resID;
        uint16_t      blockID;
        int           x;
        int           y;
        GameImage*    pImage;
    };

    static constexpr int kSlotCount = 40;

    // --- 遊戲狀態欄位 ---
    uint32_t   m_bgResID;                 // +16 (dword 4)
    int        m_totalScore;              // +20 (dword 5)
    int        m_winMark;                 // +28 (dword 7)
    int        m_difficultyBaseScore;     // +24 (dword 6)
    int        m_currentRoundScore;       // +32 (dword 8)
    int        m_finalScore;              // +36 (dword 9)
    int        m_displayScore;            // +40 (dword 10)
    // mofclient.c：*((float*)this + 520) — point 超過 baseScore 後的加分倍率
    //   Easy=2.0、Normal=8.0、Hard=4.0。SetGameDegree 寫入，EndStage 讀取。
    float      m_bonusMultiplier;         // dword 520
    // mofclient.c：*((_DWORD*)this + 521) — 結算分數上限
    //   Easy=90、Normal=180、Hard=360。
    int        m_scoreCap;                // dword 521
    uint16_t   m_point;                   // +36 offset: actually WORD[1982] = +3964

    // 5*i + 565 中的「i」剛好是 slot index，所以 slot data 從 dword 565 = byte 2260 開始。
    // 原始邏輯每個 slot 含 active(int) + resID(int) + blockID(short)+pad + x(int) + y(int) 共 20 bytes。
    ImageSlot  m_slots[kSlotCount];       // +2260 ~ +3060

    cltMiniGame_DrawNum m_drawNumTime;    // +3060：剩餘時間顯示
    cltMiniGame_DrawNum m_drawNumPoint;   // +3120：目前得分
    cltMiniGame_DrawNum m_drawNumReady;   // +3180：倒數顯示
    cltMiniGame_DrawNum m_drawNumFinal;   // +3240：最終分數顯示

    CControlAlphaBox    m_topBlackBox;    // +3300
    CControlAlphaBox    m_midBlackBox;    // +3508
    CControlAlphaBox    m_botBlackBox;    // +3716

    uint8_t    m_byte3932;                // 每回合首次結束時=1
    uint8_t    m_gameDegree;              // +3940 (= 20/30/40 依難度)
    // mofclient.c：dword 984 — 基準的 spawn interval（Easy=800 / Normal=700 / Hard=600），
    //   由 SetGameDegree 寫入、CheckGameDegree 每幀用以重算 m_spawnInterval。
    //   早先版本錯把 m_winMark (50/100/200) 當基準，導致目標生成間隔僅 50ms，
    //   木頭人與箭頭每幀連續刷新。
    int        m_baseSpawnInterval;       // +3936 (dword 984)
    int        m_dword3944;               // trainning item kind
    uint8_t    m_byte3948;                // slot hit direction

    // 4 個怪物「index slot」——對齊 Init 中的 slot id 常數
    uint8_t    m_slotMonsterBase;         // 3994 = 8
    uint8_t    m_slotMonsterHead;         // 3995 = 30
    uint8_t    m_slotHitFX;               // 3996 = 22
    uint8_t    m_slotMissFX;              // 3997 = 26
    uint8_t    m_slotMonsterAlt;          // 3998 = 24
    uint8_t    m_slotTargetFX;            // 3999 = 31
    uint8_t    m_slotEffectA;             // 4000 = 12
    uint8_t    m_slotEffectB;             // 4001 = 13
    uint8_t    m_slotEffectC;             // 4002 = 14
    uint8_t    m_slotReadyBG;             // 4003 = 17
    uint8_t    m_slotReadyTime;           // 4004 = 18
    uint8_t    m_slotPointFrame;          // 4005 = 30
    uint8_t    m_slotHelpFrame;           // 4006 = 15
    uint8_t    m_slotBtnFrame;            // 3993 = 4

    // 對應狀態
    uint8_t    m_curTarget;               // 3972 (目前顯示的攻擊目標 0..3)
    uint8_t    m_nextTarget;              // 3973 (下次目標)
    int        m_missFlash;               // 3988 dword 997
    int        m_hitLocked;               // 3968 dword 992
    int        m_needNewTarget;           // 3976 dword 994
    int        m_monsterSpawned;          // 3952 dword 988
    unsigned int m_lastSpawnTick;         // 3960 dword 990
    unsigned int m_spawnInterval;         // 3956 dword 989
    int        m_startTick;               // 4008 dword 1002
    int        m_pollFrame;               // 4012 dword 1003
    uint8_t    m_prevState;               // +2061

    // mofclient.c：*((_DWORD *)this + 211) — 每幀的 m_alphaBox 顯示旗標。
    //   Poll 開頭清為 0；state == 1/2/3 或 7 時設為 1；PrepareDrawing/Draw
    //   依它決定是否繪製整張半透明遮罩。與 base class 的 m_rankDrawCounter
    //   (DWORD[104]) 是兩個語意完全不同的欄位：前者是「本幀要不要畫 alpha」
    //   的 frame-local flag，後者是「排名資料尚未就緒，不要畫 DrawRanking」
    //   的 persistent gate。早先的版本把兩者合併成 m_rankDrawCounter，導致
    //   state==7 時排行榜 / 遮罩互相干擾。
    int        m_drawAlphaBox;

    // 與結算 popup 相關
    // m_serverAck / m_serverResult / m_serverValid 由 base class 提供。
    uint32_t   m_finalReady;              // dword 138
    uint32_t   m_serverTimeMs;            // dword 146
    uint32_t   m_exitTick;                // dword 150

    // mofclient.c：PrepareDrawing 取得的背景圖 pointer
    //   *((_DWORD *)this + 2)：以獨立欄位存放，避免透過 cltMoF_BaseMiniGame 的
    //   其他成員意外覆寫。Draw 一開始會直接 Draw(m_pBgImage)。
    GameImage* m_pBgImage;

    // mofclient.c：*((_BYTE *)this + 3966) — 每次 PrepareDrawing 都 +=6 並
    // 寫入 target FX slot 的 alpha，模擬「緩慢閃爍」的動畫狀態機。
    uint8_t    m_targetAlphaState;

    // mofclient.c：bytes 604-609 — 6 個 priority 用途的 slot 索引（在 Init 設為
    // 19/20/21/32/33/34，整個 cltMini_Sword 生命週期不再變動）。這些值被
    // Draw 使用於首個 loop 的結束界線 (byte606=33) 以及四個 priority slot。
    uint8_t    m_drawSlot604;             // byte 604 = 19 (ranking base slot)
    uint8_t    m_drawSlot605;             // byte 605 = 32 (priority slot A)
    uint8_t    m_drawSlot606;             // byte 606 = 33 (priority slot B / loop bound)
    uint8_t    m_drawSlot607;             // byte 607 = 34 (priority slot C)
    uint8_t    m_drawSlot608;             // byte 608 = 20 (ranking prev slot)
    uint8_t    m_drawSlot609;             // byte 609 = 21 (ranking next slot)
};
