#include "Test/EffectSystemTest.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEAManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h"
#include "Effect/CEffect_Battle_DownCut.h"
#include "Effect/CEffect_Battle_Hit_Normal.h"
#include "Effect/CEffect_Battle_UpperCut.h"
#include "Effect/CEffect_Battle_HorizonCut.h"
#include "Effect/CEffect_Battle_MagicBook.h"

#include "Character/ClientCharacter.h"
#include "global.h"
#include <stdio.h>

EffectSystemTest::EffectSystemTest() :
    m_pCaster(nullptr),
    m_fTimeSinceLastEffect(0.0f)
{
}

EffectSystemTest::~EffectSystemTest()
{
    Cleanup();
}

HRESULT EffectSystemTest::Initialize()
{
    printf("--- [EffectSystemTest] ��l�ƶ}�l ---\n");

    // �B�J 1: �T�O�Ҧ��������޲z�����w�إߡC
    // GetInstance() �|�b�ݭn�ɦ۰ʫإ߳�Ҫ���C
    CEffectManager::GetInstance();
    CEAManager::GetInstance();

    // �`�N�GcltMoFC_EffectKindInfo �޲z�����M�s�b�A�� CEffect_Battle_DownCut
    // �O������Ҥƪ��A���z�L CEffectManager �� AddEffect �u�t�禡�C
    // �]���A�ثe�ڭ̤��ݭn��l�� g_clEffectKindInfo->Initialize(filename)�C

    // �B�J 2: �إߤ@�Ӽ������I�k�̨���C
    printf("  ���b�إ߼����I�k�� (ClientCharacter)...\n");
    m_pCaster = new ClientCharacter();
    if (!m_pCaster) {
        printf("���~�G�إ� ClientCharacter ���ѡC\n");
        return E_FAIL;
    }
	m_pTarget = new ClientCharacter();
    m_pTarget->SetPosX(900);
	m_pTarget->SetPosY(200);
    if (!m_pTarget) {
        printf("���~�G�إ� ClientCharacter ���ѡC\n");
        return E_FAIL;
	}
    printf("--- [EffectSystemTest] ��l�Ʀ��\ ---\n");
    return S_OK;
}

void EffectSystemTest::Cleanup()
{
    printf("--- [EffectSystemTest] �M�z�귽 ---\n");

    // �z�L CEffectManager �R���Ҧ����b���ʤ����S�ġC
    CEffectManager::GetInstance()->BulletListAllDel();

    // �R����������C
    if (m_pCaster) {
        delete m_pCaster;
        m_pCaster = nullptr;
        printf("  �w�R�������I�k�̡C\n");
    }
}

void EffectSystemTest::Update(float fElapsedTime)
{
    // �֥[�ɶ�
    m_fTimeSinceLastEffect += fElapsedTime;

    // �C�j 0.5 ���ͤ@�ӷs���U�A�ٯS��
    if (m_fTimeSinceLastEffect > 3.0f) {
        //SpawnDownCutEffect();
        //SpawnUpperCutEffect();
		//SpawnHitNormalEffect();
        //SpawnHorizenCutEffect();
        SpawnMagicBookEffect();
        m_fTimeSinceLastEffect = 0.0f; // ���m�p�ɾ�
    }

    // ��s�Ҧ��b CEffectManager �����U���S�ġC
    // �o�|�I�s�C�ӯS�Ī� FrameProcess �禡�A�B�z��ͩR�g���C
    CEffectManager::GetInstance()->FrameProcess(fElapsedTime, false);
}

void EffectSystemTest::Render()
{
    // �B�J 1: �I�s�Ҧ��S�Ī� Process �禡�C
    // �o�|��s�S�Ī���m�B�i���ʵ�ø�s�e�һݪ����A�C
    CEffectManager::GetInstance()->Process();

    // �B�J 2: �I�s�Ҧ��S�Ī� Draw �禡�C
    // �o�|�N�S�Ī����I�ƾڴ��浹��V�޽u�C
    CEffectManager::GetInstance()->Draw();
}

void EffectSystemTest::SpawnDownCutEffect()
{
    if (!m_pCaster) return;

    printf("  ���b���� CEffect_Battle_DownCut �S��...\n");

    // �B�J 1: ���� new �@�� CEffect_Battle_DownCut ����C
    // ��غc�禡�|�۰ʦV CEAManager �ШD "efn_downcut.ea" �S�ļƾڡC
    CEffect_Battle_DownCut* pEffect = new CEffect_Battle_DownCut();

    // �B�J 2: �q�������������m�C
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // �B�J 3: �]�w�S�Ī���m�P��V (�H��½��H���ը�ر��p)�C
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // �B�J 4: �N�إߦn���S�Ĺ�ҥ[�J�� CEffectManager ���޲z�쵲��C���C
    // �o�O����B�J�A�u���Q�[�J���S�Ĥ~�|�Q��s�Mø�s�C
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHitNormalEffect()
{
    if (!m_pCaster) return;

    printf("  ���b���� CEffect_Battle_Hit_Normal �S��...\n");

    // �B�J 1: ���� new �@�� CEffect_Battle_DownCut ����C
    // ��غc�禡�|�۰ʦV CEAManager �ШD "efn_downcut.ea" �S�ļƾڡC
    CEffect_Battle_Hit_Normal* pEffect = new CEffect_Battle_Hit_Normal();

    // �B�J 2: �q�������������m�C
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // �B�J 3: �]�w�S�Ī���m�P��V (�H��½��H���ը�ر��p)�C
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y);

    // �B�J 4: �N�إߦn���S�Ĺ�ҥ[�J�� CEffectManager ���޲z�쵲��C���C
    // �o�O����B�J�A�u���Q�[�J���S�Ĥ~�|�Q��s�Mø�s�C
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUpperCutEffect()
{
    if (!m_pCaster) return;

    printf("  ���b���� CEffect_Battle_UpperCut �S��...\n");

    // �B�J 1: ���� new �@�� CEffect_Battle_DownCut ����C
    // ��غc�禡�|�۰ʦV CEAManager �ШD "efn_downcut.ea" �S�ļƾڡC
    CEffect_Battle_UpperCut* pEffect = new CEffect_Battle_UpperCut();

    // �B�J 2: �q�������������m�C
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // �B�J 3: �]�w�S�Ī���m�P��V (�H��½��H���ը�ر��p)�C
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // �B�J 4: �N�إߦn���S�Ĺ�ҥ[�J�� CEffectManager ���޲z�쵲��C���C
    // �o�O����B�J�A�u���Q�[�J���S�Ĥ~�|�Q��s�Mø�s�C
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHorizenCutEffect()
{
    if (!m_pCaster) return;

    printf("  ���b���� CEffect_Battle_UpperCut �S��...\n");

    // �B�J 1: ���� new �@�� CEffect_Battle_DownCut ����C
    // ��غc�禡�|�۰ʦV CEAManager �ШD "efn_downcut.ea" �S�ļƾڡC
    CEffect_Battle_HorizonCut* pEffect = new CEffect_Battle_HorizonCut();

    // �B�J 2: �q�������������m�C
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // �B�J 3: �]�w�S�Ī���m�P��V (�H��½��H���ը�ر��p)�C
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    // �B�J 4: �N�إߦn���S�Ĺ�ҥ[�J�� CEffectManager ���޲z�쵲��C���C
    // �o�O����B�J�A�u���Q�[�J���S�Ĥ~�|�Q��s�Mø�s�C
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMagicBookEffect()
{
    if (!m_pCaster) return;
    if (!m_pTarget) return;
    printf("  ���b���� CEffect_Battle_UpperCut �S��...\n");

    // �B�J 1: ���� new �@�� CEffect_Battle_DownCut ����C
    // ��غc�禡�|�۰ʦV CEAManager �ШD "efn_downcut.ea" �S�ļƾڡC
    CEffect_Battle_MagicBook* pEffect = new CEffect_Battle_MagicBook();

    // �B�J 2: �q�������������m�C
    float x = static_cast<float>(m_pCaster->GetPosX());
    float y = static_cast<float>(m_pCaster->GetPosY());

    // �B�J 3: �]�w�S�Ī���m�P��V (�H��½��H���ը�ر��p)�C
    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(m_pCaster, m_pTarget, bFlip, 0);

    // �B�J 4: �N�إߦn���S�Ĺ�ҥ[�J�� CEffectManager ���޲z�쵲��C���C
    // �o�O����B�J�A�u���Q�[�J���S�Ĥ~�|�Q��s�Mø�s�C
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}