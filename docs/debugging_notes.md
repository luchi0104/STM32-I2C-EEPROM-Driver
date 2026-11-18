# 除錯紀錄：EEPROM ACK Polling 與 HAL Timeout

## 1. 問題現象

執行單 Byte `HAL_I2C_Mem_Write()` 後回傳 `HAL_OK`，但緊接著呼叫 `HAL_I2C_IsDeviceReady()` 卻得到 `HAL_ERROR`，導致讀取步驟沒有執行。

加入固定 `HAL_Delay(10)` 後，寫入、讀回與資料比對都成功。初步推測與 EEPROM 內部寫入週期有關。

## 2. 初次 ACK Polling 實作的問題

最初的等待函式只允許 `HAL_I2C_ERROR_AF` 重試，遇到其他錯誤便立即返回。Debugger 顯示：

```text
eeprom_wait_error       = 32 (0x20)
eeprom_wait_attempts    = 1
eeprom_wait_exit_reason = 2
```

在本次 STM32L4 HAL 中：

- `HAL_I2C_ERROR_AF = 0x04`
- `HAL_I2C_ERROR_TIMEOUT = 0x20`

因此第一次檢查就因 `TIMEOUT` 錯誤碼離開，而未繼續 ACK Polling。

## 3. 追查 STM32L4 HAL 原始碼

實際查看 `HAL_I2C_IsDeviceReady()`，發現裝置位址探測的 `Trials` 耗盡後，函式尾端會執行：

```c
I2C_Trials++;
/* ... while (I2C_Trials < Trials) ... */
hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
return HAL_ERROR;
```

因此 `ErrorCode = 0x20` **不一定**代表真的等滿 Timeout，也可能是位址探測次數耗盡。將單次 Timeout 從 10 ms 增加到 100 ms 仍無法解決，因為 NACK 探測可能很快就結束。

## 4. 實驗性修正

調整等待策略：在有限的整體等待時間內，允許對 `HAL_ERROR` 且錯誤碼為 AF 或 TIMEOUT 的情況重試，兩次探測之間加入約 1 ms 延遲；遇到 `HAL_BUSY` 或其他錯誤則返回。

某次成功的 Debug 結果：

```text
eeprom_ready_status     = HAL_OK
eeprom_wait_attempts    = 3
eeprom_wait_elapsed_ms  = 4
eeprom_rx_data          = 0x5A
eeprom_verify_ok        = 1
```

這表示 ACK Polling 可在裝置恢復回應後結束，不必固定等待 10 ms。4 ms 是等待函式的粗略執行時間，不是 EEPROM 寫入週期的精密量測。

## 5. 已知限制與改善方向

**目前尚無法單靠 `HAL_I2C_ERROR_TIMEOUT` 判斷錯誤來源。** 它可能來自位址探測次數耗盡，也可能是真正的硬體旗標等待逾時。

產品級 Driver 應進一步確認 HAL 內部返回路徑、檢查 I2C 旗標與狀態，實作清楚的整體截止時間、故障分類與 Bus Recovery；不可把所有 TIMEOUT 都視為 EEPROM 正在 Busy。

此外，若多個 Chunk 的寫入中途失敗，前面已完成的 Chunk 可能已永久寫入 EEPROM，沒有自動回滾能力。
