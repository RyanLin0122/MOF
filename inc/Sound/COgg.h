#ifndef COGG_H
#define COGG_H

#include <windows.h> // For HWND
#include <fmod/fmod.h>

// FSOUND �`�� (�ھ� cogg.c �����ȱ��_)
//const unsigned int FSOUND_LOADMEMORY = 0x4000; // 16384
//const unsigned int FSOUND_LOADRAW = 0x8000; // 32768

// �w�]�����n�D (�� FMOD ��ܤ@�ӪŶ��n�D)
const int FSOUND_FREE_CHANNEL = -1; // �q�` FMOD �ϥ� -1 �N����N�Ŷ��n�D

// --- ���� FSOUND ���� ---

// �]�t CMofPacking �����Y��
#include "CMOFPacking.h" // ���]���ɮצb�]�t���|��

// COgg �ϥΪ������ܼ� (�ݦb��L�a��w�q)
extern HWND hWnd;         // ��������N�X�A�Ω� FSOUND_SetHWND
extern int dword_829254;  // �����ɮ׸��J�覡�M FSOUND_Stream_Open �X�Ъ������ܼ�
extern CMofPacking g_clMofPacking; // CMofPacking ��������

/*
 * ���� dword_C24CF4:
 * �b cogg.c �� OpenStreem �禡���A�ϥΤF &dword_C24CF4 �@�� qmemcpy ���ӷ���}�C
 * �o�t�� dword_C24CF4 �O�@�ӥ����ƽw�İϪ��Ÿ��ACMofPacking::FileReadBackGroundLoading �t�d��J���w�İϡC
 * CMofPacking::GetBufferSize() �h��^���w�İϤ����ĸ�ƪ��j�p�C
 * ���F�� COgg.cpp ����B�@�A�ڭ̰��] CMofPacking ���ѤF�@�Ӥ�k�Ӧs���o�ӽw�İϪ���ơC
 * �b�U������@���A�ڭ̷|���] g_clMofPacking.GetBackgroundBufferData() �i�H���o���w�İϫ��СA
 * �åB g_clMofPacking.GetBufferSize() ��^���O FileReadBackGroundLoading ���J�������ɮפj�p�C
 * �`�N�G�z���Ѫ� CMOFPacking.h/cpp �i��ݭn�վ�~�৹���ŦX cogg.c ���o�إΪk�C
 */

class COgg
{
public:
    COgg();
    virtual ~COgg(); // �����Ѻc�禡�A�]���ϽsĶ�X��ܤF scalar deleting destructor �M vftable

    // ��l�� FSOUND �t�ΩM COgg ��Ҫ��]�w
    // a2: ���G�O�@�Ӱj��X�� (0 �N���j��, �D0 �N��j��)
    void Initalize(int a2);

    // ���� Ogg �ɮ�
    // filePath: Ogg �ɮת����|�Φb�ʸ��ɤ����W��
    void Play(const char* filePath);

    // ����ثe���� Ogg ���T�y
    void Stop();

private:
    // �����禡�A�Ω�}�ҭ��T�y (�i��q�ɮשΰO����)
    void OpenStreem(const char* filePath);

    // �����ܼ� (�ھ� cogg.c ���� this ���Ц첾���_)
    // this+0: vfptr (�� C++ �����禡����B�z)

    // this+1 * sizeof(DWORD) -> m_streamHandle
    // FSOUND ���T�y����N�X
    FSOUND_STREAM* m_streamHandle;

    // this+2 * sizeof(DWORD) -> m_volume
    // ���q (0-255)
    int m_volume;

    // this+3 * sizeof(DWORD) -> m_channelId
    // FSOUND �����n�D ID
    // �b cogg.c �������T��l�ơA���B�����@�ӱ`�ιw�]�� FSOUND_FREE_CHANNEL
    int m_channelId;

    // this+4 * sizeof(DWORD) -> m_streamOpenFlags
    // �Ω� FSOUND_Stream_Open ���X�вզX
    unsigned int m_streamOpenFlags;

    // �`�N�Gcogg.c ���� OpenStreem �禡�ϥ� _malloc �t�m���O���� v5 ���Q���T����C
    // �o�N���� FSOUND_Stream_Open �n��ƻs�F�o���O����A�n�����I�s�̺޲z�o���O���骺�ͩR�g���A
    // �ӭ�l�X��ܤF�����񥦡]�i��ɭP�O���鬪�|�A���D FMOD �b Stream_Close �ɳB�z�^�C
    // ���B���٭�N�|��`��l�X���欰�C
    void* m_pStreamDataCopy; // �Ω��x�s OpenStreem �� _malloc ���O������СA�H�K��������
};

#endif // COGG_H