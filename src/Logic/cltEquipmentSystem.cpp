#include "cltEquipmentSystem.h"

#include <cstring>

#ifndef qmemcpy
#define qmemcpy memcpy
#endif

#ifndef __int8
#define __int8 char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int64
#define __int64 long long
#endif

cltItemKindInfo *cltEquipmentSystem::m_pclItemKindInfo = nullptr;
cltClassKindInfo *cltEquipmentSystem::m_pclClassKindInfo = nullptr;
unsigned int *cltEquipmentSystem::m_dwEquipAtbForFashion = (unsigned int *)1;
unsigned int *cltEquipmentSystem::m_dwEquipAtbForBattle = (unsigned int *)0x1000;
unsigned __int16 *cltEquipmentSystem::m_wFashionFullSetEffectedKinds = (unsigned __int16 *)0x50004;

void cltEquipmentSystem::InitializeStaticVariable(struct cltItemKindInfo *a1, struct cltClassKindInfo *a2)
{
  cltEquipmentSystem::m_pclItemKindInfo = a1;
  cltEquipmentSystem::m_pclClassKindInfo = a2;
}

//----- (0056E230) --------------------------------------------------------
cltEquipmentSystem::cltEquipmentSystem()
{
  char *v2; // ecx

  v2 = (char *)this + 32;
  *((_DWORD *)this + 4) = 0;
  *(_DWORD *)v2 = 0;
  *((_DWORD *)v2 + 1) = 0;
  *((_DWORD *)v2 + 2) = 0;
  *((_DWORD *)v2 + 3) = 0;
  *((_DWORD *)v2 + 4) = 0;
  *((_WORD *)v2 + 10) = 0;
  *(_DWORD *)((char *)this + 54) = 0;
  *(_DWORD *)((char *)this + 58) = 0;
  *(_DWORD *)((char *)this + 62) = 0;
  *(_DWORD *)((char *)this + 66) = 0;
  *(_DWORD *)((char *)this + 70) = 0;
  *((_WORD *)this + 37) = 0;
  memset((char *)this + 252, 0, 0x2Cu);
  memset((char *)this + 296, 0, 0x2Cu);
  memset((char *)this + 164, 0, 0x2Cu);
  memset((char *)this + 208, 0, 0x2Cu);
}

//----- (0056E2B0) --------------------------------------------------------
void cltEquipmentSystem::Initialize(struct cltSexSystem *a2, struct cltLevelSystem *a3, struct cltClassSystem *a4, struct cltPlayerAbility *a5, struct cltBaseInventory *a6, struct cltSkillSystem *a7, struct CPlayerSpirit *a8, struct cltBasicAppearSystem *a9, unsigned __int16 *const a10, unsigned __int16 *const a11, unsigned int *const a12, unsigned int *const a13)
{
  unsigned __int8 v14; // dl

  if ( a12 )
  {
    if ( a13 )
    {
      *((_DWORD *)this + 1) = a4;
      *((_DWORD *)this + 3) = a5;
      *((_DWORD *)this + 4) = a6;
      *((_DWORD *)this + 2) = a2;
      *((_DWORD *)this + 5) = a7;
      *((_DWORD *)this + 6) = a8;
      *((_DWORD *)this + 7) = a9;
      *(_DWORD *)this = a3;
      qmemcpy((char *)this + 32, a10, 0x16u);
      qmemcpy((char *)this + 54, a11, 0x16u);
      qmemcpy((char *)this + 164, a12, 0x2Cu);
      qmemcpy((char *)this + 208, a13, 0x2Cu);
      v14 = cltLevelSystem::GetLevel(a3);
      CPlayerSpirit::UpdatePlayerSpiritInfo(*((CPlayerSpirit **)this + 6), *((_WORD *)this + 37), v14, 1);
      UpdateValidity();
    }
  }
}

//----- (0056E370) --------------------------------------------------------
void cltEquipmentSystem::Initialize(struct cltSexSystem *a2, struct cltLevelSystem *a3, struct cltClassSystem *a4, struct cltPlayerAbility *a5, struct cltBaseInventory *a6, struct cltSkillSystem *a7, struct CPlayerSpirit *a8, struct cltBasicAppearSystem *a9, const struct stEquipItemInfo *a10, const struct stEquipItemInfo *a11)
{
  char *v12; // ecx
  _DWORD *v13; // eax
  _WORD *v14; // edx
  int v15; // edi
  char *v16; // ecx
  _DWORD *v17; // eax
  _WORD *v18; // edx
  int v19; // edi
  unsigned __int8 v20; // al

  if ( a10 && a11 )
  {
    *(_DWORD *)this = a3;
    *((_DWORD *)this + 1) = a4;
    *((_DWORD *)this + 3) = a5;
    *((_DWORD *)this + 4) = a6;
    *((_DWORD *)this + 2) = a2;
    *((_DWORD *)this + 5) = a7;
    *((_DWORD *)this + 6) = a8;
    *((_DWORD *)this + 7) = a9;
    v12 = (char *)this + 252;
    v13 = (_DWORD *)((char *)a10 + 8);
    v14 = (_WORD *)((char *)this + 32);
    v15 = 11;
    do
    {
      v12 += 4;
      *v14 = *((_WORD *)v13 - 4);
      *((_DWORD *)v12 - 23) = *(v13 - 1);
      *((_DWORD *)v12 - 1) = *v13;
      ++v14;
      v13 += 3;
      --v15;
    }
    while ( v15 );
    v16 = (char *)this + 296;
    v17 = (_DWORD *)((char *)a11 + 8);
    v18 = (_WORD *)((char *)this + 54);
    v19 = 11;
    do
    {
      v16 += 4;
      *v18 = *((_WORD *)v17 - 4);
      *((_DWORD *)v16 - 23) = *(v17 - 1);
      *((_DWORD *)v16 - 1) = *v17;
      ++v18;
      v17 += 3;
      --v19;
    }
    while ( v19 );
    v20 = cltLevelSystem::GetLevel(*(cltLevelSystem **)this);
    CPlayerSpirit::UpdatePlayerSpiritInfo(*((CPlayerSpirit **)this + 6), *((_WORD *)this + 37), v20, 1);
    UpdateValidity();
  }
}

//----- (0056E450) --------------------------------------------------------
void cltEquipmentSystem::Free()
{
  char *v2; // ecx

  v2 = (char *)this + 32;
  *((_DWORD *)this + 4) = 0;
  *(_DWORD *)v2 = 0;
  *((_DWORD *)v2 + 1) = 0;
  *((_DWORD *)v2 + 2) = 0;
  *((_DWORD *)v2 + 3) = 0;
  *((_DWORD *)v2 + 4) = 0;
  *((_WORD *)v2 + 10) = 0;
  *(_DWORD *)((char *)this + 54) = 0;
  *(_DWORD *)((char *)this + 58) = 0;
  *(_DWORD *)((char *)this + 62) = 0;
  *(_DWORD *)((char *)this + 66) = 0;
  *(_DWORD *)((char *)this + 70) = 0;
  *((_WORD *)this + 37) = 0;
  memset((char *)this + 164, 0, 0x2Cu);
  *(_DWORD *)((char *)this + 54) = 0;
  *(_DWORD *)((char *)this + 58) = 0;
  *(_DWORD *)((char *)this + 62) = 0;
  *(_DWORD *)((char *)this + 66) = 0;
  *(_DWORD *)((char *)this + 70) = 0;
  *((_WORD *)this + 37) = 0;
  memset((char *)this + 252, 0, 0x2Cu);
  memset((char *)this + 296, 0, 0x2Cu);
}

//----- (0056E4D0) --------------------------------------------------------
struct strInventoryItem *cltEquipmentSystem::CanEquipItem(int a2, unsigned __int16 a3)
{
  struct strInventoryItem *result; // eax

  result = cltBaseInventory::CanDelInventoryItem(*((cltBaseInventory **)this + 4), a3, 1u);
  if ( result )
  {
    result = cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a3);
    if ( result )
      result = CanEquipItemByItemKind(a2, *(_WORD *)result);
  }
  return result;
}

//----- (0056E520) --------------------------------------------------------
unsigned int cltEquipmentSystem::CanEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4)
{
  unsigned int result; // eax
  unsigned __int16 *v6; // eax
  unsigned __int16 *v7; // esi
  struct stItemKindInfo *v8; // edi
  unsigned int v9; // esi
  unsigned int *v10; // ecx
  char v11; // al
  int v12; // edi
  unsigned __int16 **v13; // esi
  unsigned __int16 v14; // ax

  if ( a2 && a2 != 1 )
    return 1;
  if ( a3 >= 0xB )
    return 1;
  if ( !cltBaseInventory::CanDelInventoryItem(*((cltBaseInventory **)this + 4), a4, 1u) )
    return 1;
  v6 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a4);
  v7 = v6;
  if ( !v6 )
    return 1;
  v8 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v6);
  if ( !v8 )
    return 1;
  if ( !CanEquipItemByItemKind(a2, *v7) )
    return 1;
  if ( a2 )
  {
    v9 = a3;
    v10 = (&cltEquipmentSystem::m_dwEquipAtbForBattle)[a3];
  }
  else
  {
    v10 = (&cltEquipmentSystem::m_dwEquipAtbForFashion)[a3];
    v9 = a3;
  }
  v11 = *((_BYTE *)v8 + 34);
  if ( v11 == 1 || v11 == 9 || v11 == 19 )
  {
    if ( ((unsigned int)v10 & *((_DWORD *)v8 + 20)) == 0 )
      return 1;
    if ( v9 == 4 )
    {
      if ( cltItemKindInfo::IsTwoHandWeaponByItemClassType(*((_WORD *)v8 + 42))
        && GetEquipItem(1u, 5u) )
      {
        return 400;
      }
    }
    else if ( v9 == 5 )
    {
      v14 = GetEquipItem(1u, 4u);
      if ( v14 )
      {
        if ( cltItemKindInfo::IsTwoHandWeaponByItemKind(cltEquipmentSystem::m_pclItemKindInfo, v14) == (struct stItemKindInfo *)1 )
          return 401;
      }
    }
    return 0;
  }
  if ( v11 != 2 && v11 != 10 && v11 != 15 )
    return 0;
  if ( ((unsigned int)v10 & *((_DWORD *)v8 + 20)) == 0 )
    return 1;
  if ( cltItemKindInfo::IsFullSetItem(cltEquipmentSystem::m_pclItemKindInfo, *(_WORD *)v8) != (struct stItemKindInfo *)1 )
  {
    if ( IsEquipedFashionFullSet() == 1
      && IsEffectedEquipKindByFashionFullSet(v9) == 1 )
    {
      return 406;
    }
    return 0;
  }
  if ( IsEquipedFashionFullSet() )
    return 0;
  v12 = 0;
  v13 = &cltEquipmentSystem::m_wFashionFullSetEffectedKinds;
  while ( !GetEquipItem(0, *(unsigned __int16 *)v13) )
  {
    v13 = (unsigned __int16 **)((char *)v13 + 2);
    ++v12;
    if ( (int)v13 >= (int)"RANDOM" )
      return 0;
  }
  switch ( *((_WORD *)&cltEquipmentSystem::m_wFashionFullSetEffectedKinds + v12) )
  {
    case 4:
      result = 402;
      break;
    case 5:
      result = 403;
      break;
    case 6:
      result = 404;
      break;
    case 7:
      result = 405;
      break;
    default:
      return 1;
  }
  return result;
}
// 56E5D7: conditional instruction was optimized away because of '%arg_0.4==1'

//----- (0056E790) --------------------------------------------------------
struct stItemKindInfo *cltEquipmentSystem::CanEquipItemByItemKind(int a2, unsigned __int16 a3)
{
  struct stItemKindInfo *result; // eax
  struct stItemKindInfo *v5; // esi
  char v6; // al
  char v7; // al
  unsigned __int16 v8; // ax
  struct strClassKindInfo *v9; // eax

  result = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, a3);
  v5 = result;
  if ( result )
  {
    result = cltItemKindInfo::IsEquipItem(cltEquipmentSystem::m_pclItemKindInfo, a3);
    if ( result )
    {
      v6 = *((_BYTE *)v5 + 34);
      if ( v6 != 1 && v6 != 9 && v6 != 19 )
      {
        if ( v6 == 2 || v6 == 10 || v6 == 15 )
        {
          if ( !*((_DWORD *)v5 + 20) )
            return 0;
          v7 = *((_BYTE *)v5 + 84);
          if ( v7 )
          {
            if ( v7 == 70 )
            {
              result = (struct stItemKindInfo *)cltSexSystem::IsFemale(*((cltSexSystem **)this + 2));
              if ( !result )
                return result;
            }
            else if ( v7 == 77 )
            {
              result = (struct stItemKindInfo *)cltSexSystem::IsMale(*((cltSexSystem **)this + 2));
              if ( !result )
                return result;
            }
          }
        }
        return (struct stItemKindInfo *)1;
      }
      if ( *((_BYTE *)v5 + 120) > cltLevelSystem::GetLevel(*(cltLevelSystem **)this) )
        return 0;
      if ( *((_WORD *)v5 + 50) > cltPlayerAbility::GetStr(*((cltPlayerAbility **)this + 3), a2, 0) )
        return 0;
      if ( *((_WORD *)v5 + 52) > cltPlayerAbility::GetDex(*((cltPlayerAbility **)this + 3), a2, 0) )
        return 0;
      if ( *((_WORD *)v5 + 53) > cltPlayerAbility::GetInt(*((cltPlayerAbility **)this + 3), a2, 0) )
        return 0;
      if ( *((_WORD *)v5 + 51) > cltPlayerAbility::GetVit(*((cltPlayerAbility **)this + 3), a2, 0) )
        return 0;
      v8 = cltClassSystem::GetClass(*((cltClassSystem **)this + 1));
      v9 = cltClassKindInfo::GetClassKindInfo(cltEquipmentSystem::m_pclClassKindInfo, v8);
      if ( !*((_QWORD *)v5 + 14) )
        return (struct stItemKindInfo *)1;
      if ( (*((_QWORD *)v5 + 14) & *((_QWORD *)v9 + 1)) != 0i64 )
      {
LABEL_32:
        if ( v9 )
          return (struct stItemKindInfo *)1;
      }
      else
      {
        while ( 1 )
        {
          v9 = cltClassKindInfo::GetClassKindInfo(cltEquipmentSystem::m_pclClassKindInfo, *((_WORD *)v9 + 10));
          if ( !v9 )
            break;
          if ( (*((_QWORD *)v5 + 14) & *((_QWORD *)v9 + 1)) != 0i64 )
            goto LABEL_32;
        }
      }
      return 0;
    }
  }
  return result;
}

//----- (0056E920) --------------------------------------------------------
__int16 cltEquipmentSystem::EquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4)
{
  int v5; // ebp
  __int16 v6; // bx
  struct strInventoryItem *v7; // eax
  int v9; // ecx
  unsigned __int8 v10; // dl
  int v11; // [esp+Ch] [ebp-4h]

  v5 = 0;
  v6 = 0;
  v11 = 0;
  v7 = cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a4);
  if ( !v7 )
    return 0;
  if ( a2 )
  {
    if ( a2 == 1 )
    {
      v6 = *((_WORD *)this + a3 + 27);
      *((_WORD *)this + a3 + 27) = *(_WORD *)v7;
      v5 = *((_DWORD *)this + a3 + 52);
      *((_DWORD *)this + a3 + 52) = *((_DWORD *)v7 + 1);
      v11 = *((_DWORD *)this + a3 + 74);
      *((_DWORD *)this + a3 + 74) = *((_DWORD *)v7 + 2);
    }
  }
  else
  {
    v6 = *((_WORD *)this + a3 + 16);
    *((_WORD *)this + a3 + 16) = *(_WORD *)v7;
    v5 = *((_DWORD *)this + a3 + 41);
    *((_DWORD *)this + a3 + 41) = *((_DWORD *)v7 + 1);
    v9 = *((_DWORD *)this + a3 + 63);
    *((_DWORD *)this + a3 + 63) = *((_DWORD *)v7 + 2);
    v11 = v9;
    if ( *((_DWORD *)cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *(_WORD *)v7) + 20) == 1 )
      cltBasicAppearSystem::ResetHairColorKey(*((cltBasicAppearSystem **)this + 7));
  }
  cltBaseInventory::EquipedItem(*((_DWORD *)this + 4), a4, v6, v5, v11);
  v10 = cltLevelSystem::GetLevel(*(cltLevelSystem **)this);
  CPlayerSpirit::UpdatePlayerSpiritInfo(*((CPlayerSpirit **)this + 6), *((_WORD *)this + a3 + 27), v10, 1);
  UpdateValidity();
  cltSkillSystem::UpdateValidity(*((cltSkillSystem **)this + 5));
  return v6;
}

//----- (0056EA30) --------------------------------------------------------
struct strInventoryItem *cltEquipmentSystem::CanUnEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4)
{
  struct strInventoryItem *result; // eax

  if ( a2 )
  {
    if ( a2 != 1 )
      return 0;
    if ( a3 >= 0xB )
      return 0;
    if ( !*((_WORD *)this + a3 + 27) )
      return 0;
    result = (struct strInventoryItem *)cltBaseInventory::IsValidItemSlotIndex(*((cltBaseInventory **)this + 4), 1, a4);
    if ( !result )
      return result;
    if ( cltBaseInventory::CanAddInventoryItem(*((cltBaseInventory **)this + 4), a4, *((_WORD *)this + a3 + 27), 1u) )
      return 0;
    result = cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a4);
    if ( !result )
      return result;
  }
  else
  {
    if ( a3 >= 0xB
      || !*((_WORD *)this + a3 + 16)
      || !cltBaseInventory::IsValidItemSlotIndex(*((cltBaseInventory **)this + 4), 0, a4)
      || cltBaseInventory::CanAddInventoryItem(*((cltBaseInventory **)this + 4), a4, *((_WORD *)this + a3 + 16), 1u) )
    {
      return 0;
    }
    result = cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a4);
    if ( !result )
      return result;
  }
  return (struct strInventoryItem *)!*(_WORD *)result;
}

//----- (0056EB50) --------------------------------------------------------
void cltEquipmentSystem::UnEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4)
{
  unsigned __int16 v5; // cx
  int v6; // edx
  int v7; // ecx
  unsigned __int16 v8; // cx
  int v9; // edx
  int v10; // ecx
  cltBaseInventory *v11; // ecx
  unsigned __int8 v12; // al
  unsigned __int16 v13[2]; // [esp+8h] [ebp-Ch] BYREF
  int v14; // [esp+Ch] [ebp-8h]
  int v15; // [esp+10h] [ebp-4h]

  v13[0] = 0;
  v13[1] = 0;
  v14 = 0;
  v15 = 0;
  if ( a2 )
  {
    if ( a2 == 1 )
    {
      v8 = *((_WORD *)this + a3 + 27);
      *((_WORD *)this + a3 + 27) = 0;
      v9 = *((_DWORD *)this + a3 + 52);
      v13[0] = v8;
      *((_DWORD *)this + a3 + 52) = 0;
      v10 = *((_DWORD *)this + a3 + 74);
      v14 = v9;
      v15 = v10;
      *((_DWORD *)this + a3 + 74) = 0;
    }
  }
  else
  {
    v5 = *((_WORD *)this + a3 + 16);
    *((_WORD *)this + a3 + 16) = 0;
    v6 = *((_DWORD *)this + a3 + 41);
    v13[0] = v5;
    *((_DWORD *)this + a3 + 41) = 0;
    v7 = *((_DWORD *)this + a3 + 63);
    v14 = v6;
    LOWORD(v6) = v13[0];
    v15 = v7;
    *((_DWORD *)this + a3 + 63) = 0;
    if ( *((_DWORD *)cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v6) + 20) == 1 )
      cltBasicAppearSystem::ResetHairColorKey(*((cltBasicAppearSystem **)this + 7));
  }
  v11 = (cltBaseInventory *)*((_DWORD *)this + 4);
  v13[1] = 1;
  cltBaseInventory::AddInventoryItem(v11, a4, (struct strInventoryItem *)v13, 0);
  v12 = cltLevelSystem::GetLevel(*(cltLevelSystem **)this);
  CPlayerSpirit::UpdatePlayerSpiritInfo(*((CPlayerSpirit **)this + 6), v13[0], v12, 0);
  UpdateValidity();
  cltSkillSystem::UpdateValidity(*((cltSkillSystem **)this + 5));
}

//----- (0056EC60) --------------------------------------------------------
unsigned __int16 cltEquipmentSystem::GetEquipItem(unsigned int a2, unsigned int a3)
{
  if ( !a2 )
    return *((_WORD *)this + a3 + 16);
  if ( a2 == 1 )
    return *((_WORD *)this + a3 + 27);
  return 0;
}

//----- (0056EC90) --------------------------------------------------------
int cltEquipmentSystem::GetSealedStatus(int a2, int a3)
{
  if ( !a2 )
    return this[a3 + 63];
  if ( a2 == 1 )
    return this[a3 + 74];
  return 0;
}

//----- (0056ECC0) --------------------------------------------------------
unsigned int cltEquipmentSystem::GetEquipItemTime(unsigned int a2, unsigned int a3)
{
  if ( !a2 )
    return *((_DWORD *)this + a3 + 41);
  if ( a2 == 1 )
    return *((_DWORD *)this + a3 + 52);
  return 0;
}

//----- (0056ECF0) --------------------------------------------------------
unsigned __int16 *cltEquipmentSystem::GetEquipItemList(unsigned int a2)
{
  if ( !a2 )
    return (unsigned __int16 *)((char *)this + 32);
  if ( a2 == 1 )
    return (unsigned __int16 *)((char *)this + 54);
  return 0;
}

//----- (0056ED10) --------------------------------------------------------
int cltEquipmentSystem::CanUnEquipAllItem(unsigned int a2)
{
  struct _EXCEPTION_REGISTRATION_RECORD *v2; // eax
  void *v3; // esp
  int result; // eax
  __int16 *v6; // edi
  int *v7; // esi
  int v8; // ebx
  __int16 *v9; // edi
  int *v10; // esi
  int v11; // ebx
  int v12; // eax
  char v13[10012]; // [esp+0h] [ebp-2728h] BYREF
  struct _EXCEPTION_REGISTRATION_RECORD *v14; // [esp+271Ch] [ebp-Ch]
  void *v15; // [esp+2720h] [ebp-8h]
  int v16; // [esp+2724h] [ebp-4h]

  v16 = -1;
  v2 = NtCurrentTeb()->NtTib.ExceptionList;
  v15 = &loc_69499B;
  v14 = v2;
  v3 = alloca(10012);
  cltItemList::cltItemList((cltItemList *)v13);
  v16 = 0;
  if ( a2 )
  {
    if ( a2 != 1 )
    {
LABEL_3:
      v16 = -1;
      cltItemList::~cltItemList((cltItemList *)v13);
      return 0;
    }
    v9 = (__int16 *)((char *)this + 54);
    v10 = (int *)((char *)this + 208);
    v11 = 11;
    do
    {
      if ( *v9 )
        cltItemList::AddItem((int)v13, *v9, 1, *v10, 0, 0xFFFFu, 0);
      ++v10;
      ++v9;
      --v11;
    }
    while ( v11 );
  }
  else
  {
    v6 = (__int16 *)((char *)this + 32);
    v7 = (int *)((char *)this + 164);
    v8 = 11;
    do
    {
      if ( *v6 )
        cltItemList::AddItem((int)v13, *v6, 1, *v7, 0, 0xFFFFu, 0);
      ++v7;
      ++v6;
      --v8;
    }
    while ( v8 );
  }
  if ( !cltItemList::GetItemsNum((cltItemList *)v13) )
    goto LABEL_3;
  v12 = cltBaseInventory::CanAddInventoryItems(*((cltBaseInventory **)this + 4), (struct cltItemList *)v13);
  v16 = -1;
  if ( v12 )
  {
    cltItemList::~cltItemList((cltItemList *)v13);
    result = 0;
  }
  else
  {
    cltItemList::~cltItemList((cltItemList *)v13);
    result = 1;
  }
  return result;
}

//----- (0056EE60) --------------------------------------------------------
void cltEquipmentSystem::UnEquipAllItem(struct cltItemList *a2, unsigned int a3, unsigned __int8 *a4, unsigned __int8 *a5)
{
  cltBaseInventory **v5; // edi
  int v6; // ebx
  int *v7; // esi
  __int16 *v8; // edi
  int v9; // ebx
  int *v10; // esi
  __int16 *v11; // edi

  v5 = (cltBaseInventory **)this;
  if ( !a3 )
  {
    v6 = 0;
    v7 = (int *)((char *)this + 252);
    v8 = (__int16 *)((char *)this + 32);
    do
    {
      if ( *v8 )
      {
        cltItemList::AddItem((int)a2, *v8, 1, *(v7 - 22), *v7, 0xFFFFu, 0);
        *v8 = 0;
        *(v7 - 22) = 0;
        *v7 = 0;
        if ( a5 )
          a5[v6] = 1;
      }
      ++v6;
      ++v8;
      ++v7;
    }
    while ( v6 < 11 );
    cltBasicAppearSystem::ResetHairColorKey(*((cltBasicAppearSystem **)this + 7));
LABEL_15:
    v5 = (cltBaseInventory **)this;
    goto LABEL_16;
  }
  if ( a3 == 1 )
  {
    v9 = 0;
    v10 = (int *)((char *)this + 296);
    v11 = (__int16 *)((char *)this + 54);
    do
    {
      if ( *v11 )
      {
        cltItemList::AddItem((int)a2, *v11, 1, *(v10 - 22), *v10, 0xFFFFu, 0);
        *v11 = 0;
        *(v10 - 22) = 0;
        *v10 = 0;
        if ( a5 )
          a5[v9] = 1;
      }
      ++v9;
      ++v11;
      ++v10;
    }
    while ( v9 < 11 );
    CPlayerSpirit::Initialize(*((CPlayerSpirit **)this + 6));
    goto LABEL_15;
  }
LABEL_16:
  cltBaseInventory::AddInventoryItem(v5[4], a2, a4);
  ((cltEquipmentSystem *)v5)->UpdateValidity();
  cltSkillSystem::UpdateValidity(v5[5]);
}

//----- (0056EF70) --------------------------------------------------------
void cltEquipmentSystem::UnEquipAllItem(unsigned int a2, unsigned __int8 *a3)
{
  struct _EXCEPTION_REGISTRATION_RECORD *v3; // eax
  void *v4; // esp
  cltBaseInventory **v5; // edi
  int *v6; // esi
  __int16 *v7; // edi
  int v8; // ebx
  int *v9; // esi
  __int16 *v10; // edi
  int v11; // ebx
  char v13[10012]; // [esp+4h] [ebp-2728h] BYREF
  struct _EXCEPTION_REGISTRATION_RECORD *v14; // [esp+2720h] [ebp-Ch]
  void *v15; // [esp+2724h] [ebp-8h]
  int v16; // [esp+2728h] [ebp-4h]

  v16 = -1;
  v3 = NtCurrentTeb()->NtTib.ExceptionList;
  v15 = &loc_6949BB;
  v14 = v3;
  v4 = alloca(10016);
  v5 = (cltBaseInventory **)this;
  cltItemList::cltItemList((cltItemList *)v13);
  v16 = 0;
  if ( !a2 )
  {
    v6 = (int *)(v5 + 63);
    v7 = (__int16 *)(v5 + 8);
    v8 = 11;
    do
    {
      if ( *v7 )
      {
        cltItemList::AddItem((int)v13, *v7, 1, *(v6 - 22), *v6, 0xFFFFu, 0);
        *v7 = 0;
        *(v6 - 22) = 0;
        *v6 = 0;
      }
      ++v7;
      ++v6;
      --v8;
    }
    while ( v8 );
    cltBasicAppearSystem::ResetHairColorKey(*((cltBasicAppearSystem **)this + 7));
LABEL_13:
    v5 = (cltBaseInventory **)this;
    goto LABEL_14;
  }
  if ( a2 == 1 )
  {
    v9 = (int *)(v5 + 74);
    v10 = (__int16 *)v5 + 27;
    v11 = 11;
    do
    {
      if ( *v10 )
      {
        cltItemList::AddItem((int)v13, *v10, 1, *(v9 - 22), *v9, 0xFFFFu, 0);
        *v10 = 0;
        *(v9 - 22) = 0;
        *v9 = 0;
      }
      ++v10;
      ++v9;
      --v11;
    }
    while ( v11 );
    CPlayerSpirit::Initialize(*((CPlayerSpirit **)this + 6));
    goto LABEL_13;
  }
LABEL_14:
  cltBaseInventory::AddInventoryItem(v5[4], (struct cltItemList *)v13, a3);
  ((cltEquipmentSystem *)v5)->UpdateValidity();
  cltSkillSystem::UpdateValidity(v5[5]);
  v16 = -1;
  cltItemList::~cltItemList((cltItemList *)v13);
}

//----- (0056F0B0) --------------------------------------------------------
struct stItemKindInfo *cltEquipmentSystem::CanMoveEquipItem(unsigned int a2, unsigned int a3, unsigned int a4)
{
  struct stItemKindInfo *result; // eax
  unsigned int v5; // edx
  unsigned int *v6; // ebp
  unsigned int *v7; // ebx
  unsigned __int16 *v8; // esi
  unsigned __int16 *v9; // edi
  unsigned __int16 v10; // si
  unsigned __int16 v11; // di

  if ( a2 )
  {
    if ( a2 != 1 )
      return 0;
    v5 = a3;
    if ( a3 >= 0xB )
      return 0;
  }
  else
  {
    v5 = a3;
    if ( a3 >= 0xB )
      return 0;
  }
  if ( a4 >= 0xB )
    return 0;
  if ( a2 )
  {
    v6 = (&cltEquipmentSystem::m_dwEquipAtbForBattle)[v5];
    v7 = (&cltEquipmentSystem::m_dwEquipAtbForBattle)[a4];
    v8 = (unsigned __int16 *)((char *)this + 2 * v5 + 54);
    v9 = (unsigned __int16 *)((char *)this + 2 * a4 + 54);
  }
  else
  {
    v6 = (&cltEquipmentSystem::m_dwEquipAtbForFashion)[v5];
    v7 = (&cltEquipmentSystem::m_dwEquipAtbForFashion)[a4];
    v8 = (unsigned __int16 *)((char *)this + 2 * v5 + 32);
    v9 = (unsigned __int16 *)((char *)this + 2 * a4 + 32);
  }
  v10 = *v8;
  if ( !v10 )
    return 0;
  result = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v10);
  if ( result )
  {
    if ( (cltItemKindInfo::GetEquipAtb(result) & (unsigned int)v7) == 0 )
      return 0;
    v11 = *v9;
    if ( !v11 )
      goto LABEL_22;
    result = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v11);
    if ( !result )
      return result;
    if ( (cltItemKindInfo::GetEquipAtb(result) & (unsigned int)v6) != 0 )
LABEL_22:
      result = (struct stItemKindInfo *)1;
    else
      result = 0;
  }
  return result;
}
// 56F106: conditional instruction was optimized away because of '%arg_0.4==1'

//----- (0056F1D0) --------------------------------------------------------
void cltEquipmentSystem::MoveEquipItem(unsigned int a2, unsigned int a3, unsigned int a4)
{
  char *v5; // edi
  char *v6; // eax
  char *v7; // ecx
  char *v8; // edx
  unsigned int v9; // ebx
  __int16 v11; // bx
  int v12; // eax
  unsigned int v13; // [esp+1Ch] [ebp+Ch]
  unsigned int v14; // [esp+1Ch] [ebp+Ch]

  v5 = 0;
  v6 = 0;
  v7 = 0;
  v8 = 0;
  if ( a2 )
  {
    if ( a2 == 1 )
    {
      v9 = a4;
      v7 = (char *)this + 2 * a4 + 54;
      v13 = *((_DWORD *)this + a4 + 74);
      v5 = (char *)this + 4 * v9 + 208;
      v6 = (char *)this + 2 * a3 + 54;
      v8 = (char *)this + 4 * a3 + 208;
      *((_DWORD *)this + v9 + 74) = *((_DWORD *)this + a3 + 74);
      *((_DWORD *)this + a3 + 74) = v13;
    }
  }
  else
  {
    v7 = (char *)this + 2 * a4 + 32;
    v14 = *((_DWORD *)this + a4 + 63);
    v5 = (char *)this + 4 * a4 + 164;
    v6 = (char *)this + 2 * a3 + 32;
    v8 = (char *)this + 4 * a3 + 164;
    *((_DWORD *)this + a4 + 63) = *((_DWORD *)this + a3 + 63);
    *((_DWORD *)this + a3 + 63) = v14;
  }
  v11 = *(_WORD *)v6;
  *(_WORD *)v6 = *(_WORD *)v7;
  *(_WORD *)v7 = v11;
  v12 = *(_DWORD *)v8;
  *(_DWORD *)v8 = *(_DWORD *)v5;
  *(_DWORD *)v5 = v12;
  UpdateValidity();
  cltSkillSystem::UpdateValidity(*((cltSkillSystem **)this + 5));
}

//----- (0056F2B0) --------------------------------------------------------
unsigned __int16 cltEquipmentSystem::GetTotalStr()
{
  int v1; // ebp
  int i; // esi
  unsigned __int16 v4; // ax
  struct stItemKindInfo *v5; // eax
  int v6; // edx
  unsigned __int16 *v7; // esi
  _DWORD *v8; // edi
  int v9; // ebx
  struct stItemKindInfo *v10; // eax

  v1 = 0;
  for ( i = 0; i < 11; ++i )
  {
    v4 = *((_WORD *)this + i + 27);
    if ( v4 && *((_DWORD *)this + i + 30) )
    {
      v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v4);
      if ( v5 )
      {
        LOWORD(v5) = *((_WORD *)v5 + 61);
        v1 += (int)v5;
      }
      if ( i == 10 )
      {
        LOWORD(v6) = *(_WORD *)(*((_DWORD *)this + 6) + 2);
        v1 += v6;
      }
    }
  }
  v7 = (unsigned __int16 *)((char *)this + 32);
  v8 = (_DWORD *)((char *)this + 76);
  v9 = 11;
  do
  {
    if ( *v7 )
    {
      if ( *v8 )
      {
        v10 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v7);
        if ( v10 )
          LOWORD(v1) = *((_WORD *)v10 + 66) + v1;
      }
    }
    ++v8;
    ++v7;
    --v9;
  }
  while ( v9 );
  return v1;
}
// 56F2EE: variable 'v6' is possibly undefined

//----- (0056F340) --------------------------------------------------------
unsigned __int16 cltEquipmentSystem::GetTotalDex()
{
  int v1; // ebp
  int i; // esi
  unsigned __int16 v4; // ax
  struct stItemKindInfo *v5; // eax
  int v6; // edx
  unsigned __int16 *v7; // esi
  _DWORD *v8; // edi
  int v9; // ebx
  struct stItemKindInfo *v10; // eax

  v1 = 0;
  for ( i = 0; i < 11; ++i )
  {
    v4 = *((_WORD *)this + i + 27);
    if ( v4 && *((_DWORD *)this + i + 30) )
    {
      v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v4);
      if ( v5 )
      {
        LOWORD(v5) = *((_WORD *)v5 + 63);
        v1 += (int)v5;
      }
      if ( i == 10 )
      {
        LOWORD(v6) = *(_WORD *)(*((_DWORD *)this + 6) + 6);
        v1 += v6;
      }
    }
  }
  v7 = (unsigned __int16 *)((char *)this + 32);
  v8 = (_DWORD *)((char *)this + 76);
  v9 = 11;
  do
  {
    if ( *v7 )
    {
      if ( *v8 )
      {
        v10 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v7);
        if ( v10 )
          LOWORD(v1) = *((_WORD *)v10 + 68) + v1;
      }
    }
    ++v8;
    ++v7;
    --v9;
  }
  while ( v9 );
  return v1;
}
// 56F37E: variable 'v6' is possibly undefined

//----- (0056F3D0) --------------------------------------------------------
unsigned __int16 cltEquipmentSystem::GetTotalInt()
{
  int v1; // ebp
  int i; // esi
  unsigned __int16 v4; // ax
  struct stItemKindInfo *v5; // eax
  int v6; // edx
  unsigned __int16 *v7; // esi
  _DWORD *v8; // edi
  int v9; // ebx
  struct stItemKindInfo *v10; // eax

  v1 = 0;
  for ( i = 0; i < 11; ++i )
  {
    v4 = *((_WORD *)this + i + 27);
    if ( v4 && *((_DWORD *)this + i + 30) )
    {
      v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v4);
      if ( v5 )
      {
        LOWORD(v5) = *((_WORD *)v5 + 64);
        v1 += (int)v5;
      }
      if ( i == 10 )
      {
        LOWORD(v6) = *(_WORD *)(*((_DWORD *)this + 6) + 4);
        v1 += v6;
      }
    }
  }
  v7 = (unsigned __int16 *)((char *)this + 32);
  v8 = (_DWORD *)((char *)this + 76);
  v9 = 11;
  do
  {
    if ( *v7 )
    {
      if ( *v8 )
      {
        v10 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v7);
        if ( v10 )
          LOWORD(v1) = *((_WORD *)v10 + 70) + v1;
      }
    }
    ++v8;
    ++v7;
    --v9;
  }
  while ( v9 );
  return v1;
}
// 56F411: variable 'v6' is possibly undefined

//----- (0056F460) --------------------------------------------------------
unsigned __int16 cltEquipmentSystem::GetTotalVit()
{
  int v1; // ebp
  int i; // esi
  unsigned __int16 v4; // ax
  struct stItemKindInfo *v5; // eax
  int v6; // edx
  unsigned __int16 *v7; // esi
  _DWORD *v8; // edi
  int v9; // ebx
  struct stItemKindInfo *v10; // eax

  v1 = 0;
  for ( i = 0; i < 11; ++i )
  {
    v4 = *((_WORD *)this + i + 27);
    if ( v4 && *((_DWORD *)this + i + 30) )
    {
      v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v4);
      if ( v5 )
      {
        LOWORD(v5) = *((_WORD *)v5 + 62);
        v1 += (int)v5;
      }
      if ( i == 10 )
      {
        LOWORD(v6) = *(_WORD *)(*((_DWORD *)this + 6) + 8);
        v1 += v6;
      }
    }
  }
  v7 = (unsigned __int16 *)((char *)this + 32);
  v8 = (_DWORD *)((char *)this + 76);
  v9 = 11;
  do
  {
    if ( *v7 )
    {
      if ( *v8 )
      {
        v10 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v7);
        if ( v10 )
          LOWORD(v1) = *((_WORD *)v10 + 72) + v1;
      }
    }
    ++v8;
    ++v7;
    --v9;
  }
  while ( v9 );
  return v1;
}
// 56F49E: variable 'v6' is possibly undefined

//----- (0056F4F0) --------------------------------------------------------
void cltEquipmentSystem::GetAPower(unsigned __int16 *a2, unsigned __int16 *a3)
{
  unsigned __int16 *v3; // esi
  _DWORD *v4; // edi
  int v5; // ebx
  struct stItemKindInfo *v6; // eax

  *a2 = 0;
  *a3 = 0;
  v3 = (unsigned __int16 *)((char *)this + 54);
  v4 = (_DWORD *)((char *)this + 120);
  v5 = 11;
  do
  {
    if ( *v3 && *v4 )
    {
      v6 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v3);
      if ( v6 )
      {
        *a2 += *((_WORD *)v6 + 43);
        *a3 += *((_WORD *)v6 + 44);
      }
    }
    ++v4;
    ++v3;
    --v5;
  }
  while ( v5 );
}

//----- (0056F560) --------------------------------------------------------
int cltEquipmentSystem::GetAPowerAdvantage(int a2)
{
  int v3; // esi
  unsigned __int16 *v4; // edi
  char *v5; // ebp
  struct stItemKindInfo *v6; // eax
  int v7; // eax
  unsigned __int16 *v8; // edi
  char *v9; // ebx
  int v10; // ebp
  struct stItemKindInfo *v11; // eax
  int v13; // [esp+10h] [ebp-4h]

  v3 = 0;
  v4 = (unsigned __int16 *)(this + 54);
  v5 = this + 120;
  v13 = 11;
  do
  {
    if ( *v4 )
    {
      if ( *(_DWORD *)v5 )
      {
        v6 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v4);
        if ( v6 )
        {
          v3 += *((_DWORD *)v6 + 33);
          switch ( a2 )
          {
            case 0:
              v7 = *((__int16 *)v6 + 80);
              goto LABEL_10;
            case 4:
              v3 += *((__int16 *)v6 + 79);
              break;
            case 5:
              v3 += *((__int16 *)v6 + 81);
              break;
            case 6:
              v7 = *((__int16 *)v6 + 82);
LABEL_10:
              v3 += v7;
              break;
            default:
              break;
          }
        }
      }
    }
    v5 += 4;
    ++v4;
    --v13;
  }
  while ( v13 );
  v8 = (unsigned __int16 *)(this + 32);
  v9 = this + 76;
  v10 = 11;
  do
  {
    if ( *v8 )
    {
      if ( *(_DWORD *)v9 )
      {
        v11 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v8);
        if ( v11 )
          v3 += *((_DWORD *)v11 + 29);
      }
    }
    v9 += 4;
    ++v8;
    --v10;
  }
  while ( v10 );
  return v3;
}

//----- (0056F640) --------------------------------------------------------
__int16 cltEquipmentSystem::GetDPower()
{
  __int16 v1; // bp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((_WORD *)v5 + 49);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056F690) --------------------------------------------------------
int cltEquipmentSystem::GetDPowerAdvatnage(int a2)
{
  int v3; // esi
  unsigned __int16 *v4; // edi
  char *v5; // ebp
  struct stItemKindInfo *v6; // eax
  int v7; // eax
  unsigned __int16 *v8; // edi
  char *v9; // ebx
  int v10; // ebp
  struct stItemKindInfo *v11; // eax
  int v13; // [esp+10h] [ebp-4h]

  v3 = 0;
  v4 = (unsigned __int16 *)(this + 54);
  v5 = this + 120;
  v13 = 11;
  do
  {
    if ( *v4 )
    {
      if ( *(_DWORD *)v5 )
      {
        v6 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v4);
        if ( v6 )
        {
          v3 += *((_DWORD *)v6 + 35);
          switch ( a2 )
          {
            case 0:
              v7 = *((__int16 *)v6 + 84);
              goto LABEL_10;
            case 4:
              v3 += *((__int16 *)v6 + 83);
              break;
            case 5:
              v3 += *((__int16 *)v6 + 85);
              break;
            case 6:
              v7 = *((__int16 *)v6 + 86);
LABEL_10:
              v3 += v7;
              break;
            default:
              break;
          }
        }
      }
    }
    v5 += 4;
    ++v4;
    --v13;
  }
  while ( v13 );
  v8 = (unsigned __int16 *)(this + 32);
  v9 = this + 76;
  v10 = 11;
  do
  {
    if ( *v8 )
    {
      if ( *(_DWORD *)v9 )
      {
        v11 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v8);
        if ( v11 )
          v3 += *((_DWORD *)v11 + 30);
      }
    }
    v9 += 4;
    ++v8;
    --v10;
  }
  while ( v10 );
  return v3;
}

//----- (0056F770) --------------------------------------------------------
int cltEquipmentSystem::GetMaxHPAdvantage()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((__int16 *)v5 + 72);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056F7C0) --------------------------------------------------------
int cltEquipmentSystem::GetMaxManaAdvantage()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((__int16 *)v5 + 73);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056F810) --------------------------------------------------------
int cltEquipmentSystem::GetSkillAPowerAdvantage()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((unsigned __int16 *)v5 + 68);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056F860) --------------------------------------------------------
int cltEquipmentSystem::GetHitRate()
{
  int v2; // ebp
  unsigned __int16 *v3; // esi
  _DWORD *v4; // ebx
  struct stItemKindInfo *v5; // eax
  unsigned __int16 *v6; // esi
  _DWORD *v7; // edi
  int v8; // ebx
  struct stItemKindInfo *v9; // eax
  int v11; // [esp+10h] [ebp-4h]

  v11 = 0;
  v2 = 11;
  v3 = (unsigned __int16 *)((char *)this + 54);
  v4 = (_DWORD *)((char *)this + 120);
  do
  {
    if ( *v3 )
    {
      if ( *v4 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v3);
        if ( v5 )
          v11 += *((__int16 *)v5 + 76);
      }
    }
    ++v4;
    ++v3;
    --v2;
  }
  while ( v2 );
  v6 = (unsigned __int16 *)((char *)this + 32);
  v7 = (_DWORD *)((char *)this + 76);
  v8 = 11;
  do
  {
    if ( *v6 )
    {
      if ( *v7 )
      {
        v9 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v6);
        if ( v9 )
          v11 += *((_DWORD *)v9 + 31);
      }
    }
    ++v7;
    ++v6;
    --v8;
  }
  while ( v8 );
  return v11;
}

//----- (0056F900) --------------------------------------------------------
int cltEquipmentSystem::GetCriticalHitRate()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((__int16 *)v5 + 78);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056F950) --------------------------------------------------------
int cltEquipmentSystem::GetMissRate()
{
  int v2; // ebp
  unsigned __int16 *v3; // esi
  _DWORD *v4; // ebx
  struct stItemKindInfo *v5; // eax
  unsigned __int16 *v6; // esi
  _DWORD *v7; // edi
  int v8; // ebx
  struct stItemKindInfo *v9; // eax
  int v11; // [esp+10h] [ebp-4h]

  v11 = 0;
  v2 = 11;
  v3 = (unsigned __int16 *)((char *)this + 54);
  v4 = (_DWORD *)((char *)this + 120);
  do
  {
    if ( *v3 )
    {
      if ( *v4 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v3);
        if ( v5 )
          v11 += *((__int16 *)v5 + 77);
      }
    }
    ++v4;
    ++v3;
    --v2;
  }
  while ( v2 );
  v6 = (unsigned __int16 *)((char *)this + 32);
  v7 = (_DWORD *)((char *)this + 76);
  v8 = 11;
  do
  {
    if ( *v6 )
    {
      if ( *v7 )
      {
        v9 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v6);
        if ( v9 )
          v11 += *((_DWORD *)v9 + 32);
      }
    }
    ++v7;
    ++v6;
    --v8;
  }
  while ( v8 );
  return v11;
}

//----- (0056F9F0) --------------------------------------------------------
struct stItemKindInfo *cltEquipmentSystem::GetEquipablePos(unsigned __int16 a2, unsigned int *a3, unsigned int *a4, unsigned int *a5)
{
  struct stItemKindInfo *result; // eax

  result = cltBaseInventory::GetInventoryItem(*((cltBaseInventory **)this + 4), a2);
  if ( result )
    result = GetEquipablePosByItemKind(*(_WORD *)result, a3, a4, a5);
  return result;
}

//----- (0056FA30) --------------------------------------------------------
struct stItemKindInfo *cltEquipmentSystem::GetEquipablePosByItemKind(unsigned __int16 a2, unsigned int *a3, unsigned int *a4, unsigned int *a5)
{
  int v5; // esi
  struct stItemKindInfo *result; // eax
  struct stItemKindInfo *v7; // ebx
  char v8; // al
  unsigned int v9; // eax
  unsigned int **v10; // ecx
  unsigned int v11; // eax
  unsigned int **v12; // ecx

  v5 = 0;
  result = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, a2);
  v7 = result;
  if ( !result )
    return result;
  result = cltItemKindInfo::IsEquipItem(cltEquipmentSystem::m_pclItemKindInfo, *(_WORD *)result);
  if ( !result )
    return result;
  v8 = *((_BYTE *)v7 + 34);
  if ( v8 == 2 || v8 == 10 || v8 == 15 )
  {
    v11 = 0;
    v12 = &cltEquipmentSystem::m_dwEquipAtbForFashion;
    do
    {
      if ( ((unsigned int)*v12 & *((_DWORD *)v7 + 20)) != 0 )
      {
        *a3 = 0;
        if ( v5 )
        {
          if ( v5 == 1 )
          {
            *a5 = v11;
            v5 = 2;
          }
        }
        else
        {
          *a4 = v11;
          v5 = 1;
        }
      }
      ++v12;
      ++v11;
    }
    while ( (int)v12 < (int)&cltEquipmentSystem::m_dwEquipAtbForBattle );
    return (struct stItemKindInfo *)v5;
  }
  if ( v8 != 1 && v8 != 9 && v8 != 19 )
    return (struct stItemKindInfo *)v5;
  v9 = 0;
  v10 = &cltEquipmentSystem::m_dwEquipAtbForBattle;
  do
  {
    if ( ((unsigned int)*v10 & *((_DWORD *)v7 + 20)) != 0 )
    {
      *a3 = 1;
      if ( v5 )
      {
        if ( v5 == 1 )
        {
          *a5 = v9;
          v5 = 2;
        }
      }
      else
      {
        *a4 = v9;
        v5 = 1;
      }
    }
    ++v10;
    ++v9;
  }
  while ( (int)v10 < (int)&cltEquipmentSystem::m_wFashionFullSetEffectedKinds );
  return (struct stItemKindInfo *)v5;
}

//----- (0056FB30) --------------------------------------------------------
int cltEquipmentSystem::GetUnEquipablePos(unsigned int a2, unsigned int a3, unsigned __int16 *a4)
{
  int v5; // eax
  int v7; // eax

  LOWORD(v5) = GetEquipItem(a2, a3);
  if ( !(_WORD)v5 )
    return 0;
  v7 = cltBaseInventory::GetEmptyItemSlotPos(*((cltBaseInventory **)this + 4), v5);
  if ( v7 < 0 )
    return 0;
  *a4 = v7;
  return 1;
}
// 56FB53: variable 'v5' is possibly undefined

//----- (0056FB80) --------------------------------------------------------
int cltEquipmentSystem::GetExpAdvantageByEquipItem()
{
  int v1; // ebx
  unsigned __int16 *v2; // esi
  int v3; // edi
  struct stItemKindInfo *v4; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 32);
  v3 = 11;
  do
  {
    if ( *v2 )
    {
      v4 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
      if ( v4 )
        v1 += *((__int16 *)v4 + 46);
    }
    ++v2;
    --v3;
  }
  while ( v3 );
  return v1;
}

//----- (0056FBC0) --------------------------------------------------------
void cltEquipmentSystem::UpdateValidity()
{
  _DWORD *v2; // edx
  int v3; // ebp
  _WORD *v4; // eax
  int v5; // ecx
  int v6; // eax
  int *v7; // ecx
  _WORD *v8; // edx
  int v9; // eax
  int v10; // eax
  int v11; // eax
  int v12; // eax
  int v13; // esi
  int *j; // edi
  struct stItemKindInfo *v15; // eax
  int v16; // edx
  int v17; // ecx
  int v18; // edx
  int v19; // edi
  int v20; // edx
  _WORD *v21; // eax
  int v22; // ebp
  int v23; // ebp
  int v24; // [esp+10h] [ebp-40h]
  int v25; // [esp+14h] [ebp-3Ch]
  int i; // [esp+18h] [ebp-38h]
  int v27; // [esp+1Ch] [ebp-34h]
  int v28; // [esp+20h] [ebp-30h]
  int v29[11]; // [esp+24h] [ebp-2Ch] BYREF

  v2 = (_DWORD *)((char *)this + 76);
  v3 = 0;
  memset((char *)this + 76, 0, 0x2Cu);
  memset((char *)this + 120, 0, 0x2Cu);
  v4 = (_WORD *)((char *)this + 32);
  v5 = 11;
  do
  {
    if ( *v4 )
      *v2 = 1;
    ++v4;
    ++v2;
    --v5;
  }
  while ( v5 );
  v6 = 0;
  v7 = v29;
  v8 = (_WORD *)((char *)this + 54);
  do
  {
    if ( *v8 )
    {
      *v7 = v6;
      ++v3;
      ++v7;
    }
    ++v6;
    ++v8;
  }
  while ( v6 < 11 );
  v28 = v3;
  LOWORD(v9) = cltPlayerAbility::GetBaseStr(*((cltPlayerAbility **)this + 3));
  v24 = v9;
  LOWORD(v10) = cltPlayerAbility::GetBaseDex(*((cltPlayerAbility **)this + 3));
  v25 = v10;
  LOWORD(v11) = cltPlayerAbility::GetBaseInt(*((cltPlayerAbility **)this + 3));
  v27 = v11;
  LOWORD(v12) = cltPlayerAbility::GetBaseVit(*((cltPlayerAbility **)this + 3));
  for ( i = v12; v3; v28 = --v3 )
  {
    v13 = 0;
    if ( v3 <= 0 )
      break;
    for ( j = v29; ; ++j )
    {
      v15 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *((_WORD *)this + *j + 27));
      if ( v15 )
      {
        if ( *((_WORD *)v15 + 50) <= (unsigned __int16)v24
          && *((_WORD *)v15 + 52) <= (unsigned __int16)v25
          && *((_WORD *)v15 + 53) <= (unsigned __int16)v27
          && *((_WORD *)v15 + 51) <= (unsigned __int16)i )
        {
          break;
        }
      }
      if ( ++v13 >= v3 )
        return;
    }
    LOWORD(v16) = *((_WORD *)v15 + 61);
    LOWORD(v17) = *((_WORD *)v15 + 63);
    v24 += v16;
    v25 += v17;
    HIWORD(v18) = HIWORD(v25);
    LOWORD(v18) = *((_WORD *)v15 + 62);
    LOWORD(v15) = *((_WORD *)v15 + 64);
    v19 = v18 + i;
    v20 = v29[v13];
    v27 += (int)v15;
    i = v19;
    if ( v20 == 10 )
    {
      v21 = (_WORD *)*((_DWORD *)this + 6);
      LOWORD(v19) = v21[1];
      v22 = v19 + v24;
      LOWORD(v19) = v21[3];
      v24 = v22;
      v23 = v19 + v25;
      LOWORD(v19) = v21[4];
      LOWORD(v21) = v21[2];
      v25 = v23;
      i += v19;
      v3 = v28;
      v27 += (int)v21;
    }
    *((_DWORD *)this + v20 + 30) = 1;
    _memmove(&v29[v13], &v29[v13 + 1], 4 * (v3 + 0x3FFFFFFF * (v13 + 1)));
  }
}
// 56FC31: variable 'v9' is possibly undefined
// 56FC3D: variable 'v10' is possibly undefined
// 56FC49: variable 'v11' is possibly undefined
// 56FC54: variable 'v12' is possibly undefined
// 56FCCF: variable 'v16' is possibly undefined
// 56FCDD: variable 'v17' is possibly undefined

//----- (0056FD80) --------------------------------------------------------
int cltEquipmentSystem::IsEquipItemValidity(unsigned int a2, unsigned int a3)
{
  if ( !a2 )
    return *((_DWORD *)this + a3 + 19);
  if ( a2 == 1 )
    return *((_DWORD *)this + a3 + 30);
  return 0;
}

//----- (0056FDB0) --------------------------------------------------------
int cltEquipmentSystem::GetWeaponAttackAtb()
{
  unsigned __int16 v1; // ax
  struct stItemKindInfo *v2; // eax
  int result; // eax

  v1 = GetEquipItem(1u, 4u);
  v2 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, v1);
  if ( v2 )
    result = *((_DWORD *)v2 + 55);
  else
    result = 3;
  return result;
}

//----- (0056FDE0) --------------------------------------------------------
int cltEquipmentSystem::IsEquipedFashionFullSet()
{
  unsigned __int16 **v2; // esi
  unsigned __int16 v3; // ax

  v2 = &cltEquipmentSystem::m_wFashionFullSetEffectedKinds;
  while ( 1 )
  {
    v3 = *((_WORD *)this + *(unsigned __int16 *)v2 + 16);
    if ( v3 )
    {
      if ( cltItemKindInfo::IsFullSetItem(cltEquipmentSystem::m_pclItemKindInfo, v3) )
        break;
    }
    v2 = (unsigned __int16 **)((char *)v2 + 2);
    if ( (int)v2 >= (int)"RANDOM" )
      return 0;
  }
  return 1;
}

//----- (0056FE20) --------------------------------------------------------
struct stItemKindInfo *cltEquipmentSystem::IsEquipedMultiTargetWeapon()
{
  struct stItemKindInfo *result; // eax
  unsigned __int16 v3; // ax

  result = (struct stItemKindInfo *)IsEquipItemValidity(1u, 4u);
  if ( result )
  {
    v3 = GetEquipItem(1u, 4u);
    if ( v3 )
      result = cltItemKindInfo::IsMultiTargetWeapon(cltEquipmentSystem::m_pclItemKindInfo, v3);
    else
      result = 0;
  }
  return result;
}

//----- (0056FE60) --------------------------------------------------------
int cltEquipmentSystem::IsEquipedBattleItem()
{
  int v1; // eax
  _WORD *i; // ecx

  v1 = 0;
  for ( i = (_WORD *)((char *)this + 54); !*i; ++i )
  {
    if ( ++v1 >= 11 )
      return 0;
  }
  return 1;
}

//----- (0056FE80) --------------------------------------------------------
int cltEquipmentSystem::IsEffectedEquipKindByFashionFullSet(unsigned int a2)
{
  unsigned __int16 **v2; // eax

  v2 = &cltEquipmentSystem::m_wFashionFullSetEffectedKinds;
  while ( *(unsigned __int16 *)v2 != a2 )
  {
    v2 = (unsigned __int16 **)((char *)v2 + 2);
    if ( (int)v2 >= (int)"RANDOM" )
      return 0;
  }
  return 1;
}

//----- (0056FEB0) --------------------------------------------------------
int cltEquipmentSystem::GetMagicResist()
{
  unsigned __int16 v1; // bp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((_WORD *)v5 + 87);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056FF00) --------------------------------------------------------
int cltEquipmentSystem::GetAutoRecoverHPAdvantage()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((__int16 *)v5 + 74);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056FF50) --------------------------------------------------------
int cltEquipmentSystem::GetAutoRecoverManaAdavntage()
{
  int v1; // ebp
  unsigned __int16 *v2; // esi
  _DWORD *v3; // edi
  int v4; // ebx
  struct stItemKindInfo *v5; // eax

  v1 = 0;
  v2 = (unsigned __int16 *)((char *)this + 54);
  v3 = (_DWORD *)((char *)this + 120);
  v4 = 11;
  do
  {
    if ( *v2 )
    {
      if ( *v3 )
      {
        v5 = cltItemKindInfo::GetItemKindInfo(cltEquipmentSystem::m_pclItemKindInfo, *v2);
        if ( v5 )
          v1 += *((__int16 *)v5 + 75);
      }
    }
    ++v3;
    ++v2;
    --v4;
  }
  while ( v4 );
  return v1;
}

//----- (0056FFA0) --------------------------------------------------------
int cltEquipmentSystem::IsFashionAllUnequiped()
{
  int v1; // eax
  _WORD *i; // ecx

  v1 = 0;
  for ( i = (_WORD *)((char *)this + 32); !*i; ++i )
  {
    if ( ++v1 >= 11 )
      return 1;
  }
  return 0;
}

//----- (0056FFC0) --------------------------------------------------------
void cltEquipmentSystem::SetEquipItemsByChangeSex(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4)
{
  *((_WORD *)this + 20) = a2;
  *((_WORD *)this + 23) = a4;
  *((_DWORD *)this + 23) = 1;
  *((_DWORD *)this + 24) = 1;
  *((_DWORD *)this + 26) = 1;
  *((_WORD *)this + 21) = a3;
  *((_DWORD *)this + 45) = 0;
  *((_DWORD *)this + 46) = 0;
  *((_DWORD *)this + 48) = 0;
}

//----- (00570000) --------------------------------------------------------
char cltEquipmentSystem::SetItemSealed(char a2, unsigned __int16 a3, int a4)
{
  char result; // al

  result = a2;
  if ( a2 )
  {
    if ( a2 == 1 )
    {
      result = a3;
      reinterpret_cast<_DWORD *>(this)[a3 + 74] = a4;
    }
  }
  else
  {
    result = a3;
    reinterpret_cast<_DWORD *>(this)[a3 + 63] = a4;
  }
  return result;
}
