#include "MiniGame/Ani_Board.h"

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Effect/CEffect_MiniGame_Fighter_Break.h"
#include "Effect/CEffectManager.h"
#include "Sound/GameSound.h"

extern CEffectManager g_EffectManager_MiniGame;

Ani_Board::Ani_Board()
    : m_pImage(nullptr)
    , m_curFrame(10)
    , m_targetFrame(0)
    , m_bPlaying(false)
    , m_x(0.0f)
    , m_y(0.0f)
    , m_frameAccum(0.0f)
    , m_frameThreshold(0.05f)
{
}

Ani_Board::~Ani_Board() = default;

void Ani_Board::InitAni_Board(float x, float y)
{
    m_x = x;
    m_y = y;
    m_curFrame = 10;
    m_bPlaying = false;
}

void Ani_Board::Play(int targetFrame)
{
    // mofclient.c：Play 會同時噴發 CEffect_MiniGame_Fighter_Break 並播放碎裂音效。
    m_targetFrame = targetFrame;
    m_bPlaying = true;

    CEffect_MiniGame_Fighter_Break* pEffect = new CEffect_MiniGame_Fighter_Break();
    const float fx = m_x + 17.0f;
    const float fy = m_y + 10.0f;
    pEffect->SetEffect(fx, fy);
    g_EffectManager_MiniGame.BulletAdd(pEffect);

    g_GameSoundManager.PlaySoundA((char*)"M0016", 0, 0);
}

void Ani_Board::SetPosition(float x, float y)
{
    m_x = x;
    m_y = y;
}

char Ani_Board::Process(float dt)
{
    char finished = 0;
    GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pImage = pImage;
    if (pImage)
    {
        pImage->m_bFlag_446 = true;
        pImage->SetBlockID(static_cast<unsigned short>(m_curFrame));

        if (m_bPlaying)
        {
            // mofclient.c 的 FrameSkip：累計時間並轉換為整數跳幀。
            m_frameAccum += dt;
            int step = 0;
            if (m_frameAccum >= m_frameThreshold)
            {
                step = static_cast<int>(m_frameAccum / m_frameThreshold);
                if (step != 0)
                    m_frameAccum -= static_cast<float>(step) * m_frameThreshold;
            }
            m_curFrame += step;
            if (m_curFrame >= m_targetFrame)
            {
                m_bPlaying = false;
                m_curFrame = m_targetFrame;
                finished = 1;
            }
        }

        pImage->m_fPosX = m_x;
        pImage->m_bFlag_447 = true;
        pImage->m_fPosY = m_y;
        pImage->m_bFlag_447 = true;
        pImage->Process();
    }
    return finished;
}

void Ani_Board::Render()
{
    GameImage* pImage = m_pImage;
    if (pImage && pImage->m_pGIData)
        pImage->Draw();
}
