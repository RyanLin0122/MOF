#pragma once

// �e�V�ŧi
class ClientCharacter;
class GameImage;

/**
 * @class CEffect_OverMind_Screen
 * @brief �b�����Y����ܤ@�ӯS�����ù�/UI�h�ŮĪG�C
 *
 * �o�����O���~�� CEffectBase�A�ӬO�@�� CEffectManager ���@�Ӧ����A
 * �Ω�B�z�@�ǯS���B�`�n����ı�ĪG�C
 */
class CEffect_OverMind_Screen {
public:
    CEffect_OverMind_Screen();
    ~CEffect_OverMind_Screen();

    /// @brief �ҥΨó]�w�S�Ī��ؼСC
    /// @param pTarget �n���[�S�Ī��ؼШ���C
    void SetActive(ClientCharacter* pTarget);

    /// @brief �ǳ�ø�s�ƾڡA��s�S�Ī��A�C
    void PrepareDrawing();

    /// @brief ø�s�S�ġC
    void Draw();

    /// @brief ���ߨ禡 (�b��l�X���Q�I�s�A���L��@)�C
    void Poll() {}

private:
    // --- �����ܼ� (�ھ� Effectall.c @ 0x00539F80 ���_) ---

    ClientCharacter* m_pTargetCharacter; // �첾 +0
    bool             m_bIsActive;        // �첾 +4
    // ��l�X���첾 +8 �B���@�ӥ��ϥΪ� DWORD
    int              m_dwAlpha;          // �첾 +12: �@���ͩR�g���p�ɾ��M�z����
    GameImage* m_pEffectImage;     // �첾 +16: ���V�nø�s�� GameImage
};