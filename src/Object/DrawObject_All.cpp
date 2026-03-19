#include "Object/DrawObject_All.h"
#include "Object/CObjectManager.h"
#include "Object/CBaseObject.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Effect/CEffectManager.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltHelpMessage.h"
#include "Logic/Map.h"
#include "global.h"
#include <cstdlib>
#include <cstring>

// -------------------------------------------------------------------------
// 前向宣告 — 尚未還原的全域物件和管理器
// -------------------------------------------------------------------------

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

// Ground truth 的三個 EffectManager 實例
// g_EffectManager — 主要特效管理器 (後期繪製) — 使用 CEffectManager::GetInstance()
// g_EffectManager_Befor_Chr — 角色前特效 (在第一個角色之前繪製)
// g_EffectManager_After_Chr — 角色後特效 (在最後一個角色之後繪製)
// TODO: 當這些全域實例被還原後，改為 extern 宣告
static CEffectManager* g_pEffectManager_Before_Chr = nullptr;
static CEffectManager* g_pEffectManager_After_Chr = nullptr;

// Ground truth 中的全域角色陣列和排序指標
// unk_1409D80: 角色陣列起始，每個角色 14824 bytes
// dword_1843F78: 排序指標陣列 (ClientCharacter* 陣列)
// NumOfElements: 排序指標數量
// 目前這些依賴完整的 ClientCharacterManager 記憶體佈局，使用 stub
static const int MAX_DRAW_CHARS = 1000;
static const int MAX_DRAW_OBJECTS = 1000;
static ClientCharacter** s_sortedCharPtrs = nullptr;
static int s_charCount = 0;
static CBaseObject* s_drawObjects[MAX_DRAW_OBJECTS];
static CBaseObject* s_drawPortals[MAX_DRAW_OBJECTS];

// byte_21CB35D: 暫停繪製旗標
static unsigned char s_bPauseDrawing = 0;

// dword_73D154: field item 數量
extern int dword_73D154;
// dword_B4BAB4: PK 旗幟數量
extern int dword_B4BAB4;

// field item 指標陣列
extern void* unk_73D15C;
// match flag 指標陣列
extern void* unk_813AA8;
// PK flag 指標陣列
extern void* unk_B4B924;

// -------------------------------------------------------------------------
// 比較函式 — qsort 用
// -------------------------------------------------------------------------
int compareCharYpos(const void* a1, const void* a2)
{
    int y1 = (*(ClientCharacter**)a1)->m_iPosY;
    int y2 = (*(ClientCharacter**)a2)->m_iPosY;
    double diff = (double)(y1 - y2);
    if (diff == 0.0)
        return 1;  // ground truth: 相等時回傳 1
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
    int y1 = *(int*)((char*)(*(void**)a1) + 8);
    int y2 = *(int*)((char*)(*(void**)a2) + 8);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

int compareMatchFlagYpos(const void* a1, const void* a2)
{
    int y1 = *(int*)((char*)(*(void**)a1) + 64);
    int y2 = *(int*)((char*)(*(void**)a2) + 64);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

int comparePKPlagYpos(const void* a1, const void* a2)
{
    int y1 = *(int*)((char*)(*(void**)a1) + 8);
    int y2 = *(int*)((char*)(*(void**)a2) + 8);
    double diff = (double)(y1 - y2);
    if (diff == 0.0) return 0;
    return (diff > 0.0) ? 1 : -1;
}

// -------------------------------------------------------------------------
// Constructor / Destructor — ground truth 004FA310 / empty
// -------------------------------------------------------------------------
DrawObject_All::DrawObject_All()
{
}

DrawObject_All::~DrawObject_All()
{
}

// -------------------------------------------------------------------------
// PrepareDrawing — ground truth 004FA330
// 遍歷所有角色呼叫 PrepareDrawingChar，然後準備各系統
// -------------------------------------------------------------------------
void DrawObject_All::PrepareDrawing()
{
    // Ground truth: 遍歷所有 active 角色，呼叫 PrepareDrawingChar
    // 從 unk_1409D80 開始，每 14824 bytes 一個角色，直到 dword_1843F78
    // TODO: 當 ClientCharacterManager 完整還原角色陣列佈局後，啟用此迴圈
    // ClientCharacter* pChar = (ClientCharacter*)&unk_1409D80;
    // while ((int)pChar < (int)&dword_1843F78) {
    //     if (pChar->m_dwAccountID)  // DWORD offset 1109 (active check)
    //         pChar->PrepareDrawingChar();
    //     pChar = (ClientCharacter*)((char*)pChar + 14824);
    // }

    // 處理所有地圖物件
    g_ObjectManager.Process();

    // 準備道具繪製
    g_clFieldItemMgr.PrepareDrawing();

    // Ground truth: 準備其他系統的繪製
    g_ClientCharMgr.PrepareDrawingEtcMark();
    g_clMapTitle.PrepareDrawing();
    g_clHelpMessage.PrepareDrawing();
    g_clMineMgr.PrepareDrawing();
    g_Map.PrepareDrawingClimate();
    g_clMatchManager.PrepareDrawing();
    g_clPKFlagManager.PrepareDrawing();
}

// -------------------------------------------------------------------------
// DrawObject — ground truth 004FA3C0
// 按 Y 座標排序的五路合併繪製
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

    // Ground truth: 這些數量來自真實全域變數
    // int fieldItemCount = (unsigned short)dword_73D154;
    // int matchCount = g_clMatchManager.GetMatchRoomNum();
    // int pkFlagCount = dword_B4BAB4;
    int fieldItemCount = 0;
    int matchCount = g_clMatchManager.GetMatchRoomNum();
    int pkFlagCount = 0;

    // Ground truth: 排序角色指標陣列
    // _qsort(&dword_1843F78, charCount, 4, compareCharYpos);

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
    if (drawObjCount > 0)
        qsort(s_drawObjects, drawObjCount, sizeof(CBaseObject*), compareObjectYpos);
    // Ground truth: 也排序 item, match flag, PK flag
    // qsort(&unk_73D15C, fieldItemCount, 4, compareItemYpos);
    // qsort(&unk_813AA8, matchCount, 4, compareMatchFlagYpos);
    // qsort(&unk_B4B924, pkFlagCount, 4, comparePKPlagYpos);

    // 五路合併繪製 — 角色、物件、道具、配對旗、PK旗
    int charIdx = 0, objIdx = 0, itemIdx = 0, matchIdx = 0, pkIdx = 0;
    bool drawnBeforeChr = false;

    while (true)
    {
        // 檢查是否所有都已繪完
        if (charIdx >= charCount && objIdx >= drawObjCount &&
            itemIdx >= fieldItemCount && matchIdx >= matchCount && pkIdx >= pkFlagCount)
        {
            // 結尾繪製層
            g_ClientCharMgr.DrawEtcMark();
            g_clMapTitle.Draw();
            g_Map.DrawClimate();
            CEffectManager::GetInstance()->Draw();
            g_clHelpMessage.Draw();
            return;
        }

        // 取得各類 Y 座標 (超出範圍設為 10000)
        int charY = 10000, objY = 10000;
        // int itemY = 10000, matchY = 10000, pkY = 10000;

        if (charIdx < charCount && s_sortedCharPtrs)
            charY = s_sortedCharPtrs[charIdx]->m_iPosY;
        if (objIdx < drawObjCount)
            objY = s_drawObjects[objIdx]->m_nPosY;

        // Ground truth 完整五路比較邏輯:
        // 選擇 Y 最小的繪製，如果角色 Y 最小，在第一個角色前繪製 EffectManager_Before_Chr 和 portals

        if (charY < objY)
        {
            // Ground truth: 在第一個角色之前繪製 before-chr 特效和 portals
            if (!drawnBeforeChr)
            {
                drawnBeforeChr = true;
                if (g_pEffectManager_Before_Chr)
                    g_pEffectManager_Before_Chr->Draw();
                for (int p = 0; p < portalCount; ++p)
                {
                    if (s_drawPortals[p])
                        s_drawPortals[p]->Draw();
                }
            }

            // 繪製角色
            if (s_sortedCharPtrs)
                s_sortedCharPtrs[charIdx]->DrawChar(pkIdx);
            int prevCharIdx = charIdx;
            ++charIdx;
            // Ground truth: 最後一個角色之後繪製 after-chr 特效
            if (prevCharIdx + 1 == charCount && g_pEffectManager_After_Chr)
                g_pEffectManager_After_Chr->Draw();
        }
        else
        {
            // 繪製物件
            if (objIdx < drawObjCount && s_drawObjects[objIdx])
            {
                s_drawObjects[objIdx]->Draw();
                ++objIdx;
                // Ground truth: 每畫完一般 object 後呼叫 Map::DrawClip
                g_Map.DrawClip();
            }
            else
            {
                // 避免無限迴圈
                break;
            }
        }
    }

    // 結尾繪製層
    g_ClientCharMgr.DrawEtcMark();
    g_clMapTitle.Draw();
    g_Map.DrawClimate();
    CEffectManager::GetInstance()->Draw();
    g_clHelpMessage.Draw();
}
