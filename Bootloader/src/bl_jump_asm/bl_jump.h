/*
 * bl_jump_asm.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef BOOTLOADER_BL_JUMP_ASM_RRD_H_
#define BOOTLOADER_BL_JUMP_ASM_RRD_H_

#include "stdint.h"

#if defined(__CC_ARM)
    void __bootloader_jump_to_app_keil(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_keil
#elif defined(__GNUC__)
    void __bootloader_jump_to_app_gcc(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_gcc
#elif defined(__ICCARM__)
    void __bootloader_jump_to_app_iar(uint32_t address);
    #define __bootloader_jump_to_app __bootloader_jump_to_app_iar
#else
    #define __bootloader_jump_to_app 
#endif

#endif
