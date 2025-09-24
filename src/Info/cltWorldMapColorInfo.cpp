#include "Info/cltWorldMapColorInfo.h"
#include "FileSystem/CMOFPacking.h"  // �ϥγ�һP�I�����J�w��
#include <cstdio>
#include <cstring>
#include <new>

// ----- (00503320) --------------------------------------------------------
cltWorldMapColorInfo::cltWorldMapColorInfo()
    : m_pData(nullptr) {}

// �H operator delete ��������A�קK��|
cltWorldMapColorInfo::~cltWorldMapColorInfo() {
    Free();
}

// ----- (00503340) --------------------------------------------------------
// InitializeInPack: �z�L�ʥ]�t��Ū�J�A�ýƻs 0x210D3 bytes �줺���w��
int cltWorldMapColorInfo::InitializeInPack(char* pathInPack) {
    // �ϽsĶ�X�ϥΥ��a 256 bytes �w�ī�A ChangeString
    char localPath[256];
    std::strcpy(localPath, pathInPack);

    CMofPacking* pack = CMofPacking::GetInstance();
    if (!pack) return 0;

    char* changed = pack->ChangeString(localPath);
    pack->FileReadBackGroundLoading(changed);

    // ��X�H &dword_C24CF4 �P�_�F���B�������Ҥ����I���w�İ�
    const uint8_t* src = reinterpret_cast<const uint8_t*>(pack->m_backgroundLoadBufferField);
    if (!src) return 0;

    // �t�m�P�ϽsĶ�@�P�j�p�]operator new / operator delete �t��^
    void* p = ::operator new(kSize, std::nothrow);
    if (!p) return 0;
    m_pData = static_cast<uint8_t*>(p);

    // �ϽsĶ���� qmemcpy 0x210D0�A�A�� 2 bytes �� WORD �P 1 byte�G
    //   v5[67688] �P *((_BYTE*)v6 + 2) �����첾 0x210D0�B0x210D1�B0x210D2
    std::memcpy(m_pData, src, 0x210D0);
    m_pData[0x210D0] = src[0x210D0];
    m_pData[0x210D1] = src[0x210D1];
    m_pData[0x210D2] = src[0x210D2];

    return 1;
}

// ----- (005033E0) --------------------------------------------------------
// Initialize: �����q�ɮ�Ū�J 0x210D3 bytes
int cltWorldMapColorInfo::Initialize(const char* fileName) {
    int ok = 0;
    std::FILE* fp = std::fopen(fileName, "rb");
    if (fp) {
        void* p = ::operator new(kSize, std::nothrow);
        if (p) {
            m_pData = static_cast<uint8_t*>(p);
            if (std::fread(m_pData, kSize, 1u, fp) == 1u) {
                ok = 1;
            }
            else {
                ::operator delete(m_pData);
                m_pData = nullptr;
            }
        }
        std::fclose(fp);
    }
    return ok;
}

// ----- (00503440) --------------------------------------------------------
void cltWorldMapColorInfo::Free() {
    if (m_pData) {
        ::operator delete(m_pData);
        m_pData = nullptr;
    }
}

// ----- (00503460) --------------------------------------------------------
// GetColor: �Y�w���J�B�y�Цb [0,409)��[0,331) ���A�^�Ǧ�� base + (409*y + x) �� uint8 �ȡC
int cltWorldMapColorInfo::GetColor(int x, int y) const {
    if (m_pData && x >= 0 && y >= 0 && x < kWidth && y < kHeight) {
        // �ϽsĶ�첾�G*(base + a2 + 408*a3 + a3) == base + (409*y + x)
        size_t offset = static_cast<size_t>(y) * static_cast<size_t>(kWidth) + static_cast<size_t>(x);
        return static_cast<int>(m_pData[offset]);
    }
    return 0;
}
