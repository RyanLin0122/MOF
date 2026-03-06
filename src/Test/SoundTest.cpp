#include "Test/SoundTest.h"

#include <cstdio>
#include <cstring>

#include "Sound/GameSound.h"

#include "Sound/COgg.h"
#include "Sound/CSoundSystem.h"
#include "Sound/CWaveFile.h"
#include "global.h"

namespace {

void print_case_result(const char* suite, const char* caseName, bool pass, const char* expected) {
    std::printf("[%s] %s: %s\n", suite, caseName, pass ? "PASS" : "FAIL");
    std::printf("    預期結果: %s\n", expected);
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

    // ResetAll 在 activeIds 不存在時: 預期不修改 masterDb。
    gs->m_masterDb = -777;
    gs->ResetAll();
    ok &= (gs->m_masterDb == -777);

    // 補上 activeIds 後再 ResetAll: 預期 masterDb 被重設為 0，初始化失敗旗標清除。
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

    // 未載入 stream 時 IsBGMFinish: 預期回傳 true。
    ok &= (gs->IsBGMFinish() == 1);

    gs->StopMusic();
    gs->StopAmbientSound();

    delete gs;
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

    const bool allOk = oggOk && waveOk && managerOk && gameSoundOk && mixedOk;
    std::printf("========== Sound Test Suite 結束: %s ==========\n\n", allOk ? "PASS" : "FAIL");

    return allOk ? 0 : -1;
}
