/*
 * bl_comm.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef BOOTLOADER_BL_COMM_RRD_H_
#define BOOTLOADER_BL_COMM_RRD_H_

#include "debug.h"
#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"
#include "algorithm.h"

typedef int8_t  (*bl_comm_write_rrd_fn_t)        (void *self, uint8_t *data, size_t length);
typedef void    (*bl_comm_switch_buff_rrd_fn_t)  (void *self, DMA_CIRCULAR_QUEUE_RRD *buff_addr);

typedef struct __BL_COMM_INTERFACE_RRD{
    bl_comm_write_rrd_fn_t write;
    bl_comm_switch_buff_rrd_fn_t  switch_buff;
}BL_COMM_INTERFACE_RRD,bl_comm_interface_rrd;

typedef struct __BL_COMM_RRD{
    bl_comm_interface_rrd *interface;

    DMA_CIRCULAR_QUEUE_RRD dev_rx_buff;
    DMA_CIRCULAR_QUEUE_RRD *curr_data_buff;

    void *platform_data;
}BL_COMM_RRD,bl_comm_rrd;

#endif
