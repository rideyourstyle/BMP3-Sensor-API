/**\
 * Copyright (c) 2022 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bmp3.h"
#include "stm32l1xx_hal.h"
#include "common.h"
#include "cmsis_os.h"

#include "log.h"  // TODO: make generic

#define PIN_BMP_nCS_Pin          GPIO_PIN_4
#define PIN_BMP_nCS_GPIO_Port    GPIOC

extern SPI_HandleTypeDef hspi1;  // TODO: move to SPI-module

/*! BMP3 shuttle board ID */
#define BMP3_SHUTTLE_ID  0xD3

/* Variable to store the device address */
static uint8_t dev_addr;

///*!
// * I2C read function map to COINES platform
// */
//BMP3_INTF_RET_TYPE bmp3_i2c_read( uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr )
//{
//    uint8_t device_addr = *( uint8_t * ) intf_ptr;
//
//    ( void ) intf_ptr;
//
//    return coines_read_i2c( COINES_I2C_BUS_0, device_addr, reg_addr, reg_data, ( uint16_t ) len );
//}

///*!
// * I2C write function map to COINES platform
// */
//BMP3_INTF_RET_TYPE bmp3_i2c_write( uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr )
//{
//    uint8_t device_addr = *( uint8_t * ) intf_ptr;
//
//    ( void ) intf_ptr;
//
//    return coines_write_i2c( COINES_I2C_BUS_0, device_addr, reg_addr, ( uint8_t * ) reg_data, ( uint16_t ) len );
//}

void bmp3_SelectDevice( void )
{
    HAL_GPIO_WritePin(PIN_BMP_nCS_GPIO_Port, PIN_BMP_nCS_Pin, 0 );
}

void bmp_UnselectDevice( void )
{
    HAL_GPIO_WritePin(PIN_BMP_nCS_GPIO_Port, PIN_BMP_nCS_Pin, 1 );
}

/*!
 * SPI read function map to COINES platform
 */
BMP3_INTF_RET_TYPE bmp3_spi_read( uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr )
{
//    uint8_t device_addr = *( uint8_t * ) intf_ptr;  // CS
//    ( void ) intf_ptr;
//    return coines_read_spi( COINES_SPI_BUS_0, device_addr, reg_addr, reg_data, ( uint16_t ) len );

    HAL_StatusTypeDef retVal;
    bmp3_SelectDevice();

    retVal = HAL_SPI_Transmit( &hspi1, &reg_addr, 1, 1000 );
    if ( retVal )
    {
        bmp_UnselectDevice();
        return 1;
    }

    retVal = HAL_SPI_Receive( &hspi1, reg_data, ( uint16_t ) len, 1000 );
    if ( retVal )
    {
        bmp_UnselectDevice();
        return 1;
    }
    bmp_UnselectDevice();
    return ( BMP3_INTF_RET_TYPE ) retVal;
}

/*!
 * SPI write function map to COINES platform
 */
BMP3_INTF_RET_TYPE bmp3_spi_write( uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr )
{
//    uint8_t device_addr = *( uint8_t * ) intf_ptr;
//    ( void ) intf_ptr;
//    return coines_write_spi( COINES_SPI_BUS_0, device_addr, reg_addr, ( uint8_t * ) reg_data, ( uint16_t ) len );

    HAL_StatusTypeDef retVal;
    bmp3_SelectDevice();

    retVal = HAL_SPI_Transmit( &hspi1, &reg_addr, 1, 1000 );
    if ( retVal )
    {
        bmp_UnselectDevice();
        return 1;
    }

    retVal = HAL_SPI_Transmit( &hspi1, reg_data, ( uint16_t ) len, 1000 );
    if ( retVal )
    {
        bmp_UnselectDevice();
        return 1;
    }
    bmp_UnselectDevice();
    return ( BMP3_INTF_RET_TYPE ) retVal;
}

/*!
 * Delay function map to COINES platform
 */
void bmp3_delay_us( uint32_t period, void *intf_ptr )
{
    ( void ) intf_ptr;

    osDelay( period / 1000 );
}

void bmp3_check_rslt( const char api_name[], int8_t rslt )
{
    switch ( rslt )
    {
        case BMP3_OK:
            log_debug( "API [%s] OK [%d]", api_name, rslt );
            /* Do nothing */
            break;
        case BMP3_E_NULL_PTR:
            log_error( "API [%s] Error [%d] : Null pointer", api_name, rslt );
            break;
        case BMP3_E_COMM_FAIL:
            log_error( "API [%s] Error [%d] : Communication failure", api_name, rslt );
            break;
        case BMP3_E_INVALID_LEN:
            log_error( "API [%s] Error [%d] : Incorrect length parameter", api_name, rslt );
            break;
        case BMP3_E_DEV_NOT_FOUND:
            log_error( "API [%s] Error [%d] : Device not found", api_name, rslt );
            break;
        case BMP3_E_CONFIGURATION_ERR:
            log_error( "API [%s] Error [%d] : Configuration Error", api_name, rslt );
            break;
        case BMP3_W_SENSOR_NOT_ENABLED:
            log_error( "API [%s] Error [%d] : Warning when Sensor not enabled", api_name, rslt );
            break;
        case BMP3_W_INVALID_FIFO_REQ_FRAME_CNT:
            log_error( "API [%s] Error [%d] : Warning when Fifo watermark level is not in limit", api_name, rslt );
            break;
        default:
            log_error( "API [%s] Error [%d] : Unknown error code", api_name, rslt );
            break;
    }
}

BMP3_INTF_RET_TYPE bmp3_interface_init( struct bmp3_dev *bmp3, uint8_t intf )
{
    int8_t rslt = BMP3_OK;
//    struct coines_board_info board_info;
//
//    if ( bmp3 != NULL)
//    {
//        int16_t result = coines_open_comm_intf( COINES_COMM_INTF_USB, NULL);
//        if ( result < COINES_SUCCESS )
//        {
//            log_error( "\n Unable to connect with Application Board ! \n" " 1. Check if the board is connected and powered on. \n" " 2. Check if Application Board USB driver is installed. \n"
//                    " 3. Check if board is in use by another application. (Insufficient permissions to access USB) \n" );
//            exit( result );
//        }
//
//        result = coines_get_board_info( &board_info );
//
//#if defined(PC)
//        setbuf(stdout, NULL);
//#endif
//
//        if ( result == COINES_SUCCESS )
//        {
//            if (( board_info.shuttle_id != BMP3_SHUTTLE_ID ))
//            {
//                log_error( "! Warning invalid sensor shuttle \n ," "This application will not support this sensor \n" );
//            }
//        }
//
//        ( void ) coines_set_shuttleboard_vdd_vddio_config( 0, 0 );
//        coines_delay_msec( 1000 );
//
//    /* Bus configuration : I2C */
//    if ( intf == BMP3_I2C_INTF )
//    {
//        log_error( "I2C Interface\n" );
//            dev_addr = BMP3_ADDR_I2C_PRIM;
//        bmp3->read = bmp3_i2c_read;
//        bmp3->write = bmp3_i2c_write;
//        bmp3->intf = BMP3_I2C_INTF;
//
//        /* SDO pin is made low */
//            ( void ) coines_set_pin_config( COINES_SHUTTLE_PIN_SDO, COINES_PIN_DIRECTION_OUT, COINES_PIN_VALUE_LOW );
//            ( void ) coines_config_i2c_bus( COINES_I2C_BUS_0, COINES_I2C_STANDARD_MODE );
//    }
//        /* Bus configuration : SPI */
//    else
    if ( intf == BMP3_SPI_INTF )
    {
        log_error( "SPI Interface" );
//            dev_addr = COINES_SHUTTLE_PIN_7;
        bmp3->read = bmp3_spi_read;
        bmp3->write = bmp3_spi_write;
        bmp3->intf = BMP3_SPI_INTF;
//            ( void ) coines_config_spi_bus( COINES_SPI_BUS_0, COINES_SPI_SPEED_7_5_MHZ, COINES_SPI_MODE0 );
    }

//        coines_delay_msec( 1000 );

//        ( void ) coines_set_shuttleboard_vdd_vddio_config( 3300, 3300 );
//
//        coines_delay_msec( 1000 );
//
    bmp3->delay_us = bmp3_delay_us;
    bmp3->intf_ptr = &dev_addr;
//    }
//    else
//    {
//        rslt = BMP3_E_NULL_PTR;
//    }

    return BMP3_OK;
}

void bmp3_coines_deinit( void )
{
//    ( void ) fflush(stdout);
//
//    ( void ) coines_set_shuttleboard_vdd_vddio_config( 0, 0 );
//    coines_delay_msec( 1000 );
//
//    /* Coines interface reset */
//    coines_soft_reset();
//    coines_delay_msec( 1000 );
//    ( void ) coines_close_comm_intf( COINES_COMM_INTF_USB, NULL);
}
