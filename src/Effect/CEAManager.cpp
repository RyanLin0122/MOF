#include "Effect/CEAManager.h"
#include "Effect/CCAEffect.h"
#include "CMOFPacking.h" // ���]���ʸ��ɺ޲z��
#include <new>           // for std::nothrow

// ���]�������ܼ�
extern int IsInMemory; // �X�СG1 = �q�ʸ���Ū���A0 = �q�W���ɮ�Ū��

// �R�A��Ҫ�l��
CEAManager* CEAManager::s_pInstance = nullptr;

CEAManager* CEAManager::GetInstance() {
    if (!s_pInstance) {
        s_pInstance = new (std::nothrow) CEAManager();
    }
    return s_pInstance;
}

// �����ϲ�Ķ�X: 0x0053A390
CEAManager::CEAManager() {
    // �N��ӫ��а}�C��l�Ƭ� nullptr
    memset(m_pEaData, 0, sizeof(m_pEaData));
}

// �����ϲ�Ķ�X: 0x0053A3B0
CEAManager::~CEAManager() {
    Reset();
}

// �����ϲ�Ķ�X: 0x0053A480
void CEAManager::Reset() {
    for (int i = 0; i < 65535; ++i) {
        if (m_pEaData[i]) {
            // �ѩ� EADATALISTINFO ���Ѻc�禡�|�B�z�����}�C������A
            // �o�̥u�ݭn delete �̤W�h������Y�i�C
            delete m_pEaData[i];
            m_pEaData[i] = nullptr;
        }
    }
}

// �����ϲ�Ķ�X: 0x0053A4C0
/**
 * @brief ������w���S�İʵe�ƾڡA�ñN��j�w��@�� CCAEffect ����W�C
 * @param effectID �S�Ī��ߤ@ ID�A�@���֨��}�C�����ޡC
 * @param szFileName �S�Ī��ɮצW�١A�Ȧb�ƾک|�����J�ɨϥΡC
 * @param pEffect �n�����ƾڪ� CCAEffect ������СC
 * @note ���禡���޿��T�٭�� Effectall.c �� 0x0053A4C0 �� CEAManager::GetEAData �禡�C
 * ����{�F�@���i�~�����J (Lazy Loading) ���֨�����C
 */
void CEAManager::GetEAData(int effectID, const char* szFileName, CCAEffect* pEffect)
{
    // �ѼƦ��ĩ��ˬd
    if (effectID < 0 || effectID >= 65535 || !pEffect) {
        return;
    }

    // �B�J 1: �ˬd�֨����O�_�w�s�b�ӯS�ļƾ�
    // ��l�X: v5 = *((_DWORD *)this + a2); if ( v5 ) ...
    if (m_pEaData[effectID] == nullptr)
    {
        // �B�J 2: �p�G�ƾڤ��b�֨����A�h���J��

        // 2a. ���s���S�ļƾڤ��t�D���c�O����
        // ��l�X: v7 = operator new(0x24u);
        m_pEaData[effectID] = new (std::nothrow) EADATALISTINFO();
        if (!m_pEaData[effectID]) {
            // �O������t���ѡA�L�k�~��
            return;
        }

        // 2b. �ھڥ���X�СA�M�w�O�q�W���ɮ��٬O�ʸ��ɤ�Ū���øѪR�ƾ�
        // ��l�X: if ( dword_829254 ) LoadEAInPack(..); else LoadEA(..);
        if (IsInMemory) {
            // �`�N�GLoadEAInPack �ݭn�@�ӥi�ק諸 char*�A�]����̿઺
            // CMofPacking::ChangeString �i��|�ק���|�]���ޥ���ڤW�O�ƻs��ק�^�C
            // ���F�P��l�X�欰�@�P���קK const_cast�A�ڭ̳Ыؤ@���{�ɰƥ��C
            char tempFileName[256];
            strcpy_s(tempFileName, sizeof(tempFileName), szFileName);
            LoadEAInPack(effectID, tempFileName);
        }
        else {
            LoadEA(effectID, szFileName);
        }
    }

    // �B�J 3: �N�֨������ƾڸj�w��ǤJ�� CCAEffect ����
    EADATALISTINFO* pData = m_pEaData[effectID];

    // 3a. �]�w CCAEffect ���D�ƾګ���
    // ��l�X: *((_DWORD *)a4 + 1) = v5;
    pEffect->SetData(pData);

    // 3b. �ƻs��V���A
    // ��l�X: *((_BYTE *)v6 + 80) = *(_BYTE *)(... + 24); ...
    // �`�N�G�b�ڭ̪��٭줤�ACCAEffect ���A�����x�s�o�Ǫ��A�A
    // �ӬO���Ӧb Draw �ɱq pData ��Ū���C�����F����������l�欰�A
    // CEAManager �@�� friend ���O�����]�w CCAEffect �����������C

    // 3c. �]�w��V���A��ܺX��
    // ��l�X: *((_BYTE *)v6 + 85) = v8 > 7u; (v8 �O BlendOp)
    if (pData->m_ucBlendOp > 7) {
        pEffect->m_ucRenderStateSelector = 1; // �ϥ� DrawEtcRenderState
    }
    else {
        pEffect->m_ucRenderStateSelector = 0; // �ϥ� DrawRenderState
    }

    // ��l�X���٦��@�� *((_DWORD *)v6 + 2) = ... + 12;
    // �o�b�{�N C++ ���O�����n���A�]���Ҧ��ƾڳ��i�H�z�L pData ���Цs���A
    // �G�b�ڭ̪��٭줤�N��ٲ��H�����{���X�M���סC
}

/**
 * @brief �q�ʸ��� (mof.pak) ���J .ea �ƾڡC
 * @param effectID �n���J���S�� ID�A�]�@�� m_pEaData �}�C�����ޡC
 * @param szFileName .ea �ɮצb�ʸ��ɤ����W�١C
 * @note ���禡���޿��T�٭�� Effectall.c �� 0x0053A590 �� LoadTimelineInPack �禡�C
 * "LoadTimelineInPack" �b�ϲ�Ķ�ɬO�@�өR�W���~�A��\���ڤW�O���J .ea (Effect Animation) �ɮסC
 */

void CEAManager::LoadEAInPack(int effectID, char* szFileName)
{
    // ������V EADATALISTINFO ���c�����СA�ӵ��c�w�b GetEAData �����t
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) return;

    // �ǳ��ɮ׸��|�ñq�ʸ���Ū���ƾڨ�O����w�İ�
    char szChangedPath[256];
    strcpy_s(szChangedPath, szFileName);
    CMofPacking* packer = CMofPacking::GetInstance();
    char* fileBuffer = packer->FileRead(packer->ChangeString(szChangedPath));

    if (!fileBuffer) {
        // �p�GŪ�����ѡA��ܿ��~�T��
        char errorMsg[512];
        sprintf_s(errorMsg, sizeof(errorMsg), "%s CA didn't find.", szFileName);
        MessageBoxA(nullptr, errorMsg, "LoadEAInPack Error", MB_OK);
        return;
    }

    // �ϥΤ@�ӫ��ШӹM���O����w�İ�
    char* current_ptr = fileBuffer;

    // --- �}�l�`�ǸѪR�ɮ׽w�İ� ---

    // 1. Ū�� EADATALISTINFO �����h����
    // ��l�X: *(_DWORD *)a3 = dword_C24CF4; ... qmemcpy((char *)a3 + 4, &dword_C24CF8, 0xFFu);
    // �o�̧ڭ̥Χ��T�B�v�Ӧ������覡���٭�
    pData->m_nVersion = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    pData->m_nLayerCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    pData->m_nTotalFrames = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);

    // 2. �ھ� LayerCount ���t��Ū���ϼh (Layers) �ƾ�
    if (pData->m_nLayerCount > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) return; // �O������t����

        for (int i = 0; i < pData->m_nLayerCount; ++i) {
            VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[i];

            // Ū���ӹϼh���v��� (FrameCount)
            // ��l�X: v14 = *v13; ... *(_DWORD *)(v11 + ... + 52) = v14;
            pLayer->m_nFrameCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);

            // 3. �ھ� FrameCount ���t��Ū���v�� (Frames) �ƾ�
            if (pLayer->m_nFrameCount > 0) {
                pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[pLayer->m_nFrameCount];
                if (!pLayer->m_pFrames) return;

                // Ū���Ҧ��v�檺�ƾڶ�
                // ��l�X: qmemcpy((void *)(v20 + *(_DWORD *)(v12 + v21 + 4)), v6, 0x74u); ... 
                // �o�̪��޿������A�]���C�Ӽv��i���٥]�t�@�Ӥl�}�C�C
                for (int j = 0; j < pLayer->m_nFrameCount; ++j) {
                    VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[j];

                    // �ƻs VERTEXANIMATIONFRAMEINFO ���D�n���� (116 bytes)
                    memcpy(pFrame, current_ptr, 116);
                    current_ptr += 116;

                    // Ū���l�}�C���j�p
                    // ��l�X: if ( *(_DWORD *)(v25 + v20 + 116) ) ...
                    // �g���ɡA�o�����ݩ� VERTEXANIMATIONFRAMEINFO ����������
                    pFrame->m_dwUnknown1 = *reinterpret_cast<unsigned int*>(current_ptr); current_ptr += sizeof(unsigned int);
                    pFrame->m_dwUnknown2 = *reinterpret_cast<unsigned int*>(current_ptr); current_ptr += sizeof(unsigned int);

                    // �p�G�l�}�C�s�b�A�hŪ���䤺�e (��l�X�����޿�A�� CCAEffect::Process ���ϥ�)
                    if (pFrame->m_dwUnknown1 > 0) {
                        size_t subDataSize = pFrame->m_dwUnknown1 * 102; // 0x66
                        //pFrame->m_pUnknownSubData = new char[subDataSize];
                        //memcpy(pFrame->m_pUnknownSubData, current_ptr, subDataSize);
                        current_ptr += subDataSize;
                    }
                }

            }
            else {
                pLayer->m_pFrames = nullptr;
            }
        }
    }
    else {
        pData->m_pLayers = nullptr;
    }

    // 4. Ū���ʵe���q (KeyFrame) ��T
    pData->m_nAnimationCount = *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int);
    *reinterpret_cast<int*>(current_ptr); current_ptr += sizeof(int); // ��l�X���@�Ӧh�l��Ū���A�i��O�`�v���

    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (!pData->m_pKeyFrames) return;

        size_t keyinfoDataSize = sizeof(KEYINFO) * pData->m_nAnimationCount;
        memcpy(pData->m_pKeyFrames, current_ptr, keyinfoDataSize);
        current_ptr += keyinfoDataSize;
    }
    else {
        pData->m_pKeyFrames = nullptr;
    }

    // 5. Ū����V���A (Render States)
    pData->m_ucBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcBlendOp = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcSrcBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;
    pData->m_ucEtcDestBlend = *reinterpret_cast<unsigned char*>(current_ptr); current_ptr++;

    // �ɮ׸ѪR�����ACMofPacking ���w�İϷ|�b�䤺���޲z�A���B��������
}

/**
 * @brief �q�W�ߪ� .ea �ɮ׸��J�S�İʵe�ƾڡC
 * @param effectID �n���J���S�� ID�A�]�@�� m_pEaData �}�C�����ޡC
 * @param szFileName .ea �ɮת�������|�C
 * @note ���禡���޿��T�٭�� Effectall.c �� 0x0053AA50 ���禡�C
 * ���t�d�`��Ū���ɮסA�ðʺA���t�O����ӫإߤ@�ӧ��㪺 EADATALISTINFO �ƾڵ��c�C
 */
void CEAManager::LoadEA(int effectID, const char* szFileName)
{
    // ������V EADATALISTINFO ���c�����СA�ӵ��c�w�b GetEAData ���w�����t
    EADATALISTINFO* pData = m_pEaData[effectID];
    if (!pData) {
        return;
    }

    FILE* pFile = nullptr;
    // �ϥ� fopen_s �H�G�i��Ū���Ҧ�("rb")�w���a�}���ɮ�
    if (fopen_s(&pFile, szFileName, "rb") != 0 || pFile == nullptr) {
        // �ɮ׶}�ҥ��ѡA��ܿ��~�T���A�欰�P��l�X�@�P
        char errorMsg[512];
        sprintf_s(errorMsg, sizeof(errorMsg), "%s Effect info file is not found.", szFileName);
        MessageBoxA(nullptr, errorMsg, "CEAManager::LoadEA Error", MB_OK);
        return;
    }

    // --- �}�l�`��Ū���ɮ� ---

    // 1. Ū�� EADATALISTINFO �����h����
    fread(&pData->m_nVersion, sizeof(pData->m_nVersion), 1, pFile);
    fread(&pData->m_nLayerCount, sizeof(pData->m_nLayerCount), 1, pFile);
    fread(&pData->m_nTotalFrames, sizeof(pData->m_nTotalFrames), 1, pFile);

    // 2. �ھ� LayerCount ���t��Ū���ϼh (Layers) �ƾ�
    if (pData->m_nLayerCount > 0) {
        pData->m_pLayers = new (std::nothrow) VERTEXANIMATIONLAYERINFO[pData->m_nLayerCount];
        if (!pData->m_pLayers) { fclose(pFile); return; }

        for (int i = 0; i < pData->m_nLayerCount; ++i) {
            VERTEXANIMATIONLAYERINFO* pLayer = &pData->m_pLayers[i];

            // Ū���ӹϼh���v��� (FrameCount)
            fread(&pLayer->m_nFrameCount, sizeof(pLayer->m_nFrameCount), 1, pFile);

            // 3. �ھ� FrameCount ���t��Ū���v�� (Frames) �ƾ�
            if (pLayer->m_nFrameCount > 0) {
                pLayer->m_pFrames = new (std::nothrow) VERTEXANIMATIONFRAMEINFO[pLayer->m_nFrameCount];
                if (!pLayer->m_pFrames) { fclose(pFile); return; }

                // �@����Ū���Ҧ��v�檺�ƾڶ�
                fread(pLayer->m_pFrames, sizeof(VERTEXANIMATIONFRAMEINFO), pLayer->m_nFrameCount, pFile);

                // �b��l�X���A�C�Ӽv��᭱�i���ۤ@�Ӥl�ƾڶ��A�o�̤]�nŪ��
                for (int j = 0; j < pLayer->m_nFrameCount; ++j) {
                    VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[j];
                    // �ѩ�ڭ̤w�N���������]�t�b���c�餤�A�W���� fread �w�gŪ���F���̡C
                    // �p�G�o�ǥ��������O���СA�h�ݭn�b���B�i�@�BŪ�����̫��V�����e�C
                    // �ھڤ��e�����R�A���̦��G�O�T�w�j�p���ƾکέp�ơA�w�QŪ���C
                }

            }
            else {
                pLayer->m_pFrames = nullptr;
            }
        }
    }
    else {
        pData->m_pLayers = nullptr;
    }

    // 4. Ū���ʵe���q (KeyFrame) ��T
    fread(&pData->m_nAnimationCount, sizeof(pData->m_nAnimationCount), 1, pFile);
    // ��l�X�bŪ�� AnimationCount ���٦��@��4�줸�ժ�Ū���A�����O�`�v��ƪ����ƩΥt�@�ӥ��ϥ����
    int dummy;
    fread(&dummy, sizeof(int), 1, pFile);

    if (pData->m_nAnimationCount > 0) {
        pData->m_pKeyFrames = new (std::nothrow) KEYINFO[pData->m_nAnimationCount];
        if (!pData->m_pKeyFrames) { fclose(pFile); return; }

        // �@����Ū���Ҧ� KeyInfo ���ƾڶ�
        fread(pData->m_pKeyFrames, sizeof(KEYINFO), pData->m_nAnimationCount, pFile);
    }
    else {
        pData->m_pKeyFrames = nullptr;
    }

    // 5. Ū����V���A (Render States)
    fread(&pData->m_ucBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucDestBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcBlendOp, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcSrcBlend, sizeof(unsigned char), 1, pFile);
    fread(&pData->m_ucEtcDestBlend, sizeof(unsigned char), 1, pFile);

    // --- �ɮ�Ū������ ---
    fclose(pFile);
}