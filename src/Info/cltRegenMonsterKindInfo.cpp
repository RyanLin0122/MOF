#include "Info/cltRegenMonsterKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// ---------------------------------------------------------------------------
// 반편역 mofclient.c:329365-329380
// 5 글자, 첫 글자 알파벳 + 4 자리 십진수, atoi(s+1) < 0x800.
// 與 cltCharKindInfo / cltMapInfo / cltExtraRegenMonsterKindInfo 同 body。
// ---------------------------------------------------------------------------
uint16_t cltRegenMonsterKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const uint16_t num = static_cast<uint16_t>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

// ---------------------------------------------------------------------------
// cltRegenMonsterKindInfo::Initialize  ── mofclient.c:329239-329332
//
// 解析 RegenMonsterInfo.txt（CP949/EUC-KR）。流程逐字對齊反編譯：
//
//   1. _fopen 失敗 → return 0
//   2. fgets x3 跳過表頭；任一失敗即 fclose 回傳 v18 = 0。
//   3. fgetpos → 預掃資料行數，存入 m_count；m_items = operator new(32*m_count)。
//      ※ 不 memset 0：未初始化記憶體，與 GT 一致。
//   4. fsetpos 回資料起點。
//   5. fgets 第一筆資料行；失敗 → v18 = 1 (LABEL_21)。
//   6. 對每行：
//        [0] kind        ← TranslateKindCode；v7==0 break (v18 不變)
//        [1] 이름         ← strtok 丟棄
//        [2] charKind    ← cltCharKindInfo::TranslateKindCode
//        [3] 맵 이름      ← strtok 丟棄
//        [4] mapID       ← cltMapInfo::TranslateKindCode
//        [5] x           ← atoi → +8
//        [6] y           ← atoi → +12
//        [7] width       ← +16 = +8 + atoi(width)   (右邊界)
//        [8] height      ← +20 = +12 + atoi(height) (下邊界)
//        [9] maxCount    ← atoi → +24
//        [10] interval   ← v5+=32; *(v5-4) = 1000*atoi   (毫秒)
//        fgets 下一行；失敗 → goto LABEL_21 (v18 = 1)。
//      任何中途 strtok==NULL 即 break，v18 維持 0。
//   7. fclose; return v18.
//
// 重要的反編譯細節：
//   * v18 預設為 0；只有 (a) 全部 fgets 成功直到 EOF (clean EOF) 才會被設成 1。
//   * 半解析行的尾段欄位保留 operator new 後未初始化的內容。
//   * m_count 永遠 = 預掃行數（含未完整解析的尾段），呼叫端讀取需自行檢查 .kind == 0。
// ---------------------------------------------------------------------------
int cltRegenMonsterKindInfo::Initialize(char* String2) {
    if (!String2) return 0;

    char Delimiter[4];
    Delimiter[0] = '\t'; Delimiter[1] = '\n';
    Delimiter[2] = '\0'; Delimiter[3] = '\0';

    FILE* v3 = g_clTextFileManager.fopen(String2);
    FILE* v4 = v3;
    if (!v3) return 0;

    int v18 = 0;
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    if (std::fgets(Buffer, 1023, v3)
        && std::fgets(Buffer, 1023, v4)
        && std::fgets(Buffer, 1023, v4))
    {
        fpos_t Position{};
        std::fgetpos(v4, &Position);

        // 預掃資料行數，寫入 m_count。
        for (; std::fgets(Buffer, 1023, v4); ++m_count)
            ;

        // operator new(32 * m_count)。當 m_count==0 仍呼叫 operator new(0)
        // (得到合法非空指標)；後續 fgets 必定失敗，落到 LABEL_21。
        m_items = static_cast<strRegenMonsterKindInfo*>(
            operator new(sizeof(strRegenMonsterKindInfo) * static_cast<size_t>(m_count)));

        std::fsetpos(v4, &Position);

        // v5 對齊反編譯：當前資料行寫入點。
        char* v5 = reinterpret_cast<char*>(m_items);

        if (std::fgets(Buffer, 1023, v4)) {
            for (;;) {
                // [0] ID
                char* v6 = std::strtok(Buffer, Delimiter);
                if (!v6) break;
                uint16_t v7 = TranslateKindCode(v6);
                *reinterpret_cast<uint16_t*>(v5 + 0) = v7;
                if (!v7) break;

                // [1] 이름(기획자용) ── 丟棄
                if (!std::strtok(nullptr, Delimiter)) break;

                // [2] 종류 ID
                char* v8 = std::strtok(nullptr, Delimiter);
                if (!v8) break;
                *reinterpret_cast<uint16_t*>(v5 + 2) = cltCharKindInfo::TranslateKindCode(v8);

                // [3] 맵 이름(기획자용) ── 丟棄
                if (!std::strtok(nullptr, Delimiter)) break;

                // [4] 맵 ID
                char* v9 = std::strtok(nullptr, Delimiter);
                if (!v9) break;
                *reinterpret_cast<uint16_t*>(v5 + 4) = cltMapInfo::TranslateKindCode(v9);

                // [5] x
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                *reinterpret_cast<int32_t*>(v5 + 8) = std::atoi(v10);

                // [6] y
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                *reinterpret_cast<int32_t*>(v5 + 12) = std::atoi(v11);

                // [7] width → +16 = x + width  (右邊界絕對座標)
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                *reinterpret_cast<int32_t*>(v5 + 16)
                    = *reinterpret_cast<int32_t*>(v5 + 8) + std::atoi(v12);

                // [8] height → +20 = y + height (下邊界絕對座標)
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                *reinterpret_cast<int32_t*>(v5 + 20)
                    = *reinterpret_cast<int32_t*>(v5 + 12) + std::atoi(v13);

                // [9] 최대수
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14) break;
                *reinterpret_cast<int32_t*>(v5 + 24) = std::atoi(v14);

                // [10] 리젠 간격(초) → 1000 * (毫秒)
                char* v15 = std::strtok(nullptr, Delimiter);
                if (!v15) break;
                v5 += 32;
                *reinterpret_cast<int32_t*>(v5 - 4) = 1000 * std::atoi(v15);

                if (!std::fgets(Buffer, 1023, v4)) {
                    // LABEL_21: clean EOF
                    v18 = 1;
                    break;
                }
            }
        } else {
            // LABEL_21: 第一行 fgets 即失敗 (m_count==0 等情境)
            v18 = 1;
        }
    }

    g_clTextFileManager.fclose(v4);
    return v18;
}

// 반편역 mofclient.c:329336-329344
void cltRegenMonsterKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// 반편역 mofclient.c:329353-329362
strRegenMonsterKindInfo*
cltRegenMonsterKindInfo::GetRegenMonsterKindInfoByIndex(int a2) {
    if (a2 < 0 || a2 >= m_count) return nullptr;
    return reinterpret_cast<strRegenMonsterKindInfo*>(
        reinterpret_cast<char*>(m_items) + 32 * a2);
}

// ---------------------------------------------------------------------------
// 반편역 mofclient.c:329383-329428
//
// 走訪 m_items 全表，對 entry.charKind == a2 的項目：
//   ── 取 cltCharKindInfo* (依 a2)，若 isClone(+209) != 1 則嘗試寫入。
//   ── 寫入點為 this+8 的「1 槽」buffer：
//        若 buffer[0] 為 0           → 寫入 entry.mapID
//        若 buffer[0] == entry.mapID → 寫回相同值 (no-op)
//        否則 (已被填過且不同)        → 跳過 (v6>=1 之 goto LABEL_13)
//
// 結論：本函式只記住第一個匹配的 mapID；後續來電必定先重置為 0 再走訪。
// ---------------------------------------------------------------------------
uint16_t* cltRegenMonsterKindInfo::GetRegenMapIDByMonsterKind(uint16_t a2) {
    uint16_t* outBuf = &m_mapIDOut;          // (char*)this + 8
    *outBuf = 0;
    if (m_count <= 0) return outBuf;

    char* base = reinterpret_cast<char*>(m_items);
    for (int i = 0; i < m_count; ++i) {
        char* entry = base + 32 * i;
        if (a2 != *reinterpret_cast<uint16_t*>(entry + 2))
            continue;

        // 反編譯：cltCharKindInfo::GetCharKindInfo 是非靜態方法 (this 為 g_pcltCharKindInfo)。
        unsigned char* charInfo =
            reinterpret_cast<unsigned char*>(g_pcltCharKindInfo->GetCharKindInfo(a2));
        if (!charInfo) continue;
        if (charInfo[209] == 1) continue;     // skip clone

        // 內層搜尋：v6 ∈ [0..0]；超過即 goto LABEL_13 跳過寫入。
        uint16_t* slot = outBuf;
        int v6 = 0;
        const uint16_t newMapID = *reinterpret_cast<uint16_t*>(entry + 4);
        bool skipWrite = false;
        while (*slot && newMapID != *slot) {
            ++v6;
            ++slot;
            if (v6 >= 1) { skipWrite = true; break; }
        }
        if (skipWrite) continue;
        // 落到 *((WORD*)this + v6 + 4) = newMapID  ⇒  this+8 (v6==0)
        outBuf[v6] = newMapID;
    }
    return outBuf;
}

// ---------------------------------------------------------------------------
// 반편역 mofclient.c:329431-329494
//
// 走訪 m_items 全表，對 entry.mapID == a2 的項目：
//   ── 依 entry.charKind 取 cltCharKindInfo*；若 isClone(+209) != 1 則嘗試寫入。
//   ── 寫入點為 this+10 的「5 槽 + 零終止」buffer。
//   ── 內層搜尋（去重）：
//        slot == 0                 → 跳出 (寫入)
//        entry.charKind == slot    → 跳出 (寫入；通常為原值之 no-op)
//        nameTextCode(+2) 相同     → 跳出 (寫入；同名字怪物視為相同)
//        否則 ++v7；v7>=5 即 goto LABEL_13 (跳過寫入)
//   ── 寫入值 = charInfo->monsterRegistryKind (+6)；不是 entry.charKind。
//
// 注意：buffer 的 5 槽 + 1 個零終止 (共 12 bytes，this+10..+22) 都會被本函式
//       清為 0 後再寫入。呼叫端 (CToolTip) 以 5 次迭代讀取，會在零位停止。
// ---------------------------------------------------------------------------
uint16_t* cltRegenMonsterKindInfo::GetRegenMonsterKindByMapID(uint16_t a2) {
    // 反編譯逐字：清 10 bytes (this+10..+20)。本實作多清 2 bytes 以保零終止。
    for (int i = 0; i < 6; ++i) m_kindsOut[i] = 0;

    if (m_count <= 0) return m_kindsOut;

    char* base = reinterpret_cast<char*>(m_items);
    for (int i = 0; i < m_count; ++i) {
        char* entry = base + 32 * i;
        if (a2 != *reinterpret_cast<uint16_t*>(entry + 4))
            continue;

        const uint16_t entryCharKind = *reinterpret_cast<uint16_t*>(entry + 2);
        unsigned char* v15 =
            reinterpret_cast<unsigned char*>(g_pcltCharKindInfo->GetCharKindInfo(entryCharKind));
        if (!v15) continue;
        if (v15[209] == 1) continue;          // skip clone

        // entry 角色的 nameTextCode (+2)，去重比較用。
        const uint16_t entryNameCode = *reinterpret_cast<uint16_t*>(v15 + 2);

        int v7 = 0;
        bool skipWrite = false;
        for (;;) {
            uint16_t v8 = m_kindsOut[v7];
            if (!v8) break;                    // 空槽 → 跳出寫入
            if (entryCharKind == v8) break;    // 與槽中 charKind 相同 → 跳出寫入

            // 比較槽中 charKind 對應的 nameTextCode。
            unsigned char* slotInfo =
                reinterpret_cast<unsigned char*>(g_pcltCharKindInfo->GetCharKindInfo(v8));
            // 反編譯不檢查 slotInfo == NULL；維持 bug-for-bug 行為時要小心 deref。
            // 若回傳 NULL 表示 buffer 中的 charKind 不存在於 g_pcltCharKindInfo，理論上不該發生。
            if (slotInfo) {
                if (entryNameCode == *reinterpret_cast<uint16_t*>(slotInfo + 2)) break;
            }

            ++v7;
            if (v7 >= 5) { skipWrite = true; break; }
        }
        if (skipWrite) continue;
        // 寫入值 = monsterRegistryKind (+6)
        m_kindsOut[v7] = *reinterpret_cast<uint16_t*>(v15 + 6);
    }
    return m_kindsOut;
}
