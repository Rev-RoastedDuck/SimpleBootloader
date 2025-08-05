<div align="center">
  <img src="./images/icon.webp" height="150">
  <h1>SimpleBootloader</h1>
  <span>A lightweight, modular C Bootloader solution</span>
</div>
<br>
<div align="center">
  <img src="https://img.shields.io/badge/License-GPLv3-green?logoColor=63%2C%20185%2C%2017&label=license&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
  <img src="https://img.shields.io/badge/Language-C-green?logoColor=63%2C%20185%2C%2017&labelColor=63%2C%20185%2C%2017&color=63%2C%20185%2C%2017">
</div>
<p align="center">
<a href="">English</a> | <a href="../README.md">简体中文</a>
</p>

## Introduction
**SimpleBootloader** is a lightweight C bootloader designed for embedded systems. It supports firmware upgrades, application switching, protocol extension, and features a clear structure for easy integration and secondary development.


<img src="./images/feature.png">



## Features
- Dual application area (A/B) management with rollback after upgrade. Each area has independent firmware info (address, size, version, CRC32), and integrity is checked before boot.
- Built-in XMODEM/YMODEM serial upgrade protocols, supporting 128/1K packets, CRC16/SUM check, and retransmission. Easily extendable to CAN, USB, Ethernet, and more.
- Multi-level debug output, supporting colored logs, assertions, performance statistics, and flexible redirection to UART, semihosting, etc.
- Flexible timeout protection mechanism.
- Efficient DMA ring buffer for communication.
- Modular code structure, easy to trim and extend.

## Directory Structure
- `Bootloader/`
  - `include/`: Header files and module interfaces
    - `AlgorithmSuite/`: Algorithm modules
    - `DebugSuite/`: Debug and log modules
    - `ModemSuite/`: Protocol modules
  - `interface/`: Bootloader external interfaces
  - `src/`: Bootloader core source code
    - `bl_devices/`: Communication, Flash drivers
    - `bl_jump_asm/`: Multi-compiler jump assembly
- `Docs/`: Documentation

## Quick Start

### 1. Clone the repository
```bash
git clone https://github.com/Rev-RoastedDuck/SimpleBootloader.git
cd SimpleBootloader
```

### 2. Configuration
- Edit `Bootloader/interface/bl_config.h` according to your Flash layout and application requirements:
  - `BL_FLASH_BASE_ADDRESS`: Bootloader start address
  - `BL_APPLICATION_ADDRESS_X`: Main/backup application start address
  - `BL_FRIWARE_INFO_ADDRESS`: Firmware info storage address
  - `BL_APPLICATION_NUMBER`: Number of application areas
  - `BL_TIMEOUT_NO_OPERATE_RRD`: No-operation timeout (ms)
  - `BL_TIMEOUT_DOWNLOAD_RRD`: Download timeout (ms)
  - Other MAGIC macros are for upgrade, switch, and verification, usually no need to modify

- Edit `Bootloader/interface/bl_platform_config.h` and implement or adapt the following interfaces for your platform:
  - `bl_platform_device_init` / `bl_platform_device_deinit`: Device init/deinit
  - `bl_platform_system_reset`: System reset
  - `bl_platform_enable_irq` / `bl_platform_disable_irq`: Global IRQ enable/disable
  - `bl_platform_get_systick`: Get system tick

- Edit `Bootloader/src/bl_platform.h` to register communication interfaces and Flash devices as needed:
  - `bootloader_platform_register_comm(bl_comm_rrd *comm)`: Register all communication channels (UART, CAN, USB, etc.)
  - `bootloader_platform_register_flash(bl_flash_rrd *flash)`: Register Flash driver
  - `bootloader_platform_register_debug_com_dev(bl_comm_rrd *comm)`: Register debug communication device if needed

### 3. Configure no_init Section
- **Keil**
  - Modify the scatter file (.sct) to configure the NO_INIT section:
    - Before:
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
    - After:
      ```text
      LR_IROM1 0x08000000 0x00020000  {    ; load region size_region
        ER_IROM1 0x08000000 0x00020000  {  ; load address = execution address
        *.o (RESET, +First)
        *(InRoot$$Sections)
        .ANY (+RO)
        .ANY (+XO)
        }
        RW_IRAM1 0x20000000 0x00007C00  {  ; 31KB for RW
          .ANY (+RW +ZI)
        }
        RW_IRAM2 0x20007C00 UNINIT 0x00000400 { ; Last 1KB for NO_INIT
          .ANY (NO_INIT)
        }
      }
      ```

### 4. Flashing
- Flash the generated Bootloader binary to the Flash base address, and configure the application address as needed.
- When upgrading or flashing the application, use **partial erase** to avoid erasing the Bootloader or firmware info area.

## Notes
- `Magic` macros (for upgrade, switch, reset, etc.) are **32-bit unsigned integers**. Ensure **endianness consistency** in communication and storage, use **byte representation** (not string), and avoid command recognition errors due to endianness.
- If `BL_CONSTRUCTOR_ATTRIBUTE` (i.e., `__attribute__((constructor))`) is not enabled, **you must manually call `bootloader_pre_main()` at the very top of your `main()` function** to ensure correct jump flag and boot process.
- When flashing Bootloader and applications, make sure their start addresses, sizes, and partitions do not overlap.
- When using partial erase for application upgrade, do not erase the Bootloader or firmware info area to prevent system startup or rollback failure.
- If using the NO_INIT section, ensure the linker script or scatter file is correctly partitioned to avoid overlap with normal RAM.
- The start addresses of Bootloader and applications must be **aligned to Flash page size**. For example, STM32 commonly uses 2KB, 4KB, or 8KB pages. Set `BL_FLASH_BASE_ADDRESS`, `BL_APPLICATION_ADDRESS_A/B`, etc., accordingly.

## TODO
- Provide GUI upgrade tool and host support
- Enhance breakpoint resume and auto-recovery on upgrade failure
- Add secure encrypted upgrade (e.g., AES, RSA signature verification)

## License
This project is licensed under the Apache License 2.0. See [**LICENSE**](../LICENSE) for details.