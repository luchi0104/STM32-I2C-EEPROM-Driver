# STM32-I2C-EEPROM-Driver
STM32L476RG I2C driver for 24C256 EEPROM, featuring automatic page-boundary handling, ACK polling, input validation, and readback verification.
# STM32 I2C 24C256 EEPROM Driver

基於 **NUCLEO-L476RG（STM32L476RG）** 與 **24C256 EEPROM（32 KiB）** 的模組化 I2C 驅動程式。使用 STM32 HAL 阻塞式 API，支援連續讀寫、自動跨頁分割、寫入週期 ACK Polling，以及基本參數驗證。

> **專案狀態：已於實體硬體驗證的原型。** ACK Polling 的錯誤處理依據本次 STM32L4 HAL 實作調整，尚未達到產品級故障復原要求。斷線故障注入、完整 Timeout 分類與 Bus Recovery 尚未完成。

## 功能特色

- **模組化設計**：`eeprom_24c256.h` 定義公開 API，`eeprom_24c256.c` 實作細節。
- **讀寫 API**：`EEPROM24C256_Init()`、`EEPROM24C256_Write()`、`EEPROM24C256_Read()`。
- **自動處理 64-Byte Page 邊界**：將跨頁寫入拆成多個合法的 Page Write。
- **ACK Polling**：每次寫入後等待 EEPROM 完成內部寫入週期。
- **參數驗證**：處理 NULL 指標、超出容量、零長度操作。
- **讀回比對**：透過 TX/RX Buffer 逐 Byte 驗證資料一致性。

## 開發環境與硬體

| 項目 | 設定 |
|---|---|
| 開發板 | ST NUCLEO-L476RG |
| MCU | STM32L476RG |
| EEPROM | 24C256 模組，32 KiB，64 Bytes/Page |
| 開發環境 | STM32CubeIDE 2.2.0 |
| HAL | STM32CubeL4（專案使用 FW_L4 V1.18.2） |
| I2C | I2C1，Standard Mode 100 kHz |
| SCL / SDA | PB8 / PB9 |
| 供電 | 3.3 V，共用 GND |
| EEPROM 7-bit 位址 | `0x50`（A2/A1/A0 = LOW） |
| WP | LOW（允許寫入） |

接線與電氣注意事項請參考 [docs/wiring.md](docs/wiring.md)。I2C SDA/SCL 需要上拉至 3.3 V；不可只憑模組上的電阻標記判斷其實際接法。

## 專案架構

```text
STM32-I2C-EEPROM-Driver/
├── Core/
│   ├── Inc/
│   │   └── eeprom_24c256.h
│   └── Src/
│       ├── main.c
│       └── eeprom_24c256.c
├── Drivers/
├── docs/
│   ├── wiring.md
│   ├── testing.md
│   └── debugging_notes.md
├── README.md
└── .gitignore
```

> 上方為重點檔案示意。完整可編譯的 STM32CubeIDE 專案仍需保留 `.ioc`、IDE 專案設定、Startup、Linker Script、HAL/CMSIS 等必要檔案。

## 使用範例

在 CubeMX 產生的 `MX_I2C1_Init()` 執行完成後：

```c
#include "eeprom_24c256.h"

uint8_t tx[16] = {
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF
};
uint8_t rx[16] = {0};

HAL_StatusTypeDef status = EEPROM24C256_Init(&hi2c1);
if (status == HAL_OK) {
    status = EEPROM24C256_Write(0x0078, tx, 16);
}
if (status == HAL_OK) {
    status = EEPROM24C256_Read(0x0078, rx, 16);
}
/* 仍須逐 Byte 比較 tx 與 rx，才能判定資料驗證 PASS。 */
```

**注意：** 此範例會覆寫 EEPROM `0x0078～0x0087` 的內容。

## 跨頁寫入原理

24C256 單次 Page Write 不能跨越 64-Byte 邊界。若從 `0x0078` 寫入 16 Bytes，Driver 會分為：

| 次數 | 位址範圍 | 長度 |
|---|---|---:|
| 1 | `0x0078～0x007F` | 8 Bytes |
| 2 | `0x0080～0x0087` | 8 Bytes |

核心公式：

```c
page_offset    = current_addr % 64U;
page_remaining = 64U - page_offset;
chunk_size     = (remaining < page_remaining)
               ? remaining : page_remaining;
```

每個 Chunk 寫入並完成 ACK Polling 後，同步更新 EEPROM 位址、RAM 資料指標與剩餘長度：

```c
current_addr += chunk_size;
current_data += chunk_size;
remaining    -= chunk_size;
```

## 實測成果

已完成單 Byte、16 Bytes 同頁、16 Bytes 跨頁、128 Bytes 多頁寫入與讀回驗證，並通過零長度、NULL、超容量及最後合法位址等測試。詳細數據請參考 [測試紀錄](docs/testing.md)。

ACK Polling 的 HAL 原始碼分析與修正過程請參考 [除錯紀錄](docs/debugging_notes.md)。

## 建置與執行

1. 使用 STM32CubeIDE 2.2.0 匯入完整 STM32CubeIDE 專案。
2. 確認 I2C1 使用 PB8/PB9、100 kHz，並檢查 EEPROM 7-bit 位址。
3. 斷電完成接線後，以 3.3 V 供電，確保 SDA/SCL 有正確上拉。
4. Build、燒錄並 Debug，觀察 HAL 狀態與資料比對變數。

> 本文件包**不包含完整 STM32CubeIDE 專案原始碼**。發佈可編譯 Repository 時，請補入實際驗證過的 `.ioc`、`Core/`、`Drivers/`、啟動檔、Linker Script 及 IDE 設定。

## 已知限制與後續改善

- 目前使用 **Blocking HAL API**，尚未提供 RTOS 執行緒安全保證。
- 此 STM32L4 HAL 版本的 `HAL_I2C_ERROR_TIMEOUT` 可能同時表示位址探測次數耗盡或實際旗標等待逾時；正式產品須進一步區分。
- 多 Chunk 寫入若在中途失敗，前面的 Chunk 可能已寫入；**沒有交易回滾機制**。
- 尚未完成 EEPROM 斷線故障注入、Bus Recovery 與自動化測試。
- 128 Bytes 測試已確認讀回一致，但尚未逐次記錄每個 HAL Chunk 的實際呼叫參數。

## 授權

發佈前請自行選擇專案授權條款，並保留 STMicroelectronics 原始版權聲明及遵守 STM32 HAL/CMSIS 的授權要求。
