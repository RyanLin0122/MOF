#pragma once
#include <cstdint>
#include <d3dx9math.h>

// ----------------------------------------------------------------------------
// Forward declarations (keep the header light; full types are pulled in by the
// .cpp files that actually touch them).
// ----------------------------------------------------------------------------
class CCA;
class CCANormal;
class GameImage;
class CEffectBase;
class CControlChatBallon;
class CControlAlphaBox;
class cltMoF_CharacterActionStateSystem;
class cltPetObject;
class clTransportObject;
class cltCharKindInfo;
class Map;
class CMoFNetwork;
class ClientCharacterManager;

// ----------------------------------------------------------------------------
// stCharOrder — command-queue entry.
//
// mofclient.c initialises each slot as 36 bytes (see constructor loop at
// 30627-30646).  The layout (guessed from SetOrder* callers at 29083+):
//   +0  int type        (0=stop, 1=move, 2=attack, 3=die, 4=hitted,
//                        5=spell, 6=pray, 7=died, 8=transformation)
//   +4  int param0      (targetAccount, dest X, etc.)
//   +8  int param1      (dest Y, damage, etc.)
//   +12 int param2
//   +16 int param3
//   +20 int param4
//   +24 uint16 a         (motion id)
//   +26 uint16 b         (skill kind)
//   +28 uint16 c
//   +30 uint8  d
//   +31 uint8  e
//   +32 uint8  f
//   +33 uint8  pad[3]
// ----------------------------------------------------------------------------
struct stCharOrder {
    int       type;     // +0
    int       p0;       // +4
    int       p1;       // +8
    int       p2;       // +12
    int       p3;       // +16
    int       p4;       // +20
    uint16_t  w0;       // +24
    uint16_t  w1;       // +26
    uint16_t  w2;       // +28
    uint8_t   b0;       // +30
    uint8_t   b1;       // +31
    uint8_t   b2;       // +32
    uint8_t   _pad[3];  // +33..35
};
static_assert(sizeof(stCharOrder) == 36, "stCharOrder must match decomp 36-byte layout");

// ----------------------------------------------------------------------------
// ClientCharacter — the in-world representation of any character (player,
// monster, NPC).
//
// LAYOUT POLICY
// -------------
// The decompilation is 32-bit (pointers = 4 bytes) so its raw offsets like
// *((_DWORD *)this + 1109) do not line up byte-for-byte with a 64-bit x64
// build (pointers = 8 bytes).  We therefore CANNOT reproduce the exact
// offsets; we reproduce the *semantics* instead:
//
//   * Each logical field is declared as a named member and always accessed
//     by that name, never by raw offset — so the storage for a given logical
//     field is consistent regardless of where the compiler places it.
//
//   * Embedded subsystems that mofclient.c constructs at fixed offsets
//     (CControlChatBallon at +4448, CControlAlphaBox ×4 at +7452/+7660/
//     +7868/+9732, cltMoF_CharacterActionStateSystem at +11544, cltPetObject
//     at +11564, clTransportObject at +14616) are stored here as owned
//     pointers — allocated in the ctor and released in the dtor.  Callers
//     translate "(char*)this + <offset>" in the decomp to "m_p<Subsystem>".
//
//   * A trailing raw-byte pad is reserved so that any legacy ported code
//     still using reinterpret_cast<T*>(this)[N] for unidentified offsets
//     does not walk off the end of the object and corrupt the next slot
//     in unk_1409D80[1000].  New ports should prefer named fields.
// ----------------------------------------------------------------------------
class ClientCharacter {
public:
    ClientCharacter();
    ~ClientCharacter();

    // =========================================================================
    // Static environment (mofclient.c: ClientCharacter::m_pClientCharKindInfo
    // / m_pMap / m_pNetwork / m_pCharMgr — installed by InitializeStaticVariable
    // before any character exists).
    // =========================================================================
    static cltCharKindInfo*        m_pClientCharKindInfo;
    static Map*                    m_pMap;
    static CMoFNetwork*            m_pNetwork;
    static ClientCharacterManager* m_pCharMgr;

    static void InitializeStaticVariable(cltCharKindInfo* charKindInfo,
                                         Map* map,
                                         CMoFNetwork* network,
                                         ClientCharacterManager* charMgr);

    // =========================================================================
    // Lifecycle / flags
    // =========================================================================
    void  InitFlag();
    void  ResetCharEffect();
    void  DeleteCharacter();

    // CreateCharacter has 35 params in the decomp.  Keep a forwarding helper
    // with the full signature for completeness and a lighter helper used by
    // ClientCharacterManager::AddCharacter.
    void  CreateCharacter(ClientCharacter* self,
                          int posX, int posY,
                          unsigned short charKind, unsigned short mapID,
                          int hp,
                          char* name, char* circleName, unsigned short circleMark,
                          char* coupleName, char* petName, unsigned short petKind,
                          unsigned char ucUnknown14, char cUnknown15,
                          unsigned char sex, unsigned int accountID,
                          char cUnknown18, unsigned char hair,
                          int iUnknown20, unsigned char face,
                          unsigned char ucUnknown22, unsigned short classCode,
                          unsigned short transformKind, unsigned char ucUnknown25,
                          int iUnknown26, char cUnknown27, char* privateMarketMsg,
                          unsigned short ucPCRoom, unsigned short weaponKind,
                          unsigned short shieldKind, char* screenName,
                          unsigned char nation, unsigned short titleKind,
                          int iUnknown35);

    // =========================================================================
    // Core update / draw
    // =========================================================================
    // mofclient.c 0x004A2854：tick 參數實際上是 cltTargetMark* 強轉成 int
    // (manager Poll 把 m_clTargetMark 位址傳進來當 ebp 參數)。在 32 位元
    // 反編譯下指標寬度等於 int，不會遺失資料；在 x64 下則會。
    // 這裡改成顯式接 void* 以避免 64-bit 截斷；body 內把它再轉回 int 後
    // 沿原語意傳給 OrderHitted / PollHPBox。
    int   Poll(void* targetMark);
    void  PollHPBox(int tick);
    void  PrepareDrawingChar();
    void  PrepareDrawingPlayer();
    void  PrepareDrawingMonster();
    void  PrepareDrawingHPBox();
    void  PrepareDrawingCharActionState();
    void  DrawChar(int alpha);
    void  DrawPlayer(int alpha);
    void  DrawMonster(int alpha);
    void  DrawHPBox();
    void  DrawCharActionState();
    void  DrawTextA(int alpha);
    void  DecideDrawFrame(unsigned int* outResourceID, unsigned short* outBlockID);
    void  DecideDrawFrame();
    void  InitScreenName(int alpha);

    // =========================================================================
    // Position
    // =========================================================================
    int   GetPosX() const;
    int   GetPosY() const;
    int   GetPosX();
    int   GetPosY();
    int   GetPositionX();
    int   GetPositionY();
    void  SetPosX(float x);
    void  SetPosY(float y);
    unsigned short GetCharWidthA();
    unsigned short GetCharHeight();
    unsigned short GetCharHeight() const;
    static void SetCurPosition(ClientCharacter* pChar, int x, int y);
    static void SetEndPosition(ClientCharacter* pChar, int x, int y);

    // =========================================================================
    // Stats
    // =========================================================================
    int   GetHP() const;
    void  SetHP();
    void  SetUseHPPotionEffect(int value);
    void  SetMP(int mp);
    int   GetMP();
    void  SetMaxMP(int mp);
    int   GetMaxMP(int a2);
    void  SetCharState(char state);
    void  SetGainExp(unsigned int exp);
    void  ResetGainExp();
    void  GainExp(int exp);

    // =========================================================================
    // Order queue
    // =========================================================================
    void  ClearOrder();
    void  ProcessOrder();
    void  PushOrder(stCharOrder* pOrder);
    void  PopOrder(stCharOrder* pOrder);
    int   MergeHittedOrder(stCharOrder* pOrder);
    unsigned int GetLastOrder();

    // OnOrder* — dispatch handlers invoked from ProcessOrder
    void  OnOrderStop(stCharOrder* pOrder);
    void  OnOrderMove(stCharOrder* pOrder);
    void  OnOrderAttack(stCharOrder* pOrder);
    void  OnOrderDie(stCharOrder* pOrder);
    void  OnOrderDied(stCharOrder* pOrder);
    void  OnOrderHitted(stCharOrder* pOrder);
    void  OnOrderPray(stCharOrder* pOrder);
    void  OnOrderSpell(stCharOrder* pOrder);
    void  OnOrderTransformation(stCharOrder* pOrder);

    // SetOrder* — construct an order payload
    void  SetOrderStop(stCharOrder* pOrder);
    void  SetOrderMove(stCharOrder* pOrder, unsigned int dx, unsigned int dy, unsigned int speed);
    void  SetOrderDie(stCharOrder* pOrder);
    void  SetOrderDied(stCharOrder* pOrder);
    int   SetOrderHitted(stCharOrder* pOrder, unsigned char motion,
                         int damage, int remainHP, char a6, char a7,
                         unsigned short skillKind, unsigned char hitInfo);
    void  SetOrderAttack(stCharOrder* pOrder, unsigned int targetAccount,
                         char a4, int damage, int remainHP, char a7,
                         unsigned short skillKind);
    // Tutorial-style 7-parameter SetOrderAttack (kept for existing callers).
    void  SetOrderAttack(stCharOrder* pOrder, unsigned int targetAccount,
                         int a3, int damage, int remainHP, int a6, int a7);
    void  SetOrderPray(stCharOrder* pOrder, int a3, int a4, int a5, unsigned char a6);
    void  SetOrderSpell(stCharOrder* pOrder);
    void  SetOrderTransformation(stCharOrder* pOrder, unsigned short transformKind);

    // Order* — execute the currently active order
    void  OrderStop();
    void  OrderMove();
    void  OrderAttack();
    void  OrderTripleAttack();
    void  OrderDoubleAttack();
    int   OrderDie();
    int   OrderDied();
    void  OrderHitted(int alpha);
    void  OrderPray();
    void  OrderSpell();
    void  OrderTransformation();
    void  OrderHit();

    void  SetActionState(unsigned int state);

    // =========================================================================
    // Effect creation
    // =========================================================================
    void  CreateCreateEffect(int a2);
    void  SetEffect_Healed_Self_BySkill(unsigned char a2);
    void  SetEffect_Healed_Other_BySkill(int a2, unsigned char a3);
    void  SetEffect_HealNum(int a2);
    void  SetEffect_HealedByTree();
    void  SetEffect_ResurrectBySkill(int a2, unsigned char a3);
    void  SetEffect_Debuffer_Damage(int a2, unsigned short a3);
    void  SetEffect_FireBody(unsigned short a2);
    int   CreateHittedSkillEffect(unsigned char a2);
    void  CreateNormalAttackEffect(unsigned char motionType, unsigned int targetAccountID, unsigned char hitInfo);
    void  CreateUseSkillEffect();
    void  WorkingSkillEffect(unsigned short a2);
    void  SetEffect_Sustain(unsigned short skillKind, int a3);
    void  CreateMonsterShootingObject(ClientCharacter* target);
    void  CreateHitCharKindEffect();
    void  SetEffect_OverMind();
    void  SetEmblem_Effect();
    void  CreateGiveBouquet();
    void  CreateSpecialStateEffect(unsigned char state);
    void  SetEffect_Cracker(unsigned short a2);
    void  CreateWeaponEffect(unsigned short a2, unsigned char a3);
    void  CreateLiveMCEffect(char* Str1);
    void  CreateCoupleEffect(unsigned short a2, int a3);
    void  DeleteCoupleEffect();

    // =========================================================================
    // Combat / attack / skill state
    // =========================================================================
    void  ResetAttackFlag();
    void  AttackMonster();
    void  SetSearchMonster(unsigned int accountID);
    unsigned int GetSearchMonster();
    void  SetAttackMonster(unsigned int accountID);
    unsigned int GetAttackMonster();
    void  SetWeaponRange(unsigned short a2, unsigned short a3, unsigned short a4, unsigned short a5);
    void  GetWeaponSearchRangeXY(unsigned short* outX, unsigned short* outY);
    void  GetWeaponAttackRangeXY(unsigned short* outX, unsigned short* outY);
    void  SetWeaponType(unsigned short a2, int a3);
    char  GetAttackMotion();
    void  SetTripleAttackMotion();
    void  SetDoubleAttackMotion();
    void  SetCanAttack(void (*a2)(unsigned int, unsigned int));
    void  SetAttackSpeed(unsigned int a2, unsigned char a3);
    unsigned int GetAttackSpeed();
    unsigned char GetAttackAniRate();
    int   CanAttackSpecialState();
    int   CanMoveSpecialState();
    void  SetUseSkillKind(unsigned short a2, unsigned int* const a3);
    unsigned short GetUseSkillKind();
    int   ResetUseSkillKind();
    void  SetHittedSkillKind(unsigned short a2);
    unsigned short GetHittedSkillKind();
    void  ResetHittedSkillKind();
    void  HitMissed();

    // =========================================================================
    // Hitted info (per-caster hit accumulation table)
    // =========================================================================
    int   GetHitedInfoNum(unsigned int casterAccountID);
    int   GetHitedInfoNum(int a);
    int   SearchEmptyHitedInfoIndex();
    void  SetCharHitedInfo(unsigned int a2, unsigned int a3, char a4, char a5,
                           int a6, int a7, char a8, char a9,
                           unsigned short a10, unsigned char a11);
    void  ResetHitedInfo(int slot);
    void  SetNormalHitInfo(unsigned char a2, unsigned int a3, char a4,
                           int a5, int a6, unsigned char a7);
    void  SetHited(int hitInfoID, int value);

    // =========================================================================
    // Pet / transport / emoticon / transform
    // =========================================================================
    void  SetEmoticonKind(int emoticonKind);
    void  ReleaseEmoticon();
    void  SetTransformation(unsigned short transformKind);
    void  ReleaseTransformation();
    // mofclient.c 32208：玩家是否處於變身狀態（DWORD+169 != 0）。
    int   IsTransformed() const { return m_iInitFlag_153_176[169 - 153]; }
    void  SetDied();
    void  SetPetKind(unsigned short petKind, int a3);
    void  CreatePet(unsigned short petKind, int a3);
    void  DyePet(unsigned short petKind);
    void  SetTransportKind(unsigned short transportKind);
    void  SetTransportActive(int active);
    int   GetPetPosX();
    int   GetPetPosY();
    void  AutoResurrect();
    void  SetPvPRankKind(unsigned short rank);
    void  DyeHairColor(unsigned int color);
    void  ResetHairColorKey();
    void  Divoced(int a2);
    void  ChangeCoupleRing(unsigned short ringKind);

    // =========================================================================
    // Sustain / special state
    // =========================================================================
    void  DeleteSustainEffect();
    void  DeadProcSustainEffect();
    void  StartSustainEffect(char a2);
    void  EndAllSustainSkill();
    void  EndSpiritShield();
    void  EndLifeAura();
    void  EndAttackAura();
    int   GetSustainSkillState(unsigned short skillID) const;
    bool  GetSustainSkillState(unsigned short a);
    void  EndSustainSkillState(unsigned short skillID);
    int   SetSpecialState(unsigned char state);
    void  SetSpecialStatePos(int x, int y);

    // =========================================================================
    // Social / UI
    // =========================================================================
    void  SetCircleName(char* name);
    void  SetCircleMasterMark(unsigned short mark);
    void  SetPrivateMarketMsg(char* msg);
    void  SetCharHide(int a2, unsigned char a3);
    void  SetGM(unsigned char a2, char* a3);
    void  ProcInfoPosY();
    void  SetPCRoomUser(int a2, unsigned char a3);
    void  SetClassCode(unsigned short classCode, int updateNameTag);
    void  SetNameTagInfo(unsigned char classMark, int updateNameTag);
    void  SetNameBoxColor(unsigned char a2, int a3);
    void  PlayerLevelUp(char a2);
    void  StartHidingSelf();
    int   GetHidingSelf();
    void  FinishHidingSelf();

    // =========================================================================
    // Chat
    // =========================================================================
    void  SetChatMsg(char* source);
    void  ClearerChatMsg();

    // =========================================================================
    // Run / movement
    // =========================================================================
    void  SetMyRunState();
    void  SetRunState(unsigned char state);
    void  ProcRunState();
    void  ReleaseKeepRunState();
    void  ReleaseMyFastRun();
    void  MoveCharacter();
    int   MapWarpProc();
    void  SetWarp(int a2, unsigned short mapID, int destX, int destY, unsigned short a6);
    void  InitCreateWarpEffectFlag();

    // =========================================================================
    // Identity / item slot management
    // =========================================================================
    void  SetItem(unsigned short itemKind, int qty);
    void  ResetItem(unsigned char slot);
    void  SetCAClone();

    // mofclient.c 30303：把這個 ClientCharacter 的 CA 描述資訊回填到輸出指標
    // （sex / posX / posY / accountID / kind），回傳 m_pCCA。
    CCA*  GetCAData(unsigned char* outSex, int* outA3, int* outA4,
                    unsigned int* outA5, unsigned short* outA6) {
        if (outSex) *outSex = m_ucSex;
        if (outA3)  *outA3  = m_iPosX;
        if (outA4)  *outA4  = m_iPosY;
        if (outA5)  *outA5  = m_dwAccountID;
        if (outA6)  *outA6  = m_wKind;
        return m_pCCA;
    }
    int   GetAccountID();
    int   IsMyChar();
    bool  IsHide();
    bool  IsTransparent();
    int   GetActionSide() const;
    int   GetActionSide();

    // =========================================================================
    // Static timer callbacks
    // =========================================================================
    static void OnTimer_ClearChatMsg(unsigned int timerID, ClientCharacter* pChar);
    static void OnTimer_ResetAttackSpeed(unsigned int timerID, ClientCharacter* pChar);
    static void OnTimer_ResetAttackKey(unsigned int timerID, ClientCharacter* pChar);

    // =========================================================================
    // Accessors (inline — used by many UI/debug callers)
    // =========================================================================
    unsigned short GetMapID() const { return m_wMapID; }
    unsigned int   GetActionState() const { return m_dwActionState; }
    int            GetSomeOtherState() const { return m_someOtherState; }
    bool           IsFashionItemActive() const { return m_fashionItemActive; }
    char*          GetName() { return m_szName; }
    bool           IsPCRoomUser() const { return m_isPCRoomUser; }
    bool           IsPCRoomPremium() const { return m_isPCRoomPremium; }
    bool           CanMove() const { return m_canMove; }
    bool           IsFreezing() const { return m_isFreezing; }
    bool           IsIntegrityActive() const { return m_isIntegrityActive; }
    void           SetCanSpeedUp(bool value) { m_canSpeedUp = value; }
    void           SetPreparingSpeedUp(bool value) { m_preparingSpeedUp = value; }

    // =========================================================================
    // Named data members — PRIMARY STORAGE for all logical state.
    //
    // These are the fields accessed by callers and by restored methods.
    // Offsets noted in comments reference the 32-bit decomp layout; on x64
    // the compiler will place them differently but behaviour is preserved
    // because the same field name always refers to the same storage.
    // =========================================================================

    // --- Pointers to renderable assets (decomp +0..+64) --------------------
    void*            m_pVftable;             // +0   (decomp uses as generic non-null flag)
    GameImage*       m_pShadowImage;         // +4
    GameImage*       m_pShadowImage_Sub;     // +8
    void*            m_pMonsterAniInfo;      // +16
    unsigned int     m_dwUnknown_20;         // +20
    unsigned int     m_dwUnknown_24;         // +24
    unsigned int     m_dwUnknown_28;         // +28
    unsigned int     m_dwUnknown_32;         // +32
    GameImage*       m_pGogglesImage;        // +60
    unsigned short   m_wUnknown_64;          // +64
    unsigned int     m_dwAttackSpeed;        // +68
    unsigned char    m_ucAttackAniRate;      // +72

    // --- Small-state array (decomp memset'd from +80, 200 bytes) -----------
    unsigned char    m_acEquipKind[200];     // +80..+279 (includes equip/hair slots)

    // --- Digit frames / misc motion state (decomp +280..+359) ---------------
    unsigned char    m_ucDigitFrames_Unk[10];      // +280..+289
    unsigned char    m_ucDoubleAttackMotion[2];    // +286 (overlaps; kept for legacy callers)
    unsigned char    m_ucAttackMotionType;         // +288
    unsigned char    m_ucAlpha;                    // +312
    int              m_iUnknown_348;               // +348 (DWORD+87)
    int              m_iUnknown_352;               // +352 (DWORD+88)
    int              m_iUnknown_356;               // +356 (DWORD+89)
    int              m_iUnknown_372;               // +372 (DWORD+93)
    int              m_iUnknown_380;               // +380 (DWORD+95)

    // --- Identity (decomp +432..+467) --------------------------------------
    int              m_iUnknown_432[6];            // +432..+455 (DWORD+108..113)
    unsigned int     m_dwAccountID;                // +456

    // --- Name / circle / classCode (decomp +460..+607) ---------------------
    char             m_szName[128];                // +460
    unsigned short   m_wKind;                      // +568
    int              m_dwLR_Flag;                  // +572
    unsigned short   m_wMapID;                     // +592
    unsigned char    m_ucSex;                      // +594
    unsigned char    m_ucHair;                     // +595
    unsigned char    m_ucFace;                     // +596
    char             m_cCharState;                 // +604

    // --- Flag-word region zeroed by InitFlag/ctor (decomp +612..+707) -------
    int              m_iInitFlag_153_176[24];      // +612..+707 (DWORD+153..176)
    int              m_dwTransformation;           // +676 overlaps flag-word region (DWORD+169)

    // --- Currently-active order (decomp +708..+743, 36 bytes, stCharOrder
    //     copy written by ProcessOrder after each PopOrder).
    stCharOrder      m_currentOrder;               // +708..+743

    // --- Order queue (decomp +744..+4343, 100 entries × 36 bytes) -----------
    stCharOrder      m_OrderQueue[100];            // +744..+4343

    // --- Order queue count (decomp DWORD+1086 / byte offset +4344) ----------
    int              m_nOrderCount;                // +4344

    // --- Low-offset CCA / positions (decomp +4348..+4447) -------------------
    CCA*             m_pCCA;                       // +4348
    CCANormal*       m_pCCANormal;                 // +4352
    unsigned int     m_dwUnknown_4356;             // +4356 (DWORD+1089)
    unsigned int     m_dwUnknown_4360;             // +4360 (DWORD+1090)
    unsigned int     m_dwUnknown_4368;             // +4368 (DWORD+1092)
    unsigned char    m_ucUnknown_4374;             // +4374
    int              m_iPosX;                      // +4384
    int              m_iPosY;                      // +4388
    int              m_iPrevPosX;                  // +4392
    int              m_iPrevPosY;                  // +4396
    int              m_iDestX;                     // +556 (kept for legacy names)
    int              m_iDestY;                     // +560
    float            m_fMoveSpeed;                 // +564
    unsigned char    m_ucUnknown_4400;             // +4400
    unsigned short   m_wUnknown_4432;              // +4432
    unsigned int     m_dwSlotAlive;                // +4436 — "character slot in use" flag

    // --- Rendering / HP-box state (decomp +7372..+9731) ---------------------
    int              m_iUnknown_7372;              // +7372
    int              m_iUnknown_7376;              // +7376
    int              m_iUnknown_7380;              // +7380
    unsigned char    m_ucUnknown_7384;             // +7384 (initialised to 2)

    // 自動攻擊鎖定狀態（decomp DWORD+1847..+1849 / +1854..+1856）。
    // 由 ClientCharacterManager::ProcAutoAttack / SearchAttackMonster /
    // CharKeyInputProcess / ClientCharacter::AttackMonster 取讀。
    unsigned int     m_dwAttackSearchTarget;       // +7388 (DWORD+1847) — 鎖定的搜尋目標 account
    unsigned int     m_dwAttackTargetAccount;      // +7392 (DWORD+1848) — 實際攻擊中的 account
    int              m_iAttackTargetHP;            // +7396 (DWORD+1849) — 鎖定目標 HP 暫存
    int              m_iMoveDirX;                  // +7416 (DWORD+1854) — 鍵盤左右方向 -1/0/1
    int              m_iMoveDirY;                  // +7420 (DWORD+1855) — 鍵盤上下方向 -1/0/1
    unsigned int     m_dwGainExpDisplay;           // +7424 (DWORD+1856) — 待顯示的 GainExp 值

    unsigned char    m_ucUnknown_7434;             // +7434
    unsigned char    m_ucUnknown_7435;             // +7435
    unsigned char    m_ucUnknown_7436;             // +7436
    unsigned int     m_dwUnknown_9676;             // +9676
    unsigned short   m_wUnknown_9680;              // +9680
    unsigned int     m_dwActionState;              // +9684
    bool             m_fashionItemActive;          // +9688
    unsigned short   m_wTotalFrame;                // +9688 overlap (short)
    unsigned short   m_wCurrentFrame;              // +9694
    bool             m_isIntegrityActive;          // +9700 bit0
    bool             m_canMove;                    // +9700 bit1
    bool             m_isFreezing;                 // +9700 bit4
    unsigned char    m_ucNameClassMark;            // +9728
    unsigned short   m_wClassCode;                 // +9712 / +4856
    unsigned short   m_wPvPRankKind;               // +9720 / +4860
    unsigned int     m_dwUnknown_9716;             // +9716

    // --- Screen name (decomp +9984..+10111) ---------------------------------
    char             m_szScreenName[128];          // +9984

    // --- Circle / GM / couple / pet-kind (decomp +10124..+11543) ------------
    char             m_szCircleName[16];           // +10124 (strcmp target in ProcInfoPosY)
    unsigned char    m_ucUnknown_10112;            // +10112
    int              m_someOtherState;             // +11528
    unsigned char    m_ucUnknown_11252;            // +11252
    unsigned int     m_dwUnknown_11256;            // +11256
    unsigned int     m_dwUnknown_11264;            // +11264
    unsigned char    m_ucUnknown_11260;            // +11260

    // 私販 / 交易狀態 (decomp BYTE+11524)。值意義（mofclient.c CreateCharacter
    // 寫入；CanCreatePrivateMarket / CanJoinPrivateMarket 讀取）：
    //   0 = 無；1 = 私販關閉中；2 = 私販開啟中；3 = 一般交易進行中
    unsigned char    m_ucTradeState;               // +11524

    unsigned int     m_dwGM_Level;                 // +11540
    bool             m_isPCRoomUser;               // +11556
    bool             m_isPCRoomPremium;            // +11560

    // --- Transport / tail state (decomp +14616..+14820) ---------------------
    unsigned int     m_dwUnknown_14684;            // +14684

    // 配偶名稱（decomp char[]+14690）。CreateCharacter 由參數 a9 strcpy 進來。
    // ClientCharacterManager::CreateCoupleEffect / DeleteCoupleEffect / 婚禮
    // 系統用 strcmp 比對另一角色的 m_szName 來找配偶 slot。
    char             m_szCoupleName[128];          // +14690

    // 配偶戒指 kind code（decomp WORD+7409 = +14818）。
    unsigned short   m_wCoupleRingKind;            // +14818

    unsigned int     m_dwUnknown_14820;            // +14820
    bool             m_preparingSpeedUp;           // *((_DWORD *)a2 + 174)
    bool             m_canSpeedUp;                 // *(_DWORD *)(v3 + 696)

    // =========================================================================
    // Embedded subsystems — owned by ClientCharacter (decomp constructs them
    // in place at fixed offsets; we allocate on the heap in the ctor and
    // delete in the dtor so the header can stay free of their headers).
    // =========================================================================
    CControlChatBallon*                m_pChatBallon;            // +4448
    CControlAlphaBox*                  m_pHpBoxBack;             // +7452
    CControlAlphaBox*                  m_pHpBoxFront;            // +7660
    CControlAlphaBox*                  m_pHpBoxThird;            // +7868
    CControlAlphaBox*                  m_pNameBox;               // +9732
    cltMoF_CharacterActionStateSystem* m_pActionStateSystem;     // +11544
    cltPetObject*                      m_pPetObject;             // +11564
    clTransportObject*                 m_pTransportObject;       // +14616

    // =========================================================================
    // Safety pad — keeps any legacy reinterpret_cast<T*>(this)[N] access that
    // was written against the 32-bit decomp from corrupting adjacent elements
    // of unk_1409D80[1000].  Sized to comfortably cover the decomp's maximum
    // offset (~14820 bytes) plus embedded subsystems placed before us.
    //
    // Ported methods access unnamed decomp offsets via Decomp<T>(N) — N is
    // the raw byte offset from the 32-bit decomp, stored in m_safetyPad so
    // every read and write of the same offset hits the same storage.  Once
    // an offset gets a proper named field, callers should switch to that.
    // =========================================================================
    unsigned char    m_safetyPad[20480];

    template <typename T>
    T& Decomp(size_t decompByteOffset) {
        return *reinterpret_cast<T*>(m_safetyPad + decompByteOffset);
    }
    template <typename T>
    const T& Decomp(size_t decompByteOffset) const {
        return *reinterpret_cast<const T*>(m_safetyPad + decompByteOffset);
    }
};

// Legacy enum aliases used by this class before structured layout was adopted.
// Some callers still reference them.
using _stCharOrder = stCharOrder;
