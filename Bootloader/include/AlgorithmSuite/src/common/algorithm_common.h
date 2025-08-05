/*
 * algorithm_common.h
 *	
 *  Created on: 2024_11_12
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2023 by Rev-RoastedDuck.
 */
#ifndef ALGORITHM_COMMON_ALGORITHM_COMMON_H_
#define ALGORITHM_COMMON_ALGORITHM_COMMON_H_

/******************************************************************************/
/*----------------------------------INCLUDE-----------------------------------*/
/******************************************************************************/
#include "stddef.h"
#include "stdlib.h"
#include "stdint.h"
#include "algorithm_compile_config.h"

/******************************************************************************/
/*------------------------------------MARCO-----------------------------------*/
/******************************************************************************/
#define __PUBLIC___  
#define __PRIVATE__ 
#define ____IF_____ 

/******************************************************************************/
/*-------------------------------DECLARAYIONS---------------------------------*/
/******************************************************************************/
#define array_length(array) (sizeof(array) / sizeof(array[0]))
#define container_of_offset(ptr, offset) ((void *)((char *)(ptr) - offset))
#define member_ptr(struct_ptr, offset) ((void *)((char *)(struct_ptr) + (offset)))
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
#define clamp(value,min,max)  ((value) < (min)? (min) : ((value) > (max)? (max) : (value)))

void* aligned_malloc(size_t size, size_t alignment);

#endif
