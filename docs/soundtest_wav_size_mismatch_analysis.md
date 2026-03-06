# SoundTest `wavShortCandidates` 7364 vs 7363 分析

## 現象
- 實體檔案 `minigame_se_gagebutton.wav` 為 `7364` bytes。
- 透過 `CMofPacking::FileRead` 從 MOF 讀到 `7363` bytes。

## 程式碼層面的關鍵點

1. `CMofPacking::FileRead` 讀取大小完全信任 NFS metadata (`nfs_nt_node_get_size`)。
2. `FileRead` 會以該 size 配置 buffer，然後一次 `nfs_file_read`；若回傳量不等於 metadata size 會直接失敗。
3. `nfs_file_read` 會再用同一個 NT metadata size 當作上限，不能超過該大小。

=> 只要封包內 NT 的檔案大小是 7363，`FileRead` 就永遠只會讀到 7363（不可能讀到第 7364 byte）。

## 是否是讀取流程造成？
就目前程式碼來看，不是 `FileRead` 在讀取時「少讀 1 byte」：
- 它是「照 metadata 讀」，metadata 幾 bytes 就讀幾 bytes。
- 因此問題根因是**封包內記錄的檔案大小**與你手上的實體檔案大小不一致。

## 是否是打包流程造成？
### 目前 repo 的 `DataPacking`
- 以 `rb` 開啟原檔、`ftell` 取長度、`fread` 後 `nfs_file_write`，是原始位元組拷貝流程。
- 這個流程本身不會主動把 wav 特別減 1 byte。

### 但仍可能是「既有封包」來源問題
- 若你測的是既有 `mof` 封包（非用目前這版 `DataPacking` 重打），就可能來自歷史工具/舊版打包流程。
- WAV 常見有 RIFF 對齊 padding（尾端補 1 byte）的情況；某些工具若用「邏輯長度」而非「實體長度」打包，會出現只差 1 byte。
- OGG 不走 RIFF/padding 規則，因此通常不會出現同型態的 `+/-1 byte` 現象。

## 建議驗證
1. 用目前 repo 版本 `DataPacking` 重新打包同一批檔案。
2. 重新用 `CMofPacking::FileRead` 讀 `sound/minigame_se_gagebutton.wav`，比對 `GetBufferSize()` 與原檔大小。
3. 若重打後變正常，代表原封包有歷史打包差異。
4. 若仍異常，再追 `nfs_file_write` 回傳值是否小於欲寫入大小（目前 `DataPacking` 沒檢查 short write，可補強）。
