#include "Object/cltQuestMark.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "global.h"

extern int dword_A73088;
extern int dword_A7308C;
extern int SETTING_FRAME;

cltQuestMark::cltQuestMark()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_wCurrentFrame(0)
    , _pad(0)
    , m_fAniFrame(0.0f)
    , m_dwResourceID(0)
{
    m_dwValid = 0;
}

cltQuestMark::~cltQuestMark() = default;

void cltQuestMark::Process()
{
    if (!m_dwValid)
        return;

    if (m_wCurrentFrame > 5)
    {
        m_wCurrentFrame = 0;
        m_fAniFrame = 0.0f;
    }

    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(8u, m_dwResourceID, 0, 1);
    float screenY = static_cast<float>(m_nPosY - dword_A7308C);
    float screenX = static_cast<float>(m_nPosX - dword_A73088);
    m_pGameImage->SetBlockID(m_wCurrentFrame);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->SetPosition(screenX, screenY);
    m_pGameImage->SetAlpha(255);
    m_pGameImage->m_bFlag_450 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->Process();

    m_fAniFrame += 0.2f / static_cast<float>(SETTING_FRAME);
    m_wCurrentFrame = static_cast<std::uint16_t>(static_cast<std::int64_t>(m_fAniFrame));
}

void cltQuestMark::Draw()
{
    if (m_dwValid && m_pGameImage)
        m_pGameImage->Draw();
}

void cltQuestMark::Initialize(std::uint16_t npcID, float posX, float posY, std::uint8_t type)
{
    m_siField4 = npcID;
    m_nPosX = static_cast<int>(static_cast<std::int64_t>(posX));
    m_nPosY = static_cast<int>(static_cast<std::int64_t>(posY - 95.0f));
    m_siType = static_cast<unsigned short>(type);
    m_dwField3 = 1;
    m_dwResourceID = 184550646;
    if (npcID == 26780)
        m_dwResourceID = 486539698;
}

cltRewardMark::cltRewardMark()
    : CBaseObject()
    , m_pGameImage(nullptr)
    , m_wCurrentFrame(0)
    , _pad(0)
    , m_fAniFrame(0.0f)
    , m_dwResourceID(0)
{
    m_dwValid = 0;
}

cltRewardMark::~cltRewardMark() = default;

void cltRewardMark::Process()
{
    if (!m_dwValid)
        return;

    if (m_wCurrentFrame > 5)
    {
        m_wCurrentFrame = 0;
        m_fAniFrame = 0.0f;
    }

    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(8u, m_dwResourceID, 0, 1);
    float screenY = static_cast<float>(m_nPosY - dword_A7308C);
    float screenX = static_cast<float>(m_nPosX - dword_A73088);
    m_pGameImage->SetBlockID(m_wCurrentFrame);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->SetPosition(screenX, screenY);
    m_pGameImage->SetAlpha(255);
    m_pGameImage->m_bFlag_450 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->Process();

    m_fAniFrame += 0.2f / static_cast<float>(SETTING_FRAME);
    m_wCurrentFrame = static_cast<std::uint16_t>(static_cast<std::int64_t>(m_fAniFrame));
}

void cltRewardMark::Draw()
{
    if (m_dwValid && m_pGameImage)
        m_pGameImage->Draw();
}

void cltRewardMark::Initialize(std::uint16_t npcID, float posX, float posY, std::uint8_t type)
{
    m_siField4 = npcID;
    m_nPosX = static_cast<int>(static_cast<std::int64_t>(posX));
    m_nPosY = static_cast<int>(static_cast<std::int64_t>(posY - 95.0f));
    m_siType = static_cast<unsigned short>(type);
    m_dwField3 = 1;
    m_dwResourceID = 184550647;
    if (npcID == 26780)
        m_dwResourceID = 486539699;
}
