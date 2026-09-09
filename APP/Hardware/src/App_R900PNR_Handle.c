#include "app_r900pnr_handle.h"
#include <string.h>

/*============================================================================*
 * 私有变量 (仅本文件可见)
 *============================================================================*/
static const uint8_t s_peer_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t s_spi_rx_buf[2048u];

#if (APP_WIFI_ROLE == APP_ROLE_STA)
static uint8_t  s_wifi_connected = 0;
static uint8_t  s_tx_started = 0;
static uint32_t s_assoc_start_tick = 0;
static uint32_t s_tx_total_bytes = 0;
static uint32_t s_tx_total_pkts  = 0;
static uint32_t s_tx_fail_cnt    = 0;
static uint32_t s_tx_last_print_tick = 0;
#endif

#if (APP_WIFI_ROLE == APP_ROLE_AP)
static uint32_t s_rx_total_bytes = 0;
static uint32_t s_rx_total_pkts  = 0;
static uint32_t s_rx_last_print_tick = 0;
#endif

/*============================================================================*
 * delay_ms: 基于 sys_now 的毫秒延时, 不破坏 SysTick 中断
 *============================================================================*/
void delay_ms(uint32_t ms)
{
    uint32_t start = sys_now();
    while ((sys_now() - start) < ms)
    {
    }
}

/*============================================================================*
 * hgic_platform_raw_send: 厂商驱动回调, 发送 raw 数据
 *============================================================================*/
int hgic_platform_raw_send(unsigned char *data, unsigned int len)
{
    return hgic_sdspi_write(0, data, len);
}

/*============================================================================*
 * App_WaitFwInfo: 查询固件版本, 等待模块回复
 *============================================================================*/
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
            int read_len = hgic_sdspi_read(0, s_spi_rx_buf, sizeof(s_spi_rx_buf), 0);
            if (read_len > 0)
            {
                uint8_t *p_buf = s_spi_rx_buf;
                uint32_t p_len = (uint32_t)read_len;
                hgic_raw_rx(&p_buf, &p_len);
            }
        }
    }
}

/*============================================================================*
 * App_WiFiConfigure: 配置 WiFi 参数 (AP / STA)
 *============================================================================*/
int App_WiFiConfigure(void)
{
    int ret = 0;

#if (APP_WIFI_ROLE == APP_ROLE_AP)
    /* AP 模式 */
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
    /* STA 模式 */
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

    /* 频率范围: 908-924MHz, 带宽 8MHz */
    uint16_t chan_list[] = APP_WIFI_CHAN_LIST;
    ret = hgic_raw_set_freq_range(chan_list[0], chan_list[2], APP_WIFI_BSS_BW);
    log_info("[CFG] set_freq_range = %d\r\n", ret);

    /* 带宽 */
    ret = hgic_raw_set_bss_bw(APP_WIFI_BSS_BW);
    log_info("[CFG] set_bss_bw = %d\r\n", ret);

    /* 信道列表 */
    ret = hgic_raw_set_chan_list(chan_list, 3);
    log_info("[CFG] set_chan_list = %d\r\n", ret);

    /* 超功率模式 */
    ret = hgic_raw_set_supper_pwr(1);
    log_info("[CFG] set_supper_pwr = %d\r\n", ret);

    /* ACK 超时 (远距离用) */
    ret = hgic_raw_set_acktmo(10);
    log_info("[CFG] set_acktmo = %d\r\n", ret);

    /* 开启射频 */
    ret = hgic_raw_open();
    log_info("[CFG] open = %d\r\n", ret);

    /* 保存配置到 flash */
    ret = hgic_raw_save();
    log_info("[CFG] save = %d\r\n", ret);

#if (APP_WIFI_ROLE == APP_ROLE_STA)
    /* STA 发起关联 */
    ret = hgic_raw_start_assoc();
    log_info("[CFG] start_assoc = %d\r\n", ret);
    s_assoc_start_tick = sys_now();
#endif

    return ret;
}

/*============================================================================*
 * App_RxDataHandler: 接收数据处理回调
 *============================================================================*/
void App_RxDataHandler(uint8_t *data, uint32_t len)
{
#if (APP_WIFI_ROLE == APP_ROLE_AP)
    s_rx_total_bytes += len;
    s_rx_total_pkts++;

    if ((sys_now() - s_rx_last_print_tick) >= 1000u)
    {
        uint32_t rate = (s_rx_total_bytes * 8) / 1000;  /* Kbps */
        log_info("[RX STAT] pkts=%d payload_bytes=%d rate=%d Kbps\r\n",
                 s_rx_total_pkts, s_rx_total_bytes, rate);
        s_rx_total_bytes = 0;
        s_rx_total_pkts  = 0;
        s_rx_last_print_tick = sys_now();
    }
#endif
}

/*============================================================================*
 * App_EventHandler: 事件回调
 *============================================================================*/
void App_EventHandler(uint8_t event_id, int16_t value)
{
#if (APP_WIFI_ROLE == APP_ROLE_STA)
    if (event_id == HGIC_EVENT_CONECTED)
    {
        s_wifi_connected = 1;
        log_info("[EVENT] WiFi connected!\r\n");
    }
    else if (event_id == HGIC_EVENT_DISCONECTED)
    {
        s_wifi_connected = 0;
        log_info("[EVENT] WiFi disconnected!\r\n");
    }
#endif
}

/*============================================================================*
 * App_SendData: 发送数据
 *============================================================================*/
int App_SendData(uint8_t *data, uint32_t len)
{
    return hgic_raw_send_ether(data, len);
}

/*============================================================================*
 * App_DemoSendH265: 演示发送 H265 数据 (循环发送测试数据)
 *============================================================================*/
#if (APP_WIFI_ROLE == APP_ROLE_STA)
void App_DemoSendH265(void)
{
    static uint8_t tx_buf[APP_TX_BUF_SIZE];

    /* 检查关联超时 */
    if (!s_tx_started && !s_wifi_connected)
    {
        if ((sys_now() - s_assoc_start_tick) >= APP_ASSOC_TIMEOUT_MS)
        {
            s_tx_started = 1;
            log_info("[TX] assoc timeout, start sending anyway\r\n");
        }
        else
        {
            return;
        }
    }

    if (!s_tx_started && s_wifi_connected)
    {
        s_tx_started = 1;
        log_info("[TX] WiFi connected, start sending\r\n");
    }

    if (s_tx_started)
    {
        /* 填充测试数据 */
        memset(tx_buf, 0xAA, APP_TX_PAYLOAD_LEN);

        int ret = hgic_raw_send_ether(tx_buf, APP_TX_PAYLOAD_LEN);
        if (ret > 0)
        {
            s_tx_total_bytes += APP_TX_PAYLOAD_LEN;
            s_tx_total_pkts++;
        }
        else
        {
            s_tx_fail_cnt++;
        }

        /* 每秒打印统计 */
        if ((sys_now() - s_tx_last_print_tick) >= 1000u)
        {
            uint32_t rate = (s_tx_total_bytes * 8) / 1000;  /* Kbps */
            log_info("[TX STAT] pkts=%d bytes=%d rate=%d Kbps fail=%d\r\n",
                     s_tx_total_pkts, s_tx_total_bytes, rate, s_tx_fail_cnt);
            s_tx_total_bytes = 0;
            s_tx_total_pkts  = 0;
            s_tx_fail_cnt    = 0;
            s_tx_last_print_tick = sys_now();
        }
    }
}
#else

#endif
void App_R900PnrPoll(void)
{
    /* 1. SPI 接收处理 */
    if (g_r900pnr_spi_rx_flag ||
        GPIO_ReadInputDataBit(R900PNR_SPI_INT_PORT, R900PNR_SPI_INT_PIN) == 0)
    {
        g_r900pnr_spi_rx_flag = 0;
        int read_len = hgic_sdspi_read(0, s_spi_rx_buf, sizeof(s_spi_rx_buf), 0);
        if (read_len > 0)
        {
            uint8_t *p_buf = s_spi_rx_buf;
            uint32_t p_len = (uint32_t)read_len;
            HGIC_RAW_RX_TYPE rx_type = hgic_raw_rx(&p_buf, &p_len);

            if (rx_type == HGIC_RAW_RX_TYPE_DATA && p_len > 0)
            {
                App_RxDataHandler(p_buf, p_len);
            }
            else if (rx_type == HGIC_RAW_RX_TYPE_EVENT)
            {
                /* event 格式: event_id(1B) + value(2B) */
                if (p_len >= 1)
                {
                    uint8_t event_id = p_buf[0];
                    int16_t value = (p_len >= 3) ? (int16_t)(p_buf[1] | (p_buf[2] << 8)) : 0;
                    App_EventHandler(event_id, value);
                }
            }
        }
    }
    /* 2. 数据发送 (仅 STA) */
#if (APP_WIFI_ROLE == APP_ROLE_STA)
    App_DemoSendH265();
#endif
}



/**
 * @brief 初始化 R900PNR Wi-Fi 模块硬件接口
 *
 * 该函数执行以下步骤：
 * 1. 初始化 SPI 接口（失败则返回错误码）。
 * 2. 初始化 SDIO 接口，若模块未就绪则按配置的重试次数和间隔进行重试。
 * 3. 等待 SDIO 总线稳定。
 * 4. 输出配置 Wi-Fi 的日志。
 *
 * @return 0 表示初始化成功；负值表示失败，具体错误码见 @ref r900pnr_error_t。
 */
int r900pnr_wifi_module_init(void)
{
    int ret = 0;

    /* 1. 初始化 SPI 接口 */
    if (R900PNR_SPI_Init() != 0) 
		{
        log_error("[R900PNR] SPI init failed");
        return R900PNR_ERR_SPI_INIT;
    }
    log_info("[R900PNR] SPI init success");

    /* 2. 初始化 SDIO 接口，带重试机制 */
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

    /* 3. 等待 SDIO 总线稳定 */
    delay_ms(SDIO_STABLE_DELAY_MS);
    log_info("[R900PNR] SDIO bus stable after %d ms", SDIO_STABLE_DELAY_MS);

    /* 4. 输出配置 Wi-Fi 的日志 */
    log_info("[R900PNR] Configure Wi-Fi...");

    return R900PNR_OK;
}