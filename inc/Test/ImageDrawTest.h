#pragma once

#include <d3d9.h>

// �e�V�ŧi�A�קK�b���Y�ɤ��ޤJ�L�h�����n�����e
class cltImageManager;
class GameImage;

/// @class ImageDrawTest
/// @brief �ʸˤF���J�B�B�z�M��V GameImage ���Ҧ������޿�C
class ImageDrawTest
{
public:
    ImageDrawTest();
    ~ImageDrawTest();

    /// @brief ��l�ƴ������ҡA�]�A�}��VFS�B�إߺ޲z���M���J�Ϥ��귽�C
    /// @return ���\��^ S_OK�A���Ѫ�^ E_FAIL�C
    HRESULT Initialize();

    /// @brief ����C�@�V����V�޿�C
    void Render();

private:
    /// @brief ����Ҧ��귽�C
    void Cleanup();

private:
    // --- �����ܼ� ---
    cltImageManager* m_pImageManager; // �Ϥ�������޲z��
    GameImage* m_pItemImage1;   // �Ĥ@�Ӫ��~�Ϥ�
    GameImage* m_pItemImage2;   // �ĤG�Ӫ��~�Ϥ�
    GameImage* m_pMapBgImage;   // �a�ϭI���Ϥ�
};