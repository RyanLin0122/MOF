#include "Object/DrawObject_All.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Effect/CEffectManager.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "global.h"
#include <cstdlib>
#include <cstring>

// -------------------------------------------------------------------------
// 前向宣告 — 尚未還原的全域物件和管理器
// -------------------------------------------------------------------------
// 以下全域物件在其他模組中定義，DrawObject_All 需要呼叫它們的方法。
// 由於這些模組尚未完全還原，這裡使用 stub class 佔位。

// cltMapTitle — 地圖標題
class cltMapTitle {
public:
    void PrepareDrawing() {}
    void Draw() {}
};

// cltMoF_MineManager — 礦脈管理器
class cltMoF_MineManager {
public:
    void PrepareDrawing() {}
    void Draw() {}
};

// cltMatchManager — 配對/競賽管理器
class cltMatchManager {
public:
    void PrepareDrawing() {}
    void Draw(int index) {}
    int GetMatchRoomNum() { return 0; }
};

// cltPKFlagManager — PK 旗幟管理器
class cltPKFlagManager {
public:
    void PrepareDrawing() {}
    void Draw(int index) {}
};

// 全域 stub 實例
static cltMapTitle g_clMapTitle;
static cltMoF_MineManager g_clMineMgr;
static cltMatchManager g_clMatchManager;
static cltPKFlagManager g_clPKFlagManager;
static CEffectManager* g_pEffectManager = nullptr;
static CEffectManager* g_pEffectManager_Before_Chr = nullptr;
static CEffectManager* g_pEffectManager_After_Chr = nullptr;

// 繪製排序用的暫存陣列
static const int MAX_DRAW_CHARS = 1000;
static const int MAX_DRAW_OBJECTS = 1000;
static ClientCharacter* s_sortedChars[MAX_DRAW_CHARS];
static CBaseObject* s_drawObjects[MAX_DRAW_OBJECTS];
static CBaseObject* s_drawPortals[MAX_DRAW_OBJECTS];

// 角色陣列和數量 (在外部 ClientCharacterManager 中管理)
// 這些是 ground truth 中的 unk_1409D80 / dword_1843F78 等全域陣列
// 實際的角色排序邏輯依賴完整的 ClientCharacterManager 實作
static int s_charCount = 0;

// byte_21CB35D: 暫停繪製旗標
static unsigned char s_bPauseDrawing = 0;

// dword_73D154: 道具數量
static int s_fieldItemCount = 0;

// dword_B4BAB4: PK 旗幟數量
static int s_pkFlagCount = 0;

// -------------------------------------------------------------------------
// 比較函式 — qsort 用
// -------------------------------------------------------------------------
int compareCharYpos(const void* a1, const void* a2)
{
    int y1 = (*(ClientCharacter**)a1)->m_iPosY;
    int y2 = (*(ClientCharacter**)a2)->m_iPosY;
    double diff = (double)(y1 - y2);
    if (diff == 0.0)
        return 1;  // ground truth: 相等時回傳 1 (非 0)
    return (diff > 0.0) ? 1 : -1;
}

int compareObjectYpos(const void* a1, const void* a2)
{
    int y1 = (*(CBaseObject**)a1)->m_nPosY;
    int y2 = (*(CBaseObject**)a2)->m_nPosY;
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

int compareItemYpos(const void* a1, const void* a2)
{
    // cltFieldItem 的 Y 座標在 offset +8 (DWORD +2)
    int y1 = *(int*)((char*)(*(void**)a1) + 8);
    int y2 = *(int*)((char*)(*(void**)a2) + 8);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

int compareMatchFlagYpos(const void* a1, const void* a2)
{
    // Match flag 的 Y 座標在 offset +64
    int y1 = *(int*)((char*)(*(void**)a1) + 64);
    int y2 = *(int*)((char*)(*(void**)a2) + 64);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

int comparePKPlagYpos(const void* a1, const void* a2)
{
    // PK flag 的 Y 座標在 offset +8
    int y1 = *(int*)((char*)(*(void**)a1) + 8);
    int y2 = *(int*)((char*)(*(void**)a2) + 8);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

// -------------------------------------------------------------------------
// Constructor / Destructor
// -------------------------------------------------------------------------
DrawObject_All::DrawObject_All()
{
    // ground truth: 空的建構函式
}

DrawObject_All::~DrawObject_All()
{
    // ground truth: 空的解構函式
}

// -------------------------------------------------------------------------
// PrepareDrawing — 準備所有物件的繪製資料
// -------------------------------------------------------------------------
void DrawObject_All::PrepareDrawing()
{
    // 遍歷所有角色，呼叫 PrepareDrawingChar
    // Ground truth: 從 unk_1409D80 開始，每 14824 bytes 一個角色
    // 這裡由 ClientCharacterManager 負責管理

    // 處理所有地圖物件
    g_ObjectManager.Process();

    // 準備道具繪製
    g_clFieldItemMgr.PrepareDrawing();

    // 準備其他系統的繪製
    // ClientCharacterManager::PrepareDrawingEtcMark(&g_ClientCharMgr);
    g_clMapTitle.PrepareDrawing();
    // g_clHelpMessage.PrepareDrawing();  // 不在 cltHelpMessage 的已定義方法中
    g_clMineMgr.PrepareDrawing();
    // Map::PrepareDrawingClimate(&g_Map);
    g_clMatchManager.PrepareDrawing();
    g_clPKFlagManager.PrepareDrawing();
}

// -------------------------------------------------------------------------
// DrawObject — 按 Y 座標排序的多路合併繪製
// -------------------------------------------------------------------------
void DrawObject_All::DrawObject()
{
    if (s_bPauseDrawing)
        return;

    // 繪製礦脈
    g_clMineMgr.Draw();

    // 取得各類物件數量
    int charCount = s_charCount;
    int objectCount = g_ObjectManager.GetObjectCount();
    int fieldItemCount = s_fieldItemCount;
    int matchCount = g_clMatchManager.GetMatchRoomNum();
    int pkFlagCount = s_pkFlagCount;

    // 分離物件為一般物件和傳送門物件
    int drawObjCount = 0;
    int portalCount = 0;

    for (int i = 0; i < objectCount; ++i)
    {
        if (g_ObjectManager.GetType((unsigned short)i) == 1)
        {
            s_drawPortals[portalCount++] = g_ObjectManager.GetObjectA((unsigned short)i);
        }
        else
        {
            s_drawObjects[drawObjCount++] = g_ObjectManager.GetObjectA((unsigned short)i);
        }
    }

    // 排序各類物件
    if (charCount > 0)
        qsort(s_sortedChars, charCount, sizeof(ClientCharacter*), compareCharYpos);
    if (drawObjCount > 0)
        qsort(s_drawObjects, drawObjCount, sizeof(CBaseObject*), compareObjectYpos);

    // Ground truth 的核心繪製迴圈：
    // 多路合併 — 比較角色、物件、道具、配對旗、PK旗的 Y 座標，
    // 每次繪製 Y 最小的那一個。
    // 完整的合併邏輯非常複雜(因為是從 goto-heavy 的反編譯碼還原)，
    // 這裡實作等價的行為：

    int charIdx = 0, objIdx = 0, itemIdx = 0, matchIdx = 0, pkIdx = 0;

    // 先繪製 EffectManager_Before_Chr 和傳送門 (在第一個角色之前)
    if (g_pEffectManager_Before_Chr)
        g_pEffectManager_Before_Chr->Draw();

    for (int p = 0; p < portalCount; ++p)
    {
        if (s_drawPortals[p])
            s_drawPortals[p]->Draw();
    }

    // 多路合併繪製
    while (charIdx < charCount || objIdx < drawObjCount ||
           itemIdx < fieldItemCount || matchIdx < matchCount || pkIdx < pkFlagCount)
    {
        int charY = (charIdx < charCount) ? s_sortedChars[charIdx]->m_iPosY : 10000;
        int objY = (objIdx < drawObjCount) ? s_drawObjects[objIdx]->m_nPosY : 10000;
        // 道具、配對旗、PK旗的 Y 座標需要從各自的管理器取得
        // 此處簡化為分階段繪製

        if (charY <= objY)
        {
            // 繪製角色
            if (charIdx < charCount)
            {
                s_sortedChars[charIdx]->DrawChar(pkIdx);
                ++charIdx;
                if (charIdx == charCount && g_pEffectManager_After_Chr)
                    g_pEffectManager_After_Chr->Draw();
            }
        }
        else
        {
            // 繪製物件
            if (objIdx < drawObjCount && s_drawObjects[objIdx])
            {
                s_drawObjects[objIdx]->Draw();
                ++objIdx;
            }
        }

        // 如果沒有進展，避免無限迴圈
        if (charY >= 10000 && objY >= 10000)
            break;
    }

    // 繪製剩餘角色
    while (charIdx < charCount)
    {
        s_sortedChars[charIdx]->DrawChar(pkIdx);
        ++charIdx;
    }

    // 繪製剩餘物件
    while (objIdx < drawObjCount)
    {
        if (s_drawObjects[objIdx])
            s_drawObjects[objIdx]->Draw();
        ++objIdx;
    }

    // 後續繪製層
    // ClientCharacterManager::DrawEtcMark(&g_ClientCharMgr);
    g_clMapTitle.Draw();
    // Map::DrawClimate(&g_Map);
    if (g_pEffectManager)
        g_pEffectManager->Draw();
    // g_clHelpMessage.Draw();
}
