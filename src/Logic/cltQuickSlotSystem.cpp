#include "cltQuickSlotSystem.h"

#include <cstring>

#ifndef qmemcpy
#define qmemcpy memcpy
#endif

cltItemKindInfo *cltQuickSlotSystem::m_pclItemKindInfo = nullptr;
cltSkillKindInfo *cltQuickSlotSystem::m_pclSkillKindInfo = nullptr;

//----- (0058F410) --------------------------------------------------------
void cltQuickSlotSystem::InitializeStaticVariable(struct cltItemKindInfo *a1, struct cltSkillKindInfo *a2)
{
  cltQuickSlotSystem::m_pclItemKindInfo = a1;
  cltQuickSlotSystem::m_pclSkillKindInfo = a2;
}

//----- (0058F430) --------------------------------------------------------
cltQuickSlotSystem::cltQuickSlotSystem()
{
  memset((char *)this + 12, 0, 0x90u);
  *(_DWORD *)this = 0;
  *((_DWORD *)this + 1) = 0;
  memset((char *)this + 156, 0, 0x90u);
  *((_DWORD *)this + 75) = 0;
}

//----- (0058F470) --------------------------------------------------------
void cltQuickSlotSystem::Initialize(struct cltBaseInventory *a2, struct cltMyItemSystem *a3, struct cltQuestSystem *a4, struct CMofMsg *a5)
{
  cltBaseInventory *v5; // ebp
  struct cltQuestSystem *v7; // ecx
  CMofMsg *v8; // edi
  int v9; // ebx
  struct stItemKindInfo *v10; // eax

  v5 = a2;
  v7 = a4;
  *((_DWORD *)this + 1) = a3;
  *((_DWORD *)this + 2) = v7;
  *(_DWORD *)this = v5;
  memset((char *)this + 12, 0, 0x90u);
  v8 = a5;
  LOBYTE(a3) = 0;
  if ( a5 )
    CMofMsg::Get_BYTE(a5, (unsigned __int8 *)&a3);
  v9 = 0;
  if ( (_BYTE)a3 )
  {
    do
    {
      CMofMsg::Get_BYTE(v8, (unsigned __int8 *)&a2);
      CMofMsg::Get_DWORD(v8, (unsigned int *)this + (unsigned __int8)a2 + 3);
      v10 = cltItemKindInfo::GetItemKindInfo(
              cltQuickSlotSystem::m_pclItemKindInfo,
              48 * (unsigned __int8)a2 + (_WORD)this + 12);
      if ( v10
        && *((_WORD *)v10 + 77)
        && !cltBaseInventory::GetInventoryItem(v5, 48 * (unsigned __int8)a2 + (_WORD)this + 12) )
      {
        cltQuickSlotSystem::OnItemOuted(this, (unsigned __int8)a2, (int)this + 48 * (unsigned __int8)a2 + 12);
      }
      ++v9;
    }
    while ( v9 < (unsigned __int8)a3 );
  }
  memset((char *)this + 156, 0, 0x90u);
  *((_DWORD *)this + 75) = 0;
}

//----- (0058F570) --------------------------------------------------------
unsigned __int16 cltQuickSlotSystem::GetTransportSlot()
{
  char *v2; // ebp
  int v3; // edi
  _DWORD *v4; // esi
  unsigned __int16 *v5; // eax
  struct stItemKindInfo *v6; // eax
  int v8; // [esp+10h] [ebp-4h]

  v8 = 0;
  v2 = (char *)this + 12;
  while ( 2 )
  {
    v3 = 0;
    v4 = v2;
    do
    {
      if ( *v4 )
      {
        v5 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, *v4);
        if ( v5 )
        {
          v6 = cltItemKindInfo::GetItemKindInfo(cltQuickSlotSystem::m_pclItemKindInfo, *v5);
          if ( v6 )
          {
            if ( *((_WORD *)v6 + 77) )
              return *((_WORD *)v6 + 77);
          }
        }
      }
      ++v3;
      ++v4;
    }
    while ( v3 < 12 );
    v2 += 48;
    if ( ++v8 < 3 )
      continue;
    break;
  }
  return 0;
}

//----- (0058F5F0) --------------------------------------------------------
void cltQuickSlotSystem::Free()
{
  memset((char *)this + 12, 0, 0x90u);
  *(_DWORD *)this = 0;
  *((_DWORD *)this + 1) = 0;
  memset((char *)this + 156, 0, 0x90u);
  *((_DWORD *)this + 75) = 0;
}

//----- (0058F620) --------------------------------------------------------
struct stItemKindInfo *cltQuickSlotSystem::AddForItem(unsigned int a2, unsigned int a3, unsigned __int8 a4)
{
  struct stItemKindInfo *result; // eax
  unsigned __int16 *v7; // edi
  int v8; // ebx
  int v9; // edi
  unsigned int v10; // edx
  _DWORD *v11; // ebx
  _DWORD *v12; // eax
  unsigned __int16 *v13; // [esp+14h] [ebp+4h]
  int v14; // [esp+1Ch] [ebp+Ch]

  if ( a2 >= 3 )
    return 0;
  if ( a3 >= 0xC )
    return 0;
  result = cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, a4);
  v7 = (unsigned __int16 *)result;
  v13 = (unsigned __int16 *)result;
  if ( result )
  {
    result = cltItemKindInfo::GetItemKindInfo(cltQuickSlotSystem::m_pclItemKindInfo, *(_WORD *)result);
    if ( result )
    {
      result = cltItemKindInfo::IsQuickSlotItem(cltQuickSlotSystem::m_pclItemKindInfo, *v7);
      if ( result )
      {
        v8 = a4 | 0x10000;
        v9 = 0;
        v14 = v8;
        while ( v9 != a3 )
        {
          if ( cltItemKindInfo::IsQuickSlotRelinkableItem(cltQuickSlotSystem::m_pclItemKindInfo, *v13) != 1 )
          {
            v12 = (_DWORD *)((char *)this + 48 * a2 + 4 * v9 + 12);
            if ( *v12 != v8 )
              goto LABEL_19;
            *v12 = 0;
            goto LABEL_18;
          }
          v10 = v9 + 12 * a2;
          v11 = (_DWORD *)((char *)this + 4 * v10 + 12);
          if ( (*v11 & 0x10000) != 0
            && *v13 == *(_WORD *)cltBaseInventory::GetInventoryItem(
                                   *(cltBaseInventory **)this,
                                   *((_DWORD *)this + v10 + 3)) )
          {
            *v11 = 0;
            goto LABEL_18;
          }
LABEL_19:
          if ( (unsigned int)++v9 >= 0xC )
            return (struct stItemKindInfo *)1;
          v8 = v14;
        }
        *((_DWORD *)this + 12 * a2 + v9 + 3) = v8;
LABEL_18:
        cltQuickSlotSystem::SetChange(this, a2, v9);
        goto LABEL_19;
      }
    }
  }
  return result;
}

//----- (0058F750) --------------------------------------------------------
int cltQuickSlotSystem::AddForSkill(unsigned int a2, unsigned int a3, unsigned __int16 a4)
{
  int v6; // ebp
  unsigned int i; // esi
  unsigned int v8; // eax
  int v9; // ecx
  _DWORD *v10; // eax

  if ( a2 >= 3 )
    return 0;
  if ( a3 >= 0xC )
    return 0;
  v6 = a4 | 0x20000;
  for ( i = 0; i < 0xC; ++i )
  {
    if ( i == a3 )
    {
      *((_DWORD *)this + 12 * a2 + i + 3) = v6;
    }
    else
    {
      v8 = i + 12 * a2;
      v9 = *((_DWORD *)this + v8 + 3);
      v10 = (_DWORD *)((char *)this + 4 * v8 + 12);
      if ( v9 != v6 )
        continue;
      *v10 = 0;
    }
    cltQuickSlotSystem::SetChange(this, a2, i);
  }
  return 1;
}

//----- (0058F7D0) --------------------------------------------------------
int cltQuickSlotSystem::AddForEmblem(unsigned int a2, unsigned int a3, unsigned __int16 a4, unsigned __int16 *a5, unsigned int *a6)
{
  int v8; // ebp
  unsigned int i; // esi
  _DWORD *v10; // eax

  if ( a2 >= 3 )
    return 0;
  if ( a3 >= 0xC )
    return 0;
  cltQuestSystem::CompleteFunctionQuest(*((cltQuestSystem **)this + 2), 31, a5, a6);
  v8 = a4 | 0x40000;
  for ( i = 0; i < 0xC; ++i )
  {
    if ( i == a3 )
    {
      *((_DWORD *)this + 12 * a2 + i + 3) = v8;
    }
    else
    {
      v10 = (_DWORD *)((char *)this + 48 * a2 + 4 * i + 12);
      if ( *v10 != v8 )
        continue;
      *v10 = 0;
    }
    cltQuickSlotSystem::SetChange(this, a2, i);
  }
  return 1;
}

//----- (0058F860) --------------------------------------------------------
int cltQuickSlotSystem::Del(unsigned int a2, unsigned int a3)
{
  if ( a2 >= 3 )
    return 0;
  if ( a3 >= 0xC )
    return 0;
  *((_DWORD *)this + 12 * a2 + a3 + 3) = 0;
  cltQuickSlotSystem::SetChange(this, a2, a3);
  return 1;
}

//----- (0058F8A0) --------------------------------------------------------
int cltQuickSlotSystem::Move(unsigned int a2, unsigned int a3, unsigned int a4)
{
  int v4; // edi
  unsigned int v7; // ecx
  unsigned int *v8; // eax
  unsigned int v10; // [esp+10h] [ebp+4h]

  v4 = a2;
  if ( a2 >= 3 )
    return 0;
  if ( a3 >= 0xC )
    return 0;
  if ( a4 >= 0xC )
    return 0;
  if ( (cltQuickSlotSystem *)((char *)this + 48 * a3) == (cltQuickSlotSystem *)-12 )
    return 0;
  if ( a3 == a4 )
    return 0;
  v7 = 12 * a2 + a4;
  v8 = (unsigned int *)((char *)this + 48 * a2 + 4 * a3 + 12);
  v10 = *((_DWORD *)this + v7 + 3);
  *((_DWORD *)this + v7 + 3) = *v8;
  *v8 = v10;
  cltQuickSlotSystem::SetChange(this, a2, a4);
  cltQuickSlotSystem::SetChange(this, v4, a3);
  return 1;
}

//----- (0058F940) --------------------------------------------------------
unsigned int cltQuickSlotSystem::Get(unsigned int a2, unsigned int a3)
{
  if ( a2 >= 3 )
    return 0;
  if ( a3 < 0xC )
    return *((_DWORD *)this + 12 * a2 + a3 + 3);
  return 0;
}

//----- (0058F970) --------------------------------------------------------
void cltQuickSlotSystem::OnItemMerged(unsigned __int8 a2, unsigned __int8 a3)
{
  int v3; // esi
  int v4; // edi
  char *v5; // ebx
  int v6; // eax
  _DWORD *v7; // ecx
  int v8; // eax
  _DWORD *v9; // ecx
  int v10; // [esp+10h] [ebp-8h]
  int v12; // [esp+1Ch] [ebp+4h]

  v3 = a3 | 0x10000;
  v4 = 0;
  v10 = a2 | 0x10000;
  v12 = 0;
  v5 = (char *)this + 12;
  do
  {
    v6 = 0;
    v7 = v5;
    do
    {
      if ( *v7 == v3 )
        break;
      ++v6;
      ++v7;
    }
    while ( v6 < 12 );
    if ( v6 == 12 )
    {
      v8 = 0;
      v9 = v5;
      while ( *v9 != v10 )
      {
        ++v8;
        ++v9;
        if ( v8 >= 12 )
          goto LABEL_11;
      }
      *((_DWORD *)this + v4 + v8 + 3) = v3;
      cltQuickSlotSystem::SetChange(this, v12, v8);
    }
LABEL_11:
    v4 += 12;
    v5 += 48;
    ++v12;
  }
  while ( v4 < 36 );
}

//----- (0058FA20) --------------------------------------------------------
void cltQuickSlotSystem::OnItemMoved(unsigned __int8 a2, unsigned __int8 a3)
{
  int v4; // ecx
  int v5; // ebx
  int *v6; // esi
  int i; // edi
  int v8; // [esp+10h] [ebp-4h]
  int v9; // [esp+18h] [ebp+4h]

  v4 = a2 | 0x10000;
  v8 = v4;
  v9 = a3 | 0x10000;
  v5 = 0;
  v6 = (int *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      if ( *v6 == v4 )
      {
        *v6 = v9;
      }
      else
      {
        if ( *v6 != v9 )
          goto LABEL_8;
        *v6 = v4;
      }
      cltQuickSlotSystem::SetChange(this, v5, i);
      v4 = v8;
LABEL_8:
      ++v6;
    }
    ++v5;
  }
  while ( v5 < 3 );
}

//----- (0058FAA0) --------------------------------------------------------
void cltQuickSlotSystem::OnItemOuted(unsigned __int8 a2, int a3)
{
  int v4; // edi
  signed int v5; // ebx
  _DWORD *v6; // edi
  signed int i; // esi
  int v8; // [esp+10h] [ebp+4h]

  v4 = a2 | 0x10000;
  v8 = v4;
  if ( cltItemKindInfo::IsQuickSlotRelinkableItem(cltQuickSlotSystem::m_pclItemKindInfo, a3) != 1
    || !cltQuickSlotSystem::OnRelinkItem(this, v4, a3) )
  {
    v5 = 0;
    v6 = (_DWORD *)((char *)this + 12);
    do
    {
      for ( i = 0; i < 12; ++i )
      {
        if ( *v6 == v8 )
          cltQuickSlotSystem::Del(this, v5, i);
        ++v6;
      }
      ++v5;
    }
    while ( v5 < 3 );
  }
}

//----- (0058FB10) --------------------------------------------------------
void cltQuickSlotSystem::OnSkillDeleted(unsigned __int16 a2)
{
  signed int v2; // ebx
  char *v3; // edi
  signed int v4; // eax
  _DWORD *v5; // edx

  v2 = 0;
  v3 = (char *)this + 12;
  while ( 2 )
  {
    v4 = 0;
    v5 = v3;
    do
    {
      if ( *v5 == (a2 | 0x20000) )
      {
        cltQuickSlotSystem::Del(this, v2, v4);
        return;
      }
      ++v4;
      ++v5;
    }
    while ( v4 < 12 );
    ++v2;
    v3 += 48;
    if ( v2 < 3 )
      continue;
    break;
  }
}

//----- (0058FB60) --------------------------------------------------------
void cltQuickSlotSystem::OnSkillAdded(unsigned __int16 a2)
{
  struct stSkillKindInfo *v3; // eax
  _DWORD *v4; // ecx
  int v5; // ebx
  struct stSkillKindInfo *v6; // ebp
  int v7; // esi
  int v8; // eax
  char *v9; // [esp+10h] [ebp-4h]

  v3 = cltSkillKindInfo::GetSkillKindInfo(cltQuickSlotSystem::m_pclSkillKindInfo, a2);
  v4 = (_DWORD *)((char *)this + 12);
  v5 = 0;
  v6 = v3;
  v7 = 0;
  v9 = (char *)this + 12;
  do
  {
    v8 = 0;
    while ( (*v4 & 0x20000) == 0 || (unsigned __int16)*v4 != *((_WORD *)v6 + 35) )
    {
      ++v8;
      ++v4;
      if ( v8 >= 12 )
        goto LABEL_8;
    }
    *((_DWORD *)this + v7 + v8 + 3) = a2 | 0x20000;
    cltQuickSlotSystem::SetChange(this, v5, v8);
LABEL_8:
    v7 += 12;
    ++v5;
    v4 = v9 + 48;
    v9 += 48;
  }
  while ( v7 < 36 );
}

//----- (0058FBE0) --------------------------------------------------------
void cltQuickSlotSystem::OnClassReseted()
{
  int v2; // ebx
  _DWORD *v3; // esi
  int i; // edi

  v2 = 0;
  v3 = (_DWORD *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      if ( (*v3 & 0x20000) != 0 )
      {
        *v3 = 0;
        cltQuickSlotSystem::SetChange(this, v2, i);
      }
      ++v3;
    }
    ++v2;
  }
  while ( v2 < 3 );
}

//----- (0058FC20) --------------------------------------------------------
void cltQuickSlotSystem::OnItemSorted(struct strInventoryItemForSort *a2)
{
  int v2; // ebx
  int *v3; // esi
  int i; // edi
  struct strInventoryItemForSort *v5; // ecx
  int v6; // eax

  v2 = 0;
  v3 = (int *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      if ( (*v3 & 0x10000) != 0 )
      {
        v5 = a2;
        v6 = 0;
        while ( (unsigned __int16)*v3 != *(_DWORD *)v5 )
        {
          ++v6;
          v5 = (struct strInventoryItemForSort *)((char *)v5 + 16);
          if ( v6 > 71 )
            goto LABEL_9;
        }
        *v3 = v6 | 0x10000;
        cltQuickSlotSystem::SetChange(this, v2, i);
      }
LABEL_9:
      ++v3;
    }
    ++v2;
  }
  while ( v2 < 3 );
}

//----- (0058FC80) --------------------------------------------------------
void cltQuickSlotSystem::GetQuickSlotInfoForDBQuery(char *Buffer)
{
  cltQuickSlotSystem *v2; // esi
  int v3; // ebp
  _DWORD *v4; // esi
  int v5; // edi
  int v6; // ebx

  v2 = this;
  if ( cltQuickSlotSystem::IsChange(this) )
  {
    v3 = 0;
    v4 = (_DWORD *)((char *)v2 + 156);
    do
    {
      v5 = 0;
      v6 = v3;
      do
      {
        if ( *v4 )
        {
          _sprintf(Buffer, "%s%d, %d, ", Buffer, v6, *((_DWORD *)this + v5 + v3 + 3));
          *v4 = 0;
        }
        ++v5;
        ++v6;
        ++v4;
      }
      while ( v5 < 12 );
      v3 += 12;
    }
    while ( v3 < 36 );
    *((_DWORD *)this + 75) = 0;
  }
}

//----- (0058FD00) --------------------------------------------------------
void cltQuickSlotSystem::OnPremiumQuickSlotEnabled()
{
  int v2; // ebx
  _DWORD *v3; // edi
  int i; // esi

  v2 = 0;
  v3 = (_DWORD *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      *v3 = 0;
      cltQuickSlotSystem::SetChange(this, v2, i);
      ++v3;
    }
    ++v2;
  }
  while ( v2 < 3 );
}

//----- (0058FD30) --------------------------------------------------------
void cltQuickSlotSystem::OnPremiumQuickSlotDisabled()
{
  int v2; // ebx
  _DWORD *v3; // edi
  int i; // esi

  v2 = 0;
  v3 = (_DWORD *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      *v3 = 0;
      cltQuickSlotSystem::SetChange(this, v2, i);
      ++v3;
    }
    ++v2;
  }
  while ( v2 < 3 );
}

//----- (0058FD60) --------------------------------------------------------
int cltQuickSlotSystem::OnRelinkItem(unsigned int a2, int a3)
{
  int v4; // eax
  int v6; // ebx
  int *v7; // esi
  int i; // edi
  int v9; // [esp+Ch] [ebp+8h]

  v4 = cltBaseInventory::FindFirstItem(*(cltBaseInventory **)this, a3);
  if ( v4 == -1 )
    return 0;
  v9 = v4 | 0x10000;
  v6 = 0;
  v7 = (int *)((char *)this + 12);
  do
  {
    for ( i = 0; i < 12; ++i )
    {
      if ( *v7 == a2 )
      {
        *v7 = v9;
        cltQuickSlotSystem::SetChange(this, v6, i);
      }
      ++v7;
    }
    ++v6;
  }
  while ( v6 < 3 );
  return 1;
}

//----- (0058FDD0) --------------------------------------------------------
void cltQuickSlotSystem::SetChange(int a2, int a3)
{
  *((_DWORD *)this + 12 * a2 + a3 + 39) = 1;
  if ( !*((_DWORD *)this + 75) )
    *((_DWORD *)this + 75) = 1;
}

//----- (0058FE00) --------------------------------------------------------
int cltQuickSlotSystem::IsChange()
{
  return *((_DWORD *)this + 75);
}

//----- (0058FE10) --------------------------------------------------------
void cltQuickSlotSystem::CheckQSLItem()
{
  _DWORD *v2; // esi
  signed int v3; // ebp
  signed int v4; // edi
  char *v5; // [esp+10h] [ebp-4h]

  v2 = (_DWORD *)((char *)this + 12);
  v3 = 0;
  v5 = (char *)this + 12;
  do
  {
    v4 = 0;
    while ( (*v2 & 0x10000) == 0 || *(_WORD *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, *v2) )
    {
      ++v4;
      ++v2;
      if ( v4 >= 12 )
        goto LABEL_8;
    }
    cltQuickSlotSystem::Del(this, v3, v4);
LABEL_8:
    ++v3;
    v2 = v5 + 48;
    v5 += 48;
  }
  while ( v3 < 3 );
}
