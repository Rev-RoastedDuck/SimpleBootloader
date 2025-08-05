/*
 * bl_main.h
 *	
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#ifndef BOOTLOADER_MAIN_RRD_H_
#define BOOTLOADER_MAIN_RRD_H_

#include "xymodem.h"
#include "algorithm.h"

#include "stdint.h"

#include "bl_jump.h"
#include "bl_comm.h"
#include "bl_config.h"
#include "bl_platform.h"
#include "bl_platform_config.h"


void bootloader_main(void);

#endif
