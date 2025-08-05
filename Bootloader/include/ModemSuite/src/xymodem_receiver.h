/*
 * xymodem_receiver.h
 *
 *
 *  Created on: 2025_07_28
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef MODEM_XYMODEM_RECEIVER_RRD_H_
#define MODEM_XYMODEM_RECEIVER_RRD_H_

#include "stdint.h"
#include "stddef.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "modem_common.h"

/******************************************************************************/
/*----------------------------------INTERFACE---------------------------------*/
/******************************************************************************/
typedef struct __XYMODEM_PACK_INFO{
    size_t length; 
    uint8_t data_pack[MODEM_1k_SIZE];
}XYMODEM_PACK_INFO;

typedef void    (*xymodem_receiver_stop_transfer_fn_t)(void *self);
typedef void    (*xymodem_receiver_start_transfer_fn_t)(void *self);
typedef size_t  (*xymodem_receiver_strip_padding_fn_t)(void *self, const uint8_t *data, const size_t length);
typedef int     (*xymodem_receiver_unpack_fn_t)(void *self, const uint8_t *raw_data, const size_t raw_data_length, uint8_t *dest, const size_t dest_capacity, uint8_t *pack_index, size_t *pack_length);

typedef size_t  (*xymodem_receiver_get_time_ms_fn_t)(void);
typedef void    (*xymodem_receiver_send_data_fn_t)(uint8_t *data, size_t length);
typedef void    (*xymodem_receiver_save_data_fn_t)(uint8_t *data, size_t length);

typedef struct __XYMODEM_RECEIVER_INTERFACE_RRD{
    xymodem_receiver_unpack_fn_t unpack;
    xymodem_receiver_stop_transfer_fn_t stop;
    xymodem_receiver_start_transfer_fn_t start;
    xymodem_receiver_strip_padding_fn_t strip_padding;
}XYMODEM_RECEIVER_INTERFACE_RRD;

typedef struct __XYMODEM_RECEIVER_RRD{
    struct{
        uint8_t max_retry_count;
        MODEM_TYPE_RRD modem_type; 
        MODEM_LENGTH_RRD length_type;
        MODEM_VERIFY_RRD verify_type;
    }config;

    struct{
        uint8_t retry_count;
        uint8_t next_pack_number;

        XYMODEM_PACK_INFO prev;
    }status_params;

    XYMODEM_RECEIVER_INTERFACE_RRD *interface;

    xymodem_receiver_send_data_fn_t send_data; // callback
    xymodem_receiver_save_data_fn_t save_data; // callback
    xymodem_receiver_get_time_ms_fn_t get_time_ms; // callback
}XYMODEM_RECEIVER_RRD;

// save_data == NULL: 使用缓冲区的指针来获取数据包数据；否则使用回调函数获取数据
int xymodem_receiver_init(XYMODEM_RECEIVER_RRD *self, MODEM_TYPE_RRD modem_type,
                        MODEM_LENGTH_RRD length_type, MODEM_VERIFY_RRD verify_type,
                        xymodem_receiver_send_data_fn_t send_data,
                        xymodem_receiver_get_time_ms_fn_t get_time_ms,
                        xymodem_receiver_save_data_fn_t save_data);
/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#define OPEN_XYMODEM_TEST 0
#if OPEN_XYMODEM_TEST
void xymodem_debug(void);
#endif

#endif
