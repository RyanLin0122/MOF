#pragma once

#include <cstdint>

class CMofMsg {
public:
  static void (*m_pOnBufferMaxBytesFuncPtr)(unsigned int, unsigned int);
  static void (*m_pOnBufferFulledFuncPtr)(unsigned int, unsigned int);
  static unsigned int m_dwMaxUsageSize_log;

  static void InitializeStaticVariable(void (__cdecl *a1)(unsigned int, unsigned int), void (__cdecl *a2)(unsigned int, unsigned int));

  CMofMsg();
  ~CMofMsg();
  CMofMsg(unsigned __int16 a2, int a3);
  CMofMsg(unsigned __int8 *a2, unsigned int a3);

  void Put(char *a2, int a3);
  void Put_Z1(char *a2);
  void Put_Z2(char *a2);
  void Put_Z4(char *a2);
  void Put_LONG(int a2);
  void Put_DWORD(unsigned int a2);
  void Put_SHORT(__int16 a2);
  void Put_WORD(unsigned __int16 a2);
  void Put_BYTE(unsigned __int8 a2);
  void Put_INT64(char a2, int a3);

  int Get(unsigned __int8 *a2, int a3);
  int Get_Z1(char *a2, int a3, int a4, char *a5);
  int Get_Z2(char *a2, int a3, int a4, char *a5);
  int Get_Z4(char *a2, int a3, int a4, char *a5);
  int Get_BYTE(unsigned __int8 *a2);
  int Get_SHORT(__int16 *a2);
  int Get_WORD(unsigned __int16 *a2);
  int Get_LONG(int *a2);
  int Get_DWORD(unsigned int *a2);
  int Get_INT64(__int64 *a2);

  unsigned __int8 *GetMsg();
  unsigned __int8 *GetCurrentPos();
  void SetCurrentPos(unsigned __int8 *a2);
  void InitCurrentPos();
  void ResetSize();
  int GetLength();
  int GetLeftUsageSize();
  unsigned long *operator=(unsigned long *a2);

  unsigned int EnCodeCheckSum(unsigned int a2, unsigned int a3);
  unsigned int DeCodeCheckSum(unsigned int a2, unsigned int a3);
  void AddMsg(CMofMsg *a2);
  int IsPacketOnServer() const;
  int VerifyStringValidity(char *a2);
  int IsIncludeViolationChars(char *a2, char *a3);

private:
  int m_bPacketOnServer;
  unsigned __int8 m_msg[0xFFFF];
  unsigned __int8 *m_pCurrentPos;
  unsigned int m_dwUsageSize;
  unsigned __int8 *m_pMsg;
};

class CMoFMsgMgr {
public:
  CMoFMsgMgr();
  ~CMoFMsgMgr();

  int AddMsg(unsigned __int8 *a2, unsigned __int16 a3);
  int GetMsg(unsigned int a2, unsigned __int8 *const a3);
  unsigned int GetMsgNum() const;
  void ClearMsg();

private:
  struct MsgInfo {
    unsigned __int8 *first;
    unsigned __int8 *second;
    unsigned __int16 firstLen;
    unsigned __int16 secondLen;
  };

  unsigned __int8 m_buffer[0x100000];
  unsigned __int8 *m_pReadPos;
  unsigned __int8 *m_pWritePos;
  unsigned int m_dwUsageSize;
  MsgInfo m_infos[0x2710];
  unsigned int m_dwMsgNum;
};

