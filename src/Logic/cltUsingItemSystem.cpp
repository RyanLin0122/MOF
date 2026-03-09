#include "cltUsingItemSystem.h"

#include <cstring>

#ifndef qmemcpy
#define qmemcpy memcpy
#endif

cltItemKindInfo *cltUsingItemSystem::m_pclItemKindInfo = nullptr;
cltPandoraKindInfo *cltUsingItemSystem::m_pclPandoraKindInfo = nullptr;
cltPetKindInfo *cltUsingItemSystem::m_pclPetKindInfo = nullptr;
cltTimerManager *cltUsingItemSystem::m_pclTimerManager = nullptr;
int (*cltUsingItemSystem::m_pIsPlayerDeadFuncPtr)(void*) = nullptr;
void (*cltUsingItemSystem::m_pResurrectPlayerFuncPtr)(void*) = nullptr;
void (*cltUsingItemSystem::m_pShoutFuncPtr)(int, void*, char*) = nullptr;
void (*cltUsingItemSystem::m_pUseMapUsingItemFuncPtr)(void*, std::uint16_t) = nullptr;
int (*cltUsingItemSystem::m_pIsRelayServerConnectedFuncPtr)() = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanUsePostItItemFuncPtr)(void*, std::uint16_t, char*) = nullptr;
void (*cltUsingItemSystem::m_pPostItFuncPtr)(int, void*, char*, char*) = nullptr;
int (*cltUsingItemSystem::m_pCanUseChangeSexItemFuncPtr)(void*) = nullptr;
int (*cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pPlayerCureFuncPtr)(void*) = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanPlayerCureFuncPtr)(void*) = nullptr;
unsigned int (*cltUsingItemSystem::m_pCanChangeCoupleRingFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pChangeCoupleRingFuncPtr)(void*, std::uint16_t, int) = nullptr;
void (*cltUsingItemSystem::m_pOpenGiftBoxFuncPtr)(void*, std::uint16_t) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemInitializeFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemPollFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemCustomFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*cltUsingItemSystem::m_pExternUsingItemTimeOutFuncPtr)(unsigned int, unsigned int) = nullptr;

void cltUsingItemSystem::InitializeStaticVariable(struct cltItemKindInfo *a1, struct cltPandoraKindInfo *a2, struct cltPetKindInfo *a3, struct cltTimerManager *a4, int (*a5)(void *), void (*a6)(void *), void (*a7)(int, void *, char *), void (*a8)(void *, unsigned __int16), int (*a9)(), unsigned int (*a10)(void *, unsigned __int16, char *), void (*a11)(int, void *, char *, char *), int (*a12)(void *), int (*a13)(void *, unsigned __int16), void (*a14)(void *, unsigned __int16), unsigned int (*a15)(void *), void (*a16)(void *), unsigned int (*a17)(void *, unsigned __int16), void (*a18)(void *, unsigned __int16, int), void (*a19)(unsigned int, unsigned int), void (*a20)(unsigned int, unsigned int), void (*a21)(unsigned int, unsigned int), void (*a22)(unsigned int, unsigned int))
{
  cltUsingItemSystem::m_pclTimerManager = a4;
  cltUsingItemSystem::m_pclItemKindInfo = a1;
  cltUsingItemSystem::m_pclPandoraKindInfo = a2;
  cltUsingItemSystem::m_pclPetKindInfo = a3;
  cltUsingItemSystem::m_pIsPlayerDeadFuncPtr = a5;
  cltUsingItemSystem::m_pResurrectPlayerFuncPtr = a6;
  cltUsingItemSystem::m_pShoutFuncPtr = a7;
  cltUsingItemSystem::m_pOpenGiftBoxFuncPtr = a8;
  cltUsingItemSystem::m_pIsRelayServerConnectedFuncPtr = (int (*)(void))a9;
  cltUsingItemSystem::m_pCanUsePostItItemFuncPtr = a10;
  cltUsingItemSystem::m_pPostItFuncPtr = a11;
  cltUsingItemSystem::m_pCanUseChangeSexItemFuncPtr = a12;
  cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr = a13;
  cltUsingItemSystem::m_pUseMapUsingItemFuncPtr = a14;
  cltUsingItemSystem::m_pCanPlayerCureFuncPtr = a15;
  cltUsingItemSystem::m_pPlayerCureFuncPtr = a16;
  cltUsingItemSystem::m_pCanChangeCoupleRingFuncPtr = a17;
  cltUsingItemSystem::m_pChangeCoupleRingFuncPtr = a18;
  cltUsingItemSystem::m_pExternUsingItemInitializeFuncPtr = a19;
  cltUsingItemSystem::m_pExternUsingItemPollFuncPtr = a20;
  cltUsingItemSystem::m_pExternUsingItemCustomFuncPtr = a21;
  cltUsingItemSystem::m_pExternUsingItemTimeOutFuncPtr = a22;
}

//----- (0059CD70) --------------------------------------------------------
cltUsingItemSystem::cltUsingItemSystem()
{
  *(_DWORD *)this = 0;
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 2) = 0;
  *((_DWORD *)this + 3) = 0;
  *((_DWORD *)this + 4) = 0;
  *((_DWORD *)this + 6) = 0;
  *((_DWORD *)this + 7) = 0;
  *((_DWORD *)this + 9) = 0;
  *((_DWORD *)this + 10) = 0;
  *((_DWORD *)this + 11) = 0;
  *((_DWORD *)this + 12) = 0;
  *((_DWORD *)this + 13) = 0;
  *((_DWORD *)this + 49) = 0;
  memset((char *)this + 64, 0, 0x78u);
  *((_DWORD *)this + 46) = 0;
  *((_DWORD *)this + 47) = 0;
  *((_DWORD *)this + 48) = 0;
}

//----- (0059CDE0) --------------------------------------------------------
void cltUsingItemSystem::Initialize(void *a2, struct cltBaseInventory *a3, struct cltPlayerAbility *a4, struct cltSpecialtySystem *a5, struct cltMakingItemSystem *a6, struct cltUsingSkillSystem *a7, struct cltWorkingPassiveSkillSystem *a8, struct cltEmblemSystem *a9, struct cltSkillSystem *a10, struct cltClassSystem *a11, struct cltSexSystem *a12, struct cltEquipmentSystem *a13, struct cltBasicAppearSystem *a14, struct cltPetSystem *a15, struct cltMyItemSystem *a16, struct cltMarriageSystem *a17)
{
  *((_DWORD *)this + 15) = a2;
  *(_DWORD *)this = a3;
  *((_DWORD *)this + 1) = a4;
  *((_DWORD *)this + 2) = a5;
  *((_DWORD *)this + 3) = a6;
  *((_DWORD *)this + 4) = a7;
  *((_DWORD *)this + 5) = a8;
  *((_DWORD *)this + 6) = a9;
  *((_DWORD *)this + 7) = a10;
  *((_DWORD *)this + 8) = a11;
  *((_DWORD *)this + 9) = a12;
  *((_DWORD *)this + 10) = a13;
  *((_DWORD *)this + 11) = a14;
  *((_DWORD *)this + 12) = a15;
  *((_DWORD *)this + 13) = a16;
  *((_DWORD *)this + 14) = a17;
  memset((char *)this + 64, 0, 0x78u);
  *((_DWORD *)this + 46) = 0;
  *((_DWORD *)this + 49) = 8000;
  *((_DWORD *)this + 47) = 800;
  *((_DWORD *)this + 48) = 1600;
}

//----- (0059CE90) --------------------------------------------------------
void cltUsingItemSystem::Free()
{
  int v2; // ebp
  int v3; // eax
  unsigned int *v4; // edi

  v2 = 0;
  v3 = *((_DWORD *)this + 46);
  *((_DWORD *)this + 15) = 0;
  *(_DWORD *)this = 0;
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 2) = 0;
  *((_DWORD *)this + 3) = 0;
  *((_DWORD *)this + 4) = 0;
  *((_DWORD *)this + 5) = 0;
  *((_DWORD *)this + 6) = 0;
  *((_DWORD *)this + 7) = 0;
  *((_DWORD *)this + 8) = 0;
  *((_DWORD *)this + 9) = 0;
  *((_DWORD *)this + 10) = 0;
  *((_DWORD *)this + 11) = 0;
  *((_DWORD *)this + 12) = 0;
  *((_DWORD *)this + 13) = 0;
  *((_DWORD *)this + 14) = 0;
  if ( v3 <= 0 )
  {
    *((_DWORD *)this + 46) = 0;
  }
  else
  {
    v4 = (unsigned int *)((char *)this + 64);
    do
    {
      if ( *v4 )
      {
        cltTimerManager::ReleaseTimer(cltUsingItemSystem::m_pclTimerManager, *v4);
        *v4 = 0;
      }
      ++v2;
      v4 += 3;
    }
    while ( v2 < *((_DWORD *)this + 46) );
    *((_DWORD *)this + 46) = 0;
  }
}

//----- (0059CF10) --------------------------------------------------------
bool cltUsingItemSystem::IsUseItem(unsigned __int16 a2)
{
  unsigned __int16 *v2; // eax
  unsigned __int16 *v3; // esi
  unsigned __int16 v5; // ax

  v2 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, a2);
  v3 = v2;
  if ( !v2 )
    return 0;
  v5 = *v2;
  if ( !*v3 )
    return 0;
  if ( cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, v5) )
    return cltItemKindInfo::IsUseItem(cltUsingItemSystem::m_pclItemKindInfo, *v3) != 0;
  return 0;
}

//----- (0059CF70) --------------------------------------------------------
bool cltUsingItemSystem::IsSpecialUseItem(unsigned __int16 a2)
{
  return cltItemKindInfo::IsSpecialUseItem(cltUsingItemSystem::m_pclItemKindInfo, a2);
}

//----- (0059CF90) --------------------------------------------------------
unsigned __int8 cltUsingItemSystem::GetSpecialItemType(unsigned __int16 a2)
{
  return cltItemKindInfo::GetSpecialUseItem(cltUsingItemSystem::m_pclItemKindInfo, a2);
}

//----- (0059CFB0) --------------------------------------------------------
bool cltUsingItemSystem::CanUseReturnItem(unsigned __int16 a2)
{
  return cltMapInfo::CanUseReturnItem(a2) != 0;
}

//----- (0059CFD0) --------------------------------------------------------
bool cltUsingItemSystem::CanUseTeleportItem(unsigned __int16 a2)
{
  return cltMapInfo::CanUseTeleportItem(a2) != 0;
}

//----- (0059CFF0) --------------------------------------------------------
bool cltUsingItemSystem::CanMoveTeleportItem(unsigned __int16 a2)
{
  return cltMapInfo::CanMoveTeleportItem(a2) != 0;
}

//----- (0059D010) --------------------------------------------------------
bool cltUsingItemSystem::CanUseTeleportDragon(unsigned __int16 a2)
{
  return cltMapInfo::CanMoveTeleportDragon(a2) != 0;
}

//----- (0059D030) --------------------------------------------------------
bool cltUsingItemSystem::CanUseTakeShip(unsigned __int16 a2)
{
  return cltMapInfo::CanUserTakeShip(a2) != 0;
}

//----- (0059D050) --------------------------------------------------------
BOOL cltUsingItemSystem::CanUseTownPortalItem(unsigned __int16 a2)
{
  return cltMapInfo::CanUseReturnItem(a2) != 0;
}

//----- (0059D070) --------------------------------------------------------
int cltUsingItemSystem::CanUseItem(unsigned __int16 a2, int a3, char *a4, char *a5, struct cltPartySystem *a6, int a7, unsigned int a8, unsigned int a9, unsigned int a10, unsigned int a11)
{
  unsigned __int16 v11; // di
  struct stItemKindInfo *v13; // esi
  int result; // eax
  unsigned __int16 v15; // ax
  unsigned int v16; // esi
  unsigned __int16 v17; // ax
  int v18; // eax
  int v19; // edi
  int v20; // ebp
  unsigned __int16 v21; // ax
  int v22; // esi
  unsigned __int16 v23; // ax
  unsigned __int16 v24; // ax
  unsigned __int16 v25; // ax
  int v26; // eax
  unsigned __int16 v27; // ax
  unsigned __int16 v28; // ax
  char *v29; // [esp-4h] [ebp-14h]
  int v30; // [esp+18h] [ebp+8h]

  v11 = a2;
  v13 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, a2);
  if ( !v13 )
    return 5;
  if ( !cltItemKindInfo::IsUseItem(cltUsingItemSystem::m_pclItemKindInfo, a2) )
    return 4;
  v15 = cltPetSystem::GetOriginalPetKind(*((cltPetSystem **)this + 12));
  if ( !cltItemKindInfo::IsPetCanUseItem(v15, v13) )
    return 111;
  if ( *((_DWORD *)v13 + 36) == 1 )
    return cltUsingItemSystem::m_pIsPlayerDeadFuncPtr(*((void **)this + 15)) != 1 ? 0x65 : 0;
  if ( cltUsingItemSystem::m_pIsPlayerDeadFuncPtr
    && cltUsingItemSystem::m_pIsPlayerDeadFuncPtr(*((void **)this + 15)) == 1 )
  {
    return 115;
  }
  if ( *((_BYTE *)v13 + 34) == 6 )
    return !cltBaseInventory::CanIncreaseGenericBagNum(*(cltBaseInventory **)this) ? 4 : 0;
  if ( *((_DWORD *)v13 + 37) == 1 )
    return !cltPlayerAbility::CanResetAbility(*((cltPlayerAbility **)this + 1)) ? 0x65 : 0;
  if ( *((_DWORD *)v13 + 38) == 1 )
    return !cltSpecialtySystem::CanResetSpecialty(*((cltSpecialtySystem **)this + 2), a3) ? 0x65 : 0;
  if ( *((_DWORD *)v13 + 39) == 1 )
    return cltSpecialtySystem::CanResetCircleSpecialty(*((cltSpecialtySystem **)this + 2), a3) != 1 ? 0x65 : 0;
  if ( *((_DWORD *)v13 + 40) == 1 )
    return !cltClassSystem::CanResetClass(*((cltClassSystem **)this + 8), a7) ? 0x65 : 0;
  if ( *((_DWORD *)v13 + 42) == 1 )
  {
    if ( cltMarriageSystem::IsMarried(*((cltMarriageSystem **)this + 14)) )
      return 7000;
    if ( !cltEquipmentSystem::IsFashionAllUnequiped(*((cltEquipmentSystem **)this + 10))
      || a7 && a7 != 1 && a7 != 2
      || a8 && a8 != 1 && a8 != 2 )
    {
      return 1;
    }
    if ( cltSexSystem::IsMale(*((cltSexSystem **)this + 9)) )
    {
      if ( a9 != cltItemKindInfo::TranslateKindCode("J0626")
        && a9 != cltItemKindInfo::TranslateKindCode("J0627")
        && a9 != cltItemKindInfo::TranslateKindCode("J0628")
        && a9 != cltItemKindInfo::TranslateKindCode("J0629")
        || a10 != cltItemKindInfo::TranslateKindCode("J0876")
        && a10 != cltItemKindInfo::TranslateKindCode("J0877")
        && a10 != cltItemKindInfo::TranslateKindCode("J0878")
        && a10 != cltItemKindInfo::TranslateKindCode("J0879") )
      {
        return 1;
      }
      v16 = a11;
      if ( a11 == cltItemKindInfo::TranslateKindCode("J1251")
        || a11 == cltItemKindInfo::TranslateKindCode("J1252")
        || a11 == cltItemKindInfo::TranslateKindCode("J1253") )
      {
LABEL_61:
        if ( cltUsingItemSystem::m_pCanUseChangeSexItemFuncPtr
          && !cltUsingItemSystem::m_pCanUseChangeSexItemFuncPtr(*((void **)this + 15)) )
        {
          return 4;
        }
        return 0;
      }
      v29 = "J1254";
    }
    else
    {
      if ( a9 != cltItemKindInfo::TranslateKindCode("J0501")
        && a9 != cltItemKindInfo::TranslateKindCode("J0502")
        && a9 != cltItemKindInfo::TranslateKindCode(`string')
        && a9 != cltItemKindInfo::TranslateKindCode("J0504") )
      {
        return 1;
      }
      if ( a10 != cltItemKindInfo::TranslateKindCode("J0751")
        && a10 != cltItemKindInfo::TranslateKindCode("J0752")
        && a10 != cltItemKindInfo::TranslateKindCode("J0753")
        && a10 != cltItemKindInfo::TranslateKindCode("J0754") )
      {
        return 1;
      }
      v16 = a11;
      if ( a11 == cltItemKindInfo::TranslateKindCode("J1126")
        || a11 == cltItemKindInfo::TranslateKindCode("J1127")
        || a11 == cltItemKindInfo::TranslateKindCode("J1128") )
      {
        goto LABEL_61;
      }
      v29 = "J1129";
    }
    if ( v16 != cltItemKindInfo::TranslateKindCode(v29) )
      return 1;
    goto LABEL_61;
  }
  if ( *((_DWORD *)v13 + 44) )
  {
    if ( cltSexSystem::GetSex(*((cltSexSystem **)this + 9)) != *((_BYTE *)v13 + 172) )
      return 1;
    if ( cltBasicAppearSystem::GetHair(*((cltBasicAppearSystem **)this + 11)) == *((_BYTE *)v13 + 180) )
      return 109;
    return 0;
  }
  if ( *((_DWORD *)v13 + 46) )
  {
    if ( *((_DWORD *)v13 + 46) )
    {
      if ( cltSexSystem::GetSex(*((cltSexSystem **)this + 9)) != *((_BYTE *)v13 + 181) )
        return 1;
      if ( cltBasicAppearSystem::GetFace(*((cltBasicAppearSystem **)this + 11)) == *((_BYTE *)v13 + 188) )
        return 110;
    }
    return 0;
  }
  v17 = *((_WORD *)v13 + 69);
  if ( v17 && !cltSpecialtySystem::IsAcquiredSpecialty(*((cltSpecialtySystem **)this + 2), v17) )
    return 105;
  if ( *((_DWORD *)v13 + 23) )
  {
    if ( cltUsingItemSystem::IsAlreadyUsedItem(this, a2) == 1 )
      return 102;
    v18 = -cltPlayerAbility::CanAddBuff(*((cltPlayerAbility **)this + 1));
    LOBYTE(v18) = v18 & 0x98;
    return v18 + 104;
  }
  if ( *((_WORD *)v13 + 50) || *((_WORD *)v13 + 51) || *((_WORD *)v13 + 52) || *((_WORD *)v13 + 53) )
    return 0;
  if ( *((_WORD *)v13 + 41) || *((_WORD *)v13 + 43) )
  {
    v19 = *((_DWORD *)this + 1);
    v30 = (*(int (**)(int, struct cltPartySystem *))(*(_DWORD *)v19 + 24))(v19, a6);
    if ( (*(int (**)(int))(*(_DWORD *)v19 + 28))(v19) < v30 )
      return 0;
    v11 = a2;
  }
  if ( *((_WORD *)v13 + 42) || *((_WORD *)v13 + 44) )
  {
    v20 = cltPlayerAbility::GetMaxMP(*((cltPlayerAbility **)this + 1), (int)a6, a6);
    if ( cltPlayerAbility::GetMP(*((cltPlayerAbility **)this + 1)) < v20 )
      return 0;
  }
  v21 = *((_WORD *)v13 + 70);
  if ( v21 )
  {
    if ( !cltMakingItemSystem::IsAcquireMakingItem(*((cltMakingItemSystem **)this + 3), v21) )
      return 0;
  }
  if ( *((_WORD *)v13 + 98) )
    return 0;
  if ( *((_DWORD *)v13 + 50) )
  {
    if ( *((_WORD *)v13 + 105)
      && cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr
      && !cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr(*((void **)this + 15), v11) )
    {
      return 1;
    }
    v22 = *((_DWORD *)v13 + 50) - 1;
    if ( v22 )
    {
      if ( v22 != 1 )
        return 1;
      if ( cltUsingItemSystem::m_pIsRelayServerConnectedFuncPtr )
        return cltUsingItemSystem::m_pIsRelayServerConnectedFuncPtr() == 0;
    }
    return 0;
  }
  if ( *((_DWORD *)v13 + 51) )
  {
    if ( cltUsingItemSystem::m_pCanUsePostItItemFuncPtr )
      return cltUsingItemSystem::m_pCanUsePostItItemFuncPtr(*((void **)this + 15), v11, a5);
    return 0;
  }
  v23 = *((_WORD *)v13 + 104);
  if ( v23 )
  {
    if ( cltSkillSystem::CanAcquireSkill(*((cltSkillSystem **)this + 7), v23) == 1 )
      return 0;
  }
  if ( *((_WORD *)v13 + 105) )
  {
    if ( cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr )
      return cltUsingItemSystem::m_pCanUseMapUsingItemFuncPtr(*((void **)this + 15), v11) == 0;
    return 0;
  }
  v24 = *((_WORD *)v13 + 119);
  if ( v24 )
    return cltPetSystem::CanAddPetSkill(*((cltPetSystem **)this + 12), v24);
  if ( *((_DWORD *)v13 + 60) )
    return cltPetSystem::CanIncreasePetSatiety(*((cltPetSystem **)this + 12));
  if ( *((_DWORD *)v13 + 62) && cltPetSystem::CanIncreasePetBagNum(*((cltPetSystem **)this + 12)) )
    return 0;
  if ( *((_DWORD *)v13 + 63) )
    return cltPetSystem::CanChangePetName(*((cltPetSystem **)this + 12), a4);
  v25 = *((_WORD *)v13 + 128);
  if ( !v25
    || (result = cltPetSystem::CanDyePet(*((cltPetSystem **)this + 12), v25)) != 0 && result != 114 && result != 113 )
  {
    if ( *((_DWORD *)v13 + 67) )
    {
      if ( cltUsingItemSystem::m_pCanPlayerCureFuncPtr )
        return cltUsingItemSystem::m_pCanPlayerCureFuncPtr(*((void **)this + 15));
      return 0;
    }
    if ( *((_DWORD *)v13 + 48) )
    {
      v26 = -(*((_DWORD *)v13 + 48) != cltBasicAppearSystem::GetHairColorKey(*((cltBasicAppearSystem **)this + 11)));
      LOBYTE(v26) = v26 & 0x90;
      return v26 + 112;
    }
    v27 = *((_WORD *)v13 + 136);
    if ( v27 )
    {
      result = cltMyItemSystem::CanAddMyItem(*((cltMyItemSystem **)this + 13), v27);
      if ( !result )
        return 0;
      if ( result == 102 )
        return result;
    }
    v28 = *((_WORD *)v13 + 140);
    if ( v28 )
    {
      result = cltMarriageSystem::CanChangeCoupleRing(*((cltMarriageSystem **)this + 14), v28);
      if ( result )
        return result;
      if ( cltUsingItemSystem::m_pCanChangeCoupleRingFuncPtr )
        return cltUsingItemSystem::m_pCanChangeCoupleRingFuncPtr(*((void **)this + 15), v11);
      return 0;
    }
    if ( !*((_WORD *)v13 + 141) )
    {
      if ( cltItemKindInfo::IsCoinItem(cltUsingItemSystem::m_pclItemKindInfo, v11) != (struct stItemKindInfo *)1 )
      {
        if ( !*((_DWORD *)v13 + 71) && !*((_BYTE *)v13 + 289) )
          return 101;
        return 0;
      }
      if ( cltItemKindInfo::IsChangeCoin(cltUsingItemSystem::m_pclItemKindInfo, v11) == (struct stItemKindInfo *)1 )
        result = cltBaseInventory::GetAllItemCount(*(cltBaseInventory **)this, v11) >= 100 ? 0 : 0x77;
      else
        result = 118;
      return result;
    }
    return 0;
  }
  return result;
}
// 59D8A0: conditional instruction was optimized away because of 'eax.4==0'

//----- (0059D920) --------------------------------------------------------
void cltUsingItemSystem::UseItem(unsigned __int16 a2, unsigned int a3)
{
  struct stItemKindInfo *v4; // eax
  struct stItemKindInfo *v5; // edi
  int v6; // eax
  struct strUsingItemInfo *v7; // ebx
  unsigned int v8; // [esp-1Ch] [ebp-28h]
  void (*v9)(unsigned int, unsigned int); // [esp-8h] [ebp-14h]

  v4 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, a2);
  v5 = v4;
  if ( v4 )
  {
    v6 = *((unsigned __int8 *)v4 + 80) - 1;
    if ( v6 )
    {
      if ( v6 != 1 )
        return;
      v7 = cltUsingItemSystem::GetEmptyUsingItemInfo(this);
      v9 = (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemCustom;
      *((_DWORD *)v7 + 1) = a3;
      *((_WORD *)v7 + 4) = *(_WORD *)v5;
      v8 = 1000 * *((_DWORD *)v5 + 24);
    }
    else
    {
      v7 = cltUsingItemSystem::GetEmptyUsingItemInfo(this);
      v9 = 0;
      *((_DWORD *)v7 + 1) = a3;
      *((_WORD *)v7 + 4) = *(_WORD *)v5;
      v8 = 0;
    }
    *(_DWORD *)v7 = cltTimerManager::CreateTimer(
                      cltUsingItemSystem::m_pclTimerManager,
                      1000 * *((_DWORD *)v5 + 23),
                      (unsigned int)this,
                      v8,
                      1,
                      (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemInitialize,
                      (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemPoll,
                      (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemTimeOuted,
                      v9,
                      0);
    ++*((_DWORD *)this + 46);
  }
}

//----- (0059D9F0) --------------------------------------------------------
unsigned __int16 cltUsingItemSystem::UpdateCoinItem(int a2, int a3, unsigned __int16 a4, unsigned __int8 *a5)
{
  unsigned __int16 *v6; // eax
  unsigned __int16 *v7; // ebp
  struct stItemKindInfo *v8; // eax
  char v9; // al
  int v10; // ecx
  cltBaseInventory *v11; // ecx
  int v12; // eax
  unsigned __int16 v13; // di
  unsigned __int8 *v14; // ebx
  cltBaseInventory *v15; // ecx
  cltBaseInventory *v16; // ecx
  int v18; // [esp-10h] [ebp-28h]
  __int16 v19; // [esp+Ch] [ebp-Ch] BYREF
  unsigned __int16 v20; // [esp+Eh] [ebp-Ah]
  int v21; // [esp+10h] [ebp-8h]
  int v22; // [esp+14h] [ebp-4h]

  v6 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, a3);
  v7 = v6;
  if ( !v6 )
    return 0;
  v8 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v6);
  if ( !v8 )
    return 0;
  v9 = *((_BYTE *)v8 + 288);
  v10 = 0;
  if ( v9 == 2 )
  {
    v10 = 1;
  }
  else if ( v9 == 3 )
  {
    v10 = 2;
  }
  if ( a2 != 1 )
  {
    cltBaseInventory::DelInventoryItemKind(*(cltBaseInventory **)this, *v7, 0xAu, 0, a5);
    return 0;
  }
  v18 = v10;
  v11 = *(cltBaseInventory **)this;
  a2 = 0;
  a3 = 0;
  cltBaseInventory::GetUseItemSlotPos(v11, v18, &a2, (unsigned __int16 *)&a3);
  HIWORD(v12) = HIWORD(a2);
  v13 = a4;
  if ( a2 == -1 )
  {
    v16 = *(cltBaseInventory **)this;
    LOWORD(v12) = *v7 - 1;
    v21 = 0;
    v22 = 0;
    v14 = a5;
    a3 = v12;
    v19 = v12;
    v20 = a4;
    cltBaseInventory::AddInventoryItem(v16, (struct strInventoryItem *)&v19, a5, 0);
  }
  else
  {
    v21 = 0;
    v22 = 0;
    v14 = a5;
    v19 = a3;
    v15 = *(cltBaseInventory **)this;
    v20 = a4;
    cltBaseInventory::AddInventoryItem(v15, a2, (struct strInventoryItem *)&v19, a5);
  }
  cltBaseInventory::DelInventoryItemKind(*(cltBaseInventory **)this, *v7, 0x64u, 0, v14);
  return v13;
}

//----- (0059DB20) --------------------------------------------------------
unsigned __int16 cltUsingItemSystem::ChangeCoinItem(int a2, unsigned __int8 *a3)
{
  unsigned __int16 *v4; // eax
  unsigned int v6; // eax
  unsigned int v7; // edx
  int v8; // eax
  int v9; // ecx

  v4 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, a2);
  if ( !v4 )
    return 0;
  if ( !cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) )
    return 0;
  v6 = timeGetTime();
  _srand(v6);
  v7 = _rand() % 10000;
  LOWORD(v8) = 0;
  v9 = 0;
  if ( v7 < 0x1F40 )
  {
    v9 = 1;
    if ( v7 <= 0x320 )
      return cltUsingItemSystem::UpdateCoinItem(this, 1, a2, 3u, a3);
    v8 = (0x200000640i64 - (unsigned __int64)v7) >> 32;
  }
  return cltUsingItemSystem::UpdateCoinItem(this, v9, a2, v8, a3);
}

//----- (0059DBD0) --------------------------------------------------------
int cltUsingItemSystem::UseItem(unsigned __int16 a2, int a3, unsigned int a4, char *a5, char *a6, char *a7, __int16 a8, char a9, unsigned __int16 a10, unsigned __int16 a11, unsigned __int16 a12, unsigned __int16 *a13, int *a14, unsigned __int16 *a15, int *a16, unsigned __int16 *a17, struct cltPartySystem *a18, unsigned int Seed, int a20, unsigned __int16 *a21, unsigned __int16 *a22, int *a23, int *a24, unsigned __int8 *a25)
{
  unsigned __int16 *v26; // eax
  struct stItemKindInfo *v27; // eax
  struct stItemKindInfo *v28; // edi
  struct strUsingItemInfo *v30; // ebp
  int v31; // eax
  int v32; // ebp
  __int16 v33; // ax
  int v34; // edx
  __int16 v35; // ax
  int v36; // ebx
  unsigned int v37; // edx
  int v38; // ebx
  int v39; // edx
  __int16 v40; // ax
  __int16 v41; // ax
  int v42; // ebx
  unsigned __int16 v43; // ax
  unsigned __int16 v44; // ax
  cltBaseInventory *v45; // ecx
  unsigned __int16 v46; // cx
  struct stItemKindInfo *v47; // ebp
  int v48; // eax
  int v49; // eax
  unsigned __int16 v50; // ax
  BOOL v51; // eax
  cltSexSystem *v52; // ecx
  unsigned int v53; // eax
  char v54; // al
  unsigned __int16 v55; // ax
  int v56; // eax
  cltPetSystem *v57; // ecx
  unsigned __int16 v58; // ax
  int v59; // ebp
  int v60; // eax
  unsigned __int16 v61; // ax
  unsigned int v62; // [esp-1Ch] [ebp-44h]
  void (*v63)(unsigned int, unsigned int); // [esp-8h] [ebp-30h]
  char v64; // [esp-8h] [ebp-30h]
  char v65; // [esp-8h] [ebp-30h]
  unsigned int v66; // [esp-4h] [ebp-2Ch]
  unsigned int v67; // [esp-4h] [ebp-2Ch]
  unsigned __int16 v68[2]; // [esp+10h] [ebp-18h] BYREF
  unsigned __int16 v69[2]; // [esp+14h] [ebp-14h] BYREF
  int v70; // [esp+18h] [ebp-10h]
  __int16 v71[2]; // [esp+1Ch] [ebp-Ch] BYREF
  int v72; // [esp+20h] [ebp-8h]
  int v73; // [esp+24h] [ebp-4h]

  *(_DWORD *)v68 = 0;
  *(_DWORD *)v69 = 0;
  v70 = 0;
  if ( Seed )
    _srand(Seed);
  if ( a13 )
    *a13 = 0;
  if ( a23 )
    *a23 = 0;
  v26 = (unsigned __int16 *)cltBaseInventory::GetInventoryItem(*(cltBaseInventory **)this, a2);
  v27 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v26);
  v28 = v27;
  if ( !v27 )
    return 0;
  if ( *((_BYTE *)v27 + 34) == 23 )
    return 1;
  if ( *((_BYTE *)v27 + 289) && a25 )
    return 1;
  cltBaseInventory::DelInventoryItem(*(cltBaseInventory **)this, a2, 1u, 0);
  if ( *((_BYTE *)v28 + 80) )
  {
    if ( *((_BYTE *)v28 + 80) == 1 )
    {
      v30 = cltUsingItemSystem::GetEmptyUsingItemInfo(this);
      v63 = 0;
      *((_DWORD *)v30 + 1) = a4;
      *((_WORD *)v30 + 4) = *(_WORD *)v28;
      v62 = 0;
      goto LABEL_18;
    }
    if ( *((_BYTE *)v28 + 80) == 2 )
    {
      v30 = cltUsingItemSystem::GetEmptyUsingItemInfo(this);
      v63 = (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemCustom;
      *((_DWORD *)v30 + 1) = a4;
      *((_WORD *)v30 + 4) = *(_WORD *)v28;
      v62 = 1000 * *((_DWORD *)v28 + 24);
LABEL_18:
      *(_DWORD *)v30 = cltTimerManager::CreateTimer(
                         cltUsingItemSystem::m_pclTimerManager,
                         1000 * *((_DWORD *)v28 + 23),
                         (unsigned int)this,
                         v62,
                         1,
                         (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemInitialize,
                         (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemPoll,
                         (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemTimeOuted,
                         v63,
                         0);
      ++*((_DWORD *)this + 46);
      return v70;
    }
  }
  else
  {
    if ( *((_DWORD *)v28 + 37) == 1 )
      cltPlayerAbility::ResetAbility(*((cltPlayerAbility **)this + 1));
    v31 = *((_DWORD *)v28 + 38);
    v32 = (int)a15;
    if ( v31 == 1 )
      cltSpecialtySystem::ResetSpecialty(*((cltSpecialtySystem **)this + 2), a3, a14, a15, a16, a17);
    if ( *((_DWORD *)v28 + 39) == 1 )
      cltSpecialtySystem::ResetCircleSpecialty(*((cltSpecialtySystem **)this + 2), a14, a15, a16, a17);
    if ( *((_DWORD *)v28 + 40) == 1 )
      cltClassSystem::ResetClass(*((cltClassSystem **)this + 8), a8);
    v33 = *((_WORD *)v28 + 41);
    if ( v33 )
    {
      v32 = *((__int16 *)v28 + 41)
          * cltPlayerAbility::GetItemRecoverHPAdvantage(*((cltPlayerAbility **)this + 1))
          / 1000
          + v33;
      v34 = **((_DWORD **)this + 1);
      if ( v32 <= 0 )
      {
        v32 = (unsigned __int16)-(__int16)v32;
        (*(void (__stdcall **)(int))(v34 + 44))(v32);
      }
      else
      {
        (*(void (__stdcall **)(int, struct cltPartySystem *))(v34 + 40))(v32, a18);
      }
      cltEmblemSystem::OnEvent_UsedRecoverHPItem(*((cltEmblemSystem **)this + 6));
    }
    v35 = *((_WORD *)v28 + 43);
    if ( v35 )
    {
      v36 = v35;
      v37 = (int)((unsigned __int64)(274877907i64
                                   * *((__int16 *)v28 + 43)
                                   * cltPlayerAbility::GetItemRecoverHPAdvantage(*((cltPlayerAbility **)this + 1))) >> 32) >> 6;
      v38 = (v37 >> 31) + v37 + v36;
      v39 = **((_DWORD **)this + 1);
      if ( v38 <= 0 )
      {
        LOBYTE(v38) = -(char)v38;
        (*(void (__stdcall **)(int, struct cltPartySystem *))(v39 + 52))(v38, a18);
      }
      else
      {
        (*(void (__stdcall **)(int, struct cltPartySystem *))(v39 + 48))(v38, a18);
      }
      cltEmblemSystem::OnEvent_UsedRecoverHPItem(*((cltEmblemSystem **)this + 6));
    }
    v40 = *((_WORD *)v28 + 42);
    if ( v40 )
    {
      v32 = *((__int16 *)v28 + 42)
          * cltPlayerAbility::GetItemRecoverManaAdvantage(*((cltPlayerAbility **)this + 1))
          / 1000
          + v40;
      if ( v32 <= 0 )
      {
        v32 = (unsigned __int16)-(__int16)v32;
        cltPlayerAbility::DecreaseMP(*((cltPlayerAbility **)this + 1), v32);
      }
      else
      {
        cltPlayerAbility::IncreaseMP(*((cltPlayerAbility **)this + 1), v32, v32, a18);
      }
      cltEmblemSystem::OnEvent_UsedRecoverManaItem(*((cltEmblemSystem **)this + 6));
    }
    v41 = *((_WORD *)v28 + 44);
    if ( v41 )
    {
      v42 = *((__int16 *)v28 + 44)
          * cltPlayerAbility::GetItemRecoverManaAdvantage(*((cltPlayerAbility **)this + 1))
          / 1000
          + v41;
      if ( v42 <= 0 )
        cltPlayerAbility::DecreaseMPPercent(*((cltPlayerAbility **)this + 1), v32, -(char)v42, a18);
      else
        cltPlayerAbility::IncreaseMPPercent(*((cltPlayerAbility **)this + 1), v32, v42, a18);
      cltEmblemSystem::OnEvent_UsedRecoverManaItem(*((cltEmblemSystem **)this + 6));
    }
    LOWORD(v31) = *((_WORD *)v28 + 50);
    if ( (_WORD)v31 )
      cltPlayerAbility::IncreaseStr(*((cltPlayerAbility **)this + 1), v31, 0);
    LOWORD(v31) = *((_WORD *)v28 + 51);
    if ( (_WORD)v31 )
      cltPlayerAbility::IncreaseDex(*((cltPlayerAbility **)this + 1), v31, 0);
    LOWORD(v31) = *((_WORD *)v28 + 52);
    if ( (_WORD)v31 )
      cltPlayerAbility::IncreaseInt(*((cltPlayerAbility **)this + 1), v31, 0);
    LOWORD(v31) = *((_WORD *)v28 + 53);
    if ( (_WORD)v31 )
      cltPlayerAbility::IncreaseVit(*((cltPlayerAbility **)this + 1), v31, 0);
    if ( *((_BYTE *)v28 + 34) == 6 )
      cltBaseInventory::IncreaseGenericBagNum(*(cltBaseInventory **)this);
    v43 = *((_WORD *)v28 + 70);
    if ( v43 )
    {
      cltMakingItemSystem::AcquireMakingItem(*((cltMakingItemSystem **)this + 3), v43);
      if ( a13 )
        *a13 = *((_WORD *)v28 + 70);
    }
    v44 = *((_WORD *)v28 + 98);
    if ( v44 && cltPandoraKindInfo::GenerateItem(cltUsingItemSystem::m_pclPandoraKindInfo, v44, v68, v69) )
    {
      if ( cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, v68[0])
        && !cltItemKindInfo::IsFashionItem(cltUsingItemSystem::m_pclItemKindInfo, v68[0]) )
      {
        v71[0] = v68[0];
        v45 = *(cltBaseInventory **)this;
        v71[1] = v69[0];
        v72 = 0;
        v73 = 0;
        cltBaseInventory::AddInventoryItem(v45, a2, (struct strInventoryItem *)v71, 0);
        v70 = 1;
      }
      v46 = v68[0];
      if ( a21 )
        *a21 = v68[0];
      if ( a22 )
        *a22 = v69[0];
      v47 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, v46);
      if ( v47
        && cltItemKindInfo::IsFashionItem(cltUsingItemSystem::m_pclItemKindInfo, v68[0])
        && (*((_WORD *)v47 + 77) || *((_DWORD *)v47 + 20) == 1)
        && cltUsingItemSystem::m_pOpenGiftBoxFuncPtr )
      {
        cltUsingItemSystem::m_pOpenGiftBoxFuncPtr(*((void **)this + 15), v68[0]);
      }
    }
    v48 = *((_DWORD *)v28 + 50);
    if ( v48 && cltUsingItemSystem::m_pShoutFuncPtr )
      cltUsingItemSystem::m_pShoutFuncPtr(v48, *((void **)this + 15), a5);
    v49 = *((_DWORD *)v28 + 51);
    if ( v49 && cltUsingItemSystem::m_pPostItFuncPtr )
      cltUsingItemSystem::m_pPostItFuncPtr(v49, *((void **)this + 15), a6, a7);
    if ( *((_DWORD *)v28 + 36) == 1 )
      cltUsingItemSystem::m_pResurrectPlayerFuncPtr(*((void **)this + 15));
    v50 = *((_WORD *)v28 + 104);
    if ( v50 )
      cltSkillSystem::AddSkill(*((cltSkillSystem **)this + 7), v50, a16, a17);
    if ( *((_DWORD *)v28 + 42) == 1 )
    {
      v51 = cltSexSystem::IsMale(*((cltSexSystem **)this + 9));
      v52 = (cltSexSystem *)*((_DWORD *)this + 9);
      if ( v51 )
        cltSexSystem::SetSex_female(v52);
      else
        cltSexSystem::SetSex_male(v52);
      v53 = cltBasicAppearSystem::GetHairColorKey(*((cltBasicAppearSystem **)this + 11));
      cltBasicAppearSystem::Initialize(*((cltBasicAppearSystem **)this + 11), a8, a9, v53);
      cltEquipmentSystem::SetEquipItemsByChangeSex(*((cltEquipmentSystem **)this + 10), a10, a11, a12);
    }
    if ( cltUsingItemSystem::IsCosmeticItem(this, *(_WORD *)v28) )
    {
      if ( *((_DWORD *)v28 + 44) )
      {
        v66 = cltBasicAppearSystem::GetHairColorKey(*((cltBasicAppearSystem **)this + 11));
        v64 = cltBasicAppearSystem::GetFace(*((cltBasicAppearSystem **)this + 11));
        cltBasicAppearSystem::Initialize(*((cltBasicAppearSystem **)this + 11), *((_BYTE *)v28 + 180), v64, v66);
      }
      else if ( *((_DWORD *)v28 + 46) )
      {
        v67 = cltBasicAppearSystem::GetHairColorKey(*((cltBasicAppearSystem **)this + 11));
        v65 = *((_BYTE *)v28 + 188);
        v54 = cltBasicAppearSystem::GetHair(*((cltBasicAppearSystem **)this + 11));
        cltBasicAppearSystem::Initialize(*((cltBasicAppearSystem **)this + 11), v54, v65, v67);
      }
    }
    if ( *((_WORD *)v28 + 105) && cltUsingItemSystem::m_pUseMapUsingItemFuncPtr )
      cltUsingItemSystem::m_pUseMapUsingItemFuncPtr(*((void **)this + 15), *(_WORD *)v28);
    v55 = *((_WORD *)v28 + 119);
    if ( v55 )
      cltPetSystem::AddPetSkill(*((cltPetSystem **)this + 12), v55);
    if ( *((_DWORD *)v28 + 60) )
      cltPetSystem::IncreasePetSatiety(*((cltPetSystem **)this + 12), *((_DWORD *)v28 + 60));
    v56 = *((_DWORD *)v28 + 61);
    if ( v56 )
    {
      v57 = (cltPetSystem *)*((_DWORD *)this + 12);
      a18 = 0;
      cltPetSystem::IncreasePetExp(v57, v56, (unsigned __int16 *)&a18);
      if ( (_WORD)a18 )
        *a23 = 1;
    }
    if ( *((_DWORD *)v28 + 62) )
      cltPetSystem::IncreasePetBagNum(*((cltPetSystem **)this + 12));
    if ( *((_DWORD *)v28 + 63) )
      cltPetSystem::ChangePetName(*((cltPetSystem **)this + 12), a5);
    v58 = *((_WORD *)v28 + 128);
    if ( v58 )
      cltPetSystem::DyePet(*((cltPetSystem **)this + 12), v58);
    if ( *((_DWORD *)v28 + 67) && cltUsingItemSystem::m_pPlayerCureFuncPtr )
      cltUsingItemSystem::m_pPlayerCureFuncPtr(*((void **)this + 15));
    if ( *((_DWORD *)v28 + 48) )
      cltBasicAppearSystem::SetNewHairColorKey(*((cltBasicAppearSystem **)this + 11), *((_DWORD *)v28 + 48));
    v59 = a20;
    if ( *((_WORD *)v28 + 136) )
    {
      v60 = a20 + 3600 * *((_DWORD *)v28 + 69);
      if ( a24 )
        *a24 = v60;
      cltMyItemSystem::AddMyItem(*((cltMyItemSystem **)this + 13), *((_WORD *)v28 + 136), v60, 0, 0);
    }
    v61 = *((_WORD *)v28 + 140);
    if ( v61 )
    {
      cltMarriageSystem::ChangeCoupleRing(*((cltMarriageSystem **)this + 14), v59, v61);
      if ( cltUsingItemSystem::m_pChangeCoupleRingFuncPtr )
        cltUsingItemSystem::m_pChangeCoupleRingFuncPtr(*((void **)this + 15), *(_WORD *)v28, v59);
    }
  }
  return v70;
}
// 59DF1E: variable 'v31' is possibly undefined

//----- (0059E320) --------------------------------------------------------
int cltUsingItemSystem::CanUseSustainedTypeItemWithoutItem(unsigned __int16 a2)
{
  struct stItemKindInfo *v3; // eax
  int v5; // eax

  v3 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, a2);
  if ( !v3 )
    return 1;
  if ( *((_BYTE *)v3 + 80) != 1 )
    return 1;
  if ( !*((_DWORD *)v3 + 23) )
    return 1;
  if ( cltUsingItemSystem::IsAlreadyUsedItem(this, a2) == 1 )
    return 102;
  v5 = -cltPlayerAbility::CanAddBuff(*((cltPlayerAbility **)this + 1));
  LOBYTE(v5) = v5 & 0x98;
  return v5 + 104;
}

//----- (0059E390) --------------------------------------------------------
void cltUsingItemSystem::UseSustainedTypeItemWithoutItem(unsigned __int16 a2, unsigned int a3)
{
  struct stItemKindInfo *v4; // eax
  struct stItemKindInfo *v5; // ebx
  struct strUsingItemInfo *v6; // edi

  v4 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, a2);
  v5 = v4;
  if ( v4 )
  {
    if ( *((_BYTE *)v4 + 80) == 1 )
    {
      v6 = cltUsingItemSystem::GetEmptyUsingItemInfo(this);
      *((_DWORD *)v6 + 1) = a3;
      *((_WORD *)v6 + 4) = *(_WORD *)v5;
      *(_DWORD *)v6 = cltTimerManager::CreateTimer(
                        cltUsingItemSystem::m_pclTimerManager,
                        1000 * *((_DWORD *)v5 + 23),
                        (unsigned int)this,
                        0,
                        1,
                        (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemInitialize,
                        (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemPoll,
                        (void (*)(unsigned int, unsigned int))cltUsingItemSystem::OnUsingItemTimeOuted,
                        0,
                        0);
      ++*((_DWORD *)this + 46);
    }
  }
}

//----- (0059E420) --------------------------------------------------------
struct strUsingItemInfo *cltUsingItemSystem::GetUsingItemInfo()
{
  return (cltUsingItemSystem *)((char *)this + 64);
}

//----- (0059E430) --------------------------------------------------------
struct strUsingItemInfo *cltUsingItemSystem::GetUsingItemInfo(unsigned int a2)
{
  int v2; // esi
  int v3; // eax
  _DWORD *i; // edx

  v2 = *((_DWORD *)this + 46);
  v3 = 0;
  if ( v2 <= 0 )
    return 0;
  for ( i = (_DWORD *)((char *)this + 64); *i != a2; i += 3 )
  {
    if ( ++v3 >= v2 )
      return 0;
  }
  return (cltUsingItemSystem *)((char *)this + 12 * v3 + 64);
}

//----- (0059E470) --------------------------------------------------------
__int16 cltUsingItemSystem::GetTotalStrOfUsingItem()
{
  __int16 v2; // bp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_WORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 50);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E4C0) --------------------------------------------------------
__int16 cltUsingItemSystem::GetTotalDexOfUsingItem()
{
  __int16 v2; // bp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_WORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 51);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E510) --------------------------------------------------------
__int16 cltUsingItemSystem::GetTotalIntOfUsingItem()
{
  __int16 v2; // bp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_WORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 52);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E560) --------------------------------------------------------
__int16 cltUsingItemSystem::GetTotalVitOfUsingItem()
{
  __int16 v2; // bp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_WORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 53);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E5B0) --------------------------------------------------------
void cltUsingItemSystem::OnUsingItemInitialize(unsigned int a1, cltUsingItemSystem *a2)
{
  struct strUsingItemInfo *v2; // eax

  if ( cltUsingItemSystem::m_pExternUsingItemInitializeFuncPtr )
  {
    v2 = cltUsingItemSystem::GetUsingItemInfo(a2, a1);
    cltUsingItemSystem::m_pExternUsingItemInitializeFuncPtr(a1, (unsigned int)v2);
  }
}

//----- (0059E5E0) --------------------------------------------------------
void cltUsingItemSystem::OnUsingItemPoll(unsigned int a1, cltUsingItemSystem *a2)
{
  struct strUsingItemInfo *v2; // eax

  if ( cltUsingItemSystem::m_pExternUsingItemPollFuncPtr )
  {
    v2 = cltUsingItemSystem::GetUsingItemInfo(a2, a1);
    cltUsingItemSystem::m_pExternUsingItemPollFuncPtr(a1, (unsigned int)v2);
  }
}

//----- (0059E610) --------------------------------------------------------
void cltUsingItemSystem::OnUsingItemCustom(unsigned int a1, cltUsingItemSystem *a2)
{
  struct strUsingItemInfo *v2; // eax

  if ( cltUsingItemSystem::m_pExternUsingItemCustomFuncPtr )
  {
    v2 = cltUsingItemSystem::GetUsingItemInfo(a2, a1);
    cltUsingItemSystem::m_pExternUsingItemCustomFuncPtr(a1, (unsigned int)v2);
  }
}

//----- (0059E640) --------------------------------------------------------
void cltUsingItemSystem::OnUsingItemTimeOuted(unsigned int a1, cltUsingItemSystem *a2)
{
  struct strUsingItemInfo *v2; // eax

  if ( cltUsingItemSystem::m_pExternUsingItemTimeOutFuncPtr )
  {
    v2 = cltUsingItemSystem::GetUsingItemInfo(a2, a1);
    cltUsingItemSystem::m_pExternUsingItemTimeOutFuncPtr(a1, (unsigned int)v2);
  }
  cltUsingItemSystem::OnUsingItemTimeOuted(a2, a1);
}

//----- (0059E680) --------------------------------------------------------
int cltUsingItemSystem::IsAlreadyUsedItem(unsigned __int16 a2)
{
  int v2; // edx
  int v3; // eax
  _WORD *i; // ecx

  v2 = *((_DWORD *)this + 46);
  v3 = 0;
  if ( v2 <= 0 )
    return 0;
  for ( i = (_WORD *)((char *)this + 72); *i != a2; i += 6 )
  {
    if ( ++v3 >= v2 )
      return 0;
  }
  return 1;
}

//----- (0059E6C0) --------------------------------------------------------
struct strUsingItemInfo *cltUsingItemSystem::GetEmptyUsingItemInfo()
{
  int v1; // eax
  struct strUsingItemInfo *result; // eax

  v1 = *((_DWORD *)this + 46);
  if ( v1 < 10 )
    result = (cltUsingItemSystem *)((char *)this + 12 * v1 + 64);
  else
    result = 0;
  return result;
}

//----- (0059E6E0) --------------------------------------------------------
int cltUsingItemSystem::GetUsingItemNum()
{
  return *((_DWORD *)this + 46);
}

//----- (0059E6F0) --------------------------------------------------------
int cltUsingItemSystem::IsActiveFastRun()
{
  int v2; // edi
  unsigned __int16 *i; // esi
  struct stItemKindInfo *v4; // eax

  v2 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  for ( i = (unsigned __int16 *)((char *)this + 72); ; i += 6 )
  {
    v4 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *i);
    if ( v4 )
    {
      if ( *((_DWORD *)v4 + 41) == 1 )
        break;
    }
    if ( ++v2 >= *((_DWORD *)this + 46) )
      return 0;
  }
  return 1;
}

//----- (0059E740) --------------------------------------------------------
int cltUsingItemSystem::GetMaxHPAdvantage()
{
  int v2; // ebp
  int v3; // esi
  unsigned __int16 *v4; // edi
  struct stItemKindInfo *v5; // eax
  __int16 v6; // ax

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v5 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4);
    if ( v5 )
    {
      v6 = *((_WORD *)v5 + 54);
      if ( v6 )
        v2 += v6;
    }
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E7A0) --------------------------------------------------------
int cltUsingItemSystem::GetMaxManaAdvantage()
{
  int v2; // ebp
  int v3; // esi
  unsigned __int16 *v4; // edi
  struct stItemKindInfo *v5; // eax
  __int16 v6; // ax

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v5 = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4);
    if ( v5 )
    {
      v6 = *((_WORD *)v5 + 55);
      if ( v6 )
        v2 += v6;
    }
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E800) --------------------------------------------------------
int cltUsingItemSystem::GetHitRateAdvantage()
{
  int v2; // ebp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((__int16 *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 56);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E850) --------------------------------------------------------
int cltUsingItemSystem::GetCriticalHitRateAdvantage()
{
  int v2; // ebp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((__int16 *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 58);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E8A0) --------------------------------------------------------
int cltUsingItemSystem::GetMissRateAdvantage()
{
  int v2; // ebp
  int v3; // esi
  unsigned __int16 *v4; // edi

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((__int16 *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 57);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E8F0) --------------------------------------------------------
int cltUsingItemSystem::GetAPowerAdvantage()
{
  int v2; // esi
  int v3; // edi
  unsigned __int16 *v4; // ebx

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_DWORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 65);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E940) --------------------------------------------------------
int cltUsingItemSystem::GetDPowerAdvantage()
{
  int v2; // esi
  int v3; // edi
  unsigned __int16 *v4; // ebx

  v2 = 0;
  v3 = 0;
  if ( *((int *)this + 46) <= 0 )
    return 0;
  v4 = (unsigned __int16 *)((char *)this + 72);
  do
  {
    v2 += *((_DWORD *)cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, *v4) + 66);
    ++v3;
    v4 += 6;
  }
  while ( v3 < *((_DWORD *)this + 46) );
  return v2;
}

//----- (0059E990) --------------------------------------------------------
struct stItemKindInfo *cltUsingItemSystem::IsCosmeticItem(unsigned __int16 a2)
{
  struct stItemKindInfo *result; // eax
  char v3; // cl

  result = cltItemKindInfo::GetItemKindInfo(cltUsingItemSystem::m_pclItemKindInfo, a2);
  if ( result )
  {
    v3 = *((_BYTE *)result + 34);
    result = (struct stItemKindInfo *)((!v3 || v3 == 8 || v3 == 14)
                                    && (*((_DWORD *)result + 44) || *((_DWORD *)result + 46)));
  }
  return result;
}

//----- (0059E9E0) --------------------------------------------------------
void cltUsingItemSystem::OnUsingItemTimeOuted(unsigned int a2)
{
  int v3; // eax
  int v4; // ecx
  _DWORD *i; // edx

  v3 = 0;
  v4 = *((_DWORD *)this + 46);
  if ( v4 > 0 )
  {
    for ( i = (_DWORD *)((char *)this + 64); *i != a2; i += 3 )
    {
      if ( ++v3 >= v4 )
        return;
    }
    _memmove((char *)this + 12 * v3 + 64, (char *)this + 12 * v3 + 76, 12 * v4 - 4 * (3 * v3 + 3));
    --*((_DWORD *)this + 46);
  }
}

//----- (0059EA50) --------------------------------------------------------
void cltUsingItemSystem::SetChangeCoinSucRate(int a2)
{
  *((_DWORD *)this + 49) = a2;
}

//----- (0059EA60) --------------------------------------------------------
void cltUsingItemSystem::SetChangeCoinRate(int a2, int a3)
{
  *((_DWORD *)this + 47) = a2;
  *((_DWORD *)this + 48) = a3;
}

//----- (0059EA80) --------------------------------------------------------
