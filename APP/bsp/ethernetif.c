/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"

#include "ethernetif.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'n'
#define IFNAME1 'a'

#define MEMP_NUM_RX_POOL        (32)

extern uint8_t aMACAddr[6];
extern ETH_InfoType sEthInfo;
extern ETH_TxPacketType sTxPacket;


LWIP_MEMPOOL_DECLARE(RX_POOL, MEMP_NUM_RX_POOL, sizeof(struct pbuf_custom), "RX_POOL");


/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
};


static void low_level_init(struct netif *netif);
static err_t low_level_output(struct netif *netif, struct pbuf *p);
static struct pbuf* low_level_input(struct netif *netif);


/**
  * @brief  Custom Rx pbuf free callback
  * @param  pbuf: pbuf to be freed
  * @retval None
  */
void pbufFreeCustom(struct pbuf *p)
{
    struct pbuf_custom* pCustomPbuf = (struct pbuf_custom*)p;

    LWIP_MEMPOOL_FREE(RX_POOL, pCustomPbuf);
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
    ErrorStatus EthInitStatus;

    /* Initialize ETH */
    EthInitStatus = ETH_BSP_Init();
    if (EthInitStatus == ERROR)
    {
        printf("#ERROR: ETH BSP Initialize error\n");
    }
    else
    {
        /* Set netif link flag */
        netif_set_link_up(netif);
    }

    /* Set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* Set MAC hardware address to netif */
    netif->hwaddr[0] = MAC_ADDR0;
    netif->hwaddr[1] = MAC_ADDR1;
    netif->hwaddr[2] = MAC_ADDR2;
    netif->hwaddr[3] = MAC_ADDR3;
    netif->hwaddr[4] = MAC_ADDR4;
    netif->hwaddr[5] = MAC_ADDR5;

    /* Maximum transfer unit */
    netif->mtu = ETH_NETIF_MTU;

    /* Device capabilities */
    /* Don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP);

#if LWIP_IPV6 && LWIP_IPV6_MLD
    /*
    * For hardware/netifs that implement MAC filtering.
    * All-nodes link-local is handled by default, so we must let the hardware know
    * to allow multicast packets in.
    * Should set mld_mac_filter previously. */
    if (netif->mld_mac_filter != NULL) {
        ip6_addr_t ip6_allnodes_ll;
        ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
        netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    LWIP_MEMPOOL_INIT(RX_POOL);

    /* Turn on ETH */
    ETH_StartIT(ETH, &sEthInfo);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q = NULL;
    uint32_t TempIndex = 0;
    uint32_t PacketLen = 0;
    static ETH_BufferType sTxbuffer[ETH_TX_DESC_NUMBER];
    
    memset(sTxbuffer, 0, sizeof(sTxbuffer));

    for (q = p; q != NULL; q = q->next)
    {
        if (TempIndex >= ETH_TX_DESC_NUMBER)
        {
            return (err_t)ERR_IF;
        }
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        sTxbuffer[TempIndex].pBuf = q->payload;
        sTxbuffer[TempIndex].Len  = q->len;
        PacketLen                += q->len;
        if (TempIndex > 0)
        {
            sTxbuffer[TempIndex - 1].pNext = &sTxbuffer[TempIndex];
        }
        if (q->next == NULL)
        {
            sTxbuffer[TempIndex].pNext = NULL;
        }
        TempIndex++;
    }

    sTxPacket.Length = PacketLen;
    sTxPacket.pTxBuffer = sTxbuffer;
    
    SCB_CleanDCache_by_Addr((uint32_t *)sTxPacket.pTxBuffer->pBuf, sTxPacket.Length);
    
    ETH_Transmit(ETH, &sEthInfo, &sTxPacket, 10U);

    return (err_t)ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf* low_level_input(struct netif *netif)
{
    uint32_t i = 0;
    struct pbuf *p = NULL;
    uint16_t PacketLen = 0;
    uint32_t TempIndex = sEthInfo.RxDescList.FirstAppDesc;
    static ETH_BufferType sRxbuffer[ETH_RX_DESC_NUMBER];
    struct pbuf_custom* pCustomPbuf = NULL;
    
    for (i = 0; i < ETH_RX_DESC_NUMBER; i++)
    {
        sRxbuffer[i].pNext = ((i < (ETH_RX_DESC_NUMBER - 1)) ? &sRxbuffer[i + 1] : &sRxbuffer[0]);
    }
    
    /* Get the data buffer */
    if (ETH_GetRxDataBuffer(&sEthInfo, &sRxbuffer[TempIndex]) == ETH_SUCCESS)
    {
        /* Get the data length */
        PacketLen = ETH_GetRxDataLength(&sEthInfo);
        /* Valid data is processed by pbuf only when it is received */
        if (PacketLen != 0U)
        {
            SCB_InvalidateDCache_by_Addr(sRxbuffer[TempIndex].pBuf, PacketLen);
            
            /* Frequent memory allocation without timely release may lead to memory overflows */
            pCustomPbuf = (struct pbuf_custom*)LWIP_MEMPOOL_ALLOC(RX_POOL);
            if (pCustomPbuf == NULL)
            {
                LWIP_DEBUGF(PBUF_DEBUG, ("Failed to allocate pCustomPbuf\n"));
                return p;
            }
            /* Register a custom pbuf release function */
            pCustomPbuf->custom_free_function = pbufFreeCustom;
            /* Create a custom pbuf to enable zero-copy data reception */
            p = pbuf_alloced_custom(PBUF_RAW, PacketLen, PBUF_REF, pCustomPbuf, sRxbuffer[TempIndex].pBuf, ETH_RX_BUFFER_SIZE);
            if (p == NULL)
            {
                LWIP_DEBUGF(PBUF_DEBUG, ("pbuf_alloced_custom failed\n"));
                LWIP_MEMPOOL_FREE(RX_POOL, pCustomPbuf);
                pCustomPbuf = NULL;
            }
        }
    }

    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(struct netif *netif)
{
    struct pbuf *p = NULL;
    uint8_t noReceivedCompleteFlag = 0;
    
    do
    {
        /* Move received packet into a new pbuf */
        p = low_level_input(netif);
        /* Build Rx descriptor to be ready for next data reception */
        ETH_BuildRxDescriptors(ETH, &sEthInfo);
        
        /* If no packet could be read, silently ignore this */
        if (p != NULL)
        {
            /* Pass all packets to ethernet_input, which decides what packets it supports */
            if (netif->input(p, netif) != ERR_OK)
            {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                pbuf_free(p);
                p = NULL;
            }
            else
            {
                /* Attempt to continue fetching the remaining data fragments */
                noReceivedCompleteFlag = 1;
            }
        }
        else
        {
            /* No remaining packets to receive, jump out of loop */
            noReceivedCompleteFlag = 0;
        }
    } while (noReceivedCompleteFlag);
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc((mem_size_t)sizeof(struct ethernetif));
    if (ethernetif == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return (err_t)ERR_MEM;
    }

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
//    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;

    /*
    * We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...)
    */
#if LWIP_IPV4
    netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    /* Initialize the hardware */
    low_level_init(netif);
    
    ethernetif->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);

    return (err_t)ERR_OK;
}

