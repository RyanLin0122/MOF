#pragma once

// ���F�� cltImageManager ����sĶ�A�ڭ̻ݭn GameImage ���w�q�C
// �o�̧ڭ̥]�t�@�Ӯھ� GameImage.c ���_�X�� GameImage.h ���Y�ɡC
#include "Image/GameImage.h" 
#include "Image/ResourceMgr.h"

/// @class cltImageManager
/// @brief GameImage ������޲z���C
///
/// �t�d�޲z�@�өT�w�j�p�� GameImage ����}�C�A
/// ��������M���� GameImage ���󪺤����A�H�F�쭫�ƧQ�ΡB���ɮį઺�ت��C
class cltImageManager {
public:
    static cltImageManager* GetInstance();

    /// @brief �Ѻc�禡�C
    /// �|�۰ʩI�s�}�C���Ҧ� GameImage ���󪺸Ѻc�禡�C
    ~cltImageManager();

    /// @brief ��l�ƺ޲z���C
    /// ���������C�@�� GameImage ����w���إ߳��I�w�İϡC
    void Initialize();

    /// @brief ����Ҧ��귽�C
    /// ���]�����Ҧ��� GameImage ����C
    void Free();

    /// @brief �q��������@�ӥi�Ϊ� GameImage ����C
    /// @param dwGroupID     �귽���s�� ID (���� a2)�C
    /// @param dwResourceID  �귽���ߤ@ ID (���� a3)�C
    /// @param a4            �ǻ����귽�޲z�����ѼơC
    /// @param a5            �ǻ����귽�޲z�����ѼơC
    /// @return ���V�@�Ӥw�]�w�n�귽�� GameImage ���󪺫��СA�p�G���w���h��^ nullptr�C
    GameImage* GetGameImage(unsigned int dwGroupID, unsigned int dwResourceID, int a4 = 0, int a5 = 0);

    /// @brief �N�@�� GameImage �����k�٨�����C
    /// @param pImage �n���� GameImage ������СC
    void ReleaseGameImage(GameImage* pImage);

    /// @brief ����Ҧ����b�ϥΪ� GameImage ����C
    void ReleaseAllGameImage();

    /// @brief ��s�Ҧ����b�ϥΪ� GameImage ���󪺳��I��ơC
    void ProcessAllGameImage();

    // ��������j�p
    static const int MAX_IMAGES = 5000;

    // GameImage ������}�C
    GameImage m_Images[MAX_IMAGES];
private:
    cltImageManager();
    static cltImageManager* s_pInstance;
};