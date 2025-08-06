/*
 * queue_interface.h
 *
 *  Created on: 2024_12_12
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 *
 * :copyright: (c) 2023 by Rev-RoastedDuck.
 */

#ifndef ALGORITHM_DATA_STRUCTURE_QUEUE_INTERFACE_RRD_H_
#define ALGORITHM_DATA_STRUCTURE_QUEUE_INTERFACE_RRD_H_

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

typedef void    (*circular_queue_clear_fn_t)(void *self);
typedef bool    (*circular_queue_is_empty_fn_t)(void *self);
typedef size_t  (*circular_queue_get_lenth_fn_t)(void *self);
typedef void    (*circular_queue_enqueue_fn_t)(void *self,void *value);
typedef void    (*circular_queue_dequeue_fn_t)(void *self,void *value);
typedef size_t  (*circular_queue_batch_dequeue_fn_t)(void *self, void *buff, uint32_t max_size);
typedef void    (*circular_queue_batch_enqueue_fn_t)(void *self, void *buff, uint32_t size);

typedef size_t  (*circular_queue_get_next_index_fn_t)(void *self, size_t index);
typedef size_t  (*circular_queue_get_next_index_rev_fn_t)(void *self, size_t index);

typedef struct __CIRCULAR_QUEUE_INTERFACE_RRD{
    circular_queue_clear_fn_t clear;
    circular_queue_enqueue_fn_t enqueue;
    circular_queue_dequeue_fn_t dequeue;
    circular_queue_is_empty_fn_t is_empty;
    circular_queue_get_lenth_fn_t get_lenth;
    circular_queue_batch_dequeue_fn_t batch_dequeue;
    circular_queue_batch_enqueue_fn_t batch_enqueue;

    circular_queue_get_next_index_fn_t get_next_index;
    circular_queue_get_next_index_rev_fn_t get_next_index_rev;
}CIRCULAR_QUEUE_INTERFACE_RRD;

#endif
