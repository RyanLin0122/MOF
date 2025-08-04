#pragma once

#include "Effect/CBulletBase.h"

// �e�V�ŧi
class GameImage;

/**
 * @class CBulletArrow
 * @brief �N��@�䪽�u����óv���H�X���b�ڡC
 *
 * �~�Ӧ� CBulletBase�A�O�@�ӥѵ{���X����ʵe��²���g���C
 */
class CBulletArrow : public CBulletBase {
public:
    CBulletArrow();
    virtual ~CBulletArrow();

    // --- �����禡�мg ---

    /// @brief �Ыبó]�w�b�ڪ���l���A�C
    virtual void Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed) override;

    /// @brief ��s�b�ڪ���m�M�z���סC
    /// @return �p�G�b�ڥͩR�g�������A�h�^�� true�C
    virtual bool Process(float fElapsedTime) override;

    /// @brief ø�s�b�ڡC
    virtual void Draw() override;

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x0052D570 ���_) ---
    int m_dwAlpha;          // �첾 +28: �@���ͩR�g���p�ɾ��M�z����
    GameImage* m_pArrowImage;  // �첾 +32: ���V�b�ڪ� GameImage
};