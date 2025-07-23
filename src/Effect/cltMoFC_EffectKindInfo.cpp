#include "Effect/cltMoFC_EffectKindInfo.h"
#include "cltTextFileManager.h" // ���]���ɮ׺޲z��
#include <new>

// ���]�������ɮ׺޲z�����
extern cltTextFileManager g_clTextFileManager;

// �����ϲ�Ķ�X: 0x0053BC30
cltMoFC_EffectKindInfo::cltMoFC_EffectKindInfo()
{
    // �N���а}�C���Ҧ�������l�Ƭ��ū���
    memset(m_pEffectInfo, 0, sizeof(m_pEffectInfo));
}

// �����ϲ�Ķ�X: 0x0053BC70 (���t�F�令�����M�z)
cltMoFC_EffectKindInfo::~cltMoFC_EffectKindInfo()
{
    // ����Ҧ��b Initialize ���ʺA���t�� stEffectKindInfo ����
    for (int i = 0; i < 65535; ++i) {
        if (m_pEffectInfo[i]) {
            delete m_pEffectInfo[i];
            m_pEffectInfo[i] = nullptr;
        }
    }
}

// �����ϲ�Ķ�X: 0x0053BC80
int cltMoFC_EffectKindInfo::Initialize(char* szFileName)
{
    FILE* pFile = g_clTextFileManager.fopen(szFileName);
    if (!pFile) {
        return 0; // �ɮ׶}�ҥ���
    }

    char buffer[1024];
    const char* delimiters = "\t\n"; // �ϥ� Tab �M����ŧ@�����j��

    // ���L�ɮ׼��Y (��l�XŪ���F4���A�e3���i��O���ѩμ��D��)
    for (int i = 0; i < 4; ++i) {
        if (!fgets(buffer, sizeof(buffer), pFile)) {
            g_clTextFileManager.fclose(pFile);
            return 1; // �ɮפ��e������A���������`����
        }
    }

    // �}�l�v��ѪR
    do {
        char* token = strtok(buffer, delimiters);
        if (!token) continue; // �Ŧ�

        unsigned short kindID = TranslateKindCode(token);
        if (kindID == 0) continue; // �L�Ī� Kind Code

        // �p�G�� ID �|���Q�w�q
        if (m_pEffectInfo[kindID] == nullptr) {
            stEffectKindInfo* pNewInfo = new (std::nothrow) stEffectKindInfo();
            if (!pNewInfo) break; // �O������t����

            m_pEffectInfo[kindID] = pNewInfo;
            pNewInfo->usKindID = kindID;

            // �ѪR�ɮצW�� (���L�ĤG�����)
            strtok(nullptr, delimiters);
            token = strtok(nullptr, delimiters);
            if (token) {
                strcpy_s(pNewInfo->szFileName, sizeof(pNewInfo->szFileName), token);
            }

            // �ѪR�S������
            token = strtok(nullptr, delimiters);
            if (token) {
                if (_stricmp(token, "ONCE") == 0) pNewInfo->ucType = EFFECT_TYPE_ONCE;
                else if (_stricmp(token, "DIRECT") == 0) pNewInfo->ucType = EFFECT_TYPE_DIRECTED;
                else if (_stricmp(token, "SUSTAIN") == 0) pNewInfo->ucType = EFFECT_TYPE_SUSTAIN;
                else if (_stricmp(token, "SHOOTUNIT") == 0) pNewInfo->ucType = EFFECT_TYPE_SHOOT_UNIT;
                else if (_stricmp(token, "SHOOTNOTEA") == 0) pNewInfo->ucType = EFFECT_TYPE_SHOOT_NO_EA;
                else if (_stricmp(token, "ITEMONCE") == 0) pNewInfo->ucType = EFFECT_TYPE_ITEM_ONCE;
                else pNewInfo->ucType = EFFECT_TYPE_UNKNOWN;
            }
        }
    } while (fgets(buffer, sizeof(buffer), pFile));

    g_clTextFileManager.fclose(pFile);
    return 1; // ��l�Ʀ��\
}

// �����ϲ�Ķ�X: 0x0053BF20
stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(unsigned short kindID)
{
    // �����ϥ� ID �@�����ެd�߰}�C
    if (kindID > 0 && kindID < 65535) {
        return m_pEffectInfo[kindID];
    }
    return nullptr;
}

// �����ϲ�Ķ�X: 0x0053BF30
stEffectKindInfo* cltMoFC_EffectKindInfo::GetEffectKindInfo(char* szKindCode)
{
    unsigned short kindID = TranslateKindCode(szKindCode);
    return GetEffectKindInfo(kindID);
}

// �����ϲ�Ķ�X: 0x0053BF50
unsigned short cltMoFC_EffectKindInfo::TranslateKindCode(char* szKindCode)
{
    if (strlen(szKindCode) != 5) {
        return 0; // �榡����
    }

    // �N�Ĥ@�Ӧr�� ('A'~'P') �ഫ�����줸
    int high_part = (toupper(szKindCode[0]) + 31) << 11;

    // �N��|��Ʀr�ഫ���C�줸
    int low_part = atoi(szKindCode + 1);

    // �ˬd�Ʀr�����O�_�b���Ľd�� (0-2047)
    if (low_part < 0x800) { // 0x800 = 2048
        return static_cast<unsigned short>(high_part | low_part);
    }

    return 0; // �W�X�d��
}