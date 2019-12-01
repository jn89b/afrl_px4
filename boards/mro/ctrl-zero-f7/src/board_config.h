/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file board_config.h
 *
 * mro_ctrl-zero-f7 internal definitions
 */

#pragma once

/****************************************************************************************************
 * Included Files
 ****************************************************************************************************/

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

#include <stm32_gpio.h>

/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/

/* GPIOs ***********************************************************************************/

/* LEDs are driven with push open drain to support Anode to 5V or 3.3V */

#define GPIO_nLED_RED	/* PB11 */	(GPIO_OUTPUT|GPIO_OPENDRAIN|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN11)
#define GPIO_nLED_GREEN	/* PB1 */	(GPIO_OUTPUT|GPIO_OPENDRAIN|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN1)
#define GPIO_nLED_BLUE	/* PB3 */	(GPIO_OUTPUT|GPIO_OPENDRAIN|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN3)

#define BOARD_HAS_CONTROL_STATUS_LEDS      1
#define BOARD_OVERLOAD_LED     LED_RED
#define BOARD_ARMED_STATE_LED  LED_BLUE

#define PX4_SPI_BUS_1	1
#define PX4_SPI_BUS_2	2
#define PX4_SPI_BUS_5	5

/*
 *  Define the ability to shut off off the sensor signals
 *  by changing the signals to inputs
 */
#define _PIN_OFF(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT|GPIO_PULLDOWN|GPIO_SPEED_2MHz))

/*  Define the Chip Selects, Data Ready and Control signals per SPI bus */

/* SPI 1 CS */
#define GPIO_SPI1_CS1_ICM20602	/* PC2 */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTC|GPIO_PIN2)
#define GPIO_SPI1_CS2_ICM20948	/* PE15 */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTE|GPIO_PIN15)

/*  Define the SPI1 Data Ready interrupts */
#define GPIO_SPI1_DRDY1_ICM20602    /* PD15  */  (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTD|GPIO_PIN15)
#define GPIO_SPI1_DRDY2_ICM20948    /* PE12  */  (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTE|GPIO_PIN12)

/* SPI 2 CS */
#define GPIO_SPI2_CS1_FRAM	/* PD10 */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN10)
#define GPIO_SPI2_CS2_BARO	/* PD7 */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN7)

/* SPI 5 CS */
#define GPIO_SPI5_CS1_BMI088_ACCEL	/* PF6  */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTF|GPIO_PIN6)
#define GPIO_SPI5_CS2_BMI088_GYRO	/* PF10  */	(GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTF|GPIO_PIN10)

/* SPI 5 BMI088 Data Ready interrupts */
#define GPIO_DRDY_BMI088_INT1_ACCEL /* PF1 */  (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTF|GPIO_PIN1)
#define GPIO_DRDY_BMI088_INT2_ACCEL /* PF2 */  (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTF|GPIO_PIN2)
#define GPIO_DRDY_BMI088_INT3_GYRO  /* PF3 */  (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTF|GPIO_PIN3)
#define GPIO_DRDY_BMI088_INT4_GYRO  /* PF4 */  (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTF|GPIO_PIN4)

/* v BEGIN Legacy SPI defines TODO: fix this with enumeration */
#define PX4_SPI_BUS_RAMTRON  PX4_SPI_BUS_2
/* ^ END Legacy SPI defines TODO: fix this with enumeration */

#define PX4_SPIDEV_ICM_20602        PX4_MK_SPI_SEL(PX4_SPI_BUS_1,0)
#define PX4_SPIDEV_ICM_20948        PX4_MK_SPI_SEL(PX4_SPI_BUS_1,1)
#define PX4_SPI_BUS_1_CS_GPIO       {GPIO_SPI1_CS1_ICM20602, GPIO_SPI1_CS2_ICM20948}

#define PX4_SPIDEV_MEMORY           PX4_MK_SPI_SEL(PX4_SPI_BUS_2,0)
#define PX4_SPIDEV_BARO             PX4_MK_SPI_SEL(PX4_SPI_BUS_2,1)
#define PX4_SPI_BUS_2_CS_GPIO       {GPIO_SPI2_CS1_FRAM, GPIO_SPI2_CS2_BARO}

#define PX4_SPIDEV_BMI088_ACC       PX4_MK_SPI_SEL(PX4_SPI_BUS_5,0)
#define PX4_SPIDEV_BMI088_GYR       PX4_MK_SPI_SEL(PX4_SPI_BUS_5,1)
#define PX4_SPI_BUS_5_CS_GPIO       {GPIO_SPI5_CS1_BMI088_ACCEL, GPIO_SPI5_CS2_BMI088_GYRO}


/* I2C busses */

#define PX4_I2C_BUS_EXPANSION       1
#define PX4_I2C_BUS_LED             PX4_I2C_BUS_EXPANSION

#define BOARD_NUMBER_I2C_BUSES      1
#define BOARD_I2C_BUS_CLOCK_INIT    {100000}

/*
 * ADC channels
 *
 * These are the channel numbers of the ADCs of the microcontroller that
 * can be used by the Px4 Firmware in the adc driver
 */

/* ADC defines to be used in sensors.cpp to read from a particular channel */
#define ADC1_CH(n)                  (n)
#define ADC1_GPIO(n)                GPIO_ADC1_IN##n

/* Define GPIO pins used as ADC N.B. Channel numbers must match below */
#define PX4_ADC_GPIO  \
	/* PA2 */  ADC1_GPIO(2),  \
	/* PA3 */  ADC1_GPIO(3),  \
	/* PA4 */  ADC1_GPIO(4),  \
	/* PC1 */  ADC1_GPIO(11)

/* Define Channel numbers must match above GPIO pin IN(n)*/
#define ADC_BATTERY_VOLTAGE_CHANNEL        /* PA2 */  ADC1_CH(2)
#define ADC_BATTERY_CURRENT_CHANNEL        /* PA3 */  ADC1_CH(3)
#define ADC_SCALED_V5_CHANNEL              /* PA4 */  ADC1_CH(4)
#define ADC_RSSI_IN_CHANNEL                /* PC1 */  ADC1_CH(11)

#define ADC_CHANNELS \
	((1 << ADC_BATTERY_VOLTAGE_CHANNEL)       | \
	 (1 << ADC_BATTERY_CURRENT_CHANNEL)       | \
	 (1 << ADC_SCALED_V5_CHANNEL)             | \
	 (1 << ADC_RSSI_IN_CHANNEL))

/* Define Battery 1 Voltage Divider and A per V */
#define BOARD_BATTERY_V_DIV         (18.1f)     /* measured with the provided PM board */
#define BOARD_BATTERY_A_PER_V       (36.367515152f)

/* HW has to large of R termination on ADC todo:change when HW value is chosen */
#define BOARD_ADC_OPEN_CIRCUIT_V     (5.6f)

/* CAN Silence: Silent mode control \ ESC Mux select */
#define GPIO_CAN1_SILENT_S0  /* PF5  */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTF|GPIO_PIN5)

/* PWM
 *
 * 8  PWM outputs are configured.
 *
 * Pins:
 *
 * FMU_CH1 : PE14 : TIM1_CH4
 * FMU_CH2 : PE13 : TIM1_CH3
 * FMU_CH3 : PE11 : TIM1_CH2
 * FMU_CH4 : PE9  : TIM1_CH1
 * FMU_CH5 : PD13 : TIM4_CH2
 * FMU_CH6 : PD14 : TIM4_CH3
 * FMU_CH7 : PI5  : TIM8_CH1
 * FMU_CH8 : PI6  : TIM8_CH2
 *
 */
#define GPIO_TIM8_CH2OUT	/* PI6   T8C1   FMU8 */ GPIO_TIM8_CH2OUT_2
#define GPIO_TIM8_CH1OUT	/* PI5   T8C1   FMU7 */ GPIO_TIM8_CH1OUT_2
#define GPIO_TIM4_CH3OUT	/* PD14  T4C3   FMU6 */ GPIO_TIM4_CH3OUT_2
#define GPIO_TIM4_CH2OUT	/* PD13  T4C2   FMU5 */ GPIO_TIM4_CH2OUT_2
#define GPIO_TIM1_CH1OUT	/* PE9   T1C1   FMU4 */ GPIO_TIM1_CH1OUT_2
#define GPIO_TIM1_CH2OUT	/* PE11  T1C2   FMU3 */ GPIO_TIM1_CH2OUT_2
#define GPIO_TIM1_CH3OUT	/* PE13  T1C3   FMU2 */ GPIO_TIM1_CH3OUT_2
#define GPIO_TIM1_CH4OUT	/* PE14  T1C4   FMU1 */ GPIO_TIM1_CH4OUT_2

#define DIRECT_PWM_OUTPUT_CHANNELS  8

#define GPIO_TIM8_CH2IN		/* PI6   T8C2   FMU8 */ GPIO_TIM8_CH2IN_2
#define GPIO_TIM8_CH1IN		/* PI5   T8C1   FMU7 */ GPIO_TIM8_CH1IN_2
#define GPIO_TIM4_CH3IN		/* PD14  T4C3   FMU6 */ GPIO_TIM4_CH3IN_2
#define GPIO_TIM4_CH2IN		/* PD13  T4C2   FMU5 */ GPIO_TIM4_CH2IN_2
#define GPIO_TIM1_CH1IN		/* PE9   T1C1   FMU4 */ GPIO_TIM1_CH1IN_2
#define GPIO_TIM1_CH2IN		/* PE11  T1C2   FMU3 */ GPIO_TIM1_CH2IN_2
#define GPIO_TIM1_CH3IN		/* PE13  T1C3   FMU2 */ GPIO_TIM1_CH3IN_2
#define GPIO_TIM1_CH4IN		/* PE14  T1C4   FMU1 */ GPIO_TIM1_CH4IN_2

#define DIRECT_INPUT_TIMER_CHANNELS  8

/* User GPIOs: GPIO0-5 are the PWM servo outputs. */
#define _MK_GPIO_INPUT(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT|GPIO_PULLUP))

#define GPIO_GPIO7_INPUT        /* PI6   T8C2   FMU8 */ _MK_GPIO_INPUT(GPIO_TIM8_CH2IN)
#define GPIO_GPIO6_INPUT        /* PI5   T8C1   FMU7 */ _MK_GPIO_INPUT(GPIO_TIM8_CH1IN)
#define GPIO_GPIO5_INPUT        /* PD14  T4C3   FMU6 */ _MK_GPIO_INPUT(GPIO_TIM4_CH3IN)
#define GPIO_GPIO4_INPUT        /* PD13  T4C2   FMU5 */ _MK_GPIO_INPUT(GPIO_TIM4_CH2IN)
#define GPIO_GPIO3_INPUT        /* PE9   T1C1   FMU4 */ _MK_GPIO_INPUT(GPIO_TIM1_CH1IN)
#define GPIO_GPIO2_INPUT        /* PE11  T1C2   FMU3 */ _MK_GPIO_INPUT(GPIO_TIM1_CH2IN)
#define GPIO_GPIO1_INPUT        /* PE13  T1C3   FMU2 */ _MK_GPIO_INPUT(GPIO_TIM1_CH3IN)
#define GPIO_GPIO0_INPUT        /* PE14  T1C4   FMU1 */ _MK_GPIO_INPUT(GPIO_TIM1_CH4IN)

#define _MK_GPIO_OUTPUT(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR))

#define GPIO_GPIO7_OUTPUT        /* PI6   T8C2   FMU8 */ _MK_GPIO_OUTPUT(GPIO_TIM8_CH2OUT)
#define GPIO_GPIO6_OUTPUT        /* PI5   T8C1   FMU7 */ _MK_GPIO_OUTPUT(GPIO_TIM8_CH1OUT)
#define GPIO_GPIO5_OUTPUT        /* PD14  T4C3   FMU6 */ _MK_GPIO_OUTPUT(GPIO_TIM4_CH3OUT)
#define GPIO_GPIO4_OUTPUT        /* PD13  T4C2   FMU5 */ _MK_GPIO_OUTPUT(GPIO_TIM4_CH2OUT)
#define GPIO_GPIO3_OUTPUT        /* PE9   T1C1   FMU4 */ _MK_GPIO_OUTPUT(GPIO_TIM1_CH1OUT)
#define GPIO_GPIO2_OUTPUT        /* PE11  T1C2   FMU3 */ _MK_GPIO_OUTPUT(GPIO_TIM1_CH2OUT)
#define GPIO_GPIO1_OUTPUT        /* PA10  T1C3   FMU2 */ _MK_GPIO_OUTPUT(GPIO_TIM1_CH3OUT)
#define GPIO_GPIO0_OUTPUT        /* PE14  T1C4   FMU1 */ _MK_GPIO_OUTPUT(GPIO_TIM1_CH4OUT)


/* Power supply control and monitoring GPIOs */
#define GPIO_nPOWER_IN_A                /* PB5 */ (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTB|GPIO_PIN5)

#define GPIO_nVDD_BRICK1_VALID          GPIO_nPOWER_IN_A /* Brick 1 Is Chosen */
#define BOARD_NUMBER_BRICKS             1

#define GPIO_VDD_3V3_SENSORS_EN         /* PE3  */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN3)
#define GPIO_VDD_3V3_SPEKTRUM_POWER_EN  /* PE4  */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN4)


/* Define True logic Power Control in arch agnostic form */
#define VDD_3V3_SENSORS_EN(on_true)        px4_arch_gpiowrite(GPIO_VDD_3V3_SENSORS_EN, (on_true))
#define VDD_3V3_SPEKTRUM_POWER_EN(on_true) px4_arch_gpiowrite(GPIO_VDD_3V3_SPEKTRUM_POWER_EN, (on_true))
#define READ_VDD_3V3_SPEKTRUM_POWER_EN()   px4_arch_gpioread(GPIO_VDD_3V3_SPEKTRUM_POWER_EN)

/* Tone alarm output */
#define TONE_ALARM_TIMER        2  /* timer 2 */
#define TONE_ALARM_CHANNEL      1  /* PA15 TIM2_CH1 */

#define GPIO_BUZZER_1           /* PA15 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN15)

#define GPIO_TONE_ALARM_IDLE    GPIO_BUZZER_1
#define GPIO_TONE_ALARM         GPIO_TIM2_CH1OUT_2

/* USB OTG FS
 *
 * PA9  OTG_FS_VBUS VBUS sensing
 */
#define GPIO_OTGFS_VBUS         /* PA9 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_SPEED_100MHz|GPIO_PORTA|GPIO_PIN9)

/* High-resolution timer */
#define HRT_TIMER               3  /* use timer3 for the HRT */
#define HRT_TIMER_CHANNEL       3  /* use capture/compare channel 3 */

#define HRT_PPM_CHANNEL         /* T3C2 */  2  /* use capture/compare channel 1 */
#define GPIO_PPM_IN             /* PC7 T3C2 */ GPIO_TIM3_CH2IN_3

/* RC Serial port */
#define RC_SERIAL_PORT                     "/dev/ttyS3"
#define RC_SERIAL_SINGLEWIRE

#define GPIO_RSSI_IN                       /* PC1  */ (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTC|GPIO_PIN1)

/* Safety Switch: Enable the FMU to control it if there is no px4io fixme:This should be BOARD_SAFETY_LED(__ontrue) */
#define GPIO_SAFETY_SWITCH_IN              /* PC4 */ (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTC|GPIO_PIN4)
/* Enable the FMU to use the switch it if there is no px4io fixme:This should be BOARD_SAFTY_BUTTON() */
#define GPIO_BTN_SAFETY GPIO_SAFETY_SWITCH_IN /* Enable the FMU to control it if there is no px4io */

/* Power switch controls ******************************************************/
#define SPEKTRUM_POWER(_on_true)           VDD_3V3_SPEKTRUM_POWER_EN(_on_true)

/*
 * FMUv5 has a separate RC_IN
 *
 * GPIO PPM_IN on PC7 T3CH2
 * SPEKTRUM_RX (it's TX or RX in Bind) on UART6 PG9 (NOT FMUv5 test HW ONLY)
 *   In version is possible in the UART
 * and can drive  GPIO PPM_IN as an output
 */
#define GPIO_PPM_IN_AS_OUT             (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN0)
#define SPEKTRUM_RX_AS_GPIO_OUTPUT()   px4_arch_configgpio(GPIO_PPM_IN_AS_OUT)
#define SPEKTRUM_RX_AS_UART()          /* Can be left as uart */
#define SPEKTRUM_OUT(_one_true)        px4_arch_gpiowrite(GPIO_PPM_IN_AS_OUT, (_one_true))

#define SDIO_SLOTNO                    0  /* Only one slot */
#define SDIO_MINOR                     0

/* By Providing BOARD_ADC_USB_CONNECTED (using the px4_arch abstraction)
 * this board support the ADC system_power interface, and therefore
 * provides the true logic GPIO BOARD_ADC_xxxx macros.
 */
#define BOARD_ADC_USB_CONNECTED	       (px4_arch_gpioread(GPIO_OTGFS_VBUS))
#define BOARD_ADC_USB_VALID            BOARD_ADC_USB_CONNECTED
#define BOARD_ADC_SERVO_VALID          (1)	/* never powers off the Servo rail */
#define BOARD_ADC_BRICK_VALID          (!px4_arch_gpioread(GPIO_nVDD_BRICK1_VALID))

#define BOARD_HAS_PWM  DIRECT_PWM_OUTPUT_CHANNELS

/* This board provides a DMA pool and APIs */
#define BOARD_DMA_ALLOC_POOL_SIZE 5120

/* This board provides the board_on_reset interface */
#define BOARD_HAS_ON_RESET 1

/* The list of GPIO that will be initialized */
#define PX4_GPIO_PWM_INIT_LIST { \
		GPIO_GPIO7_INPUT, \
		GPIO_GPIO6_INPUT, \
		GPIO_GPIO5_INPUT, \
		GPIO_GPIO4_INPUT, \
		GPIO_GPIO3_INPUT, \
		GPIO_GPIO2_INPUT, \
		GPIO_GPIO1_INPUT, \
		GPIO_GPIO0_INPUT, \
	}

#define PX4_GPIO_INIT_LIST { \
		PX4_ADC_GPIO,                     \
		GPIO_CAN1_TX,                     \
		GPIO_CAN1_RX,                     \
		GPIO_CAN1_SILENT_S0,              \
		GPIO_nPOWER_IN_A,                 \
		GPIO_VDD_3V3_SENSORS_EN,          \
		GPIO_VDD_3V3_SPEKTRUM_POWER_EN,   \
		GPIO_TONE_ALARM_IDLE,             \
		GPIO_SAFETY_SWITCH_IN,            \
	}

#define BOARD_ENABLE_CONSOLE_BUFFER

#define BOARD_NUM_IO_TIMERS 3

__BEGIN_DECLS

#ifndef __ASSEMBLY__

/****************************************************************************************************
 * Public Functions
 ****************************************************************************************************/

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDIO-based MMC/SD card support
 *
 ****************************************************************************/

int stm32_sdio_initialize(void);

/****************************************************************************************************
 * Name: stm32_spiinitialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the PX4FMU board.
 *
 ****************************************************************************************************/

extern void stm32_spiinitialize(void);

void board_spi_reset(int ms);

extern void stm32_usbinitialize(void);

extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
