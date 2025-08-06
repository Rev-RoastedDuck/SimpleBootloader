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
- support `Keil MDK (ARMCC)`,`GNUC` and `IAR(ICCARM)`

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
**To obtain the project examples, execute the following command:**
``` bash
git lfs install
git lfs pull --include="Example/stm32g431rbt6/**"
```

### 2. Configuration
- Edit `Bootloader/interface/bl_config.h` according to your Flash layout and application requirements
- Edit `Bootloader/interface/bl_platform_config.h` and implement or adapt the interfaces for your platform
- Edit `Bootloader/src/bl_platform.h` to register communication interfaces and Flash devices as needed

### 3. Configure no_init Section
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

## Document
Before using **SimpleBootloader**, please read the [help document](./user-guide/SimpleBootolader_manual_en.md). Everything you need to know is here!

## TODO
- Provide GUI upgrade tool and host support
- Enhance breakpoint resume and auto-recovery on upgrade failure
- Add secure encrypted upgrade (e.g., AES, RSA signature verification)

## License
This project is licensed under the Apache License 2.0. See [**LICENSE**](../LICENSE) for details.