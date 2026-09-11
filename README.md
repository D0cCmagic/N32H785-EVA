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

---

# 附录 A：APP 工程目录架构

> 本附录用于**免读文件快速定位**。以下内容与 `APP/MDK-ARM/LWIP_PING.uvprojx` target `N32H78x` 的实际情况一致。

## A.1 分层总览

```
                        ┌─────────────────────────────┐
   src/main.c ─────────▶ │  初始化编排 + while(1) 主循环 │
                        └──────────────┬──────────────┘
                                       │ Mcal_InitSum()
                        ┌──────────────▼──────────────┐
                        │  Mcal 层（硬件抽象）          │
                        │  LedGpio / TimerTask(1ms时基) │
                        └──────────────┬──────────────┘
                                       │ 5 个任务槽注册
                        ┌──────────────▼──────────────┐
                        │ Hardware 层（应用业务）        │
                        │ TaskServer / R900PNR / OTA /  │
                        │ VectorTable                  │
                        └──────────────┬──────────────┘
                                       │
                        ┌──────────────▼──────────────┐
                        │ R900PNR_CMD 驱动层（泰芯）     │
                        │ hgic_raw → hgic_sdspi_v2      │
                        └──────────────┬──────────────┘
                                       │ spidrv_* 回调
                        ┌──────────────▼──────────────┐
                        │ Mcal/r900pnr_spi_init.c       │
                        │ SPI1 + GPIO + EXTI 寄存器操作  │
                        └─────────────────────────────┘
```

**调用方向恒为自上而下**，下层不反向调用上层（`hgic_platform_raw_send` 由 Hardware 层实现、驱动层通过 `raw_send()` 宏回调，是本设计中唯一的上行注入点）。

## A.2 目录树与模块职责

```
APP/
├── src/
│   ├── main.c                     入口：VTOR/时钟/MPU/Cache/SysTick 初始化 → Mcal_InitSum
│   │                              → Wi-Fi 初始化 → while(1){ Task_Handle_Cycle(); }
│   │                              定义 TimeBase、ReceiveDataFlag、sys_now()
│   └── n32h7xx_it.c               中断向量：SysTick_Handler(TimeBase++)、NMI、
│                                  HardFault、ETH_GLOBAL_IRQHANDLER
├── inc/
│   ├── main.h                     应用主头：聚合 SDK 头、板级/PHY 选择、TimeBase 声明
│   ├── n32h7xx_it.h               中断处理函数声明
│   └── n32eth_cfg.h               以太网配置（LwIP 已移出构建，当前仅头文件残留）
├── Mcal/                          【硬件抽象层】
│   ├── inc|src/Mcal_InitSum.*     统一初始化入口：LED + log + 任务调度器
│   ├── inc|src/Mcal_LedGpio_Init.*  LED GPIO 抽象（LED1=PA2 / LED2=PA3，高电平点亮）
│   ├── inc|src/Mcal_TimerTask.*    1ms 时基 + 5 槽协作式软定时器（无可抢占）
│   ├── inc/r900pnr_spi_cfg.h       SPI1/GPIO/EXTI 引脚与参数配置 + R900PNR_SPI_Init 声明
│   └── src/r900pnr_spi_init.c      SPI1 + GPIO(PA4~PA8) + EXTI9_5 初始化
├── Hardware/                      【应用业务层】
│   ├── inc|src/App_TaskServer_Handle.*  5 个任务体（10/20/50/200/300ms）
│   ├── inc|src/App_R900PNR_Handle.*     Wi-Fi 应用层：角色配置、收发、轮询、
│   │                                    App_WiFiConfigure、delay_ms（基于 sys_now）
│   ├── inc|src/App_OTA_Handle.*         "UPDATE" 命令解析 + OTA 标志写入 + 复位
│   └── inc|src/App_VectorTable.*        ITCM 向量表运行时槽位重定向
├── R900PNR_CMD/                   【泰芯 non-OS 驱动层 v2.0.0】
│   ├── hgic.h / hgic_raw.h         对外接口与全局 struct hgic_raw 定义
│   ├── hgic_raw.c                  raw API：set_*/get_*/send/rx 分发（60+ 命令）
│   ├── hgic_cmd.c                  命令表
│   ├── hgic_sdspi.h                sdspi 接口声明
│   ├── hgic_sdspi_v2.c             SDIO-SPI 协议层：CMD0/CMD5/CMD52/CMD53、FRM2 帧
│   ├── hgic_bootdl.h               bootloader 通信结构（部分函数仅声明未实现）
│   ├── r900pnr_spi_api.h           spidrv_* 与 g_r900pnr_spi_rx_flag 声明
│   └── r900pnr_spi_api.c           N32H785 侧 SPI 实现：纯 CPU 轮询，无 DMA/无 SPI 中断
├── bsp/                           【板级支持】
│   ├── src/log.c                   日志/串口 USART1(PA9/PA10,115200) + OTA 命令接收 ISR
│   ├── src/delay.c                 systick_delay_us/ms（⚠ 会改写 SysTick，见注意事项）
│   ├── inc/log.h                   log_info/warning/error/debug 分级宏
│   ├── inc/delay.h                 systick_delay_* 声明
│   ├── inc/n32h7xx_eval.h          板级宏（硬编码 DEMO_BOARD = N32H787_EVB）
│   ├── bsp_eth.h                   ETH/PHY 选择（硬编码 PHY_USE_YT8522H）
│   └── （未参与构建：bsp_eth.c / ethernetif.c / sys_arch.c / src/bsp_sdram.c
│        / src/print_remap.c / lwipopts.h / arch/）
├── firmware/                      【Nations SDK，原样保留】
│   ├── CMSIS/core/                  CMSIS 内核头
│   ├── CMSIS/device/                n32h7xx.h、system_n32h7xx.c、startup_n32h78x_cm7.s
│   └── n32h7xx_std_periph_driver/   标准外设驱动（inc + src）
└── lwip-2.2.0/                    【LwIP 2.2.0 完整源码，整体未参与构建】
```

## A.3 构建参与清单（target `N32H78x`）

**参与构建（共 30 个 .c/.s）**

| 分组 | 文件 |
|---|---|
| `src/` | `main.c`、`n32h7xx_it.c` |
| `Mcal/src/` | `Mcal_InitSum.c`、`Mcal_LedGpio_Init.c`、`Mcal_TimerTask.c`、`r900pnr_spi_init.c` |
| `Hardware/src/` | `App_OTA_Handle.c`、`App_R900PNR_Handle.c`、`App_TaskServer_Handle.c`、`App_VectorTable.c` |
| `R900PNR_CMD/` | `hgic_cmd.c`、`hgic_raw.c`、`hgic_sdspi_v2.c`、`r900pnr_spi_api.c` |
| `bsp/src/` | `log.c`、`delay.c` |
| `firmware/CMSIS/device/` | `system_n32h7xx.c`、`startup_n32h78x_cm7.s`、`startup_n32h73x_76x.s` |
| `firmware/.../src/` | `misc.c`、`n32h7xx_dcmu.c`、`n32h7xx_dma.c`、`n32h7xx_dmamux.c`、`n32h7xx_eth.c`、`n32h7xx_exti.c`、`n32h7xx_gpio.c`、`n32h7xx_pwr.c`、`n32h7xx_rcc.c`、`n32h7xx_spi.c`、`n32h7xx_usart.c` |

**未参与构建**
`lwip-2.2.0/**`（全部）、`bsp/bsp_eth.c`、`bsp/ethernetif.c`、`bsp/sys_arch.c`、`bsp/src/bsp_sdram.c`、`bsp/src/print_remap.c`、`bsp/arch/**`

> 影响：`inc/main.h` 仍 `#include "bsp_eth.h"`、`bsp_eth.h` 仍定义 PHY，但以太网协议栈未链接；`n32h7xx_eth.c` 虽在构建中，其功能当前无调用方。`bsp/sys_arch.c` 内的 `sys_now()` 未参与链接，因此不与 `main.c` 的 `sys_now()` 冲突。

**构建关键配置**

| 项 | 值 |
|---|---|
| Device / Target | `N32H785xIx7:CM7` / `N32H78x`（唯一可编译 target） |
| 宏定义 | `N32H78x, CORE_CM7, USE_STDPERIPH_DRIVER, USING_TCM, __ICACHE_PRESENT=1, __DCACHE_PRESENT=1` |
| 命令行附加 | `-DN32H785 -D__MICROLIB -O0 --c99`（ARM Compiler 5 / V5.06 update 5） |
| 分散加载 | `M7_TCM.sct` |
| 栈 / 堆 | 8KB / 8KB（Heap 被 microlib 移除） |
| Post-build | `fromelf --bin --output=.\N32H78x\bin\LWIP_PING.bin .\N32H78x\Objects\LWIP_PING.axf` |

> ⚠ `N32H73x_76x` target 缺少 `Hardware`/`Mcal`/`R900PNR_CMD` 的 include path，**无法编译**。
> ⚠ 镜像链接在 `0x00000000`(ITCM)，生成的 `.hex` 带 ITCM 绝对地址**不可直接烧录**；进 Bootloader 的必须是 `fromelf --bin` 产生的 `.bin`。

---

# 附录 B：关键 API 签名与调用关系

仅列**跨模块调用**的函数。格式：`原型` ▸ 定义处 ▸ 调用方。

## B.1 Mcal 层

```c
void Mcal_InitSum(void);                    /* Mcal/src/Mcal_InitSum.c   ◀ main.c */
void Mcal_LedGpio_Init(void);               /* Mcal/src/Mcal_LedGpio_Init.c ◀ Mcal_InitSum */
void Led_On(Led_Id_t id);                   /* ◀ App_TaskServer_Handle */
void Led_Off(Led_Id_t id);
void Led_Toggle(Led_Id_t id);
void Led_SetState(Led_Id_t id, Led_State_t state);
void Led_SetLevel(Led_Id_t id, Led_Level_t level);
void Led_AllOn(void);
void Led_AllOff(void);

void Mcal_TimerTask_Init(void);             /* Mcal/src/Mcal_TimerTask.c ◀ Mcal_InitSum */
void Task_Create(Task_Time_HandleFun fun, Task_Handle_ID Task_ID, uint32_t Task_Time);
void Task_Stop(Task_Handle_ID Task_ID);
void Task_Run(Task_Handle_ID Task_ID);
void Task_Handle_Cycle(void);               /* ◀ main.c 主循环（唯一调度入口） */

int  R900PNR_SPI_Init(void);                /* Mcal/src/r900pnr_spi_init.c
                                               ◀ r900pnr_wifi_module_init */
```

`Led_Id_t`：`LED_ID_1`(=PA2) / `LED_ID_2`(=PA3)
`Task_Handle_ID`：`Task_10ms_ID` / `Task_20ms_ID` / `Task_50ms_ID` / `Task_200ms_ID` / `Task_300ms_ID`

## B.2 Hardware 层

```c
/* App_TaskServer_Handle.c —— 任务体，由 Task_Create 注册后经 Task_Handle_Cycle 调度 */
void Task_10ms_Handle(void);     /* → App_R900PnrPoll()          R900PNR 轮询 */
void Task_20ms_Handle(void);     /* → OTA_UpdateCheck_Handle()    OTA 标志检查 */
void Task_50ms_Handle(void);     /*   空实现 */
void Task_200ms_Handle(void);    /* → Led_Toggle(LED_ID_1)        PA2 */
void Task_300ms_Handle(void);    /* → Led_Toggle(LED_ID_2)        PA3 */

/* App_R900PNR_Handle.c */
int  r900pnr_wifi_module_init(void);        /* ◀ main.c：SPI_Init → sdspi_init(5 次重试) */
int  App_WiFiConfigure(void);               /* ◀ main.c：按 AP/STA 下发配置并启动关联 */
void App_R900PnrPoll(void);                 /* ◀ Task_10ms_Handle */
void App_DemoSendH265(void);                /* STA 发送演示，每次调用发 1 包 */
int  App_SendData(uint8_t *data, uint32_t len);
void App_RxDataHandler(uint8_t *data, uint32_t len);      /* 收到数据帧回调 */
void App_EventHandler(uint8_t event_id, int16_t value);   /* 固件事件回调 */
void App_WaitFwInfo(void);                  /* 3000ms 自旋等待固件信息 */
int  hgic_platform_raw_send(unsigned char *data, unsigned int len); /* 驱动层上行注入点 */
void delay_ms(uint32_t ms);                 /* 基于 sys_now() 忙等，不清 SysTick */

/* App_OTA_Handle.c */
void    OTA_UpdateCheck_Handle(void);       /* ◀ Task_20ms_Handle */
uint8_t ota_cmd_feed(uint8_t ch);           /* ◀ USART1_IRQHandler，逐字节匹配 "UPDATE" */
void    ota_request_reset(void);            /* 写 OTA 标志 → CleanDCache → 系统复位 */

/* App_VectorTable.c */
void App_VectorTableInit(void);             /* ◀ main.c 第一句：VTOR=0 + SysTick 槽位重定向 */
void App_VectorTableRedirect(const AppVectorTable_t *config, uint32_t count);
```

## B.3 R900PNR 驱动层

```c
/* r900pnr_spi_api.c —— 供泰芯驱动通过 spidrv_* 回调使用 */
int  R900PNR_SPI_Init(void);                /* ◀ r900pnr_wifi_module_init */
void spidrv_cs(void *priv, char enable);                          /* enable!=0 → CS 拉低 */
void spidrv_write_read(void *priv, u8 *wdata, u8 *rdata, u32 len);
void spidrv_write(void *priv, u8 *data, u32 len, char dma_flag);
void spidrv_read(void *priv, u8 *data, u32 len, char dma_flag);
int  spidrv_hw_crc(void *priv, u8 *data, u32 len, char flag);     /* 恒返回 0 */
void R900PNR_SPI_INT_IRQHandler(void);      /* 宏映射到 EXTI9_5_IRQHandler */

/* hgic_sdspi_v2.c —— 协议层 */
int hgic_sdspi_init(void *priv);            /* ◀ r900pnr_wifi_module_init，0=成功 */
int hgic_sdspi_detect_alive(void *priv);
int hgic_sdspi_write(void *priv, u8 *data, u32 len);   /* 返回实写长度 */
int hgic_sdspi_read(void *priv, u8 *buf, u32 len, u32 flags);  /* 0=无数据 */

/* hgic_raw.c —— raw API */
HGIC_RAW_RX_TYPE hgic_raw_rx(unsigned char **data, unsigned int *len);  /* ◀ App_R900PnrPoll */
int hgic_raw_send_ether(unsigned char *data, unsigned int len);
int hgic_raw_send_cmd(int cmd, unsigned char *data, unsigned int len);  /* 驱动内部使用 */
/* 另含 60+ 条 hgic_raw_set_xxx / hgic_raw_get_xxx 配置命令，签名见 hgic_raw.h */

struct hgic_raw hgic;   /* 驱动全局状态：tx_buf[2048] / cmd_buf[512] / status / 站点表 */
```

`HGIC_RAW_RX_TYPE`：`IGNORE(0)` / `DATA(1)` / `CMD_RESP(2)` / `BOOTDL_RESP(3)` / `OTA_RESP(4)` / `EVENT(5)`

## B.4 BSP 层

```c
void     log_init(void);                    /* bsp/src/log.c ◀ Mcal_InitSum，初始化 USART1 */
/* log_info / log_warning / log_error / log_debug —— printf 宏，受 LOG_LEVEL 控制 */

uint32_t sys_now(void);                     /* src/main.c，返回 TimeBase(ms) */
#define  get_tick_ms()  sys_now()
void     systick_delay_us(uint32_t us);     /* bsp/src/delay.c  ⚠ 见注意事项 */
void     systick_delay_ms(uint32_t ms);
```

## B.5 初始化时序（`main()` 实际执行顺序）

```
App_VectorTableInit()      VTOR=0，SysTick 槽位 15 ← 重定向到真实 handler
SCB_InvalidateI/DCache()   清 Cache
RCC_SetSysClkToMode0()     切系统时钟
MPU_Config()               MPU 使能（当前为空实现，未配置任何 region）
SCB_EnableI/DCache()       开 Cache
SysTick_Config(M7ClkFreq/1000)  1ms 时基，优先级 1
Mcal_InitSum()             → Mcal_LedGpio_Init + log_init + Mcal_TimerTask_Init(注册 5 槽)
r900pnr_wifi_module_init() → R900PNR_SPI_Init + hgic_sdspi_init(≤5 次 ×1000ms) + 200ms 稳定延时
App_WiFiConfigure()        → 下发 SSID/PSK/信道/带宽等并 start_assoc
while(1) { Task_Handle_Cycle(); }
```

---

# 附录 C：关键全局符号与共享状态

| 符号 | 类型 | 定义处 | 写入方 | 读取方 |
|---|---|---|---|---|
| `TimeBase` | `volatile uint32_t` | `src/main.c:29` | `SysTick_Handler`（1ms） | `Mcal_TimerTask`、`sys_now()` |
| `ReceiveDataFlag` | `volatile int` | `src/main.c:28` | `ETH_GLOBAL_IRQHANDLER` | —（LwIP 已移出） |
| `g_r900pnr_spi_rx_flag` | `volatile uint8_t` | `R900PNR_CMD/r900pnr_spi_api.c` | `EXTI9_5_IRQHandler`（PA8 下降沿） | `App_R900PnrPoll` |
| `ota_request_flag` | `volatile uint8_t` | `Hardware/src/App_OTA_Handle.c` | `USART1_IRQHandler` | `OTA_UpdateCheck_Handle` |
| `hgic` | `struct hgic_raw` | `R900PNR_CMD/hgic_raw.c` | 驱动内部 | 驱动内部 |

**并发模型**：无 RTOS。`SysTick` / `EXTI9_5` / `USART1` 三个 ISR 与主循环并发；ISR 只置标志或自增计数，重活全部在主循环的 5 个任务槽内完成。**泰芯驱动栈不可重入**，只能在主循环（任务槽）中调用，不可在 ISR 内调用。