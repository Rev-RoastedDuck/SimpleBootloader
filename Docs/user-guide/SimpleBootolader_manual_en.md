<div align="center">
  <img src="../images/icon.webp" height="150">
  <h1>SimpleBootloader</h1>
  <span>A lightweight, modular C Bootloader solution</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-GPLv3-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>

## Configuration Files

### `bl_config.h`

- **Important Parameters**
  - `BL_FLASH_BASE_ADDRESS`  
    Start address of the Bootloader in Flash. Must match this address when flashing the Bootloader.

  - `BL_APPLICATION_ADDRESS_A` / `BL_APPLICATION_ADDRESS_B`  
    Start addresses of the primary/backup application areas. Supports dual application area (A/B) management, allowing rollback after an upgrade. Each area should be independently partitioned and must not overlap with the Bootloader region.

  - `BL_FRIWARE_INFO_ADDRESS`  
    Address of the firmware information storage area. Used to store metadata of each application area (e.g., version, size, checksum, etc.)

  - `BL_APPLICATION_NUMBER`  
    Number of supported application areas.

  - `BL_USE_CONSTRUCTOR_ATTRIBUTE_RRD`  
    Whether to enable the constructor attribute (`__attribute__((constructor))`), used to automatically execute the Bootloader preprocessing flow.

- **Magic Numbers**
  - `BL_UPGRADE_MAGIC_RRD`  
    Magic flag for firmware upgrade.

  - `BL_SWITCH_APP_MAGIC_RRD`  
    Magic flag for switching application areas.

  - `BL_RESET_APP_MANAGER_MAGIC_RRD`  
    Magic flag for resetting the application manager.

  - `BL_FIRWARE_INFO_MAGIC_RRD`  
    Magic flag of the firmware info area, used to verify the validity of the info section.

  - `BL_JUMP_APP_FLAG_MAGIC_RRD`  
    Flag magic before jumping to the application.

- **Timeout Configuration Parameters**
  - `BL_TIMEOUT_NO_OPERATE_RRD`  
    Timeout for no operation (unit: ms). After timeout, enters firmware or standby waiting for firmware loading.

  - `BL_TIMEOUT_DOWNLOAD_RRD`  
    Download timeout (unit: ms), used for timeout protection during the upgrade process.

### `bl_platform_config.h`

- **Important Interfaces**
  - `bl_platform_device_init` / `bl_platform_device_deinit`  
    Function pointers for device initialization and deinitialization. Must be implemented by the user according to the target platform, mainly used to initialize/release peripheral resources.  
    `bl_platform_device_deinit` is mainly used for memory release. SimpleBootloader resets and jumps to the application firmware, so manual peripheral release is not required.

  - `bl_platform_system_reset`  
    Function pointer for system reset. Can be called internally in the Bootloader to implement software reset for firmware jump.

  - `bl_platform_enable_irq` / `bl_platform_disable_irq`  
    Function pointers for enabling/disabling global interrupts. Used to control interrupts during key Bootloader processes.

  - `bl_platform_get_systick`  
    Get system timestamp, used for timeout checks.

- **Other Interfaces**
  - `SECTION_NO_INIT`  
    Macro for defining NO_INIT sections. Variables in the NO_INIT section will not be initialized and are used to store upgrade flags and firmware jump addresses. Must be correctly configured with linker scripts or scatter files.

  - `BL_CONSTRUCTOR_ATTRIBUTE`  
    Macro for constructor attribute.

## Platform Interfaces
Used mainly for registering and managing communication channels, Flash devices, and debug channels supported by the Bootloader.

### `bl_platform.h`

- When porting or integrating, users need to call the following interfaces to register low-level resources:
  - `bootloader_platform_register_comm(bl_comm_rrd *comm)`: Register communication channel (e.g., UART, CAN, USB, etc.)
  - `bootloader_platform_register_flash(bl_flash_rrd *flash)`: Register Flash device driver.
  - `bootloader_platform_register_debug_com_dev(bl_comm_rrd *comm)`: Register debug communication device (optional).

## Device Abstraction

### Communication Device Base Class (`bl_comm.h`)
SimpleBootloader separates command and firmware handling by detecting the `dev_rx_buff` and `curr_data_buff` in the `bl_comm_rrd` structure registered via `bootloader_platform_register_comm(bl_comm_rrd *comm)`.

- **Struct Description**
  - `bl_comm_rrd`: Communication device object, includes interface pointers, receive buffer, current data buffer, and platform-specific data pointer.
  - `bl_comm_interface_rrd`: Set of communication interface functions. Must implement the following function pointers:
    - `write(self, data, length)`: Data send interface.
    - `switch_buff(self, buff_addr)`: DMA ring buffer switch interface.  
      SimpleBootloader adopts a double-buffer design, separating command buffer and firmware receiving buffer, reducing RAM usage and supporting multiple communication devices for firmware upgrades.

- **Main Members**
  - `dev_rx_buff`: Device receive buffer.
  - `curr_data_buff`: Pointer to current data buffer (either command or firmware).
  - `platform_data`: Platform-specific custom data pointer, usually used to store subclass object pointers.

- **Usage Instructions**
  1. Implement `write` and `switch_buff` functions and fill them into the `bl_comm_interface_rrd` structure.
  2. Initialize the `bl_comm_rrd` object and register it to the Bootloader using `bootloader_platform_register_comm`.
  3. Supports concurrent registration and management of multiple communication devices.

- **Others**
  - **DMA_CIRCULAR_QUEUE_RRD**
    - **Related Methods:**
      - `enqueue(self, value)`: Write a single data unit into the queue.
      - `batch_enqueue(self, buff, size)`: Write batch data into the queue, suitable for DMA or interrupt batch receiving scenarios.
      - `dma_circular_queue_init(self, buff_lenth, unit_size, buff_alignment)`: Initialize the circular queue, allocate a buffer with the specified length and unit size. On some platforms (e.g., with DMA requirements), the buffer address needs to be aligned. The `buff_alignment` parameter ensures this.

    - **Usage Flow:**
      1. Call `dma_circular_queue_init` to initialize the `DMA_CIRCULAR_QUEUE_RRD`.
      2. In the interrupt or DMA callback, use `enqueue(self, value)` or `batch_enqueue(self, buff, size)` to add newly received data into `curr_data_buff`.

### Flash Device Base Class (`bl_flash.h`)

- **Struct Description**
  - `bl_flash_rrd`: Flash device object, includes interface pointer and platform-specific data pointer.
  - `bl_flash_interface_rrd`: Set of Flash operation interface functions. Must implement the following function pointers:
    - `read(self, start_addr, buff, length)`: Flash read interface.
    - `write(self, start_addr, buff, length)`: Flash write interface.
    - `write_pre(self, start_addr)`: Preparation before writing (e.g., erase).

- **Main Members**
  - `platform_data`: Platform-specific custom data pointer, usually used to store subclass object pointers.

- **Usage Instructions**
  1. Implement `read`, `write`, and `write_pre` functions and fill them into the `bl_flash_interface_rrd` structure.
  2. Initialize the `bl_flash_rrd` object and register it to the Bootloader using `bootloader_platform_register_flash`.
  3. Only one Flash device is supported.

### Development Process

1. **Implement Interface Functions**: Implement the communication and Flash interfaces according to the target hardware.
2. **Fill Interface Structures**: Fill the implemented function pointers into the corresponding interface structure.
3. **Initialize Device Objects**: Create and initialize `bl_comm_rrd` or `bl_flash_rrd` objects.
4. **Register to Bootloader**: Register via `bootloader_platform_register_comm` or `bootloader_platform_register_flash`.

## Notes
### About NO_INIT
#### **Keil**
[**.sct example**](./ld_example/bl_example.sct)

**1. Modify Keil Settings**

<img src="./images/keil_sct_config.png" height="400">

**2. Modify the `scatter` file (.sct)**
- **Before Modification:**
  ```text
  LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
    ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
    *.o (RESET, +First)
    *(InRoot$$Sections)
    .ANY (+RO)
    .ANY (+XO)
    }
    RW_IRAM1 0x20000000 0x00000400  {
      .ANY (+RW +ZI)
    }
  }
  ```
- **After Modification:**
  ```text
  LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
    ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
    *.o (RESET, +First)
    *(InRoot$$Sections)
    .ANY (+RO)
    .ANY (+XO)
    }
    RW_IRAM1 0x20000000 0x00007C00  {  ; Shrink RW region to 31KB
      .ANY (+RW +ZI)
    }
    RW_IRAM2 0x20007C00 UNINIT 0x00000400 { ; Last 1KB used for NO_INIT
      .ANY (NO_INIT)
    }
  }
  ```

#### **GNUC**
[**.ld example**](./ld_example/STM32G431RBTx_FLASH.ld)

**1. Open the linker script file (`.ld`), and add a `.noinit` section after `.bss`.**

- **Before Modification:**

  ```text
  ...
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM
  ...
  ```

- **After Modification:**

  ```text
  ...
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* Uninitialized non-zeroed data section (NO_INIT) */
  .noinit (NOLOAD) :
  {
    . = ALIGN(4);
    *(.noinit)
    *(.NO_INIT)
    *(.noinit*)
    . = ALIGN(4);
  } >RAM
  ...
  ```
### Application Firmware Notes
- **The start address of the application must match BL_APPLICATION_ADDRESS_X in the Bootloader configuration file**
- **Firmware files downloaded to the device must be in bin format. Hex or elf formats are not supported.**
- `Magic` macros (for upgrade, switch, reset, etc.) are **32-bit unsigned integers**. Ensure **endianness consistency** in communication and storage, use **byte representation** (not string), and avoid command recognition errors due to endianness.
- If `BL_CONSTRUCTOR_ATTRIBUTE` (i.e., `__attribute__((constructor))`) is not enabled, **you must manually call `bootloader_pre_main()` at the very top of your `main()` function** to ensure correct jump flag and boot process.
- When flashing Bootloader and applications, make sure their start addresses, sizes, and partitions do not overlap.
- When using partial erase for application upgrade, do not erase the Bootloader or firmware info area to prevent system startup or rollback failure.
- If using the NO_INIT section, ensure the linker script or scatter file is correctly partitioned to avoid overlap with normal RAM.
- The start addresses of Bootloader and applications must be **aligned to Flash page size**. For example, STM32 commonly uses 2KB, 4KB, or 8KB pages. Set `BL_FLASH_BASE_ADDRESS`, `BL_APPLICATION_ADDRESS_A/B`, etc., accordingly.

## License
This project **SimpleBootloader** is released under the Apache License 2.0. For details, please refer to [**LICENSE**](/LICENSE)
