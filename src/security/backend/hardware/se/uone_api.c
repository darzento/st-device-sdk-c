/****************************************************************************
 * Copyright (C) Ubivelox, Inc - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
//#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "security/backend/hardware/se/uone_custom.h"

#include "esp_pm.h"


// Check Point - Driver Release Version
uint32_t uone_driver_version = 0x010000;

// I2C Initialize Flag
#define INIT_ENABLE		(uint8_t)0x5A
#define INIT_DISABLE	(uint8_t)0xA5
static uint8_t initFlag = INIT_DISABLE;


// I2C TransBuffer Options.
#define USE_MALLOC_POOL

#ifdef USE_MALLOC_POOL
#define USE_MALLOC_POOL_SIZE	(2100 / 2)
static unsigned char *mall_pool;
static unsigned char *mall_recv_pool;
#endif


SemaphoreHandle_t xSemaphore;

i2c_master_bus_handle_t tool_bus_handle;

esp_pm_lock_handle_t sleep_lock;


/*
 *	Debug Display API
*/
void disp_transbuffer (uint8_t *buffer, uint16_t length)
{

	uint16_t i;

	for ( i = 0; i < length; i++ ) {
		printf("%02X", buffer[i]);
	}

}	// End - void disp_transbuffer()

/*
 *	I2C APIs
 */
// S #################################################################################

#define CHECK_I2C_ERROR(ret)	\
	do {	\
		if ( (ret) != ESP_OK ) {	\
			goto error;		\
		}	\
	} while(0)


void uone_reset (void)
{

	UONE_DEBUG_ENTER;

	gpio_set_level(I2C_MASTER_RESET_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(50));

	gpio_set_level(I2C_MASTER_RESET_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(50));
	
}	// End - void uone_reset()

void uone_on(void)
{
	UONE_DEBUG_ENTER;
	gpio_set_level(UB_POWER_CTRL_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(5));
	gpio_set_level(UB_POWER_CTRL_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(10));
}

void uone_off(void)
{
	UONE_DEBUG_ENTER;
	gpio_set_level(UB_POWER_CTRL_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(10));
	gpio_set_level(UB_POWER_CTRL_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(5));
}

uint8_t uone_mutex_init(void)
{
	xSemaphore = xSemaphoreCreateMutex();
	if ( xSemaphore == NULL ) {
		return UONE_SEMAPHORE_CREATE_ERROR;
	}

	return UONE_SUCCESS;
}

void uone_mutex_deinit(void)
{
	vSemaphoreDelete(xSemaphore);
}


uint8_t uone_i2c_init (void)
{

	UONE_DEBUG_ENTER;

	// Check Initialize state
	if ( initFlag == INIT_ENABLE ) {
		return UONE_I2C_ALREADY_INITIALIZED;
	}

	// Create Binary Semaphore
	xSemaphore = xSemaphoreCreateBinary();
	if ( xSemaphore == NULL ) {
		//UONE_DEBUG_LOG("\n[SYSTEM] Failed to Create Semaphore.!\n", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_CREATE_ERROR;
	}

	// Releasing the Semaphtore Initially
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	esp_err_t err = 0;

	err = esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "I2C_read", &sleep_lock);
	if (err != ESP_OK) {
		printf("esp_pm_lock_create error : %d\n", err);
	} else {
		printf("esp_pm_lock_create success\n");
	}
	err = esp_pm_lock_acquire(sleep_lock);
	if (err != ESP_OK) {
		printf("esp_pm_lock_acquire error : %d\n", err);
	} else {
		printf("esp_pm_lock_acquire success\n");
	}


	// Reset PIN Set
	{
	    gpio_config_t reset_conf;
	    reset_conf.intr_type        = GPIO_INTR_DISABLE;    // Disable interrupt
	    reset_conf.mode             = GPIO_MODE_OUTPUT;     // Set as Output Mode
	    reset_conf.pin_bit_mask     = (1ULL << I2C_MASTER_RESET_PIN);   // Bit mask of the pin to set
	    reset_conf.pull_down_en     = 0;    // Disable pull-down mode
	    reset_conf.pull_up_en       = 0;    // Disable pull-up mode
	    gpio_config(&reset_conf);
	}

	// Power PIN Set
	{
		gpio_config_t ctrl_conf = {
			.intr_type 		= GPIO_INTR_DISABLE,
			.mode			= GPIO_MODE_OUTPUT,
			.pin_bit_mask	= (1ULL << UB_POWER_CTRL_PIN),
			.pull_down_en	= 0,
			.pull_up_en		= true,
		};
		gpio_config(&ctrl_conf);
	}

	uone_on();

	i2c_master_bus_config_t conf = {
		.clk_source = I2C_CLK_SRC_XTAL,
		.i2c_port = I2C_MASTER_NUM,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
		.trans_queue_depth = 0,
	};

	err = i2c_new_master_bus(&conf, &tool_bus_handle);
	if (err != ESP_OK) {
		return UONE_I2C_INIT_ERROR;
	}

	// I2C PIN Set
	// i2c_config_t conf = {
	//     .mode               = I2C_MODE_MASTER,
	//     .sda_io_num         = I2C_MASTER_SDA_IO,
	//     .sda_pullup_en      = GPIO_PULLUP_ENABLE,
	//     .scl_io_num         = I2C_MASTER_SCL_IO,
	//     .scl_pullup_en      = GPIO_PULLUP_ENABLE,
	//     .master.clk_speed   = I2C_MASTER_FREQ_HZ,
	//     // .clk_flags = 0,	/* Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
	// };

	// err = i2c_param_config(I2C_MASTER_NUM, &conf);
	// if (err != ESP_OK) {
	//     return UONE_I2C_INIT_ERROR;
	// }

	// err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	// if ( err != ESP_OK ) {
	// 	return UONE_I2C_INIT_ERROR;
	// }

	// Alloc to Internal I2C Trans Buffer
#ifdef USE_MALLOC_POOL
	mall_pool = malloc(USE_MALLOC_POOL_SIZE);
	if ( mall_pool == NULL ) {
		return UONE_ALLOC_MEMORY_ERROR;
	}

	mall_recv_pool = malloc(USE_MALLOC_POOL_SIZE);
	if ( mall_recv_pool == NULL ) {
		return UONE_ALLOC_MEMORY_ERROR;
	}
#endif

	uone_reset();

	initFlag = INIT_ENABLE;

	return UONE_SUCCESS;
	
}	// End - uint8_t uone_i2c_init()


uint8_t uone_i2c_deinit (void)
{

	UONE_DEBUG_ENTER;

	// Check Initialize state
	if ( initFlag == INIT_DISABLE ) {
		return UONE_I2C_ALREADY_DEINITIALIZED;
	}

	// Delete the Semaphore
	vSemaphoreDelete(xSemaphore);

	esp_err_t err = 0;

	err = i2c_del_master_bus(tool_bus_handle);
	if (err != ESP_OK) {
		return UONE_I2C_INIT_ERROR;
	}
	// err = i2c_driver_delete(I2C_MASTER_NUM);
	// if ( err != ESP_OK ) {
	// 	return UONE_I2C_DEINIT_ERROR;
	// }

#ifdef USE_MALLOC_POOL
	free(mall_pool);
	free(mall_recv_pool);
#endif
	uone_off();

	err = esp_pm_lock_release(sleep_lock);
	if (err != ESP_OK) {
		printf("esp_pm_lock_release error : %d\n", err);
		esp_pm_lock_release(sleep_lock);
	} else {
		printf("esp_pm_lock_release success\n");
	}
	err = esp_pm_lock_delete(sleep_lock);
	if (err != ESP_OK) {
		printf("esp_pm_lock_delete error : %d\n", err);
		esp_pm_lock_delete(sleep_lock);
	} else {
		printf("esp_pm_lock_delete success\n");
	}


	initFlag = INIT_DISABLE;

	return UONE_SUCCESS;

}	// End - uint8_t uone_i2c_deinit


uint8_t i2c_write (uint8_t *data, uint16_t length)
{
	i2c_device_config_t i2c_dev_conf = {
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
		.device_address = REAL_SE_ADDRESS,
		.flags.disable_ack_check = false,
	};

	i2c_master_dev_handle_t dev_handle;
	
	CHECK_I2C_ERROR(i2c_master_bus_add_device(tool_bus_handle, &i2c_dev_conf, &dev_handle));
	// printf("i2c_write_1\n");
	CHECK_I2C_ERROR(i2c_master_transmit(dev_handle, data, length, I2C_LIMIT_TRANS_TIME_MS));
	// printf("i2c_write_2\n");
	CHECK_I2C_ERROR(i2c_master_bus_rm_device(dev_handle));
	// i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	
	// CHECK_I2C_ERROR(i2c_master_start(cmd));
	// CHECK_I2C_ERROR(i2c_master_write_byte(cmd, (SE_ADDRESS | WRITE_BIT), ACK_CHECK_EN));
	// CHECK_I2C_ERROR(i2c_master_write(cmd, data, length, ACK_CHECK_EN));
	// CHECK_I2C_ERROR(i2c_master_stop(cmd));

	// CHECK_I2C_ERROR(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_LIMIT_TRANS_TIME_MS));

	// i2c_cmd_link_delete(cmd);

	return UONE_SUCCESS;

error :
	i2c_master_bus_rm_device(dev_handle);
	// i2c_cmd_link_delete(cmd);
	return UONE_I2C_WRITE_ERROR;

}	// End - static uint8_t i2c_write()


uint8_t i2c_read (uint8_t *data, uint16_t size)
{
	i2c_device_config_t i2c_dev_conf = {
		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
		.device_address = REAL_SE_ADDRESS,
		.flags.disable_ack_check = false,
	};

	i2c_master_dev_handle_t dev_handle;

	CHECK_I2C_ERROR(i2c_master_bus_add_device(tool_bus_handle, &i2c_dev_conf, &dev_handle));
	CHECK_I2C_ERROR(i2c_master_receive(dev_handle, data, size, I2C_LIMIT_TRANS_TIME_MS));
	CHECK_I2C_ERROR(i2c_master_bus_rm_device(dev_handle));
	// i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	// CHECK_I2C_ERROR(i2c_master_start(cmd));
	// CHECK_I2C_ERROR(i2c_master_write_byte(cmd, (SE_ADDRESS | READ_BIT), ACK_CHECK_EN));
	// CHECK_I2C_ERROR(i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK));
	// CHECK_I2C_ERROR(i2c_master_stop(cmd));

	// CHECK_I2C_ERROR(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_LIMIT_TRANS_TIME_MS));

	// i2c_cmd_link_delete(cmd);

	
	return UONE_SUCCESS;

error :
	// i2c_cmd_link_delete(cmd);
	i2c_master_bus_rm_device(dev_handle);
	return UONE_I2C_READ_ERROR;

}	// End - static uint8_t i2c_read()

// E - I2C APIs ######################################################################


/*
 * Common APIs
 */
// S #################################################################################

uint8_t uone_process (uint8_t *transdata, uint8_t *sec_readdata, uint16_t *length, uint8_t *response_code,
						uint16_t *crc, uint8_t i2c_repeat_counter, uint32_t delay_ms)
{
	uint8_t ret = 0;
	uint8_t readdata[UONE_RES_HEADER_AND_LENGTH_LEN];
	uint32_t internal_delay = I2C_SHORT_DELAY_MS;


	// Calc CRC16 & Set Value to TransData
	*crc = crc16_genibus(transdata, *length);
	memcpy((transdata + *length), crc, CRC_LEN);
	*length = *length + CRC_LEN;

	// YG - Debug Point - Write Data Print the transdata + CRC
	if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_PROCESS) ) {
		printf("\n  ## Transmit Data + CRC16 : ");
		disp_transbuffer(transdata, *length);
	}

	// I2C Write & Repeat
	
	ret = i2c_write(transdata, *length);
	while ( ((ret == UONE_I2C_WRITE_ERROR) && i2c_repeat_counter) ) {

		vTaskDelay(pdMS_TO_TICKS(internal_delay));

		ret = i2c_write(transdata, *length);

		internal_delay += 1;	// Increment delay time to 1 ms.
		i2c_repeat_counter--;

	}

	if ( (ret == UONE_I2C_WRITE_ERROR) && (i2c_repeat_counter == 0) ) {
		return ret;
	}

	// Wait the Processing Time of eSE
	vTaskDelay(pdMS_TO_TICKS(delay_ms));

	// Read Sequence 1. Read Response Header & Length
	i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
	internal_delay = I2C_SHORT_DELAY_MS;
	ret = i2c_read(readdata, UONE_RES_HEADER_AND_LENGTH_LEN);
	while ( ((ret == UONE_I2C_READ_ERROR) && i2c_repeat_counter) ) {

		vTaskDelay(pdMS_TO_TICKS(internal_delay));

		ret = i2c_read(readdata, UONE_RES_HEADER_AND_LENGTH_LEN);

		internal_delay += 1;	// Increment delay time to 1 ms.
		i2c_repeat_counter--;

	}
	if ( (ret == UONE_I2C_READ_ERROR) && (i2c_repeat_counter == 0) ) {
		return ret;
	}

	// YG - Debug Point - Read Data that response header & length of second read
	if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_PROCESS) ) {
		printf("\n  ## Receive-1 Data : ");
		disp_transbuffer(readdata, UONE_RES_HEADER_AND_LENGTH_LEN);
	}


	// Read Sequence 2. Read Response Data
	i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
	internal_delay = I2C_SHORT_DELAY_MS;

	*response_code = readdata[UONE_RES_OFFSET_HEADER];
	// Response Check is performed after completing the second read.

	*length = readdata[UONE_RES_OFFSET_LENGTH_HIGH] << 8 | readdata[UONE_RES_OFFSET_LENGTH_LOW];

	// Copy Response Header
	memcpy(sec_readdata, readdata, UONE_RES_HEADER_LEN);

	vTaskDelay(pdMS_TO_TICKS(internal_delay));

	ret = i2c_read((uint8_t *)(sec_readdata + UONE_RES_HEADER_LEN), *length);
	if ( ret != UONE_SUCCESS ) {
		return ret;
	}

	// Check Response Header
	if ( *response_code != UONE_SUCCESS ) {
		return *response_code;
	}

	// YG - Debug Point - Read Data that second read.
	if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_PROCESS) ) {
		printf("\n  ## Receive-2 Data : ");
		disp_transbuffer((sec_readdata + UONE_RES_HEADER_LEN), *length);
	}


	// Calc CRC16
	*length += UONE_RES_HEADER_LEN;
	*crc = crc16_genibus(sec_readdata, (*length - CRC_LEN));

	return ret;

}	//	End - uint8_t uone_process()


uint8_t uone_process_wakeup (void)
{

	uint8_t wakeup[3] = {0xD4, 0x00, 0x00};

	return i2c_write(wakeup, sizeof(wakeup));
	
}	// End - uint8_t uone_process_wakeup()


uint8_t uone_transfer (uint8_t *transdata, uint8_t *sec_readdata, uint16_t *length, uint8_t *response_code,
						uint16_t *crc, uint8_t i2c_repeat_counter, uint32_t delay_ms)
{

	uint16_t length_backup = *length;
	uint8_t ret, retry = 0;

	// YG - Debug - 임시 주석처리 (UONE-A 사용)
	//    -> 실제 UONE-M SE로 Test / Release 하는 경우, 주석 제거하여 적용해야 함
	// Wake-Up
	ret = uone_process_wakeup();

	// YG - Debug Point - Print the transdata
	if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_UONE) ) {
		printf("\n  ## Transmit Data : ");
		disp_transbuffer(transdata, *length);
	}

	vTaskDelay(pdMS_TO_TICKS(I2C_SHORT_DELAY_MS));

	ret = uone_process(transdata, sec_readdata, length, response_code, crc, i2c_repeat_counter, delay_ms);
	if ( ret != UONE_SUCCESS ) {

		if ( (ret & ERROR_I2C_TRANSPER) == ERROR_I2C_TRANSPER ) {

			for ( retry = 0; retry < ERROR_RETRY_COUNT; retry++ ) {

				*length = length_backup;

					// YG - Debug Point - Retry Check
					if ( IS_DEBUG_LEVEL(DEBUG_LEVEL_PROCESS) ) {
						printf("\n\n !!! Retry Occur. / Retrun = 0x%02X, Count = %d\n\n", ret, retry);
					}

					vTaskDelay(pdMS_TO_TICKS(I2C_LONG_DELAY_MS));
					delay_ms += (I2C_SHORT_DELAY_MS * 2);

					ret = uone_process(transdata, sec_readdata, length, response_code, crc, i2c_repeat_counter, delay_ms);
					if ( (ret & ERROR_I2C_TRANSPER) != ERROR_I2C_TRANSPER ) {
						break;
					}

			}
		}
	}

	return ret;

}	// End - uint8_t uone_transfer()


uint8_t uone_get_status (hal_data *outData, uint8_t data_type)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	if ( (outData == NULL) || (data_type > PARAM_GET_DATA_CSN) ) {
		ret = UONE_INCORRECT_PARAMETER;
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif

	// Command & Data Set
	length = 0;

	transdata[length++] = GET_DATA_STATUS | UONE_HEADER_LAST_MESSAGE;
	transdata[length++] = data_type;

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_API_GENERAL);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
		outData->data_len = (length - UONE_RES_HEADER_LEN - CRC_LEN);
		memcpy(outData->data, (sec_readdata + UONE_RES_HEADER_LEN), outData->data_len);
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}


	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_tp_get_status ()


uint8_t uone_get_product_manage_code (hal_data *pmCode)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	if ( pmCode == NULL ) {
		ret = UONE_INCORRECT_PARAMETER;
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif

	// Command & Data Set
	length = 0;

	transdata[length++] = 0x00;
	transdata[length++] = 0x01;

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_API_GENERAL);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
		pmCode->data_len = (length - UONE_RES_HEADER_LEN - CRC_LEN);
		memcpy(pmCode->data, (sec_readdata + UONE_RES_HEADER_LEN), pmCode->data_len);
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;

u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_get_product_manage_code()

// E - Common APIs ###################################################################


/*
 * Key Management APIs
 */
// S #################################################################################

uint8_t uone_generate_asymmetric_key (key_type mode, uint16_t index)\
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	//UONE_DEBUG_LOG("  . Key Index Info : %04X\n", index, DEBUG_LEVEL_PROCESS);
	//UONE_DEBUG_LOG("    - Sub Type : %02X\n", sub_type, DEBUG_LEVEL_PROCESS);
	//UONE_DEBUG_LOG("    - Slot : %02X\n", slot, DEBUG_LEVEL_PROCESS);

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	length = 0;

	// Command Parameter / Object ID
	switch (mode) {

		case KEY_ECC_EDWARD_X25519 :
			{

				// Command Code
				transdata[length++] = GENERATE_KEY_PAIR_X25519 | UONE_HEADER_LAST_MESSAGE;

				// Command Data - Parameter
				transdata[length++] = GENERATE_KEY_IN_SE | PARAM_ECC_KEY;	// Save In SE Storage | Parameter

				// Command Data - Object ID
				transdata[length++] = OBJECT_ID_ECC_X25519;
			}
			break;

		default :
			ret = UONE_ECC_CURVE_NOT_SUPPORTED;
			goto u_error;

	}	// End - switch (mode)

	// Command Data - Key Index (Slot Check & Key Index Set)
	if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
		transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
	} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	} else {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_GEN_ASYMMETRIC_KEY);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( !check_CRC16(&crc, sec_readdata, &length) ) {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_generate_asymmetric_key()


uint8_t uone_get_asymmetric_key (key_type mode, uint16_t index, hal_data *key)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	if ( sub_type != SUB_TYPE_PUBLIC_KEY ) {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	length = 0;

	// Command Data - Key Type
	switch (mode) {

		case KEY_ECC_EDWARD_ED25519 :
			{

				// Command Code
				transdata[length++] = GET_KEY | UONE_HEADER_LAST_MESSAGE;

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519) & 0xFF;

				// Command Data = Key Index
				if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
					transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
				} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
					transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
				} else {
					ret = UONE_INCORRECT_KEY_INDEX;
					goto u_error;
				}

			}
			break;

		case KEY_ECC_EDWARD_X25519 :
			{

				// Command Code
				transdata[length++] = GET_KEY | UONE_HEADER_LAST_MESSAGE;

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519) & 0xFF;

				// Command Data = Key Index
				if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
					transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
				} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
					transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
				} else {
					ret = UONE_INCORRECT_KEY_INDEX;
					goto u_error;
				}

			}
			break;

		default :
			ret = UONE_ECC_CURVE_NOT_SUPPORTED;
			goto u_error;
	}


	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_GET_ASYMMETRIC_KEY);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {

		// Set Buffer to get Key
		key->data_len = ( (sec_readdata[UONE_RES_OFFSET_LENGTH_HIGH] << 8) | sec_readdata[UONE_RES_OFFSET_LENGTH_LOW] );
		memcpy(key->data, &sec_readdata[UONE_RES_OFFSET_DATA], key->data_len);

	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_get_asymmetric_key()


uint8_t uone_remove_asymmetric_key(key_type mode, uint16_t index)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	length = 0;

	// Command Code
	transdata[length++] = SET_KEY | UONE_HEADER_LAST_MESSAGE;

	// Key Type
	switch (mode) {

		case KEY_ECC_EDWARD_ED25519 :
			{

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519) & 0xFF;
			}
			break;

		case KEY_ECC_EDWARD_X25519 :
			{

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519) & 0xFF;
			}
			break;

		default :
			ret = UONE_ECC_CURVE_NOT_SUPPORTED;
			goto u_error;
	}

	// Slot Check & Key Index Set
	if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
		transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
	} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	} else {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_SET_ASYMMETRIC_KEY);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( !check_CRC16(&crc, sec_readdata, &length) ) {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_remove_asymmetric_key


uint8_t uone_set_asymmetric_key (key_type mode, uint16_t index, hal_data *pubKey, hal_data *priKey)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;

	uint16_t keyLen = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	length = 0;

	// Command Data - Key Type
	switch (mode) {

		case KEY_ECC_EDWARD_ED25519 :
			{

				// Command Code
				transdata[length++] = SET_KEY | UONE_HEADER_LAST_MESSAGE;

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_ED25519) & 0xFF;
			}
			break;

		case KEY_ECC_EDWARD_X25519 :
			{

				// Command Code
				transdata[length++] = SET_KEY | UONE_HEADER_LAST_MESSAGE;

				// Command Data - Key Type
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 16) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519 >> 8) & 0xFF;
				transdata[length++] = (KEY_TYPE_ECC_X25519) & 0xFF;
			}
			break;

		default :
			ret = UONE_ECC_CURVE_NOT_SUPPORTED;
			goto u_error;
	}

	// Slot Check & Key Index Set
	if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
		transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
	} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	} else {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

	// Key Type Check, Key Length & Key Set
	switch (sub_type) {

		case SUB_TYPE_PRIVATE_KEY :
			{

				if ( (priKey->data == NULL) || (priKey->data_len == 0) ) {
					ret = UONE_INCORRECT_PARAMETER;
					goto u_error;
				}

				// Set Length of Private Key
				keyLen = (uint16_t)(priKey->data_len & 0xFFFF);
				transdata[length++] = HIGH(keyLen);
				transdata[length++] = LOW(keyLen);

				// Set Private Key
				memcpy((transdata + length), priKey->data, keyLen);
				length += keyLen;

				// Set Length of Public Key
				transdata[length++] = 0x00;
				transdata[length++] = 0x00;

			}
			break;

		case SUB_TYPE_PUBLIC_KEY :
			{

				if ( (pubKey->data == NULL) || (pubKey->data_len == 0) ) {
					ret = UONE_INCORRECT_PARAMETER;
					goto u_error;
				}

				// Set Length of Private Key
				transdata[length++] = 0x00;
				transdata[length++] = 0x00;

				// Set Length of Public Key
				keyLen = (uint16_t)(pubKey->data_len & 0xFFFF);
				transdata[length++] = HIGH(keyLen);
				transdata[length++] = LOW(keyLen);

				// Set Public Key
				memcpy((transdata + length), pubKey->data, keyLen);
				length += keyLen;

			}
			break;

		case SUB_TYPE_KEY_PAIR :
			{

				if ( (priKey->data == NULL) || (pubKey->data == NULL) 
					|| (priKey->data_len == 0) || (pubKey->data_len == 0) ) {
						ret = UONE_INCORRECT_PARAMETER;
						goto u_error;
				}

				// Set Length of Private Key
				keyLen = (uint16_t)(priKey->data_len & 0xFFFF);
				transdata[length++] = HIGH(keyLen);
				transdata[length++] = LOW(keyLen);

				// Set Private Key
				memcpy((transdata + length), priKey->data, keyLen);
				length += keyLen;

				// Set Length of Public Key
				keyLen = (uint16_t)(pubKey->data_len & 0xFFFF);
				transdata[length++] = HIGH(keyLen);
				transdata[length++] = LOW(keyLen);

				// Set Public Key
				memcpy((transdata + length), pubKey->data, keyLen);
				length += keyLen;

			}
			break;

		default :
			ret = UONE_INCORRECT_KEY_INDEX;
			goto u_error;

	}

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_SET_ASYMMETRIC_KEY);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( !check_CRC16(&crc, sec_readdata, &length) ) {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_set_asymmetric_key()

// E - Key Management APIs ###########################################################

/*
 * Random Number Generate APIs
 */
// S #################################################################################
uint8_t uone_generate_random (hal_data *outData)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}

	// Change length to word number
	if ( outData->data_len & 0x03 ) {
		outData->data_len = (outData->data_len + 4 - (outData->data_len & 0x03));
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif

	// Command & Data Set
	length = 0;

	transdata[length++] = GENERATE_RANDOM | UONE_HEADER_LAST_MESSAGE;

	// Command Data Length
	transdata[length++] = HIGH(outData->data_len);
	transdata[length++] = LOW(outData->data_len);

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_API_GENERATE_RANDOM);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
		outData->data_len = (length - UONE_RES_HEADER_LEN - CRC_LEN);
		memcpy(outData->data, (sec_readdata + UONE_RES_HEADER_LEN), outData->data_len);
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_generate_random ()

// E - Random Number Generate APIs ###################################################


/*
 * EdDSA - Edward Curve APIs
*/
// S #################################################################################

uint8_t uone_get_serial_number (hal_data *outData)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif

	
	length = 0;

	// Command Code
	transdata[length++] = READ_BINARY | UONE_HEADER_LAST_MESSAGE;

	// Read Bianry Offset
	transdata[length++] = (SE_UPDATE_READ_BINARY_OFFSET_MSB | (SLOT_SECURE_STORAGE_1F + SE_STORAGE_INDEX_MAPPING_SECURE));
	transdata[length++] = 0x00;

	// Set Data Length
	transdata[length++] = HIGH(MALL_SECURE_STORAGE_MAX_SIXE);
	transdata[length++] = LOW(MALL_SECURE_STORAGE_MAX_SIXE);

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_API_GENERAL);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
       
		memcpy(outData->data, (sec_readdata + UONE_RES_SERIAL_NUMBER_VALUE), sec_readdata[UONE_RES_SERIAL_NUMBER_LENGTH]);
		outData->data_len = sec_readdata[UONE_RES_SERIAL_NUMBER_LENGTH];
                
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_get_serial_number ()


uint8_t uone_eddsa_signature (struct sECC_SIGN *ecc_sign, hal_data *in_Hash_Data, uint16_t index)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	if ( sub_type != SUB_TYPE_PRIVATE_KEY ) {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	length = 0;

	// Command Code
	transdata[length++] = GENERATE_EDDSA_SIGN | UONE_HEADER_LAST_MESSAGE;

	// Curve Index
	transdata[length++] = CURVE_INDEX_ED25519;

	// Slot Check & Key Index Set
	if ( slot == SLOT_ODM_ED25519 ) {
		transdata[length++] = KEY_INDEX_ED25519;
	} else if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
		transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
	} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	}

	// Input Hash Length
	transdata[length++] = in_Hash_Data->data_len;

	// Input Hash Messages
	memcpy((transdata + length), in_Hash_Data->data, in_Hash_Data->data_len);
	length += in_Hash_Data->data_len;

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_EDDSA_SIGNATURE);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
       
		ecc_sign->r_length = (length - UONE_RES_HEADER_LEN - CRC_LEN) / 2;
		ecc_sign->s_length = (length - UONE_RES_HEADER_LEN - CRC_LEN) / 2;

		memcpy(ecc_sign->r, sec_readdata + UONE_RES_HEADER_LEN, ecc_sign->r_length);
		memcpy(ecc_sign->s, sec_readdata + UONE_RES_HEADER_LEN + ecc_sign->r_length, ecc_sign->s_length);
                
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;

u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_eddsa_signature


uint8_t uone_eddsa_verify (struct sECC_SIGN *ecc_sign, hal_data *in_data, uint16_t index)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;

	uint8_t *signature;
	uint8_t len_signature= 0;

	uint8_t put_r_padding_count = 0;
	uint8_t put_s_padding_count = 0;

	uint8_t offset = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	signature = sec_readdata;

	// Set Padding Count
	put_r_padding_count = SECP256_SIZE - ecc_sign->r_length;
	put_s_padding_count = SECP256_SIZE - ecc_sign->s_length;

	len_signature = ecc_sign->r_length + ecc_sign->s_length
					+ put_r_padding_count + put_s_padding_count;

	// Set Padded Signatrue 
	//  - Sign r
	offset = 0;
	if ( put_r_padding_count > 0 ) {
		memset(signature + offset, 0x00, put_r_padding_count);
		offset += put_r_padding_count;
	}

	memcpy((signature + offset), ecc_sign->r, ecc_sign->r_length);
	offset += ecc_sign->r_length;

	//  - Sign s
	if ( put_s_padding_count > 0 ) {
		memset((signature + offset), 0x00, put_s_padding_count);
		offset += put_s_padding_count;
	}

	memcpy((signature + offset), ecc_sign->s, ecc_sign->s_length);


	length = 0;

	// Command Code
	transdata[length++] = VERIFY_EDDSA_SIGN | UONE_HEADER_LAST_MESSAGE;

	// Command Data - Curve Index
	transdata[length++] = CURVE_INDEX_ED25519;

	// Command Data - Signature Length
	transdata[length++] = len_signature;

	// Command Data - Signature
	memcpy((transdata + length), signature, len_signature);
	length += len_signature;

	// Command Data - Input Message Length
	transdata[length++] = in_data->data_len;

	// Command Data - Input Messages
	memcpy((transdata + length), in_data->data, in_data->data_len);
	length += in_data->data_len;

	// Driver Key Index Parsing & Parameters Check
	ret = parse_key_index(index, &sub_type, &slot);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	if ( sub_type != SUB_TYPE_PUBLIC_KEY ) {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

	// Command Data - Length EncKeyBuf ( eSE Internal Key (Seted Key) Used )
	transdata[length++] = HIGH(SE_EXT_SETED_KEY_USED_FLAG);
	transdata[length++] = LOW(SE_EXT_SETED_KEY_USED_FLAG);

	// Slot Check & Key Index Set
	if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	} else {
		ret = UONE_INCORRECT_KEY_INDEX;
		goto u_error;
	}

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_EDDSA_VERIFY);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	// CRC Verify
	if ( !check_CRC16(&crc, sec_readdata, &length) ) {
      
		ret = UONE_CRC_INVALID;
		goto u_error;

	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}


uint8_t uone_ecdh_compute_shared_secret (struct sECC_KEY *ecc_pubicKey, uint16_t priKey_idx, hal_data *shared_secret)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

	uint8_t sub_type, slot = 0;

	uint8_t *pubkey;
	uint8_t pubkey_len = 0;

	uint8_t put_x_padding_count = 0;


#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}

    // Driver Key Index Parsing & Parameters Check
    ret = parse_key_index(priKey_idx, &sub_type, &slot);
    if ( ret != UONE_SUCCESS ) {
        goto u_error;
    }

    if ( sub_type != SUB_TYPE_PRIVATE_KEY ) {
        ret = UONE_INCORRECT_KEY_INDEX;
        goto u_error;
    }

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	pubkey = sec_readdata;

	// Padding & Set Public Key - x
	put_x_padding_count = SECP256_SIZE - ecc_pubicKey->x_length;
	if ( put_x_padding_count > 0 ) {
		memset((pubkey + pubkey_len), 0x00, put_x_padding_count);
		pubkey_len += put_x_padding_count;
	}

	memcpy((pubkey + pubkey_len), ecc_pubicKey->publicKey_x, ecc_pubicKey->x_length);
	pubkey_len += ecc_pubicKey->x_length;


	length = 0;

	// Command Code
	transdata[length++] = GENERATE_SECRET_X25519 | UONE_HEADER_LAST_MESSAGE;

	// Command Data - Parameter
	transdata[length++] = SE_GENERATE_SECRET_ECDHE;

	// Command Data - Curve Index
	transdata[length++] = CURVE_INDEX_X25519;

	// Command Data - Length of Public Key
	transdata[length++] = pubkey_len;

	// Command Data - Public Key
	memcpy((transdata + length), pubkey, pubkey_len);
	length += pubkey_len;

	// Command Data- Length of Key Buffer
	if ( slot == SLOT_ODM_ED25519 ) {
		transdata[length++] = KEY_INDEX_USE_ODM;
	} else if ( (slot >= SLOT_EP_KEY_0_NVM) && (slot <= SLOT_EP_KEY_9_NVM) ) {		// Ephemeral NVM
		transdata[length++] = KEY_INDEX_USE_STORAGE;
		transdata[length++] = slot - SE_KEY_INDEX_MAPPING_EP_NVM;	// 0x20 - 0x1D = (0x03) <= EP NVM <= 0x29 - 0x1D = (0x0C)
	} else if ( (slot >= SLOT_EP_KEY_0_RAM) && (slot <= SLOT_EP_KEY_2_RAM) ) {		// Ephemeral RAM
		transdata[length++] = KEY_INDEX_USE_STORAGE;
		transdata[length++] = slot + SE_KEY_INDEX_MAPPING_EP_RAM;	// 0x30 + 0xC0 = (0xF0) <= EP RAM <= 0x32 + 0xC0 = (0xF2)
	}

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_ECDH_COMPUTTE_SC);
	if ( ret != UONE_SUCCESS ) {
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
		shared_secret->data_len = (length - UONE_RES_HEADER_LEN - CRC_LEN);
		memcpy(shared_secret->data, (sec_readdata + UONE_RES_HEADER_LEN), shared_secret->data_len);
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;	

}	// End - uint8_t uone_ecdh_compute_shared_secret()


uint8_t uone_get_Factory_edKey (hal_data *pubKey)
{

	uint8_t ret = 0;
	uint16_t crc, length = 0;

	uint8_t *transdata;
	uint8_t *sec_readdata;

	uint8_t response_code = UONE_SUCCESS;

#ifdef POLLING_I2C_SHORT_DELAY
	uint8_t i2c_repeat_counter = I2C_SHORT_DELAY_COUNTER;
#endif

	UONE_DEBUG_ENTER;

	// YG - Check - Take Spemaphore
	if ( !xSemaphoreTake(xSemaphore, portMAX_DELAY) ) {
		//UONE_DEBUG_LOG("\n .! Semaphore is not acquired.!", NULL, DEBUG_LEVEL_UONE);
		return UONE_SEMAPHORE_TAKE_ERROR;
	}


	if ( pubKey == NULL ) {
		ret = UONE_INCORRECT_PARAMETER;
		goto u_error;
	}

#if defined (USE_MALLOC_POOL)
	transdata = mall_pool;
	sec_readdata = mall_recv_pool;
#else
#endif


	// Command & Data Set
	length = 0;

	// Command Code
	transdata[length++] = EXPORT_PUBLIC_KEY_ED | UONE_HEADER_LAST_MESSAGE;

	ret = uone_transfer(transdata, sec_readdata, &length, &response_code, &crc, i2c_repeat_counter, UONE_GEN_ASYMMETRIC_KEY);
	if ( ret != UONE_SUCCESS ){
		goto u_error;
	}

	// CRC Verify
	if ( check_CRC16(&crc, sec_readdata, &length) ) {
		pubKey->data_len = (length - UONE_RES_HEADER_LEN - CRC_LEN);
		memcpy(pubKey->data, (sec_readdata + UONE_RES_HEADER_LEN), pubKey->data_len);
	} else {
		ret = UONE_CRC_INVALID;
		goto u_error;
	}

	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return UONE_SUCCESS;


u_error :
	// YG - Check - Unlock Semaphore
	if ( !xSemaphoreGive(xSemaphore) ) {
		return UONE_SEMAPHORE_GIVE_ERROR;
	}

	return ret;

}	// End - uint8_t uone_get_Factory_edKey()

// E - EdDSA - Edward Curve APIs #####################################################
