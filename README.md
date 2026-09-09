# N32H785-EVA

基于国民技术 N32H785（Cortex-M7 600MHz 双核）的 **Bootloader + OTA-IAP + 业务应用** 完整方案。

在官方 SDK IAP_Uart 例程基础上移植修复，并融合实际业务工程（以太网 + R900PNR Wi-Fi 模块 SPI 通信）。

## 功能特性

- **双 Bank A/B 交替升级**：BankA(880K) / BankB(1024K) 交替写入，传输中断电自动回退上一个完整版本，**永不变砖**
- **APP 阶段触发升级**：业务运行中串口发送 `UPDATE` + 回车即可进入升级模式（不依赖按键/复位）
- **WKUP 按键触发**：复位后 5 秒内长按 WKUP(PA0) ≥2s 进入升级模式（调试/生产备用通道）
- **XMODEM-CRC 传输**：Tera Term 即可作上位机，128 字节包逐包 CRC16 校验
- **60 秒超时自恢复**：升级等待期无数据自动复位运行旧版；等待期收到脏数据（误发命令）不再卡死
- **ITCM 满速执行**：APP 镜像由 boot 拷贝至 ITCM(0x00000000) 执行，600MHz 零等待
- **三级防变砖防御**：交替写入（不碰运行中 Bank）→ 计数器 EOT 提交（半成品无凭证）→ CRC 拒跳（坏固件不执行）

## Flash 空间规划（2MB 版本）

```
0x15000000 ┌──────────────────────────────┐
           │ Bootloader (16K，镜像须<12K)   │  Flash 直接执行
0x15003000 ├──────────────────────────────┤
           │ 计数器/CRC 扇区 (4K)           │  0x15003FF8: APP CRC32
           │                               │  0x15003FFC: BankB 计数器(u16)
0x15004000 │                               │  0x15003FFE: BankA 计数器(u16)
           │ BankA (880K)                  │  APP 镜像槽 1
0x150E0000 │                               │
           │ BankB (1024K)                 │  APP 镜像槽 2
0x151DFFFF └──────────────────────────────┘
```

## RAM / TCM 规划

```
0x00000000 ┌──────────────────────────────┐
           │ ITCM 1MB                      │  APP 完整镜像（boot 拷入，断电即失）
0x000FFFFF └──────────────────────────────┘
0x24000000 ┌──────────────────────────────┐
           │ AXI SRAM1 128K               │  boot 向量表(0x400) / RW+ZI
0x2401F000 ├──────────────────────────────┤
           │ 保留 4K                       │  OTA 升级标志 @ 0x2401FF00
0x2401FFFF └──────────────────────────────┘
```

## 目录结构

```
Lwip_Ping_Test/
├── APP/                          # 业务应用工程（Keil）
│   ├── src/                      #   main.c、n32h7xx_it.c
│   ├── bsp/                      #   log.c（串口+OTA命令中断）、SDRAM
│   ├── Hardware/                  #   App_OTA_Handle（UPDATE 命令/标志/复位）
│   │                             #   App_VectorTable（ITCM 向量表槽位重定向）
│   ├── Mcal/                     #   R900PNR SPI 驱动配置
│   ├── R900PNR_CMD/              #   hgic 命令层
│   └── firmware/                 #   SDK 驱动（CMSIS + 标准外设）
└── BOOTLOADER/
    └── Bootloader/                # Bootloader 工程（Keil）
        ├── src/                   #   main.c、system_n32h7xx.c（PWR/TCM 配置）
        ├── upgrade/               #   IAP.c（双Bank选库/跳转）、xmodem.c、crc.c
        └── inc/                   #   flash.h（空间布局定义）
```

## 使用方法

### 首次部署

1. Keil 编译 Bootloader → SWD 烧录（0x15000000）
2. Keil 编译 APP → 生成 `app.bin`（fromelf，已配置 After Build）
3. SWD 将 `app.bin` 烧录至 `0x15004000`（BankA）
4. 复位 → 5 秒倒计时 → CRC 校验通过 → 自动进入 APP

### OTA 升级（日常）

1. APP 运行中，串口（PA9/PA10，115200）发送 `UPDATE` + 回车
2. 自动复位进入 Bootloader 升级模式（打印 `OTA update request from APP`）
3. Tera Term → Transfer → XMODEM → Send → 选择 `app.bin`
4. 传输完成打印 `Firmware updated!` → 自动跳转新版

### 日常开发调试

APP 链接于 ITCM（RAM），Keil Debug 直接载入运行（已关闭 Update Flash before Debugging）：
- 改代码 → Ctrl+F5 进 Debug → 断点/单步调试（秒级迭代）
- 注意：调试版断电即失，里程碑版本需 OTA 落盘 Bank 持久化

## 关键移植修复（相对官方例程）

| # | 问题 | 修复 |
|---|------|------|
| 1 | 例程为 H76x 工程未移植 H78x：Device/宏/启动文件/供电模式全错 | 四件套全面替换（N32H785xIx7 / N32H78x / startup_78x / PWR 按板选） |
| 2 | xmodem 1024 字节包 CRC 偏移错误 | CRC 读取改为 [3+size]/[4+size]（本工程用 128 字节包，未触发） |
| 3 | 等 C 期间收到任意非协议字节永久卡死 | 循环顶部白名单过滤 + rcvFlag 快照消费 |
| 4 | SDK 启动文件 SysTick 向量槽位为空函数 | App_VectorTable 模块运行时重定向（ITCM 可写） |
| 5 | DMA+空闲中断改造删除了等待超时 | 60s 超时 + 1Hz C 节奏 |
| 6 | APP 开 D-Cache 后升级标志写入不落 SRAM | 复位前 `SCB_CleanDCache()` |

## 硬件说明

- 开发板：N32H785 自建板（**LDO 供电**，非官方 EVAL 板的 SMPS——`PWR_SUPPLY_SELECTION` 按实际电路选择）
- 调试器：CMSIS-DAP
- 串口：USART1（PA9/PA10，115200-8-N-1）

## 注意事项

- APP 编译后必须 **≤880KB**（受 BankA 短板限制，ITCM 1MB 亦为上限）
- Bootloader 镜像必须 **<12KB**（0x15003000 起的末扇区存计数器，EOT 时会被擦写）
- `UPDATE` 命令必须整词 + 行终止符（`UPDATE`+回车），`UPDATEE`/`UPDAT` 等不会误触发
- 断电语义：升级请求后断电 = 放弃升级回旧版（SRAM 标志消失，无需任何处理）
- 双 Bank 交替时 BankA/B 覆盖旧版本，如需版本追溯请在 APP 内自行记录版本号

## 参考

- [Nations N32H7xx SDK](https://www.nationstech.com/)（Nations.N32H7xx_Library.1.2.0）
- XMODEM 协议：Tera Term 内置发送支持