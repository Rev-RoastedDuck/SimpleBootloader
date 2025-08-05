/*
 * bl_flash.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */
#ifndef BOOTLOADER_BL_FLASH_RRD_H_
#define BOOTLOADER_BL_FLASH_RRD_H_

#include "stddef.h"
#include "stdint.h"

typedef int8_t (*bl_flash_read_rrd_fn_t)        (void *self, uint32_t start_addr, uint8_t *buff, size_t length);
typedef int8_t (*bl_flash_write_rrd_fn_t)       (void *self, uint32_t start_addr, const uint8_t *buff, size_t length);
typedef void   (*bl_flash_write_pre_rrd_fn_t)   (void *self, uint32_t start_addr);

typedef struct __BL_FLASH_INTERFACE_RRD{
    bl_flash_read_rrd_fn_t read;
    bl_flash_write_rrd_fn_t write;
    bl_flash_write_pre_rrd_fn_t write_pre;
}BL_FLASH_INTERFACE_RRD, bl_flash_interface_rrd;

typedef struct __BL_FLASH_RRD{
    BL_FLASH_INTERFACE_RRD *interface;

    void *platform_data;
}BL_FLASH_RRD,bl_flash_rrd;

#endif
