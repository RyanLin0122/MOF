#pragma once

#include <cstdint>
#include "Character/ClientCharacter.h"

// Forward declarations
class CCA;
class cltCharKindInfo;
class cltMonsterAniInfo;
class cltClientCharKindInfo;
class CEffectBase;
class CEffect_Player_MapConqueror;
struct stCharKindInfo;

// =========================================================================
// Embedded helper subsystems referenced by ClientCharacterManager.
// In mofclient.c they live at fixed byte offsets inside the manager
// (loc_4BE0F3+5 / loc_4BE113+1 / loc_4BE93A+2 / loc_4BE974).  Here they
// are stored as named members; behaviour matches the decomp.
// =========================================================================

// mofclient.c 0x4172C0+：滑鼠/快捷攻擊鎖定後，於目標角色頭頂顯示的箭頭標記。
class cltTargetMark {
public:
    cltTargetMark();
    ~cltTargetMark();
    void SetActive(int active);
    void SetPos(float x, float y);
    void Poll();
    void PrepareDrawing();
    void Draw();
private:
    int   m_iActive{ 0 };
    float m_fPosX{ 0.0f };
    float m_fPosY{ 0.0f };
    int   m_iCounter{ 0 };
};

// mofclient.c 0x004133D0+：自動移動目的地的箭頭標記（玩家點擊地面後產生）。
class cltAutoMoveTarget {
public:
    cltAutoMoveTarget();
    ~cltAutoMoveTarget();
    void SetActive(int active);
    void InitNewTarget(int x, int y);
    void Poll();
    void PrepareDrawing();
    void Draw();
private:
    int   m_iActive{ 0 };
    int   m_iX{ 0 };
    int   m_iY{ 0 };
    int   m_iCounter{ 0 };
};

// mofclient.c 0x00413300+：跑步狀態鎖（持續跑）。
class cltKeepRunState {
public:
    cltKeepRunState();
    ~cltKeepRunState();
    void SetActive(ClientCharacter* pChar, int active);
    void ReleaseViewKeepRunState();
    static void OnTimer_ReleaseViewKeepRunState(unsigned int timerID,
                                                cltKeepRunState* self);
    void Poll();
    void PrepareDrawing();
    void Draw();
private:
    int               m_iActive{ 0 };
    ClientCharacter*  m_pChar{ nullptr };
    unsigned int      m_dwTimer{ 0 };
};

// mofclient.c 0x00413BE0+：副本入口判定（特殊地圖傳送）。
class cltInstansDungeonPortal {
public:
    cltInstansDungeonPortal();
    ~cltInstansDungeonPortal();
    void Init();
    int  AddPortal(unsigned short mapID);
    unsigned char IsPortalAction(int x, int y, int* outMapID);
private:
    enum { kMaxPortals = 16 };
    unsigned short m_mapIDs[kMaxPortals]{};
    int            m_count{ 0 };
};

// =========================================================================
// ClientCharacterManager — 客戶端所有角色（自己+他人+怪物）的容器與調度器。
//
// In mofclient.c the class embeds 300 ClientCharacter slots (each 14824
// bytes) followed by per-manager state.  In this restoration the slot
// storage lives in the existing `unk_1409D80[1000]` global so we don't
// duplicate the heavy in-place array — index `i` (1..299) of unk_1409D80
// corresponds to slot `i` in the decomp.
// =========================================================================
class ClientCharacterManager {
public:
    ClientCharacterManager();
    ~ClientCharacterManager();

    // --- Lifecycle / 整體控制 ----------------------------------------------
    int  Initialize();
    void Free();
    void Poll();
    void DeleteAllChar();
    void DeleteAllMonster();
    void AllDieMonster();
    void DeleteCharacter(unsigned int account, int slotHint);

    // --- 計數 / 查找 -------------------------------------------------------
    short GetMonsterCount();
    short GetPlayerCount();
    ClientCharacter* GetMyCharacterPtr();
    ClientCharacter* GetCharByAccount(unsigned int account);
    ClientCharacter* GetCharByName(char* name);
    ClientCharacter* GetCharByIndex(int index);

    // --- 我的帳號管理 ------------------------------------------------------
    void         SetMyAccount(unsigned int account);
    unsigned int GetMyAccount() const { return m_dwMyAccount; }

    // --- AddCharacter（mofclient.c 0x40DE60，33 個參數加 this） ---------
    // 參數名稱對齊 mofclient.c：a2..a34（其中常見的語意已標註於後）。
    // 為相容既有 cltTutorialSystem 呼叫使用 const char*/int 等寬鬆型別，
    // 內部會 const_cast / static_cast 後轉發到 ClientCharacter::CreateCharacter。
    //
    // CreateCharacter 的呼叫順序為 mofclient.c 35617 的 permute（不是直接前向）：
    //   a2..a12 直接前向 → CreateCharacter a2..a12
    //   AddCharacter a27 → CreateCharacter a13 (petKind, uint16)
    //   AddCharacter a30 → CreateCharacter a14 (sexLetter, 'M'/'F')
    //   AddCharacter a31 → CreateCharacter a15 (cUnknown15, char)
    //   AddCharacter a32 → CreateCharacter a16 (face, uint8)
    //   AddCharacter a33 → CreateCharacter a17 (accountID, uint32)
    //   AddCharacter a13..a22 → CreateCharacter a18..a27
    //   AddCharacter a23 → CreateCharacter a28 (privateMarketMsg)
    //   AddCharacter a29 → CreateCharacter a29 (ucPCRoom, uint16)
    //   AddCharacter a28 → CreateCharacter a30 (weaponKind, uint16)
    //   AddCharacter a25 → CreateCharacter a31 (shieldKind, uint16)
    //   AddCharacter a24 → CreateCharacter a32 (screenName)
    //   AddCharacter a26 → CreateCharacter a33 (nation, uint8)
    //   AddCharacter a34 → CreateCharacter a34 (titleKind, uint16)
    //   m_iCreateClientEffectFlag → CreateCharacter a35 (iUnknown35)
    void AddCharacter(ClientCharacter* a2, int a3, int a4,
                      unsigned short a5, unsigned short a6, int a7,
                      const char* a8, const char* a9, int a10,
                      const char* a11, const char* a12,
                      int a13, int a14, int a15, int a16, int a17,
                      int a18, int a19, int a20, int a21, int a22,
                      const char* a23, const char* a24,
                      unsigned int a25, int a26, int a27, int a28, int a29,
                      unsigned char a30, unsigned char a31, unsigned char a32,
                      int a33, int a34);

    // --- 角色狀態通用設置 --------------------------------------------------
    void SetUse(unsigned int account, int use);
    void ResetAutoAttack(unsigned int account);

    // --- AutoAttack/Move 相關 ---------------------------------------------
    int  ProcAutoAttack();
    void SetContinueAutoAttack();
    void MoveForAttack(unsigned short skillKind);
    void DiedPlayer(int a2);
    void SetResurrectPlayer(unsigned int account, unsigned short mp);
    void AutoResurrect();

    // --- 位置 -------------------------------------------------------------
    void SetPosition(unsigned int account, unsigned int x, unsigned int y, int speed);
    void SetCurPosition(unsigned int account, int x, int y);
    int  GetPositionX(unsigned int account);
    int  GetPositionY(unsigned int account);
    int  GetCurPosX(unsigned int account);
    int  GetCurPosY(unsigned int account);
    void SetSpeed(unsigned int account, float speed);
    double GetSpeed(unsigned int account);

    void SetMyPosition(int x, int y);
    int  GetMyPositionX();
    int  GetMyPositionY();
    int  GetMyCurPosX();
    int  GetMyCurPosY();
    void SetMySpeed(float speed);
    double GetMySpeed();

    unsigned int GetMyAttackMonster();
    unsigned int GetMyTargetMonster();
    void GetMySearchMonRange(unsigned short* outX, unsigned short* outY);

    char* GetMyCharName();

    void  SetMyGainExp(unsigned int exp);
    unsigned int GetMyGainExp();
    void  ViewMyGainExp(unsigned int exp);
    void  ResetMyGainExp();

    void  SetRunState(unsigned int account, int state);
    void  ReleaseMyFastRun();
    void  SetKeepRunState();
    void  ReleaseKeepRunState();

    char* GetPlayerName(unsigned int account);
    void  SetCharName(unsigned int account, char* name);
    char* GetCharName(unsigned int account);

    void  SetChatMsg(unsigned int account, char* source);
    void  SetChatMsg(char* charName, char* source);
    void  ClearerChatMsg(unsigned int account);
    void  ClearerChatMsg(char* charName);

    void  CharKeyInputProcess();

    unsigned short GetCharWidthA(unsigned int account);
    unsigned short GetCharHeight(unsigned int account);
    unsigned short GetMonsterInfoPosY(unsigned int account);

    void  SetMyHP(int hp);
    void  SetMyTempHP(int hp);
    void  UseHPPotion(int amount);
    int   GetMyHP();
    void  SetHP(unsigned int account, int hp);
    ClientCharacter* GetHP(unsigned int account);
    void  SetTempHP(unsigned int account, int hp);

    unsigned short GetMapID(unsigned int account);

    void          SetSearchMonster(unsigned int account);
    unsigned char SearchMonsterInSkillRange(unsigned short skillKind);
    void          SetMyCharLR(int lr);
    int           GetMyCharLR();
    void          SetCharLR(unsigned int account, int lr);
    void          SearchAttackMonster();
    unsigned int  GetUseSkillAttackMonAccount(unsigned short rangeX,
                                              unsigned short rangeY);
    void          SetRealAttackMonAccount(unsigned int account);

    void Attack(unsigned int attacker, unsigned int target,
                char a4, int damage, int remainHP, char a7);
    void AttackedBySkill(unsigned int attackerAccount,
                         unsigned short skillKind,
                         unsigned int* const targetAccounts,
                         unsigned char (*const motions)[3],
                         int (*const damages)[3],
                         int* const remainHPs,
                         unsigned char* const hitInfos);
    void AttackedByMine(unsigned int attackerAccount,
                        unsigned short attackerKind,
                        unsigned int mineHandle,
                        unsigned short skillKind,
                        unsigned int* const targetAccounts,
                        unsigned char (*const motions)[3],
                        int (*const damages)[3],
                        int* const remainHPs,
                        unsigned char* const hitInfos);

    void          SearchPlayer();
    void          SetSearchPlayer(unsigned int account);
    unsigned int  GetSearchPlayerAccount();
    void          SetSearchDiedPlayer(unsigned int account);
    unsigned int  GetSearchDiedPlayerAccount();
    void          ResetSearchPlayer();

    void IsInPortralArea(unsigned short mapID);
    void SetMyWarp(int a2, unsigned short mapKind, int a4, int a5,
                   unsigned short worldKind);

    void SetItem(unsigned int account, unsigned short itemKind, int qty);
    void SetMyItem(unsigned short itemKind, int qty);
    void ResetMyItem(unsigned char slot);
    void ResetItem(unsigned int account, unsigned char slot);
    void SetMyCAClone();

    unsigned int     MouseOverCharacter(int x, int y, int isMonster);
    ClientCharacter* MouseOverCharacterPtr(int x, int y, int isMonster);
    ClientCharacter* MouseTargetMonster();
    ClientCharacter* MouseTargetMonsterPtr();

    bool           IsMonster(unsigned int account);
    bool           IsMonster(unsigned short kindCode);
    bool           IsPlayer(unsigned short kindCode);
    unsigned short GetCharKind(unsigned int account);
    unsigned short GetMonsterName(unsigned short kindCode);

    void PrepareDrawingEtcMark();
    void DrawEtcMark();
    void SetDrawInfoText(unsigned int account, int active);

    CCA* GetCAData(unsigned int account, unsigned char* outSex,
                   int* outA3, int* outA4, unsigned int* outA5,
                   unsigned short* outA6);
    void SetCharState(unsigned int account, unsigned char state);

    stCharKindInfo*    GetCharKindInfo(unsigned short kindCode);
    cltMonsterAniInfo* GetMonsterAniInfo(unsigned short kindCode);

    void SetEmoticonKind(unsigned int account, int emoticonKind);
    void SetEmoticonKind(char* charName, int emoticonKind);
    void ReleaseEmoticon(ClientCharacter* pChar);

    ClientCharacter* GetMyTransformationed();
    bool             GetMyTransformationed_Bool() {
        return GetMyTransformationed() != nullptr;
    }
    void             SetSpiritSpeech(int a2);

    int              CanCreatePrivateMarket();
    int              CanCreatePrivateMarket(unsigned int account);
    ClientCharacter* CanJoinPrivateMarket(unsigned int account);

    void SetCanNotChatting(int active);
    int  CanNotChatting();
    void CreateClientEffect();
    void ChangeMySex();

    void SetMapConqueror(char* name);
    bool IsMapConqueror(char* name);
    int  IsGM(char* name);
    void MoveMapFail();
    void SetViewPartyMember(unsigned int account);

    void CreateCoupleEffect();
    void CreateCoupleEffect(ClientCharacter* a2, int a3);
    void DeleteCoupleEffect(unsigned int account);

    void SetMoveTarget(int x, int y);
    void ResetMoveTarget();

    void SetTransport(unsigned int account, unsigned short transportKind,
                      unsigned char a4);

    // mofclient.c：cltInstansDungeonPortal 在原始 binary 是 manager 內嵌
    //   成員（unk_18C4D0C 即此實體位址）。Map::CreateMap 等外部需要呼叫
    //   Init/AddPortal 時透過此 getter 拿到參考。
    cltInstansDungeonPortal& GetInstansDungeonPortal() { return m_clInstansDungeonPortal; }

    // --- 對齊舊有 callsite 的 helper -------------------------------------
    bool GetMyTransformationed() const {
        return const_cast<ClientCharacterManager*>(this)
                   ->GetMyTransformationed() != nullptr;
    }

private:
    // ---------- 我的角色狀態 ---------------------------------------------
    unsigned int     m_dwMyAccount{ 0 };       // loc_43E090
    unsigned int     m_dwMyAccountSlot{ 0 };   // loc_43E092+2 (slot index 1..299)
    int              m_iCharLiveCount{ 0 };    // loc_43E097+1 — Poll 內計數
    char             m_szMapConquerorName[64]{}; // loc_43E09B+1
    ClientCharacter* m_pMyCharacter{ nullptr };// loc_43E0DC

    // ---------- 搜尋 / 攻擊狀態 ------------------------------------------
    unsigned int     m_dwSearchAttackMonAccount{ 0 }; // loc_4BE0F3+1
    unsigned int     m_dwSearchMonsterAccount{ 0 };   // loc_4BE12F+1
    unsigned int     m_aSearchMonsterInRange[40]{};   // loc_4BE133+1（最多40）
    unsigned int     m_dwSearchMonInRangeCount{ 0 };

    int              m_iSpiritSpeech{ 0 };            // loc_4BE92B+1
    unsigned int     m_dwSearchPlayerAccount{ 0 };    // loc_4BE930
    unsigned int     m_dwSearchDiedPlayerAccount{ 0 };// loc_4BE933+1
    int              m_iKeepRunStateFlag{ 1 };        // loc_4BE935+3 (default 1)

    int              m_iCreateClientEffectFlag{ 0 };  // loc_4BE966+2
    int              m_iAttackReady{ 1 };             // loc_4BE96B+1 (default 1)
    int              m_iIsAttackProc{ 0 };            // loc_4BE96D+3

    unsigned int                  m_dwLastWarpFailTime{ 0 };    // loc_4BEA74
    CEffect_Player_MapConqueror*  m_pMapConquerorEffect{ nullptr };  // loc_4BEA74+4
    CEffectBase*                  m_pBeginningEffect{ nullptr };     // loc_4BEA7C+4
    unsigned short   m_wMoveForAttackTarget{ 0 };     // loc_4BEA83+1
    unsigned int     m_dwViewCharAccount{ 0 };        // loc_4BEA8C
    int              m_iMoveTargetX{ 0 };             // loc_4BEA8C+4
    int              m_iMoveTargetY{ 0 };             // loc_4BEA93+1
    bool             m_bMoveTargetActive{ false };    // loc_4BEA98 (byte)
    unsigned int     m_dwAutoAttackTimer{ 0 };        // loc_4BFA44

    // ---------- 相依的 helper 子系統 -------------------------------------
    cltTargetMark             m_clTargetMark;
    cltAutoMoveTarget         m_clAutoMoveTarget;
    cltKeepRunState           m_clKeepRunState;
    cltInstansDungeonPortal   m_clInstansDungeonPortal;

    // mofclient.c 在 manager 內嵌一份 cltClientCharKindInfo（dword_43E0E8）。
    // 我們改持指標，由 ctor/dtor 負責生命週期；介面與內嵌等價。
    cltClientCharKindInfo*    m_pClientCharKindInfo{ nullptr };
};
