/*
 * bl_config.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */
#ifndef BOOTLOADER_BL_CONFIG_RRD_H_
#define BOOTLOADER_BL_CONFIG_RRD_H_

/******************************************************************************/
/*----------------------------------INCLUDE-----------------------------------*/
/******************************************************************************/
#include "stdint.h"

/******************************************************************************/
/*-----------------------------------CONFIG-----------------------------------*/
/******************************************************************************/
/** \addtogroup FLASH
 ** \{ */
//          BASE      APP_A       APP_B     firware-info
// START 0x8000000  0x0800A800  0x08014000   0x0801D800
//  END  0x0800A7FF 0x08013FFF  0x0801D7FF   0x0801FFFF  
// SIZE     42K        44K        44K          10K       ---> 128 KB
#define BL_FLASH_BASE_ADDRESS                   ((uint32_t)0x8000000)
#define BL_APPLICATION_ADDRESS_A                ((uint32_t)0x0800A800)
#define BL_APPLICATION_ADDRESS_B                ((uint32_t)0x08014000)
#define BL_FRIWARE_INFO_ADDRESS                 ((uint32_t)0x0801D800)
#define BL_APPLICATION_NUMBER                   (2)
/** \}*/

/** \addtogroup magic
 ** \{ */
#define BL_UPGRADE_MAGIC_RRD                    ((uint32_t)0xFEDCBA98)
#define BL_SWITCH_APP_MAGIC_RRD                 ((uint32_t)0x12345678)
#define BL_RESET_APP_MANAGER_MAGIC_RRD          ((uint32_t)0x87654321)
#define BL_FIRWARE_INFO_MAGIC_RRD               ((uint32_t)0x89ABCDEF)
#define BL_JUMP_APP_FLAG_MAGIC_RRD              ((uint32_t)0xDEADBEEF)
/** \}*/

/** \addtogroup compile ops
 ** \{ */
#define BL_USE_CONSTRUCTOR_ATTRIBUTE_RRD        (1)
/** \}*/

/** \addtogroup timeout ops
 ** \{ */
#define BL_TIMEOUT_NO_OPERATE_RRD               (10000)
#define BL_TIMEOUT_DOWNLOAD_RRD                 (30000)
/** \}*/

#endif
