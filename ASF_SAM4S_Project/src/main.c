/**
 * \file
 *
 * \brief Starter Kit Demo.
 *
 * Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage Starter Kit Demo
 *
 * \section Purpose
 *
 * The Starter Kit Demo will help new users get familiar with Atmel's
 * SAM family of microcontrollers. This demo features the IO1 and OLED1
 * extension boards for the SAM4 Xplained Pro.
 *
 * \section Requirements
 *
 * This package can be used with SAM Xplained Pro evaluation kits.
 *
 * \section Description
 *
 * The demonstration program can operate in 3 different modes; temperature
 * information, light sensor information and SD card status.
 * The user can switch between the various mode by pressing Button1.
 * When running in mode 3 (SD card content), the user can browse the SD
 * content using Button2 (previous) and Button3 (next). Filenames are directly
 * printed on the OLED screen.
 *
 * IO1 extension must be connected on EXT2.
 * OLED1 extension must be connected on EXT3.
 *
 */

#include <asf.h>
#include <string.h>
#include "uart.h"
#include "usart.h"
#include "conf_uart_serial.h"
#include "blowfish.h"


/* These settings will force to set and refresh the temperature mode. */
volatile uint32_t app_mode = 2;
volatile uint32_t app_mode_switch = 1;

volatile uint32_t sd_update = 0;
volatile uint32_t sd_fs_found = 0;
volatile uint32_t sd_listing_pos = 0;
volatile uint32_t sd_num_files = 0;

FATFS fs;

/**
 * \brief Process Buttons Events.
 *
 * \param uc_button The button number.
 */
static void ProcessButtonEvt(uint8_t uc_button)
{
	/* Switch between temperature, light and SD mode. */
	if (uc_button == 1)
	{
		app_mode_switch = 1;
	}
	/* Page UP button. */
	else if ((uc_button == 2) &&
				(app_mode == 2) &&
				(sd_fs_found == 1) &&
				(sd_update == 0))
	{
		if (sd_listing_pos > 0)
		{
			sd_listing_pos -= 1;
			sd_update = 1;
		}
	}
	/* Page DOWN button. */
	else if ((uc_button == 3) &&
				(app_mode == 2) &&
				(sd_fs_found == 1) &&
				(sd_update == 0))
	{
		/* Lock DOWN button when showing the last file. */
		if (sd_listing_pos < sd_num_files)
		{
			sd_listing_pos += 1;
			sd_update = 1;
		}
	}
}

/**
 * \brief Handler for Button 1 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button1_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_1_ID == id) && (PIN_PUSHBUTTON_1_MASK == mask))
		ProcessButtonEvt(1);
}

/**
 * \brief Handler for Button 2 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button2_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_2_ID == id) && (PIN_PUSHBUTTON_2_MASK == mask))
		ProcessButtonEvt(2);
}

/**
 * \brief Handler for Button 3 rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void Button3_Handler(uint32_t id, uint32_t mask)
{
	if ((PIN_PUSHBUTTON_3_ID == id) && (PIN_PUSHBUTTON_3_MASK == mask))
		ProcessButtonEvt(3);
}

/**
 * \brief Handler for SD card detect rising edge interrupt.
 * \param id The button ID.
 * \param mask The button mask.
 */
static void SD_Detect_Handler(uint32_t id, uint32_t mask)
{
	if ((SD_MMC_0_CD_ID == id) && (SD_MMC_0_CD_MASK == mask))
	{
		sd_listing_pos = 0;
		sd_num_files = 0;
		sd_fs_found = 0;
		sd_update = 1;
	}
}

/* IRQ priority for PIO (The lower the value, the greater the priority) */
#define IRQ_PRIOR_PIO    0

/**
 * \brief Configure the Pushbuttons.
 *
 * Configure the PIO as inputs and generate corresponding interrupt when
 * pressed or released.
 */
static void configure_buttons(void)
{
	/* Configure Pushbutton 1. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_1_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID,
			PIN_PUSHBUTTON_1_MASK, PIN_PUSHBUTTON_1_ATTR, Button1_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_1_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_1_PIO, (IRQn_Type) PIN_PUSHBUTTON_1_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);

	/* Configure Pushbutton 2. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_2_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_ID,
			PIN_PUSHBUTTON_2_MASK, PIN_PUSHBUTTON_2_ATTR, Button2_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_2_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_2_PIO, (IRQn_Type) PIN_PUSHBUTTON_2_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK);

	/* Configure Pushbutton 3. */
	pmc_enable_periph_clk(PIN_PUSHBUTTON_3_ID);
	pio_set_debounce_filter(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_MASK, 10);
	pio_handler_set(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_ID,
			PIN_PUSHBUTTON_3_MASK, PIN_PUSHBUTTON_3_ATTR, Button3_Handler);
	NVIC_EnableIRQ((IRQn_Type) PIN_PUSHBUTTON_3_ID);
	pio_handler_set_priority(PIN_PUSHBUTTON_3_PIO, (IRQn_Type) PIN_PUSHBUTTON_3_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(PIN_PUSHBUTTON_3_PIO, PIN_PUSHBUTTON_3_MASK);

	/* Configure SD card detection. */
	pmc_enable_periph_clk(SD_MMC_0_CD_ID);
	pio_set_debounce_filter(SD_MMC_0_CD_PIO, SD_MMC_0_CD_MASK, 10);
	pio_handler_set(SD_MMC_0_CD_PIO, SD_MMC_0_CD_ID, SD_MMC_0_CD_MASK,
			SD_MMC_0_CD_ATTR, SD_Detect_Handler);
	NVIC_EnableIRQ((IRQn_Type) SD_MMC_0_CD_ID);
	pio_handler_set_priority(SD_MMC_0_CD_PIO, (IRQn_Type) SD_MMC_0_CD_ID, IRQ_PRIOR_PIO);
	pio_enable_interrupt(SD_MMC_0_CD_PIO, SD_MMC_0_CD_MASK);
}

/**
 * \brief Configure the ADC for the light sensor.
 */
static void configure_adc(void)
{
	/* Configure ADC pin for light sensor. */
	gpio_configure_pin(LIGHT_SENSOR_GPIO, LIGHT_SENSOR_FLAGS);

	/* Enable ADC clock. */
	pmc_enable_periph_clk(ID_ADC);

	/* Configure ADC. */
	adc_init(ADC, sysclk_get_cpu_hz(), 1000000, ADC_MR_STARTUP_SUT0);
	adc_enable_channel(ADC, ADC_CHANNEL_4);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);
}

/**
 * \brief Get the number of files at the root of the SD card.
 * Result is stored in global sd_num_files.
 */
static void get_num_files_on_sd(void)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char *pc_fn;
	const char *path = "0:";
#if _USE_LFN
	char c_lfn[_MAX_LFN + 1];
	fno.lfname = c_lfn;
	fno.lfsize = sizeof(c_lfn);
#endif

	sd_num_files = 0;

	/* Open the directory */
	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
			{
				break;
			}

#if _USE_LFN
			pc_fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			pc_fn = fno.fname;
#endif
			if (*pc_fn == '.')
			{
				continue;
			}

			sd_num_files += 1;
		}
	}
}

/**
 * \brief Show SD card status on the OLED screen.
 */
static void display_sd_info(void)
{
	FRESULT res;
	uint8_t card_check;
	uint8_t sd_card_type;
	uint32_t sd_card_size;
	char size[64];

	// Is SD card present?
	if (gpio_pin_is_low(SD_MMC_0_CD_GPIO) == false)
	{
		ssd1306_write_text("Please insert SD card...");
	}
	else
	{
		ssd1306_write_text("SD card information:");

		sd_mmc_init();
		card_check = sd_mmc_check(0);
		while (card_check != SD_MMC_OK)
		{
			card_check = sd_mmc_check(0);
			delay_ms(1);
		}

		if (card_check == SD_MMC_OK)
		{
			sd_card_type = sd_mmc_get_type(0);
			sd_card_size = sd_mmc_get_capacity(0);

			ssd1306_set_page_address(1);
			ssd1306_set_column_address(0);

			// Card type
			switch(sd_card_type)
			{
				case CARD_TYPE_SD:
				ssd1306_write_text("- Type: Normal SD card");
				break;
				case CARD_TYPE_SDIO:
				ssd1306_write_text("- Type: SDIO card");
				break;
				case CARD_TYPE_HC:
				ssd1306_write_text("- Type: High Capacity card");
				break;
				case CARD_TYPE_SD_COMBO:
				ssd1306_write_text("- Type: SDIO/Memory card");
				break;
				default:
				ssd1306_write_text("- Type: unknown");
			}

			ssd1306_set_page_address(2);
			ssd1306_set_column_address(0);

			sprintf(size, "- Total size: %lu KB", sd_card_size);
			ssd1306_write_text(size);

			ssd1306_set_page_address(3);
			ssd1306_set_column_address(0);

			// Try to mount file system.
			memset(&fs, 0, sizeof(FATFS));
			res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
			if (FR_INVALID_DRIVE == res)
			{
				ssd1306_write_text("   <No FAT FS found on SD>");
				sd_fs_found = 0;
			}
			else
			{
				get_num_files_on_sd();
				if (sd_num_files == 0)
				{
					ssd1306_write_text("         <no content>");
					sd_fs_found = 1;
				}
				else
				{
					ssd1306_write_text("  <Press B2-3 to browse SD>");
					sd_fs_found = 1;
				}
			}
		}
	}
}

/**
 * \brief Show SD card content on the OLED screen.
 * \note Does not browse sub folders.
 */
static void display_sd_files(void)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	uint32_t line;
	uint32_t pos;
	char *pc_fn;
	const char *path = "0:";
#if _USE_LFN
	char c_lfn[_MAX_LFN + 1];
	fno.lfname = c_lfn;
	fno.lfsize = sizeof(c_lfn);
#endif

	line = 0;
	pos = 1;

	/* Open the directory */
	res = f_opendir(&dir, path);
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
			{
				break;
			}

#if _USE_LFN
			pc_fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			pc_fn = fno.fname;
#endif
			if (*pc_fn == '.')
			{
				continue;
			}

			if ((pos >= sd_listing_pos) && (line < 4))
			{
				ssd1306_set_page_address(line++);
				ssd1306_set_column_address(0);
				ssd1306_write_text("/");
				ssd1306_write_text(pc_fn);
			}

			pos += 1;
		}
	}
}

/**
 * \brief Draw graph on the OLED screen using the provided point array.
 * \param col X coordinate.
 * \param page Y coordinate (please refer to OLED datasheet for page description).
 * \param width Graph width.
 * \param height Graph height.
 * \param tab Data to draw. Must contain width elements.
 */
static void ssd1306_draw_graph(uint8_t col, uint8_t page, uint8_t width, uint8_t height, uint8_t *tab)
{
	uint8_t page_start;
	uint8_t i, j, k, s;
	uint8_t scale;

	for (i = col; i < width; ++i) {
		for (page_start = page; page_start <= height; ++page_start) {
			ssd1306_write_command(SSD1306_CMD_SET_PAGE_START_ADDRESS(page_start));
			ssd1306_set_column_address(i);
			j = tab[i];
			scale = 8 * (height - page_start + 1);
			if (j > scale)
				j = 8;
			else
				j -= (scale - 8);

			for (k = 0, s = j; j > 0; --j)
				k = (k << 1) + 1;
			for (s = 8 - s; s > 0; --s)
				k <<= 1;
			ssd1306_write_data(k);
		}
	}
}

/**
 * \brief Clear one character at the cursor current position on the OLED
 * screen.
 */
static void ssd1306_clear_char(void)
{
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
	ssd1306_write_data(0x00);
}

#define BUFFER_SIZE 128
void printStr(const char* textStr);
void printStrF(unsigned long L, unsigned long R);

int main(void)
{
	uint8_t i;
	uint8_t temperature[BUFFER_SIZE];
	uint8_t light[BUFFER_SIZE];
	char value_disp[5];
	uint32_t adc_value;
	uint32_t light_value;
	double temp;
	
	unsigned long L  = 0x12345678, R   = 0x16fde256;
	unsigned long bL = 0x12345678, bR  = 0x16fde256;

	BLOWFISH_CTX ctx;

	char mess[120];
	short x;
	
	
	// Initialize USART in RS232 mode.
	usart_serial_options_t USART_SERIAL_OPTIONS =
	{
		.baudrate     = CONF_UART_BAUDRATE,
		.charlength   = 8,
		.paritytype   = CONF_UART_PARITY,
		.stopbits     = 1,
	};

	
	
	// Initialize clocks.
	sysclk_init();

	// Initialize GPIO states.
	board_init();

	// Configure ADC for light sensor.
	configure_adc();

	// Initialize at30tse.
	at30tse_init();

	// Configure IO1 buttons.
	configure_buttons();

	// Initialize SPI and SSD1306 controller.
	ssd1306_init();
	ssd1306_clear();
		
	// stdio should go to usart
	stdio_serial_init(CONF_UART, &USART_SERIAL_OPTIONS);	
	
	  Blowfish_Init (&ctx, (unsigned char*)"TESTKEY", 7);

	  printStr("Original Data\n");
	  printStrF(L,R);
	  //
	  Blowfish_Encrypt(&ctx, &bL, &bR);
	  printStrF(bL,bR);
	   
	  if (bL == 0x47B8B9ECL && bR == 0xCE55FF90L)
	  printStr("Test encryption OK.\n");
	  else
	  printStr("Test encryption failed.\n\0");

	  //
	  Blowfish_Decrypt(&ctx, &bL, &bR);
	  printStrF(bL,bR);
	  if (L == bL && R == bR)
	  printStr("Test decryption OK.\n");
	  else
	  printStr("Test decryption failed.\n\0");

	while(1){};	 
		
	 
}


void printStr(const char* textStr)
{
	short x;
	size_t messStr = strlen(textStr);
	for(x=0;x<messStr;x++)
		ptr_put(CONF_UART,textStr[x]);
}

void printStrF(unsigned long L, unsigned long R)
{
	short x,len;
	char buf[128];
	len = sprintf(buf,"%08lX %08lX\n", L, R);
	buf[len] = 0;
	for(x=0;x<len;x++)
	ptr_put(CONF_UART,buf[x]);
}
