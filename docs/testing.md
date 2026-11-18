# 實體硬體測試紀錄

**測試平台：** NUCLEO-L476RG、24C256 EEPROM 模組、I2C1 100 kHz、STM32CubeIDE 2.2.0。

## 測試矩陣

| 測試項目 | 預期結果 | 實際觀察 | 狀態 |
|---|---|---|---|
| 位址 `0x50` 裝置偵測 | `HAL_OK` | `HAL_OK` | PASS |
| `0x0010` 單 Byte 寫入／讀回 | 讀回 `0x5A` | 讀回 `0x5A`，Verify = 1 | PASS |
| 寫入後 ACK Polling | 最終收到 ACK | 某次實測 3 次探測、約 4 ms | PASS |
| `0x0040` 同頁 16 Bytes | 全部一致 | Mismatch = 0 | PASS |
| `0x0078` 跨頁 16 Bytes | 分兩個 8-Byte Chunk | 寫入嘗試次數 2、Mismatch = 0 | PASS |
| 模組化 Driver 16 Bytes 回歸測試 | Init/Write/Read 成功 | 全部 `HAL_OK`、Verify = 1 | PASS |
| 零長度寫入 | `HAL_OK` | `HAL_OK` | PASS |
| NULL 指標、長度 8 | `HAL_ERROR` | `HAL_ERROR` | PASS |
| `0x7FFC` 寫入 8 Bytes（超界） | `HAL_ERROR` | `HAL_ERROR` | PASS |
| `0x0030` 寫入／讀回 128 Bytes | 全部一致 | Write/Read `HAL_OK`、Mismatch = 0 | PASS |
| 最後合法位址 `0x7FFF` 寫入／讀回 `0xA5` | 成功且一致 | `HAL_OK`、`0xA5`、Verify = 1 | PASS |
| `0x7FFF` 寫入 2 Bytes（超界） | `HAL_ERROR` | `HAL_ERROR` | PASS |
| 探測不存在的位址 `0x51` | 無 ACK | `HAL_ERROR`、ErrorCode = 32、Tick 差 = 0 | PASS |
| 不存在裝置的 `HAL_I2C_Mem_Write()` | 應回傳失敗 | 尚未提供實測結果 | 未測 |
| EEPROM 實體斷線／Bus Recovery | 有限時間返回並妥善復原 | 尚未測試 | 未測 |

## 128 Bytes 多頁寫入：預期分割

從 `0x0030` 寫入 128 Bytes，依據 Driver 的 Page Boundary 演算法：

| Chunk | EEPROM 位址範圍 | 長度 |
|---|---|---:|
| 1 | `0x0030～0x003F` | 16 Bytes |
| 2 | `0x0040～0x007F` | 64 Bytes |
| 3 | `0x0080～0x00AF` | 48 Bytes |

**說明：** 上表是根據程式邏輯計算的分割結果；目前 128 Bytes Debug 截圖證明整體讀寫資料一致，但尚未獨立記錄每個 Chunk 的 HAL 呼叫參數。

## 測試解讀注意事項

- `HAL_I2C_Mem_Write() == HAL_OK` 表示 I2C 傳輸成功，不保證 EEPROM 內部寫入週期已完成；須進行 ACK Polling。
- `HAL_GetTick()` 為毫秒級計時；`elapsed_ms = 0` 代表兩次取樣可能落在同一 Tick，並非耗時真正為零。
- 寫入測試會覆寫 EEPROM 指定位址，執行前應確認沒有重要資料。
- 以上為開發階段的實體測試，並非完整的產品級驗證。

