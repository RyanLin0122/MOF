#pragma once

#include <cstdint>

#include "MiniGame/cltValidMiniGameScore.h"
#include "UI/CControlAlphaBox.h"
#include "Logic/cltMiniGame_Button.h"

class cltMyCharData;
class cltImageManager;
class CMoFNetwork;
class ClientCharacterManager;
class DirectInputManager;
class GameSound;
class DCTTextManager;

// mofclient.c 還原：cltMoF_BaseMiniGame — 所有小遊戲的共通基底。
// 提供排名資料、讀秒/剩餘時間、按鈕焦點管理、送分與請求排名等邏輯。
class cltMoF_BaseMiniGame {
public:
    static constexpr int kButtonCount = 13;
    static constexpr int kRankingCount = 10;

    struct RankingEntry {
        char     rank[8];       // "%02i"
        char     name[16];      // 玩家名稱
        char     classLabel[8]; // 職業名稱
        std::int32_t score;     // 分數
    };

    cltMoF_BaseMiniGame();
    virtual ~cltMoF_BaseMiniGame();

    // Ground truth: mofclient.c vftable 第 0 個槽指到 Poll，預設空實作。
    virtual int Poll();

    static void InitializeStaticVariable(cltMyCharData* myCharData,
                                         cltImageManager* imageMgr,
                                         CMoFNetwork* network,
                                         ClientCharacterManager* charMgr,
                                         DirectInputManager* inputMgr,
                                         GameSound* gameSound,
                                         DCTTextManager* textMgr);

    int SendScore(std::uint8_t lessonType, std::uint32_t score,
                  std::uint32_t seed, std::uint8_t hitType, std::uint32_t finishMode);

    void SetStage(std::uint8_t stage);
    void SetScore(std::uint32_t score);
    std::uint8_t  GetStage();
    std::uint32_t GetScore();

    int  RequestRanking(std::uint8_t lessonType, std::uint8_t page);
    void SetRankingData(std::uint8_t index, char* name, std::uint8_t classKind, int score);
    void SetReceiveRankingData(std::uint8_t count);
    void DrawRanking(std::uint16_t x, std::uint16_t y, std::uint8_t decimalMode);
    void SetMyRanking(int rank);

    void InitMiniGameTime(unsigned int remainSec, unsigned int readySec);
    void ResetTime();
    unsigned int GetRemainTime();
    unsigned int GetReadyTime();
    void DecreaseReadyTime();
    void DecreaseRemainTime();

    void InitBtnFocus();
    void MoveBtnFocus(std::uint8_t gameStateFilter);

    std::uint16_t GetTrainningItemKind();
    int           GetMultipleNum();

    // --- 靜態管理器指標（對齊 mofclient.c 的 class-static 變數） ---
    static cltMyCharData*         m_pclMyChatData;
    static cltImageManager*       m_pclImageMgr;
    static CMoFNetwork*           m_pNetwork;
    static ClientCharacterManager* m_pCharMgr;
    static DirectInputManager*    m_pInputMgr;
    static GameSound*             m_pGameSoundMgr;
    static DCTTextManager*        m_pDCTTextManager;

public:
    // --- 對齊 mofclient.c 的欄位（以邏輯語意命名） ---
    cltValidMiniGameScore m_validScore;          // +4 子物件
    std::uint32_t m_score;                        // DWORD[11]
    std::uint8_t  m_stage;                        // BYTE[48]
    std::uint8_t  m_showTime;                     // BYTE[49]（存檔/繪圖用旗標）
    std::uint8_t  m_byte50;                       // BYTE[50]
    std::uint8_t  m_byte51;                       // BYTE[51]
    RankingEntry  m_ranking[kRankingCount];       // +52 .. +411
    std::uint8_t  m_curRankPage;                  // BYTE[412]
    std::uint8_t  m_pad_413[3];
    std::uint32_t m_rankDrawCounter;              // DWORD[104] / +416
    std::uint8_t  m_rankCount;                    // BYTE[420]
    char          m_myRankingText[215];           // +421 .. +635（31 bytes buffer 實際夠用，對齊到 AlphaBox）

    CControlAlphaBox m_alphaBox;                  // +636

    std::uint32_t m_readyTime;                    // DWORD[143]
    std::uint32_t m_remainTime;                   // DWORD[144]
    std::uint32_t m_dword148;                     // DWORD[148]
    std::uint32_t m_dword149;                     // DWORD[149]

    // UI 座標（對齊原始 WORD[305..316]）
    std::int16_t  m_uiPos[12];

    cltMiniGame_Button m_buttons[kButtonCount];   // +848

    int m_focusIdx;                                // DWORD[511]，為 m_buttonOrder 索引
    int m_focusLocked;                             // DWORD[516]
    int m_focusEnabled;                            // DWORD[517]
    int m_prevMouseX;                              // DWORD[518]
    int m_prevMouseY;                              // DWORD[519]
    std::uint8_t m_buttonOrder[kButtonCount];      // +2048: [0,1,6,7,2,11,10,9,3,4,5,8,12]
    int m_dword522;                                // DWORD[522]
    int m_screenX;                                 // DWORD[523]
    int m_screenY;                                 // DWORD[524]
};
