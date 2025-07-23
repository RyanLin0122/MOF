#include "Test/ImageDrawTest.h"
#include "Image/cltImageManager.h"
#include "Image/ResourceMgr.h"
#include "Image/GameImage.h"
#include "CMOFPacking.h"
#include <stdio.h>

ImageDrawTest::ImageDrawTest()
{
    // �b�غc�禡���A�N�Ҧ����Ъ�l�Ƭ� nullptr�A�T�O�w��
    m_pImageManager = nullptr;
    m_pItemImage1 = nullptr;
    m_pItemImage2 = nullptr;
    m_pMapBgImage = nullptr;
}

ImageDrawTest::~ImageDrawTest()
{
    // �b�Ѻc�ɡA�۰ʩI�s Cleanup �禡����Ҧ��귽
    Cleanup();
}

HRESULT ImageDrawTest::Initialize()
{
    printf("--- [ImageDrawTest] ��l�ƴ��ճ��� ---\n");

    // 1. �}�� VFS (�����ɮרt��)
    // ResourceMgr ���غc�禡�|�ˬd mof.pak �O�_�s�b�ó]�w m_bUsePackFile �X�СC
    printf("  [ImageDrawTest] ���b�}�� VFS �ʸ��� mof.pak...\n");
    if (!CMofPacking::GetInstance()->PackFileOpen("mof"))
    {
        printf("  [ImageDrawTest] ���~�G�L�k�}�� mof.pak/mof.paki�C\n");
        return E_FAIL;
    }
    printf("  [ImageDrawTest] VFS �}�Ҧ��\�C\n");

    // 2. ��l�ƹϤ�������޲z��
    printf("  [ImageDrawTest] ���b��l�� cltImageManager...\n");
    m_pImageManager = cltImageManager::GetInstance();
    m_pImageManager->Initialize(); // ������������Ҧ��Ϥ��w���إ߳��I�w�İ�
    printf("  [ImageDrawTest] cltImageManager ��l�Ƨ����C\n");

    // 3. �P�B���J�z���w���귽
    printf("  [ImageDrawTest] ���b�P�B���J�귽...\n");

    // ���J���~ 1 (ID: 201326853)
    printf("    ���J���~ 1 (ID: 201326853, Type: RES_ITEM)...\n");
    m_pItemImage1 = m_pImageManager->GetGameImage(ResourceMgr::RES_ITEM, 201326853);
    if (!m_pItemImage1) { printf("    ���J����!\n"); return E_FAIL; }

    // ���J���~ 2 (ID: 201327617)
    printf("    ���J���~ 2 (ID: 201327617, Type: RES_ITEM)...\n");
    m_pItemImage2 = m_pImageManager->GetGameImage(ResourceMgr::RES_ITEM, 201327617);
    if (!m_pItemImage2) { printf("    ���J����!\n"); return E_FAIL; }

    // ���J�a�ϭI�� (ID: 536871078)
    printf("    ���J�a�ϭI�� (ID: 536871078, Type: RES_MAPBG)...\n");
    m_pMapBgImage = m_pImageManager->GetGameImage(ResourceMgr::RES_MAPBG, 536871078);
    if (!m_pMapBgImage) { printf("    ���J����!\n"); return E_FAIL; }

    printf("  [ImageDrawTest] �Ҧ��귽���J���\�C\n");

    return S_OK;
}

void ImageDrawTest::Render()
{
    // �ˬd�޲z���O�_�s�b�A�T�O�w��
    if (!m_pImageManager) return;

    // --- �]�w���󪬺A ---
    // �bø�s�e�A���C�Ӫ���]�w��m�B�ʵe�v��B�Y���ݩ�

    // �B�z�a�ϭI��
    if (m_pMapBgImage)
    {
        m_pMapBgImage->SetPosition(0, 0);     // ��b���W��
        m_pMapBgImage->SetBlockID(0);         // ��ܲĤ@�ӵe��
        m_pMapBgImage->Process();             // �p�⳻�I
    }

    // �B�z���~ 1
    if (m_pItemImage1)
    {
        m_pItemImage1->SetPosition(100, 200);  // ���w��m
        m_pItemImage1->SetBlockID(0);
        m_pItemImage1->SetScale(150);         // ��j�� 150%
        m_pItemImage1->Process();
    }

    // �B�z���~ 2
    if (m_pItemImage2)
    {
        m_pItemImage2->SetPosition(300, 200);  // ���w��m
        m_pItemImage2->SetBlockID(0);
        m_pItemImage2->SetFlipX(true);        // ����½��
        m_pItemImage2->Process();
    }

    // --- ���ø�s ---
    // ø�s���ǫܭ��n�A��e���|�\�b���e���W��
    if (m_pMapBgImage) m_pMapBgImage->Draw(); //
    if (m_pItemImage1) m_pItemImage1->Draw();
    if (m_pItemImage2) m_pItemImage2->Draw();
}

void ImageDrawTest::Cleanup()
{
    if (m_pImageManager != nullptr)
    {
        printf("  [ImageDrawTest] ���b����Ҧ� GameImage...\n");
        m_pImageManager->ReleaseAllGameImage(); //

        printf("  [ImageDrawTest] ���b�R�� cltImageManager...\n");
        delete m_pImageManager;
        m_pImageManager = nullptr;
    }

    // �ѩ� CMofPacking �M ResourceMgr �O��ҡA���̪��ͩR�g���� GetInstance/DestroyInstance �޲z
    // �b�o�̧ڭ̤���ʧR�����̡A��ѥD�{���� Cleanup �禡�Τ@�B�z�C
}