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

/* System tick in milliseconds, defined in main.c and incremented by SysTick */
uint32_t sys_now(void);
#define get_tick_ms()  sys_now()

/* Millisecond delay based on sys_now, does not block SysTick */
void delay_ms(uint32_t ms);

/* SPI receive flag, defined in r900pnr_spi_api.c */
extern volatile uint8_t g_r900pnr_spi_rx_flag;

/* R900PNR application settings */
#define APP_ROLE_AP                     (1u)
#define APP_ROLE_STA                    (0u)

/* Select the role for this build:
 * receiver: APP_WIFI_ROLE = APP_ROLE_AP  (AP mode, statistics only)
 * sender:   APP_WIFI_ROLE = APP_ROLE_STA (STA mode, sends at full speed)
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



#define R900PNR_SPI_INIT_RETRY_COUNT       (0)     /* SPI init is not retried, returns error */
#define SDIO_INIT_MAX_RETRY_COUNT          (5)     /* Max SDIO init attempts */
#define SDIO_INIT_RETRY_DELAY_MS           (1000)  /* Delay between SDIO init attempts, ms */
#define SDIO_STABLE_DELAY_MS               (200)   /* Bus settle delay after SDIO init, ms */


/* Return codes */
typedef enum {
    R900PNR_OK = 0,                 /**< Success */
    R900PNR_ERR_SPI_INIT = -1,      /**< SPI init failed */
    R900PNR_ERR_SDIO_INIT = -2,     /**< SDIO init failed */
} r900pnr_error_t;

/* Public API */
void App_RxDataHandler(uint8_t *data, uint32_t len);
void App_EventHandler(uint8_t event_id, int16_t value);
int  App_WiFiConfigure(void);
void App_R900PnrPoll(void);
void App_DemoSendH265(void);
int  App_SendData(uint8_t *data, uint32_t len);
int  hgic_platform_raw_send(unsigned char *data, unsigned int len);
void App_WaitFwInfo(void);
void App_R900PnrLinkCheck(void);
int r900pnr_wifi_module_init(void);
#endif
