#include "Logic/cltMoF_MineManager.h"
#include "Effect/CEffect_Skill_Trap_Explosion.h"
#include "Effect/CEffectManager.h"
#include "global.h"
#include <cstring>

// ---------------------------------------------------------------------------
// cltMoF_MineManager
// ---------------------------------------------------------------------------

cltMoF_MineManager::cltMoF_MineManager()
    : m_mineKindInfo()
{
    // m_mines[50] 各自由 cltMoF_ClientMine 建構函式初始化
}

cltMoF_MineManager::~cltMoF_MineManager()
{
}

// ---------------------------------------------------------------------------
// Initialize — 讀取 Mine.txt
// ---------------------------------------------------------------------------
int cltMoF_MineManager::Initialize(const char* filename)
{
    int result = m_mineKindInfo.Initialize(filename);
    if (result)
    {
        // 清空所有地雷 slot
        for (int i = 0; i < MAX_MINES; ++i)
            m_mines[i].DeleteMine();
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// GetMineByHandel — 依 handle 找到地雷
// ---------------------------------------------------------------------------
cltMoF_ClientMine* cltMoF_MineManager::GetMineByHandel(unsigned int handle)
{
    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetHandle() == handle)
            return &m_mines[i];
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// GetMineKindInfoByKind
// ---------------------------------------------------------------------------
strMineKindInfo* cltMoF_MineManager::GetMineKindInfoByKind(uint16_t kind)
{
    return m_mineKindInfo.GetMineKindInfo(kind);
}

// ---------------------------------------------------------------------------
// ResetSearched
// ---------------------------------------------------------------------------
void cltMoF_MineManager::ResetSearched(unsigned int handle)
{
    cltMoF_ClientMine* mine = GetMineByHandel(handle);
    if (mine)
        mine->ResetSearched();
}

// ---------------------------------------------------------------------------
// AddMine — 從 strMineKindInfo 建立地雷
// ---------------------------------------------------------------------------
void cltMoF_MineManager::AddMine(unsigned int handle, uint16_t mineKind)
{
    strMineKindInfo* info = m_mineKindInfo.GetMineKindInfo(mineKind);
    if (!info)
        return;

    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetActive() != 1)
        {
            m_mines[i].CreateMine(
                handle,
                (int)info->attack,          // hp
                (int)info->attack,          // maxHp
                (int)info->attackRange,     // range
                (int)info->attackRange,     // maxRange
                info->disappearMs,
                info->resourceIdHex,
                info->totalFlame,
                (char)info->airGround,
                (char)info->attackCountType,
                (char)info->moveType);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// DeleteAllMine
// ---------------------------------------------------------------------------
void cltMoF_MineManager::DeleteAllMine()
{
    for (int i = 0; i < MAX_MINES; ++i)
        m_mines[i].DeleteMine();
}

// ---------------------------------------------------------------------------
// DeleteMineByHandle — 刪除地雷並播放爆炸特效
// ---------------------------------------------------------------------------
void cltMoF_MineManager::DeleteMineByHandle(unsigned int handle, uint16_t effectKind)
{
    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetHandle() == handle)
        {
            float sx = m_mines[i].GetScreenX();
            float sy = m_mines[i].GetScreenY();
            m_mines[i].DeleteMine();

            // 建立爆炸特效
            CEffect_Skill_Trap_Explosion* fx = new CEffect_Skill_Trap_Explosion();
            fx->SetEffect(sx, sy, (unsigned char)effectKind);
            if (g_pEffectManager_After_Chr)
                g_pEffectManager_After_Chr->BulletAdd(fx);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Poll
// ---------------------------------------------------------------------------
void cltMoF_MineManager::Poll()
{
    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetActive())
            m_mines[i].Poll();
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing
// ---------------------------------------------------------------------------
void cltMoF_MineManager::PrepareDrawing()
{
    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetActive() == 1)
            m_mines[i].PrepareDrawing();
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void cltMoF_MineManager::Draw()
{
    for (int i = 0; i < MAX_MINES; ++i)
    {
        if (m_mines[i].GetActive() == 1)
            m_mines[i].Draw();
    }
}
