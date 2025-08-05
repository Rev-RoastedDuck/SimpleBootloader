/*
 * dma_circular_queue.c
 *	
 *  Created on: 2024_10_24
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2023 by Rev-RoastedDuck.
 */

#include "dma_circular_queue.h"

#ifdef ALGORITHM_COMPILE_DMA_CIRCULAR_QUEUE

/******************************************************************************/
/*----------------------------------FUNCTION----------------------------------*/
/******************************************************************************/
static void circular_queue_clear(DMA_CIRCULAR_QUEUE_RRD *self){
    self->front = self->rear;
}

static size_t circular_queue_get_length(DMA_CIRCULAR_QUEUE_RRD *self){
    return self->is_full? self->capacity : ((self->capacity + self->rear - self->front) % self->capacity);
}

static bool circular_queue_is_empty(DMA_CIRCULAR_QUEUE_RRD *self){
    return self->rear == self->front && !self->is_full;
}

static void circular_queue_enqueue(DMA_CIRCULAR_QUEUE_RRD *self,void *value){
    void *data_pos = (void *)((char *)self->data + self->unit_size * self->rear);
    memcpy(data_pos, value, self->unit_size);

    self->rear = (self->rear + 1) % self->capacity;

    // why?
    if(self->is_full){
        self->front = self->rear;
    } 
    if(self->rear == self->front){
        self->is_full = true;
    }
}

static void circular_queue_dequeue(DMA_CIRCULAR_QUEUE_RRD *self,void *value){
    if(circular_queue_is_empty(self)){
        return;
    }

    void *data_pos = (char *)self->data + (self->front * self->unit_size);
    memcpy(value, data_pos, self->unit_size);

    self->is_full = false;
    self->front = (self->front + 1) % self->capacity;
}

static int32_t circular_queue_get_next_index(DMA_CIRCULAR_QUEUE_RRD *self, size_t index){
    if (index >= circular_queue_get_length(self)) {
        return -1;
    }

    return (self->front + index) % self->capacity;
}

static int32_t circular_queue_get_next_index_rev(DMA_CIRCULAR_QUEUE_RRD *self, size_t index){
    if(index >= circular_queue_get_length(self)){
        return -1;
    }

    return (self->rear - index - 1 + self->capacity) % self->capacity;
}

static size_t circular_queue_batch_dequeue(DMA_CIRCULAR_QUEUE_RRD *self, void *buff, uint32_t max_size) {
    if(circular_queue_is_empty(self)) {
        return 0;
    }

    size_t available_count = circular_queue_get_length(self);
    max_size = max_size > available_count? available_count : max_size;

    uint32_t fir_part_max_size = self->capacity - self->front - 1;

    uint32_t fir_part_size = max_size > fir_part_max_size? fir_part_max_size : max_size;
    uint32_t fir_part_start_index = self->front % self->capacity;

    uint32_t sec_part_size = max_size - fir_part_size;
    uint32_t sec_part_start_index = self->rear > self->front? self->rear : 0;

    memcpy(buff, (char* )self->data + fir_part_start_index * self->unit_size, self->unit_size * fir_part_size);
    if(sec_part_size > 0){
        memcpy((char* )buff + self->unit_size * fir_part_size, (char* )self->data + sec_part_start_index * self->unit_size, self->unit_size * sec_part_size);
    }

    self->is_full = false;
    self->front = (self->front + max_size) % self->capacity;
    return max_size;
}

static void circular_queue_batch_enqueue(DMA_CIRCULAR_QUEUE_RRD *self, void *buff, uint32_t size) {
    char* src_start_index = size <= self->capacity? (char* )buff : (char* )buff + (size - self->capacity) * self->unit_size;
    bool need_change_font_ptr = size >= self->capacity - circular_queue_get_length(self);
    size = size > self->capacity? self->capacity : size;

    uint32_t fir_part_max_size = self->capacity - self->rear;

    uint32_t fir_part_size = size > fir_part_max_size? fir_part_max_size : size;
    uint32_t fir_part_start_index = self->rear;

    uint32_t sec_part_size = size - fir_part_size;
    uint32_t sec_part_start_index = 0;

    memcpy((char *)self->data + fir_part_start_index * self->unit_size, src_start_index, fir_part_size);
    if(sec_part_size > 0){
        memcpy((char *)self->data + sec_part_start_index * self->unit_size, src_start_index + fir_part_size * self->unit_size,sec_part_size);
    }

    self->rear = (self->rear + size) % self->capacity;
    if(self->rear == self->front){
        self->is_full = true;
    }
}

/******************************************************************************/
/*--------------------------Construction/Destruction--------------------------*/
/******************************************************************************/
static void circular_queue_del(void **self){
    if (self != NULL && *self != NULL) {
        free(((DMA_CIRCULAR_QUEUE_RRD*)*self)->data);
        free(*self);
        *self = NULL;
    }
}

static CIRCULAR_QUEUE_INTERFACE_RRD g_DAM_CIRCULAR_QUEUE_INTERFACE = {
    .clear = (circular_queue_clear_fn_t)circular_queue_clear,
    .dequeue = (circular_queue_dequeue_fn_t)circular_queue_dequeue,
    .enqueue = (circular_queue_enqueue_fn_t)circular_queue_enqueue,
    .is_empty = (circular_queue_is_empty_fn_t)circular_queue_is_empty,
    .get_lenth = (circular_queue_get_lenth_fn_t)circular_queue_get_length,
    .get_next_index = (circular_queue_get_next_index_fn_t)circular_queue_get_next_index,
    .get_next_index_rev = (circular_queue_get_next_index_rev_fn_t)circular_queue_get_next_index_rev,
    .batch_dequeue = (circular_queue_batch_dequeue_fn_t)circular_queue_batch_dequeue,
    .batch_enqueue = (circular_queue_batch_enqueue_fn_t)circular_queue_batch_enqueue,
};

void dma_circular_queue_init(DMA_CIRCULAR_QUEUE_RRD * self,uint32_t buff_lenth,uint8_t unit_size,size_t buff_alignment)
{
    self->data = buff_alignment? aligned_malloc((buff_lenth) * unit_size, buff_alignment)
                                : (uint8_t*)malloc((buff_lenth) * unit_size);
    if(NULL == self->data){
        return;
    }

    self->rear = 0;
    self->front = 0;
    self->is_full = false;
    self->dma_wrap_count = 0;
    self->unit_size = unit_size;
    self->capacity = buff_lenth;

    self->del = circular_queue_del;
    self->interface = &g_DAM_CIRCULAR_QUEUE_INTERFACE;
}

/******************************************************************************/
/*-----------------------------------DEBUG------------------------------------*/
/******************************************************************************/
#if OPEN_DMA_CIRCULAR_QUEUE_TEST
#include <stdio.h>
void dma_circular_queue_test_start(void){
    {
        printf("-----uint8-batch_dequeue----\r\n");
        DMA_CIRCULAR_QUEUE_RRD circular_queu_uint8_t;
        DMA_CIRCULAR_QUEUE_RRD *circular_queu_uint8 = &circular_queu_uint8_t;
        dma_circular_queue_init(circular_queu_uint8, 8, sizeof(uint8_t), 0);
        for(uint8_t i = 0;i < 12;++i){
            circular_queu_uint8->interface->enqueue(circular_queu_uint8,&i);
        }
        printf("front: %d rear: %d is_full: %d \r\n",circular_queu_uint8->front,circular_queu_uint8->rear,circular_queu_uint8->is_full);
        uint8_t buff[4];
        size_t size =  circular_queu_uint8->interface->batch_dequeue(circular_queu_uint8,buff,4);
        printf("size:%ld \r\n",size);
        for(int8_t i = 0;i < sizeof(buff);++i){
            printf("%d ",buff[i]);
        }
        printf("\r\n");
        printf("front: %d rear: %d is_full: %d \r\n",circular_queu_uint8->front,circular_queu_uint8->rear,circular_queu_uint8->is_full);
    }

    {
        printf("-----uint8-batch_enqueue----\r\n");
        DMA_CIRCULAR_QUEUE_RRD circular_queu_batch_enqueue_t;
        DMA_CIRCULAR_QUEUE_RRD* circular_queu_batch_enqueue = &circular_queu_batch_enqueue_t;
        dma_circular_queue_init(circular_queu_batch_enqueue, 5, sizeof(uint8_t), 0);
        uint8_t batch_enqueue_data[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

        uint8_t* for_data_u8 = NULL;
        circular_queu_batch_enqueue->interface->batch_enqueue(circular_queu_batch_enqueue,batch_enqueue_data,13);
        printf("front: %d rear: %d is_full: %d \r\n",circular_queu_batch_enqueue->front,circular_queu_batch_enqueue->rear,circular_queu_batch_enqueue->is_full);
        dma_circular_queue_for_each(circular_queu_batch_enqueue,for_data_u8){
            printf("%d  ",*for_data_u8);
        }
        printf("\r\n");

        printf("front: %d rear: %d is_full: %d \r\n",circular_queu_batch_enqueue->front,circular_queu_batch_enqueue->rear,circular_queu_batch_enqueue->is_full);
    }

}
#endif

#endif /* ALGORITHM_COMPILE_DMA_CIRCULAR_QUEUE */
