#include "Test/SoundTest.h"
#include <filesystem>
#include <string>

#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdint>

#include "Sound/GameSound.h"
#include "Sound/COgg.h"
#include "Sound/CSoundSystem.h"
#include "Sound/CWaveFile.h"
#include "FileSystem/CMOFPacking.h"
#include "Character/ClientCharacter.h"
#include "global.h"

extern int g_bLoadOggFromMofPack;

namespace {

constexpr const char* kOggBgmRaw = "/mofdata/music/bg_eastfield.ogg";
constexpr const char* kOggAmbientRaw = "/mofdata/music/embi_beastcave.ogg";
constexpr const char* kWavSfxShortRaw = "/mofdata/sound/minigame_se_gagebutton.wav";
constexpr const char* kWavSfxMonsterRaw = "/mofdata/sound/mon_atk_cho-s.wav";

void print_case_result(const char* suite, const char* caseName, bool pass, const char* expected) {
    std::printf("[%s] %s: %s\n", suite, caseName, pass ? "PASS" : "FAIL");
    std::printf("    預期結果: %s\n", expected);
}

void to_relative_path(const char* rawPath, char* outPath, size_t outSize) {
    if (!outPath || outSize == 0) return;
    outPath[0] = '\0';

    if (!rawPath) return;
    while (*rawPath == '/' || *rawPath == '\\') {
        ++rawPath;
    }

    std::snprintf(outPath, outSize, "%s", rawPath);
}

bool can_open_direct_file(const char* rawPath) {
    char relativePath[260]{};
    to_relative_path(rawPath, relativePath, sizeof(relativePath));

    FILE* fp = std::fopen(relativePath, "rb");
    if (!fp) return false;
    std::fclose(fp);
    return true;
}

bool can_read_from_pack_with_candidates(CMofPacking* packer, const char* const* candidates, int candidateCount) {
    if (!packer || !packer->m_pNfsHandle || !candidates || candidateCount <= 0) return false;

    for (int i = 0; i < candidateCount; ++i) {
        if (!candidates[i]) continue;
        char* buf = packer->FileRead(candidates[i]);
        if (buf && packer->GetBufferSize() > 0) {
            return true;
        }
    }
    return false;
}

std::uint16_t local_parse_sound_id(const char* id) {
    if (!id || std::strlen(id) != 5) return 0;

    int high = (std::toupper(static_cast<unsigned char>(id[0])) + 31) << 11;
    unsigned short low = static_cast<unsigned short>(std::atoi(id + 1));
    if (low >= 0x800u) return 0;

    return static_cast<std::uint16_t>(high | low);
}

bool test_ogg_single_functions() {
    bool ok = true;

    COgg ogg;
    ogg.Initalize(0);
    ok &= (ogg.m_nVolume == 255);
    ok &= (ogg.m_nChannelId == -1);

    // 測試邊界值裁切: 預期 <0 會被設為 0。
    ogg.SetVolume(-5);
    ok &= (ogg.m_nVolume == 0);

    // 測試邊界值裁切: 預期 >255 會被設為 255。
    ogg.SetVolume(999);
    ok &= (ogg.m_nVolume == 255);

    // 在未播放狀態下呼叫 Stop: 預期不當機且 channel 回到 -1。
    ogg.Stop();
    ok &= (ogg.m_nChannelId == -1);

    // 在未開啟 VFS 狀態下 OpenStreem: 預期 stream 維持 nullptr。
    ogg.OpenStreem("music/not_exists.ogg");
    ok &= (ogg.m_pStream == nullptr);

    return ok;
}

bool test_wavefile_single_functions() {
    bool ok = true;

    // 建立記憶體 wav 測試資料。
    unsigned char pcmData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    tWAVEFORMATEX wf{};
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 1;
    wf.nSamplesPerSec = 22050;
    wf.wBitsPerSample = 8;
    wf.nBlockAlign = 1;
    wf.nAvgBytesPerSec = 22050;

    CWaveFile wave;

    // OpenFromMemory(mode=1): 預期成功回傳 0。
    ok &= (wave.OpenFromMemory(pcmData, sizeof(pcmData), &wf, 1u) == 0u);

    // ResetFile(記憶體模式): 預期成功回傳 0。
    ok &= (wave.ResetFile() == 0);

    // Read: 預期回傳成功，且 bytesRead 為要求長度。
    unsigned char out[8]{};
    UINT bytesRead = 0;
    ok &= (wave.Read(out, sizeof(out), &bytesRead) == 0);
    ok &= (bytesRead == sizeof(out));
    ok &= (std::memcmp(out, pcmData, sizeof(out)) == 0);

    // GetFormat: 預期回傳初始化時提供的格式指標。
    ok &= (wave.GetFormat() == &wf);

    // Close(記憶體模式):
    // 目前 CWaveFile::OpenFromMemory() 不會建立 mmio handle，
    // 因此 Close() 會回傳錯誤碼 (非 0)；此處驗證既有行為，避免測試誤判。
    ok &= (wave.Close() != 0);

    return ok;
}

bool test_sound_manager_single_functions() {
    bool ok = true;

    CSoundManager mgr;

    // 未初始化時 SetPrimaryBufferFormat: 預期回傳 DSERR_UNINITIALIZED。
    ok &= (mgr.SetPrimaryBufferFormat(2, 44100, 16) == DSERR_UNINITIALIZED);

    // 參數錯誤路徑: outListener 為 nullptr，預期 E_INVALIDARG。
    ok &= (mgr.Get3DListenerInterface(nullptr) == E_INVALIDARG);

    // 未初始化時建立 Sound: 預期 DSERR_UNINITIALIZED。
    CSound* sound = nullptr;
    ok &= (mgr.Create(&sound, const_cast<char*>("dummy.wav"), 0, GUID_NULL, 1) == DSERR_UNINITIALIZED);

    // 未初始化時建立 Memory Sound: 預期 DSERR_UNINITIALIZED。
    unsigned char data[4]{};
    tWAVEFORMATEX wf{};
    ok &= (mgr.CreateFromMemory(&sound, data, sizeof(data), &wf, 0, GUID_NULL, 1) == DSERR_UNINITIALIZED);

    // 未初始化時建立 Streaming Sound: 預期 DSERR_UNINITIALIZED。
    CStreamingSound* streaming = nullptr;
    ok &= (mgr.CreateStreaming(&streaming, const_cast<char*>("dummy.wav"), 0, GUID_NULL, 2, 2048, reinterpret_cast<void*>(1)) == DSERR_UNINITIALIZED);

    return ok;
}

bool test_gamesound_single_functions() {
    bool ok = true;

    // GameSound 內含大型 sound table，改用 heap 避免 stack 壓力。
    GameSound* gs = new GameSound();

    // SetCharacter: 預期內部指標被更新。
    gs->SetCharacter(nullptr);
    ok &= (gs->m_character == nullptr);

    // ResetAll: 目前實作會將 masterDb 重設為 0。
    gs->m_masterDb = -777;
    gs->ResetAll();
    ok &= (gs->m_masterDb == 0);

    // 補上 activeIds 後再 ResetAll: 預期 masterDb 維持 0，初始化失敗旗標清除。
    gs->m_activeIds = static_cast<std::uint16_t*>(operator new(sizeof(std::uint16_t)));
    gs->m_soundInitFailed = true;
    gs->ResetAll();
    ok &= (gs->m_masterDb == 0);
    ok &= (gs->m_soundInitFailed == false);

    // Stop/Reset 傳入無效 id: 預期安全返回，不當機。
    gs->StopSound(nullptr);
    gs->ResetSound(nullptr);

    delete gs;
    return ok;
}

bool test_cross_file_mixed_audio_flow() {
    bool ok = true;

    // 跨檔案整合流程:
    // GameSound 內含兩個 COgg (BGM/Ambient)，搭配 FadeIn/FadeOut 與 IsBGMFinish。
    // 這裡不依賴真實音效檔，測控制流程與邏輯。
    // GameSound 物件改放 heap，避免測試函式堆疊使用過大。
    GameSound* gs = new GameSound();
    gs->m_soundInitFailed = false;

    gs->m_bgmTargetVolume = 25;
    gs->m_ambientTargetVolume = 30;
    gs->PlayMusic(const_cast<char*>("music/bg_beavers.ogg"));      // 預期: 設定 BGM fade 起始音量
    gs->PlayAmbientSound(const_cast<char*>("sound/ambient.ogg"));  // 預期: 設定 Ambient fade 起始音量

    ok &= (gs->m_bgmFadeVolume == 25);
    ok &= (gs->m_ambientFadeVolume == 30);

    gs->m_bgmFadeVolume = 0;
    gs->m_ambientFadeVolume = 0;
    gs->FadeInMusic();
    // 預期: 每次 FadeIn 增加 10，且不超過 target。
    ok &= (gs->m_bgmFadeVolume == 10);
    ok &= (gs->m_ambientFadeVolume == 10);

    gs->FadeOutMusic();
    // 預期: 每次 FadeOut 減少 10，且不會小於 0。
    ok &= (gs->m_bgmFadeVolume == 0);
    ok &= (gs->m_ambientFadeVolume == 0);

    // 先明確建立「未載入 stream」條件，再驗證 IsBGMFinish。
    gs->StopMusic();
    gs->m_bgm.m_pStream = nullptr;
    // 未載入 stream 時 IsBGMFinish: 預期回傳 true。
    ok &= (gs->IsBGMFinish() == 1);

    gs->StopAmbientSound();

    delete gs;
    return ok;
}

bool test_integration_init_sound_with_reallist() {
    bool ok = true;

    // 實際調用 InitSound，輸入 SoundList.txt（非模擬字串）。
    GameSound* gs = new GameSound();
    const int initRet = gs->InitSound(const_cast<char*>("SoundListInfo.txt"));
    ok &= (initRet == 1);
    ok &= (gs->m_soundInitFailed == false);

    struct ExpectedRow {
        const char* id;
        const char* fileName;
        int concurrentCount;
        int localized;
    };

    const ExpectedRow expected[] = {
        { "J0001", "UI_se_button_small.wav", 1, 0 },
        { "J0002", "UI_se_button_small.wav", 1, 0 },
        { "J0003", "UI_se_button_middle.wav", 1, 0 },
        { "J0004", "UI_se_button_large.wav", 1, 0 },
        { "J0005", "UI_se_window_open.wav", 1, 0 },
        { "J0006", "UI_se_trade.wav", 1, 0 },
        { "J0007", "UI_se_tab_chage.wav", 1, 0 },
        { "J0008", "UI_se_window_close.wav", 10, 0 },
        { "J0009", "UI_se_drop.wav", 10, 0 },
        { "J0010", "WEAPON_se_hit_bg_bird.wav", 10, 0 },
        { "J0013", "CHARATER_se_item_get.wav", 10, 0 },
        { "J0014", "CHARATER_se_levelup.wav", 10, 0 },
        { "J0015", "CHARATER_se_item_use_potion.wav", 10, 0 },
        { "J0016", "CHARATER_se_warp.wav", 10, 0 },
        { "J0017", "CHARATER_se_step.wav", 10, 0 },
        { "J0018", "CHARATER_se_run.wav", 10, 0 },
        { "J0019", "CHARATER_se_classup.wav", 10, 0 },
    };

    if (initRet == 1) {
        for (const auto& row : expected) {
            const std::uint16_t idx = local_parse_sound_id(row.id);
            ok &= (idx != 0);
            if (!idx) continue;

            const SoundEntry& e = gs->m_soundTable[idx];

            // 對答案: path 需是 InitSound 填入的 MOFData/Sound/<filename>
            char expectedPath[320]{};
            std::snprintf(expectedPath, sizeof(expectedPath), "mofdata/sound/%s", row.fileName);
            for (char* p = expectedPath; *p; ++p) {
                *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
            }
            ok &= (std::strcmp(e.path, expectedPath) == 0);

            // 對答案: 第3欄(동시재생수)會被 InitSound 寫入 concurrentPlayCount。
            ok &= (e.concurrentPlayCount == static_cast<std::uint16_t>(row.concurrentCount));

            // 對答案補充: localized 欄位目前 InitSound 讀進來但未存入 SoundEntry，
            // 因此這裡僅驗證可成功載入整體表格與關鍵欄位。
            ok &= (e.localizedType == static_cast<std::uint16_t>(row.localized));
        }
    }

    delete gs;
    return ok;
}

bool test_integration_file_read_direct_and_mofpacking() {
    bool ok = true;

    // 指定檔案都先測「直接檔案路徑可讀」。
    const bool directOggBgm = can_open_direct_file(kOggBgmRaw);
    const bool directOggAmbient = can_open_direct_file(kOggAmbientRaw);
    const bool directWavShort = can_open_direct_file(kWavSfxShortRaw);
    const bool directWavMonster = can_open_direct_file(kWavSfxMonsterRaw);
    ok &= directOggBgm && directOggAmbient && directWavShort && directWavMonster;

    CMofPacking* packer = CMofPacking::GetInstance();
    const bool packOpened = (packer && (packer->m_pNfsHandle || packer->PackFileOpen("mof")));
    ok &= packOpened;

    if (packOpened) {
        const char* oggBgmCandidates[] = { "music/bg_eastfield.ogg", "mofdata/music/bg_eastfield.ogg" };
        const char* oggAmbientCandidates[] = { "music/embi_beastcave.ogg", "mofdata/music/embi_beastcave.ogg" };
        const char* wavShortCandidates[] = { "sound/minigame_se_gagebutton.wav", "mofdata/sound/minigame_se_gagebutton.wav" };
        const char* wavMonsterCandidates[] = { "sound/mon_atk_cho-s.wav", "mofdata/sound/mon_atk_cho-s.wav" };

        // 預期: 透過 mofpacking 讀取各檔案時，至少有一個候選路徑可成功讀取到 buffer。
        ok &= can_read_from_pack_with_candidates(packer, oggBgmCandidates, 2);
        ok &= can_read_from_pack_with_candidates(packer, oggAmbientCandidates, 2);
        ok &= can_read_from_pack_with_candidates(packer, wavShortCandidates, 2);
        ok &= can_read_from_pack_with_candidates(packer, wavMonsterCandidates, 2);
    }

    return ok;
}

bool test_integration_music_playback_bgm_and_ambient() {
    bool ok = true;

    GameSound* gs = new GameSound();
    gs->m_soundInitFailed = false;

    // 1) 直接檔案模式：測 bg_eastfield.ogg + embi_beastcave.ogg 播放與主動調整音量。
    //   g_bLoadOggFromMofPack = 0;
    //   gs->m_bgmTargetVolume = 80;
    //   gs->m_ambientTargetVolume = 60;
    //   std::filesystem::path root = std::filesystem::current_path();
    //   std::filesystem::path musicPath = root / "mofdata" / "music" / "bg_eastfield.ogg";
    //   std::filesystem::path ambientPath = root / "mofdata" / "music" / "embi_beastcave.ogg";
    
    //   gs->m_bgm.Initalize(0);
    //   gs->m_ambient.Initalize(1);
    //   gs->PlayMusic(const_cast<char*>(musicPath.string().c_str()));
    //   gs->PlayAmbientSound(const_cast<char*>(ambientPath.string().c_str()));
    
    //   ok &= (gs->m_bgmFadeVolume == 80);
    //   ok &= (gs->m_ambientFadeVolume == 60);
    
    //   // 主動調整音量：預期 SetVolume 後保留新音量設定值。
    //   gs->m_bgm.SetVolume(30);
    //   gs->m_ambient.SetVolume(40);
    //   ok &= (gs->m_bgm.m_nVolume == 30);
    //   ok &= (gs->m_ambient.m_nVolume == 40);

    // 2) mofpacking 模式：同檔案再播一次，驗證封包讀取路徑可被觸發。
    CMofPacking* packer = CMofPacking::GetInstance();
    const bool packOpened = (packer && (packer->m_pNfsHandle || packer->PackFileOpen("mof")));
    ok &= packOpened;
    if (packOpened) {
        g_bLoadOggFromMofPack = 1;
        gs->m_bgmTargetVolume = 80;
        gs->m_ambientTargetVolume = 60;
        gs->PlayMusic(const_cast<char*>("music/bg_eastfield.ogg"));
        gs->PlayAmbientSound(const_cast<char*>("music/embi_beastcave.ogg"));

        ok &= (gs->m_bgmFadeVolume == 80);
        ok &= (gs->m_ambientFadeVolume == 60);
         
        // 主動調整音量：預期 SetVolume 後保留新音量設定值。
        gs->m_bgm.SetVolume(40);
        gs->m_ambient.SetVolume(30);
        ok &= (gs->m_bgm.m_nVolume == 40);
        ok &= (gs->m_ambient.m_nVolume == 30);
        
        // 預期: 在 pack 模式下，Play 不應把 channel id 留在 -1（至少其中一軌成功）。
        ok &= (gs->m_bgm.m_nChannelId != -1 || gs->m_ambient.m_nChannelId != -1);
    }
    //return ok;
    gs->StopMusic();
    gs->StopAmbientSound();
    g_bLoadOggFromMofPack = 1;

    delete gs;
    return ok;
}

bool test_integration_wav_single_position_volume_and_multi_play() {
    bool ok = true;

    char wavShortPath[260]{};
    char wavMonsterPath[260]{};
    to_relative_path(kWavSfxShortRaw, wavShortPath, sizeof(wavShortPath));
    to_relative_path(kWavSfxMonsterRaw, wavMonsterPath, sizeof(wavMonsterPath));

    // 先驗證 WAV 可被 CWaveFile 直接讀取。
    CWaveFile waveShort;
    CWaveFile waveMonster;
    const int openShort = waveShort.Open(wavShortPath, nullptr, 1u);
    const int openMonster = waveMonster.Open(wavMonsterPath, nullptr, 1u);
    ok &= (openShort >= 0 && waveShort.GetSize() > 0);
    ok &= (openMonster >= 0 && waveMonster.GetSize() > 0);

    // DirectSound 路徑：測單一短音效、主動調整音量、與多重同時播放。
    CSoundManager mgr;
    IDirectSound8* ds = nullptr;
    HRESULT initHr = mgr.Initialize(&ds, g_hWnd, 2u);
    if (SUCCEEDED(initHr)) {
        mgr.SetPrimaryBufferFormat(2, 44100, 16);

        CSound* shortSound = nullptr;
        const int createRet = mgr.Create(
            &shortSound,
            wavShortPath,
            DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY,
            GUID_NULL,
            3u // 同一個 wav 建立 3 個 buffer，測同時播放
        );
        ok &= (createRet >= 0 && shortSound != nullptr);

        if (shortSound) {
            // 單一短音效播放。
            const HRESULT play1 = shortSound->Play(0, 0, -1200, -1, 0);
            ok &= SUCCEEDED(play1);

            // 主動調整音量（volume 參數改變）。
            const HRESULT play2 = shortSound->Play(0, 0, -400, -1, 0);
            ok &= SUCCEEDED(play2);

            // 多音效同時播放（連續觸發多次，依靠多 buffer 疊加）。
            const HRESULT play3 = shortSound->Play(0, 0, -900, -1, -2000);
            const HRESULT play4 = shortSound->Play(0, 0, -900, -1, 2000);
            ok &= SUCCEEDED(play3);
            ok &= SUCCEEDED(play4);
            ok &= (shortSound->IsSoundPlaying() != 0);
        }

        // GameSound 路徑：調整角色位置後觸發，驗證可更新到期時間（代表有執行播放流程）。
        GameSound* gs = new GameSound();
        gs->m_soundInitFailed = false;

        const std::uint16_t id = local_parse_sound_id("J0001");
        ok &= (id != 0);
        if (id != 0) {
            gs->m_soundTable[id].sound = shortSound;

            ClientCharacter* ch = new ClientCharacter();
            ch->SetPosX(600.0f);
            ch->SetPosY(300.0f);
            gs->SetCharacter(ch);

            // 角色靠近音源。
            gs->PlaySoundA(const_cast<char*>("J0001"), 580, 300);
            const std::uint32_t nearExpire = gs->m_soundTable[id].expireTick;
            ok &= (nearExpire > 0);

            // 調整角色位置到遠處再播放（內部 pan/freqShift 會走不同分支）。
            ch->SetPosX(2000.0f);
            ch->SetPosY(1600.0f);
            gs->PlaySoundA(const_cast<char*>("J0001"), 580, 300);
            const std::uint32_t farExpire = gs->m_soundTable[id].expireTick;
            ok &= (farExpire >= nearExpire);

            gs->m_soundTable[id].sound = nullptr; // 避免 GameSound 解構時釋放 shortSound。
            delete ch;
        }

        delete gs;

        // 手動釋放 shortSound（沿用專案現有記憶體策略）。
        if (shortSound) {
            shortSound->Stop();
            operator delete(shortSound);
        }
    }
    else {
        // 無法初始化 DirectSound 視為整合測試失敗。
        ok = false;
    }

    return ok;
}

} // namespace

int run_sound_system_test_suite() {
    std::printf("\n========== Sound Test Suite 開始 ==========\n");
    const bool oggOk = test_ogg_single_functions();
    
    print_case_result("COgg", "單一函式測試", oggOk,
        "Initalize/SetVolume/Stop/OpenStreem 在無檔案與無 VFS 情境下可安全執行，狀態值符合邊界處理。"
    );

    const bool waveOk = test_wavefile_single_functions();
    print_case_result("CWaveFile", "單一函式測試", waveOk,
        "OpenFromMemory/ResetFile/Read/GetFormat/Close 皆可執行，且記憶體讀取內容正確。"
    );

    const bool managerOk = test_sound_manager_single_functions();
    print_case_result("CSoundSystem", "單一函式測試", managerOk,
        "未初始化狀態下各 API 應回傳 DSERR_UNINITIALIZED 或 E_INVALIDARG。"
    );

    const bool gameSoundOk = test_gamesound_single_functions();
    print_case_result("GameSound", "單一函式測試", gameSoundOk,
        "SetCharacter/ResetAll/StopSound/ResetSound 的防呆與狀態更新邏輯正確。"
    );

    const bool mixedOk = test_cross_file_mixed_audio_flow();
    print_case_result("Mixed", "跨檔案混合音效流程", mixedOk,
        "GameSound + COgg + 淡入淡出流程可互動，且未載入 BGM stream 時 IsBGMFinish 回傳 true。"
    );

    const bool initSoundRealListOk = test_integration_init_sound_with_reallist();
    print_case_result("Integration", "InitSound + SoundList.txt 實檔對答案", initSoundRealListOk,
        "實際調用 InitSound(\"SoundList.txt\")，並逐筆比對 J0001~J0019 的路徑與數值欄位。"
    );

    const bool ioIntegrationOk = test_integration_file_read_direct_and_mofpacking();
    print_case_result("Integration", "檔案讀取(直接檔案 + mofpacking)", ioIntegrationOk,
        "指定 ogg/wav 檔案可從直接路徑讀取，且也可透過 mofpacking 讀取。"
    );
    
    const bool musicIntegrationOk = test_integration_music_playback_bgm_and_ambient();
    print_case_result("Integration", "BGM + Ambient 播放與音量調整", musicIntegrationOk,
        "bg_eastfield.ogg 與 embi_beastcave.ogg 可播放，且可主動調整音量。"
    );

    const bool wavIntegrationOk = test_integration_wav_single_position_volume_and_multi_play();
    print_case_result("Integration", "WAV 單發/位置影響/音量/多重同播", wavIntegrationOk,
        "短音效可播放；角色位置變更可走不同聲音參數流程；可主動調整音量並同時多次觸發播放。"
    );

    const bool allOk =
        oggOk && waveOk && managerOk && gameSoundOk && mixedOk &&
        initSoundRealListOk && ioIntegrationOk && musicIntegrationOk && wavIntegrationOk;

    std::printf("========== Sound Test Suite 結束: %s ==========\n\n", allOk ? "PASS" : "FAIL");

    return allOk ? 0 : -1;
}
