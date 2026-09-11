#include "app_r900pnr_handle.h"
#include <string.h>

/* Module-private statics */
static const uint8_t Peer_Mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t Spi_Rx_Buf[2048u];

#if (APP_WIFI_ROLE == APP_ROLE_STA)
static uint8_t  Wifi_Connected = 0;
static uint8_t  Tx_Started = 0;
static uint32_t Assoc_Start_Tick = 0;
static uint32_t Tx_Total_Bytes = 0;
static uint32_t Tx_Total_Pkts  = 0;
static uint32_t Tx_Fail_Cnt    = 0;
static uint32_t Tx_Last_Print_Tick = 0;
#endif

#if (APP_WIFI_ROLE == APP_ROLE_AP)
static uint32_t Rx_Total_Bytes = 0;
static uint32_t Rx_Total_Pkts  = 0;
static uint32_t Rx_Last_Print_Tick = 0;
#endif

/**
 * @name    delay_ms
 * @brief   Millisecond delay driven by sys_now, does not block SysTick.
 * @param   ms: delay length in milliseconds.
 * @retval  None
 */
void delay_ms(uint32_t ms)
{
    uint32_t start = sys_now();
    while ((sys_now() - start) < ms)
    {
    }
}

/**
 * @name    hgic_platform_raw_send
 * @brief   Platform hook that pushes a raw frame onto the SDIO link.
 * @param   data: pointer to the frame buffer.
 * @param   len: frame length in bytes.
 * @retval  Number of bytes written, or a negative value on error.
 */
int hgic_platform_raw_send(unsigned char *data, unsigned int len)
{
    return hgic_sdspi_write(0, data, len);
}

/**
 * @name    App_WaitFwInfo
 * @brief   Request the firmware version and poll the link until the module replies.
 * @param   None
 * @retval  None
 */
void App_WaitFwInfo(void)
{
    log_info("[INIT] get fwinfo...\r\n");
    hgic_raw_get_fwinfo();

    uint32_t fwinfo_start = sys_now();
    while ((sys_now() - fwinfo_start) < 3000u)
    {
        if (g_r900pnr_spi_rx_flag ||
            GPIO_ReadInputDataBit(R900PNR_SPI_INT_PORT, R900PNR_SPI_INT_PIN) == 0)
        {
            g_r900pnr_spi_rx_flag = 0;
            int read_len = hgic_sdspi_read(0, Spi_Rx_Buf, sizeof(Spi_Rx_Buf), 0);
            if (read_len > 0)
            {
                uint8_t *p_buf = Spi_Rx_Buf;
                uint32_t p_len = (uint32_t)read_len;
                hgic_raw_rx(&p_buf, &p_len);
            }
        }
    }
}

/**
 * @name    App_WiFiConfigure
 * @brief   Apply the SSID, key, band and channel settings for the compiled role.
 * @param   None
 * @retval  Result of the last configuration command (0 on success).
 */
int App_WiFiConfigure(void)
{
    int ret = 0;

#if (APP_WIFI_ROLE == APP_ROLE_AP)
    /* AP mode */
    ret = hgic_raw_set_mode("ap");
    if (ret != 0)
    {
        log_info("[CFG] set_mode(ap) = %d (try 1)\r\n", ret);
        delay_ms(500);
        ret = hgic_raw_set_mode("ap");
    }
    log_info("[CFG] set_mode(ap) = %d (try 2)\r\n", ret);

    ret = hgic_raw_set_ssid(APP_WIFI_SSID);
    log_info("[CFG] set_ssid = %d\r\n", ret);

    ret = hgic_raw_set_key_mgmt("wpa2");
    log_info("[CFG] set_key_mgmt = %d\r\n", ret);

    ret = hgic_raw_set_wpa_psk(APP_WIFI_PSK);
    log_info("[CFG] set_wpa_psk = %d\r\n", ret);

#elif (APP_WIFI_ROLE == APP_ROLE_STA)
    /* STA mode */
    ret = hgic_raw_set_mode("sta");
    if (ret != 0)
    {
        log_info("[CFG] set_mode(sta) = %d (try 1)\r\n", ret);
        delay_ms(500);
        ret = hgic_raw_set_mode("sta");
    }
    log_info("[CFG] set_mode(sta) = %d (try 1)\r\n", ret);

    ret = hgic_raw_set_ssid(APP_WIFI_SSID);
    log_info("[CFG] set_ssid = %d\r\n", ret);

    ret = hgic_raw_set_key_mgmt("wpa2");
    log_info("[CFG] set_key_mgmt = %d\r\n", ret);

    ret = hgic_raw_set_wpa_psk(APP_WIFI_PSK);
    log_info("[CFG] set_wpa_psk = %d\r\n", ret);
#endif

    /* Band: 908-924MHz, 8MHz bandwidth */
    uint16_t chan_list[] = APP_WIFI_CHAN_LIST;
    ret = hgic_raw_set_freq_range(chan_list[0], chan_list[2], APP_WIFI_BSS_BW);
    log_info("[CFG] set_freq_range = %d\r\n", ret);

    /* Bandwidth */
    ret = hgic_raw_set_bss_bw(APP_WIFI_BSS_BW);
    log_info("[CFG] set_bss_bw = %d\r\n", ret);

    /* Channel list */
    ret = hgic_raw_set_chan_list(chan_list, 3);
    log_info("[CFG] set_chan_list = %d\r\n", ret);

    /* Super power mode */
    ret = hgic_raw_set_supper_pwr(1);
    log_info("[CFG] set_supper_pwr = %d\r\n", ret);

    /* ACK timeout, for long-distance links */
    ret = hgic_raw_set_acktmo(10);
    log_info("[CFG] set_acktmo = %d\r\n", ret);

    /* Start the radio */
    ret = hgic_raw_open();
    log_info("[CFG] open = %d\r\n", ret);

    /* Persist the configuration to flash */
    ret = hgic_raw_save();
    log_info("[CFG] save = %d\r\n", ret);

#if (APP_WIFI_ROLE == APP_ROLE_STA)
    /* Kick off association */
    ret = hgic_raw_start_assoc();
    log_info("[CFG] start_assoc = %d\r\n", ret);
    Assoc_Start_Tick = sys_now();
#endif

    return ret;
}

/**
 * @name    App_RxDataHandler
 * @brief   Accumulate received payload and log the throughput once per second.
 * @param   data: pointer to the received payload.
 * @param   len: payload length in bytes.
 * @retval  None
 */
void App_RxDataHandler(uint8_t *data, uint32_t len)
{
#if (APP_WIFI_ROLE == APP_ROLE_AP)
    Rx_Total_Bytes += len;
    Rx_Total_Pkts++;

    if ((sys_now() - Rx_Last_Print_Tick) >= 1000u)
    {
        uint32_t rate = (Rx_Total_Bytes * 8) / 1000;  /* Kbps */
        log_info("[RX STAT] pkts=%d payload_bytes=%d rate=%d Kbps\r\n",
                 Rx_Total_Pkts, Rx_Total_Bytes, rate);
        Rx_Total_Bytes = 0;
        Rx_Total_Pkts  = 0;
        Rx_Last_Print_Tick = sys_now();
    }
#endif
}

/**
 * @name    App_EventHandler
 * @brief   Track the association state reported by the module.
 * @param   event_id: event identifier from the raw layer.
 * @param   value: event payload.
 * @retval  None
 */
void App_EventHandler(uint8_t event_id, int16_t value)
{
#if (APP_WIFI_ROLE == APP_ROLE_STA)
    if (event_id == HGIC_EVENT_CONECTED)
    {
        Wifi_Connected = 1;
        log_info("[EVENT] WiFi connected!\r\n");
    }
    else if (event_id == HGIC_EVENT_DISCONECTED)
    {
        Wifi_Connected = 0;
        log_info("[EVENT] WiFi disconnected!\r\n");
    }
#endif
}

/**
 * @name    App_SendData
 * @brief   Send one payload frame over the wireless link.
 * @param   data: pointer to the payload buffer.
 * @param   len: payload length in bytes.
 * @retval  Number of bytes sent, or a negative value on error.
 */
int App_SendData(uint8_t *data, uint32_t len)
{
    return hgic_raw_send_ether(data, len);
}

#if (APP_WIFI_ROLE == APP_ROLE_STA)
/**
 * @name    App_DemoSendH265
 * @brief   Stream a constant test payload and log the send rate once per second.
 * @param   None
 * @retval  None
 */
void App_DemoSendH265(void)
{
    static uint8_t tx_buf[APP_TX_BUF_SIZE];

    /* Wait for association to complete */
    if (!Tx_Started && !Wifi_Connected)
    {
        if ((sys_now() - Assoc_Start_Tick) >= APP_ASSOC_TIMEOUT_MS)
        {
            Tx_Started = 1;
            log_info("[TX] assoc timeout, start sending anyway\r\n");
        }
        else
        {
            return;
        }
    }

    if (!Tx_Started && Wifi_Connected)
    {
        Tx_Started = 1;
        log_info("[TX] WiFi connected, start sending\r\n");
    }

    if (Tx_Started)
    {
        /* Fill the payload */
        memset(tx_buf, 0xAA, APP_TX_PAYLOAD_LEN);

        int ret = hgic_raw_send_ether(tx_buf, APP_TX_PAYLOAD_LEN);
        if (ret > 0)
        {
            Tx_Total_Bytes += APP_TX_PAYLOAD_LEN;
            Tx_Total_Pkts++;
        }
        else
        {
            Tx_Fail_Cnt++;
        }

        /* Log the rate once per second */
        if ((sys_now() - Tx_Last_Print_Tick) >= 1000u)
        {
            uint32_t rate = (Tx_Total_Bytes * 8) / 1000;  /* Kbps */
            log_info("[TX STAT] pkts=%d bytes=%d rate=%d Kbps fail=%d\r\n",
                     Tx_Total_Pkts, Tx_Total_Bytes, rate, Tx_Fail_Cnt);
            Tx_Total_Bytes = 0;
            Tx_Total_Pkts  = 0;
            Tx_Fail_Cnt    = 0;
            Tx_Last_Print_Tick = sys_now();
        }
    }
}
#endif

/**
 * @name    App_R900PnrLinkCheck
 * @brief   Probe the SDIO link periodically and re-initialize it if it went down.
 * @param   None
 * @retval  None
 */
void App_R900PnrLinkCheck(void)
{
    static uint32_t Link_Check_Tick = 0U;

    if ((sys_now() - Link_Check_Tick) >= APP_ALIVE_PERIOD_MS)
    {
        Link_Check_Tick = sys_now();

        /* Module falls back to SD mode on a comms fault */
        if (hgic_sdspi_detect_alive(0) < 0)
        {
            log_error("[R900PNR] SDIO link lost, re-init\r\n");
            hgic_sdspi_init(0);
        }
    }
}

/**
 * @name    App_R900PnrPoll
 * @brief   Service one radio frame per call; runs as the main-loop background task.
 * @param   None
 * @retval  None
 */
void App_R900PnrPoll(void)
{
    /* 1. Link health check */
    App_R900PnrLinkCheck();

    /* 2. Receive one frame when the module signals or INT is held low */
    if (g_r900pnr_spi_rx_flag ||
        GPIO_ReadInputDataBit(R900PNR_SPI_INT_PORT, R900PNR_SPI_INT_PIN) == 0)
    {
        g_r900pnr_spi_rx_flag = 0;
        int read_len = hgic_sdspi_read(0, Spi_Rx_Buf, sizeof(Spi_Rx_Buf), 0);
        if (read_len > 0)
        {
            uint8_t *p_buf = Spi_Rx_Buf;
            uint32_t p_len = (uint32_t)read_len;
            HGIC_RAW_RX_TYPE rx_type = hgic_raw_rx(&p_buf, &p_len);

            if (rx_type == HGIC_RAW_RX_TYPE_DATA && p_len > 0)
            {
                App_RxDataHandler(p_buf, p_len);
            }
            else if (rx_type == HGIC_RAW_RX_TYPE_EVENT)
            {
                /* Event format: event_id(1B) + value(2B) */
                if (p_len >= 1)
                {
                    uint8_t event_id = p_buf[0];
                    int16_t value = (p_len >= 3) ? (int16_t)(p_buf[1] | (p_buf[2] << 8)) : 0;
                    App_EventHandler(event_id, value);
                }
            }
        }
    }

    /* 3. Send path, STA only */
#if (APP_WIFI_ROLE == APP_ROLE_STA)
    App_DemoSendH265();
#endif
}

/**
 * @name    r900pnr_wifi_module_init
 * @brief   Initialize the SPI interface and the SDIO link to the R900PNR module.
 * @param   None
 * @retval  R900PNR_OK on success, or a negative r900pnr_error_t value on failure.
 */
int r900pnr_wifi_module_init(void)
{
    int ret = 0;

    /* 1. SPI interface */
    if (R900PNR_SPI_Init() != 0)
		{
        log_error("[R900PNR] SPI init failed");
        return R900PNR_ERR_SPI_INIT;
    }
    log_info("[R900PNR] SPI init success");

    /* 2. SDIO interface, up to SDIO_INIT_MAX_RETRY_COUNT attempts */
    ret = hgic_sdspi_init(0);
    for (int attempt = 1; (ret != 0) && (attempt < SDIO_INIT_MAX_RETRY_COUNT); attempt++)
		{
			log_info("[R900PNR] SDIO init failed (attempt %d/%d), module not ready, retry in %d ms",
							 attempt, SDIO_INIT_MAX_RETRY_COUNT, SDIO_INIT_RETRY_DELAY_MS);
			delay_ms(SDIO_INIT_RETRY_DELAY_MS);
			ret = hgic_sdspi_init(0);
    }

    if (ret != 0)
		{
			log_info("[R900PNR] SDIO init failed after %d attempts", SDIO_INIT_MAX_RETRY_COUNT);
			return R900PNR_ERR_SDIO_INIT;
    }
    log_info("[R900PNR] SDIO init success");

    /* 3. Wait for the bus to settle */
    delay_ms(SDIO_STABLE_DELAY_MS);
    log_info("[R900PNR] SDIO bus stable after %d ms", SDIO_STABLE_DELAY_MS);

    log_info("[R900PNR] Configure Wi-Fi...");

    return R900PNR_OK;
}
