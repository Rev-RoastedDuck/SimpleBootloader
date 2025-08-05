/*
 * bl_platform_config.h
 *	
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */
#ifndef BOOTLOADER_PLATFORM_BL_PALTFORM_CONFIG_RRD_H_
#define BOOTLOADER_PLATFORM_BL_PALTFORM_CONFIG_RRD_H_

#include "stdint.h"

/** \addtogroup callback
 ** \{ */
typedef void (*bl_platform_device_init_fn_t)(void);
#define bl_platform_device_init     ((bl_platform_device_init_fn_t)(0))

typedef void (*bl_platform_device_deinit_fn_t)(void);
#define bl_platform_device_deinit   ((bl_platform_device_deinit_fn_t)(0))

typedef void (*bl_platform_system_reset_fn_t)(void);
#define bl_platform_system_reset    ((bl_platform_system_reset_fn_t)(0))

typedef void (*bl_platform_enable_irq_fn_t)(void);
#define bl_platform_enable_irq    ((bl_platform_enable_irq_fn_t)(0))

typedef void (*bl_platform_disable_irq_fn_t)(void);
#define bl_platform_disable_irq    ((bl_platform_disable_irq_fn_t)(0))

typedef size_t (*bl_platform_get_systick_fn_t)(void);
#define bl_platform_get_systick    ((bl_platform_get_systick_fn_t)(0))
/** \} */


/** \addtogroup section no_init
 ** \{ */
#if defined(__GNUC__) && !defined(__linux__)
  #define SECTION_NO_INIT     __attribute__((section("NO_INIT"), zero_init))
#elif defined(__GNUC__) && defined(__linux__)
  #define SECTION_NO_INIT
#elif defined(__ICCARM__)
  #define SECTION_NO_INIT     __no_init
#elif defined(__CC_ARM)
  #define SECTION_NO_INIT     __attribute__((section("NO_INIT"), zero_init))
#else
  #define SECTION_NO_INIT     // 默认空
#endif
/** \} */

/** \addtogroup main pre
 ** \{ */
#ifdef BL_USE_CONSTRUCTOR_ATTRIBUTE_RRD
    #define BL_CONSTRUCTOR_ATTRIBUTE __attribute__((constructor))
#else
    #define BL_CONSTRUCTOR_ATTRIBUTE
#endif
/** \} */

#endif
