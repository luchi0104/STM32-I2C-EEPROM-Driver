# 硬體接線與 I2C 設定

## 接線表

| 24C256 EEPROM 模組 | NUCLEO-L476RG | 用途 |
|---|---|---|
| VCC | 3.3 V | 模組供電 |
| GND | GND | 共地 |
| SCL | PB8（Arduino D15） | I2C1_SCL |
| SDA | PB9（Arduino D14） | I2C1_SDA |

## EEPROM 位址設定

本次實驗將 A2、A1、A0 設為 LOW，因此 7-bit 位址為 `0x50`。STM32 HAL 呼叫使用左移一位的位址：

```c
#define EEPROM24C256_I2C_ADDR (0x50U << 1)
```

WP 設為 LOW，允許 EEPROM 寫入。本次模組的跳線與接地關係曾以三用電表做導通／電阻檢查，但不同模組版本的跳線接法可能不同，請依實際板子確認。

## 電氣注意事項

- I2C 使用 Open-Drain，**SDA 與 SCL 都需要上拉電阻**，上拉電源應符合 3.3 V 邏輯規格。
- 不可只因板上電阻印有 `102` 就認定它一定是 SDA/SCL 的上拉電阻；需確認走線或量測。
- 改線前先關閉開發板電源。
- 三用電表的蜂鳴／電阻檔只能在**斷電**時使用。
- 未確認電壓相容性前，不要將 I2C 線路接到 5 V。

## STM32CubeMX 設定

| 項目 | 設定 |
|---|---|
| Peripheral | I2C1 |
| Speed | Standard Mode，100 kHz |
| Addressing Mode | 7-bit |
| SCL | PB8 |
| SDA | PB9 |
| Analog Filter | Enabled |
| Digital Filter | 0 |

## 接線後的基本驗證

```c
HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(
    &hi2c1, (0x50U << 1), 3, 100
);
```

本次實體測試得到 `HAL_OK`，代表裝置位址探測收到 ACK；這**不等於** EEPROM 的寫入及讀取已完成驗證，仍須進行 Write → WaitReady → Read → Compare。

