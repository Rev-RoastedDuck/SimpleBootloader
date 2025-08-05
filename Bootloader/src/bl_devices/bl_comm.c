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
#include "bl_comm.h"

inline void bl_comm_write_rrd_i(void *self, uint8_t *data, size_t length){
    (*(BL_COMM_INTERFACE_RRD **)self)->write(self,data,length);
}

inline void bl_comm_switch_buff_rrd_i(void *self, DMA_CIRCULAR_QUEUE_RRD *buff_addr){
    (*(BL_COMM_INTERFACE_RRD **)self)->switch_buff(self,buff_addr);
}

