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
#include "bl_flash.h"

int8_t bl_flash_read_rrd_i(void *self, uint32_t start_addr, uint8_t *buff, size_t length){
    return (*(BL_FLASH_INTERFACE_RRD **)self)->read(self,start_addr,buff,length); 
}

int8_t bl_flash_write_rrd_i(void *self, uint32_t start_addr, const uint8_t *buff, size_t length){
    return (*(BL_FLASH_INTERFACE_RRD **)self)->write(self,start_addr,buff,length); 
}

void bl_flash_write_pre_rrd_i(void *self, uint32_t start_addr){
    (*(BL_FLASH_INTERFACE_RRD **)self)->write_pre(self, start_addr); 
}
