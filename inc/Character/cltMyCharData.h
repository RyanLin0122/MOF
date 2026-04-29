#pragma once

// =============================================================================
// cltMyCharData
//
// 還原自 mofclient.c 0x517970 起的同名類別（IDA 反編譯區段
// 229526~230954 行）。在原 binary 中，cltMyCharData 把幾乎所有玩家狀態
// 子系統 (cltLevelSystem / cltSkillSystem / cltClassSystem / cltQuestSystem
// / cltClientPartySystem / cltTradeSystem / cltLessonSystem / …) 以
// **byte offset 內嵌** 的方式塞進同一物件；本 C++ 還原採用「外掛全域
// 子系統」的設計，所有 (char*)this + N 的存取都改為對應的全域實例
// （g_clQuestSystem / g_clTradeSystem / g_clMyInventory / g_clLessonSystem /
// g_clPartySystem 等），維持邏輯行為等價。
//
// 介面採用「靜態方法 + cltMyCharData* self」的呼叫風格，刻意對齊原
// `cltMyCharData::Method((cltMyCharData *)&g_clMyCharData, …)` 的呼叫式，
// 不破壞現有 caller (cltTutorialSystem / Map / ClientCharacterManager 等)。
//
// 原始碼裡 `*((_DWORD *)this + N)` 的所有狀態欄位都搬到下方 public 成員，
// 命名按用途；其位置順序對齊原 binary（不保證 byte-equal，但保留語意分群）。
// =============================================================================

#include <cstdint>

class cltItemList;
class cltClientPartySystem;
class cltQuestSystem;
class CMofMsg;
struct stEquipItemInfo;
struct stPlayingQuestInfo;
struct strLessonHistory;

class cltMyCharData {
public:
    cltMyCharData();
    ~cltMyCharData();

    // ------------------------------------------------------------------------
    // (00517CF0) Initialize
    //   進入遊戲時、伺服器回傳全套角色狀態封包後呼叫；把封包中所有
    //   欄位扇形分派到各子系統。原型保持與 mofclient.c 一致，確保 caller
    //   (cltTutorialSystem / 登入流程) 寫法不變。
    // ------------------------------------------------------------------------
    static void Initialize(
        cltMyCharData* self,
        std::uint16_t charKind,             // a2
        char classKind,                     // a3
        std::int64_t expValue,              // a4
        int hp,                             // a5
        int mp,                             // a6
        std::uint16_t str,                  // a7
        std::uint16_t dex,                  // a8
        std::uint16_t intel,                // a9
        std::uint16_t vit,                  // a10
        std::uint16_t bonusPoint,           // a11
        int teamKind,                       // GT a12  → cltBaseInventory::Initialize 第 5 參數
        cltItemList* pItemList,             // GT a13
        int initialMoney,                   // GT a14  → cltMoneySystem::InitiaizeMoney
        std::uint16_t numQuickSlot,         // GT a15
        std::uint16_t* skillKindList,       // GT a16
        stEquipItemInfo* pEquipPrimary,
        stEquipItemInfo* pEquipSecondary,
        std::uint16_t lessonItemKind,
        std::uint8_t* lessonRecord1,
        std::uint8_t* lessonRecord2,
        unsigned int lessonExp1,
        unsigned int lessonExp2,
        unsigned int lessonExp3,
        unsigned int lessonExp4,
        unsigned int lessonAmount1,
        unsigned int lessonAmount2,
        unsigned int lessonAmount3,
        unsigned int lessonAmount4,
        strLessonHistory (*lessonHistory)[3],
        std::uint8_t sex,
        char appearKind,
        char hairKind,
        unsigned int appearLook,
        stPlayingQuestInfo* questData,
        std::int16_t specialtyA,
        std::uint16_t specialtyB,
        std::uint16_t* specialtyC,
        std::uint16_t makingItemA,
        std::uint16_t* makingItemB,
        std::uint8_t grade,
        unsigned int meritExp,
        unsigned int meritScore,
        std::uint16_t meritA,
        std::uint16_t meritB,
        int meritFlag,
        char meritC,
        char meritD,
        std::uint16_t* meritE,
        std::uint16_t* meritF,
        std::uint16_t emblemNum,            // a51
        std::uint16_t* emblemKinds,         // a52
        std::uint16_t emblemUsingKind,      // a53
        std::uint16_t quizID,               // a54
        std::uint8_t quizPlayed,            // a55
        std::uint8_t quizAnswerSize,        // a56
        std::uint8_t* quizAnswerPos,        // a57
        std::uint16_t* quizAnswerItem,      // a58
        std::uint16_t currentTitle,         // a59
        std::uint8_t indunMode,             // a60
        CMofMsg* msg);                      // a61

    static void UnInit(cltMyCharData* self);

    // ---- 帳號 / 角色名稱 ----------------------------------------------------
    static void SetMyAccount(cltMyCharData* self, unsigned int account);
    static unsigned int GetMyAccount(cltMyCharData* self);

    static void SetMyCharName(cltMyCharData* self, const char* name);

    // GT (00518610): 直接 `return this;`。呼叫端把回傳值當作 char* 餵進
    // _wsprintfA 的 "%s"；本還原的 cltMyCharData 第一個 member 即是
    // m_szCharName，因此 reinterpret_cast<char*>(self) 會正確指向
    // NUL 結尾的角色名稱字串。
    static cltMyCharData* GetMyCharName(cltMyCharData* self);

    // ---- 伺服器時間 / 金錢 / 拾取 -------------------------------------------
    static void SetServerTime(cltMyCharData* self, unsigned int serverTime);
    static int  GetGameMoney(cltMyCharData* self);
    static void PickUpItem(cltMyCharData* self);
    static void SetNearItemInfo(cltMyCharData* self,
                                std::uint16_t fieldItemID,
                                int money,
                                std::uint16_t itemKind,
                                std::uint16_t itemNum);
    static void RequestPickUpItem(cltMyCharData* self);

    // ---- 等級 / QuickSlot reload --------------------------------------------
    static int  IsLevelUp(cltMyCharData* self);
    static void LevelUpProcess(cltMyCharData* self);
    static void ReLoadQSLData(cltMyCharData* self, CMofMsg* msg);

    // ---- 地圖 / PvP ---------------------------------------------------------
    static void SetMapID(cltMyCharData* self, std::uint16_t mapID);
    static int  GetPKUsableItem(cltMyCharData* self);

    // ---- 物品同步 ----------------------------------------------------------
    static void SetItem(cltMyCharData* self, std::uint16_t itemKind, int slot);
    static void ResetItem(cltMyCharData* self, std::uint8_t slot);
    static void InitMyCharItemInfo(cltMyCharData* self);

    // ---- 小遊戲 / Lesson ----------------------------------------------------
    static std::uint16_t GetMiniGameKind(cltMyCharData* self);
    static unsigned int GetLessonPoint(cltMyCharData* self);
    static void IncLessonPt_Sword(cltMyCharData* self, unsigned int v);
    static void IncLessonPt_Bow(cltMyCharData* self, unsigned int v);
    static void IncLessonPt_Theology(cltMyCharData* self, unsigned int v);
    static void IncLessonPt_Magic(cltMyCharData* self, unsigned int v);

    // ---- 隊伍 ---------------------------------------------------------------
    static int  RequestJoinParty(cltMyCharData* self, char* name);
    static int  ResponesJoinParty(cltMyCharData* self, std::uint8_t accept);
    static void SetCSR_ResponesParty(cltMyCharData* self, int v);
    static int  IsResponesSuccess(cltMyCharData* self);
    static int  RequestLeaveParty(cltMyCharData* self);
    static int  GetAcceptParty(cltMyCharData* self);
    static int  RequestKickOutParty(cltMyCharData* self, unsigned int kickedAccount);
    static unsigned int GetKickedAccount(cltMyCharData* self);
    static cltClientPartySystem* GetPartySystemByIsCreated(cltMyCharData* self);
    static void ReactiveCommunity(cltMyCharData* self);

    // ---- 交易 ---------------------------------------------------------------
    static int  RequestTrade(cltMyCharData* self, unsigned int otherAccount);
    static int  RequestAddTradeBasket(cltMyCharData* self, std::uint8_t pos, std::uint16_t num);
    static void AddTradeBasket(cltMyCharData* self);
    static void AddTradeBasket(cltMyCharData* self, std::uint16_t itemKind, std::uint16_t num, unsigned int time);
    static int  RequestDeleteTradeBasket(cltMyCharData* self, std::uint8_t slot, std::uint16_t num);
    static void SetRequestedTradeAccount(cltMyCharData* self, unsigned int otherAccount);
    static int  RequestResponseTrade(cltMyCharData* self, int accept);
    static int  GetAcceptTrade(cltMyCharData* self);
    static void InitializeTradeSystem(cltMyCharData* self);
    static void DeleteTradeBasket(cltMyCharData* self);
    static void DeleteTradeBasket(cltMyCharData* self, std::uint8_t slot, std::uint16_t num);
    static int  RequestChangeTradeMoney(cltMyCharData* self, unsigned int money);
    static void ChangeMoney(cltMyCharData* self);
    static void ChangeMoney(cltMyCharData* self, unsigned int money);
    static int  RequestTradeDecision(cltMyCharData* self, int decision);
    static int  RequestTradeCancel(cltMyCharData* self);
    static int  SetTradeDecision(cltMyCharData* self);
    static int  SetTradeDecision(cltMyCharData* self, int decision);
    static int  GetMyTradeOK(cltMyCharData* self);
    static int  GetOtherTradeOK(cltMyCharData* self);
    static void CancelTrade(cltMyCharData* self);
    static void DelCharCancelTrade(cltMyCharData* self);

    // ---- 任務 ---------------------------------------------------------------
    static cltQuestSystem* GetQuestSystem(cltMyCharData* self);
    static int  CanReward(cltMyCharData* self, int npcID);
    static std::uint16_t UpdateHuntQuest(cltMyCharData* self, std::uint16_t monsterKind);
    static void SetSpiritSpeechMonsterKill(cltMyCharData* self, std::uint16_t monsterKind);
    static void AddQuestMark(cltMyCharData* self);
    static void CompleteFunctionQuest(cltMyCharData* self, std::uint16_t npcID);

    // ---- Effect / 禁言 / 副本提示 ------------------------------------------
    // GT 5992：idb-only，本體未在反編譯結果中出現；本還原以 no-op 實作
    // （CA / Effect 系統的實際預載仍由其他流程負責）。
    static void LoadEffectImage(cltMyCharData* self);
    static void SetCanNotChatting(cltMyCharData* self, std::uint8_t active);
    static int  GetCanNotChatting(cltMyCharData* self);
    static void IndunQuestNotice(cltMyCharData* self);

    // ---- 自動攻擊 / 事件 ---------------------------------------------------
    static void SetAutoAttack(cltMyCharData* self, int active);
    // 補：取得自動攻擊狀態（ClientCharacterManager 內部需要）。
    static int  GetAutoAttack(cltMyCharData* self);

    // GT 5997/5998：idb-only 宣告，本體未被反編譯出來（推測為事件加成
    // 計時器啟動）。本還原以 no-op 實作。
    static void StartEventAPowerAdvantage(cltMyCharData* self, std::uint16_t a2);
    static void StartEventDPowerAdvantage(cltMyCharData* self, std::uint16_t a2);

    static void SetCorporationUser(cltMyCharData* self, std::uint8_t active);

    // -----------------------------------------------------------------------
    // 狀態欄位（對齊 mofclient.c 的 `*((_DWORD *)this + N)` 取址，保留分群）
    //   - 第一個成員必須是 m_szCharName，使
    //     `reinterpret_cast<char*>(self)` 等同 GetMyCharName 回傳值能讀到
    //     NUL 結尾的角色名稱（GT 0x518610: `return this;`）。
    // -----------------------------------------------------------------------
    char            m_szCharName[50];           // bytes 0..49

    std::uint16_t   m_uNearItemPos;             // GT *(WORD*)+25  (byte 50)
    int             m_iNearItemMoney;           // GT *(DWORD*)+13 (byte 52)
    std::uint16_t   m_uNearItemKind;            // GT *(WORD*)+28  (byte 56)
    std::uint16_t   m_uNearItemNum;             // GT *(WORD*)+29  (byte 58)
    int             m_iRequestPickUpFlag;       // GT *(DWORD*)+15 (byte 60)
    int             m_iSomething16;             // GT *(DWORD*)+16 (byte 64)  — Init 0
    int             m_iSomething17;             // GT *(DWORD*)+17 (byte 68)  — UnInit 0
    std::uint8_t    m_byPrevLevel;              // GT *(BYTE*)+72         IsLevelUp/LevelUp
    int             m_iSomething19;             // GT *(DWORD*)+19 (byte 76) — Init 0
    std::uint16_t   m_uSomething40;             // GT *(WORD*)+40  (byte 80) — Init 0

    unsigned int    m_dwMyAccount;              // GT *(DWORD*)+21 (byte 84)
    unsigned int    m_dwServerTime;             // GT *(DWORD*)+24 (byte 96)
    unsigned int    m_dwSyncLocalTime;          // GT *(DWORD*)+25 (byte 100) ← _time(0)
    int             m_iCanNotChatting;          // GT *(DWORD*)+26 (byte 104)
    int             m_iSomething27;             // GT *(DWORD*)+27 (byte 108) — UnInit 0
    int             m_iAutoAttack;              // GT *(DWORD*)+30 (byte 120)

    std::uint16_t   m_uMapID;                   // GT *(WORD*)+62  (byte 124)
    std::uint16_t   m_uMapAreaName;             // GT *(WORD*)+63  (byte 126)
    std::uint16_t   m_uMapVorF;                 // GT *(WORD*)+64  (byte 128)
    int             m_iIsPvPMap;                // GT *(DWORD*)+33 (byte 132)
    int             m_iIsPvPLobbyMap;           // GT *(DWORD*)+34 (byte 136)
    int             m_iIsContinent;             // GT *(DWORD*)+35 (byte 140)
    int             m_iPKUsableItem;            // GT *(DWORD*)+36 (byte 144)
    int             m_iSomething37;             // GT *(DWORD*)+37 (byte 148) — UnInit 0
    std::uint8_t    m_byCorporationUser;        // GT *(BYTE*)+160

    // 隊伍 / 交易暫存（原 binary 在子系統區塊之後，DWORD index 17330+）
    int             m_iIsResponesSuccess;       // GT *(DWORD*)+17330
    int             m_iAcceptParty;             // GT *(DWORD*)+17331
    unsigned int    m_dwKickedAccount;          // GT *(DWORD*)+17332
    unsigned int    m_dwRequestedTradeAccount;  // GT *(DWORD*)+17335
    unsigned int    m_dwTradeMoneyChange;       // GT *(DWORD*)+17336
    int             m_iAcceptTrade;             // GT *(DWORD*)+17337
    int             m_iTradeDecision;           // GT *(DWORD*)+17338

    std::uint8_t    m_byRequestAddPos;          // GT *(BYTE*)+69332
    std::uint16_t   m_uRequestAddNum;           // GT *(WORD*)+34667
    std::uint8_t    m_byRequestDelPos;          // GT *(BYTE*)+69336
    std::uint16_t   m_uRequestDelNum;           // GT *(WORD*)+34669
};

extern cltMyCharData g_clMyCharData;
