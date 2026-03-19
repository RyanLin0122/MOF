#include "Character/ClientCharacterManager.h"
#include "global.h"
#include <new>


ClientCharacterManager::ClientCharacterManager() {

}

ClientCharacterManager::~ClientCharacterManager() {

}

ClientCharacter* ClientCharacterManager::GetCharByAccount(unsigned int account) {
    for (int i = 0; i < 300; ++i)
    {
        ClientCharacter* pChar = &unk_1409D80[i];
        if (reinterpret_cast<unsigned int*>(pChar)[1109] && pChar->m_dwAccountID == account)
            return pChar;
    }
    return nullptr;
}

bool ClientCharacterManager::IsMapConqueror(char* Name) {
    return true;
}

void ClientCharacterManager::ResetMoveTarget(){}

void ClientCharacterManager::AddCharacter(
        ClientCharacter* /*account*/, int /*x*/, int /*y*/,
        unsigned short /*charKind*/, unsigned short /*mapKind*/,
        int /*hp*/, const char* /*name*/,
        const char* /*guild*/, int /*guildMark*/,
        const char* /*a10*/, const char* /*a11*/,
        int /*a12*/, int /*a13*/, int /*a14*/, int /*a15*/, int /*a16*/, int /*a17*/,
        int /*a18*/, int /*a19*/, int /*a20*/, int /*a21*/,
        const char* /*a22*/, const char* /*a23*/,
        unsigned int /*teamKind*/, int /*a25*/, int /*a26*/, int /*a27*/, int /*a28*/,
        unsigned char /*nation*/, unsigned char /*sex*/, unsigned char /*hair*/,
        int /*a32*/, int /*a33*/) {
    // Stub: real implementation allocates and registers a new ClientCharacter.
}

void ClientCharacterManager::SetMyAccount(unsigned int account) {
    m_dwMyAccount = account;
}

ClientCharacter* ClientCharacterManager::GetMyCharacterPtr() {
    return GetCharByAccount(m_dwMyAccount);
}

void ClientCharacterManager::SetItem(unsigned int /*account*/, unsigned short /*itemKind*/, int /*qty*/) {
    // Stub: real implementation updates the character's equipment/inventory.
}

void ClientCharacterManager::DeleteAllChar() {
    // Stub: real implementation destroys all registered ClientCharacter objects.
}

void ClientCharacterManager::SetMyCAClone() {
    // Stub: real implementation clones the CA (character animation) for the local player.
}

char* ClientCharacterManager::GetMyCharName() {
    // Stub: real implementation returns the local player's character name string.
    return nullptr;
}


ClientCharacter* ClientCharacterManager::GetCharByName(char* name) {
    (void)name;
    return nullptr;
}

void ClientCharacterManager::SetEmoticonKind(char* /*name*/, std::uint32_t /*emoticonKind*/) {
    // Stub: emoticon visual state is not modeled in this reduced client implementation.
}

void ClientCharacterManager::ReleaseEmoticon(ClientCharacter* /*pChar*/) {
    // Stub: emoticon visual state is not modeled in this reduced client implementation.
}

void ClientCharacterManager::PrepareDrawingEtcMark() {
    // Stub: ground truth 遍歷所有角色，準備名牌/標記繪製
}

void ClientCharacterManager::DrawEtcMark() {
    // Stub: ground truth 繪製所有角色的名牌/標記
}

bool ClientCharacterManager::GetMyTransformationed() {
    // Stub: ground truth 檢查自己的角色是否處於變身狀態
    return false;
}
