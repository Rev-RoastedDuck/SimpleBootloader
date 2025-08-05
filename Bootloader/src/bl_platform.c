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

#include "bl_platform.h"

/** \addtogroup comm
 ** \{ */
#define BL_COMM_BUFF_SIZE   (3)
uint8_t g_bl_comm_buff_list_length = 0;
bl_comm_rrd *g_bl_comm_buff_list[BL_COMM_BUFF_SIZE];

#define bootloader_platform_com_for(_entry_ptr)                             \
    for(uint8_t __index = 0;                                                \
        (__index) < (g_bl_comm_buff_list_length)                            \
            && ((_entry_ptr) = g_bl_comm_buff_list[(__index)], true);       \
        ++(__index))

int8_t bootloader_platform_register_comm(bl_comm_rrd *comm){
    DEBUG_ASSERT(comm != NULL);
    DEBUG_ASSERT(g_bl_comm_buff_list_length < BL_COMM_BUFF_SIZE);
    g_bl_comm_buff_list[g_bl_comm_buff_list_length++] = comm;
    return 0;
}
/** \} */

/** \addtogroup flash
 ** \{ */
bl_flash_rrd *g_bl_flash;
int8_t bootloader_platform_register_flash(bl_flash_rrd *flash){
    DEBUG_ASSERT(flash != NULL);
    g_bl_flash = flash;
    return 0;
}
/** \} */

/** \addtogroup debug_com_dev
 ** \{ */
bl_comm_rrd *g_bl_debug_comm_dev;
int8_t bootloader_platform_register_debug_com_dev(bl_comm_rrd *comm){
    DEBUG_ASSERT(comm != NULL);
    g_bl_debug_comm_dev = comm;
    return 0;
}
/** \} */
