#define PI 3.141592653589793

#include "tm4c1294ncpdt.h"

#ifdef __TM4C1294NCPDT_H__
#define PART_TM4C1294NCPDT
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"
#include "uartstdio.h"
#include "dac124S085.h"

// Parameters can be changes as required.
const int32_t step_size = 200, saturation_margin = 40, ctrl_v_max = 4095, ctrl_v_min = 0, delay_time = 10, perturbation = 40, spring = 80;

// Parameters used for controling and tracking.
int32_t ctrl_v[4], grad[4], ctrl_v_mid, ctrl_v_range;
uint32_t init_ADC_v, fnl_ADC_v, curr_ADC_v, prev_adc_v, change_ADC_v[4];

struct DAC124S085 dac = DACInitValue(SSI0_BASE);
uint32_t sys_clk;

static inline void DelayMS(uint32_t count_ms)
{
    SysCtlDelay((sys_clk / 10e3) * count_ms);
}

void ADCRead(uint32_t *value_ADC)
{
    //
    // Trigger the ADC conversion.
    //
    ADCProcessorTrigger(ADC0_BASE, 3);

    //
    // Wait for conversion to be completed.
    //
    while (!ADCIntStatus(ADC0_BASE, 3, false))
    {
    }

    //
    // Clear the ADC interrupt flag.
    //
    ADCIntClear(ADC0_BASE, 3);

    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 3, value_ADC);
}

void handle_saturation(int32_t *ctrl_vol)
{
    if (*ctrl_vol > ctrl_v_max)
        *ctrl_vol -= (*ctrl_vol - ctrl_v_max) + perturbation;
    else if (*ctrl_vol < ctrl_v_min)
        *ctrl_vol += (ctrl_v_min - *ctrl_vol) + perturbation;
    else
        *ctrl_vol -= (*ctrl_vol - ctrl_v_mid) / perturbation;
}

void algo_sprte_GD()
{
    for (int i = 0; i < 4; i++)
    {
        DACUpdateChannel(&dac, i, ctrl_v[i] - perturbation);
        DelayMS(delay_time);
        ADCRead(&init_ADC_v);
        DACUpdateChannel(&dac, i, ctrl_v[i] + perturbation);
        DelayMS(delay_time);
        ADCRead(&fnl_ADC_v);
        grad[i] = ((int32_t)fnl_ADC_v - (int32_t)curr_ADC_v) * step_size / perturbation;
        ctrl_v[i] -= grad[i];
        handle_saturation(&ctrl_v[i]);
        DACUpdateChannel(&dac, i, ctrl_v[i]);
    }
}

void algo_RGD()
{
    int32_t rand_pertu[4], adc_del_v, range = ctrl_v_range;
    // Initial photodetector voltage is measured before perturbation.
    DelayMS(delay_time);
    ADCRead(&init_ADC_v);
    // Random perturbation is applied to all control voltages.
    for (int i = 0; i < 4; i++)
    {
        rand_pertu[i] = (rand() % 3 - 1) * perturbation;
        DACUpdateChannel(&dac, i, ctrl_v[i] + rand_pertu[i]);
    }
    // Final photodetector voltage is measured after perturbation.
    DelayMS(delay_time);
    ADCRead(&fnl_ADC_v);
    // Difference between final and initial photodector voltage is stored.
    adc_del_v = (int32_t)fnl_ADC_v - (int32_t)init_ADC_v;
    // New values of control voltages are calcualted and updated.
    for (int i = 0; i < 4; i++)
    {
        ctrl_v[i] -= adc_del_v * rand_pertu[i] / step_size * (cbrt(init_ADC_v) + 5);
    }
    for (int i = 0; i < 4; i++)
    {
        // uint32_t is_saturated = (((ctrl_v[i] - ctrl_v_min) / ctrl_v_range + (ctrl_v_max - ctrl_v[i]) / ctrl_v_range)) & 0b1;
        uint32_t is_saturated = (ctrl_v[i] >= ctrl_v_max - perturbation) || (ctrl_v[i] <= ctrl_v_min + perturbation);
        ctrl_v[i] = (ctrl_v[i] % ctrl_v_range) - (((ctrl_v[i] - ctrl_v_min) % ctrl_v_range) * is_saturated);
        // ctrl_v[i] = (ctrl_v[i] > ctrl_v_max) * ctrl_v_max + (ctrl_v[i] < ctrl_v_min) * ctrl_v_min;
        ctrl_v[i] -= (ctrl_v[i] - ctrl_v_mid) / step_size / 5 / sqrt(fnl_ADC_v);
    }
    // DAC is updated with new control voltages.
    for (int i = 0; i < 4; i++)
    {
        DACUpdateChannel(&dac, i, ctrl_v[i]);
    }
}

void algo_sin_RGD()
{
    int32_t rand_pertu[4], adc_del_v, range = ctrl_v_range;
    // Initial photodetector voltage is measured before perturbation.
    DelayMS(delay_time);
    ADCRead(&init_ADC_v);
    // Random perturbation is applied to all control voltages.
    for (int i = 0; i < 4; i++)
    {
        rand_pertu[i] = (rand() % 3 - 1) * perturbation;
        DACUpdateChannel(&dac, i, ctrl_v[i] + rand_pertu[i]);
    }
    // Final photodetector voltage is measured after perturbation.
    DelayMS(delay_time);
    ADCRead(&fnl_ADC_v);
    // Difference between final and initial photodector voltage is stored.
    adc_del_v = (int32_t)fnl_ADC_v - (int32_t)init_ADC_v;
    // New values of control voltages are calcualted and updated.
    for (int i = 0; i < 4; i++)
    {

        if (rand_pertu[i])
        {
            ctrl_v[i] -= (cbrt(init_ADC_v) + 10) * adc_del_v * rand_pertu[i] / step_size;
            double ctrl_temp = (range / 2) * sin(PI * (ctrl_v[i] - (range / 2)) / (range));
            int32_t int_ctrl_temp = (range / PI) * asin(ctrl_temp / (ctrl_v_range / 2)) + ctrl_v_mid;
            // UARTprintf("%4d;%4d; ", ctrl_v[i], int_ctrl_temp);
            ctrl_v[i] = int_ctrl_temp - (ctrl_v[i] - ctrl_v_mid) / perturbation;
        }
    }
    // DAC is updated with new control voltages.
    for (int i = 0; i < 4; i++)
    {
        DACUpdateChannel(&dac, i, ctrl_v[i]);
    }
}

void algo_triag_RGD()
{
    int32_t rand_pertu[4], adc_del_v, range = ctrl_v_range;
    // Initial photodetector voltage is measured before perturbation.
    {
        DelayMS(delay_time);
        ADCRead(&init_ADC_v);
    }
    // Random perturbation is applied to all control voltages.
    for (int i = 0; i < 4; i++)
    {
        rand_pertu[i] = (rand() % 3 - 1) * perturbation;
        DACUpdateChannel(&dac, i, ctrl_v[i] + rand_pertu[i]);
    }
    // Final photodetector voltage is measured after perturbation.
    {
        DelayMS(delay_time);
        ADCRead(&fnl_ADC_v);
    }
    // Difference between final and initial photodector voltage is stored.
    adc_del_v = (int32_t)fnl_ADC_v - (int32_t)init_ADC_v;
    // New values of control voltages are calcualted and updated.
    for (int i = 0; i < 4; i++)
    {
        // Control voltage is updated with the calcualted gradient.
        ctrl_v[i] -= (cbrt(init_ADC_v) + 10) * adc_del_v * rand_pertu[i] / step_size;
        // Control voltage is passed through a triangular function to handle saturaion.
        ctrl_v[i] -= perturbation;
        ctrl_v[i] = (ctrl_v[i] % ctrl_v_range) * ((ctrl_v[i] / ctrl_v_range + 1) % 2) + (ctrl_v_range - ctrl_v[i] % ctrl_v_range) * ((ctrl_v[i] / ctrl_v_range) % 2) + perturbation;
        // Control voltage is brought closer to the mid value with a spring function.
        ctrl_v[i] -= (ctrl_v[i] - ctrl_v_mid) / step_size;
    }
    // DAC is updated with new control voltages.
    for (int i = 0; i < 4; i++)
    {
        DACUpdateChannel(&dac, i, ctrl_v[i]);
    }
}

void setup()
{
    sys_clk = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    SysCtlDelay(sys_clk);

    //
    // Enable the peripherals used by this example.
    // The UART itself needs to be enabled, as well as the GPIO port
    // containing the pins that will be used.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure the GPIO pin muxing for the UART function.
    // This is only necessary if your part supports GPIO pin function muxing.
    // Study the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Since GPIO A0 and A1 are used for the UART function, they must be
    // configured for use as a peripheral function (instead of GPIO).
    // TODO: change this to match the port/pin you are using
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, sys_clk);

    //
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //
    // For this example ADC0 is used with AIN0 on port E7.
    // The actual port and pins used may be different on your part, consult
    // the data sheet for more information.  GPIO port E needs to be enabled
    // so these pins can be used.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Select the analog ADC function for these pins.
    // Consult the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);

    //
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 3.
    //
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // information on the ADC sequences and steps, reference the datasheet.
    //
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END);

    ADCHardwareOversampleConfigure(ADC0_BASE, 32);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 3);

    DACEnable(&dac);

    ctrl_v_mid = (ctrl_v_max + ctrl_v_min) / 2;
    ctrl_v_range = ctrl_v_max - ctrl_v_min - 2 * perturbation;
    for (int i = 0; i < 4; i++)
        ctrl_v[i] = ctrl_v_mid;
}

void loop()
{
    // static int32_t data = 0;
    // int32_t range = (ctrl_v_range - perturbation * 2 - 1);
    // double sin_val = (range / 2) * sin(PI * (data - (range / 2)) / (range)) + (range / 2);
    // int32_t sin_int = sin_val;
    // int32_t inv_sin_val = (range / PI) * asin((sin_val - range / 2) / (range / 2)) + ctrl_v_mid;
    // UARTprintf("%5d %5d %5d\n", (data++ & 0xFFF), sin_int, inv_sin_val);
    // DelayMS(200);

    algo_RGD();

    prev_adc_v = curr_ADC_v;
    DelayMS(delay_time);
    ADCRead(&curr_ADC_v);
    for (int i = 0; i < 4; i++)
    {
        UARTprintf("%4d; ", ctrl_v[i]);
    }
    UARTprintf("%4d;\n", curr_ADC_v);
}
