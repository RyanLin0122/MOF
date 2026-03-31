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
class cltPKFlagManager;
class CSpiritSpeechMgr;
class clClientTransportKindInfo;
class clTransportKindInfo;
class cltPetKindInfo;
class cltClientPetKindInfo;
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
extern cltPKFlagManager         g_clPKFlagManager;
extern CSpiritSpeechMgr         g_clSpiritSpeechMgr;
extern CSpiritSpeechParser      g_clSpiritSpeechParser;
extern CSpiritGossipParser      g_clSpiritGossipParser;
extern clTransportKindInfo      g_clTransportKindInfo;
extern clClientTransportKindInfo g_clClientTransportKindInfo;

extern ClientCharacterManager   g_ClientCharMgr;
extern cltConfiguration*        g_clConfig;

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
extern cltPetSystem             g_clPetSystem;
extern cltMoneySystem           g_clMoneySystem;
extern cltMoFC_EffectKindInfo   g_clEffectKindInfo;
extern cltMyCharData            g_clMyCharData;
extern cltSystemMessage         g_clSysemMessage;

// MessageBox
extern CMessageBoxManager*      g_pMsgBoxMgr;

// Chatting
extern cltChattingMgr           g_clChattingMgr;

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
extern cltFieldItem*            unk_73D15C[1024];
extern void*                    unk_813AA8[1024];
extern void*                    unk_B4B924[1024];
extern ClientCharacter          unk_1409D80[1000];
extern ClientCharacter*         dword_1843F78[1000];
extern size_t                   NumOfElements;

// CControlChatBallon 九宮格樣式表（每種樣式 11 個 uint16 block ID）
// 對齊反編譯位址 0x6C6AD0
extern const uint16_t           g_ChatBallonStyleTable[];
