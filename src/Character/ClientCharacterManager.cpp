#include "Character/ClientCharacterManager.h"

#include "global.h"

#include <cstring>
#ifndef _WIN32
#include <strings.h>
#endif

ClientCharacterManager::ClientCharacterManager() {
    m_dwMyAccount = 0;
    ResetMoveTarget();
}

ClientCharacterManager::~ClientCharacterManager() {
    ResetMoveTarget();
}

ClientCharacter* ClientCharacterManager::GetCharByAccount(unsigned int account) {
    for (int i = 1; i < 300; ++i) {
        ClientCharacter* pChar = &unk_1409D80[i];
        if (pChar->m_dwSlotAlive && pChar->m_dwAccountID == account) {
            return pChar;
        }
    }
    return nullptr;
}

bool ClientCharacterManager::IsMapConqueror(char* /*Name*/) {
    return true;
}

// mofclient.c 39000: delegates to cltCharKindInfo::IsMonsterChar on the static
// ClientCharacter::m_pClientCharKindInfo (when present).  Returns false when
// the kind-info has not been initialised yet.
bool ClientCharacterManager::IsMonster(unsigned short kindCode) {
    if (!ClientCharacter::m_pClientCharKindInfo) return false;
    return ClientCharacter::m_pClientCharKindInfo->IsMonsterChar(kindCode) != 0;
}

bool ClientCharacterManager::IsPlayer(unsigned short kindCode) {
    if (!ClientCharacter::m_pClientCharKindInfo) return false;
    return ClientCharacter::m_pClientCharKindInfo->IsPlayerChar(kindCode) != 0;
}

// mofclient.c 35772: clears the "currently-auto-attacking" flag for the slot
// whose m_dwAccountID matches.  The real implementation also nudges the CCA
// animation; we only update the state because auto-attack bookkeeping lives
// in the not-yet-ported combat path.
void ClientCharacterManager::ResetAutoAttack(unsigned int accountID) {
    ClientCharacter* pChar = GetCharByAccount(accountID);
    if (pChar) {
        // DWORD+161 in InitFlag defaults to 1 — the "auto-attack ready" flag.
        pChar->m_iInitFlag_153_176[161 - 153] = 0;
    }
}

void ClientCharacterManager::ResetMoveTarget() {
}

void ClientCharacterManager::AddCharacter(
    ClientCharacter* account, int x, int y,
    unsigned short charKind, unsigned short mapKind,
    int hp, const char* name,
    const char* guild, int guildMark,
    const char* a10, const char* a11,
    int a12, int a13, int a14, int a15, int a16, int a17,
    int a18, int a19, int a20, int a21,
    const char* a22, const char* a23,
    unsigned int teamKind, int a25, int a26, int a27, int a28,
    unsigned char nation, unsigned char sex, unsigned char hair,
    int a32, int a33) {
    (void)guild;
    (void)guildMark;
    (void)a10;
    (void)a11;
    (void)a12;
    (void)a13;
    (void)a14;
    (void)a15;
    (void)a16;
    (void)a17;
    (void)a18;
    (void)a19;
    (void)a20;
    (void)a21;
    (void)a22;
    (void)a23;
    (void)teamKind;
    (void)a25;
    (void)a26;
    (void)a27;
    (void)a28;
    (void)nation;
    (void)a32;
    (void)a33;

    int slot = 1;
    for (; slot < 300; ++slot) {
        if (!unk_1409D80[slot].m_dwSlotAlive) {
            break;
        }
    }
    if (slot >= 300) {
        return;
    }

    ClientCharacter& dst = unk_1409D80[slot];
    dst.m_dwAccountID = account ? account->m_dwAccountID : 0;
    dst.m_iPosX = x;
    dst.m_iPosY = y;
    dst.m_iDestX = x;
    dst.m_iDestY = y;
    dst.m_wKind = charKind;
    dst.m_wMapID = mapKind;
    dst.m_ucSex = sex;
    dst.m_ucHair = hair;
    if (name) {
        std::strncpy(dst.m_szName, name, sizeof(dst.m_szName) - 1);
        dst.m_szName[sizeof(dst.m_szName) - 1] = '\0';
    } else {
        dst.m_szName[0] = '\0';
    }
    dst.m_dwSlotAlive = 1;
    (void)hp;  // TODO: real ground truth stores HP via SetHP/UseHPPotionEffect; slot +448 offset is a custom shortcut retained from pre-restoration code.
}

void ClientCharacterManager::SetMyAccount(unsigned int account) {
    m_dwMyAccount = account;
}

ClientCharacter* ClientCharacterManager::GetMyCharacterPtr() {
    return GetCharByAccount(m_dwMyAccount);
}

void ClientCharacterManager::SetItem(unsigned int account, unsigned short itemKind, int qty) {
    ClientCharacter* pChar = GetCharByAccount(account);
    if (!pChar) {
        return;
    }
    pChar->SetItem(itemKind, qty);
}

void ClientCharacterManager::DeleteAllChar() {
    for (int i = 1; i < 300; ++i) {
        ClientCharacter* pChar = &unk_1409D80[i];
        if (pChar->m_dwSlotAlive) {
            pChar->DeleteCharacter();
            pChar->m_dwSlotAlive = 0;
        }
    }
    ResetMoveTarget();
}

void ClientCharacterManager::SetMyCAClone() {
    ClientCharacter* pMyChar = GetMyCharacterPtr();
    if (pMyChar) {
        pMyChar->SetCAClone();
    }
}

char* ClientCharacterManager::GetMyCharName() {
    ClientCharacter* pMyChar = GetMyCharacterPtr();
    return pMyChar ? pMyChar->m_szName : nullptr;
}

// mofclient.c 0x0040F3E0
char* ClientCharacterManager::GetCharName(unsigned int accountId) {
    static char s_empty[1] = { '\0' };
    ClientCharacter* pChar = GetCharByAccount(accountId);
    return pChar ? pChar->m_szName : s_empty;
}

ClientCharacter* ClientCharacterManager::GetCharByName(char* name) {
    if (!name) {
        return nullptr;
    }

    for (int i = 1; i < 300; ++i) {
        ClientCharacter* pChar = &unk_1409D80[i];
        if (!pChar->m_dwSlotAlive) {
            continue;
        }
        if (pChar->m_dwAccountID == 0) {
            continue;
        }
#ifdef _WIN32
        if (_stricmp(pChar->m_szName, name) == 0) {
#else
        if (strcasecmp(pChar->m_szName, name) == 0) {
#endif
            return pChar;
        }
    }
    return nullptr;
}

void ClientCharacterManager::SetEmoticonKind(char* name, std::uint32_t emoticonKind) {
    ClientCharacter* pChar = GetCharByName(name);
    if (pChar) {
        pChar->SetEmoticonKind(static_cast<int>(emoticonKind));
    }
}

void ClientCharacterManager::ReleaseEmoticon(ClientCharacter* pChar) {
    if (pChar) {
        pChar->ReleaseEmoticon();
    }
}

void ClientCharacterManager::PrepareDrawingEtcMark() {
}

void ClientCharacterManager::DrawEtcMark() {
}

bool ClientCharacterManager::GetMyTransformationed() {
    ClientCharacter* pMyChar = GetMyCharacterPtr();
    return pMyChar && pMyChar->IsTransformed();
}
