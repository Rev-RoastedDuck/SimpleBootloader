/*
 * algorithm_common.c
 *	
 *  Created on: 2024_11_12
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2023 by Rev-RoastedDuck.
 */
#include "algorithm_common.h"

/**
 * @brief 分配一个按指定对齐方式对齐的内存块
 * @param size 要分配的内存块大小（字节）
 * @param alignment 对齐要求（必须是2的幂，通常是32、64、128等字节对齐）
 * @return 返回对齐后的内存块地址。如果分配失败，则返回NULL。
 * @note 内部会预先分配一个冗余的内存块再进行字节对齐，这回损失一部分的内存
 */
inline void* aligned_malloc(size_t size, size_t alignment)
{
    uintptr_t raw = (uintptr_t)malloc(size + alignment - 1 + sizeof(void*));
    if (raw == 0) return NULL;
    uintptr_t aligned = (raw + sizeof(void*) + alignment - 1) & ~(alignment - 1);
    ((void**)aligned)[-1] = (void*)raw;

    return (void*)aligned;
}

void aligned_free(void* aligned_ptr)
{
    if (aligned_ptr) {
        free(((void**)aligned_ptr)[-1]);
    }
}
