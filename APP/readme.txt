# N32H785 + R900PNR SPI 工程交接文档

## 一、项目概述

N32H785 开发板通过 SPI 接口驱动泰芯 R900PNR (TXW8301 FMAC) WiFi 模块，实现 raw data 透传通信。两块板子分别配置为 AP（接收端）和 STA（发送端），测试不同距离下的通信速率。

**当前状态**: 30m-500m 距离测试已完成，SPI 37.5MHz 稳定运行，全程 0 丢包。

---

## 二、文件架构

```
Lwip_Ping_Test/
├── src/
│   └── main.c                    # 主程序入口
├── Hardware/
│   ├── inc/
│   │   └── App_R900PNR_Handle.h  # R900PNR 应用层封装头文件
│   └── src/
│       └── App_R900PNR_Handle.c  # R900PNR 应用层封装实现
├── Mcal/
│   ├── inc/
│   │   └── r900pnr_spi_cfg.h     # SPI 引脚/时钟/参数宏定义
│   └── src/
│       └── r900pnr_spi_init.c    # GPIO/SPI/EXTI/NVIC 初始化
├── R900PNR_CMD/                  # 泰芯厂商驱动 (勿改)
│   ├── hgic.h                    #   主头文件
│   ├── hgic_cmd.c                #   命令接口实现
│   ├── hgic_raw.c                #   raw 数据收发
│   ├── hgic_raw.h                #   raw 接口定义 + 枚举
│   ├── hgic_sdspi.h              #   SDIO-SPI 驱动头文件
│   ├── hgic_sdspi_v2.c           #   SDIO-SPI 驱动实现 (核心)
│   ├── r900pnr_spi_api.c         #   SPI 底层读写 API
│   └── r900pnr_spi_api.h         #   SPI 读写 API 声明
├── bsp/                          # 板级支持包
│   ├── bsp_eth.c/h               #   以太网初始化 (当前禁用)
│   ├── ethernetif.c/h            #   LWIP 以太网接口 (当前禁用)
│   ├── lwipopts.h                #   LWIP 配置选项
│   ├── inc/
│   │   ├── delay.h               #   延时函数声明
│   │   ├── log.h                 #   日志打印声明
│   │   └── n32h7xx_eval.h        #   板级评估板定义
│   └── src/
│       ├── delay.c               #   延时函数 (systick_delay_ms)
│       ├── log.c                 #   串口日志初始化 + printf
│       └── print_remap.c         #   printf 重映射
├── firmware/                     # CMSIS + 标准外设库 (勿改)
└── MDK-ARM/                      # Keil 工程
    └── LWIP_PING.uvprojx         #   Keil 工程文件
```

---

## 三、代码架构分层

```
┌─────────────────────────────────────┐
│           main.c                     │  流程编排
│  BSP_Init() → SPI_Init → App_Wait   │  (初始化 + 主循环)
│  → App_WiFiConfigure → App_Poll     │
├─────────────────────────────────────┤
│      App_R900PNR_Handle.c/h          │  应用层
│  WiFi配置 / 数据收发 / 事件处理      │  (你主要改这里)
├─────────────────────────────────────┤
│  r900pnr_spi_cfg.h / spi_init.c      │  BSP层
│  GPIO/SPI/EXTI 初始化                │
├─────────────────────────────────────┤
│  r900pnr_spi_api.c                   │  驱动适配层
│  SPI 读写底层操作                     │
├─────────────────────────────────────┤
│  hgic_sdspi_v2.c / hgic_cmd.c        │  厂商驱动 (勿改)
│  hgic_raw.c                          │  SDIO-SPI 协议
└─────────────────────────────────────┘
```

---

## 四、核心文件详解

### 4.1 main.c — 主程序

**职责**: BSP 初始化 + 流程编排，不含业务逻辑

**关键变量**:
```c
__IO uint32_t TimeBase = 0;    // 系统 1ms 计数器, SysTick 中断自增
__IO int ReceiveDataFlag = 0;  // 以太网接收标志 (LWIP 禁用时不用)
```

**关键函数**:
```c
uint32_t sys_now(void);        // 返回 TimeBase, LWIP 需要的时钟接口
#define get_tick_ms() sys_now() // R900PNR 用的时间接口 (宏, 零开销)
```

**启动流程**:
```
BSP_Init()              → 时钟/Cache/SysTick/串口 初始化
R900PNR_SPI_Init()      → SPI1 + GPIO + EXTI 初始化
hgic_sdspi_init(0)      → 模块 SDIO 总线初始化 (失败则重试)
App_WaitFwInfo()        → 查询固件版本 (可选, 不影响功能)
App_WiFiConfigure()     → 配置 WiFi (AP/STA 角色 + 参数)
while(1) { App_R900PnrPoll(); }  → 主循环轮询
```

**编译开关**:
```c
#define LWIP_ENABLE (0u)  // 0=禁用以太网, 1=启用 LWIP
```

### 4.2 App_R900PNR_Handle.h — 应用层配置

**角色选择** (AP 板和 STA 板编译时切换):
```c
#define APP_WIFI_ROLE  APP_ROLE_STA   // STA 板用这个
#define APP_WIFI_ROLE  APP_ROLE_AP    // AP 板用这个
```

**WiFi 参数**:
```c
#define APP_WIFI_SSID       "hgic_ah_test"
#define APP_WIFI_PSK        "12345678"
#define APP_WIFI_BSS_BW     (8u)              // 8MHz 带宽
#define APP_WIFI_CHAN_LIST  {9080u, 9160u, 9240u}  // 908/916/924 MHz
```

**数据包参数**:
```c
#define APP_TX_PAYLOAD_LEN  (1400u)   // STA 发送包大小
#define APP_TX_BUF_SIZE     (1500u)   // 发送缓冲区大小
```

**对外接口声明**:
```c
void App_R900PnrPoll(void);                        // 主循环轮询
int  App_WiFiConfigure(void);                      // WiFi 配置
void App_WaitFwInfo(void);                         // 查询固件版本
void App_RxDataHandler(uint8_t *data, uint32_t len);   // 接收回调
void App_EventHandler(uint8_t event_id, int16_t value); // 事件回调
int  App_SendData(uint8_t *data, uint32_t len);    // 数据发送
int  hgic_platform_raw_send(unsigned char *data, unsigned int len); // 厂商回调
void delay_ms(uint32_t ms);                        // 安全延时
```

### 4.3 App_R900PNR_Handle.c — 应用层实现

**内部变量 (static)**:
```c
static uint8_t s_spi_rx_buf[2048];     // SPI 接收缓冲区
static uint8_t s_tx_buf[1500];         // 发送缓冲区
// STA 专属:
static uint8_t  s_wifi_connected;      // 关联标志
static uint8_t  s_tx_started;          // 发包启动标志 (防重复打印)
static uint32_t s_tx_total_bytes;      // 发送字节统计
static uint32_t s_tx_total_pkts;       // 发送包数统计
// AP 专属:
static uint32_t s_rx_total_bytes;      // 接收字节统计
static uint32_t s_rx_total_pkts;       // 接收包数统计
```

**App_R900PnrPoll() — 主循环核心**:
```c
void App_R900PnrPoll(void)
{
    // 1. SPI 接收: 检查 INT 引脚 (PA8) 或标志位, 读取数据
    //    hgic_raw_rx() 返回值判断:
    //    HGIC_RAW_RX_TYPE_DATA  → App_RxDataHandler()
    //    HGIC_RAW_RX_TYPE_EVENT → App_EventHandler()

    // 2. 数据发送 (仅 STA):
    //    App_DemoSendH265() 全速发送 1400 字节测试包
    //    每秒打印 [TX STAT] 速率统计
}
```

**App_WiFiConfigure() — WiFi 配置流程**:
```
AP 模式:                          STA 模式:
hgic_raw_set_mode("ap")          hgic_raw_set_mode("sta")
hgic_raw_set_ssid(SSID)          hgic_raw_set_ssid(SSID)
hgic_raw_set_key_mgmt("wpa2")    hgic_raw_set_key_mgmt("wpa2")
hgic_raw_set_wpa_psk(PSK)        hgic_raw_set_wpa_psk(PSK)
hgic_raw_set_freq_range(...)     hgic_raw_set_freq_range(...)
hgic_raw_set_bss_bw(8)           hgic_raw_set_bss_bw(8)
hgic_raw_set_chan_list(...)      hgic_raw_set_chan_list(...)
hgic_raw_set_supper_pwr(1)       hgic_raw_set_supper_pwr(1)
hgic_raw_set_acktmo(10)          hgic_raw_set_acktmo(10)
hgic_raw_open()                  hgic_raw_open()
hgic_raw_save()                  hgic_raw_save()
                                 hgic_raw_start_assoc()
```

**App_DemoSendH265() — 数据发送**:
```c
// 构造 1400 字节测试包, 调用 hgic_raw_send_ether() 发送
// 每秒统计并打印: [TX STAT] pkts=N bytes=N rate=N Kbps fail=N
// 关联超时 3 秒后自动开始发包 (不等 connected 事件)
```

**delay_ms() — 安全延时**:
```c
// 基于 sys_now() 的轮询延时, 不操作 SysTick 寄存器
// 注意: 不要用 systick_delay_ms(), 它会重配 SysTick 并关中断
//       导致 TimeBase 停止增长, 所有超时逻辑失效
```

### 4.4 r900pnr_spi_cfg.h — SPI 硬件配置

```c
#define R900PNR_SPIx              SPI1
#define R900PNR_SPI_BAUDRATE_PRES SPI_BR_PRESCALER_4   // 37.5MHz

// 引脚: SCK=PA5, MISO=PA6, MOSI=PA7, CS=PA4, INT=PA8
#define R900PNR_SPI_CPOL          SPI_CLKPOL_LOW        // Mode 0
#define R900PNR_SPI_CPHA          SPI_CLKPHA_FIRST_EDGE
#define R900PNR_SPI_DATA_SIZE     SPI_DATA_SIZE_8BITS
#define R900PNR_SPI_FIRST_BIT     SPI_FB_MSB


## 引脚对应关系

开发板丝印从上往下顺序：`D2, D3, CMD, SVCC, CLK, GND, D0, D1`

| 开发板丝印 | SDIO 引脚 | SPI 功能       | 主控引脚 | 说明                         |
|-----------|-----------|----------------|----------|------------------------------|
| D2        | SD_D2     | NC             | 不连接   | 未使用，悬空即可             |
| D3        | SD_D3     | CS1            | PA4      | SPI 片选，低有效             |
| CMD       | SD_CMD    | SPI_MOSI1      | PA7      | SPI 主机输出 / 从机输入      |
| SVCC      | VCC       | 电源           | 3.3V     | 模块供电，推荐接 3.3V        |
| CLK       | SD_CLK    | SPI_CLK1       | PA5      | SPI 时钟                     |
| GND       | GND       | GND            | GND      | 共地                         |
| D0        | SD_D0     | SPI_MISO1      | PA6      | SPI 主机输入 / 从机输出      |
| D1        | SD_D1     | SPI_INTIO1     | PA8      | 中断/状态 IO，可接主控 GPIO  |

## 主控连接汇总

| 主控引脚 | 功能        | 连接至模块 |
|----------|-------------|------------|
| PA4      | SPI_CS      | D3 / CS1   |
| PA5      | SPI_SCK     | CLK        |
| PA6      | SPI_MISO    | D0         |
| PA7      | SPI_MOSI    | CMD        |
| PA8      | INT / GPIO  | D1         |
| 3.3V     | 电源        | SVCC       |
| GND      | 地          | GND        |

## 注意事项

1. **D2 不连接**：`SD_D2` 在 SPI 模式下未使用，保持悬空即可。
2. **电源**：`SVCC` 推荐接主控的 `3.3V`，确保与 SD 卡模块电平匹配。
3. **INT 引脚**：`D1 / INTIO1` 可作为中断或卡状态检测，若不需要可悬空或配置为普通 GPIO。
4. **SPI 模式**：使用 SPI1，CS 引脚通常由软件 GPIO 控制，初始化时建议使用较低时钟频率，再逐步提高。

### 4.5 r900pnr_spi_init.c — SPI 硬件初始化

```
R900PNR_SPI_GPIO_Init()   → PA5/6/7 复用推挽, PA4 输出, PA8 浮空输入
R900PNR_SPI_EXTI_Init()   → PA8 映射到 EXTI_Line8, 下降沿中断
R900PNR_SPI_Periph_Init() → SPI1 Master, Mode 0, 8bit, MSB First, /4 分频
R900PNR_SPI_Init()        → 组合调用以上三个, 返回 0 成功
```

---

## 五、厂商驱动 API 速查 (R900PNR_CMD/)

### 5.1 初始化

```c
int hgic_sdspi_init(int irq_mode);    // 初始化 SDIO-SPI 总线, 返回 0 成功
                                       // irq_mode: 0=轮询, 1=中断
```

### 5.2 WiFi 配置命令

```c
int hgic_raw_set_mode(char *mode);              // "ap" 或 "sta"
int hgic_raw_set_ssid(char *ssid);              // SSID 字符串
int hgic_raw_set_wpa_psk(char *psk);            // WPA 密钥
int hgic_raw_set_key_mgmt(char *key_mgmt);      // "wpa2" / "none"
int hgic_raw_set_freq_range(u16 min, u16 max, u8 bss_bw);  // 频率范围 + 带宽
int hgic_raw_set_bss_bw(u8 bw);                 // BSS 带宽 (MHz)
int hgic_raw_set_chan_list(u16 *list, int count); // 频点列表
int hgic_raw_set_supper_pwr(int enable);        // 超级功率开关
int hgic_raw_set_acktmo(int tmo);               // ACK 超时
int hgic_raw_open(void);                        // 开启 WiFi
int hgic_raw_save(void);                        // 保存配置到 flash
int hgic_raw_start_assoc(void);                 // STA 发起关联
void hgic_raw_get_fwinfo(void);                 // 查询固件版本
```

### 5.3 数据收发

```c
// 发送 raw 数据 (广播, 不需要目标地址)
int hgic_raw_send_ether(uint8_t *data, uint32_t len);

// 读取 SPI 数据
int hgic_sdspi_read(int port, uint8_t *buf, uint32_t size, uint32_t timeout);

// 解析收到的数据, 返回类型
// 返回值: HGIC_RAW_RX_TYPE_DATA(1)=数据, HGIC_RAW_RX_TYPE_EVENT(5)=事件
HGIC_RAW_RX_TYPE hgic_raw_rx(uint8_t **data, uint32_t *len);

// 平台回调 (被 hgic_cmd.c 调用, 需在应用层实现)
int hgic_platform_raw_send(unsigned char *data, unsigned int len);
```

### 5.4 数据类型

```c
typedef enum {
    HGIC_RAW_RX_TYPE_DATA    = 1,   // 数据包
    HGIC_RAW_RX_TYPE_CMD     = 2,   // 命令响应
    HGIC_RAW_RX_TYPE_EVENT   = 5,   // 事件通知
} HGIC_RAW_RX_TYPE;
```

### 5.5 事件 ID

```c
#define HGIC_EVENT_CONECTED     1   // STA 关联成功 (注意拼写: CONECTED)
#define HGIC_EVENT_DISCONECTED  2   // STA 断开
// 事件数据格式: event_id(1B) + value(2B)
```

---

## 六、关键注意事项

### 6.1 不要用 systick_delay_ms

```c
// 错误 - 会破坏 SysTick 中断:
systick_delay_ms(100);

// 正确 - 用基于 sys_now() 的 delay_ms:
delay_ms(100);
```

原因: systick_delay_ms 内部重配 SysTick 寄存器并关闭 TICKINT 中断, 导致 TimeBase 停止增长, sys_now() 返回固定值, 所有超时逻辑失效。

### 6.2 不要加 alive check

模块通信正常时不需要心跳检测。hgic_sdspi_detect_alive() 不是心跳, 是检测物理存在的, 正常工作时也返回失败, 会导致误触发 reinit 死循环。

### 6.3 hgic_raw_rx 必须检查返回值

```c
// 正确写法:
HGIC_RAW_RX_TYPE rx_type = hgic_raw_rx(&p_buf, &p_len);
if (rx_type == HGIC_RAW_RX_TYPE_DATA && p_len > 0) {
    App_RxDataHandler(p_buf, p_len);
} else if (rx_type == HGIC_RAW_RX_TYPE_EVENT) {
    // 处理事件
}
```

不检查返回值会导致数据包丢弃, AP 侧 RX 速率只有 TX 的 72%。

### 6.4 AP 和 STA 必须分别编译

```c
// App_R900PNR_Handle.h 中:
#define APP_WIFI_ROLE  APP_ROLE_AP    // AP 板
#define APP_WIFI_ROLE  APP_ROLE_STA   // STA 板
```

两块板子用同一个工程, 改宏后重新编译烧录。

### 6.5 SPI 分频只有 2 的幂次

N32H785 SPI 分频器: /2, /4, /8, /16, /32, /64, /128, /256

无法得到精确的 30/36MHz。当前用 /4 = 37.5MHz (APB2=150MHz), 谐波不干扰 916MHz 射频。

### 6.6 DMA 不可用

hgic_sdspi_v2.c 内部用同一 buffer 做 TX+RX, DMA + D-Cache 会导致数据损坏。必须用 CPU 轮询模式。

### 6.7 LWIP 当前禁用

```c
#define LWIP_ENABLE (0u)  // main.c 第 19 行
```

如需启用 LWIP 以太网功能, 改为 1 并添加以太网初始化代码。当前工程保留 LWIP 代码但未启用。

---

## 七、数据流路径

### 7.1 发送 (STA → AP)

```
App_DemoSendH265()
  → 填充 s_tx_buf[1400 字节]
  → hgic_raw_send_ether(s_tx_buf, 1400)
    → hgic_sdspi_write()          // SPI 写入模块
      → R900PNR_SPI_Write()       // 底层 SPI 读写
        → CS 拉低 → SPI 交换 → CS 拉高
  → R900PNR 模块 WiFi 射频发送
  → AP 端 R900PNR 模块接收
  → PA8 拉低 (INT 中断)
```

### 7.2 接收 (AP 侧)

```
PA8 下降沿中断 (EXTI8)
  → g_r900pnr_spi_rx_flag = 1

App_R900PnrPoll() 主循环
  → 检查 g_r900pnr_spi_rx_flag 或 PA8 电平
  → hgic_sdspi_read(0, s_spi_rx_buf, 2048, 0)
  → hgic_raw_rx(&p_buf, &p_len)
    → 返回 HGIC_RAW_RX_TYPE_DATA
  → App_RxDataHandler(p_buf, p_len)
    → 统计 [RX STAT] pkts=N bytes=N rate=N Kbps
```

---

## 八、测试结果汇总

| 距离 | TX 速率 (STA) | RX 速率 (AP) | fail | 状态 |
|------|-------------|-------------|------|------|
| 30m | 12.9 Mbps | 12.9 Mbps | 0 | 稳定 |
| 100m | 10-13 Mbps | 10-13 Mbps | 0 | 稳定 |
| 300m | 3-6 Mbps | 3-6 Mbps | 0 | 稳定 |
| 500m | 1-3 Mbps | 1-3 Mbps | 0 | 可用 |

SPI 37.5MHz, MCS7, WPA-PSK, 超级功率开启, ACK 超时=10

---

## 九、后续扩展方向

1. **H.265 视频传输**: 在 App_DemoSendH265 基础上加帧头封装 (帧类型/序号/分片信息), 接收端重组 NAL 单元
2. **启用 LWIP**: 改 LWIP_ENABLE=1, 通过 UDP/TCP 传输, 获得可靠传输但牺牲带宽
3. **双向通信**: AP 侧也加发送逻辑, 当前 AP 只收不发
4. **多 STA**: AP 侧支持多个 STA 连接, 需要管理 sta_list
5. **降低 BSS 带宽**: 远距离时把 BW 从 8 降到 4 或 2, 换取更高接收灵敏度 (速率会降)
6. **MCS 限制**: 远距离连不上时限制 MCS 到 0-3, 降低速率换取连接稳定性

---

*文档结束*
