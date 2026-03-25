#pragma once

#include <windows.h>
#include <cstdio>
#include <cstring>

class cltConfiguration {
public:
    // The object itself stores the INI file path (char[]) at offset 0.
    // The constructor is trivial (returns this).
    cltConfiguration();
    ~cltConfiguration();

    BOOL Initialize(char* filePath);

    // Channel
    void SaveChannel(LPCSTR lpFileName, int worldIndex, int channelIndex);
    void GetChannel(LPCSTR lpFileName, int* outWorldIndex, int* outChannelIndex);

    // Screen
    void SaveScreenMode(LPCSTR lpFileName, int windowMode);
    void GetScreenMode(LPCSTR lpFileName, int* outWindowMode);
    void SaveScreenRate(LPCSTR lpFileName, int screenRate);
    void GetScreenRate(LPCSTR lpFileName, int* outScreenRate);

    // Sound
    void SaveSound(LPCSTR lpFileName, int bgSound, int effectSound);
    void GetSound(LPCSTR lpFileName, int* outBgSound, int* outEffectSound);

    // UI Settings
    void SaveChatWindow(LPCSTR lpFileName, int chatWindowSize);
    void GetChatWindow(LPCSTR lpFileName, int* outChatWindowSize);

    // Community
    void SavePartyState(LPCSTR lpFileName, int state);
    void GetPartyState(LPCSTR lpFileName, int* outState);
    void SaveWhisperState(LPCSTR lpFileName, int state);
    void GetWhisperState(LPCSTR lpFileName, int* outState);
    void SaveTradeState(LPCSTR lpFileName, int state);
    void GetTradeState(LPCSTR lpFileName, int* outState);
    void SaveFriendState(LPCSTR lpFileName, int state);
    void GetFriendState(LPCSTR lpFileName, int* outState);
    void SaveHelpState(LPCSTR lpFileName, int state);
    void GetHelpState(LPCSTR lpFileName, int* outState);
    void SaveSpiritSpeechState(LPCSTR lpFileName, int state);
    void GetSpiritSpeechState(LPCSTR lpFileName, int* outState);
    void SavePartyListState(LPCSTR lpFileName, int state);
    void GetPartyListState(LPCSTR lpFileName, int* outState);
    void SaveAddFriendState(LPCSTR lpFileName, int state);
    void GetAddFriendState(LPCSTR lpFileName, int* outState);
    void SaveAddCircleState(LPCSTR lpFileName, int state);
    void GetAddCircleState(LPCSTR lpFileName, int* outState);
    void SaveAddTandSState(LPCSTR lpFileName, int state);
    void GetAddTandSState(LPCSTR lpFileName, int* outState);

    // Event
    void SaveSummerEventWindow(LPCSTR lpFileName, int state);
    void GetSummerEventWindow(LPCSTR lpFileName, int* outState);

    // Key Settings (56 entries)
    void GetKeySetting(LPCSTR lpFileName, int* outKeys);
    void SaveKeySetting(LPCSTR lpFileName, int* keys);

    // Cash Shop Wish List (20 entries)
    void SaveCashShopWishList(LPCSTR lpFileName, int* wishList);
    void GetCashShopWishList(LPCSTR lpFileName, int* outWishList);

    // Meritorious
    void SaveState_MeritoriousView(LPCSTR lpFileName, int state);
    void GetState_MeritoriousView(LPCSTR lpFileName, int* outState);

    // Emoticon Shortcut
    void SaveState_EmoticonShortCut(LPCSTR lpFileName, int index, unsigned char value);
    void GetState_EmoticonShortCut(LPCSTR lpFileName, int index, unsigned char* outValue);

    // Quick Chat
    void SaveQuickChat(LPCSTR lpFileName, int playerIndex, int chatIndex, LPCSTR chatString);
    void GetQuickChat(LPCSTR lpFileName, int playerIndex, int chatIndex, LPSTR outString);

    // Auto Attack
    void SaveAutoAttackMode(LPCSTR lpFileName, int mode);
    void GetAutoAttackMode(LPCSTR lpFileName, int* outMode);

    // Save ID
    void SaveID(LPCSTR lpFileName, int saveId);
    void GetSaveID(LPCSTR lpFileName, int* outSaveId);
    void SaveDefaultID(LPCSTR lpFileName, LPCSTR defaultId);
    void GetDefaultID(LPCSTR lpFileName, LPSTR outDefaultId);
};

extern cltConfiguration* g_clConfig;
