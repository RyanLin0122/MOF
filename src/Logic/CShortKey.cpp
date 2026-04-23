#include "Logic/CShortKey.h"

#include <cstring>

#include "global.h"
#include "Logic/cltConfiguration.h"
#include "Text/DCTTextManager.h"
#include "UI/CUIBase.h"
#include "UI/CUIBasic.h"
#include "UI/CUIManager.h"
#include "UI/CUIMenu.h"

// ---------------------------------------------------------------------------
// Static storage.
// ---------------------------------------------------------------------------
// Populated by InitStaticDefaultKey().  Layout matches the 56-entry
// m_nDefaultKey table at 0x6DE480 in mofclient.c (key order = setting index).
int CShortKey::m_nDefaultKey[CShortKey::KEY_COUNT] = { 0 };

// Keyboard-layout table populated at startup by AddKeyboard().  Capacity of
// 84 entries mirrors the `_E228_0` per-slot ctor loop in the ground truth.
strBoardKey CShortKey::m_strBoardKey[CShortKey::BOARD_CAPACITY];

// Text IDs used by DCTTextManager::GetParsedText to print localized labels
// for each of the 56 short-key slots (recovered from the binary data section
// at VA 0x006C86AC — Hex-Rays collapsed the whole 112-byte uint16 array into
// the bogus `int m_wUserKeyName = 243928713` declaration at mofclient.c:21630,
// but the raw bytes are intact).  GetUserKeyName() skips index -1 and 42.
// Label column is the Korean string the ID resolves to in MoFTexts.txt; the
// slot order matches the KEYTYPE_* assignments in InitStaticDefaultKey below.
uint16_t CShortKey::m_wUserKeyName[CShortKey::KEY_COUNT] = {
    0x0E89,  // [ 0] 메뉴          — 選單        / Menu               (KEYTYPE_MENU)
    0x0E8A,  // [ 1] 도움말        — 說明        / Help               (KEYTYPE_HELP)
    0x0E8B,  // [ 2] 키입력        — 按鍵設定    / Key setting        (KEYTYPE_KEYSETTING)
    0x0E8C,  // [ 3] 스샷          — 螢幕截圖    / Screenshot         (KEYTYPE_SCREENCAPTURE)
    0x20B8,  // [ 4] 동영상 찍기   — 錄影        / Record video (AVI) (KEYTYPE_MAKEAVI)
    0x0E8D,  // [ 5] 슬롯1         — 快捷列 1    / QSL slot 1         (KEYTYPE_QSL1)
    0x0E8E,  // [ 6] 슬롯2         — 快捷列 2    / QSL slot 2
    0x0E8F,  // [ 7] 슬롯3         — 快捷列 3    / QSL slot 3
    0x0E90,  // [ 8] 슬롯4         — 快捷列 4    / QSL slot 4
    0x0E91,  // [ 9] 슬롯5         — 快捷列 5    / QSL slot 5
    0x0E92,  // [10] 슬롯6         — 快捷列 6    / QSL slot 6
    0x0E93,  // [11] 슬롯7         — 快捷列 7    / QSL slot 7
    0x0E94,  // [12] 슬롯8         — 快捷列 8    / QSL slot 8
    0x1FF2,  // [13] 슬롯9         — 快捷列 9    / QSL slot 9
    0x1FF3,  // [14] 슬롯 10       — 快捷列 10   / QSL slot 10
    0x1FF4,  // [15] 슬롯 11       — 快捷列 11   / QSL slot 11
    0x1FF5,  // [16] 슬롯 12       — 快捷列 12   / QSL slot 12        (KEYTYPE_QSL12)
    0x0E95,  // [17] 퀘스트        — 任務        / Quest              (KEYTYPE_QUEST)
    0x0E96,  // [18] 월드맵        — 世界地圖    / World map          (KEYTYPE_WORLDMAP)
    0x0E97,  // [19] 장비          — 裝備        / Equipment          (KEYTYPE_EQUIP)
    0x0E98,  // [20] 내보관함      — 倉庫        / Inventory          (KEYTYPE_INVEN)
    0x0E99,  // [21] 환경설정      — 環境設定    / Options            (KEYTYPE_OPTION)
    0x0E9A,  // [22] 친구          — 好友        / Friend             (KEYTYPE_FRIEND)
    0x0E9B,  // [23] 다이어리      — 日記        / Diary              (KEYTYPE_DIARY)
    0x0E9C,  // [24] 스킬          — 技能        / Skill              (KEYTYPE_SKILL)
    0x0E9E,  // [25] 내정보        — 角色資訊    / My info            (KEYTYPE_MYINFO)
    0x0E9F,  // [26] 미니맵        — 小地圖      / Minimap            (KEYTYPE_MINIMAP)
    0x0EA2,  // [27] 채팅창        — 開啟聊天    / Chat start         (KEYTYPE_CHATSTART)
    0x0EA7,  // [28] 채팅닫기      — 關閉聊天    / Chat end           (KEYTYPE_CHATEND)
    0xE2D5,  // [29] 기본대화      — 一般聊天    / Basic chat         (KEYTYPE_BASICCHAT)
    0x0F5C,  // [30] 파티대화      — 組隊聊天    / Party chat         (KEYTYPE_PARTYCHAT)
    0x0F5D,  // [31] 서클대화      — 公會聊天    / Circle/guild chat  (KEYTYPE_CIRCLECHAT)
    0x0F5E,  // [32] 대답          — 悄悄話回覆  / Whisper reply      (KEYTYPE_WHISPERCHAT)
    0x10E2,  // [33] 슬롯↑         — 快捷列上頁  / QSL page up        (KEYTYPE_QSLPAGEUP)
    0x10E3,  // [34] 슬롯↓         — 快捷列下頁  / QSL page down      (KEYTYPE_QSLPAGEDOWN)
    0x1184,  // [35] 달리기        — 跑步切換    / Run toggle         (KEYTYPE_RUN)
    0x1185,  // [36] 정보          — 資訊        / Info               (KEYTYPE_INFO)
    0x1186,  // [37] 서클          — 公會視窗    / Circle/guild       (KEYTYPE_CIRCLE)
    0x1187,  // [38] 전공          — 專精        / Major / profession (KEYTYPE_MAJOR)
    0x1188,  // [39] 공적          — 功績        / Meritorious deeds  (KEYTYPE_MERITORIOUS)
    0x1189,  // [40] 대항전        — 對抗戰      / Hunt / PvP         (KEYTYPE_HUNT)
    0x118A,  // [41] 엠블렘        — 徽章        / Emblem             (KEYTYPE_EMBLEM)
    0x12AF,  // [42] 타겟변경      — 切換目標    / Change mob target  (KEYTYPE_CHANGEMOPTARGET — GetUserKeyName skips this slot)
    0x133E,  // [43] 퀵채팅        — 快捷聊天    / Quick chat         (KEYTYPE_QUICKCHAT)
    0x0F7B,  // [44] 펫            — 寵物        / Pet                (KEYTYPE_PET)
    0x1FAE,  // [45] 마이템        — 我的道具    / My item            (KEYTYPE_MYITEM)
    0x1FD7,  // [46] UI 숨기기     — 隱藏 UI     / Hide UI            (KEYTYPE_HIDEUI)
    0x1FF1,  // [47] 수업          — 技能修練    / Academy / lesson   (KEYTYPE_ACADEMY)
    0x11AB,  // [48] 퀘스트알리미  — 任務提示    / Quest alarm        (KEYTYPE_QUESTALARM)
    0x0EA1,  // [49] 동료지정      — 指定僚機    / Change companion   (KEYTYPE_CHANGETARGET)
    0x0E9D,  // [50] 줍기          — 拾取        / Pick up / Get item (KEYTYPE_GETITEM)
    0x0EA0,  // [51] 공격          — 攻擊        / Attack             (KEYTYPE_ATTACK)
    0x0EA3,  // [52] 상            — 上          / Move up            (KEYTYPE_MOVEUP)
    0x0EA4,  // [53] 좌            — 左          / Move left          (KEYTYPE_MOVELEFT)
    0x0EA5,  // [54] 우            — 右          / Move right         (KEYTYPE_MOVERIGHT)
    0x0EA6,  // [55] 하            — 下          / Move down          (KEYTYPE_MOVEDOWN)
};

int CShortKey::s_nKeyCount = 0;

// ---------------------------------------------------------------------------
// strBoardKey
// ---------------------------------------------------------------------------
strBoardKey::strBoardKey() {
    Init();
}

void strBoardKey::Init() {
    scanCode  = -1;
    x         = 0;
    y         = 0;
    w1        = 0;
    w2        = 0;
    isSetable = 0;
    name[0]   = '\0';
}

void strBoardKey::Set(int a2, int a3, int a4, const char* a5, int a6, uint16_t a7, uint16_t a8) {
    // Field-write order mirrors mofclient.c exactly.
    x         = a3;
    scanCode  = a2;
    w1        = a7;
    y         = a4;
    isSetable = a6;
    w2        = a8;
    std::strcpy(name, a5);
}

int strBoardKey::IsSetableKey() const {
    return isSetable;
}

// ---------------------------------------------------------------------------
// CShortKey — per-instance
// ---------------------------------------------------------------------------
// 004E00A0
void CShortKey::ReadKeySetting() {
    // cltConfiguration stores the INI filename at offset 0 of its object,
    // so passing the object pointer as LPCSTR is how mofclient.c reaches
    // the profile-file path.
    g_clConfig->GetKeySetting(reinterpret_cast<LPCSTR>(g_clConfig), m_nSettedKey);
    CopySettedKeyToSettingKey();
    ProcessInvalidKey();
    AdjustClientKey();
}

// 004E00D0
void CShortKey::CopySettedKeyToSettingKey() {
    std::memcpy(m_nSettingKey, m_nSettedKey, sizeof(m_nSettedKey));
}

// 004E00F0
void CShortKey::CopySettingKeyToSettedKey() {
    std::memcpy(m_nSettedKey, m_nSettingKey, sizeof(m_nSettedKey));
}

// 004E0110
void CShortKey::SaveKeySetting() {
    CopySettingKeyToSettedKey();
    g_clConfig->SaveKeySetting(reinterpret_cast<LPCSTR>(g_clConfig), m_nSettedKey);
    AdjustClientKey();
}

// 004E0130 — scans the working-copy bindings, collapses duplicates to an
// index-tagged sentinel, and falls back to the default table when required
// bindings (movement keys) have been lost.
void CShortKey::ProcessInvalidKey() {
    // 1:1 with mofclient.c: `int v8; char v9[1016];` on the stack forms a
    // 1020-byte (= 255 int) seen-flag table indexed by the scan code value.
    int seen[255] = { 0 };
    int anyFixed = 0;

    for (unsigned int i = 0; i < KEY_COUNT; ++i) {
        int key = m_nSettingKey[i];
        if (static_cast<unsigned int>(key) < INVALID_KEY) {
            if (seen[key]) {
                SendExtraKeySpace(static_cast<int>(i));
                anyFixed = 1;
            } else {
                seen[key] = 1;
            }
        }
    }

    if (anyFixed) {
        SaveKeySetting();
    }
    if (!IsRequiredKeyInKeyboard()) {
        SetAllDefaultKey();
        SaveKeySetting();
    }
}

// 004E01C0 — the seven entries checked here are the movement / attack keys
// (indices 49..55) that the game cannot operate without.
BOOL CShortKey::IsRequiredKeyInKeyboard() {
    for (int i = 49; i <= 55; ++i) {
        if (static_cast<unsigned int>(m_nSettingKey[i]) >= INVALID_KEY) {
            return FALSE;
        }
    }
    return TRUE;
}

// 004E0210
void CShortKey::SetAllDefaultKey() {
    std::memcpy(m_nSettingKey, m_nDefaultKey, sizeof(m_nDefaultKey));
}

// 004E0230 — marks the indicated setting slot as unbound, using an index-tagged
// sentinel (high byte = slot index) so later passes can tell different slots
// apart when comparing against each other.
// Ground truth does NOT bounds-check a2 — writes *((_DWORD*)this + a2 + 56)
// directly.  Callers (ProcessInvalidKey) always pass a valid slot.
void CShortKey::SendExtraKeySpace(int a2) {
    m_nSettingKey[a2] = static_cast<int>(INVALID_KEY | (static_cast<unsigned int>(a2) << 24));
}

// 004E0260
void* CShortKey::GetSettingKey(int a2) {
    if (a2 < 0 || a2 > static_cast<int>(KEY_COUNT)) {
        // Out-of-range returns a high sentinel (address of unk_1FFFFFF in the
        // decompilation) — callers only compare it to INVALID_KEY to detect
        // "no binding", so any value >= 0xFFFFFF works.
        return reinterpret_cast<void*>(static_cast<uintptr_t>(0x1FFFFFF));
    }
    return reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<unsigned int>(m_nSettingKey[a2])));
}

// 004E0290
void CShortKey::SetSettingKey(int a2, int a3) {
    if (a2 >= 0 && a2 <= static_cast<int>(KEY_COUNT)) {
        m_nSettingKey[a2] = a3;
    }
}

// 004E02B0 — the ground truth indexes by the raw a2 without bounds-checking
// (callers always pass a valid slot), so we reproduce that.
int CShortKey::GetShortKey(int a2) {
    return m_nSettedKey[a2];
}

// ---------------------------------------------------------------------------
// CShortKey — static helpers
// ---------------------------------------------------------------------------
// 004E02C0 — linear scan.  Matches behaviour of the ground truth: stops on
// the first entry whose scanCode field equals `a1`, returns -1 if we fall off
// the end of the table.
int CShortKey::FindKeyBoardIndex(int a1) {
    for (int i = 0; i < BOARD_CAPACITY; ++i) {
        if (m_strBoardKey[i].scanCode == a1) {
            return i;
        }
    }
    return -1;
}

// 004E02F0
strBoardKey* CShortKey::GetBoardKey(int a1) {
    return &m_strBoardKey[a1];
}

// 004E0310 — appends a new keyboard descriptor and bumps the slot counter.
void CShortKey::AddKeyboard(int a1, int a2, int a3, const char* a4,
                            int a5, uint16_t a6, uint16_t a7) {
    m_strBoardKey[s_nKeyCount].Set(a1, a2, a3, a4, a5, a6, a7);
    ++s_nKeyCount;
}

// 004E0360 — returns the DCT-managed label string for a short-key slot.
// a1 == -1 and a1 == 42 are legitimate "no label" conditions from the ground
// truth; everything else resolves to m_wUserKeyName[a1].
char* CShortKey::GetUserKeyName(int a1) {
    static char s_empty[1] = { '\0' };
    if (a1 == -1)  return s_empty;
    if (a1 == 42)  return s_empty;
    return g_DCTTextManager.GetParsedText(m_wUserKeyName[a1], 1, nullptr);
}

// 004E03A0 — returns the printable label for whatever scan code is currently
// bound to slot `a1` of the working-copy.  `a2` selects whether unbound slots
// get the placeholder "none" string (GetText(3947)) or an empty string.
char* CShortKey::GetUserKeySettingName(int a1, int a2) {
    static char s_empty[1] = { '\0' };
    if (a1 != -1) {
        void* v2 = GetSettingKey(g_pShortKeyList, a1);
        uintptr_t raw = reinterpret_cast<uintptr_t>(v2);
        if (raw >= INVALID_KEY && a2) {
            return g_DCTTextManager.GetText(3947);
        }
        int v4 = FindKeyBoardIndex(static_cast<int>(raw));
        if (v4 != -1) {
            return m_strBoardKey[v4].name;
        }
        if (a2) {
            return g_DCTTextManager.GetText(3947);
        }
    }
    return s_empty;
}

// 004E0410 — pushes the freshly-committed bindings to every UI widget that
// displays a short-key label.  Called after SaveKeySetting / ReadKeySetting.
void CShortKey::AdjustClientKey() {
    // ---- Basic UI (window 0): QSL bar, QSL page buttons, main menu ----
    CUIBasic* pBasic = static_cast<CUIBasic*>(g_UIMgr ? g_UIMgr->GetUIWindow(0) : nullptr);
    if (pBasic) {
        char qslNames[12][256];
        std::memset(qslNames, 0, sizeof(qslNames));
        for (int i = 0; i < 12; ++i) {
            const char* src;
            int setting = m_nSettingKey[5 + i];
            if (static_cast<unsigned int>(setting) < INVALID_KEY) {
                int idx = FindKeyBoardIndex(setting);
                src = (idx != -1) ? m_strBoardKey[idx].name : "";
            } else {
                src = "";
            }
            std::memcpy(qslNames[i], src, std::strlen(src) + 1);
        }
        pBasic->SetQSLShortKeyName(qslNames);

        char qslPageNames[2][256];
        std::memset(qslPageNames, 0, sizeof(qslPageNames));
        for (int i = 0; i < 2; ++i) {
            const char* src = GetUserKeySettingName(i + 33, 1);
            std::memcpy(qslPageNames[i], src, std::strlen(src) + 1);
        }
        pBasic->SetQSLPageShortKeyName(qslPageNames);

        int menuKeys[10];
        std::memcpy(menuKeys, pBasic->GetMenuKeyIndex(), sizeof(menuKeys));
        char menuNames[10][256];
        std::memset(menuNames, 0, sizeof(menuNames));
        for (int i = 0; i < 10; ++i) {
            const char* src = GetUserKeySettingName(menuKeys[i], 1);
            std::memcpy(menuNames[i], src, std::strlen(src) + 1);
        }
        pBasic->SetMenuShortKeyName(menuNames);
    }

    // ---- Main-menu UI (window 1) ----
    CUIMenu* pMenu = static_cast<CUIMenu*>(g_UIMgr ? g_UIMgr->GetUIWindow(1) : nullptr);
    if (pMenu) {
        int menuKeys[6];
        std::memcpy(menuKeys, pMenu->GetMenuKeyIndex(), sizeof(menuKeys));
        char menuNames[6][256];
        std::memset(menuNames, 0, sizeof(menuNames));
        for (int i = 0; i < 6; ++i) {
            const char* src = GetUserKeySettingName(menuKeys[i], 1);
            std::memcpy(menuNames[i], src, std::strlen(src) + 1);
        }
        pMenu->SetMenuShortKeyName(menuNames);
    }

    // ---- Window 84: writes the QUESTALARM label into its internal buffer
    // at offset 0x226DC.  Full widget not yet restored — the lookup returns
    // nullptr and the branch is skipped at runtime.  We still issue the same
    // GetUserKeySettingName(48, 1) call so the ground-truth call chain runs.
    CUIBase* pW84 = g_UIMgr ? g_UIMgr->GetUIWindow(84) : nullptr;
    if (pW84) {
        // Ground truth: 64-byte stack buffer cleared then strcpy'd to both the
        // local copy and the window+140956 slot.
        char buf[64];
        buf[0] = '\0';
        std::memset(buf + 1, 0, sizeof(buf) - 1);
        const char* label = GetUserKeySettingName(48, 1);
        std::strcpy(buf, label);
        std::strcpy(reinterpret_cast<char*>(pW84) + 140956, buf);
    }
}

// ---------------------------------------------------------------------------
// 004E06A0 — seeds the default-key table and populates the on-screen keyboard
// layout.  Called once at program start-up (right before the first
// ReadKeySetting).
// ---------------------------------------------------------------------------
void CShortKey::InitStaticDefaultKey() {
    // Default-key table — mofclient.c initializes per-slot in the order the
    // dword_6DExxx symbols appear.  Preserve that exact ordering of writes.
    m_nDefaultKey[0]  = 1;          // ESC            (KEYTYPE_MENU)
    m_nDefaultKey[1]  = 59;         // F1             (KEYTYPE_HELP)
    m_nDefaultKey[2]  = 60;         // F2             (KEYTYPE_KEYSETTING)
    m_nDefaultKey[3]  = 183;        // PrintScreen    (KEYTYPE_SCREENCAPTURE)
    m_nDefaultKey[4]  = 70;         // ScrollLock     (KEYTYPE_MAKEAVI)
    m_nDefaultKey[5]  = 2;          // 1              (KEYTYPE_QSL1)
    m_nDefaultKey[6]  = 3;          // 2
    m_nDefaultKey[7]  = 4;          // 3
    m_nDefaultKey[8]  = 5;          // 4
    m_nDefaultKey[9]  = 6;          // 5
    m_nDefaultKey[10] = 7;          // 6
    m_nDefaultKey[11] = 8;          // 7
    m_nDefaultKey[12] = 9;          // 8
    m_nDefaultKey[13] = 10;         // 9
    m_nDefaultKey[14] = 11;         // 0
    m_nDefaultKey[15] = 12;         // -
    m_nDefaultKey[16] = 13;         // =              (KEYTYPE_QSL12)
    m_nDefaultKey[17] = 16;         // Q              (KEYTYPE_QUEST)
    m_nDefaultKey[18] = 17;         // W              (KEYTYPE_WORLDMAP)
    m_nDefaultKey[19] = 18;         // E              (KEYTYPE_EQUIP)
    m_nDefaultKey[20] = 23;         // I              (KEYTYPE_INVEN)
    m_nDefaultKey[21] = 24;         // O              (KEYTYPE_OPTION)
    m_nDefaultKey[22] = 33;         // F              (KEYTYPE_FRIEND)
    m_nDefaultKey[23] = 32;         // D              (KEYTYPE_DIARY)
    m_nDefaultKey[24] = 31;         // S              (KEYTYPE_SKILL)
    m_nDefaultKey[50] = 44;         // Z              (KEYTYPE_GETITEM)
    m_nDefaultKey[25] = 46;         // C              (KEYTYPE_MYINFO)
    m_nDefaultKey[26] = 50;         // M              (KEYTYPE_MINIMAP)
    m_nDefaultKey[51] = 29;         // LCtrl          (KEYTYPE_ATTACK)
    m_nDefaultKey[49] = 56;         // LAlt           (KEYTYPE_CHANGETARGET)
    m_nDefaultKey[27] = 28;         // Enter          (KEYTYPE_CHATSTART)
    m_nDefaultKey[28] = 15;         // Tab            (KEYTYPE_CHATEND)
    m_nDefaultKey[52] = 200;        // Up             (KEYTYPE_MOVEUP)
    m_nDefaultKey[53] = 203;        // Left           (KEYTYPE_MOVELEFT)
    m_nDefaultKey[54] = 205;        // Right          (KEYTYPE_MOVERIGHT)
    m_nDefaultKey[55] = 208;        // Down           (KEYTYPE_MOVEDOWN)
    m_nDefaultKey[29] = 63;         // F5             (KEYTYPE_BASICCHAT)
    m_nDefaultKey[30] = 64;         // F6             (KEYTYPE_PARTYCHAT)
    m_nDefaultKey[31] = 65;         // F7             (KEYTYPE_CIRCLECHAT)
    m_nDefaultKey[32] = 66;         // F8             (KEYTYPE_WHISPERCHAT)
    m_nDefaultKey[33] = 201;        // PgUp           (KEYTYPE_QSLPAGEUP)
    m_nDefaultKey[34] = 209;        // PgDown         (KEYTYPE_QSLPAGEDOWN)
    m_nDefaultKey[35] = 19;         // R              (KEYTYPE_RUN)
    m_nDefaultKey[36] = 34;         // G              (KEYTYPE_INFO)
    m_nDefaultKey[37] = 35;         // H              (KEYTYPE_CIRCLE)
    m_nDefaultKey[38] = 36;         // J              (KEYTYPE_MAJOR)
    m_nDefaultKey[39] = 37;         // K              (KEYTYPE_MERITORIOUS)
    m_nDefaultKey[40] = 38;         // L              (KEYTYPE_HUNT)
    m_nDefaultKey[41] = 39;         // ;              (KEYTYPE_EMBLEM)
    m_nDefaultKey[42] = 61;         // F3             (KEYTYPE_CHANGEMOPTARGET)
    m_nDefaultKey[43] = 62;         // F4             (KEYTYPE_QUICKCHAT)
    m_nDefaultKey[44] = 25;         // P              (KEYTYPE_PET)
    m_nDefaultKey[45] = 21;         // Y              (KEYTYPE_MYITEM)
    m_nDefaultKey[46] = 22;         // U              (KEYTYPE_HIDEUI)
    m_nDefaultKey[47] = 20;         // T              (KEYTYPE_ACADEMY)
    m_nDefaultKey[48] = 30;         // A              (KEYTYPE_QUESTALARM)

    // On-screen keyboard layout.  Ground truth does NOT reset _nKeyCount here —
    // it relies on the BSS-zero initial value and on InitStaticDefaultKey being
    // called only once at program startup.  Match that behaviour exactly.
    AddKeyboard(1,   16,  39, "ESC",   0, 0x25u, 0x25u);
    AddKeyboard(59,  94,  39, "F1",    1, 0x25u, 0x25u);
    AddKeyboard(60,  133, 39, "F2",    1, 0x25u, 0x25u);
    AddKeyboard(61,  172, 39, "F3",    1, 0x25u, 0x25u);
    AddKeyboard(62,  211, 39, "F4",    1, 0x25u, 0x25u);
    AddKeyboard(63,  269, 39, "F5",    1, 0x25u, 0x25u);
    AddKeyboard(64,  308, 39, "F6",    1, 0x25u, 0x25u);
    AddKeyboard(65,  347, 39, "F7",    1, 0x25u, 0x25u);
    AddKeyboard(66,  386, 39, "F8",    1, 0x25u, 0x25u);
    AddKeyboard(67,  445, 39, "F9",    1, 0x25u, 0x25u);
    AddKeyboard(68,  484, 39, "F10",   1, 0x25u, 0x25u);
    AddKeyboard(87,  523, 39, "F11",   1, 0x25u, 0x25u);
    AddKeyboard(88,  562, 39, "F12",   1, 0x25u, 0x25u);
    AddKeyboard(183, 621, 39, "",      0, 0x25u, 0x25u);
    AddKeyboard(70,  660, 39, "",      1, 0x25u, 0x25u);
    AddKeyboard(197, 699, 39, "",      1, 0x25u, 0x25u);
    AddKeyboard(41,  16,  89, "`",     1, 0x25u, 0x25u);
    AddKeyboard(2,   55,  89, "1",     1, 0x25u, 0x25u);
    AddKeyboard(3,   94,  89, "2",     1, 0x25u, 0x25u);
    AddKeyboard(4,   133, 89, "3",     1, 0x25u, 0x25u);
    AddKeyboard(5,   172, 89, "4",     1, 0x25u, 0x25u);
    AddKeyboard(6,   211, 89, "5",     1, 0x25u, 0x25u);
    AddKeyboard(7,   250, 89, "6",     1, 0x25u, 0x25u);
    AddKeyboard(8,   289, 89, "7",     1, 0x25u, 0x25u);
    AddKeyboard(9,   328, 89, "8",     1, 0x25u, 0x25u);
    AddKeyboard(10,  367, 89, "9",     1, 0x25u, 0x25u);
    AddKeyboard(11,  406, 89, "0",     1, 0x25u, 0x25u);
    AddKeyboard(12,  445, 89, "-",     1, 0x25u, 0x25u);
    AddKeyboard(13,  484, 89, "=",     1, 0x25u, 0x25u);
    AddKeyboard(43,  523, 89, "\\",    1, 0x25u, 0x25u);
    AddKeyboard(14,  562, 89, "",      1, 0x25u, 0x25u);
    AddKeyboard(15,  16,  128, "",     1, 0x36u, 0x25u);
    AddKeyboard(16,  72,  128, "Q",    1, 0x25u, 0x25u);
    AddKeyboard(17,  111, 128, "W",    1, 0x25u, 0x25u);
    AddKeyboard(18,  150, 128, "E",    1, 0x25u, 0x25u);
    AddKeyboard(19,  189, 128, "R",    1, 0x25u, 0x25u);
    AddKeyboard(20,  228, 128, "T",    1, 0x25u, 0x25u);
    AddKeyboard(21,  267, 128, "Y",    1, 0x25u, 0x25u);
    AddKeyboard(22,  306, 128, "U",    1, 0x25u, 0x25u);
    AddKeyboard(23,  345, 128, "I",    1, 0x25u, 0x25u);
    AddKeyboard(24,  384, 128, "O",    1, 0x25u, 0x25u);
    AddKeyboard(25,  423, 128, "P",    1, 0x25u, 0x25u);
    AddKeyboard(30,  89,  167, "A",    1, 0x25u, 0x25u);
    AddKeyboard(31,  128, 167, "S",    1, 0x25u, 0x25u);
    AddKeyboard(32,  167, 167, "D",    1, 0x25u, 0x25u);
    AddKeyboard(33,  206, 167, "F",    1, 0x25u, 0x25u);
    AddKeyboard(34,  245, 167, "G",    1, 0x25u, 0x25u);
    AddKeyboard(35,  284, 167, "H",    1, 0x25u, 0x25u);
    AddKeyboard(36,  323, 167, "J",    1, 0x25u, 0x25u);
    AddKeyboard(37,  362, 167, "K",    1, 0x25u, 0x25u);
    AddKeyboard(38,  401, 167, "L",    1, 0x25u, 0x25u);
    AddKeyboard(44,  106, 206, "Z",    1, 0x25u, 0x25u);
    AddKeyboard(45,  145, 206, "X",    1, 0x25u, 0x25u);
    AddKeyboard(46,  184, 206, "C",    1, 0x25u, 0x25u);
    AddKeyboard(47,  223, 206, "V",    1, 0x25u, 0x25u);
    AddKeyboard(48,  262, 206, "B",    1, 0x25u, 0x25u);
    AddKeyboard(49,  301, 206, "N",    1, 0x25u, 0x25u);
    AddKeyboard(50,  340, 206, "M",    1, 0x25u, 0x25u);
    AddKeyboard(28,  540, 137, "Enter",1, 0x39u, 0x4Au);
    AddKeyboard(58,  16,  167, "CL",   1, 0x47u, 0x25u);
    AddKeyboard(42,  16,  206, "Shift",1, 0x25u, 0x25u);
    AddKeyboard(54,  496, 206, "Shift",1, 0x25u, 0x25u);
    AddKeyboard(29,  16,  245, "Ctrl", 1, 0x47u, 0x25u);
    AddKeyboard(157, 528, 245, "Ctrl", 1, 0x47u, 0x25u);
    AddKeyboard(56,  89,  245, "",     1, 0x47u, 0x25u);
    AddKeyboard(184, 455, 245, "",     1, 0x47u, 0x25u);
    AddKeyboard(57,  162, 245, "",     1, 0x123u, 0x25u);
    AddKeyboard(26,  462, 128, "[",    1, 0x25u, 0x25u);
    AddKeyboard(27,  501, 128, "]",    1, 0x25u, 0x25u);
    AddKeyboard(39,  440, 167, ";",    1, 0x25u, 0x25u);
    AddKeyboard(40,  479, 167, "'",    1, 0x25u, 0x25u);
    AddKeyboard(51,  379, 206, ",",    1, 0x25u, 0x25u);
    AddKeyboard(52,  418, 206, "",     1, 0x25u, 0x25u);
    AddKeyboard(53,  457, 206, "/",    1, 0x25u, 0x25u);
    AddKeyboard(210, 621, 89,  "",     1, 0x25u, 0x25u);
    AddKeyboard(211, 621, 128, "",     1, 0x25u, 0x25u);
    AddKeyboard(199, 660, 89,  "",     1, 0x25u, 0x25u);
    AddKeyboard(207, 660, 128, "",     1, 0x25u, 0x25u);
    AddKeyboard(201, 699, 89,  "",     1, 0x25u, 0x25u);
    AddKeyboard(209, 699, 128, "",     1, 0x25u, 0x25u);
    AddKeyboard(200, 660, 206, "UP",   1, 0x25u, 0x25u);
    AddKeyboard(203, 621, 245, "",     1, 0x25u, 0x25u);
    AddKeyboard(205, 699, 245, "",     1, 0x25u, 0x25u);
    AddKeyboard(208, 660, 245, "Down", 1, 0x25u, 0x25u);
}
