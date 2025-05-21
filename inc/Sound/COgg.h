#ifndef COGG_H
#define COGG_H

#include <fmod/fmod.h>      // FMOD ���Įw API
#include <windows.h>   // FSOUND_SetHWND �ݭn HWND

class COgg {
public:
    COgg();                          // �غc�禡
    virtual ~COgg();                 // �����Ѻc�禡

    // ��l�� COgg �t�ΡA�]�w FMOD �÷ǳƼ���Ѽ�
    // a2: �@�Ӿ�ưѼơA�Ω�M�w��y���`���Ҧ� (0: ���`��, �D0: �`��)
    void Initalize(int loopEnabled);

    // ������w�� Ogg Vorbis �ɮ�
    // filePath: Ogg �ɮת����|�ΦW�� (�p�G dword_829254 �� true�A�h���ʸˤ����ɮ׸��|)
    void Play(const char* filePath);

    // ����ثe���񪺦�y
    void Stop();

private:
    // �}�ҭ��T��y�A�ھ� dword_829254 �M�w�O�q�ɮרt�Ϊ����}���٬O�q CMofPacking ���J
    // filePath: Ogg �ɮת����|�ΦW��
    void OpenStreem(const char* filePath);

    // �����ܼ� (�ھ� cogg.c �����O����첾���_)
    FSOUND_STREAM* m_pStream;        // ���V FMOD ���T��y������ (���� this+1)
    int            m_nVolume;          // ���q�j�p (0-255) (���� this+2)
    int            m_nChannelId;       // FMOD �Ω󼽩񦹦�y���n�D ID (���� this+3)
    unsigned int   m_nStreamOpenMode;  // �}�� FMOD ��y�ɨϥΪ��Ҧ��X�� (���� this+4)
};

#endif // COGG_H