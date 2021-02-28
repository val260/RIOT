/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net/netdev/ieee802154.h"
#include "net/ieee802154.h"

#include "common.h"

#include "od.h"

#define _MAX_ADDR_LEN (8)
#define MAC_VECTOR_SIZE (2) /* mhr + payload */

static inline int _dehex(char c, int default_)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else
    {
        return default_;
    }
}

size_t parse_addr(uint8_t *out, size_t out_len, const char *in)
{
    const char *end_str = in;
    uint8_t *out_end = out;
    size_t count = 0;
    int assert_cell = 1;

    if (!in || !*in)
    {
        return 0;
    }
    while (end_str[1])
    {
        ++end_str;
    }

    while (end_str >= in)
    {
        int a = 0, b = _dehex(*end_str--, -1);
        if (b < 0)
        {
            if (assert_cell)
            {
                return 0;
            }
            else
            {
                assert_cell = 1;
                continue;
            }
        }
        assert_cell = 0;

        if (end_str >= in)
        {
            a = _dehex(*end_str--, 0);
        }

        if (++count > out_len)
        {
            return 0;
        }
        *out_end++ = (a << 4) | b;
    }
    if (assert_cell)
    {
        return 0;
    }
    /* out is reversed */

    while (out < --out_end)
    {
        uint8_t tmp = *out_end;
        *out_end = *out;
        *out++ = tmp;
    }

    return count;
}

int send(int iface, le_uint16_t dst_pan, uint8_t *dst, size_t dst_len,
         char *data)
{
    int res;
    netdev_ieee802154_t *dev;
    uint8_t *src;
    size_t src_len;
    uint8_t mhr[IEEE802154_MAX_HDR_LEN];
    uint8_t flags;
    le_uint16_t src_pan;

    if (((unsigned)iface) > (AT86RF2XX_NUM - 1))
    {
        printf("txtsnd: %d is not an interface\n", iface);
        return 1;
    }

    iolist_t iol_data = {
        .iol_base = data,
        .iol_len = strlen(data)};

    dev = (netdev_ieee802154_t *)&devs[iface];
    flags = (uint8_t)(dev->flags & NETDEV_IEEE802154_SEND_MASK);
    flags |= IEEE802154_FCF_TYPE_DATA;
    src_pan = byteorder_btols(byteorder_htons(dev->pan));
    if (dst_pan.u16 == 0)
    {
        dst_pan = src_pan;
    }
    if (dev->flags & NETDEV_IEEE802154_SRC_MODE_LONG)
    {
        src_len = 8;
        src = dev->long_addr;
    }
    else
    {
        src_len = 2;
        src = dev->short_addr;
    }
    /* fill MAC header, seq should be set by device */
    if ((res = ieee802154_set_frame_hdr(mhr, src, src_len,
                                        dst, dst_len,
                                        src_pan, dst_pan,
                                        flags, dev->seq++)) < 0)
    {
        puts("txtsnd: Error preperaring frame");
        return 1;
    }

    iolist_t iol_hdr = {
        .iol_next = &iol_data,
        .iol_base = mhr,
        .iol_len = (size_t)res};

    res = dev->netdev.driver->send((netdev_t *)dev, &iol_hdr);
    if (res < 0)
    {
        puts("txtsnd: Error on sending");
        return 1;
    }
    else
    {
        printf("txtsnd: send %u bytes to ", (unsigned)iol_data.iol_len);
        print_addr(dst, dst_len);
        printf(" (PAN: ");
        print_addr((uint8_t *)&dst_pan, sizeof(dst_pan));
        puts(")");
    }
    return 0;
}

/** @} */
