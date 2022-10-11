#define PART_TM4C1294NCPDT
#include "dac124S085.h"

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"

#ifdef _DAC124S085_H_

// void DACInitialize(struct DAC124S085 *dac, const uint32_t ssi_base)
// {
//     switch (ssi_base)
//     {
//     case SSI0_BASE:
//         SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
//         SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//         GPIOPinConfigure(GPIO_PA2_SSI0CLK);
//         GPIOPinConfigure(GPIO_PA3_SSI0FSS);
//         GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
//         GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);
//         GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
//         break;
//     case SSI2_BASE:
//         SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
//         SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//         GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
//         break;
//     }

//     SSIConfigSetExpClk(ssi_base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 100000, 16);

//     static struct DAC124S085 new_dac = {.SSI.base = SSI0_BASE,
//                                         .channel[0].bits.addr = 0b00,
//                                         .channel[1].bits.addr = 0b01,
//                                         .channel[2].bits.addr = 0b10,
//                                         .channel[3].bits.addr = 0b11};
//     dac = &new_dac;
// }

void DACEnable(struct DAC124S085 *dac)
{
    switch (dac->SSI.base)
    {
    case SSI0_BASE:
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        GPIOPinConfigure(GPIO_PA2_SSI0CLK);
        GPIOPinConfigure(GPIO_PA3_SSI0FSS);
        GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);
        GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);
        GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
        break;
    case SSI2_BASE:
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
        break;
    }

    SSIConfigSetExpClk(dac->SSI.base, SysCtlClockGet(), SSI_FRF_MOTO_MODE_2, SSI_MODE_MASTER, 4000000, 16);
    SSIEnable(dac->SSI.base);
}

void DACDisable(struct DAC124S085 *dac)
{
    SSIDataPut(dac->SSI.base, DAC_POWER_DOWN);
    SSIDisable(dac->SSI.base);
}

void DACSetChannel(struct DAC124S085 *dac, int channel_no, uint16_t uint12_val)
{
    dac->channel[channel_no].bits.mode = 0b00;
    dac->channel[channel_no].bits.value = uint12_val & 0xFFF;
    SSIDataPut(dac->SSI.base, dac->channel[channel_no].total);
}

void DACUpdateChannel(struct DAC124S085 *dac, int channel_no, uint16_t uint12_val)
{
    dac->channel[channel_no].bits.mode = 0b01;
    dac->channel[channel_no].bits.value = uint12_val & 0xFFF;
    SSIDataPut(dac->SSI.base, dac->channel[channel_no].total);
}

void DACUpdateAll(struct DAC124S085 *dac, uint16_t uint12_val)
{
    for (int i = 0; i < 4; i++)
    {
        dac->channel[i].bits.mode = 0b10;
        dac->channel[i].bits.value = uint12_val & 0xFFF;
    }
    SSIDataPut(dac->SSI.base, dac->channel[3].total);
}
#endif