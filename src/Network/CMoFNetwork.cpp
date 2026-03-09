#include "Network/CMoFNetwork.h"

CMoFNetwork *__thiscall CMoFNetwork::CMoFNetwork(CMoFNetwork *this)
{
  DCTNetwork::DCTNetwork(this);
  CMoFMsgMgr::CMoFMsgMgr((CMoFNetwork *)((char *)this + 60276));
  *(_DWORD *)this = &CMoFNetwork::`vftable';
  return this;
}
// 69B450: using guessed type void *CMoFNetwork::`vftable';

//----- (0054C290) --------------------------------------------------------
CMoFNetwork *__thiscall CMoFNetwork::`vector deleting destructor'(CMoFNetwork *this, char a2)
{
  CMoFNetwork::~CMoFNetwork(this);
  if ( (a2 & 1) != 0 )
    operator delete(this);
  return this;
}

//----- (0054C2B0) --------------------------------------------------------
void __thiscall CMoFNetwork::~CMoFNetwork(CMoFNetwork *this)
{
  *(_DWORD *)this = &CMoFNetwork::`vftable';
  CMoFMsgMgr::~CMoFMsgMgr((CMoFNetwork *)((char *)this + 60276));
  DCTNetwork::~DCTNetwork(this);
}
// 69B450: using guessed type void *CMoFNetwork::`vftable';

//----- (0054C2D0) --------------------------------------------------------
void __thiscall CMoFNetwork::Initialize(CMoFNetwork *this, void (__cdecl *a2)(unsigned __int8 *, unsigned __int16, unsigned int), unsigned int a3)
{
  *((_DWORD *)this + 307217) = a2;
  *((_DWORD *)this + 307218) = a3;
  DCTNetwork::Initialize(this);
}

//----- (0054C2F0) --------------------------------------------------------
void __thiscall CMoFNetwork::Poll(CMoFNetwork *this)
{
  unsigned int v2; // ebp
  unsigned int i; // esi
  int v4; // eax
  unsigned __int8 v5[65536]; // [esp+10h] [ebp-10000h] BYREF

  DCTNetwork::Poll(this);
  v2 = CMoFMsgMgr::GetMsgNum((CMoFNetwork *)((char *)this + 60276));
  for ( i = 0; i < v2; ++i )
  {
    v4 = CMoFMsgMgr::GetMsg((CMoFNetwork *)((char *)this + 60276), i, v5);
    if ( (_WORD)v4 )
      (*((void (__cdecl **)(unsigned __int8 *, int, _DWORD))this + 307217))(v5, v4, *((_DWORD *)this + 307218));
  }
  CMoFMsgMgr::ClearMsg((CMoFNetwork *)((char *)this + 60276));
}

//----- (0054C360) --------------------------------------------------------
void __thiscall CMoFNetwork::OnRecv(CMoFNetwork *this)
{
  _DWORD *v2; // ebx
  int v3; // edi
  size_t v4; // edx

  if ( *((_DWORD *)this + 15067) > 4u )
  {
    v2 = (_DWORD *)((char *)this + 268);
    do
    {
      v3 = (unsigned __int16)*v2;
      if ( *((_DWORD *)this + 15067) - 4 < v3 - 4 )
        break;
      CMoFMsgMgr::AddMsg((CMoFNetwork *)((char *)this + 60276), (unsigned __int8 *)this + 272, v3 - 4);
      v4 = *((_DWORD *)this + 15067) - v3;
      *((_DWORD *)this + 15067) = v4;
      _memmove((char *)this + 268, (char *)this + v3 + 268, v4);
    }
    while ( *((_DWORD *)this + 15067) > 4u );
  }
}

//----- (0054C3E0) --------------------------------------------------------
int __thiscall CMoFNetwork::SetSendData(CMoFNetwork *this, char *a2, unsigned __int16 a3)
{
  char v5[1024]; // [esp+Ch] [ebp-800h] BYREF
  char v6[1024]; // [esp+40Ch] [ebp-400h] BYREF

  memset(v5, 0, sizeof(v5));
  memset(v6, 0, sizeof(v6));
  *(_WORD *)v5 = a3 + 4;
  qmemcpy(&v5[4], a2, a3);
  CCrypt::Encode((CCrypt *)&g_Crypt, *(unsigned __int16 *)v5, v5, v6);
  return DCTNetwork::SetSendData(this, v6, *(unsigned __int16 *)v5);
}
// 7C36A0: using guessed type char g_Crypt;

//----- (0054C470) --------------------------------------------------------
int __thiscall CMoFNetwork::Login(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 1u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054C510) --------------------------------------------------------
int __thiscall CMoFNetwork::Move(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned __int8 a4, unsigned __int16 a5)
{
  char *v7; // eax
  int v8; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  if ( g_dwSubGameState != 4 )
    return 1;
  CMofMsg::CMofMsg((CMofMsg *)v10, 0x1001u, 0);
  v11 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v10, a2);
  CMofMsg::Put_WORD((CMofMsg *)v10, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v10, a4);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v7 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v8 = CMoFNetwork::SetSendData(this, v7, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v8;
}
// B3D6FC: using guessed type unsigned int g_dwSubGameState;

//----- (0054C5E0) --------------------------------------------------------
int __thiscall CMoFNetwork::Chatting(CMoFNetwork *this, char *a2, unsigned __int8 a3, unsigned __int16 a4)
{
  char *v5; // eax
  int result; // eax
  char *v7; // eax
  int v8; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x1002u, 0);
  v11 = 0;
  v5 = CMoFNetwork::CheckChatMsg(this, a2);
  if ( v5 )
  {
    CMofMsg::Put_Z2((CMofMsg *)v10, v5);
    CMofMsg::Put_BYTE((CMofMsg *)v10, a3);
    CMofMsg::Put_WORD((CMofMsg *)v10, a4);
    v9 = CMofMsg::GetLength((CMofMsg *)v10);
    v7 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
    v8 = CMoFNetwork::SetSendData(this, v7, v9);
    v11 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v10);
    result = v8;
  }
  else
  {
    v11 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v10);
    result = 0;
  }
  return result;
}

//----- (0054C6C0) --------------------------------------------------------
int __thiscall CMoFNetwork::PartyChatting(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int result; // eax
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x1010u, 0);
  v9 = 0;
  v3 = CMoFNetwork::CheckChatMsg(this, a2);
  if ( v3 )
  {
    CMofMsg::Put_Z2((CMofMsg *)v8, v3);
    v7 = CMofMsg::GetLength((CMofMsg *)v8);
    v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
    v6 = CMoFNetwork::SetSendData(this, v5, v7);
    v9 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v8);
    result = v6;
  }
  else
  {
    v9 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v8);
    result = 0;
  }
  return result;
}

//----- (0054C780) --------------------------------------------------------
int __thiscall CMoFNetwork::Whisper(CMoFNetwork *this, char *a2, char *a3)
{
  char *v4; // edi
  int result; // eax
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10028h]
  char v9[65552]; // [esp+8h] [ebp-1001Ch] BYREF
  int v10; // [esp+10020h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x51u, 0);
  v10 = 0;
  v4 = CMoFNetwork::CheckChatMsg(this, a3);
  if ( v4 )
  {
    CMofMsg::Put_Z1((CMofMsg *)v9, a2);
    CMofMsg::Put_Z2((CMofMsg *)v9, v4);
    v8 = CMofMsg::GetLength((CMofMsg *)v9);
    v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
    v7 = CMoFNetwork::SetSendData(this, v6, v8);
    v10 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v9);
    result = v7;
  }
  else
  {
    v10 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v9);
    result = 0;
  }
  return result;
}

//----- (0054C850) --------------------------------------------------------
char *__thiscall CMoFNetwork::CheckChatMsg(CMoFNetwork *this, char *a2)
{
  int v2; // esi
  int v3; // edi
  char *v4; // ebx

  v2 = 0;
  v3 = strlen(a2);
  if ( v3 > 0 )
  {
    v4 = a2;
    do
    {
      if ( *v4 == 10 )
      {
        _memmove(v4, &a2[v2 + 1], v3 - v2);
        --v2;
        --v4;
        --v3;
      }
      ++v2;
      ++v4;
    }
    while ( v2 < v3 );
  }
  return (char *)(strlen(a2) != 0 ? (unsigned int)a2 : 0);
}

//----- (0054C8B0) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveMap(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 3u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054C950) --------------------------------------------------------
int __thiscall CMoFNetwork::ATTACK(CMoFNetwork *this, unsigned int a2, unsigned int a3, int a4, unsigned __int8 a5, unsigned int *a6)
{
  struct _EXCEPTION_REGISTRATION_RECORD *v6; // eax
  void *v7; // esp
  int v9; // edi
  char *v11; // eax
  int v12; // esi
  unsigned __int16 v14; // [esp-14h] [ebp-10030h]
  char v15[65552]; // [esp+0h] [ebp-1001Ch] BYREF
  struct _EXCEPTION_REGISTRATION_RECORD *v16; // [esp+10010h] [ebp-Ch]
  void *v17; // [esp+10014h] [ebp-8h]
  int v18; // [esp+10018h] [ebp-4h]

  v18 = -1;
  v6 = NtCurrentTeb()->NtTib.ExceptionList;
  v17 = &loc_6925FB;
  v16 = v6;
  v7 = alloca(65552);
  CMofMsg::CMofMsg((CMofMsg *)v15, 0x1003u, 0);
  v18 = 0;
  if ( a5 )
  {
    CMofMsg::Put_DWORD((CMofMsg *)v15, a2);
    CMofMsg::Put_DWORD((CMofMsg *)v15, a3);
    CMofMsg::Put_LONG((CMofMsg *)v15, a4);
    CMofMsg::Put_BYTE((CMofMsg *)v15, a5);
    v9 = a5;
    do
    {
      CMofMsg::Put_DWORD((CMofMsg *)v15, *a6++);
      --v9;
    }
    while ( v9 );
  }
  v14 = CMofMsg::GetLength((CMofMsg *)v15);
  v11 = (char *)CMofMsg::GetMsg((CMofMsg *)v15);
  v12 = CMoFNetwork::SetSendData(this, v11, v14);
  v18 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v15);
  return v12;
}
// 54C9E0: conditional instruction was optimized away because of '%arg_C.1!=0'

//----- (0054CA50) --------------------------------------------------------
int __thiscall CMoFNetwork::PickUpItem(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 5u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054CB00) --------------------------------------------------------
int __thiscall CMoFNetwork::DropItem(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xDu, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054CBB0) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xFu, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054CC60) --------------------------------------------------------
int __thiscall CMoFNetwork::UseItem(CMoFNetwork *this, unsigned __int16 a2, char *a3, char *a4, char *a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9, unsigned int a10)
{
  int result; // eax
  char *v12; // eax
  int v13; // esi
  unsigned __int16 v14; // [esp-4h] [ebp-1002Ch]
  char v15[65552]; // [esp+Ch] [ebp-1001Ch] BYREF
  int v16; // [esp+10024h] [ebp-4h]

  if ( g_dwSubGameState == 4 )
  {
    CMofMsg::CMofMsg((CMofMsg *)v15, 7u, 0);
    v16 = 0;
    if ( a3 && a4 )
    {
      CMofMsg::Put_WORD((CMofMsg *)v15, a2);
      CMofMsg::Put_Z1((CMofMsg *)v15, a3);
      CMofMsg::Put_Z1((CMofMsg *)v15, a4);
      CMofMsg::Put_Z2((CMofMsg *)v15, a5);
      CMofMsg::Put_DWORD((CMofMsg *)v15, a6);
      CMofMsg::Put_DWORD((CMofMsg *)v15, a7);
      CMofMsg::Put_DWORD((CMofMsg *)v15, a8);
      CMofMsg::Put_DWORD((CMofMsg *)v15, a9);
      CMofMsg::Put_DWORD((CMofMsg *)v15, a10);
      v14 = CMofMsg::GetLength((CMofMsg *)v15);
      v12 = (char *)CMofMsg::GetMsg((CMofMsg *)v15);
      v13 = CMoFNetwork::SetSendData(this, v12, v14);
      v16 = -1;
      CMofMsg::~CMofMsg((CMofMsg *)v15);
      result = v13;
    }
    else
    {
      v16 = -1;
      CMofMsg::~CMofMsg((CMofMsg *)v15);
      result = 0;
    }
  }
  else
  {
    ClientCharacterManager::SetMyTempHP((ClientCharacterManager *)&g_ClientCharMgr, 100);
    ClientCharacterManager::UseHPPotion((ClientCharacterManager *)&g_ClientCharMgr, 100);
    result = 1;
  }
  return result;
}
// B3D6FC: using guessed type unsigned int g_dwSubGameState;

//----- (0054CDF0) --------------------------------------------------------
int __thiscall CMoFNetwork::BuyItem(CMoFNetwork *this, unsigned __int16 a2, struct cltItemList *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 9u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  cltItemList::FillOutItemListInfo(a3, (struct CMofMsg *)v8);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054CEA0) --------------------------------------------------------
int __thiscall CMoFNetwork::SellItem(CMoFNetwork *this, unsigned __int16 a2, struct cltItemList *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xBu, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  cltItemList::FillOutItemListInfo(a3, (struct CMofMsg *)v8);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054CF50) --------------------------------------------------------
int __thiscall CMoFNetwork::SysCommand(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1004u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054CFF0) --------------------------------------------------------
int __thiscall CMoFNetwork::ApplyAbility(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x1005u, 0);
  v11 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v10, a2);
  CMofMsg::Put_WORD((CMofMsg *)v10, a3);
  CMofMsg::Put_WORD((CMofMsg *)v10, a4);
  CMofMsg::Put_WORD((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (0054D0C0) --------------------------------------------------------
int __thiscall CMoFNetwork::AddToQuickSlot(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, unsigned __int8 a4, unsigned int a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10028h]
  char v10[65552]; // [esp+8h] [ebp-1001Ch] BYREF
  int v11; // [esp+10020h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x1006u, 0);
  v11 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v10, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v10, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v10, a4);
  if ( a3 )
  {
    if ( a3 == 1 || a3 == 2 )
      CMofMsg::Put_WORD((CMofMsg *)v10, a5);
  }
  else
  {
    CMofMsg::Put_BYTE((CMofMsg *)v10, a5);
  }
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (0054D1D0) --------------------------------------------------------
int __thiscall CMoFNetwork::DelFromQuickSlot(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x1007u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054D280) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveOnQuickSlot(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, unsigned __int8 a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x1008u, 0);
  v10 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v9, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (0054D340) --------------------------------------------------------
int __thiscall CMoFNetwork::EquipItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, unsigned __int16 a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x11u, 0);
  v10 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v9, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (0054D400) --------------------------------------------------------
int __thiscall CMoFNetwork::UnEquipItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, unsigned __int16 a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x13u, 0);
  v10 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v9, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (0054D4C0) --------------------------------------------------------
int __thiscall CMoFNetwork::UnEquipAllItem(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x15u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054D560) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveEquipItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, unsigned int a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x17u, 0);
  v10 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v9, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (0054D620) --------------------------------------------------------
int __thiscall CMoFNetwork::UpgradeClass(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x19u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054D6C0) --------------------------------------------------------
int __thiscall CMoFNetwork::UpgradeClassGiftItem(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xEEu, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054D760) --------------------------------------------------------
int __thiscall CMoFNetwork::RunState(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  if ( g_dwSubGameState != 4 )
    return 1;
  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1009u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v5 = CMoFNetwork::SetSendData(this, v4, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v5;
}
// B3D6FC: using guessed type unsigned int g_dwSubGameState;

//----- (0054D810) --------------------------------------------------------
int __thiscall CMoFNetwork::UseSkill(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned int *a4)
{
  int v5; // edi
  char *v7; // eax
  int v8; // esi
  unsigned __int16 v10; // [esp-4h] [ebp-1002Ch]
  char v11[65552]; // [esp+Ch] [ebp-1001Ch] BYREF
  int v12; // [esp+10024h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v11, 0x21u, 0);
  v12 = 0;
  OutputDebugStringA(" User Skill \n");
  CMofMsg::Put_WORD((CMofMsg *)v11, a2);
  if ( a3 )
  {
    CMofMsg::Put_WORD((CMofMsg *)v11, a3);
    v5 = a3;
    do
    {
      CMofMsg::Put_DWORD((CMofMsg *)v11, *a4++);
      --v5;
    }
    while ( v5 );
  }
  v10 = CMofMsg::GetLength((CMofMsg *)v11);
  v7 = (char *)CMofMsg::GetMsg((CMofMsg *)v11);
  v8 = CMoFNetwork::SetSendData(this, v7, v10);
  v12 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v11);
  return v8;
}
// 54D884: conditional instruction was optimized away because of '%arg_4.2!=0'

//----- (0054D8F0) --------------------------------------------------------
int __thiscall CMoFNetwork::LessonFinished(CMoFNetwork *this, unsigned __int8 a2, unsigned int a3, unsigned int a4, unsigned __int8 a5, unsigned __int16 a6, unsigned int a7)
{
  unsigned int v8; // eax
  char *v9; // eax
  int v10; // esi
  unsigned __int16 v12; // [esp-4h] [ebp-1002Ch]
  char v13[65552]; // [esp+Ch] [ebp-1001Ch] BYREF
  int v14; // [esp+10024h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v13, 0x40u, 0);
  v14 = 0;
  v8 = CMofMsg::EnCodeCheckSum((CMofMsg *)v13, a3 + 3815, a7);
  CMofMsg::Put_DWORD((CMofMsg *)v13, v8);
  CMofMsg::Put_BYTE((CMofMsg *)v13, a2);
  CMofMsg::Put_DWORD((CMofMsg *)v13, a3);
  CMofMsg::Put_DWORD((CMofMsg *)v13, a4);
  CMofMsg::Put_BYTE((CMofMsg *)v13, a5);
  CMofMsg::Put_WORD((CMofMsg *)v13, a6);
  CMofMsg::Put_DWORD((CMofMsg *)v13, a7);
  v12 = CMofMsg::GetLength((CMofMsg *)v13);
  v9 = (char *)CMofMsg::GetMsg((CMofMsg *)v13);
  v10 = CMoFNetwork::SetSendData(this, v9, v12);
  v14 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v13);
  return v10;
}

//----- (0054DA00) --------------------------------------------------------
int __thiscall CMoFNetwork::SetSchedule(CMoFNetwork *this, unsigned __int16 a2, unsigned __int8 *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x23u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put((CMofMsg *)v8, (char *)a3, 4);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054DAB0) --------------------------------------------------------
int __thiscall CMoFNetwork::CancelSchedule(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x25u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054DB40) --------------------------------------------------------
int __thiscall CMoFNetwork::BuySkill(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x27u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054DBE0) --------------------------------------------------------
int __thiscall CMoFNetwork::EnterMinigame(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x29u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054DC70) --------------------------------------------------------
int __thiscall CMoFNetwork::LeaveMinigame(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x2Bu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054DD00) --------------------------------------------------------
int __thiscall CMoFNetwork::GetMyChars(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x2Du, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054DDA0) --------------------------------------------------------
int __thiscall CMoFNetwork::Auth(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x30u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054DE40) --------------------------------------------------------
int __thiscall CMoFNetwork::CreateChar(CMoFNetwork *this, unsigned __int8 a2, char *a3, unsigned __int8 a4, unsigned __int16 a5, unsigned __int8 a6, unsigned __int8 a7, unsigned __int8 a8, unsigned __int8 a9, unsigned __int8 a10, unsigned __int8 a11, unsigned __int16 a12, unsigned __int16 a13, unsigned __int16 a14)
{
  char *v15; // eax
  int v16; // esi
  unsigned __int16 v18; // [esp-4h] [ebp-10024h]
  char v19[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v20; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v19, 0x32u, 0);
  v20 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v19, a2);
  CMofMsg::Put_Z1((CMofMsg *)v19, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a4);
  CMofMsg::Put_WORD((CMofMsg *)v19, a5);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a6);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a7);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a8);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a9);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a10);
  CMofMsg::Put_BYTE((CMofMsg *)v19, a11);
  CMofMsg::Put_WORD((CMofMsg *)v19, a12);
  CMofMsg::Put_WORD((CMofMsg *)v19, a13);
  CMofMsg::Put_WORD((CMofMsg *)v19, a14);
  v18 = CMofMsg::GetLength((CMofMsg *)v19);
  v15 = (char *)CMofMsg::GetMsg((CMofMsg *)v19);
  v16 = CMoFNetwork::SetSendData(this, v15, v18);
  v20 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v19);
  return v16;
}

//----- (0054DFB0) --------------------------------------------------------
int __thiscall CMoFNetwork::DeleteChar(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x34u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E050) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestJoinParty(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x36u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E0F0) --------------------------------------------------------
int __thiscall CMoFNetwork::ResponseJoinParty(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x38u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E190) --------------------------------------------------------
int __thiscall CMoFNetwork::LeaveParty(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x3Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054E220) --------------------------------------------------------
int __thiscall CMoFNetwork::KickOutParty(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x3Cu, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E2C0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestTrade(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x42u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E360) --------------------------------------------------------
int __thiscall CMoFNetwork::ResponseTrade(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x44u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E400) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestAddTradeItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x46u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054E4B0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestDeleteTradeItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x48u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054E560) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestChangeTradeMoney(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x4Au, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E600) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestTradeDecision(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x4Cu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054E690) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestTradeCancel(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x4Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054E720) --------------------------------------------------------
int __thiscall CMoFNetwork::AcceptQuest(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x53u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E7C0) --------------------------------------------------------
int __thiscall CMoFNetwork::RewardQuest(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x55u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E860) --------------------------------------------------------
int __thiscall CMoFNetwork::MeetNPC(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x57u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E900) --------------------------------------------------------
int __thiscall CMoFNetwork::GiveupQuest(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x59u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054E9A0) --------------------------------------------------------
int __thiscall CMoFNetwork::EternalGiveupQuest(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xC3u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EA40) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestCharDetailInfo(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x60u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EAE0) --------------------------------------------------------
int __thiscall CMoFNetwork::MyCharDead(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x5Bu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EB80) --------------------------------------------------------
int __thiscall CMoFNetwork::ServerLists(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x5Du, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EC20) --------------------------------------------------------
BOOL __thiscall CMoFNetwork::SendHeartBeat(CMoFNetwork *this)
{
  BOOL result; // eax
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  result = DCTNetwork::IsConnected(this);
  if ( result )
  {
    CMofMsg::CMofMsg((CMofMsg *)v6, 0x1011u, 0);
    v7 = 0;
    v5 = CMofMsg::GetLength((CMofMsg *)v6);
    v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
    v4 = CMoFNetwork::SetSendData(this, v3, v5);
    v7 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v6);
    result = v4;
  }
  return result;
}

//----- (0054ECD0) --------------------------------------------------------
void __cdecl CMoFNetwork::OnSocketClosed()
{
  ShowGameTerminateMessageBox(0, 0x2710u);
}

//----- (0054ECE0) --------------------------------------------------------
int __thiscall CMoFNetwork::SendReturnOrderSheet(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xAAu, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054ED80) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTeleportOrderSheet(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xACu, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_DWORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054EE30) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTeleportDragon(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xAEu, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EED0) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTakeShip(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xB9u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054EF70) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTownPortalOrderSheet(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xBBu, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054F010) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTownPortalReturn(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xBDu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F0A0) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyJoin(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x64u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F130) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyQuit(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x66u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F1C0) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyRegist(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned int a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x68u, 0);
  v10 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v9, a2);
  CMofMsg::Put_WORD((CMofMsg *)v9, a3);
  CMofMsg::Put_DWORD((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (0054F280) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyGetMyList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x70u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F310) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyCancelRegist(CMoFNetwork *this, unsigned int a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x78u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  CMofMsg::Put_DWORD((CMofMsg *)v8, a2);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054F3C0) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyGetSearchList(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x6Du, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054F460) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyBuyItem(CMoFNetwork *this, unsigned int a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x6Au, 0);
  v11 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v10, a3);
  CMofMsg::Put_DWORD((CMofMsg *)v10, a2);
  CMofMsg::Put_WORD((CMofMsg *)v10, a4);
  CMofMsg::Put_WORD((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (0054F530) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyGetMyReturnList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x72u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F5C0) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyGetMyReturnMoney(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x74u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054F650) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyReceiveItem(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x7Au, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054F6F0) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyReceiveMoney(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x76u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054F790) --------------------------------------------------------
int __thiscall CMoFNetwork::SalesAgencyGetAvgPrice(CMoFNetwork *this, int a2, unsigned __int16 *a3)
{
  int v4; // edi
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-1002Ch]
  char v10[65552]; // [esp+Ch] [ebp-1001Ch] BYREF
  int v11; // [esp+10024h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x170u, 0);
  v4 = a2;
  v11 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v10, a2);
  if ( a2 > 0 )
  {
    do
    {
      CMofMsg::Put_WORD((CMofMsg *)v10, *a3++);
      --v4;
    }
    while ( v4 );
  }
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (0054F850) --------------------------------------------------------
int __thiscall CMoFNetwork::LessonRanking(CMoFNetwork *this, unsigned __int8 a2, int a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x90u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_LONG((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054F900) --------------------------------------------------------
int __thiscall CMoFNetwork::OpenStorage(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x100u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054F9A0) --------------------------------------------------------
int __thiscall CMoFNetwork::OpenExStorage(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x241u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054FA40) --------------------------------------------------------
int __thiscall CMoFNetwork::CloseStorage(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x102u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054FAD0) --------------------------------------------------------
int __thiscall CMoFNetwork::CloseExStorage(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x243u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (0054FB60) --------------------------------------------------------
int __thiscall CMoFNetwork::AddItemToStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x104u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054FC10) --------------------------------------------------------
int __thiscall CMoFNetwork::AddItemToExStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x245u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054FCC0) --------------------------------------------------------
int __thiscall CMoFNetwork::DelItemFromStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x106u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054FD70) --------------------------------------------------------
int __thiscall CMoFNetwork::StorageCreatePW(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x10Eu, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (0054FE10) --------------------------------------------------------
int __thiscall CMoFNetwork::StorageChangePW(CMoFNetwork *this, char *a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x160u, 0);
  v9 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054FEC0) --------------------------------------------------------
int __thiscall CMoFNetwork::DelItemFromExStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x247u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (0054FF70) --------------------------------------------------------
int __thiscall CMoFNetwork::DepositMoney(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x108u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550010) --------------------------------------------------------
int __thiscall CMoFNetwork::ExDepositMoney(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x249u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005500B0) --------------------------------------------------------
int __thiscall CMoFNetwork::WithdrawingMoney(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x10Au, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550150) --------------------------------------------------------
int __thiscall CMoFNetwork::ExWithdrawingMoney(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x24Bu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005501F0) --------------------------------------------------------
int __thiscall CMoFNetwork::ExpandStorageGenericBag(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x10Cu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550280) --------------------------------------------------------
int __thiscall CMoFNetwork::ExpandExStorageGenericBag(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x24Du, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550320) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveItemStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x261u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005503D0) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveItemExStorage(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x263u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00550480) --------------------------------------------------------
int __thiscall CMoFNetwork::RefreshFriendList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x98u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550510) --------------------------------------------------------
int __thiscall CMoFNetwork::AddFriendList(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x92u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005505B0) --------------------------------------------------------
int __thiscall CMoFNetwork::DelFriendList(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x94u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550650) --------------------------------------------------------
int __thiscall CMoFNetwork::GetFriendList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x96u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005506E0) --------------------------------------------------------
int __thiscall CMoFNetwork::AddBlackList(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x9Au, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550780) --------------------------------------------------------
int __thiscall CMoFNetwork::DelBlackList(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x9Cu, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550820) --------------------------------------------------------
int __thiscall CMoFNetwork::GetBlackList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x9Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005508B0) --------------------------------------------------------
int __thiscall CMoFNetwork::RefreshBlackList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xA1u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550940) --------------------------------------------------------
int __thiscall CMoFNetwork::GetRMonsterBook(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x110u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005509D0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetNpcBook(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xB7u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550A60) --------------------------------------------------------
int __thiscall CMoFNetwork::StrikedMine(CMoFNetwork *this, unsigned int a2, unsigned __int8 a3, unsigned int *a4)
{
  struct _EXCEPTION_REGISTRATION_RECORD *v4; // eax
  void *v5; // esp
  int v7; // edi
  char *v9; // eax
  int v10; // esi
  unsigned __int16 v12; // [esp-14h] [ebp-10030h]
  char v13[65552]; // [esp+0h] [ebp-1001Ch] BYREF
  struct _EXCEPTION_REGISTRATION_RECORD *v14; // [esp+10010h] [ebp-Ch]
  void *v15; // [esp+10014h] [ebp-8h]
  int v16; // [esp+10018h] [ebp-4h]

  v16 = -1;
  v4 = NtCurrentTeb()->NtTib.ExceptionList;
  v15 = &loc_69321B;
  v14 = v4;
  v5 = alloca(65552);
  CMofMsg::CMofMsg((CMofMsg *)v13, 0x62u, 0);
  v16 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v13, a2);
  if ( a3 )
  {
    CMofMsg::Put_BYTE((CMofMsg *)v13, a3);
    v7 = a3;
    do
    {
      CMofMsg::Put_DWORD((CMofMsg *)v13, *a4++);
      --v7;
    }
    while ( v7 );
  }
  v12 = CMofMsg::GetLength((CMofMsg *)v13);
  v9 = (char *)CMofMsg::GetMsg((CMofMsg *)v13);
  v10 = CMoFNetwork::SetSendData(this, v9, v12);
  v16 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v13);
  return v10;
}
// 550ACB: conditional instruction was optimized away because of '%arg_4.1!=0'

//----- (00550B40) --------------------------------------------------------
int __thiscall CMoFNetwork::RecallResponse(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x201u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550BE0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetTestingBegin(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x114u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550C70) --------------------------------------------------------
int __thiscall CMoFNetwork::GetTestingFinished(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3, char *a4, unsigned __int16 *a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10028h]
  char v10[65552]; // [esp+8h] [ebp-1001Ch] BYREF
  int v11; // [esp+10020h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x116u, 0);
  v11 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v10, a2);
  if ( a2 )
  {
    CMofMsg::Put((CMofMsg *)v10, a4, 10);
    CMofMsg::Put((CMofMsg *)v10, (char *)a5, 10);
  }
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (00550D40) --------------------------------------------------------
int __thiscall CMoFNetwork::AcquireSpecialty(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x112u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00550DE0) --------------------------------------------------------
int __thiscall CMoFNetwork::MakingItem(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x118u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00550E90) --------------------------------------------------------
int __thiscall CMoFNetwork::MakeCircle(CMoFNetwork *this, char *a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x200u, 0);
  v9 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v8, a2);
  CMofMsg::Put_Z2((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00550F40) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleInfo(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x208u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00550FD0) --------------------------------------------------------
int __thiscall CMoFNetwork::ReqAddCircleUser(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x202u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551070) --------------------------------------------------------
int __thiscall CMoFNetwork::ResAddcircleUser(CMoFNetwork *this, unsigned __int8 a2, char *a3, char *a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x205u, 0);
  v10 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v9, a2);
  CMofMsg::Put_Z2((CMofMsg *)v9, a3);
  CMofMsg::Put_Z2((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (00551130) --------------------------------------------------------
int __thiscall CMoFNetwork::DelCircleUser(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x20Au, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005511D0) --------------------------------------------------------
int __thiscall CMoFNetwork::DissolutionCircle(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x20Cu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551260) --------------------------------------------------------
int __thiscall CMoFNetwork::LoginCircleUser(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x20Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005512F0) --------------------------------------------------------
int __thiscall CMoFNetwork::LogoutCircleUser(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x211u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551380) --------------------------------------------------------
int __thiscall CMoFNetwork::CircleChat(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x213u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551420) --------------------------------------------------------
int __thiscall CMoFNetwork::CircleQuestStart(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x215u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005514B0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleRanking(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x223u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551540) --------------------------------------------------------
int __thiscall CMoFNetwork::GetMyCircleRank(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x225u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005515D0) --------------------------------------------------------
int __thiscall CMoFNetwork::ReqAddCircleSubMaster(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x235u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551670) --------------------------------------------------------
int __thiscall CMoFNetwork::ResAppointSubMaster(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x238u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551710) --------------------------------------------------------
int __thiscall CMoFNetwork::StartCircleQuiz(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x250u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005517A0) --------------------------------------------------------
int __thiscall CMoFNetwork::SetCircleQuiz(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x252u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00551850) --------------------------------------------------------
int __thiscall CMoFNetwork::DeleteCircleQuiz(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x254u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005518E0) --------------------------------------------------------
int __thiscall CMoFNetwork::CompleteCircleQuiz(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x256u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551970) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleQuiz(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x258u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551A00) --------------------------------------------------------
int __thiscall CMoFNetwork::TradeCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1012u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551AA0) --------------------------------------------------------
int __thiscall CMoFNetwork::WhisperCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1013u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551B40) --------------------------------------------------------
int __thiscall CMoFNetwork::PartyCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1014u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551BE0) --------------------------------------------------------
int __thiscall CMoFNetwork::PartyListCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1016u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551C80) --------------------------------------------------------
int __thiscall CMoFNetwork::AddFriendCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1015u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551D20) --------------------------------------------------------
int __thiscall CMoFNetwork::AddCircleCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1019u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551DC0) --------------------------------------------------------
int __thiscall CMoFNetwork::AddStudentCut(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x101Au, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00551E60) --------------------------------------------------------
int __thiscall CMoFNetwork::GiveupCircleQuest(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x25Du, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551EF0) --------------------------------------------------------
int __thiscall CMoFNetwork::FriendListSize(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x11Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00551F80) --------------------------------------------------------
int __thiscall CMoFNetwork::AddCircleUserInmap(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x227u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552010) --------------------------------------------------------
int __thiscall CMoFNetwork::DelCircleUserInmap(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x229u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005520A0) --------------------------------------------------------
int __thiscall CMoFNetwork::SetCircleNotice(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x21Du, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552140) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleNotice(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x220u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005521D0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleUserList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x232u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552260) --------------------------------------------------------
int __thiscall CMoFNetwork::EnchantItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x120u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00552310) --------------------------------------------------------
int __thiscall CMoFNetwork::GUIDAckMsg(CMoFNetwork *this, unsigned int a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x302u, 0);
  v9 = 0;
  if ( (unsigned __int8)MoFFont::GetNationCode((MoFFont *)&g_MoFFont) != 4 )
  {
    CMofMsg::Put_DWORD((CMofMsg *)v8, a2);
    CMofMsg::Put((CMofMsg *)v8, a3, a2);
  }
  g_bGetMyChar = 1;
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}
// 7C39FC: using guessed type int g_bGetMyChar;

//----- (005523E0) --------------------------------------------------------
int __thiscall CMoFNetwork::AckMsg(CMoFNetwork *this, unsigned int a2, unsigned int a3, char *a4, unsigned __int8 a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x304u, 0);
  v11 = 0;
  if ( (unsigned __int8)MoFFont::GetNationCode((MoFFont *)&g_MoFFont) == 4
    || (unsigned __int8)MoFFont::GetNationCode((MoFFont *)&g_MoFFont) == 5 )
  {
    CMofMsg::Put((CMofMsg *)v10, a4, 128);
  }
  else
  {
    CMofMsg::Put_DWORD((CMofMsg *)v10, a2);
    CMofMsg::Put_DWORD((CMofMsg *)v10, a3);
    CMofMsg::Put((CMofMsg *)v10, a4, a3);
    CMofMsg::Put_BYTE((CMofMsg *)v10, a5);
  }
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (005524F0) --------------------------------------------------------
int __thiscall CMoFNetwork::OpenCashShop(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x310u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552580) --------------------------------------------------------
int __thiscall CMoFNetwork::CloseCashShop(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x312u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552610) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveCashItem2Inven(CMoFNetwork *this, int a2, __int64 *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10028h]
  char v8[65552]; // [esp+8h] [ebp-1001Ch] BYREF
  int v9; // [esp+10020h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x316u, 0);
  v9 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v8, a2);
  CMofMsg::Put((CMofMsg *)v8, (char *)a3, 8 * a2);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005526D0) --------------------------------------------------------
int __thiscall CMoFNetwork::VerifyingCharName(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x318u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552770) --------------------------------------------------------
int __thiscall CMoFNetwork::BuyCashItem(CMoFNetwork *this, unsigned __int8 a2, char *a3, unsigned __int8 a4, unsigned __int8 a5, int *a6)
{
  char *v7; // eax
  int v8; // esi
  unsigned __int16 v10; // [esp-4h] [ebp-1002Ch]
  char v11[65552]; // [esp+Ch] [ebp-1001Ch] BYREF
  int v12; // [esp+10024h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v11, 0x320u, 0);
  v12 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v11, a2);
  if ( a2 )
    CMofMsg::Put_Z1((CMofMsg *)v11, a3);
  CMofMsg::Put_BYTE((CMofMsg *)v11, a4);
  CMofMsg::Put_BYTE((CMofMsg *)v11, a5);
  CMofMsg::Put((CMofMsg *)v11, (char *)a6, 4 * a5);
  v10 = CMofMsg::GetLength((CMofMsg *)v11);
  v7 = (char *)CMofMsg::GetMsg((CMofMsg *)v11);
  v8 = CMoFNetwork::SetSendData(this, v7, v10);
  v12 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v11);
  return v8;
}

//----- (00552860) --------------------------------------------------------
int __thiscall CMoFNetwork::UpdateCashMoney(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x314u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005528F0) --------------------------------------------------------
int __thiscall CMoFNetwork::AddPrivateMarket(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4, unsigned int a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x408u, 0);
  v11 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v10, a2);
  CMofMsg::Put_WORD((CMofMsg *)v10, a3);
  CMofMsg::Put_WORD((CMofMsg *)v10, a4);
  CMofMsg::Put_DWORD((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (005529C0) --------------------------------------------------------
int __thiscall CMoFNetwork::DelPrivateMarket(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x40Au, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552A60) --------------------------------------------------------
int __thiscall CMoFNetwork::ClosePrivateMarket(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x404u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552AF0) --------------------------------------------------------
int __thiscall CMoFNetwork::SetTitlePrivateMarket(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x410u, 0);
  v8 = 0;
  CMofMsg::Put_Z2((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552B90) --------------------------------------------------------
int __thiscall CMoFNetwork::ReadyPrivateMarket(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x400u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552C20) --------------------------------------------------------
int __thiscall CMoFNetwork::OpenPrivateMarket(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x402u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552CB0) --------------------------------------------------------
int __thiscall CMoFNetwork::ExitPrivateMarket(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x419u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552D40) --------------------------------------------------------
int __thiscall CMoFNetwork::GetPrivateMarket(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x406u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552DE0) --------------------------------------------------------
int __thiscall CMoFNetwork::BuyPrivateMarket(CMoFNetwork *this, char *a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x40Cu, 0);
  v11 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v10, a2);
  CMofMsg::Put_WORD((CMofMsg *)v10, a3);
  CMofMsg::Put_WORD((CMofMsg *)v10, a4);
  CMofMsg::Put_WORD((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (00552EB0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetTitlePrivateMarket(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x412u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00552F50) --------------------------------------------------------
int __thiscall CMoFNetwork::ChildsDayEventBeginTime(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x701u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00552FE0) --------------------------------------------------------
int __thiscall CMoFNetwork::ChildsDayEventEndTime(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x703u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553070) --------------------------------------------------------
int __thiscall CMoFNetwork::StartWarMeritorious(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x500u, 0);
  v10 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v9, a2);
  CMofMsg::Put_WORD((CMofMsg *)v9, a3);
  CMofMsg::Put_WORD((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (00553130) --------------------------------------------------------
int __thiscall CMoFNetwork::CompleteWarMeritorious(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x502u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005531C0) --------------------------------------------------------
int __thiscall CMoFNetwork::StartSupplyMeritorious(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x510u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553250) --------------------------------------------------------
int __thiscall CMoFNetwork::CompleteSupplyMeritorious(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x512u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005532E0) --------------------------------------------------------
int __thiscall CMoFNetwork::RewardMeritoriousItem(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x518u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553380) --------------------------------------------------------
int __thiscall CMoFNetwork::SetCheerMessage(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x705u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553420) --------------------------------------------------------
int __thiscall CMoFNetwork::MakeMatch(CMoFNetwork *this, unsigned __int8 a2, char *a3, unsigned __int16 a4, unsigned __int16 a5, char *a6, unsigned __int16 a7, unsigned __int8 a8)
{
  char *v9; // eax
  int v10; // esi
  unsigned __int16 v12; // [esp-4h] [ebp-10024h]
  char v13[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v14; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v13, 0x600u, 0);
  v14 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v13, a2);
  CMofMsg::Put_Z1((CMofMsg *)v13, a3);
  CMofMsg::Put_WORD((CMofMsg *)v13, a4);
  CMofMsg::Put_WORD((CMofMsg *)v13, a5);
  CMofMsg::Put_Z1((CMofMsg *)v13, a6);
  CMofMsg::Put_WORD((CMofMsg *)v13, a7);
  CMofMsg::Put_BYTE((CMofMsg *)v13, a8);
  v12 = CMofMsg::GetLength((CMofMsg *)v13);
  v9 = (char *)CMofMsg::GetMsg((CMofMsg *)v13);
  v10 = CMoFNetwork::SetSendData(this, v9, v12);
  v14 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v13);
  return v10;
}

//----- (00553520) --------------------------------------------------------
int __thiscall CMoFNetwork::JoinMatch(CMoFNetwork *this, unsigned __int8 a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x603u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005535D0) --------------------------------------------------------
int __thiscall CMoFNetwork::ExitMatch(CMoFNetwork *this, unsigned int a2, unsigned int a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x607u, 0);
  v9 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v8, a2);
  CMofMsg::Put_DWORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00553680) --------------------------------------------------------
int __thiscall CMoFNetwork::MoveTeam(CMoFNetwork *this, unsigned __int8 a2, unsigned int a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x60Bu, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_DWORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00553730) --------------------------------------------------------
int __thiscall CMoFNetwork::DestroyMatch(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x60Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005537C0) --------------------------------------------------------
int __thiscall CMoFNetwork::StartMatch(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x612u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553850) --------------------------------------------------------
int __thiscall CMoFNetwork::EscapeMatch(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x61Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005538E0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetCircleID(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x25Bu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553970) --------------------------------------------------------
int __thiscall CMoFNetwork::WarpMatch(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x622u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553A10) --------------------------------------------------------
int __thiscall CMoFNetwork::AcquireEmblem(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x131u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553AB0) --------------------------------------------------------
int __thiscall CMoFNetwork::SetUsingEmblem(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x133u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553B50) --------------------------------------------------------
int __thiscall CMoFNetwork::SetQuizEvent(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x801u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00553C00) --------------------------------------------------------
int __thiscall CMoFNetwork::DelQuizEvent(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x803u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553C90) --------------------------------------------------------
int __thiscall CMoFNetwork::CompleteQuizEvent(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x805u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553D20) --------------------------------------------------------
int __thiscall CMoFNetwork::FindPartyList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x141u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00553DB0) --------------------------------------------------------
int __thiscall CMoFNetwork::GetPostIt(CMoFNetwork *this, unsigned __int8 a2)
{
  int result; // eax
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x812u, 0);
  v8 = 0;
  if ( a2 )
  {
    CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
    v6 = CMofMsg::GetLength((CMofMsg *)v7);
    v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
    v5 = CMoFNetwork::SetSendData(this, v4, v6);
    v8 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v7);
    result = v5;
  }
  else
  {
    v8 = -1;
    CMofMsg::~CMofMsg((CMofMsg *)v7);
    result = 0;
  }
  return result;
}

//----- (00553E70) --------------------------------------------------------
int __thiscall CMoFNetwork::DelPostIt(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x814u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00553F10) --------------------------------------------------------
int __thiscall CMoFNetwork::CheckIDPostIt(CMoFNetwork *this, char *a2)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10028h]
  char v7[65552]; // [esp+8h] [ebp-1001Ch] BYREF
  int v8; // [esp+10020h] [ebp-4h]

  if ( !a2 )
    return 0;
  CMofMsg::CMofMsg((CMofMsg *)v7, 0x817u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v5 = CMoFNetwork::SetSendData(this, v4, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v5;
}

//----- (00553FC0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestStduent(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x151u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554060) --------------------------------------------------------
int __thiscall CMoFNetwork::ResponseStudent(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x153u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554100) --------------------------------------------------------
int __thiscall CMoFNetwork::DeleteTeacher(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x155u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554190) --------------------------------------------------------
int __thiscall CMoFNetwork::DeleteStudent(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x157u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554230) --------------------------------------------------------
int __thiscall CMoFNetwork::RefreshTasList(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x159u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005542D0) --------------------------------------------------------
int __thiscall CMoFNetwork::SendTasPostit(CMoFNetwork *this, char *a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x819u, 0);
  v9 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v8, a2);
  CMofMsg::Put_Z2((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00554380) --------------------------------------------------------
int __thiscall CMoFNetwork::SetNickname(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x900u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554420) --------------------------------------------------------
int __thiscall CMoFNetwork::GiveupWarMeritorious(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x521u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005544B0) --------------------------------------------------------
int __thiscall CMoFNetwork::GiveupSupMeritorious(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x523u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554540) --------------------------------------------------------
int __thiscall CMoFNetwork::RecallNpc(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xC5u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005545E0) --------------------------------------------------------
int __thiscall CMoFNetwork::TutorialStart(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xC7u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554670) --------------------------------------------------------
int __thiscall CMoFNetwork::TutorialEnd(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xC9u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554700) --------------------------------------------------------
int __thiscall CMoFNetwork::ChangeActionState(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x1017u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005547A0) --------------------------------------------------------
int __thiscall CMoFNetwork::UseEmoticonItem(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x1018u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00554850) --------------------------------------------------------
int __thiscall CMoFNetwork::SetSortItem(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xCBu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005548F0) --------------------------------------------------------
int __thiscall CMoFNetwork::Request_Server_Notify(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xCDu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554990) --------------------------------------------------------
int __thiscall CMoFNetwork::Request_Server_Memo(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xD3u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554A30) --------------------------------------------------------
int __thiscall CMoFNetwork::MemoSaveToServer(CMoFNetwork *this, unsigned __int8 a2, int a3, char *a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10030h]
  char Buffer; // [esp+4h] [ebp-10028h] BYREF
  int v10; // [esp+5h] [ebp-10027h]
  int v11; // [esp+9h] [ebp-10023h]
  char v12; // [esp+Dh] [ebp-1001Fh]
  char v13[65552]; // [esp+10h] [ebp-1001Ch] BYREF
  int v14; // [esp+10028h] [ebp-4h]

  Buffer = `string';
  v10 = 0;
  v11 = 0;
  v12 = 0;
  _sprintf(&Buffer, `string', a3);
  CMofMsg::CMofMsg((CMofMsg *)v13, 0xD1u, 0);
  v14 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v13, a2);
  CMofMsg::Put_Z1((CMofMsg *)v13, &Buffer);
  CMofMsg::Put_Z2((CMofMsg *)v13, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v13);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v13);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v14 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v13);
  return v6;
}

//----- (00554B20) --------------------------------------------------------
int __thiscall CMoFNetwork::MemoDeleteToServer(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10030h]
  char Buffer; // [esp+4h] [ebp-10028h] BYREF
  int v8; // [esp+5h] [ebp-10027h]
  int v9; // [esp+9h] [ebp-10023h]
  char v10; // [esp+Dh] [ebp-1001Fh]
  char v11[65552]; // [esp+10h] [ebp-1001Ch] BYREF
  int v12; // [esp+10028h] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v11, 0xD5u, 0);
  Buffer = `string';
  v8 = 0;
  v9 = 0;
  v12 = 0;
  v10 = 0;
  _sprintf(&Buffer, `string', a2);
  CMofMsg::Put_Z1((CMofMsg *)v11, &Buffer);
  v6 = CMofMsg::GetLength((CMofMsg *)v11);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v11);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v12 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v11);
  return v4;
}

//----- (00554BF0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestComeinPartyMember(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xD9u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554C90) --------------------------------------------------------
int __thiscall CMoFNetwork::SetNewTitle(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xD7u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554D30) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestCreateIndun(CMoFNetwork *this, int a2, unsigned __int8 a3, unsigned __int16 a4, unsigned __int16 a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x650u, 0);
  v11 = 0;
  if ( a2 )
  {
    if ( a2 == 1 )
    {
      CMofMsg::Put_LONG((CMofMsg *)v10, 1);
    }
    else if ( a2 == 2 )
    {
      CMofMsg::Put_LONG((CMofMsg *)v10, 2);
      CMofMsg::Put_WORD((CMofMsg *)v10, a5);
    }
  }
  else
  {
    CMofMsg::Put_LONG((CMofMsg *)v10, 0);
    CMofMsg::Put_BYTE((CMofMsg *)v10, a3);
    CMofMsg::Put_WORD((CMofMsg *)v10, a4);
  }
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (00554E20) --------------------------------------------------------
int __thiscall CMoFNetwork::InstansDungeonMoveMap(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x652u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00554EC0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestLeaveIndun(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x654u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554F50) --------------------------------------------------------
int __thiscall CMoFNetwork::JoinHighGradeShop(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x670u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00554FE0) --------------------------------------------------------
int __thiscall CMoFNetwork::LeaveHighGradeShop(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x672u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555070) --------------------------------------------------------
int __thiscall CMoFNetwork::MapConquerorInfo(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xE2u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555100) --------------------------------------------------------
int __thiscall CMoFNetwork::MapCandidateInfo(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xE4u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555190) --------------------------------------------------------
int __thiscall CMoFNetwork::MapCandidateChallenge(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xE6u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555220) --------------------------------------------------------
int __thiscall CMoFNetwork::MapCandidateGiveup(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xE8u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005552B0) --------------------------------------------------------
int __thiscall CMoFNetwork::MapcandidateStart(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xEAu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555340) --------------------------------------------------------
int __thiscall CMoFNetwork::MapcandidatePause(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0xECu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005553D0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetCreate(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x480u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00555480) --------------------------------------------------------
int __thiscall CMoFNetwork::PetSetActivity(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x482u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00555520) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMoveItemToInven(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x484u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005555D0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMoveItemFromInven(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x486u, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00555680) --------------------------------------------------------
int __thiscall CMoFNetwork::PetPickUpItem(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x488u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00555730) --------------------------------------------------------
int __thiscall CMoFNetwork::PetSetUsingSkill(CMoFNetwork *this, unsigned __int8 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x48Au, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005557E0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMoveItem(CMoFNetwork *this, unsigned __int8 a2, unsigned __int8 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x48Cu, 0);
  v9 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v8, a2);
  CMofMsg::Put_BYTE((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00555890) --------------------------------------------------------
int __thiscall CMoFNetwork::PetReleaseUsingpet(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x48Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555920) --------------------------------------------------------
int __thiscall CMoFNetwork::PetGetkeeping(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x492u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005559B0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetGetkeepingRelease(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x494u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555A40) --------------------------------------------------------
int __thiscall CMoFNetwork::PetKeeping(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x490u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555AD0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetReleaseKeepingPet(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x496u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00555B70) --------------------------------------------------------
int __thiscall CMoFNetwork::PetTakeKeepingpet(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x498u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00555C10) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketSalepetlist(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x49Cu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555CA0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketRegistry(CMoFNetwork *this, int a2, char *a3, char *a4)
{
  char *v5; // eax
  int v6; // esi
  unsigned __int16 v8; // [esp-4h] [ebp-10024h]
  char v9[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v10; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v9, 0x49Au, 0);
  v10 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v9, a2);
  CMofMsg::Put_Z1((CMofMsg *)v9, a3);
  CMofMsg::Put_Z1((CMofMsg *)v9, a4);
  v8 = CMofMsg::GetLength((CMofMsg *)v9);
  v5 = (char *)CMofMsg::GetMsg((CMofMsg *)v9);
  v6 = CMoFNetwork::SetSendData(this, v5, v8);
  v10 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v9);
  return v6;
}

//----- (00555D60) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketSalePetInfo(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x4A2u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00555E00) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketMyPetInfo(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x4A4u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555E90) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketMypetinfoRelease(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x4A8u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00555F20) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketRegistryCancel(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x49Eu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00555FC0) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketAcceptMoney(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x4A6u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556050) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketBuyPet(CMoFNetwork *this, int a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x4A0u, 0);
  v9 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00556100) --------------------------------------------------------
int __thiscall CMoFNetwork::PetMarketReregistry(CMoFNetwork *this, int a2, int a3, char *a4, char *a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x4AAu, 0);
  v11 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v10, a2);
  CMofMsg::Put_LONG((CMofMsg *)v10, a3);
  CMofMsg::Put_Z1((CMofMsg *)v10, a4);
  CMofMsg::Put_Z1((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (005561D0) --------------------------------------------------------
int __thiscall CMoFNetwork::PKJoin(CMoFNetwork *this, unsigned int a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x542u, 0);
  v9 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00556280) --------------------------------------------------------
int __thiscall CMoFNetwork::PKCreate(CMoFNetwork *this, int a2, int a3, int a4, char *a5, char *a6, unsigned __int16 a7)
{
  char *v8; // eax
  int v9; // esi
  unsigned __int16 v11; // [esp-4h] [ebp-10024h]
  char v12[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v13; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v12, 0x540u, 0);
  v13 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v12, a2);
  CMofMsg::Put_LONG((CMofMsg *)v12, a3);
  CMofMsg::Put_LONG((CMofMsg *)v12, a4);
  CMofMsg::Put_Z1((CMofMsg *)v12, a5);
  CMofMsg::Put_Z1((CMofMsg *)v12, a6);
  CMofMsg::Put_WORD((CMofMsg *)v12, a7);
  v11 = CMofMsg::GetLength((CMofMsg *)v12);
  v8 = (char *)CMofMsg::GetMsg((CMofMsg *)v12);
  v9 = CMoFNetwork::SetSendData(this, v8, v11);
  v13 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v12);
  return v9;
}

//----- (00556370) --------------------------------------------------------
int __thiscall CMoFNetwork::PKLeave(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x544u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556400) --------------------------------------------------------
int __thiscall CMoFNetwork::PKReady(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x546u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556490) --------------------------------------------------------
int __thiscall CMoFNetwork::PKReadyCancel(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x548u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556520) --------------------------------------------------------
int __thiscall CMoFNetwork::PKBan(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x54Cu, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005565C0) --------------------------------------------------------
int __thiscall CMoFNetwork::PKStart(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x54Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556650) --------------------------------------------------------
int __thiscall CMoFNetwork::PKMoveTeam(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x54Eu, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005566F0) --------------------------------------------------------
int __thiscall CMoFNetwork::PKSetSupport(CMoFNetwork *this, int a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x550u, 0);
  v9 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005567A0) --------------------------------------------------------
int __thiscall CMoFNetwork::PvPBuyItem(CMoFNetwork *this, unsigned __int16 a2, struct cltItemList *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0x552u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  cltItemList::FillOutItemListInfo(a3, (struct CMofMsg *)v8);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00556850) --------------------------------------------------------
int __thiscall CMoFNetwork::ExpendCircleMembers(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x270u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005568F0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestLogin(CMoFNetwork *this, char *a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xF000u, 0);
  v9 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005569A0) --------------------------------------------------------
int __thiscall CMoFNetwork::ShoutEventAnswer(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x809u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00556A40) --------------------------------------------------------
int __thiscall CMoFNetwork::AttendanceEventStart(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x708u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556AD0) --------------------------------------------------------
int __thiscall CMoFNetwork::AttendanceEventEnd(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x70Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556B60) --------------------------------------------------------
int __thiscall CMoFNetwork::MarriagePropose(CMoFNetwork *this, char *a2, char *a3, int a4, int a5)
{
  char *v6; // eax
  int v7; // esi
  unsigned __int16 v9; // [esp-4h] [ebp-10024h]
  char v10[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v11; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v10, 0x630u, 0);
  v11 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v10, a2);
  CMofMsg::Put_Z1((CMofMsg *)v10, a3);
  CMofMsg::Put_LONG((CMofMsg *)v10, a4);
  CMofMsg::Put_LONG((CMofMsg *)v10, a5);
  v9 = CMofMsg::GetLength((CMofMsg *)v10);
  v6 = (char *)CMofMsg::GetMsg((CMofMsg *)v10);
  v7 = CMoFNetwork::SetSendData(this, v6, v9);
  v11 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v10);
  return v7;
}

//----- (00556C30) --------------------------------------------------------
int __thiscall CMoFNetwork::MarriageProposeCancel(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x634u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00556CD0) --------------------------------------------------------
int __thiscall CMoFNetwork::ResponsePropose(CMoFNetwork *this, unsigned __int8 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x632u, 0);
  v8 = 0;
  CMofMsg::Put_BYTE((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00556D70) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestMarriageDivorce(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x636u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556E00) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestDivorceCancel(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x638u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556E90) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestSpouseRecall(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x63Au, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556F20) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestWeddingHallList(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x63Eu, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00556FB0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestJoinWeddingHall(CMoFNetwork *this, unsigned int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x640u, 0);
  v8 = 0;
  CMofMsg::Put_DWORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00557050) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestBegineWedding(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x642u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (005570E0) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestGiveBouquet(CMoFNetwork *this, char *a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x644u, 0);
  v8 = 0;
  CMofMsg::Put_Z1((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00557180) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestWeddingHallInfo(CMoFNetwork *this)
{
  char *v2; // eax
  int v3; // esi
  unsigned __int16 v5; // [esp-4h] [ebp-10024h]
  char v6[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v7; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v6, 0x646u, 0);
  v7 = 0;
  v5 = CMofMsg::GetLength((CMofMsg *)v6);
  v2 = (char *)CMofMsg::GetMsg((CMofMsg *)v6);
  v3 = CMoFNetwork::SetSendData(this, v2, v5);
  v7 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v6);
  return v3;
}

//----- (00557210) --------------------------------------------------------
int __thiscall CMoFNetwork::RequestWeddingMoveMap(CMoFNetwork *this, int a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0x648u, 0);
  v8 = 0;
  CMofMsg::Put_LONG((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (005572B0) --------------------------------------------------------
int __thiscall CMoFNetwork::UseTransport(CMoFNetwork *this, unsigned __int16 a2)
{
  char *v3; // eax
  int v4; // esi
  unsigned __int16 v6; // [esp-4h] [ebp-10024h]
  char v7[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v8; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v7, 0xF6u, 0);
  v8 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v7, a2);
  v6 = CMofMsg::GetLength((CMofMsg *)v7);
  v3 = (char *)CMofMsg::GetMsg((CMofMsg *)v7);
  v4 = CMoFNetwork::SetSendData(this, v3, v6);
  v8 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v7);
  return v4;
}

//----- (00557350) --------------------------------------------------------
int __thiscall CMoFNetwork::CanChangeCharName(CMoFNetwork *this, unsigned __int16 a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xF0u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00557400) --------------------------------------------------------
int __thiscall CMoFNetwork::ChangeCharName(CMoFNetwork *this, unsigned __int16 a2, char *a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xF2u, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_Z1((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (005574B0) --------------------------------------------------------
int __thiscall CMoFNetwork::ItemSealLock(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xFBu, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00557560) --------------------------------------------------------
int __thiscall CMoFNetwork::ItemSealUnLock(CMoFNetwork *this, unsigned __int16 a2, unsigned __int16 a3)
{
  char *v4; // eax
  int v5; // esi
  unsigned __int16 v7; // [esp-4h] [ebp-10024h]
  char v8[65552]; // [esp+4h] [ebp-1001Ch] BYREF
  int v9; // [esp+1001Ch] [ebp-4h]

  CMofMsg::CMofMsg((CMofMsg *)v8, 0xFDu, 0);
  v9 = 0;
  CMofMsg::Put_WORD((CMofMsg *)v8, a2);
  CMofMsg::Put_WORD((CMofMsg *)v8, a3);
  v7 = CMofMsg::GetLength((CMofMsg *)v8);
  v4 = (char *)CMofMsg::GetMsg((CMofMsg *)v8);
  v5 = CMoFNetwork::SetSendData(this, v4, v7);
  v9 = -1;
  CMofMsg::~CMofMsg((CMofMsg *)v8);
  return v5;
}

//----- (00557610) --------------------------------------------------------
