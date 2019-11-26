// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <stdlib.h>
#include <string.h>

#include "soc.h"
#include "bsp/common/soc_bsp_common.h"
#include "xcore_c.h"
#include "bitstream_devices.h"

#include "gpio_driver.h"

#include "debug_print.h"

#define GPIO_TOTAL_PORT_CNT   (32)

#define GPIO_PASS   (1)
#define GPIO_FAIL   (0)

typedef struct {
    port_id_t key;
    unsigned port_res;
} port_res_lookup_t;

const port_res_lookup_t GPIO_Initial_Table [ GPIO_TOTAL_PORT_CNT ] =
{
    { port_1B,  0 }, { port_1C,  0 }, { port_1A,  0 }, { port_1D,  0 },
    { port_1F,  0 }, { port_1G,  0 }, { port_1E,  0 }, { port_1H,  0 },
    { port_1J,  0 }, { port_1K,  0 }, { port_1I,  0 }, { port_1L,  0 },
    { port_1M,  0 }, { port_1N,  0 }, { port_1O,  0 }, { port_1P,  0 },
    /* 4 Bit Ports */
    { port_4A,  0 }, { port_4B,  0 }, { port_4C,  0 }, { port_4D,  0 },
    { port_4E,  0 }, { port_4F,  0 },
    /* 8 Bit Ports */
    { port_8A,  0 }, { port_8B,  0 }, { port_8C,  0 }, { port_8D,  0 },
    /* 16 Bit Ports */
    { port_16A, 0 }, { port_16B, 0 }, { port_16C, 0 }, { port_16D, 0 },
    /* 32 Bit Ports */
    { port_32A, 0 }, { port_32B, 0 }
};

port_res_lookup_t GPIO_lookup[ BITSTREAM_GPIO_DEVICE_COUNT ][ GPIO_TOTAL_PORT_CNT ] = {};

static void debugprint_GPIO_table(void)
{
    for(int i=0; i<BITSTREAM_GPIO_DEVICE_COUNT; i++)
    {
        for(int j=0; j<GPIO_TOTAL_PORT_CNT; j++)
        {
            debug_printf("key: %d res: %d\n", GPIO_lookup[i][j].key, GPIO_lookup[i][j].port_res );
        }
    }
}

__attribute__((fptrgroup("stdlib_bsearch")))
static int compare_keys(const void *va, const void *vb)
{
    const port_res_lookup_t *a = va, *b = vb;
    return a->key - b->key;
}

static int get_GPIO_dev_id( soc_peripheral_t dev )
{
    int retVal = -1;
    for( int i=0; i< BITSTREAM_GPIO_DEVICE_COUNT; i++ )
    {
        if( dev == bitstream_gpio_devices[i] )
        {
            retVal = i;
            break;
        }
    }

    xassert( retVal >= 0 );  // Invalid device passed

    return retVal;
}

static unsigned get_port_res( soc_peripheral_t dev, port_id_t p )
{
    unsigned retVal;

    int dev_id = get_GPIO_dev_id( dev );
    const port_res_lookup_t search_key[1] = {{p}};

    port_res_lookup_t *key_pair = bsearch(search_key,
                                          GPIO_lookup[dev_id],
                                          GPIO_TOTAL_PORT_CNT,
                                          sizeof(port_res_lookup_t),
                                          compare_keys );
    if( key_pair->port_res == 0 )
    {
        xassert(0); // GPIO resource was not initalized
    }

    retVal = key_pair->port_res;

    return retVal;
}

static int set_port_res( soc_peripheral_t dev, port_id_t p, unsigned port_res )
{
    int retVal;

    int dev_id = get_GPIO_dev_id(dev);
    const port_res_lookup_t search_key[1] = {{p}};

    port_res_lookup_t *key_pair = bsearch(search_key,
                                          GPIO_lookup[dev_id],
                                          GPIO_TOTAL_PORT_CNT,
                                          sizeof(port_res_lookup_t),
                                          compare_keys );

    if( key_pair == NULL )
    {
        xassert(0); // GPIO not found
    }
    else
    {
        if( key_pair->port_res != 0 )
        {
            retVal = GPIO_FAIL;
        }
        else
        {
            key_pair->port_res = port_res;
            retVal = GPIO_PASS;
        }
    }

    return retVal;
}

static void gpio_driver_alloc(
        soc_peripheral_t dev,
        unsigned *p,
        int id)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_ALLOC );

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(unsigned), p,
            sizeof(id), &id);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(unsigned), p);
}

static void gpio_driver_write(
        soc_peripheral_t dev,
        unsigned p,
        uint32_t data)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, GPIO_DEV_PORT_OUT);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(p), &p,
            sizeof(data), &data);
}

static void gpio_driver_read(
        soc_peripheral_t dev,
        unsigned p,
        uint32_t *data)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, GPIO_DEV_PORT_IN);

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(p), &p);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(uint32_t), data);
}

static void gpio_driver_peek(
        soc_peripheral_t dev,
        unsigned p,
        uint32_t *data)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, GPIO_DEV_PORT_PEEK);

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(p), &p);

    soc_peripheral_varlist_rx(
            c, 1,
            sizeof(uint32_t), data);
}

static void gpio_driver_free(
        soc_peripheral_t dev,
        unsigned *p)
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx( c, GPIO_DEV_PORT_FREE );

    soc_peripheral_varlist_tx(
            c, 1,
            sizeof(unsigned), p);
}

int gpio_init( soc_peripheral_t dev, port_id_t p )
{
    int retVal;
    unsigned new_port_res;

    gpio_driver_alloc( dev, &new_port_res, p );

    if( ( retVal = set_port_res(dev, p, new_port_res) ) == GPIO_FAIL )
    {
        debug_printf("port already allocated\n");
    }

    return retVal;
}

void gpio_free( soc_peripheral_t dev, port_id_t p )
{
    unsigned port_res;
    port_res = get_port_res(dev, p);

    gpio_driver_free( dev, &port_res );
}

void gpio_write( soc_peripheral_t dev, port_id_t p, uint32_t value )
{
    unsigned port_res;
    port_res = get_port_res(dev, p);

    gpio_driver_write(dev, port_res, value);
}

void gpio_write_pin( soc_peripheral_t dev, port_id_t p, int pin, uint32_t value )
{
    unsigned port_res;
    uint32_t initial_state;

    port_res = get_port_res(dev, p);

    gpio_driver_peek(dev, p, &initial_state );

    if( value == 0 )
    {
        initial_state &= ~( 1 << pin );
    }
    else
    {
        initial_state |= ( 1 << pin );
    }

    gpio_driver_write(dev, port_res, initial_state);
}

uint32_t gpio_read( soc_peripheral_t dev, port_id_t p )
{
    uint32_t retVal;
    unsigned port_res;
    port_res = get_port_res(dev, p);

    gpio_driver_read(dev, p, &retVal);
    return retVal;
}

uint32_t gpio_read_pin( soc_peripheral_t dev, port_id_t p, int pin )
{
    uint32_t retVal;
    unsigned port_res;
    port_res = get_port_res(dev, p);

    gpio_driver_read(dev, p, &retVal);
    retVal = (( retVal >> pin ) & 0x0001 );
    return retVal;
}

soc_peripheral_t gpio_driver_init(
        int device_id)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_GPIO_DEVICE_COUNT);

    device = bitstream_gpio_devices[device_id];

    memcpy( GPIO_lookup[ device_id ], GPIO_Initial_Table, sizeof(GPIO_Initial_Table) );

    return device;
}