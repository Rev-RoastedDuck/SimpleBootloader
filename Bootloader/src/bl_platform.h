/*
 * bl_platform.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef BOOTLOADER_PLATFROM_RRD_H_
#define BOOTLOADER_PLATFROM_RRD_H_

#include "debug.h"
#include "stdint.h"

#include "bl_comm.h"
#include "bl_flash.h"

/** \addtogroup comm
 ** \{ */
extern uint8_t g_bl_comm_buff_list_length;
extern bl_comm_rrd *g_bl_comm_buff_list[];

#define bootloader_platform_com_for(_entry_ptr)                             \
    for(uint8_t __index = 0;                                                \
        (__index) < (g_bl_comm_buff_list_length)                            \
            && ((_entry_ptr) = g_bl_comm_buff_list[(__index)], true);       \
        ++(__index))

int8_t bootloader_platform_register_comm(bl_comm_rrd *comm);
/** \} */

/** \addtogroup flash
 ** \{ */
extern bl_flash_rrd *g_bl_flash;
int8_t bootloader_platform_register_flash(bl_flash_rrd *flash);
/** \} */

/** \addtogroup debug_com_dev
 ** \{ */
extern bl_comm_rrd *g_bl_debug_comm_dev;
int8_t bootloader_platform_register_debug_com_dev(bl_comm_rrd *comm);
/** \} */
#endif
