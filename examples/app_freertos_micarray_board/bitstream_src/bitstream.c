// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int initialized;

soc_peripheral_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];
soc_peripheral_t bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_COUNT];
soc_peripheral_t bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_COUNT];
soc_peripheral_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];
soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];

void device_register(
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A] = soc_peripheral_register(mic_dev_ch);
    bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_A] = soc_peripheral_register(eth_dev_ch);
    bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_A] = soc_peripheral_register(i2s_dev_ch);
    bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_A] = soc_peripheral_register(i2c_dev_ch);
    bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A] = soc_peripheral_register(t0_gpio_dev_ch);
    bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_B] = soc_peripheral_register(t1_gpio_dev_ch);

    initialized = 1;
}

int soc_tile0_bitstream_initialized(void)
{
    return initialized;
}

void soc_tile0_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    i2c_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    i2c_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2c_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    i2c_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    t1_gpio_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);

    tile0_device_instantiate(eth_dev_ch, i2s_dev_ch, i2c_dev_ch, t1_gpio_dev_ch);
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    i2c_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    i2c_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2c_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    i2c_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    t1_gpio_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);

    tile1_device_instantiate(eth_dev_ch, i2s_dev_ch, i2c_dev_ch, t1_gpio_dev_ch);
}
