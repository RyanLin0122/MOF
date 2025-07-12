#pragma once

#include "cltBaseResource.h" // �]�t�����O���w�q

// �e�V�ŧi�A�קK�����n�� include
struct ImageResourceListData;
class CDeviceResetManager;

/// @class cltGIResource
/// @brief GI �Ϥ��귽�޲z���C
///
/// �~�Ӧ� cltBaseResource�A�M���t�d���J�B�޲z�M���� GI �榡���Ϥ��귽�C
class cltGIResource : public cltBaseResource {
public:
    cltGIResource();
    virtual ~cltGIResource();

    /// @brief ��l�� GI �귽�޲z���C
    /// @param pPath �s�� GI �귽����¦���|�C
    /// @param timeout �귽�۰ʦ^�����O�ɮɶ��C
    void Initialize(const char* pPath, unsigned int timeout);

protected:
    /// @brief �q�W���ɮ׸��J GI �귽�C
    /// @override
    virtual ImageResourceListData* LoadResource(unsigned int id, int a3, unsigned char a4) override;

    /// @brief �q�ʸ��� (pack file) ���J GI �귽�C
    /// @override
    virtual ImageResourceListData* LoadResourceInPack(unsigned int id, int a3, unsigned char a4) override;

    /// @brief ����@�Ӥw���J�� GI �귽�C
    /// @override
    virtual void FreeResource(void* pResourceData) override;

private:
    char m_szBasePath[260];          // �귽����¦�j�M���| (�첾 +24)
    char m_szNationPath[260];        // �ϰ�Ƹ귽����Ƹ��| (�첾 +284)
    bool m_bNationPathSearched;      // �O�_�w�g�j�M�L�ϰ�Ƹ��|���X�� (�첾 +544, dword at +136)
};