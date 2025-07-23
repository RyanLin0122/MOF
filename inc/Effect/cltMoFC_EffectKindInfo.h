#pragma once

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cctype> // for toupper

// �e�V�ŧi
class cltTextFileManager; // ���]���ɮ׺޲z���O

/**
 * @enum E_EFFECT_TYPE
 * @brief �w�q�S�Ī��欰�����A�q Initialize �禡���޿���_�C
 */
enum E_EFFECT_TYPE : unsigned char {
    EFFECT_TYPE_UNKNOWN = 0,
    EFFECT_TYPE_ONCE = 1, // �b�I�k�̦�m����@��
    EFFECT_TYPE_DIRECTED = 2, // �b�ؼЦ�m����@��
    EFFECT_TYPE_SUSTAIN = 3, // �b�I�k�̨��W���򼽩�
    EFFECT_TYPE_SHOOT_UNIT = 4, // �@����g������
    EFFECT_TYPE_SHOOT_NO_EA = 5, // ��g���A�����ϥ� .ea �ɮ� (�Ҧp�l�u)
    EFFECT_TYPE_ITEM_ONCE = 6  // ���~�ϥήɪ��@���ʯS��
};

/**
 * @struct stEffectKindInfo
 * @brief (�j�p: 132 bytes) �x�s��@�S�ĺ���������w�q�C
 */
struct stEffectKindInfo {
    /// @brief (�첾 +0) �S�Ī��ߤ@�Ʀr ID�C
    unsigned short usKindID;

    /// @brief (�첾 +2) �S�ĸ귽�� (.ea) ���W�١C
    char szFileName[128];

    /// @brief (�첾 +130) �S�Ī��欰�����C
    E_EFFECT_TYPE ucType;

    /// @brief (�첾 +131) ���F�O����������R�줸�աC
    char padding;
};

/**
 * @class cltMoFC_EffectKindInfo
 * @brief �޲z�Ҧ��S�ĺ����w�q����Ʈw�C
 * * �@���@�ӳ�ҩΥ��쪫��A�����J�ô��ѹ� stEffectKindInfo ���c���ֳt�d�ߡC
 */
class cltMoFC_EffectKindInfo {
public:
    cltMoFC_EffectKindInfo();
    ~cltMoFC_EffectKindInfo();

    /// @brief �q���w����r�ɮ׸��J�Ҧ��S�ĩw�q�C
    /// @param szFileName �]�t�S�ĩw�q���ɮ׸��|�C
    /// @return ���\��^ 1�A���Ѫ�^ 0�C
    int Initialize(char* szFileName);

    /// @brief �ھڼƦr ID ����S�ĩw�q�C
    /// @param kindID �S�Ī� short ���� ID�C
    /// @return ���V stEffectKindInfo �����СA�Y���s�b�h�� nullptr�C
    stEffectKindInfo* GetEffectKindInfo(unsigned short kindID);

    /// @brief �ھڦr�� ID ����S�ĩw�q�C
    /// @param szKindCode �榡�� "A0001" ���r�� ID�C
    /// @return ���V stEffectKindInfo �����СA�Y���s�b�h�� nullptr�C
    stEffectKindInfo* GetEffectKindInfo(char* szKindCode);

    /// @brief �N�r�� ID �ഫ�� short �������Ʀr���ޡC
    /// @param szKindCode �榡�� "A0001" ���r�� ID�C
    /// @return ������ short ���ޡC
    unsigned short TranslateKindCode(char* szKindCode);

private:
    // �����禡����СA�ѽsĶ���޲z
    // void* m_pVftable; // �첾 +0

    // ���а}�C�A�Ω�ֳt�d�ߡC�}�C�j�p 65535 (0xFFF F)�C
    stEffectKindInfo* m_pEffectInfo[65535]; // �첾 +4
};