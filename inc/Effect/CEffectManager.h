#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CEffect_OverMind_Screen.h"
#include "Effect/CEffect_Skill_Type_Once.h" // ���M�b���Ѫ��{���X��������γ~�A���̾ګغc�禡�٭�
#include "Effect/cltMoFC_EffectKindInfo.h"

/**
 * @struct EffectInfo
 * @brief �Ω�걵 CEffectBase �������V�쵲��C�`�I�C
 * (�j�p: 12 bytes)
 */
struct EffectInfo {
    EffectInfo* pPrev;       // ���V�W�@�Ӹ`�I
    EffectInfo* pNext;       // ���V��@�Ӹ`�I
    CEffectBase* pEffect;    // ���V��ڪ��S�Ī���
};

/**
 * @class CEffectManager
 * @brief �S�ĺ޲z���A�ϥγ�ҼҦ��C
 * * �t�d�Ҧ��ʺA�C���S�Ī��ͩR�g���޲z�C�����@�@�� CEffectBase ����
 * ���V�쵲��C�A�æb�C�@�V���B�z���̪���s�Bø�s�M�P���C
 */
class CEffectManager {
public:
    /// @brief ���o�ߤ@���޲z����ҡC
    static CEffectManager* GetInstance();

    /// @brief �Ѻc�禡�A�|�M�z�Ҧ��ݾl���S�ġC
    ~CEffectManager();

    // --- �S�ĺ޲z�֤ߨ禡 ---

    /// @brief �N�@�Ӥw�g�إߪ��S�Ī���[�J��޲z�����쵲��C���C
    /// @param pEffect ���V CEffectBase �l�����O���󪺫��СC
    void BulletAdd(CEffectBase* pEffect);

    /// @brief �ھگS�ĺ���ID�A�إߨå[�J�@�ӷs���S�ġC
    /// �o�O�D�n���S�Ĥu�t�禡�C
    /// @param effectKindID �S�ĺ�����ID (���� .ea �ɮ�)�C
    /// @param pCaster �I�k�̩Ψӷ�����C
    /// @param pTarget �ؼШ���C
    /// @param ... ��L�ѼƥΩ�S�w�S�ġC
    /// @return ���\�إߨå[�J���S�Ī�����СA���ѫh��^ nullptr�C
    CEffectBase* AddEffect(unsigned short effectKindID, ClientCharacter* pCaster, ClientCharacter* pTarget = nullptr, int a5 = 0, unsigned short a6 = 0, unsigned short a7 = 0, unsigned char a8 = 2);

    /// @brief �ھگS�ĦW�١A�إߨå[�J�@�ӷs���S�ġC
    CEffectBase* AddEffect(char* szEffectName, ClientCharacter* pCaster);

    /// @brief �B�z�Ҧ����޲z�S�Ī��ͩR�g���A�ò����w�������S�ġC
    /// @param fElapsedTime �g�L���ɶ��C
    /// @param bForceDeleteAll �O�_�j��R���Ҧ��S�ġC
    void FrameProcess(float fElapsedTime, bool bForceDeleteAll = false);

    /// @brief �I�s�Ҧ����޲z�S�Ī� Process �禡�A�ǳ�ø�s�ƾڡC
    void Process();

    /// @brief �I�s�Ҧ����޲z�S�Ī� Draw �禡�A�N��ø�s��ù��C
    void Draw();

    /// @brief �q�޲z������ʧR���@�ӫ��w���S�ġC
    /// @param pEffect �n�R�����S�Ī�����СC
    /// @return ���\��^ true�A���Ѫ�^ false�C
    bool DeleteEffect(CEffectBase* pEffect);

    /// @brief �B�z�S���B�D�쵲��C�޲z���S�ġ]�p���ù��ĪG�^�C
    void AddEtcEffect(unsigned short type, unsigned int accountID);

    /// @brief �M�ũҦ��S���쵲��C�C
    void BulletListAllDel();
	
    cltMoFC_EffectKindInfo g_clEffectKindInfo;
    
private:
    // --- �p������ ---

    // �p���غc�禡�A�T�O��ҼҦ�
    CEffectManager();
    CEffectManager(const CEffectManager&) = delete;
    CEffectManager& operator=(const CEffectManager&) = delete;
    static CEffectManager* s_pInstance;

    // �S���쵲��C���Y�����ЩM�p�ƾ�
    EffectInfo* m_pHead;           // �첾 +0
    EffectInfo* m_pTail;           // �첾 +4
    unsigned int     m_uEffectCount;    // �첾 +8

    // �S���B�@�������ܼƪ��S�Ī���
    CEffect_OverMind_Screen m_OverMindScreenEffect; // �첾 +12
    CEffect_Skill_Type_Once m_SkillTypeOnceEffect;  // �첾 +32, ���M�γ~�����A���̾ګغc�禡�٭�
};