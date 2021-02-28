#include <stdio.h>
#include "board.h"

#include "net/netdev.h"
#include "thread.h"
#include "xtimer.h"

#include "lpsxxx.h"
#include "lpsxxx_internal.h"
#include "lpsxxx_params.h"
// #include "net/gnrc/netif.h"

#include "common.h"

#define _STACKSIZE (THREAD_STACKSIZE_DEFAULT + THREAD_EXTRA_STACKSIZE_PRINTF)
#define MSG_TYPE_ISR (0x3456)

#define _MAX_ADDR_LEN (8)

#ifndef MAIN_NODE
#define MAIN_NODE "bead723d9a668c34"
#endif

static char stack[_STACKSIZE];
static kernel_pid_t _recv_pid;

at86rf2xx_t devs[AT86RF2XX_NUM];

static void platform_run(void)
{
    int i, j, size;
    int temp_abs;
    int16_t temp;
    lpsxxx_t sensor;
    char data[32];
    uint8_t addr[_MAX_ADDR_LEN];
    int res;
    le_uint16_t pan = {0};

    // gnrc_netif_t *netif = NULL;
    // netif = gnrc_netif_iter(netif);

    puts("Run platform\n");
    j = lpsxxx_init(&sensor, &lpsxxx_params[0]);
    if (j == -LPSXXX_ERR_NODEV)
        puts("No valid device found\n");
    else if (j == -LPSXXX_ERR_I2C)
        puts("I2C error\n");
    else
    {
        res = parse_addr(addr, sizeof(addr), MAIN_NODE);
        for (i = 0; i < 20; i++)
        {
            j = lpsxxx_enable(&sensor);
            if (j == -LPSXXX_ERR_I2C)
                puts("I2C error\n");
            else
            {
                xtimer_sleep(1);
                j = lpsxxx_read_temp(&sensor, &temp);
                if (j == LPSXXX_OK)
                {
                    temp_abs = temp / 100;
                    temp -= temp_abs * 100;
                    snprintf(data, 32, "temp : %2i.%02iC", temp_abs, temp);
                    data[31] = '\0';
                    send(0, pan, addr, res, data);
                    (void)size;
                }
            }
            lpsxxx_disable(&sensor);
        }
    }
}

static void _event_cb(netdev_t *dev, netdev_event_t event)
{
    if (event == NETDEV_EVENT_ISR)
    {
        msg_t msg;

        msg.type = MSG_TYPE_ISR;
        msg.content.ptr = dev;

        if (msg_send(&msg, _recv_pid) <= 0)
            puts("gnrc_netdev: possibly lost interrupt.");
    }
    else
    {
        switch (event)
        {
        case NETDEV_EVENT_RX_COMPLETE:
        {
            recv(dev);
            break;
        }
        default:
            puts("Unexpected event received");
            break;
        }
    }
}

void *_recv_thread(void *arg)
{
    (void)arg;
    while (1)
    {
        msg_t msg;
        msg_receive(&msg);
        if (msg.type == MSG_TYPE_ISR)
        {
            netdev_t *dev = msg.content.ptr;
            dev->driver->isr(dev);
        }
        else
            puts("unexpected message type");
    }
}

int main(void)
{
    board_init();

    puts("AT86RF2xx device driver test");

    unsigned dev_success = 0;
    for (unsigned i = 0; i < AT86RF2XX_NUM; i++)
    {
        netopt_enable_t en = NETOPT_ENABLE;
        const at86rf2xx_params_t *p = &at86rf2xx_params[i];
        netdev_t *dev = (netdev_t *)(&devs[i]);

        printf("Initializing AT86RF2xx radio at SPI_%d\n", p->spi);
        at86rf2xx_setup(&devs[i], p, i);
        dev->event_callback = _event_cb;
        if (dev->driver->init(dev) < 0)
            continue;
        dev->driver->set(dev, NETOPT_RX_END_IRQ, &en, sizeof(en));
        dev_success++;
    }

    if (!dev_success)
    {
        puts("No device could be initialized");
        return 1;
    }

    _recv_pid = thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1,
                              THREAD_CREATE_STACKTEST, _recv_thread, NULL,
                              "recv_thread");

    if (_recv_pid <= KERNEL_PID_UNDEF)
    {
        puts("Creation of receiver thread failed");
        return 1;
    }

    /* start the shell */
    puts("Initialization successful - starting the shell now");

    platform_run();

    return 0;
}
