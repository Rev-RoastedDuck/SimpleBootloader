/*
 * bl_main.h
 *	
 *
 *  Created on: 2025_07_31
 *      Author: Rev_RoastDuck
 *      Github: https://github.com/Rev-RoastedDuck
 * 
 * :copyright: (c) 2025 by Rev-RoastedDuck.
 */

#include "bl_main.h"
/******************************************************************************/
/*------------------------------bl_firware_info-------------------------------*/
/******************************************************************************/

/** \addtogroup crc32
 ** \{ */
static uint32_t *g_bl_firware_crc32_table;
void bl_firware_crc32_table_init(void) {
    g_bl_firware_crc32_table = (uint32_t*)malloc(sizeof(uint32_t)*256);
    uint32_t crc;
    for (uint32_t i = 0; i < 256; i++) {
        crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
        g_bl_firware_crc32_table[i] = crc;
    }
}

void bl_firware_crc32_table_deinit(void) {
    free(g_bl_firware_crc32_table);
}

uint32_t bl_firware_crc32_compute(const uint8_t *data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ g_bl_firware_crc32_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}
/** \} */

/** \addtogroup app_info
 ** \{ */
// aligned(4)
typedef struct __BL_APP_INFO_RRD{
    uint8_t valid_firware;
    uint32_t magic;
    uint32_t app_crc32;
    uint32_t app_addr;
    uint32_t app_size;
    uint32_t app_version;
}BL_APP_INFO_RRD, bl_app_info_rrd;

// aligned(4)
typedef struct __BL_APPS_MANAGER_RRD{
    bl_app_info_rrd apps[2];

    uint32_t magic;
    uint8_t upgrade_flag;
    uint8_t download_app_count;
    uint8_t download_app_index;
}BL_APPS_MANAGER_RRD, bl_apps_manager_rrd;

static bl_apps_manager_rrd *g_bl_apps_manager_rrd;
static inline void bl_apps_manager_new(void){
    g_bl_apps_manager_rrd = (bl_apps_manager_rrd *)malloc(sizeof(bl_apps_manager_rrd));
}

static inline void bl_apps_manager_init(bl_apps_manager_rrd *self){
    self->upgrade_flag = 0;
    self->download_app_index = 0;
    self->magic = BL_FIRWARE_INFO_MAGIC_RRD;

    self->apps[0].app_size = 0;
    self->apps[0].app_crc32 = 0;
    self->apps[0].app_version = 0;
    self->apps[0].valid_firware = 0;
    self->apps[0].magic = BL_FIRWARE_INFO_MAGIC_RRD;
    self->apps[0].app_addr = BL_APPLICATION_ADDRESS_A;

    self->apps[1].app_size = 0;
    self->apps[1].app_crc32 = 0;
    self->apps[1].app_version = 0;
    self->apps[1].valid_firware = 0;
    self->apps[1].magic = BL_FIRWARE_INFO_MAGIC_RRD;
    self->apps[1].app_addr = BL_APPLICATION_ADDRESS_B;
}

static void bl_apps_manager_reset(bl_apps_manager_rrd *self){
    bl_apps_manager_init(self);
    g_bl_flash->interface->write_pre(g_bl_flash, BL_FRIWARE_INFO_ADDRESS);
    g_bl_flash->interface->write(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));
}

static inline bool bl_apps_manager_have_firware(bl_apps_manager_rrd *self){
    return self->apps[0].valid_firware || self->apps[1].valid_firware;
}

static inline void bl_apps_manager_upgrade_success(bl_apps_manager_rrd *self, uint32_t finished_addr){
    bl_app_info_rrd *app = &self->apps[self->download_app_index];

    app->valid_firware = true;
    app->app_size = finished_addr - app->app_addr;
    app->app_crc32 = bl_firware_crc32_compute((const uint8_t *)(uintptr_t)app->app_addr, app->app_size);

    self->upgrade_flag = 0;
    self->download_app_count ++;
    self->download_app_index = (self->download_app_index + 1) % BL_APPLICATION_NUMBER;

    g_bl_flash->interface->write_pre(g_bl_flash, BL_FRIWARE_INFO_ADDRESS);
    g_bl_flash->interface->write(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));

    DEBUG_PRINT_SUCCESS(1, "[bootloader] app-%d-info: addr-0x%08X "
                                "crc32-0x%08X size-%d version-%u ",
                                self->download_app_index,
                                app->app_addr, app->app_crc32,
                                app->app_size, app->app_version);
}

static inline void bl_apps_manager_clear_app_info(bl_apps_manager_rrd *self, uint8_t app_index){
    DEBUG_ASSERT(app_index < BL_APPLICATION_NUMBER);
    self->apps[app_index].app_size = 0;
    self->apps[app_index].app_crc32 = 0;
    self->apps[app_index].app_version = 0;
    self->apps[app_index].valid_firware = 0;
    self->apps[app_index].magic = BL_FIRWARE_INFO_MAGIC_RRD;
    self->apps[app_index].app_addr = app_index == 0? BL_APPLICATION_ADDRESS_A : BL_APPLICATION_ADDRESS_B;
    g_bl_flash->interface->write_pre(g_bl_flash, BL_FRIWARE_INFO_ADDRESS);
    g_bl_flash->interface->write(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));
}

static inline int8_t bl_apps_manager_switch_app(bl_apps_manager_rrd *self){
    if(!self->apps[self->download_app_index].valid_firware){
        return -1;
    }
    self->download_app_index = (self->download_app_index + 1) % BL_APPLICATION_NUMBER;
    g_bl_flash->interface->write_pre(g_bl_flash, BL_FRIWARE_INFO_ADDRESS);
    g_bl_flash->interface->write(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));
    return 0;
}

static inline bool bl_apps_manager_check_app_info(bl_apps_manager_rrd *self, uint8_t app_index){
    DEBUG_ASSERT(app_index < BL_APPLICATION_NUMBER);
    bl_app_info_rrd *app = &self->apps[app_index];
    return app->magic == BL_FIRWARE_INFO_MAGIC_RRD 
            && app->app_crc32 == bl_firware_crc32_compute((const uint8_t *)(uintptr_t)app->app_addr, app->app_size);
}

static inline uint32_t bl_apps_manager_get_download_addr(bl_apps_manager_rrd *self){
    return self->apps[self->download_app_index].app_addr;
}

static int8_t bl_apps_manager_get_app_index(bl_apps_manager_rrd *self){
    uint8_t index = (BL_APPLICATION_NUMBER + self->download_app_index - 1) % BL_APPLICATION_NUMBER;
    if(!self->apps[index].valid_firware){
        return -1;
    }
    return index;
}

static uint32_t bl_apps_manager_get_app_addr(bl_apps_manager_rrd *self){
    uint8_t index = (BL_APPLICATION_NUMBER + self->download_app_index - 1) % BL_APPLICATION_NUMBER;
    if(!self->apps[index].valid_firware){
        return 0;
    }
    return self->apps[index].app_addr;
}

static void bl_apps_manager_load_info(bl_apps_manager_rrd *self){
    g_bl_flash->interface->read(g_bl_flash, BL_FRIWARE_INFO_ADDRESS, self, sizeof(BL_APPS_MANAGER_RRD));

    if(self->magic != BL_FIRWARE_INFO_MAGIC_RRD){
        bl_apps_manager_reset(self);
        DEBUG_PRINT_ERROR(1,"[bootloader] bl_apps_manager magic checked failed. magic-0x%08X", self->magic);
    }else{
            DEBUG_PRINT_INFO(1, "\n[bootloader] App Manager Info:");
            DEBUG_PRINT_INFO(1, "+--------+--------+------------+------------+--------+------------+");
            DEBUG_PRINT_INFO(1, "| Index  | Valid  | Addr       | CRC32      | Size   | Version    |");
            DEBUG_PRINT_INFO(1, "+--------+--------+------------+------------+--------+------------+");

            for (uint8_t index = 0; index < BL_APPLICATION_NUMBER; ++index) {
                DEBUG_PRINT_INFO(1, "| %-6d | %-6d | 0x%08X | 0x%08X | %-6d | %-10u |",
                            index,
                            self->apps[index].valid_firware,
                            self->apps[index].app_addr,
                            self->apps[index].app_crc32,
                            self->apps[index].app_size,
                            self->apps[index].app_version);
            }

            DEBUG_PRINT_INFO(1, "+--------+--------+------------+------------+--------+------------+");

            DEBUG_PRINT_INFO(1, "[bootloader] Download Count: %u, Upgrade Flag: %d, APP Index: %d",
                                                                self->download_app_count,
                                                                self->upgrade_flag,
                                                                bl_apps_manager_get_app_index(self));

        for(uint8_t index = 0; index < BL_APPLICATION_NUMBER; ++index){
            if(self->apps[index].valid_firware){
                bool ret = bl_apps_manager_check_app_info(self, index);
                if(!ret){
                    DEBUG_PRINT_ERROR(1,"[bootloader] app-%d magic/crc32 checked failed. ",index);
                    bl_apps_manager_clear_app_info(self, index);
                }
            }
        }
    }
}

/** \} */

/******************************************************************************/
/*--------------------------------bl_timeout----------------------------------*/
/******************************************************************************/
typedef enum __BL_TIMEOUT_TYPE{
    BL_TIMEOUT_OPERATE = 0,
    BL_TIMEOUT_DOWNLOAD,
    BL_TIMEOUT_MAX
}BL_TIMEOUT_TYPE;

typedef struct __BL_TIMEOUT_MANAGER{
    uint32_t timestamp[BL_TIMEOUT_MAX];
}BL_TIMEOUT_MANAGER, bl_timeout_manager;

static void bl_timeout_update(bl_timeout_manager *self, BL_TIMEOUT_TYPE type) {
    if (type < BL_TIMEOUT_MAX)
        self->timestamp[type] = bl_platform_get_systick();
}

static bool bl_timeout_expired(bl_timeout_manager *self, BL_TIMEOUT_TYPE type, uint32_t timeout_ms) {
    if (type >= BL_TIMEOUT_MAX)
        return true;
    return (bl_platform_get_systick() - self->timestamp[type]) > timeout_ms;
}

/******************************************************************************/
/*----------------------------------bl_jump-----------------------------------*/
/******************************************************************************/
SECTION_NO_INIT volatile uint32_t bl_app_jump_flag;
SECTION_NO_INIT volatile uint32_t bl_app_jump_address;

static inline void bootloader_jump_to_app_pre(void){
    bl_app_jump_address = bl_apps_manager_get_app_addr(g_bl_apps_manager_rrd);
    if(bl_app_jump_address == 0){
        DEBUG_PRINT_ERROR(1,"[bootloader] failed to get app addr. ");
        return;
    }
    bl_app_jump_flag = BL_JUMP_APP_FLAG_MAGIC_RRD;
    bl_platform_system_reset();
}

static inline void bootloader_jump_to_app(void){
   bl_platform_disable_irq();
   #ifdef SCB
       SCB->VTOR = bl_app_jump_address;
   #endif
	bl_platform_enable_irq();
   __bootloader_jump_to_app(bl_app_jump_address);
}

/******************************************************************************/
/*---------------------------------bl_upgrade---------------------------------*/
/******************************************************************************/
/** \addtogroup modem callback
 ** \{ */
static bl_comm_rrd *g_modem_comm_dev;
static uint32_t g_bl_download_addr = 0;
static void modem_save_data(uint8_t *data, size_t length){
    g_bl_flash->interface->write(g_bl_flash, g_bl_download_addr, data, length);
    g_bl_download_addr += (uint32_t)length;
}

static void modem_send_data(uint8_t *data, size_t length){
    g_modem_comm_dev->interface->write(g_modem_comm_dev,data,length);
}
/** \} */

typedef enum __BL_CMD{
    BL_CMD_NONE = 0,
    BL_CMD_UPGRADE,
    BL_CMD_SWITCH_APP,
    BL_CMD_RESET_APP_MANAGER,
}BL_CMD;

typedef enum __BL_STATUS{
    BL_STATUS_WAIT_FOR_CMD = 0,
    BL_STATUS_UPGRADE,
}BL_STATUS;
BL_STATUS bl_status = BL_STATUS_WAIT_FOR_CMD;
static BL_CMD bootloader_upgrade_wait_for_cmd(bl_comm_rrd **g_modem_comm_dev){
    size_t length = 0;
    bl_comm_rrd *temp;
    bootloader_platform_com_for(temp){
        length = temp->dev_rx_buff.interface->get_lenth(&temp->dev_rx_buff);
        if(length > 3){
            uint8_t *data_buff = (uint8_t*)temp->dev_rx_buff.data;
            for(uint32_t i = 0;i < length - 3;++i){
                uint32_t magic = (*(data_buff + temp->dev_rx_buff.interface->get_next_index_rev(&temp->dev_rx_buff,i+3)) << 24) 
                               | (*(data_buff + temp->dev_rx_buff.interface->get_next_index_rev(&temp->dev_rx_buff,i+2)) << 16) 
                               | (*(data_buff + temp->dev_rx_buff.interface->get_next_index_rev(&temp->dev_rx_buff,i+1)) << 8) 
                               | (*(data_buff + temp->dev_rx_buff.interface->get_next_index_rev(&temp->dev_rx_buff,i)));
                if(BL_UPGRADE_MAGIC_RRD == magic
                    || BL_SWITCH_APP_MAGIC_RRD == magic
                    || BL_RESET_APP_MANAGER_MAGIC_RRD == magic){
                    *g_modem_comm_dev = temp;
                    temp->dev_rx_buff.interface->clear(&temp->dev_rx_buff);
                    if(BL_UPGRADE_MAGIC_RRD == magic){
                        DEBUG_PRINT_INFO(1,"[bootloader] get cmd UPGRADE. ");
                        return BL_CMD_UPGRADE;
                    }else if (BL_SWITCH_APP_MAGIC_RRD == magic){
                        DEBUG_PRINT_INFO(1,"[bootloader] get cmd SWITCH_APP. ");
                        return BL_CMD_SWITCH_APP;
                    }else if (BL_RESET_APP_MANAGER_MAGIC_RRD == magic){
                        DEBUG_PRINT_INFO(1,"[bootloader] get cmd RESET_APP_MANAGER. ");
                        return BL_CMD_RESET_APP_MANAGER;
                    }
                }
            }
        }
    }
    return BL_CMD_NONE;
}

static void bootloader_upgrade_app(void){
    /** upgrade_buff
     ** \{ */
    DMA_CIRCULAR_QUEUE_RRD upgrade_buff;
    DMA_CIRCULAR_QUEUE_RRD *upgrade_buff_ptr = &upgrade_buff;
    dma_circular_queue_init(upgrade_buff_ptr,256,sizeof(uint8_t),256);
    /** \} */

    /** modem
     ** \{ */
    XYMODEM_RECEIVER_RRD xmodem;
    xmodem.config.max_retry_count = 10;
    xymodem_receiver_init(&xmodem, modem_xmodem, modem_128, modem_crc16, modem_send_data, bl_platform_get_systick, modem_save_data);
    /** \} */

    /** timeout_manager
     ** \{ */
    bl_timeout_manager bl_timeout_manager = {0};
    #define bl_timeout_update_operate()     bl_timeout_update(&bl_timeout_manager, BL_TIMEOUT_OPERATE)
    #define bl_timeout_update_download()    bl_timeout_update(&bl_timeout_manager, BL_TIMEOUT_DOWNLOAD)
    #define bl_operate_timeout()            bl_timeout_expired(&bl_timeout_manager, BL_TIMEOUT_OPERATE, BL_TIMEOUT_NO_OPERATE_RRD)
    #define bl_download_timeout()           bl_timeout_expired(&bl_timeout_manager, BL_TIMEOUT_DOWNLOAD, BL_TIMEOUT_DOWNLOAD_RRD)
    
    bl_timeout_update_operate();
    bl_timeout_update_download();
    /** \} */

    while(1){
        if(BL_STATUS_WAIT_FOR_CMD == bl_status){
            BL_CMD cmd = bootloader_upgrade_wait_for_cmd(&g_modem_comm_dev);
            cmd != BL_CMD_NONE && (bl_timeout_update_operate(),true);
            if(bl_apps_manager_have_firware(g_bl_apps_manager_rrd) && bl_operate_timeout()){
                goto bl_operate_timeoutcode;
            }
            switch (cmd){
                case BL_CMD_UPGRADE:
                    DEBUG_PRINT_INFO(1,"[bootloader] waitting for first pack of new firware. ");
                    g_bl_download_addr = bl_apps_manager_get_download_addr(g_bl_apps_manager_rrd);
                    g_modem_comm_dev->interface->switch_buff(g_modem_comm_dev,upgrade_buff_ptr);
                    g_modem_comm_dev->curr_data_buff->interface->clear(g_modem_comm_dev->curr_data_buff);
                    bl_status = BL_STATUS_UPGRADE;
                    bl_timeout_update_download();
                    while (1){
                        xmodem.interface->start(&xmodem);
                        size_t length = upgrade_buff_ptr->interface->get_lenth(upgrade_buff_ptr);
                        if(length > 0){
                            bl_status = BL_STATUS_UPGRADE;
                            g_bl_flash->interface->write_pre(g_bl_flash, g_bl_download_addr);
                            DEBUG_PRINT_INFO(1,"[bootloader] start upgrade.");
                            break;
                        }
                        if(bl_download_timeout()){
                            bl_status = BL_STATUS_WAIT_FOR_CMD;
                            g_modem_comm_dev->interface->switch_buff(g_modem_comm_dev,NULL);
                            DEBUG_PRINT_FATAL(1,"[bootloader] timeout waitting for first pack of new firware.");
                            break;
                        }
                    }
                    break;
                case BL_CMD_SWITCH_APP:
                    if(bl_apps_manager_switch_app(g_bl_apps_manager_rrd) >= 0){
                        DEBUG_PRINT_SUCCESS(1,"[bootloader] success to switch app-%d-addr-%X.",bl_apps_manager_get_app_index(g_bl_apps_manager_rrd),
                                                                                           bl_apps_manager_get_app_addr(g_bl_apps_manager_rrd));
                        goto bl_switch_download_app;
                    }
                    DEBUG_PRINT_FATAL(1,"[bootloader] failed to switch app.");
                    break;
                case BL_CMD_RESET_APP_MANAGER:
                    bl_apps_manager_reset(g_bl_apps_manager_rrd);
                    DEBUG_PRINT_INFO(1,"[bootloader] reset apps manager info.");
                    break;
                default:
                    break;
            }
        }
        else if(BL_STATUS_UPGRADE == bl_status){
            if(bl_download_timeout()){
                xmodem.interface->stop(&xmodem);
                goto bl_download_timeout_code;
            }
            size_t length = upgrade_buff_ptr->interface->get_lenth(upgrade_buff_ptr);
            if(length == 1 || length > MODEM_128_SIZE){
                uint8_t *raw_data = (uint8_t*)malloc(sizeof(uint8_t) * length);
                {
                    uint8_t *data_u8;
                    uint8_t *raw_data_temp = raw_data;
                    dma_circular_queue_for_each_const(upgrade_buff_ptr, data_u8){
                        *(raw_data_temp++) = *data_u8;
                    }
                }
                
                int result = xmodem.interface->unpack(&xmodem, raw_data, length, NULL, 0, NULL, NULL);
                if(result < 0){
                    DEBUG_PRINT_WARNING(1,"[bootloader] [upgrade] unpack error %d.",result);
                } else if (result == MODEM_CODE_UNPACK_SUCCESS){
                    DEBUG_PRINT_SUCCESS(1,"[bootloader] [upgrade] unpack success.");
                    bl_timeout_update_download();
                    upgrade_buff_ptr->interface->clear(upgrade_buff_ptr);
                } else if (result == MODEM_CODE_PACK_FINISHED){
                    DEBUG_PRINT_SUCCESS(1,"[bootloader] [upgrade] unpack finished.");
                    bl_timeout_update_download();
                    bl_apps_manager_upgrade_success(g_bl_apps_manager_rrd, g_bl_download_addr);
                    upgrade_buff_ptr->interface->clear(upgrade_buff_ptr);
                    break;
                }
                free(raw_data);
            }
        }
    }

bl_switch_download_app:
bl_download_timeout_code:
bl_operate_timeoutcode:
    upgrade_buff_ptr->del((void **)&upgrade_buff_ptr);
    DEBUG_PRINT_INFO(1, "[bootloader] launch app%d...",bl_apps_manager_get_app_index(g_bl_apps_manager_rrd));
}

/******************************************************************************/
/*------------------------------bl_debug_comm_dev-----------------------------*/
/******************************************************************************/
#if defined(__GNUC__)
    int _write(int file, char *ptr, int len) {
        if(g_bl_debug_comm_dev != NULL){
            if(!(bl_status != BL_STATUS_WAIT_FOR_CMD
                && g_bl_debug_comm_dev == g_modem_comm_dev)){
                g_bl_debug_comm_dev->interface->write(g_bl_debug_comm_dev,(uint8_t *)ptr,len);
            }
        }
        return len;
    }
#elif defined(__ICCARM__)
    size_t __write(int handle, const unsigned char *buf, size_t size) {
        if (g_bl_debug_comm_dev != NULL) {
            g_bl_debug_comm_dev->interface->write(g_bl_debug_comm_dev, (uint8_t *)buf, size);
            return size;
        }
        return 0;
    }
#elif defined(__CC_ARM)
    int fputc(int ch, FILE *f){
        if(g_bl_debug_comm_dev != NULL){
            if(!(bl_status != BL_STATUS_WAIT_FOR_CMD
                && g_bl_debug_comm_dev == g_modem_comm_dev)){
                g_bl_debug_comm_dev->interface->write(g_bl_debug_comm_dev,(uint8_t *)(&ch),1);
            }
        }
        return ch;
    }
#endif

/******************************************************************************/
/*-----------------------------------bl_main----------------------------------*/
/******************************************************************************/
BL_CONSTRUCTOR_ATTRIBUTE
void bootloader_pre_main(void) {
  if(BL_JUMP_APP_FLAG_MAGIC_RRD == bl_app_jump_flag){
        bl_app_jump_flag = 0;
  	    bootloader_jump_to_app();
  }
}

void bootloader_main(void){
    bl_platform_device_init();
    bl_firware_crc32_table_init();
    DEBUG_PRINT_INFO(1, "\n[bootloader] Timeout Config:");
    DEBUG_PRINT_INFO(1, "+---------------+------------+");
    DEBUG_PRINT_INFO(1, "| Item          | Timeout(ms)|");
    DEBUG_PRINT_INFO(1, "+---------------+------------+");
    DEBUG_PRINT_INFO(1, "| no operation  | %-10d |", BL_TIMEOUT_NO_OPERATE_RRD);
    DEBUG_PRINT_INFO(1, "| download      | %-10d |", BL_TIMEOUT_DOWNLOAD_RRD);
    DEBUG_PRINT_INFO(1, "+---------------+------------+");

    DEBUG_PRINT_INFO(1, "\n[bootloader] Command:");
    DEBUG_PRINT_INFO(1, "+---------------+------------+");
    DEBUG_PRINT_INFO(1, "| Command       | Value      |");
    DEBUG_PRINT_INFO(1, "+---------------+------------+");
    DEBUG_PRINT_INFO(1, "| upgrade       | 0x%08X |", BL_UPGRADE_MAGIC_RRD);
    DEBUG_PRINT_INFO(1, "| switch_app    | 0x%08X |", BL_SWITCH_APP_MAGIC_RRD);
    DEBUG_PRINT_INFO(1, "| reset_bl      | 0x%08X |", BL_RESET_APP_MANAGER_MAGIC_RRD);
    DEBUG_PRINT_INFO(1, "+---------------+------------+");

    bl_apps_manager_new();
    bl_apps_manager_load_info(g_bl_apps_manager_rrd);

    bootloader_upgrade_app();
    
    bl_platform_device_deinit();
    bl_firware_crc32_table_deinit();

    bootloader_jump_to_app_pre();

    while (1){
    }
}
