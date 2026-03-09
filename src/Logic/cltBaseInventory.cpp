#include "cltBaseInventory.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef __thiscall
#define __thiscall
#endif
#ifndef __cdecl
#define __cdecl
#endif


cltItemKindInfo* cltBaseInventory::m_pclItemKindInfo = nullptr;
DCTTextManager* cltBaseInventory::m_pclTextManager = nullptr;
void (*cltBaseInventory::m_pExternCriticalErrorFuncPtr)(char*, char*, unsigned int) = nullptr;

void cltBaseInventory::InitializeStaticVariable(struct cltItemKindInfo *a1, struct DCTTextManager *a2, void (*a3)(char *, char *, unsigned int))
{
  cltBaseInventory::m_pclItemKindInfo = a1;
  cltBaseInventory::m_pclTextManager = a2;
  cltBaseInventory::m_pExternCriticalErrorFuncPtr = a3;
}

//----- (0055F640) --------------------------------------------------------
cltBaseInventory::cltBaseInventory()
{
  int v2; // ecx
  char *v3; // eax

  v2 = 255;
  v3 = (char *)this + 1056;
  do
  {
    *(_WORD *)v3 = 0;
    *((_WORD *)v3 + 1) = 0;
    *((_DWORD *)v3 + 1) = 0;
    *((_DWORD *)v3 + 2) = 0;
    v3 += 12;
    --v2;
  }
  while ( v2 );
  *((_BYTE *)this + 1040) = 1;
  *((_DWORD *)this + 261) = 0;
  memset((char *)this + 1056, 0, 0xBF4u);
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 2) = 0;
  memset((char *)this + 12, 0, 0x400u);
}

//----- (0055F6C0) --------------------------------------------------------
cltBaseInventory::~cltBaseInventory()
{
}

//----- (0055F6D0) --------------------------------------------------------
void cltBaseInventory::Initialize(struct cltQuickSlotSystem *a2, struct cltTitleSystem *a3, struct cltPetSystem *a4, int a5, struct cltItemList *a6)
{
  cltItemList *v7; // ebx
  char v8; // cl
  struct cltTitleSystem *v9; // edx
  struct cltPetSystem *v10; // eax
  int v11; // edi
  int v12; // eax

  if ( a2 )
  {
    v7 = a6;
    if ( a6 )
    {
      v8 = a5;
      v9 = a3;
      *((_DWORD *)this + 261) = a2;
      v10 = a4;
      *((_BYTE *)this + 1040) = v8;
      *((_DWORD *)this + 262) = v9;
      *((_DWORD *)this + 263) = v10;
      v11 = 0;
      if ( cltItemList::GetItemsNum(v7) )
      {
        do
        {
          a2 = 0;
          a6 = 0;
          a5 = 0;
          a3 = 0;
          cltItemList::GetItem(
            v7,
            v11,
            (unsigned __int16 *)&a2,
            (unsigned __int16 *)&a6,
            (unsigned int *)&a3,
            (unsigned __int16 *)&a5,
            0);
          *((_WORD *)this + 6 * (unsigned __int16)a5 + 528) = (_WORD)a2;
          *((_WORD *)this + 6 * (unsigned __int16)a5 + 529) = (_WORD)a6;
          *((_DWORD *)this + 3 * (unsigned __int16)a5 + 265) = a3;
          v12 = cltItemList::GetSealedStatus((unsigned __int16 *)v7, v11++);
          *((_DWORD *)this + 3 * (unsigned __int16)a5 + 266) = v12;
        }
        while ( v11 < cltItemList::GetItemsNum(v7) );
      }
      *((_DWORD *)this + 1) = 0;
      *((_DWORD *)this + 2) = 0;
      memset((char *)this + 12, 0, 0x400u);
    }
  }
}

//----- (0055F7F0) --------------------------------------------------------
unsigned int cltBaseInventory::CanAddInventoryItem(int a2, unsigned __int16 a3, unsigned __int16 a4)
{
  unsigned int result; // eax
  struct stItemKindInfo *v6; // eax
  __int16 v7; // ax
  int v8; // [esp+10h] [ebp-8h] BYREF
  int v9; // [esp+14h] [ebp-4h] BYREF

  if ( cltBaseInventory::IsLock(this) == 1 )
    return 108;
  if ( !a3 )
    goto LABEL_11;
  if ( !a4 )
    goto LABEL_11;
  v6 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, a3);
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v6 == 0, &v8, &v9);
  if ( a2 < v8 || a2 > v9 )
    goto LABEL_11;
  v7 = *((_WORD *)this + 6 * a2 + 528);
  if ( !v7 )
    return 0;
  if ( v7 == a3 )
    result = a4 + *((unsigned __int16 *)this + 6 * a2 + 529) > cltBaseInventory::GetPileUpNum(this, a3);
  else
LABEL_11:
    result = 1;
  return result;
}

//----- (0055F8D0) --------------------------------------------------------
int cltBaseInventory::CanAddInventoryItem(unsigned __int16 a2, unsigned __int16 a3)
{
  int v5; // ebp
  int result; // eax
  int v7; // esi
  unsigned __int16 *v8; // esi
  __int16 v9; // ax
  int v10; // esi
  int v11; // [esp+10h] [ebp-8h] BYREF
  int v12; // [esp+14h] [ebp-4h] BYREF
  int v13; // [esp+1Ch] [ebp+4h]
  int v14; // [esp+20h] [ebp+8h]

  if ( cltBaseInventory::IsLock(this) == 1 || !a2 || !a3 )
    return 1;
  v5 = a3;
  v12 = 0;
  v11 = 0;
  if ( !cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, a2) )
    return 5;
  v7 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, a2) == 0;
  v14 = v7;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v7, &v12, &v11);
  v13 = v12;
  if ( v12 > v11 )
  {
LABEL_16:
    v10 = cltBaseInventory::GetEmptyItemSlotNum(this, v7);
    if ( v10 )
      result = v5 <= v10 * cltBaseInventory::GetPileUpNum(this, a2) ? 0 : 0x67;
    else
      result = 103;
  }
  else
  {
    v8 = (unsigned __int16 *)((char *)this + 12 * v12 + 1058);
    while ( 1 )
    {
      v9 = *(v8 - 1);
      if ( v9 && v9 == a2 )
      {
        if ( v5 + *v8 <= cltBaseInventory::GetPileUpNum(this, a2) )
          break;
        if ( v5 + *v8 > cltBaseInventory::GetPileUpNum(this, a2) )
          v5 += *v8 - cltBaseInventory::GetPileUpNum(this, a2);
        if ( v5 < 0 )
          break;
      }
      v8 += 6;
      if ( ++v13 > v11 )
      {
        v7 = v14;
        goto LABEL_16;
      }
    }
    result = 0;
  }
  return result;
}

//----- (0055FA50) --------------------------------------------------------
int cltBaseInventory::CanAddInventoryItems(struct cltItemList *a2)
{
  int result; // eax
  bool v4; // zf
  unsigned int v5; // eax
  unsigned int v6; // eax
  int *v7; // ebp
  int v8; // eax
  unsigned __int8 v9; // bl
  int v10; // eax
  unsigned __int16 i; // cx
  _WORD *v12; // edi
  unsigned __int8 v13; // al
  int v14; // edx
  unsigned __int8 v15; // al
  unsigned __int8 v16; // al
  unsigned __int8 v17; // al
  unsigned __int8 v18; // al
  unsigned __int16 v19; // [esp-4h] [ebp-44h]
  unsigned __int16 v20[2]; // [esp+10h] [ebp-30h] BYREF
  unsigned __int16 v21[2]; // [esp+14h] [ebp-2Ch] BYREF
  int v22; // [esp+18h] [ebp-28h] BYREF
  int v23; // [esp+1Ch] [ebp-24h] BYREF
  unsigned int v24; // [esp+20h] [ebp-20h]
  int v25; // [esp+24h] [ebp-1Ch] BYREF
  int v26; // [esp+28h] [ebp-18h] BYREF
  int v27; // [esp+2Ch] [ebp-14h]
  unsigned int v28; // [esp+30h] [ebp-10h]
  int v29; // [esp+34h] [ebp-Ch]
  int v30; // [esp+38h] [ebp-8h]
  unsigned int v31; // [esp+3Ch] [ebp-4h] BYREF

  v25 = 0;
  v26 = 0;
  *(_DWORD *)v20 = 0;
  *(_DWORD *)v21 = 0;
  v23 = 0;
  v22 = 0;
  result = cltBaseInventory::IsLock(this);
  if ( result != 1 )
  {
    v29 = cltBaseInventory::GetEmptyItemSlotNum(this, 1);
    v30 = cltBaseInventory::GetEmptyItemSlotNum(this, 0);
    v5 = cltItemList::GetItemsNum(a2);
    v4 = v5 == 0;
    v28 = v5;
    v6 = 0;
    v24 = 0;
    if ( v4 )
    {
LABEL_16:
      if ( v25 <= v29 )
        result = v26 <= v30 ? 0 : 0x67;
      else
        result = 103;
    }
    else
    {
      while ( 1 )
      {
        cltItemList::GetItem(a2, v6, v20, v21, &v31, 0, 0);
        result = cltBaseInventory::CanAddInventoryItem(this, v20[0], v21[0]);
        if ( result )
          break;
        v7 = &v25;
        v8 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, v20[0]) == 0;
        if ( !v8 )
          v7 = &v26;
        cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v8, &v23, &v22);
        v9 = v23;
        v10 = (unsigned __int8)v23;
        for ( i = v20[0]; v9 <= v22; v10 = v9 )
        {
          if ( *((_WORD *)this + 6 * v10 + 528) == i )
          {
            v12 = (_WORD *)((char *)this + 12 * v10 + 1058);
            v13 = cltBaseInventory::GetPileUpNum(this, i);
            if ( v21[0] + (unsigned __int16)*v12 <= v13 )
              goto LABEL_15;
            LOWORD(v14) = *v12 - cltBaseInventory::GetPileUpNum(this, v20[0]);
            i = v20[0];
            *(_DWORD *)v21 += v14;
          }
          LOBYTE(v27) = ++v9;
        }
        v15 = cltBaseInventory::GetPileUpNum(this, i);
        if ( v21[0] <= v15
          || (v16 = cltBaseInventory::GetPileUpNum(this, v20[0]), v21[0] >= v16)
          && (v17 = cltBaseInventory::GetPileUpNum(this, v20[0]),
              v19 = v20[0],
              *v7 += v21[0] / (int)v17,
              v18 = cltBaseInventory::GetPileUpNum(this, v19),
              v21[0] % (int)v18 > 0) )
        {
          ++*v7;
        }
LABEL_15:
        v6 = ++v24;
        if ( v24 >= v28 )
          goto LABEL_16;
      }
    }
  }
  return result;
}
// 55FB9C: variable 'v14' is possibly undefined

//----- (0055FC80) --------------------------------------------------------
int cltBaseInventory::CanDelInventoryItems(struct cltItemList *a2)
{
  unsigned int v2; // esi
  unsigned __int16 v5; // ax
  signed int v6; // ebx
  struct strInventoryItem *v7; // eax
  unsigned __int16 v8[2]; // [esp+10h] [ebp-10h] BYREF
  unsigned __int16 v9[2]; // [esp+14h] [ebp-Ch] BYREF
  unsigned int v10; // [esp+18h] [ebp-8h] BYREF
  unsigned __int16 v11[2]; // [esp+1Ch] [ebp-4h] BYREF

  v2 = 0;
  *(_DWORD *)v11 = 0;
  *(_DWORD *)v9 = 0;
  *(_DWORD *)v8 = 0;
  v10 = 0;
  if ( cltBaseInventory::IsLock(this) == 1 )
    return 0;
  if ( !a2 )
    return 0;
  v5 = cltItemList::GetItemsNum(a2);
  v6 = v5;
  if ( v5 )
  {
    while ( 1 )
    {
      cltItemList::GetItem(a2, v2, v11, v9, &v10, v8, 0);
      v7 = cltBaseInventory::GetInventoryItem(this, v8[0]);
      if ( !v7 || !*(_WORD *)v7 || v9[0] > *((_WORD *)v7 + 1) || *((_DWORD *)v7 + 2) )
        break;
      if ( (int)++v2 >= v6 )
        return 1;
    }
    return 0;
  }
  return 1;
}

//----- (0055FD40) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::CanDelInventoryItem(unsigned __int16 a2, unsigned __int16 a3)
{
  struct strInventoryItem *result; // eax

  if ( cltBaseInventory::IsLock(this) == 1 )
    result = 0;
  else
    result = cltBaseInventory::IsThereInventoryItem(this, a2, a3);
  return result;
}

//----- (0055FD70) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::CanDelInventoryItem(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4)
{
  struct strInventoryItem *result; // eax

  if ( cltBaseInventory::IsLock(this) == 1 )
    result = 0;
  else
    result = cltBaseInventory::IsThereInventoryItem(this, a2, a3, a4);
  return result;
}

//----- (0055FDA0) --------------------------------------------------------
BOOL cltBaseInventory::CanDelInventoryItemByKindNQty(int a2, unsigned __int16 a3)
{
  BOOL result; // eax

  if ( cltBaseInventory::IsLock(this) == 1 )
    result = 0;
  else
    result = cltBaseInventory::IsThereInventoryItemByKindNQty(this, a2, a3);
  return result;
}

//----- (0055FDD0) --------------------------------------------------------
int cltBaseInventory::AddInventoryItem(struct cltItemList *a2, unsigned __int8 *a3)
{
  signed int v3; // esi
  unsigned __int16 v6; // ax
  signed int v7; // ebx
  int v8; // eax
  unsigned __int16 v9[2]; // [esp+Ch] [ebp-1Ch] BYREF
  unsigned __int16 v10[2]; // [esp+10h] [ebp-18h] BYREF
  unsigned int v11; // [esp+14h] [ebp-14h] BYREF
  unsigned __int16 v12[2]; // [esp+18h] [ebp-10h] BYREF
  __int16 v13[2]; // [esp+1Ch] [ebp-Ch] BYREF
  unsigned int v14; // [esp+20h] [ebp-8h]
  int v15; // [esp+24h] [ebp-4h]

  v3 = 0;
  *(_DWORD *)v9 = 0;
  *(_DWORD *)v10 = 0;
  v11 = 0;
  *(_DWORD *)v12 = 0;
  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("AddInventoryItem:1", (char *)this + 12, *((_DWORD *)this + 259));
  if ( cltBaseInventory::CanAddInventoryItems(this, a2) )
    return 0;
  v6 = cltItemList::GetItemsNum(a2);
  v7 = v6;
  if ( v6 )
  {
    do
    {
      cltItemList::GetItem(a2, v3, v9, v10, &v11, 0, 0);
      v8 = cltItemList::GetSealedStatus((unsigned __int16 *)a2, v3);
      v13[0] = v9[0];
      v13[1] = v10[0];
      v15 = v8;
      v14 = v11;
      cltBaseInventory::AddInventoryItem(this, (struct strInventoryItem *)v13, a3, v12);
      cltItemList::AddTradeItemPos(a2, v3++, v12[0]);
    }
    while ( v3 < v7 );
  }
  return 1;
}

//----- (0055FEC0) --------------------------------------------------------
int cltBaseInventory::AddInventoryItem(struct strInventoryItem *a2, unsigned __int8 *a3, unsigned __int16 *a4)
{
  unsigned __int16 v7; // cx
  _WORD *v8; // ebp
  __int16 v9; // cx
  unsigned __int16 v10; // bx
  int v11; // ebp
  unsigned __int16 v12; // bx
  unsigned __int16 v13; // bp
  int v14; // [esp+10h] [ebp-Ch] BYREF
  int v15; // [esp+14h] [ebp-8h] BYREF
  int v16; // [esp+18h] [ebp-4h]
  struct strInventoryItem *v17; // [esp+20h] [ebp+4h]
  struct strInventoryItem *v18; // [esp+20h] [ebp+4h]

  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("AddInventoryItem:2", (char *)this + 12, *((_DWORD *)this + 259));
  if ( cltBaseInventory::CanAddInventoryItem(this, *(_WORD *)a2, *((_WORD *)a2 + 1)) )
    return 0;
  v7 = *(_WORD *)a2;
  v15 = 0;
  v14 = 0;
  if ( !cltBaseInventory::GetItemSlotIndexRangeByItemKind(this, v7, &v15, &v14) )
    return 0;
  v17 = (struct strInventoryItem *)v15;
  if ( v15 > v14 )
  {
LABEL_15:
    v11 = *((unsigned __int16 *)a2 + 1) / (int)cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2);
    if ( *((unsigned __int16 *)a2 + 1) % (int)cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2) > 0 )
      LOWORD(v11) = v11 + 1;
    v18 = 0;
    v16 = (unsigned __int16)v11;
    if ( !(_WORD)v11 )
      return 1;
    while ( 1 )
    {
      v12 = cltBaseInventory::GetEmptyItemSlotPos(this, *(_WORD *)a2, v15, v14);
      if ( v12 == -1 )
        break;
      v13 = *((_WORD *)a2 + 1);
      if ( v13 > cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2) )
      {
        v13 = cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2);
        *((_WORD *)a2 + 1) -= cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2);
      }
      *((_WORD *)a2 + 1) = v13;
      cltBaseInventory::AddInventoryItem(this, v12, a2, a3);
      if ( a4 )
        *a4 = v12;
      v18 = (struct strInventoryItem *)((char *)v18 + 1);
      if ( (int)v18 >= v16 )
        return 1;
    }
    return 0;
  }
  v8 = (_WORD *)((char *)this + 12 * v15 + 1058);
  while ( 1 )
  {
    v9 = *(v8 - 1);
    if ( v9 )
    {
      if ( v9 == *(_WORD *)a2 )
        break;
    }
LABEL_14:
    v8 += 6;
    v17 = (struct strInventoryItem *)((char *)v17 + 1);
    if ( (int)v17 > v14 )
      goto LABEL_15;
  }
  v10 = *((_WORD *)a2 + 1);
  if ( cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2) - (unsigned __int16)*v8 < v10 )
  {
    *((_WORD *)a2 + 1) = cltBaseInventory::GetPileUpNum(this, *(_WORD *)a2) - *v8;
    cltBaseInventory::AddInventoryItem(this, (unsigned __int16)v17, a2, a3);
    *((_WORD *)a2 + 1) = v10;
    if ( !v10 )
      return 1;
    goto LABEL_14;
  }
  cltBaseInventory::AddInventoryItem(this, (unsigned __int16)v17, a2, a3);
  return 1;
}

//----- (00560110) --------------------------------------------------------
int cltBaseInventory::AddInventoryItem(unsigned __int16 a2, struct strInventoryItem *a3, unsigned __int8 *a4)
{
  int result; // eax
  char *v6; // ecx

  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("AddInventoryItem:3", (char *)this + 12, *((_DWORD *)this + 259));
  result = (int)cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    if ( *((_WORD *)a3 + 1) )
    {
      *((_WORD *)this + 6 * a2 + 528) = *(_WORD *)a3;
      *((_WORD *)this + 6 * a2 + 529) += *((_WORD *)a3 + 1);
      v6 = (char *)this + 12 * a2;
      *((_DWORD *)v6 + 265) = *((_DWORD *)a3 + 1);
      *((_DWORD *)v6 + 266) = *((_DWORD *)a3 + 2);
      HIWORD(v6) = HIWORD(a4);
      if ( a4 )
        a4[a2] = 1;
      LOWORD(v6) = *(_WORD *)a3;
      cltTitleSystem::OnEvent_getitem(*((cltTitleSystem **)this + 262), (int)v6, *((unsigned __int16 *)a3 + 1));
    }
    result = 1;
  }
  return result;
}

//----- (005601D0) --------------------------------------------------------
int cltBaseInventory::DelInventoryItem(struct cltItemList *a2, unsigned __int8 *a3)
{
  unsigned __int16 v4; // ax
  signed int v5; // esi
  signed int v6; // edi
  unsigned __int16 v8; // [esp+12h] [ebp-Eh] BYREF
  unsigned __int16 v9[2]; // [esp+14h] [ebp-Ch] BYREF
  unsigned __int16 v10[2]; // [esp+18h] [ebp-8h] BYREF
  unsigned int v11; // [esp+1Ch] [ebp-4h] BYREF

  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("DelInventoryItem:1", (char *)this + 12, *((_DWORD *)this + 259));
  v4 = cltItemList::GetItemsNum(a2);
  v5 = 0;
  v6 = v4;
  if ( v4 )
  {
    do
    {
      cltItemList::GetItem(a2, v5, &v8, v9, &v11, v10, 0);
      cltBaseInventory::DelInventoryItem(this, v10[0], v9[0], a3);
      ++v5;
    }
    while ( v5 < v6 );
  }
  return 1;
}

//----- (00560270) --------------------------------------------------------
BOOL cltBaseInventory::DelInventoryItemKind(unsigned __int16 a2, unsigned __int16 a3, struct cltItemList *a4, unsigned __int8 *a5)
{
  BOOL result; // eax
  int v7; // edi
  __int16 *v9; // eax
  __int16 *v10; // esi
  __int16 v11; // cx
  unsigned __int16 v12; // ax
  int v13; // [esp+8h] [ebp-8h] BYREF
  unsigned __int16 v14[2]; // [esp+Ch] [ebp-4h] BYREF

  *(_DWORD *)v14 = 0;
  v13 = 0;
  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("DelInventoryItem:2", (char *)this + 12, *((_DWORD *)this + 259));
  result = cltBaseInventory::GetItemSlotIndexRangeByItemKind(this, a2, (int *)v14, &v13);
  if ( !result )
    return result;
  v7 = *(_DWORD *)v14;
  if ( *(int *)v14 > v13 )
    return 1;
  while ( 1 )
  {
    v9 = (__int16 *)cltBaseInventory::GetInventoryItem(this, v7);
    v10 = v9;
    if ( !v9 )
      goto LABEL_12;
    v11 = *v9;
    if ( *v9 != a2 )
      goto LABEL_12;
    v12 = v9[1];
    if ( v12 > a3 )
      break;
    if ( a4 )
      cltItemList::AddItem((int)a4, v11, v12, 0, 0, v7, 0);
    a3 -= v10[1];
    cltBaseInventory::DelInventoryItem(this, v7, v10[1], a5);
    if ( !a3 )
      return 1;
LABEL_12:
    if ( ++v7 > v13 )
      return 1;
  }
  if ( a4 )
    cltItemList::AddItem((int)a4, *v10, a3, 0, 0, v7, 0);
  cltBaseInventory::DelInventoryItem(this, v7, a3, a5);
  return 1;
}

//----- (00560380) --------------------------------------------------------
int cltBaseInventory::DelInventoryItem(unsigned __int16 a2, unsigned __int16 a3, unsigned __int8 *a4)
{
  int result; // eax

  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("DelInventoryItem:3", (char *)this + 12, *((_DWORD *)this + 259));
  result = (int)cltBaseInventory::IsThereInventoryItem(this, a2, a3);
  if ( result )
  {
    result = (int)cltBaseInventory::GetInventoryItem(this, a2);
    if ( result )
    {
      if ( a3 )
      {
        *(_WORD *)(result + 2) -= a3;
        if ( !*(_WORD *)(result + 2) )
          cltBaseInventory::EmptyInventoryItem(this, a2);
        if ( a4 )
          a4[a2] = 1;
      }
      result = 1;
    }
  }
  return result;
}

//----- (00560410) --------------------------------------------------------
int cltBaseInventory::DelInventoryItem(int a2, unsigned __int16 a3, unsigned __int8 *a4)
{
  int v5; // edi
  int result; // eax
  unsigned __int16 *v8; // ebp
  struct stItemKindInfo *v9; // eax
  unsigned __int16 v10; // ax
  int v11; // [esp+10h] [ebp-8h] BYREF
  unsigned __int16 v12[2]; // [esp+14h] [ebp-4h] BYREF

  if ( cltBaseInventory::IsLock((cltBaseInventory *)this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("DelInventoryItem:3", (char *)(this + 12), *(_DWORD *)(this + 1036));
  *(_DWORD *)v12 = 0;
  v11 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType((cltBaseInventory *)this, 1, (int *)v12, &v11);
  v5 = *(_DWORD *)v12;
  result = v11;
  if ( *(int *)v12 <= v11 )
  {
    v8 = (unsigned __int16 *)(this + 12 * *(_DWORD *)v12 + 1058);
    do
    {
      v9 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *(v8 - 1));
      if ( v9 && *((unsigned __int8 *)v9 + 288) == a2 )
      {
        v10 = *v8;
        if ( *v8 > a3 )
          return cltBaseInventory::DelInventoryItem((cltBaseInventory *)this, v5, a3, a4);
        a3 -= v10;
        result = cltBaseInventory::DelInventoryItem((cltBaseInventory *)this, v5, v10, a4);
        if ( !a3 )
          return result;
      }
      result = v11;
      ++v5;
      v8 += 6;
    }
    while ( v5 <= v11 );
  }
  return result;
}

//----- (005604E0) --------------------------------------------------------
void cltBaseInventory::DelInventoryItem(char a2, unsigned __int8 *a3)
{
  struct strInventoryItem *v4; // eax

  v4 = cltBaseInventory::GetInventoryItem(this, (unsigned __int8)a2);
  cltBaseInventory::DelInventoryItem(this, (unsigned __int8)a2, *((_WORD *)v4 + 1), a3);
}

//----- (00560510) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::GetInventoryItem(unsigned __int16 a2)
{
  struct strInventoryItem *result; // eax

  result = (struct strInventoryItem *)cltBaseInventory::IsValidItemSlotIndex(this, a2);
  if ( result )
    result = (cltBaseInventory *)((char *)this + 12 * a2 + 1056);
  return result;
}

//----- (00560540) --------------------------------------------------------
int cltBaseInventory::EmptyInventoryItem(unsigned __int16 a2)
{
  int result; // eax
  int v4; // edx
  cltQuickSlotSystem *v5; // ecx

  if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("EmptyInventoryItem:1", (char *)this + 12, *((_DWORD *)this + 259));
  result = (int)cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    LOWORD(v4) = *(_WORD *)result;
    *(_DWORD *)result = 0;
    *(_DWORD *)(result + 4) = 0;
    *(_DWORD *)(result + 8) = 0;
    v5 = (cltQuickSlotSystem *)*((_DWORD *)this + 261);
    if ( v5 )
      cltQuickSlotSystem::OnItemOuted(v5, a2, v4);
    result = 1;
  }
  return result;
}
// 56059A: variable 'v4' is possibly undefined

//----- (005605B0) --------------------------------------------------------
void cltBaseInventory::Free()
{
  *((_DWORD *)this + 261) = 0;
  memset((char *)this + 1056, 0, 0xBF4u);
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 2) = 0;
  memset((char *)this + 12, 0, 0x400u);
}

//----- (005605E0) --------------------------------------------------------
int cltBaseInventory::GetEmptyItemSlotPos(int a2)
{
  unsigned __int16 v2; // di
  int v5; // [esp-Ch] [ebp-18h]
  int v6; // [esp+8h] [ebp-4h] BYREF

  v2 = a2;
  v5 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, a2) != (struct stItemKindInfo *)1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v5, &v6, &a2);
  return cltBaseInventory::GetEmptyItemSlotPos(this, v2, v6, a2);
}

//----- (00560640) --------------------------------------------------------
int cltBaseInventory::GetEmptyItemSlotPos(unsigned __int16 a2, int a3, int a4)
{
  int v4; // ebx
  _WORD *v5; // esi
  int result; // eax
  _WORD *i; // ecx
  cltBaseInventory *v8; // [esp+10h] [ebp-4h]

  v8 = this;
  v4 = a3;
  if ( a3 <= a4 )
  {
    v5 = (_WORD *)((char *)this + 12 * a3 + 1058);
    do
    {
      if ( *(v5 - 1) == a2 )
      {
        if ( (unsigned int)*v5 < cltBaseInventory::GetPileUpNum(this, a2) )
          return v4;
        this = v8;
      }
      ++v4;
      v5 += 6;
    }
    while ( v4 <= a4 );
  }
  result = a3;
  if ( a3 > a4 )
    return -1;
  for ( i = (_WORD *)((char *)this + 12 * a3 + 1056); *i; i += 6 )
  {
    if ( ++result > a4 )
      return -1;
  }
  return result;
}

//----- (005606C0) --------------------------------------------------------
int cltBaseInventory::GetUseItemSlotPos(int a2)
{
  __int16 v2; // di
  int result; // eax
  _WORD *i; // ecx
  int v6; // [esp-Ch] [ebp-18h]
  int v7; // [esp+8h] [ebp-4h] BYREF

  v2 = a2;
  v6 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, a2) != (struct stItemKindInfo *)1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v6, &v7, &a2);
  result = v7;
  if ( v7 > a2 )
    return -1;
  for ( i = (_WORD *)((char *)this + 12 * v7 + 1056); ; i += 6 )
  {
    if ( *i == v2 )
    {
      *i = v2;
      if ( v2 )
        break;
    }
    if ( ++result > a2 )
      return -1;
  }
  return result;
}

//----- (00560740) --------------------------------------------------------
struct stItemKindInfo *cltBaseInventory::GetUseItemSlotPos(int a2, int *a3, unsigned __int16 *a4)
{
  struct stItemKindInfo *result; // eax
  int v6; // edi
  unsigned __int16 *i; // esi
  int v8; // [esp+10h] [ebp-8h] BYREF
  int v9; // [esp+14h] [ebp-4h] BYREF

  v9 = 0;
  v8 = 0;
  *a3 = -1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, &v9, &v8);
  result = (struct stItemKindInfo *)v9;
  v6 = v9;
  if ( v9 <= v8 )
  {
    for ( i = (unsigned __int16 *)((char *)this + 12 * v9 + 1056); ; i += 6 )
    {
      result = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *i);
      if ( result )
      {
        if ( *((unsigned __int8 *)result + 288) == a2 )
        {
          *a4 = *i;
          if ( *((unsigned __int8 *)result + 35) >= i[1] + 3 )
            break;
        }
      }
      result = (struct stItemKindInfo *)v8;
      if ( ++v6 > v8 )
        return result;
    }
    *a3 = v6;
  }
  return result;
}

//----- (005607F0) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::IsThereInventoryItem(unsigned __int16 a2, unsigned __int16 a3)
{
  struct strInventoryItem *result; // eax

  result = cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    if ( *(_WORD *)result )
      result = (struct strInventoryItem *)(*((_WORD *)result + 1) >= a3);
    else
      result = 0;
  }
  return result;
}

//----- (00560820) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::IsThereInventoryItem(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4)
{
  struct strInventoryItem *result; // eax

  result = cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    if ( *(_WORD *)result == a3 )
      result = (struct strInventoryItem *)(*((_WORD *)result + 1) >= a4);
    else
      result = 0;
  }
  return result;
}

//----- (00560850) --------------------------------------------------------
int cltBaseInventory::IsThereInventoryItem(struct cltItemList *a2)
{
  cltItemList *v2; // ebx
  unsigned __int16 v4; // ax
  unsigned int v5; // esi
  signed int v6; // edi
  unsigned __int16 v8[2]; // [esp+10h] [ebp-Ch] BYREF
  unsigned __int16 v9[2]; // [esp+14h] [ebp-8h] BYREF
  unsigned int v10; // [esp+18h] [ebp-4h] BYREF

  v2 = a2;
  v4 = cltItemList::GetItemsNum(a2);
  v5 = 0;
  v6 = v4;
  if ( !v4 )
    return 1;
  while ( cltItemList::GetItem(v2, v5, v8, (unsigned __int16 *)&a2, &v10, v9, 0)
       && cltBaseInventory::IsThereInventoryItem(this, v9[0], v8[0], (unsigned __int16)a2) == (struct strInventoryItem *)1 )
  {
    if ( (int)++v5 >= v6 )
      return 1;
  }
  return 0;
}

//----- (005608E0) --------------------------------------------------------
BOOL cltBaseInventory::IsThereInventoryItemByKindNQty(int a2, unsigned __int16 a3)
{
  return (unsigned __int16)cltBaseInventory::GetInventoryItemQuantity(this, a2) >= a3;
}

//----- (00560900) --------------------------------------------------------
int cltBaseInventory::FindFirstItem(int a2)
{
  __int16 v2; // bx
  int result; // eax
  _WORD *i; // edx
  int v6; // [esp+Ch] [ebp-4h] BYREF

  v2 = a2;
  if ( !(_WORD)a2 )
    return -1;
  if ( cltBaseInventory::GetItemSlotIndexRangeByItemKind(this, a2, &a2, &v6) == -1 )
    return -1;
  result = a2;
  if ( a2 > v6 )
    return -1;
  for ( i = (_WORD *)((char *)this + 12 * a2 + 1056); !*i || *i != v2; i += 6 )
  {
    if ( ++result > v6 )
      return -1;
  }
  return result;
}

//----- (00560960) --------------------------------------------------------
unsigned __int8 cltBaseInventory::GetGenericBagNum()
{
  return *((_BYTE *)this + 1040);
}

//----- (00560970) --------------------------------------------------------
unsigned __int8 cltBaseInventory::GetPileUpNum(unsigned __int16 a2)
{
  return *((_BYTE *)cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, a2) + 35);
}

//----- (00560990) --------------------------------------------------------
__int16 cltBaseInventory::GetInventoryItemQuantity(int a2)
{
  __int16 v2; // di
  __int16 v5; // bx
  int v6; // eax
  _WORD *v7; // ecx
  int v8; // [esp+Ch] [ebp-4h] BYREF

  v2 = a2;
  if ( !(_WORD)a2 )
    return 0;
  v5 = 0;
  if ( cltBaseInventory::GetItemSlotIndexRangeByItemKind(this, a2, &a2, &v8) == -1 )
    return 0;
  if ( a2 <= v8 )
  {
    v6 = v8 - a2 + 1;
    v7 = (_WORD *)((char *)this + 12 * a2 + 1056);
    do
    {
      if ( *v7 )
      {
        if ( *v7 == v2 )
          v5 += v7[1];
      }
      v7 += 6;
      --v6;
    }
    while ( v6 );
  }
  return v5;
}

//----- (00560A10) --------------------------------------------------------
int cltBaseInventory::CanMoveItem(unsigned __int16 a2, unsigned __int16 a3)
{
  int result; // eax

  if ( cltBaseInventory::IsLock(this) == 1 )
    return 0;
  if ( a2 == a3 )
    return 0;
  result = cltBaseInventory::IsIdenticalItemTypeSlotIndex(this, a2, a3);
  if ( result )
    result = *(_WORD *)cltBaseInventory::GetInventoryItem(this, a2) != 0;
  return result;
}

//----- (00560A70) --------------------------------------------------------
int cltBaseInventory::MoveItem(unsigned __int16 a2, unsigned __int16 a3)
{
  int result; // eax
  struct strInventoryItem *v5; // edi
  struct strInventoryItem *v6; // esi
  unsigned __int16 v7; // bp
  unsigned __int16 v8; // cx
  int v9; // eax
  int v10; // ecx
  unsigned __int8 v11; // [esp+10h] [ebp-18h]
  int v12; // [esp+18h] [ebp-10h]
  int v13; // [esp+24h] [ebp-4h]

  result = cltBaseInventory::CanMoveItem(this, a2, a3);
  if ( result )
  {
    if ( cltBaseInventory::IsLock(this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
      cltBaseInventory::m_pExternCriticalErrorFuncPtr("MoveItem:1", (char *)this + 12, *((_DWORD *)this + 259));
    v5 = cltBaseInventory::GetInventoryItem(this, a2);
    v6 = cltBaseInventory::GetInventoryItem(this, a3);
    if ( *(_WORD *)v5 != *(_WORD *)v6 )
      goto LABEL_11;
    result = (int)cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *(_WORD *)v5);
    v12 = result;
    if ( !result )
      return result;
    v7 = *((_WORD *)v6 + 1);
    v11 = *(_BYTE *)(result + 35);
    if ( v11 > v7 )
    {
      v8 = *((_WORD *)v5 + 1);
      if ( v11 >= v7 + v8 )
      {
        *((_WORD *)v6 + 1) = v7 + v8;
        cltQuickSlotSystem::OnItemMerged(*((cltQuickSlotSystem **)this + 261), a2, a3);
        cltBaseInventory::EmptyInventoryItem(this, a2);
        result = 1;
      }
      else
      {
        result = 1;
        *((_WORD *)v5 + 1) = v7 + v8 - v11;
        *((_WORD *)v6 + 1) = *(unsigned __int8 *)(v12 + 35);
      }
    }
    else
    {
LABEL_11:
      v9 = *(_DWORD *)v5;
      v10 = *((_DWORD *)v5 + 1);
      v13 = *((_DWORD *)v5 + 2);
      *(_DWORD *)v5 = *(_DWORD *)v6;
      *((_DWORD *)v5 + 1) = *((_DWORD *)v6 + 1);
      *((_DWORD *)v5 + 2) = *((_DWORD *)v6 + 2);
      *(_DWORD *)v6 = v9;
      *((_DWORD *)v6 + 1) = v10;
      *((_DWORD *)v6 + 2) = v13;
      cltQuickSlotSystem::OnItemMoved(*((cltQuickSlotSystem **)this + 261), a2, a3);
      result = 1;
    }
  }
  return result;
}

//----- (00560BF0) --------------------------------------------------------
unsigned __int16 *cltBaseInventory::IsEquipTypeItem(unsigned __int16 a2)
{
  unsigned __int16 *result; // eax

  result = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
    result = (unsigned __int16 *)(cltItemKindInfo::IsEquipItem(cltBaseInventory::m_pclItemKindInfo, *result) != 0);
  return result;
}

//----- (00560C20) --------------------------------------------------------
BOOL cltBaseInventory::GetItemSlotIndexRangeByItemKind(unsigned __int16 a2, int *a3, int *a4)
{
  BOOL result; // eax
  struct stItemKindInfo *v6; // eax

  result = cltItemKindInfo::IsValidItem(cltBaseInventory::m_pclItemKindInfo, a2);
  if ( result )
  {
    v6 = cltItemKindInfo::IsFashionItem(cltBaseInventory::m_pclItemKindInfo, a2);
    cltBaseInventory::GetItemSlotIndexRangeByItemType(this, v6 == 0, a3, a4);
    result = 1;
  }
  return result;
}

//----- (00560C70) --------------------------------------------------------
void cltBaseInventory::GetItemSlotIndexRangeByItemType(int a2, int *a3, int *a4)
{
  if ( a2 == 1 )
  {
    *a3 = 0;
    if ( 24 * cltBaseInventory::GetGenericBagNum(this) - 1 <= 71 )
      *a4 = 24 * cltBaseInventory::GetGenericBagNum(this) - 1;
    else
      *a4 = 71;
  }
  else
  {
    *a3 = 168;
    *a4 = 191;
  }
}

//----- (00560CF0) --------------------------------------------------------
int cltBaseInventory::GetEmptyItemSlotNum(int a2)
{
  int v3; // edi
  int v4; // eax
  _WORD *v5; // edx
  int v7; // [esp+8h] [ebp-4h] BYREF

  v3 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, a2, &a2, &v7);
  if ( a2 <= v7 )
  {
    v4 = v7 - a2 + 1;
    v5 = (_WORD *)((char *)this + 12 * a2 + 1056);
    do
    {
      if ( !*v5 )
        ++v3;
      v5 += 6;
      --v4;
    }
    while ( v4 );
  }
  return v3;
}

//----- (00560D40) --------------------------------------------------------
BOOL cltBaseInventory::IsValidItemSlotIndex(int a2)
{
  BOOL result; // eax

  result = cltBaseInventory::IsValidItemSlotIndex(this, 1, a2);
  if ( !result )
    result = cltBaseInventory::IsValidItemSlotIndex(this, 0, a2);
  return result;
}

//----- (00560D70) --------------------------------------------------------
BOOL cltBaseInventory::IsValidItemSlotIndex(int a2, int a3)
{
  int v4; // [esp+0h] [ebp-4h] BYREF

  v4 = (int)this;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, a2 == 1, &a2, &v4);
  return a3 >= a2 && a3 <= v4;
}

//----- (00560DB0) --------------------------------------------------------
BOOL cltBaseInventory::IsIdenticalItemTypeSlotIndex(int a2, int a3)
{
  BOOL v4; // ebx
  BOOL v5; // eax
  BOOL v7; // edi
  BOOL v8; // eax

  v4 = cltBaseInventory::IsValidItemSlotIndex(this, 1, a2);
  v5 = cltBaseInventory::IsValidItemSlotIndex(this, 1, a3);
  if ( v4 && v5 )
    return 1;
  v7 = cltBaseInventory::IsValidItemSlotIndex(this, 0, a2);
  v8 = cltBaseInventory::IsValidItemSlotIndex(this, 0, a3);
  return v7 && v8;
}

//----- (00560E20) --------------------------------------------------------
void cltBaseInventory::FillOutInventoryInfo(struct CMofMsg *a2)
{
  CMofMsg *v3; // esi
  unsigned __int8 v4; // di
  char *v5; // eax
  _WORD *v6; // edx
  char *v7; // eax
  _WORD *v8; // edx
  int v9; // ebp
  char *v10; // edi
  int v11; // ebp
  char *v12; // edi
  unsigned __int8 v13[4]; // [esp+10h] [ebp-4h] BYREF

  v3 = a2;
  v4 = 0;
  CMofMsg::Put_BYTE(a2, *((_BYTE *)this + 1040));
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v13, (int *)&a2);
  if ( *(int *)v13 <= (int)a2 )
  {
    v5 = (char *)a2 - *(_DWORD *)v13 + 1;
    v6 = (_WORD *)((char *)this + 12 * *(_DWORD *)v13 + 1056);
    do
    {
      if ( *v6 )
        ++v4;
      v6 += 6;
      --v5;
    }
    while ( v5 );
  }
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 0, (int *)v13, (int *)&a2);
  if ( *(int *)v13 <= (int)a2 )
  {
    v7 = (char *)a2 - *(_DWORD *)v13 + 1;
    v8 = (_WORD *)((char *)this + 12 * *(_DWORD *)v13 + 1056);
    do
    {
      if ( *v8 )
        ++v4;
      v8 += 6;
      --v7;
    }
    while ( v7 );
  }
  CMofMsg::Put_BYTE(v3, v4);
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v13, (int *)&a2);
  v9 = *(_DWORD *)v13;
  if ( *(int *)v13 <= (int)a2 )
  {
    v10 = (char *)this + 12 * *(_DWORD *)v13 + 1056;
    do
    {
      if ( *(_WORD *)v10 )
      {
        CMofMsg::Put_BYTE(v3, v9);
        CMofMsg::Put_WORD(v3, *(_WORD *)v10);
        CMofMsg::Put_WORD(v3, *((_WORD *)v10 + 1));
        CMofMsg::Put_DWORD(v3, *((_DWORD *)v10 + 1));
        CMofMsg::Put_BYTE(v3, v10[8]);
      }
      ++v9;
      v10 += 12;
    }
    while ( v9 <= (int)a2 );
  }
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 0, (int *)v13, (int *)&a2);
  v11 = *(_DWORD *)v13;
  if ( *(int *)v13 <= (int)a2 )
  {
    v12 = (char *)this + 12 * *(_DWORD *)v13 + 1056;
    do
    {
      if ( *(_WORD *)v12 )
      {
        CMofMsg::Put_BYTE(v3, v11);
        CMofMsg::Put_WORD(v3, *(_WORD *)v12);
        CMofMsg::Put_WORD(v3, *((_WORD *)v12 + 1));
        CMofMsg::Put_DWORD(v3, *((_DWORD *)v12 + 1));
        CMofMsg::Put_BYTE(v3, v12[8]);
      }
      ++v11;
      v12 += 12;
    }
    while ( v11 <= (int)a2 );
  }
}

//----- (00560FA0) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::EquipedItem(unsigned __int16 a2, __int16 a3, int a4, int a5)
{
  struct strInventoryItem *result; // eax

  if ( cltBaseInventory::IsLock((cltBaseInventory *)this) == 1 && cltBaseInventory::m_pExternCriticalErrorFuncPtr )
    cltBaseInventory::m_pExternCriticalErrorFuncPtr("EquipedItem:1", (char *)(this + 12), *(_DWORD *)(this + 1036));
  result = cltBaseInventory::IsThereInventoryItem((cltBaseInventory *)this, a2, 1u);
  if ( result )
  {
    result = cltBaseInventory::GetInventoryItem((cltBaseInventory *)this, a2);
    if ( result )
    {
      if ( *((_WORD *)result + 1) == 1 )
      {
        if ( a3 )
        {
          *(_WORD *)result = a3;
          *((_WORD *)result + 1) = 1;
          *((_DWORD *)result + 1) = a4;
          *((_DWORD *)result + 2) = a5;
        }
        else
        {
          cltBaseInventory::EmptyInventoryItem((cltBaseInventory *)this, a2);
        }
        result = (struct strInventoryItem *)1;
      }
      else
      {
        result = 0;
      }
    }
  }
  return result;
}

//----- (00561040) --------------------------------------------------------
BOOL cltBaseInventory::CanIncreaseGenericBagNum()
{
  return *((_BYTE *)this + 1040) < 3u;
}

//----- (00561050) --------------------------------------------------------
void cltBaseInventory::IncreaseGenericBagNum()
{
  unsigned __int8 v1; // al

  v1 = *((_BYTE *)this + 1040);
  if ( v1 < 3u )
    *((_BYTE *)this + 1040) = v1 + 1;
}

//----- (00561070) --------------------------------------------------------
int cltBaseInventory::GetExpandExStorageItemQty()
{
  int v2; // ebp
  unsigned __int16 v3; // di
  unsigned __int16 *v4; // eax
  unsigned __int16 *v5; // esi
  struct stItemKindInfo *v6; // eax
  char v7; // al
  int v9; // [esp+Ch] [ebp-8h] BYREF
  unsigned __int16 v10[2]; // [esp+10h] [ebp-4h] BYREF

  v2 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v10, &v9);
  v3 = v10[0];
  if ( v10[0] > v9 )
    return 0;
  do
  {
    v4 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, v3);
    v5 = v4;
    if ( v4 )
    {
      v6 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *v4);
      if ( v6 )
      {
        v7 = *((_BYTE *)v6 + 34);
        if ( v7 == 13 || v7 == 21 )
          v2 += v5[1];
      }
    }
    ++v3;
  }
  while ( v3 <= v9 );
  return v2;
}

//----- (00561100) --------------------------------------------------------
int cltBaseInventory::DecreaseExpandExStorageItemQty()
{
  unsigned __int16 v2; // si
  unsigned __int16 *v3; // eax
  struct stItemKindInfo *v4; // eax
  char v5; // al
  int v7; // [esp+8h] [ebp-8h] BYREF
  unsigned __int16 v8[2]; // [esp+Ch] [ebp-4h] BYREF

  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v8, &v7);
  v2 = v8[0];
  if ( v8[0] > v7 )
    return -1;
  while ( 1 )
  {
    v3 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, v2);
    if ( v3 )
    {
      v4 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *v3);
      if ( v4 )
      {
        v5 = *((_BYTE *)v4 + 34);
        if ( v5 == 13 || v5 == 21 )
          break;
      }
    }
    if ( ++v2 > v7 )
      return -1;
  }
  cltBaseInventory::DelInventoryItem(this, v2, 1u, 0);
  return v2;
}

//----- (00561190) --------------------------------------------------------
unsigned int cltBaseInventory::GetTownPortalItemQty(int *a2)
{
  unsigned __int16 v3; // bx
  int v4; // esi
  unsigned __int16 *v5; // eax
  unsigned __int16 *v6; // edi
  unsigned __int16 *v7; // eax
  unsigned int result; // eax
  int v9; // [esp+10h] [ebp-10h]
  unsigned int v10; // [esp+14h] [ebp-Ch]
  int v11; // [esp+18h] [ebp-8h] BYREF
  unsigned __int16 v12[2]; // [esp+1Ch] [ebp-4h] BYREF

  v10 = 0;
  v9 = -1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v12, &v11);
  v3 = v12[0];
  v4 = v12[0];
  if ( v12[0] > v11 )
  {
    result = 0;
    *a2 = -1;
  }
  else
  {
    do
    {
      v5 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, v3);
      v6 = v5;
      if ( v5 )
      {
        v7 = (unsigned __int16 *)cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *v5);
        if ( v7 )
        {
          if ( cltItemKindInfo::IsTownPortalOrderSheet(cltBaseInventory::m_pclItemKindInfo, *v7) )
          {
            if ( v9 == -1 )
              v9 = v4;
            v10 += v6[1];
          }
        }
      }
      v4 = ++v3;
    }
    while ( v3 <= v11 );
    *a2 = v9;
    result = v10;
  }
  return result;
}

//----- (00561260) --------------------------------------------------------
unsigned int cltBaseInventory::GetPostItItemQty(int *a2)
{
  unsigned __int16 v3; // di
  int v4; // esi
  unsigned __int16 *v5; // eax
  unsigned __int16 *v6; // ebx
  unsigned int result; // eax
  int v8; // [esp+10h] [ebp-10h]
  unsigned int v9; // [esp+14h] [ebp-Ch]
  int v10; // [esp+18h] [ebp-8h] BYREF
  unsigned __int16 v11[2]; // [esp+1Ch] [ebp-4h] BYREF

  v9 = 0;
  v8 = -1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v11, &v10);
  v3 = v11[0];
  v4 = v11[0];
  if ( v11[0] > v10 )
  {
    *a2 = -1;
    result = 0;
  }
  else
  {
    do
    {
      v5 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, v3);
      v6 = v5;
      if ( v5 && cltItemKindInfo::IsPostItItem(cltBaseInventory::m_pclItemKindInfo, *v5) )
      {
        if ( v8 == -1 )
          v8 = v4;
        v9 += v6[1];
      }
      v4 = ++v3;
    }
    while ( v3 <= v10 );
    result = v9;
    *a2 = v8;
  }
  return result;
}

//----- (00561320) --------------------------------------------------------
unsigned __int16 cltBaseInventory::GetTransportItem()
{
  unsigned __int16 v2; // si
  unsigned __int16 *v3; // eax
  struct stItemKindInfo *v4; // eax
  int v6; // [esp+8h] [ebp-8h] BYREF
  unsigned __int16 v7[2]; // [esp+Ch] [ebp-4h] BYREF

  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 0, (int *)v7, &v6);
  v2 = v7[0];
  if ( v7[0] > v6 )
    return 0;
  while ( 1 )
  {
    v3 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(this, v2);
    if ( v3 )
    {
      v4 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *v3);
      if ( v4 )
      {
        if ( *((_WORD *)v4 + 77) )
          break;
      }
    }
    if ( ++v2 > v6 )
      return 0;
  }
  return *((_WORD *)v4 + 77);
}

//----- (005613A0) --------------------------------------------------------
int cltBaseInventory::GetAllItemCount(unsigned __int16 a2)
{
  int v4; // esi
  int i; // ebx
  struct strInventoryItem *v6; // eax
  int v7; // [esp+4h] [ebp-8h] BYREF
  unsigned __int16 v8[2]; // [esp+8h] [ebp-4h] BYREF

  if ( !a2 )
    return 0;
  *(_DWORD *)v8 = 0;
  v7 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v8, &v7);
  v4 = *(_DWORD *)v8;
  for ( i = 0; v4 <= v7; ++v4 )
  {
    v6 = cltBaseInventory::GetInventoryItem(this, v4);
    if ( v6 )
    {
      if ( *(_WORD *)v6 == a2 )
        i += *((unsigned __int16 *)v6 + 1);
    }
  }
  return i;
}

//----- (00561420) --------------------------------------------------------
int cltBaseInventory::GetAllItemCount(int a2)
{
  int v3; // ebx
  int v4; // edi
  unsigned __int16 *v5; // esi
  struct stItemKindInfo *v6; // eax
  int v8; // [esp+Ch] [ebp-8h] BYREF
  int v9; // [esp+10h] [ebp-4h] BYREF

  v3 = 0;
  v9 = 0;
  v8 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, &v9, &v8);
  v4 = v9;
  if ( v9 > v8 )
    return 0;
  v5 = (unsigned __int16 *)((char *)this + 12 * v9 + 1058);
  do
  {
    v6 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *(v5 - 1));
    if ( v6 )
    {
      if ( *((unsigned __int8 *)v6 + 288) == a2 )
        v3 += *v5;
    }
    ++v4;
    v5 += 6;
  }
  while ( v4 <= v8 );
  return v3;
}

//----- (005614B0) --------------------------------------------------------
int cltBaseInventory::IsLock()
{
  return *((_DWORD *)this + 2);
}

//----- (005614C0) --------------------------------------------------------
int cltBaseInventory::Lock(char *a2)
{
  int v4; // eax

  if ( cltBaseInventory::IsLock(this) == 1 )
    return 0;
  v4 = *((_DWORD *)this + 1) + 1;
  *((_DWORD *)this + 1) = v4;
  if ( !v4 )
    *((_DWORD *)this + 1) = 1;
  *((_DWORD *)this + 2) = 1;
  *((_DWORD *)this + 259) = timeGetTime();
  strcpy((char *)this + 12, a2);
  return *((_DWORD *)this + 1);
}

//----- (00561530) --------------------------------------------------------
int cltBaseInventory::Unlock(int a2)
{
  int result; // eax

  result = cltBaseInventory::IsLock(this);
  if ( result )
  {
    if ( *((_DWORD *)this + 1) == a2 )
    {
      *((_DWORD *)this + 2) = 0;
      result = 1;
    }
    else
    {
      result = 0;
    }
  }
  return result;
}

//----- (00561570) --------------------------------------------------------
int cltBaseInventory::SortInventoryItems(int a2)
{
  char *v2; // eax
  int v4; // ecx
  int v6; // edx
  char *v7; // eax
  char *v8; // ecx
  __int16 v9; // bp
  unsigned int v10; // eax
  unsigned int v11; // eax
  char *v12; // eax
  char *v13; // ecx
  int v14; // edx
  __int16 v15; // si
  int (*v16)(const void *, const void *); // [esp-4h] [ebp-1000h]
  char Base[4]; // [esp+Ch] [ebp-FF0h] BYREF
  char v18; // [esp+10h] [ebp-FECh] BYREF
  char v19[4074]; // [esp+12h] [ebp-FEAh] BYREF

  v2 = v19;
  v4 = 255;
  do
  {
    *(_DWORD *)(v2 - 6) = 0;
    *((_WORD *)v2 - 1) = 0;
    *(_WORD *)v2 = 0;
    *(_DWORD *)(v2 + 2) = 0;
    *(_DWORD *)(v2 + 6) = 0;
    v2 += 16;
    --v4;
  }
  while ( v4 );
  if ( cltBaseInventory::IsLock(this) == 1 )
    return 0;
  v6 = 0;
  v7 = &v18;
  v8 = (char *)this + 1058;
  do
  {
    v9 = *((_WORD *)v8 - 1);
    *((_DWORD *)v7 - 1) = v6;
    *(_WORD *)v7 = v9;
    *((_WORD *)v7 + 1) = *(_WORD *)v8;
    *((_DWORD *)v7 + 1) = *(_DWORD *)(v8 + 2);
    *((_DWORD *)v7 + 2) = *(_DWORD *)(v8 + 6);
    ++v6;
    v8 += 12;
    v7 += 16;
  }
  while ( v6 <= 71 );
  v10 = abs32(a2) - 1;
  if ( v10 )
  {
    v11 = v10 - 1;
    if ( v11 )
    {
      if ( v11 != 1 )
        return 0;
      if ( a2 < 0 )
        v16 = comp_arry_rare_down;
      else
        v16 = comp_arry_rare_up;
    }
    else if ( a2 < 0 )
    {
      v16 = comp_arry_kind_down;
    }
    else
    {
      v16 = comp_arry_kind_up;
    }
  }
  else if ( a2 < 0 )
  {
    v16 = comp_arry_string_down;
  }
  else
  {
    v16 = comp_arry_string_up;
  }
  _qsort(Base, 0x48u, 0x10u, v16);
  v12 = v19;
  v13 = (char *)this + 1058;
  v14 = 72;
  do
  {
    v15 = *((_WORD *)v12 - 1);
    v12 += 16;
    *((_WORD *)v13 - 1) = v15;
    *(_WORD *)v13 = *((_WORD *)v12 - 8);
    *(_DWORD *)(v13 + 2) = *(_DWORD *)(v12 - 14);
    *(_DWORD *)(v13 + 6) = *(_DWORD *)(v12 - 10);
    v13 += 12;
    --v14;
  }
  while ( v14 );
  cltQuickSlotSystem::OnItemSorted(*((cltQuickSlotSystem **)this + 261), (struct strInventoryItemForSort *)Base);
  return 1;
}

//----- (005616D0) --------------------------------------------------------
int comp_arry_kind_up(const void *a1, const void *a2)
{
  struct stItemKindInfo *v2; // esi
  struct stItemKindInfo *v3; // eax
  struct stItemKindInfo *v4; // edx
  int result; // eax

  v2 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  v4 = v3;
  if ( !*((_WORD *)a1 + 2) )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  result = *((unsigned __int16 *)v2 + 26) - *((unsigned __int16 *)v3 + 26);
  if ( !result )
  {
    result = *((_DWORD *)v2 + 16) - *((_DWORD *)v4 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 561717: conditional instruction was optimized away because of 'cx.2==0'

//----- (00561760) --------------------------------------------------------
int comp_arry_kind_down(const void *a1, const void *a2)
{
  struct stItemKindInfo *v2; // esi
  struct stItemKindInfo *v3; // eax
  struct stItemKindInfo *v4; // edx
  int result; // eax

  v2 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  v4 = v3;
  if ( !*((_WORD *)a1 + 2) )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  result = *((unsigned __int16 *)v3 + 26) - *((unsigned __int16 *)v2 + 26);
  if ( !result )
  {
    result = *((_DWORD *)v2 + 16) - *((_DWORD *)v4 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 5617A7: conditional instruction was optimized away because of 'cx.2==0'

//----- (005617F0) --------------------------------------------------------
int comp_arry_string_up(const void *a1, const void *a2)
{
  struct stItemKindInfo *v2; // edi
  struct stItemKindInfo *v3; // ebp
  int result; // eax
  const char *v5; // esi

  v2 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  if ( !*((_WORD *)a1 + 2) )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  v5 = DCTTextManager::GetText(cltBaseInventory::m_pclTextManager, *((unsigned __int16 *)v3 + 1));
  result = strcmp(DCTTextManager::GetText(cltBaseInventory::m_pclTextManager, *((unsigned __int16 *)v2 + 1)), v5);
  if ( !result )
  {
    result = *((_DWORD *)v2 + 16) - *((_DWORD *)v3 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 561839: conditional instruction was optimized away because of 'ax.2==0'

//----- (005618C0) --------------------------------------------------------
int comp_arry_string_down(const void *a1, const void *a2)
{
  struct stItemKindInfo *v2; // edi
  struct stItemKindInfo *v3; // ebp
  int result; // eax
  const char *v5; // esi

  v2 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  if ( !*((_WORD *)a1 + 2) )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  v5 = DCTTextManager::GetText(cltBaseInventory::m_pclTextManager, *((unsigned __int16 *)v2 + 1));
  result = strcmp(DCTTextManager::GetText(cltBaseInventory::m_pclTextManager, *((unsigned __int16 *)v3 + 1)), v5);
  if ( !result )
  {
    result = *((_DWORD *)v2 + 16) - *((_DWORD *)v3 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 561909: conditional instruction was optimized away because of 'ax.2==0'

//----- (00561990) --------------------------------------------------------
int comp_arry_rare_up(const void *a1, const void *a2)
{
  struct stItemKindInfo *v3; // ebp
  unsigned __int16 v4; // ax
  int result; // eax
  struct stItemKindInfo *v6; // edi
  struct stItemKindInfo *v7; // [esp+10h] [ebp+4h]

  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v7 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  v4 = *((_WORD *)a1 + 2);
  if ( !v4 )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  v6 = cltItemKindInfo::IsRareItem(cltBaseInventory::m_pclItemKindInfo, v4);
  result = cltItemKindInfo::IsRareItem(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2)) - v6;
  if ( !result )
  {
    result = *((_DWORD *)v3 + 16) - *((_DWORD *)v7 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 5619D9: conditional instruction was optimized away because of 'ax.2==0'

//----- (00561A30) --------------------------------------------------------
int comp_arry_rare_down(const void *a1, const void *a2)
{
  struct stItemKindInfo *v3; // ebp
  unsigned __int16 v4; // ax
  int result; // eax
  struct stItemKindInfo *v6; // esi
  struct stItemKindInfo *v7; // [esp+10h] [ebp+4h]

  v3 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a1 + 2));
  v7 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  v4 = *((_WORD *)a1 + 2);
  if ( !v4 )
    return *((_WORD *)a2 + 2) != 0;
  if ( !*((_WORD *)a2 + 2) )
    return -1;
  v6 = cltItemKindInfo::IsRareItem(cltBaseInventory::m_pclItemKindInfo, v4);
  result = v6 - cltItemKindInfo::IsRareItem(cltBaseInventory::m_pclItemKindInfo, *((_WORD *)a2 + 2));
  if ( !result )
  {
    result = *((_DWORD *)v3 + 16) - *((_DWORD *)v7 + 16);
    if ( !result )
      result = *(_DWORD *)a1 - *(_DWORD *)a2;
  }
  return result;
}
// 561A79: conditional instruction was optimized away because of 'ax.2==0'

//----- (00561AD0) --------------------------------------------------------
BOOL cltBaseInventory::ISGetItemID(unsigned int a2, unsigned int *a3, unsigned int *a4)
{
  unsigned int v5; // ebp
  unsigned __int16 v6; // di
  unsigned int i; // esi
  struct strInventoryItem *v8; // eax
  unsigned int v10; // [esp+10h] [ebp-Ch]
  int v11; // [esp+14h] [ebp-8h] BYREF
  unsigned __int16 v12[2]; // [esp+18h] [ebp-4h] BYREF

  v10 = 0;
  v5 = -1;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v12, &v11);
  v6 = v12[0];
  for ( i = v12[0]; v6 <= v11; i = v6 )
  {
    v8 = cltBaseInventory::GetInventoryItem(this, v6);
    if ( v8 && *(unsigned __int16 *)v8 == a2 )
    {
      if ( v5 == -1 )
        v5 = i;
      v10 += *((unsigned __int16 *)v8 + 1);
    }
    ++v6;
  }
  *a3 = v5;
  *a4 = v10;
  return v5 != -1;
}

//----- (00561B70) --------------------------------------------------------
struct cltPetInventorySystem *cltBaseInventory::CanMoveItemToPetInventory(unsigned __int8 a2, unsigned __int16 a3)
{
  struct cltPetInventorySystem *result; // eax
  cltPetInventorySystem *v5; // edi

  result = (struct cltPetInventorySystem *)cltPetSystem::GetPetID(*((cltPetSystem **)this + 263));
  if ( result )
  {
    result = cltPetSystem::GetPetInventorySystem(*((cltPetSystem **)this + 263));
    v5 = result;
    if ( result )
    {
      result = cltBaseInventory::CanDelInventoryItem(this, a2, a3);
      if ( result )
      {
        result = cltBaseInventory::GetInventoryItem(this, a2);
        if ( result )
        {
          if ( *((_DWORD *)result + 2) )
            result = 0;
          else
            result = (struct cltPetInventorySystem *)(cltPetInventorySystem::CanAddItem(v5, *(_WORD *)result, a3) == 0);
        }
      }
    }
  }
  return result;
}

//----- (00561C10) --------------------------------------------------------
void cltBaseInventory::MoveItemToPetInventory(char a2, int a3, unsigned __int8 *a4, unsigned __int8 *a5)
{
  cltPetInventorySystem *v6; // eax
  unsigned __int16 v7; // [esp-Ch] [ebp-18h]

  v7 = *(_WORD *)cltBaseInventory::GetInventoryItem(this, (unsigned __int8)a2);
  v6 = cltPetSystem::GetPetInventorySystem(*((cltPetSystem **)this + 263));
  cltPetInventorySystem::AddItem(v6, v7, a3, a5);
  cltBaseInventory::DelInventoryItem(this, (unsigned __int8)a2, a3, a4);
}

//----- (00561C60) --------------------------------------------------------
cltPetInventorySystem *cltBaseInventory::CanMoveItemFromPetInventory(unsigned __int8 a2, unsigned __int16 a3)
{
  cltPetInventorySystem *result; // eax
  cltPetInventorySystem *v5; // esi

  result = (cltPetInventorySystem *)cltPetSystem::GetPetID(*((cltPetSystem **)this + 263));
  if ( result )
  {
    result = cltPetSystem::GetPetInventorySystem(*((cltPetSystem **)this + 263));
    v5 = result;
    if ( result )
    {
      result = (cltPetInventorySystem *)cltPetInventorySystem::CanDelItemBySlot(result, a2, a3);
      if ( result )
      {
        result = cltPetInventorySystem::GetPetInventoryItem(v5, a2);
        if ( result )
          result = (cltPetInventorySystem *)(cltBaseInventory::CanAddInventoryItem(this, *(_WORD *)result, a3) == 0);
      }
    }
  }
  return result;
}

//----- (00561CE0) --------------------------------------------------------
void cltBaseInventory::MoveItemFromPetInventory(unsigned __int8 a2, unsigned __int16 a3, unsigned __int8 *a4, unsigned __int8 *a5)
{
  cltPetInventorySystem *v6; // edi
  __int16 v7[2]; // [esp+10h] [ebp-Ch] BYREF
  int v8; // [esp+14h] [ebp-8h]
  int v9; // [esp+18h] [ebp-4h]

  v6 = cltPetSystem::GetPetInventorySystem(*((cltPetSystem **)this + 263));
  v7[0] = *(_WORD *)cltPetInventorySystem::GetPetInventoryItem(v6, a2);
  v7[1] = a3;
  v8 = 0;
  v9 = 0;
  cltBaseInventory::AddInventoryItem(this, (struct strInventoryItem *)v7, a4, 0);
  cltPetInventorySystem::DelItemBySlot(v6, a2, a3, a5);
}

//----- (00561D50) --------------------------------------------------------
int cltBaseInventory::IsExistMoveServer()
{
  int v2; // esi
  unsigned __int16 *i; // edi
  struct stItemKindInfo *v4; // eax
  int v6; // [esp+Ch] [ebp-8h] BYREF
  unsigned __int16 v7[2]; // [esp+10h] [ebp-4h] BYREF

  *(_DWORD *)v7 = 0;
  v6 = 0;
  cltBaseInventory::GetItemSlotIndexRangeByItemType(this, 1, (int *)v7, &v6);
  v2 = *(_DWORD *)v7;
  if ( *(int *)v7 > v6 )
    return 0;
  for ( i = (unsigned __int16 *)((char *)this + 12 * *(_DWORD *)v7 + 1056); ; i += 6 )
  {
    if ( cltBaseInventory::GetInventoryItem(this, v2) )
    {
      v4 = cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *i);
      if ( v4 )
      {
        if ( *((_BYTE *)v4 + 289) )
          break;
      }
    }
    if ( ++v2 > v6 )
      return 0;
  }
  return 1;
}

//----- (00561DE0) --------------------------------------------------------
struct strInventoryItem *cltBaseInventory::GetItemSealed(unsigned __int16 a2)
{
  struct strInventoryItem *result; // eax

  result = cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
    result = (struct strInventoryItem *)*((_DWORD *)result + 2);
  return result;
}

//----- (00561E00) --------------------------------------------------------
int cltBaseInventory::SetItemSealed(unsigned __int16 a2, int a3, int a4)
{
  int result; // eax

  result = (int)cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    *(_DWORD *)(result + 8) = a3;
    result = 1;
  }
  return result;
}

//----- (00561E20) --------------------------------------------------------
int cltBaseInventory::IsSaveChangeCoinItem(int a2)
{
  int result; // eax
  char v4; // al
  int v5; // ecx
  int v6; // [esp+4h] [ebp-4h] BYREF

  result = (int)cltBaseInventory::GetInventoryItem(this, a2);
  if ( result )
  {
    result = (int)cltItemKindInfo::GetItemKindInfo(cltBaseInventory::m_pclItemKindInfo, *(_WORD *)result);
    if ( result )
    {
      v4 = *(_BYTE *)(result + 288);
      v5 = 0;
      if ( v4 == 2 )
      {
        v5 = 1;
      }
      else if ( v4 == 3 )
      {
        v5 = 2;
      }
      a2 = 0;
      v6 = 0;
      cltBaseInventory::GetUseItemSlotPos(this, v5, &a2, (unsigned __int16 *)&v6);
      if ( a2 != -1 || (result = cltBaseInventory::GetEmptyItemSlotNum(this, 1)) != 0 )
        result = 1;
    }
  }
  return result;
}
