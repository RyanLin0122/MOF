#pragma once
#include <iostream>
#include <stdio.h>     // 用於 printf

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>   // 用於 HWND, GetConsoleWindow, Sleep
#include <mmsystem.h>  // 用於 timeGetTime
#include <conio.h>     // 用於 _kbhit, _getch (檢查鍵盤輸入)
#include <d3d9.h>
#include "Text/cltTextFileManager.h"
#include "Text/DCTTextManager.h"
#include "Font/MoFFont.h"
#include "Text/DCTIMMList.h"
#include "Util/cltTimer.h"

class cltClassKindInfo;
class cltItemKindInfo;
class cltNPCInfo;
class cltMapInfo;
class Map;
class ClientCharacter;
class ClientCharacterManager;
class CAManager;
class cltQuestSystem;
class CMeritoriousSystem;
class cltLevelSystem;
class cltClassSystem;
class CSpiritSystem;
class cltBaseInventory;
class cltCharKindInfo;
class cltFieldItem;
class cltFieldItemManager;
class cltTimerManager;
class cltHelpMessage;
class cltConfiguration;
class CUITutorial;
class CUIManager;
class cltNPCManager;
class CObjectManager;
class CObjectResourceManager;
class DrawObject_All;
class CMoFNetwork;
class CEffectManager;
class cltMapTitle;
class cltMoF_MineManager;
class cltMatchManager;
class cltPKManager;
class cltPKFlagManager;
class CSpiritSpeechMgr;
class clClientTransportKindInfo;
class clTransportKindInfo;
class cltPetKindInfo;
class cltClientPetKindInfo;
class cltPortalInfo;
class cltClientPortalInfo;
class cltPetSystem;
class cltMoneySystem;
class cltMoFC_EffectKindInfo;
class cltMyCharData;
class cltSystemMessage;
class CDeviceManager;
class CSpiritSpeechParser;
class CSpiritGossipParser;
class cltQuestKindInfo;
class cltLessonSystem;
class cltSkillSystem;
class cltPlayerAbility;
class DirectInputManager;
class cltCashShopItem;
class cltBasicAppearKindInfo;
class GameSound;
class CMessageBoxManager;
class cltChattingMgr;
class cltSkillKindInfo;
class cltLessonKindInfo;
class cltEmblemKindInfo;
class cltSpecialtyKindInfo;
class cltMakingItemKindInfo;
class cltPetSkillKindInfo;
class cltCoupleRingKindInfo;
class cltTransformKindInfo;
class cltEmoticonKindInfo;
class cltPKRankKindInfo;
class cltCharKindInfo;
class cltRegenMonsterKindInfo;
class cltMapUseItemInfoKindInfo;
class cltEquipmentSystem;
class cltSkillSystem;
class cltUsingSkillSystem;
class cltUsingItemSystem;
class cltSexSystem;
class cltPlayerAbility;
class cltBaseAbility;
class cltMarriageSystem;
class cltSpecialtySystem;
class CToolTipManager;
struct strTransformKindInfo;

//-----------------------------------------------------------------------------
// 全域變數
//-----------------------------------------------------------------------------
// --- 全域變數定義 ---
extern int IsInMemory;
extern bool IsDialogBoxMode;
extern int g_LButtonUp;
extern HWND g_hWnd;
extern LPDIRECT3D9 g_pD3D;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern D3DPRESENT_PARAMETERS g_d3dpp;
extern LPDIRECT3DDEVICE9 Device;
extern int SETTING_FRAME;

struct GameSystemInfo {
    int ScreenWidth;
    int ScreenHeight;
    float ScreenX;
    float ScreenY;
};

extern GameSystemInfo g_Game_System_Info;

extern bool DontDraw;
extern unsigned char NationCode;
extern bool g_bRenderStateLocked;

extern short kGlyphIndexByStyleChar[]; //word_6C6B6C
extern int   kDotGlyphIndexByStyle[]; //dword_6C6C54
extern int   kAdvanceByStyle[]; //dword_6C6C58
extern int   kAtlasIdByStyle[]; //dword_6C6C5C

// Camera/scroll offset (world coordinate of the top-left screen corner)
extern int dword_A73088;  // camera scroll X
extern int dword_A7308C;  // camera scroll Y
// (dword_21B8DFC declared below)

// mofclient.c 0x21B8DFC: PK-mode flag (non-zero = PvP map, HP bars visible
// on all chars, dead effect spawns pop a different respawn UI).
extern int dword_21B8DFC;

// mofclient.c 0x6DD7CC: debug-info overlay flag.  When set, InitScreenName
// formats the char's HUD name as "acct:name:x:y:hp:mp" instead of the
// normal "[Lv.N] name" / "name" forms.  Toggled by the ~ debug key.
extern int fInfoFlag;

// -----------------------------------------------------------------------------
// 由 ClientCharacterManager / ClientCharacter 取用的 mofclient.c 雜項全域旗標。
// 全部對齊 ground truth BSS 弱符號（默認 0）。原 IDA 自動命名於 .cpp 註解保留。
// -----------------------------------------------------------------------------
extern unsigned int   g_dwSubGameState;          // mofclient.c：g_dwSubGameState @ 0xB3D6FC
extern int            g_iNPCDialogActive;        // mofclient.c：dword_21B8DC4
extern int            g_iResurrectInstantWarp;   // mofclient.c：dword_21B8DE4
extern int            g_iAutoAttackEnabled;      // mofclient.c：dword_21B8DF0
extern unsigned int   g_wInstantDungeonMapKind;  // mofclient.c：dword_21C9C64
extern int            g_iMouseTargetEnabled;     // mofclient.c：dword_73E30C
extern int            g_bIsMoveMap;              // mofclient.c：g_bIsMoveMap @ 0x22F2A24
extern unsigned short g_wLastSentMoveX;          // mofclient.c：dwlastsentx
extern unsigned short g_wLastSentMoveY;          // mofclient.c：dwlastsenty
extern unsigned int   g_dwDoubleTapTime1;        // mofclient.c：dwKeyDown_1
extern unsigned int   g_dwDoubleTapTime2;        // mofclient.c：dwKeyDown_2
extern int            g_iDoubleTapKeyUp;         // mofclient.c：fUpFlag
extern int            g_iDoubleTapKeyDown;       // mofclient.c：fDownFlag
extern unsigned short g_wMapid;                  // mofclient.c：g_wMapid @ 0x6DD7D0
extern int            g_iMapPixelHeight;         // mofclient.c：dword_A7311C
extern int            g_iPickupKeyHeld;          // mofclient.c：dword_AFD350
extern int            g_bLocalData;              // mofclient.c：g_bLocalData @ 0xB8BB9C

// mofclient.c 0x21BA7B4：cltBasicAppearSystem 在原始 binary 是 cltMyCharData
// 內嵌欄位；本還原把它升級為獨立全域以便 ClientCharacterManager::ChangeMySex
// 與其他系統共享同一份外觀資料。
class cltBasicAppearSystem;
extern cltBasicAppearSystem g_clBasicAppearSystem;

extern unsigned int D3DRS_BLENDOP_TYPE[8];
extern unsigned int D3DRS_SRCBLEND_TYPE[8];
extern unsigned int D3DRS_DESTBLEND_TYPE[8];

extern DWORD g_dwHeartBeatTime;

extern unsigned int g_dwMyAccountID;

// Character-select slot data (filled from server packets; 3 slots × 96 bytes).
// Slot layout (byte offset from slot base = 96 * slotIndex):
//   0x00  char[32]   name           (byte_23158C8)
//   0x20  short      charKind       (word_23158E8)  — AddCharacter
//   0x22  char       nation         (byte_23158EA)
//   0x24  short      charKindInit   (word_23158EC)  — cltMyCharData::Initialize
//   0x26  char       sex            (byte_23158EE)
//   0x27  char       hair           (byte_23158EF)
//   0x28  int        mapKind        (dword_23158F0)
//   0x2C  char       classKind      (byte_23158F4)
//   0x30  uint16[11] equipKind1     (&word_23158F8)
//   0x46  uint16[11] equipKind2     (&word_231590E)
extern char           byte_23158C8[];   // name × 3 slots
extern short          word_23158E8[];   // charKind × 3 slots       (stride: 48 shorts)
extern char           byte_23158EA[];   // nation × 3 slots
extern short          word_23158EC[];   // charKindInit × 3 slots   (stride: 48 shorts)
extern char           byte_23158EE[];   // sex × 3 slots
extern char           byte_23158EF[];   // hair × 3 slots
extern int            dword_23158F0[];  // mapKind × 3 slots        (stride: 24 ints)
extern char           byte_23158F4[];   // classKind × 3 slots
extern unsigned short word_23158F8[];   // equipKind1 array start   (base of 3×11 shorts, stride 96 bytes)
extern unsigned short word_231590E[];   // equipKind2 array start   (base of 3×11 shorts, stride 96 bytes)

// Tutorial globals
// (full class definitions are in their respective headers)
extern int g_nTutorialState;
extern int g_nFieldItemPickupFlag;

enum Direction : uint8_t {
    DirUp = 0,        // 氣球箭頭朝上（置中）
    DirLeft = 1,      // 箭頭朝左（垂直置中）
    DirRight = 2,     // 箭頭朝右（垂直置中）
    DirDown = 3       // 箭頭朝下（置中）== (DirLeft | DirRight)
};

// Class Object

// Text & Font
extern cltTextFileManager       g_clTextFileManager;
extern DCTTextManager           g_DCTTextManager;
extern MoFFont                  g_MoFFont;
extern DCTIMMList               g_IMMList;

// Map
extern Map                      g_Map;
extern unsigned short           dword_21B8DF4; // current map ID (ground truth: 0x21B8DF4)
extern cltMapTitle              g_clMapTitle;
// mofclient.c 0x6E16C8：天氣風向旗標。InitClimate 隨機翻轉，cltClimateUnit
// 在繪製粒子時讀取 g_bDir 寫入 GameImage 的 flip 欄位。
extern int                      g_bDir;

// mofclient.c 0x21B8E08：fight-room session 旗標。
//   - Map::CreateMap 進入非 PvP 地圖時 = 1
//   - CUIFightRoomChannel::CompleteJoin / FailJoin 時 = (dword_AFAF5C == 0)
// 即「目前不在 fight room session」之布林。原 binary 名稱：dword_21B8E08。
extern int                      g_iNotInFightRoomSession;

// mofclient.c 0x230BB48 / 0x230BB4A：寬螢幕 letterbox 區塊（pair A）的
// 螢幕寬高暫存。Map::CreateMap 在 sysW>800 且地圖比螢幕窄時寫入。
// 原 binary 名稱：word_230BB48 / word_230BB4A。
extern unsigned short           g_wLetterboxAW;
extern unsigned short           g_wLetterboxAH;

// mofclient.c 0x230B8D8 / 0x230B8DA：letterbox 區塊（pair B）的鏡射暫存；
// 與 pair A 同步寫入相同值。原 binary 名稱：word_230B8D8 / word_230B8DA。
extern unsigned short           g_wLetterboxBW;
extern unsigned short           g_wLetterboxBH;

// mofclient.c：寬螢幕 letterbox 兩側的半透明遮罩 box。Map::CreateMap 與
// 繪製管線會操作這兩個 CControlAlphaBox 實體。
class CControlAlphaBox;
extern CControlAlphaBox         g_clAlphaClipBox_L;
extern CControlAlphaBox         g_clAlphaClipBox_R;

// mofclient.c 0x7C368C：SetViewChar 將目前 view char 指標也鏡射到此全域，
// 供 ObjectManager / 其他系統共用。原 binary 名稱：dword_7C368C。
extern ClientCharacter*         g_pLastSetViewChar;

// Network
extern CMoFNetwork              g_Network;

// Info
extern cltClassKindInfo         g_clClassKindInfo;
extern cltItemKindInfo          g_clItemKindInfo;
extern cltNPCInfo               g_clNPCInfo;
extern cltQuestKindInfo         g_clQuestKindInfo;

// System
extern CSpiritSystem            g_clSpiritSystem;
extern cltQuestSystem           g_clQuestSystem;
extern CMeritoriousSystem       g_clMeritoriousSystem;
extern cltLevelSystem           g_clLevelSystem;
extern cltClassSystem           g_clClassSystem;

// Logic
extern cltBaseInventory         g_clMyInventory;
extern cltTimerManager          g_clTimerManager;
extern cltFieldItemManager      g_clFieldItemMgr;
extern cltHelpMessage           g_clHelpMessage;
extern cltNPCManager            g_clNPCManager;
extern cltMoF_MineManager       g_clMineMgr;
extern cltMatchManager          g_clMatchManager;
extern cltPKManager             g_clPKManager;
extern cltPKFlagManager         g_clPKFlagManager;
extern CSpiritSpeechMgr         g_clSpiritSpeechMgr;

// 玩家操作引導系統（mofclient.c 0x21B8E1C：unk_21B8E1C）。
// ClientCharacterManager::Attack 與 CharKeyInputProcess 在玩家攻擊／擊殺時呼叫。
class cltHelpSystem;
extern cltHelpSystem            g_clHelpSystem;
extern CSpiritSpeechParser      g_clSpiritSpeechParser;
extern CSpiritGossipParser      g_clSpiritGossipParser;
extern clTransportKindInfo      g_clTransportKindInfo;
extern clClientTransportKindInfo g_clClientTransportKindInfo;

extern ClientCharacterManager   g_ClientCharMgr;
extern CAManager                g_CAManager;
class CCAClone;
extern CCAClone                 g_clCAClone;
extern cltConfiguration*        g_clConfig;

// mofclient.c 0x6E1620：鍵位設定管理器 (CShortKey)。由初始化流程 `operator new(0x1C0u)`
// 建立後，隨即呼叫 CShortKey::InitStaticDefaultKey / ReadKeySetting 填入預設鍵位。
class CShortKey;
extern CShortKey*               g_pShortKeyList;

// Object
extern CObjectManager           g_ObjectManager;
extern CObjectResourceManager   g_ObjectResourceManager;
extern DrawObject_All           g_DrawObject_ALL;
extern CEffectManager           g_EffectManager;
extern CEffectManager*          g_pEffectManager_Before_Chr;
extern CEffectManager*          g_pEffectManager_After_Chr;

// UI
extern CUITutorial*             g_pUITutorial;
extern CUIManager*              g_UIMgr;

// Pet / Monster / Money
extern cltPetKindInfo           g_clPetKindInfoBase;
extern cltClientPetKindInfo     g_clClientPetKindInfo;
extern cltPortalInfo            g_clPortalInfo;
extern cltClientPortalInfo      g_clClientPortalInfo;
extern cltPetSystem             g_clPetSystem;
extern cltMoneySystem           g_clMoneySystem;
extern cltMoFC_EffectKindInfo   g_clEffectKindInfo;
extern cltMyCharData            g_clMyCharData;
extern cltSystemMessage         g_clSysemMessage;

// MessageBox
extern CMessageBoxManager*      g_pMsgBoxMgr;

// Chatting
extern cltChattingMgr           g_clChattingMgr;

// mofclient.c 0x6E17B0 / g_Block：聊天輸入游標上方的選取方塊
// （輸入法組字時顯示）。原始碼使用 CControlAlphaBox 全域實體。
class CControlAlphaBox;
extern CControlAlphaBox         g_Block;
extern int                      dword_6E17B0; // 控制 DrawBlockBox 是否繪製

// mofclient.c: GDI font measuring handles used by cltChattingMgr::SendInputChat
// to compute the composition block position.  In this restored build we leave
// them as NULL placeholders — SelectObject with NULL is a no-op and
// MoFFont::GetTextLength does not actually require them.
extern HDC                      hdc;
extern HGDIOBJ                  h;

// mofclient.c 0x6C8CF8：主遊戲狀態（10 = InGame）。cltChattingMgr::Poll 只在
// 狀態為 10 時才處理輸入。
extern unsigned int             g_dwMainGameState;

// 社群/髒字過濾相關全域物件（mofclient.c 直接取位址）。
class DCTAbuseWordManager;
extern DCTAbuseWordManager      g_DCTAbuseWordManager;
class CInterfaceDataCommunity;
extern CInterfaceDataCommunity* g_pInterfaceDataCommunity;

// mofclient.c: the chat manager reads the party state from
// m_pclMyChatData + 6728 (the party system embedded inside cltMyCharData).
// Our cltMyCharData does not embed that subsystem, so we expose the same
// cltClientPartySystem via a global instance and redirect every embedded-
// offset access to it.
class cltClientPartySystem;
extern cltClientPartySystem     g_clPartySystem;

// CashShop
extern cltCashShopItem          g_clCashShopItem;

// BasicAppear
extern cltBasicAppearKindInfo   g_clBasicAppearKindInfo;

// 工具函式（對齊反編譯）
unsigned int ExGetTimeOutItemRemindTime(unsigned int a1, unsigned short a2);
unsigned int ExGetMyAccount();

//-----------------------------------------------------------------------------
// 工具函式（對齊反編譯 CircleLoopUpper / CircleLoopLower）
//-----------------------------------------------------------------------------
inline void CircleLoopUpper(int* val, int maxCount)
{
    if (maxCount) {
        if (*val >= maxCount - 1)
            *val = 0;
        else
            ++(*val);
    }
}

inline void CircleLoopLower(int* val, int maxCount)
{
    if (maxCount) {
        if (*val <= 0)
            *val = maxCount - 1;
        else
            --(*val);
    }
}

// Device
extern CDeviceManager&          g_clDeviceManager;

// Sound
extern GameSound                g_GameSoundManager;

// MiniGame
class cltMoF_BaseMiniGame;
class cltMoF_MiniGame_Mgr;
extern cltMoF_BaseMiniGame      g_clMiniGame;
extern cltMoF_MiniGame_Mgr      g_clMiniGameMgr;
extern CEffectManager           g_EffectManager_MiniGame;
extern cltLessonSystem          g_clLessonSystem;

// cltMini_Sword 所使用的全域狀態（對齊 mofclient.c）
extern unsigned char            g_cGameSwordState;  // 0..7 / 100
extern unsigned char            g_cGameSword_2State; // cltMini_Sword_2 專用狀態機
extern unsigned char            g_cGameExorcist_2State; // cltMini_Exorcist_2 專用狀態機
extern unsigned char            g_cGameExorcistState;   // cltMini_Exorcist 專用狀態機
extern unsigned char            g_cGameBowState;        // cltMini_Bow 專用狀態機
extern unsigned char            g_cGameBow_2State;      // cltMini_Bow_2 專用狀態機
extern unsigned char            g_cGameMagicState;      // cltMini_Magic 專用狀態機
extern unsigned char            g_cGameMagic_2State;    // cltMini_Magic_2 專用狀態機
extern unsigned char            g_cCardState[3];        // cltMini_Exorcist 三張卡片動畫狀態
extern int                      g_stGsGameFieldArg;     // mofclient.c: _UNKNOWN，用於 MiniGame 結束後場景切換
extern unsigned int             dwFrameCnt;
extern int                      MonPosY[10];        // 怪物頭頂抖動位移
extern int                      g_GAMESCORE;        // 通用小遊戲分數累計（cltMini_Exorcist 等使用）
extern float                    g_fBowSpearTable[34]; // mofclient.c: g_fPoint/dword_6CDB84 — 17 組 [x,y] 飛矛生成位移

// Info (additional)
extern cltSkillKindInfo         g_clSkillKindInfo;
extern cltLessonKindInfo        g_clLessonKindInfo;
extern cltEmblemKindInfo        g_clEmblemKindInfo;
extern cltSpecialtyKindInfo     g_clSpecialtyKindInfo;
extern cltMakingItemKindInfo    g_clMakingItemKindInfo;
extern cltPetSkillKindInfo      g_clPetSkillKindInfo;
extern cltCoupleRingKindInfo    g_clCoupleRingKindInfo;
extern cltTransformKindInfo     g_clTransformKindInfo;
extern cltEmoticonKindInfo      g_clEmoticonKindInfo;
extern cltPKRankKindInfo        g_clPKRankKindInfo;
extern cltCharKindInfo*         g_pcltCharKindInfo;
extern cltRegenMonsterKindInfo  g_cltRegenMonsterKindInfo;
extern cltMapUseItemInfoKindInfo g_clMapUseItemInfoKindInfo;
extern cltMapInfo*              g_pcltMapInfo;

// System (additional)
extern cltEquipmentSystem*      dword_21BA32C;
extern cltSkillSystem           g_clSkillSystem;
extern cltSpecialtySystem       g_clSpecialtySystem;       // unk_21BCF88
extern cltUsingItemSystem       g_clUsingItemSystem;       // unk_21BA4B0
extern cltSexSystem             g_clSexSystem;             // unk_21BA7BC
extern cltPlayerAbility         g_clPlayerAbility;         // unk_21B9F30
extern cltUsingSkillSystem      g_clUsingSkillSystem;      // unk_21BA578
extern cltMarriageSystem*       g_clMarriageSystem;        // dword_21C9C04

// -----------------------------------------------------------------------------
// cltMyCharData 內嵌子系統 (mofclient.c 把它們塞在 cltMyCharData byte offset
// 4512..69260 之間)。本還原採用「外掛全域」設計，這些全域既支援
// cltMyCharData::Initialize 的扇形 wire-up，也讓其他模組能直接取用同一份
// 玩家狀態，無需穿透 cltMyCharData。
// -----------------------------------------------------------------------------
class cltQuickSlotSystem;       extern cltQuickSlotSystem        g_clQuickSlotSystem;       // GT cltMyCharData +5252
class cltEquipmentSystem;       extern cltEquipmentSystem        g_clEquipmentSystem;       // GT +5556 (back of dword_21BA32C)
class cltGradeSystem;           extern cltGradeSystem            g_clGradeSystem;           // GT +16900
class cltTestingSystem;         extern cltTestingSystem          g_clTestingSystem;         // GT +16996
class cltMakingItemSystem;      extern cltMakingItemSystem       g_clMakingItemSystem;      // GT +17032
class cltTransformSystem;       extern cltTransformSystem        g_clTransformSystem;       // GT +19044
class cltEnchantSystem;         extern cltEnchantSystem          g_clEnchantSystem;         // GT +19056
class cltCashshopSystem;        extern cltCashshopSystem         g_clCashshopSystem;        // GT +19072
class cltOverMindSystem;        extern cltOverMindSystem         g_clOverMindSystem;        // GT +39240
class cltEmblemSystem;          extern cltEmblemSystem           g_clEmblemSystem;          // GT +39308
class CQuizEventSystem;         extern CQuizEventSystem          g_clQuizEventSystem;       // GT +39544
class cltTASSystem;             extern cltTASSystem              g_clTASSystem;             // GT +39616
class cltWorkingPassiveSkillSystem;
                                extern cltWorkingPassiveSkillSystem g_clWorkingPassiveSkillSystem;  // GT +40384
class cltMonsterToleranceSystem;extern cltMonsterToleranceSystem g_clMonsterToleranceSystem;// GT +40568
class cltMoFC_EquipmentInfo;    extern cltMoFC_EquipmentInfo     g_clMoFC_EquipmentInfo;    // GT +40576
class cltNPCRecallSystem;       extern cltNPCRecallSystem        g_clNPCRecallSystem;       // GT +40712
class cltTitleSystem;           extern cltTitleSystem            g_clTitleSystem;           // GT +40720
class cltPetKeepingSystem;      extern cltPetKeepingSystem       g_clPetKeepingSystem;      // GT +42048
class cltPetMarketMySalePetSystem;
                                extern cltPetMarketMySalePetSystem g_clPetMarketMySalePetSystem;  // GT +67752
class cltPKRankSystem;          extern cltPKRankSystem           g_clPKRankSystem;          // GT +69044
class cltMyItemSystem;          extern cltMyItemSystem           g_clMyItemSystem;          // GT +69084
class cltStorageSystem;         extern cltStorageSystem          g_clStorageSystem;         // GT +9524 (back of dword_21BB2AC)
class cltExStorageSystem;       /* g_clExStorageSystem 在 inc/System/cltExStorageSystem.h 已宣告 */
class cltEmoticonSystem;        extern cltEmoticonSystem         g_clEmoticonSystem;        // GT unk_82723C
class CPlayerSpirit;            extern CPlayerSpirit             g_clPlayerSpirit;          // GT cltMyCharData +14660

// 與 dword_21BA32C / dword_21BB2AC 配對：兩個 pointer 全域於啟動後指向上方
// g_clEquipmentSystem / g_clStorageSystem 實體（global.cpp 進行 wiring）。
extern cltStorageSystem*        dword_21BB2AC;

// Free helper functions (ground truth: mofclient.c)
unsigned int ExGetTextCodeHuntItemType(unsigned int a1);
unsigned int ExGetTextCodeFashionItemType(unsigned int a1);
unsigned int ExGetTextCodeInstantItemType(unsigned int a1);
int ExGetTextCodeHuntItemAttribute(struct stItemKindInfo* a1);
float SetRatePerThousand(int a1);
float SetRatePerTenThousand(int a1);
uint8_t ExGetMyLevel();

// UI 輸入消耗旗標（對齊反編譯 dword_AFD34C）
extern int                      dword_AFD34C;

// 全域字型高度（對齊反編譯 cHeight）
extern int                      cHeight;

// cltHelpMessage 使用的全域旗標
extern int                      dword_AFD344;

// DrawObject_All / rendering globals restored from mofclient.c
extern int                      dword_73D154;
extern int                      dword_B4BAB4;
extern char                     byte_21CB35D;

// CAManager packed-vs-direct timeline load flag (reverse of packing origin).
// Mirrors mofclient.c's `dword_829254`: when non-zero, timelines are fetched
// via CMofPacking::FileReadBackGroundLoading; otherwise they come from direct
// fopen/fread.  The original initializes this elsewhere; we default to 1
// (packed) because the shipped .ca files live inside the MOF pack.
extern int                      dword_829254;

// -----------------------------------------------------------------------------
// CCA / CCAClone per-slot fallback case maps (mofclient.c 0x525EB4 / 0x5280FC).
//
// Purpose
// -------
// CCA::Process(GameImage*) and CCAClone::Process() iterate every visible
// layer slot (0..22) and resolve a GameImage for each CA_DRAWENTRY in the
// current frame.  When cltImageManager::GetGameImage returns null / an
// un-textured node (e.g. a missing item-kind asset), Process() falls back
// to one of these two LUTs — indexed by `slot - 1` for valid slots 1..19
// — to pick an alternate (kind, idx) layer to re-query with the SAME
// frame + entry index.  This lets a character keep rendering sensible
// geometry when an equipped item is missing its CA data.
//
// Case legend (both tables share the same encoding)
// -------------------------------------------------
//   0 → re-resolve via the current hair layer (kind 0, GetHairLayerIndexDot)
//   1 → hand layer lo (kind 5, 2*sex)
//   2 → re-resolve via the current face layer (kind 1, GetFaceLayerIndexDot)
//   3 → shoes layer (kind 4, sex)
//   4 → triusers layer (kind 3, sex)
//   5 → coat layer (kind 2, sex)
//   6 → hair layer +1 (kind 0, GetHairLayerIndexDot + 1)  // back-hair
//   7 → hand layer hi (kind 5, 2*sex + 1)
//   8 → skip this entry entirely (no fallback — leave the slot empty)
//
// Any other value is treated as 8 (skip).
//
// Usage
// -----
// Both arrays are zero-initialized at startup (matching the shipped weak
// symbols), so EVERY slot defaults to case 0 — i.e. missing items fall
// back to the character's current hair silhouette.  To change the fallback
// strategy for a particular slot from tooling / init code, just write the
// desired case number into the table before the next Process() tick:
//
//     // Skip fallback entirely for slot 7 (ACC1), so a missing ACC1 item
//     // simply renders nothing instead of bleeding hair geometry:
//     byte_525EB4[7 - 1] = 8;
//
//     // Make missing shoes fall back to the triusers layer:
//     byte_525EB4[8 - 1] = 4;
//
// Both CCA and CCAClone consult their table on every Process() call, so
// changes take effect on the very next frame — no re-link, no relayout.
// Valid slot range is 1..19 (slot 0 and 20..22 never hit the fallback).
//
// Full case dispatch lives in src/Character/CCA.cpp::CCA::Process(GameImage*)
// and src/Character/CCAClone.cpp::CCAClone::Process(); both share the same
// CCA_TryFallbackResolve / CCAClone_TryFallbackResolve helper that mirrors
// mofclient.c 241143-241408 / 243031-243223.
// -----------------------------------------------------------------------------
// Indexed as `byte_525EB4[slot - 1]`; slot ∈ [1, 19].  Set to 0..8 to pick a
// fallback strategy (see legend above), or 8 to disable fallback for a slot.
extern unsigned char            byte_525EB4[19];  // CCA fallback case map
// Indexed as `byte_5280FC[slot - 1]`; slot ∈ [1, 19].  Same encoding as
// byte_525EB4 but consulted by CCAClone::Process() instead of CCA::Process().
extern unsigned char            byte_5280FC[19];  // CCAClone fallback case map
extern cltFieldItem*            unk_73D15C[1024];
extern void*                    unk_813AA8[1024];
extern void*                    unk_B4B924[1024];
extern ClientCharacter          unk_1409D80[1000];
extern ClientCharacter*         dword_1843F78[1000];
extern size_t                   NumOfElements;

// CControlChatBallon 九宮格樣式表（每種樣式 11 個 uint16 block ID）
// 對齊反編譯位址 0x6C6AD0
extern const uint16_t           g_ChatBallonStyleTable[];

// -----------------------------------------------------------------------------
// 由 mofclient.c 還原之新增全域 (CLog/CLogWriter/cltRegistry/CDeleteCash...)。
// 把宣告集中在這裡以遵守「全域變數必須宣告在 global.h/.cpp」的專案規定。
// -----------------------------------------------------------------------------
class CDeleteCashItemManager;
extern CDeleteCashItemManager   g_DeleteCashItemManager; // mofclient.c 0x...

class cltRegistry;
extern cltRegistry              g_clRegistry;             // mofclient.c 0x... 約 264 bytes

class CCmdLine;
extern CCmdLine                 g_CmdLine;                // mofclient.c 0x...

class MainTitleBG;
extern MainTitleBG*             g_pLobbyBackground;       // mofclient.c 0x...

extern float                    flt_21CB358;              // mofclient.c 全域 deltaTime
