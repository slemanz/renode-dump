#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_adc.h"
#include "driver_systick.h"
#include "driver_clock.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Thresholds (12-bit ADC: 0 = 0V, 4095 = 3.3V)                             */
/*                                                                             */
/*  For an agricultural ECU, these could represent:                            */
/*   - Hydraulic pressure sensor (0-250 bar mapped to 0-3.3V)                */
/*   - Coolant temperature sensor (NTC thermistor)                            */
/*   - Battery voltage (through voltage divider)                              */
/* -------------------------------------------------------------------------- */

#define ADC_THRESHOLD_WARNING   3000u   /* ~2.42V — high pressure / high temp */
#define ADC_THRESHOLD_CLEAR     2800u   /* hysteresis: must drop below this   */

/* -------------------------------------------------------------------------- */
/*  State                                                                      */
/* -------------------------------------------------------------------------- */

typedef enum {
    SENSOR_OK,
    SENSOR_WARNING
} sensor_state_t;

static sensor_state_t g_sensor_state = SENSOR_OK;

/* -------------------------------------------------------------------------- */
/*  UART2                                                                      */
/* -------------------------------------------------------------------------- */

static void uart2_init(void)
{
    GPIO_PinConfig_t tx = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_2,
        .GPIO_PinMode = GPIO_MODE_ALTFN, .GPIO_PinSpeed = GPIO_SPEED_FAST,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_PIN_PU,
        .GPIO_PinAltFunMode = PA2_ALTFN_UART2_TX
    };
    GPIO_Init(&tx);

    GPIO_PinConfig_t rx = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_3,
        .GPIO_PinMode = GPIO_MODE_ALTFN, .GPIO_PinSpeed = GPIO_SPEED_FAST,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_PIN_PU,
        .GPIO_PinAltFunMode = PA3_ALTFN_UART2_RX
    };
    GPIO_Init(&rx);

    UART_Config_t cfg = {
        .pUARTx = UART2, .UART_Baud = UART_STD_BAUD_115200,
        .UART_Mode = UART_MODE_TXRX, .UART_WordLength = UART_WORDLEN_8BITS,
        .UART_NoOfStopBits = UART_STOPBITS_1, .UART_ParityControl = UART_PARITY_DISABLE,
        .UART_HWFlowControl = UART_HW_FLOW_CTRL_NONE
    };
    UART_Init(&cfg);
    UART_PeripheralControl(UART2, ENABLE);
}

int __io_putchar(int ch)
{
    UART_WriteByte(UART2, (uint8_t)ch);
    return ch;
}

/* -------------------------------------------------------------------------- */
/*  GPIO: LEDs                                                                 */
/* -------------------------------------------------------------------------- */

static void gpio_init(void)
{
    /* PA5 — Status LED */
    GPIO_PinConfig_t led_status = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_5,
        .GPIO_PinMode = GPIO_MODE_OUT, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&led_status);

    /* PB3 — Warning LED */
    GPIO_PinConfig_t led_warn = {
        .pGPIOx = GPIOB, .GPIO_PinNumber = GPIO_PIN_NO_3,
        .GPIO_PinMode = GPIO_MODE_OUT, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&led_warn);

    /* PA0 — ADC input (analog mode) */
    GPIO_PinConfig_t adc_pin = {
        .pGPIOx = GPIOA, .GPIO_PinNumber = GPIO_PIN_NO_0,
        .GPIO_PinMode = GPIO_MODE_ANALOG, .GPIO_PinSpeed = GPIO_SPEED_LOW,
        .GPIO_PinOPType = GPIO_OP_TYPE_PP, .GPIO_PinPuPdControl = GPIO_NO_PUPD,
        .GPIO_PinAltFunMode = GPIO_PIN_NO_ALTFN
    };
    GPIO_Init(&adc_pin);
}

/* -------------------------------------------------------------------------- */
/*  ADC1                                                                       */
/* -------------------------------------------------------------------------- */

static ADC_Config_t g_adc_cfg;

static void adc1_init(void)
{
    g_adc_cfg.pADCx            = ADC1;
    g_adc_cfg.ADC_Resolution   = ADC_RESOLUTION_12BIT;
    g_adc_cfg.ADC_SampleTime   = ADC_SAMPLETIME_480;
    g_adc_cfg.ADC_DataAlignment = ADC_ALIGN_RIGHT;

    ADC_Init(&g_adc_cfg);
    ADC_PeripheralControl(ADC1, ENABLE);
}

/* -------------------------------------------------------------------------- */
/*  Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
    systick_init(TICK_HZ);
    uart2_init();
    gpio_init();
    adc1_init();

    printf("ADC ready\r\n");

    uint64_t last_sample = ticks_get();
    uint64_t last_blink  = ticks_get();

    while (1)
    {
        /* Heartbeat blink */
        if ((ticks_get() - last_blink) >= 500u)
        {
            GPIO_ToggleOutputPin(GPIOA, GPIO_PIN_NO_5);
            last_blink = ticks_get();
        }

        /* Sample ADC every 250ms */
        if ((ticks_get() - last_sample) >= 250u)
        {
            uint16_t raw = 0;
            ADC_Error_e err = ADC_ReadChannel(ADC1, ADC_CHANNEL_0, &raw);

            if (err == ADC_OK)
            {
                printf("[ADC] ch0=%u\r\n", (unsigned)raw);

                /* Threshold check with hysteresis */
                if (g_sensor_state == SENSOR_OK && raw >= ADC_THRESHOLD_WARNING)
                {
                    g_sensor_state = SENSOR_WARNING;
                    GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_3, 1);
                    printf("[WARN] overvoltage\r\n");
                }
                else if (g_sensor_state == SENSOR_WARNING && raw < ADC_THRESHOLD_CLEAR)
                {
                    g_sensor_state = SENSOR_OK;
                    GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_3, 0);
                    printf("[WARN] cleared\r\n");
                }
            }
            else
            {
                printf("[ADC] error=%u\r\n", (unsigned)err);
            }

            last_sample = ticks_get();
    }
}