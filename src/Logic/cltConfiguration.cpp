#include "Logic/cltConfiguration.h"

cltConfiguration::cltConfiguration() {
    // Trivial constructor – original just returns this.
}

cltConfiguration::~cltConfiguration() {
    // Trivial destructor.
}

BOOL cltConfiguration::Initialize(char* filePath) {
    strcpy(reinterpret_cast<char*>(this), filePath);
    return this != nullptr;
}

// ---- Channel ----

void cltConfiguration::SaveChannel(LPCSTR lpFileName, int worldIndex, int channelIndex) {
    char buf1[1024];
    char buf2[1024];
    sprintf(buf1, "%d", worldIndex);
    sprintf(buf2, "%d", channelIndex);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("CHANNELINFO", "WorldIndex", buf1, lpFileName);
    WritePrivateProfileStringA("CHANNELINFO", "ChannelIndex", buf2, lpFileName);
}

void cltConfiguration::GetChannel(LPCSTR lpFileName, int* outWorldIndex, int* outChannelIndex) {
    *outWorldIndex  = GetPrivateProfileIntA("CHANNELINFO", "WorldIndex", 0, lpFileName);
    *outChannelIndex = GetPrivateProfileIntA("CHANNELINFO", "ChannelIndex", 0, lpFileName);
}

// ---- Screen ----

void cltConfiguration::SaveScreenMode(LPCSTR lpFileName, int windowMode) {
    char buf[1024];
    sprintf(buf, "%d", windowMode);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("SCREENMODEINFO", "WindowMode", buf, lpFileName);
}

void cltConfiguration::GetScreenMode(LPCSTR lpFileName, int* outWindowMode) {
    *outWindowMode = GetPrivateProfileIntA("SCREENMODEINFO", "WindowMode", 0, lpFileName);
}

void cltConfiguration::SaveScreenRate(LPCSTR lpFileName, int screenRate) {
    char buf[1024];
    sprintf(buf, "%d", screenRate);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("SCREENRATEINFO", "ScreenRate", buf, lpFileName);
}

void cltConfiguration::GetScreenRate(LPCSTR lpFileName, int* outScreenRate) {
    *outScreenRate = GetPrivateProfileIntA("SCREENRATEINFO", "ScreenRate", 0, lpFileName);
}

// ---- Sound ----

void cltConfiguration::SaveSound(LPCSTR lpFileName, int bgSound, int effectSound) {
    char buf1[1024];
    char buf2[1024];
    sprintf(buf1, "%d", bgSound);
    sprintf(buf2, "%d", effectSound);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("SOUNDINFO", "BackgroundSound", buf1, lpFileName);
    WritePrivateProfileStringA("SOUNDINFO", "EffectSound", buf2, lpFileName);
}

void cltConfiguration::GetSound(LPCSTR lpFileName, int* outBgSound, int* outEffectSound) {
    *outBgSound    = GetPrivateProfileIntA("SOUNDINFO", "BackgroundSound", 125, lpFileName);
    *outEffectSound = GetPrivateProfileIntA("SOUNDINFO", "EffectSound", -1000, lpFileName);
}

// ---- UI Settings ----

void cltConfiguration::SaveChatWindow(LPCSTR lpFileName, int chatWindowSize) {
    char buf[1024];
    sprintf(buf, "%d", chatWindowSize);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("UISETTING", "ChatWindowSize", buf, lpFileName);
}

void cltConfiguration::GetChatWindow(LPCSTR lpFileName, int* outChatWindowSize) {
    *outChatWindowSize = GetPrivateProfileIntA("UISETTING", "ChatWindowSize", 0, lpFileName);
}

// ---- Community ----

void cltConfiguration::SavePartyState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "PARTYSTATE", buf, lpFileName);
}

void cltConfiguration::GetPartyState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "PARTYSTATE", 0, lpFileName);
}

void cltConfiguration::SaveWhisperState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "WHISPERSTATE", buf, lpFileName);
}

void cltConfiguration::GetWhisperState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "WHISPERSTATE", 0, lpFileName);
}

void cltConfiguration::SaveTradeState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "TRADESTATE", buf, lpFileName);
}

void cltConfiguration::GetTradeState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "TRADESTATE", 0, lpFileName);
}

void cltConfiguration::SaveFriendState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "FRIENDSTATE", buf, lpFileName);
}

void cltConfiguration::GetFriendState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "FRIENDSTATE", 0, lpFileName);
}

void cltConfiguration::SaveHelpState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "HELPSTATE", buf, lpFileName);
}

void cltConfiguration::GetHelpState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "HELPSTATE", 1, lpFileName);
}

void cltConfiguration::SaveSpiritSpeechState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "SPIRITSPEECHSTATE", buf, lpFileName);
}

void cltConfiguration::GetSpiritSpeechState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "SPIRITSPEECHSTATE", 1, lpFileName);
}

void cltConfiguration::SavePartyListState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "PARTYLISTSTATE", buf, lpFileName);
}

void cltConfiguration::GetPartyListState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "PARTYLISTSTATE", 1, lpFileName);
}

void cltConfiguration::SaveAddFriendState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "ADDFRIENDSTATE", buf, lpFileName);
}

void cltConfiguration::GetAddFriendState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "ADDFRIENDSTATE", 0, lpFileName);
}

void cltConfiguration::SaveAddCircleState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "ADDCIRCLESTATE", buf, lpFileName);
}

void cltConfiguration::GetAddCircleState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "ADDCIRCLESTATE", 0, lpFileName);
}

void cltConfiguration::SaveAddTandSState(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("COMMUNITY", "ADDTANDSSTATE", buf, lpFileName);
}

void cltConfiguration::GetAddTandSState(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("COMMUNITY", "ADDTANDSSTATE", 0, lpFileName);
}

// ---- Event ----

void cltConfiguration::SaveSummerEventWindow(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("EVENTWINDOWS", "EVENTWINDOW", buf, lpFileName);
}

void cltConfiguration::GetSummerEventWindow(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("EVENTWINDOWS", "EVENTWINDOW", 1, lpFileName);
}

// ---- Key Settings ----

void cltConfiguration::GetKeySetting(LPCSTR lpFileName, int* a2) {
    a2[0]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MENU", 1, lpFileName);
    a2[1]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_HELP", 59, lpFileName);
    a2[2]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_KEYSETTING", 60, lpFileName);
    a2[3]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_SCREENCAPTURE", 183, lpFileName);
    a2[4]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MAKEAVI", 70, lpFileName);
    a2[5]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL1", 2, lpFileName);
    a2[6]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL2", 3, lpFileName);
    a2[7]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL3", 4, lpFileName);
    a2[8]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL4", 5, lpFileName);
    a2[9]  = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL5", 6, lpFileName);
    a2[10] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL6", 7, lpFileName);
    a2[11] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL7", 8, lpFileName);
    a2[12] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL8", 9, lpFileName);
    a2[13] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL9", 10, lpFileName);
    a2[14] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL10", 11, lpFileName);
    a2[15] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL11", 12, lpFileName);
    a2[16] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSL12", 13, lpFileName);
    a2[17] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QUEST", 16, lpFileName);
    a2[18] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_WORLDMAP", 17, lpFileName);
    a2[19] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_EQUIP", 18, lpFileName);
    a2[20] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_INVEN", 23, lpFileName);
    a2[21] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_OPTION", 24, lpFileName);
    a2[22] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_FRIEND", 33, lpFileName);
    a2[23] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_DIARY", 32, lpFileName);
    a2[24] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_SKILL", 31, lpFileName);
    a2[50] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_GETITEM", 44, lpFileName);
    a2[25] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MYINFO", 46, lpFileName);
    a2[26] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MINIMAP", 50, lpFileName);
    a2[51] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_ATTACK", 29, lpFileName);
    a2[49] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CHANGETARGET", 56, lpFileName);
    a2[27] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CHATSTART", 28, lpFileName);
    a2[28] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CHATEND", 15, lpFileName);
    a2[52] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MOVEUP", 200, lpFileName);
    a2[53] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MOVELEFT", 203, lpFileName);
    a2[54] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MOVERIGHT", 205, lpFileName);
    a2[55] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MOVEDOWN", 208, lpFileName);
    a2[29] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_BASICCHAT", 63, lpFileName);
    a2[30] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_PARTYCHAT", 64, lpFileName);
    a2[31] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CIRCLECHAT", 65, lpFileName);
    a2[32] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_WHISPERCHAT", 66, lpFileName);
    a2[33] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSLPAGEUP", 201, lpFileName);
    a2[34] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QSLPAGEDOWN", 209, lpFileName);
    a2[35] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_RUN", 19, lpFileName);
    a2[36] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_INFO", 34, lpFileName);
    a2[37] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CIRCLE", 35, lpFileName);
    a2[38] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MAJOR", 36, lpFileName);
    a2[39] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MERITORIOUS", 37, lpFileName);
    a2[40] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_HUNT", 38, lpFileName);
    a2[41] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_EMBLEM", 39, lpFileName);
    a2[42] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_CHANGEMOPTARGET", 61, lpFileName);
    a2[43] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QUICKCHAT", 62, lpFileName);
    a2[44] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_PET", 25, lpFileName);
    a2[45] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_MYITEM", 21, lpFileName);
    a2[46] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_HIDEUI", 22, lpFileName);
    a2[47] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_ACADEMY", 20, lpFileName);
    a2[48] = GetPrivateProfileIntA("KEYSETTING", "KEYTYPE_QUESTALARM", 30, lpFileName);

    // If any key is 0, set it to a sentinel value: (index << 24) | 0xFFFFFF
    int* p = a2;
    for (int i = 0; i < 56; ++i) {
        if (!*p)
            *p = (i << 24) | 0x00FFFFFF;
        ++p;
    }
}

void cltConfiguration::SaveKeySetting(LPCSTR lpFileName, int* a2) {
    char buf[1024];

    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    sprintf(buf, "%d", a2[0]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MENU", buf, lpFileName);
    sprintf(buf, "%d", a2[1]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_HELP", buf, lpFileName);
    sprintf(buf, "%d", a2[2]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_KEYSETTING", buf, lpFileName);
    sprintf(buf, "%d", a2[3]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_SCREENCAPTURE", buf, lpFileName);
    sprintf(buf, "%d", a2[4]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MAKEAVI", buf, lpFileName);
    sprintf(buf, "%d", a2[5]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL1", buf, lpFileName);
    sprintf(buf, "%d", a2[6]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL2", buf, lpFileName);
    sprintf(buf, "%d", a2[7]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL3", buf, lpFileName);
    sprintf(buf, "%d", a2[8]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL4", buf, lpFileName);
    sprintf(buf, "%d", a2[9]);  WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL5", buf, lpFileName);
    sprintf(buf, "%d", a2[10]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL6", buf, lpFileName);
    sprintf(buf, "%d", a2[11]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL7", buf, lpFileName);
    sprintf(buf, "%d", a2[12]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL8", buf, lpFileName);
    sprintf(buf, "%d", a2[13]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL9", buf, lpFileName);
    sprintf(buf, "%d", a2[14]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL10", buf, lpFileName);
    sprintf(buf, "%d", a2[15]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL11", buf, lpFileName);
    sprintf(buf, "%d", a2[16]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSL12", buf, lpFileName);
    sprintf(buf, "%d", a2[17]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QUEST", buf, lpFileName);
    sprintf(buf, "%d", a2[18]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_WORLDMAP", buf, lpFileName);
    sprintf(buf, "%d", a2[19]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_EQUIP", buf, lpFileName);
    sprintf(buf, "%d", a2[20]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_INVEN", buf, lpFileName);
    sprintf(buf, "%d", a2[21]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_OPTION", buf, lpFileName);
    sprintf(buf, "%d", a2[22]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_FRIEND", buf, lpFileName);
    sprintf(buf, "%d", a2[23]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_DIARY", buf, lpFileName);
    sprintf(buf, "%d", a2[24]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_SKILL", buf, lpFileName);
    sprintf(buf, "%d", a2[50]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_GETITEM", buf, lpFileName);
    sprintf(buf, "%d", a2[25]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MYINFO", buf, lpFileName);
    sprintf(buf, "%d", a2[26]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MINIMAP", buf, lpFileName);
    sprintf(buf, "%d", a2[51]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_ATTACK", buf, lpFileName);
    sprintf(buf, "%d", a2[49]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CHANGETARGET", buf, lpFileName);
    sprintf(buf, "%d", a2[27]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CHATSTART", buf, lpFileName);
    sprintf(buf, "%d", a2[28]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CHATEND", buf, lpFileName);
    sprintf(buf, "%d", a2[52]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MOVEUP", buf, lpFileName);
    sprintf(buf, "%d", a2[53]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MOVELEFT", buf, lpFileName);
    sprintf(buf, "%d", a2[54]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MOVERIGHT", buf, lpFileName);
    sprintf(buf, "%d", a2[55]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MOVEDOWN", buf, lpFileName);
    sprintf(buf, "%d", a2[29]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_BASICCHAT", buf, lpFileName);
    sprintf(buf, "%d", a2[30]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_PARTYCHAT", buf, lpFileName);
    sprintf(buf, "%d", a2[31]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CIRCLECHAT", buf, lpFileName);
    sprintf(buf, "%d", a2[32]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_WHISPERCHAT", buf, lpFileName);
    sprintf(buf, "%d", a2[33]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSLPAGEUP", buf, lpFileName);
    sprintf(buf, "%d", a2[34]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QSLPAGEDOWN", buf, lpFileName);
    sprintf(buf, "%d", a2[35]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_RUN", buf, lpFileName);
    sprintf(buf, "%d", a2[36]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_INFO", buf, lpFileName);
    sprintf(buf, "%d", a2[37]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CIRCLE", buf, lpFileName);
    sprintf(buf, "%d", a2[38]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MAJOR", buf, lpFileName);
    sprintf(buf, "%d", a2[39]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MERITORIOUS", buf, lpFileName);
    sprintf(buf, "%d", a2[40]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_HUNT", buf, lpFileName);
    sprintf(buf, "%d", a2[41]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_EMBLEM", buf, lpFileName);
    sprintf(buf, "%d", a2[42]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_CHANGEMOPTARGET", buf, lpFileName);
    sprintf(buf, "%d", a2[43]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QUICKCHAT", buf, lpFileName);
    sprintf(buf, "%d", a2[44]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_PET", buf, lpFileName);
    sprintf(buf, "%d", a2[45]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_MYITEM", buf, lpFileName);
    sprintf(buf, "%d", a2[46]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_HIDEUI", buf, lpFileName);
    sprintf(buf, "%d", a2[47]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_ACADEMY", buf, lpFileName);
    sprintf(buf, "%d", a2[48]); WritePrivateProfileStringA("KEYSETTING", "KEYTYPE_QUESTALARM", buf, lpFileName);
}

// ---- Cash Shop Wish List ----

void cltConfiguration::SaveCashShopWishList(LPCSTR lpFileName, int* wishList) {
    char keyName[32];
    char buf[1024];
    for (int i = 0; i < 20; ++i) {
        sprintf(keyName, "LIST%02d", i);
        sprintf(buf, "%d", *wishList);
        SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
        WritePrivateProfileStringA("CASHSHOPWISHLIST", keyName, buf, lpFileName);
        ++wishList;
    }
}

void cltConfiguration::GetCashShopWishList(LPCSTR lpFileName, int* outWishList) {
    char keyName[32];
    for (int i = 0; i < 20; ++i) {
        sprintf(keyName, "LIST%02d", i);
        *outWishList++ = GetPrivateProfileIntA("CASHSHOPWISHLIST", keyName, 0, lpFileName);
    }
}

// ---- Meritorious ----

void cltConfiguration::SaveState_MeritoriousView(LPCSTR lpFileName, int state) {
    char buf[1024];
    sprintf(buf, "%d", state);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("MERITORIOUSVIEW", "MERITORIOUS", buf, lpFileName);
}

void cltConfiguration::GetState_MeritoriousView(LPCSTR lpFileName, int* outState) {
    *outState = GetPrivateProfileIntA("MERITORIOUSVIEW", "MERITORIOUS", 1, lpFileName);
}

// ---- Emoticon Shortcut ----

void cltConfiguration::SaveState_EmoticonShortCut(LPCSTR lpFileName, int index, unsigned char value) {
    char keyName[256];
    char buf[1024];
    strcpy(keyName, "SHORTCUT");
    sprintf(buf, "%d", index);
    strcat(keyName, buf);
    sprintf(buf, "%d", value);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("EMOTICON_SHORTCUT", keyName, buf, lpFileName);
}

void cltConfiguration::GetState_EmoticonShortCut(LPCSTR lpFileName, int index, unsigned char* outValue) {
    char keyName[256];
    char buf[1024];
    strcpy(keyName, "SHORTCUT");
    sprintf(buf, "%d", index);
    strcat(keyName, buf);
    *outValue = static_cast<unsigned char>(GetPrivateProfileIntA("EMOTICON_SHORTCUT", keyName, 255, lpFileName));
}

// ---- Quick Chat ----

void cltConfiguration::SaveQuickChat(LPCSTR lpFileName, int playerIndex, int chatIndex, LPCSTR chatString) {
    char prefix[256];
    char fullKey[256];
    char keyName[1024];
    sprintf(prefix, "P.%d_", playerIndex + 1);
    sprintf(fullKey, "%s%d", prefix, chatIndex);
    strcpy(keyName, fullKey);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    if (lstrcmpA(chatString, "") != 0)
        WritePrivateProfileStringA("QUICK_CHATTING", keyName, chatString, lpFileName);
    else
        WritePrivateProfileStringA("QUICK_CHATTING", keyName, nullptr, lpFileName);
}

void cltConfiguration::GetQuickChat(LPCSTR lpFileName, int playerIndex, int chatIndex, LPSTR outString) {
    char prefix[256];
    char fullKey[256];
    char keyName[1024];
    sprintf(prefix, "P.%d_", playerIndex + 1);
    sprintf(fullKey, "%s%d", prefix, chatIndex);
    strcpy(keyName, fullKey);
    GetPrivateProfileStringA("QUICK_CHATTING", keyName, "", outString, 256, lpFileName);
}

// ---- Auto Attack ----

void cltConfiguration::SaveAutoAttackMode(LPCSTR lpFileName, int mode) {
    char buf[1024];
    sprintf(buf, "%d", mode);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("AUTOATTACKMODEINFO", "AutoAttackMode", buf, lpFileName);
}

void cltConfiguration::GetAutoAttackMode(LPCSTR lpFileName, int* outMode) {
    *outMode = GetPrivateProfileIntA("AUTOATTACKMODEINFO", "AutoAttackMode", 1, lpFileName);
}

// ---- Save ID ----

void cltConfiguration::SaveID(LPCSTR lpFileName, int saveId) {
    char buf[1024];
    sprintf(buf, "%d", saveId);
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("SAVEID", "SaveID", buf, lpFileName);
}

void cltConfiguration::GetSaveID(LPCSTR lpFileName, int* outSaveId) {
    *outSaveId = GetPrivateProfileIntA("SAVEID", "SaveID", 0, lpFileName);
}

void cltConfiguration::SaveDefaultID(LPCSTR lpFileName, LPCSTR defaultId) {
    int saveIdState;
    GetSaveID(lpFileName, &saveIdState);
    if (!saveIdState)
        strcpy(const_cast<char*>(defaultId), "");
    SetFileAttributesA(lpFileName, FILE_ATTRIBUTE_NORMAL);
    WritePrivateProfileStringA("SAVEDEFAULTID", "SaveDefaultID", defaultId, lpFileName);
}

void cltConfiguration::GetDefaultID(LPCSTR lpFileName, LPSTR outDefaultId) {
    int saveIdState;
    GetSaveID(lpFileName, &saveIdState);
    if (saveIdState)
        GetPrivateProfileStringA("SAVEDEFAULTID", "SaveDefaultID", "", outDefaultId, 64, lpFileName);
    else
        strcpy(outDefaultId, "");
}
