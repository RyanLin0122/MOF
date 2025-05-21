#ifndef CMOFPACKING_H
#define CMOFPACKING_H

#include "nfs.h"    // �]�t NfsHandle, NfsGlobResults, �H�ΩҦ� nfs_* API ���
#include <windows.h> // ���F WIN32_FIND_DATAA, HANDLE �� (�w�� nfs.h �����]�t)
#include <string.h>  // ���F memset, _strlwr (�Ψϥ� <cstring>)
#include <stdio.h>   // ���F _sprintf (�Ψϥ� <cstdio>)
#include <cstring>   // C++������ string.h
#include <cstdio>    // C++������ stdio.h


// CMofPacking ���O�w�q
class CMofPacking {
public:
	CMofPacking();
	virtual ~CMofPacking(); // �����Ѻc�禡�A�]���ϽsĶ�X��ܤF scalar deleting destructor

	bool Init(); // ��l�ƨ��
	bool PackFileOpen(const char* packFileName); // �}��NFS�ʸ��ɮ�
	bool PackFileClose(); // ����NFS�ʸ��ɮ�

	bool AddFile(const char* filePathInPack); // �s�W�ɮצܫʸ� (�Y�w�s�b�h�������A�إ�)
	bool RemoveFile(const char* filePathInPack); // �q�ʸˤ������ɮ�

	// DataPacking ��^�ȡG
	// 0: FindFirstFileA ���� (�ؿ����s�b�ά���)
	// 1: ��� "mof.ini"�A�B�z�פ�
	// 2: �ؿ����`�B�z����
	int DataPacking(const char* directoryPath); // �N���w�ؿ��U���Ҧ��ɮץ��]�iNFS

	void FileReadBackGroundLoading(const char* filePathInPack); // �I�����J��Ū���ɮצܤ����w�İ� m_backgroundLoadBuffer
	char* FileRead(const char* filePathInPack); // Ū���ɮצܰʺA���t�������w�İ� m_pReadBuffer

	void DeleteBuffer();  // ���� m_pReadBuffer
	void DeleteBuffer1(); // ���� m_pBuffer1 (���w�İϦb���Ѫ�C�X�����G���Q�Ω���J)

	int GetBufferSize() const; // ���o�W��Ū���ާ@���줸�ռ� (m_nReadBytes)

	NfsGlobResults* SearchString(const char* pattern); // �bNFS���j�M�ŦX�Ҧ����ɮצW��
	void DeleteSearchData(); // ���� SearchString �����G

	char* ChangeString(char* str); // �N�r���ର�p�g (���U���)

	// m_backgroundLoadBufferField (�����q292�A�j�p0x7A1200)
	// �Ω� FileReadBackGroundLoading ���T�w�j�p�w�İϡC
	// �ھ� `memset((char *)this + 292, 0, 0x7A1200u);`�A�����H��e����������C
	// �e�z���� (vtable, ptrs, char[256], int, NfsGlobResults) �`�j�p��n�� 292 �줸�� (���]32�줸���ЩMsize_t)�C
	char  m_backgroundLoadBufferField[0x7A1200];
private:
	// �����ܼƪ����ǩM��������ϽsĶC�X�����R
	// ��ڰO����G���ѽsĶ���M�w�A���o�̪����ǰѦҤF��l�ƩMmemset�ާ@������

	NfsHandle* m_pNfsHandle;          // ���VNFS�ɮרt�α���N�X������ (���� this+1 * sizeof(DWORD))
	void* m_pReadBuffer;         // ��FileRead���t�A�Ω��x�s�����ɮפ��e (���� this+2 * sizeof(DWORD))
	void* m_pBuffer1;            // �t�@�ӽw�İϫ��СADeleteBuffer1�|���� (���� this+3 * sizeof(DWORD))

	// m_tempPathBuffer (�����q16�A�j�p256�줸��)
	// �bCMofPacking::CMofPacking���� memset((char *)this + 16, 0, 0x100u) ��l�ơC
	// �bDataPacking���A���a�ܼ�FileName, Buffer, v13���j�p����256�A�������ܼƥi��Ω������ت��A�ά����ϥΡC
	// �bAddFile���A���a�ܼ�v5�j�p��2048�C
	// ���B�O�d�@��256�줸�ժ������A�p�ϽsĶ�X�ҥܡC
	char       m_tempPathBuffer[256];

	int            m_nReadBytes;      // �O���W��Ū���ާ@���줸�ռ� (���� this+68 * sizeof(DWORD))
	NfsGlobResults m_globResults;     // �x�snfs_glob���j�M���G (�䭺�a�}���� this+276)
	// m_globResults.internal_callback_error_flag ���� this+72 * sizeof(DWORD)

// m_isLoadingFlag ���� C �X���� byte_7A1324[(_DWORD)this]
// �o�O�@�Ӱ�������w�����аO�A���ܬO�_���ɮץ��b�QŪ���C
// ��b�O���餤���T����m�۹���L�����A�qC�X���H����������C++���зǧG�����A
// �S�O�O�Ҽ{�� m_backgroundLoadBufferField �� memset �q this+292 �}�l�C
// �o�̱N���n�����@��bool�����AC++�sĶ���|������t��m�C
	bool           m_isLoadingFlag;
};

#endif // CMOFPACKING_H