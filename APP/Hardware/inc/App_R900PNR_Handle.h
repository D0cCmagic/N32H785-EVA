#ifndef __APP_R900PNR_HANDLE_H__
#define __APP_R900PNR_HANDLE_H__

#include "stdint.h"
#include "n32h7xx.h"
#include "n32h7xx_gpio.h"
#include "r900pnr_spi_cfg.h"
#include "r900pnr_spi_api.h"
#include "hgic_raw.h"
#include "hgic_sdspi.h"
#include "log.h"

/* 系统时间 (main.c 中定义, SysTick 中断驱动 TimeBase++) */
uint32_t sys_now(void);
#define get_tick_ms()  sys_now()

/* 基于 sys_now 的延时, 不破坏 SysTick 中断*/
void delay_ms(uint32_t ms);

/* SPI 接收标志 (r900pnr_spi_api.c 中定义) */
extern volatile uint8_t g_r900pnr_spi_rx_flag;

/******************************************************************************
 * R900PNR 配置区
 ******************************************************************************/
#define APP_ROLE_AP                     (1u)
#define APP_ROLE_STA                    (0u)

/* 测试配置:
 * 接收端: APP_WIFI_ROLE = APP_ROLE_AP (AP 模式, 只收统计)
 * 发送端: APP_WIFI_ROLE = APP_ROLE_STA (STA 模式, 连 AP 后全速发包)
 */
#define APP_WIFI_ROLE                   APP_ROLE_AP

#define APP_WIFI_SSID                   "hgic_ah_test"
#define APP_WIFI_PSK                    "12345678"
#define APP_WIFI_BSS_BW                 (8u)
#define APP_WIFI_CHAN_LIST              {9080u, 9160u, 9240u}

#define APP_ALIVE_PERIOD_MS             (100u)
#define APP_TX_BUF_SIZE                 (1500u)

#if (APP_WIFI_ROLE == APP_ROLE_STA)
    #define APP_TX_PAYLOAD_LEN          (1400u)
#else
    #define APP_TX_PAYLOAD_LEN          (100u)
#endif

#define APP_ASSOC_TIMEOUT_MS            (3000u)



#define R900PNR_SPI_INIT_RETRY_COUNT       (0)     /* SPI 初始化失败不重试，立即返回错误 */
#define SDIO_INIT_MAX_RETRY_COUNT          (5)     /* SDIO 初始化最大重试次数 */
#define SDIO_INIT_RETRY_DELAY_MS           (1000)  /* SDIO 初始化重试间隔（毫秒） */
#define SDIO_STABLE_DELAY_MS               (200)   /* SDIO 总线稳定等待时间（毫秒） */


/* 错误码定义 */
typedef enum {
    R900PNR_OK = 0,                 /**< 成功 */
    R900PNR_ERR_SPI_INIT = -1,      /**< SPI 初始化失败 */
    R900PNR_ERR_SDIO_INIT = -2,     /**< SDIO 初始化失败 */
} r900pnr_error_t;
/******************************************************************************
 * 函数声明
 ******************************************************************************/
void App_RxDataHandler(uint8_t *data, uint32_t len);
void App_EventHandler(uint8_t event_id, int16_t value);
int  App_WiFiConfigure(void);
void App_R900PnrPoll(void);
void App_DemoSendH265(void);
int  App_SendData(uint8_t *data, uint32_t len);
int  hgic_platform_raw_send(unsigned char *data, unsigned int len);
void App_WaitFwInfo(void);
int r900pnr_wifi_module_init(void);
#endif
