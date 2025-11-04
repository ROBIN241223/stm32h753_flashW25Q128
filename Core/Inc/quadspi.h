/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.h
  * @brief   This file contains all the function prototypes for
  *          the quadspi.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __QUADSPI_H__
#define __QUADSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
/* Configuration switches */
#ifndef QSPI_ENABLE_XIP
#define QSPI_ENABLE_XIP          1   /* 1: enter memory-mapped mode after init, 0: skip */
#endif
#ifndef QSPI_ENABLE_DEMO
#define QSPI_ENABLE_DEMO         1   /* 1: run erase/program demo (sector 0) after init */
#endif
#ifndef QSPI_DEMO_SECTOR_ADDR
#define QSPI_DEMO_SECTOR_ADDR    0x000000UL /* sector used in demo when enabled */
#endif
/* New feature switches */
#ifndef QSPI_ENABLE_QUAD
#define QSPI_ENABLE_QUAD         1   /* 1: enable Quad Fast Read (0x6B) memory-mapped */
#endif
#ifndef QSPI_QUAD_DUMMY_CYCLES
#define QSPI_QUAD_DUMMY_CYCLES   8   /* Typical dummy cycles for fast read (adjust per datasheet & CLK) */
#endif
#ifndef QSPI_ENABLE_PATTERN_FILL
#define QSPI_ENABLE_PATTERN_FILL 0   /* 1: program a small deterministic pattern if flash appears blank */
#endif
#ifndef QSPI_PATTERN_LEN_BYTES
#define QSPI_PATTERN_LEN_BYTES   4096 /* bytes of pattern to program when enabled (multiple of page) */
#endif
#ifndef QSPI_ENABLE_APP_HEADER_CHECK
#define QSPI_ENABLE_APP_HEADER_CHECK 0 /* 1: validate simple header before allowing jump */
#endif
#ifndef QSPI_APP_MAGIC
#define QSPI_APP_MAGIC 0x51535049UL /* 'QSPI' */
#endif

/* External flash geometry (Winbond W25Q128JV and similar) */
#ifndef MEMORY_PAGE_SIZE
#define MEMORY_PAGE_SIZE         256UL     /* bytes */
#endif
#ifndef MEMORY_SECTOR_SIZE
#define MEMORY_SECTOR_SIZE       0x1000UL  /* 4KB */
#endif
#ifndef MEMORY_BLOCK_SIZE
#define MEMORY_BLOCK_SIZE        0x10000UL /* 64KB */
#endif
#ifndef MEMORY_FLASH_SIZE
#define MEMORY_FLASH_SIZE        0x01000000UL /* 16MB total */
#endif

typedef struct __attribute__((packed)) {
  uint32_t magic;      /* QSPI_APP_MAGIC */
  uint32_t length;     /* length of application image following header */
  uint32_t crc32;      /* CRC32 of application image (poly 0x04C11DB7, init 0xFFFFFFFF, xor out 0xFFFFFFFF) */
} qspi_app_header_t;

/* W25Q128 command */
#define W25Q128_CMD_WRITE_ENABLE         0x06
#define W25Q128_CMD_WRITE_DISABLE        0x04
#define W25Q128_CMD_READ_STATUS_REG      0x05
#define W25Q128_CMD_WRITE_STATUS_REG     0x01
#define W25Q128_CMD_READ_DATA            0x03
#define W25Q128_CMD_PAGE_PROGRAM         0x02
#define W25Q128_CMD_SECTOR_ERASE        0x20
#define W25Q128_CMD_BLOCK_ERASE         0xD8
#define W25Q128_CMD_CHIP_ERASE          0xC7
#define W25Q128_CMD_POWER_DOWN          0xB9
#define W25Q128_CMD_RELEASE_POWER_DOWN 0xAB
#define W25Q128_CMD_JEDEC_ID            0x9F
#define W25Q128_CMD_DEVICE_ID           0x90
/* W25Q128 JEDEC ID components */
#ifndef W25Q_MANUFACTURER_ID
#define W25Q_MANUFACTURER_ID            0xEF  /* Winbond */
#endif
#ifndef W25Q128_MEM_TYPE_40
#define W25Q128_MEM_TYPE_40             0x40  /* Some variants report 0x40 */
#endif
#ifndef W25Q128_MEM_TYPE_60
#define W25Q128_MEM_TYPE_60             0x60  /* Others report 0x60 */
#endif
#ifndef W25Q128_CAPACITY_ID
#define W25Q128_CAPACITY_ID             0x18  /* 128Mbit */
#endif
#define W25Q128_ADDRESS_BITS            24    /* 24-bit addressing */
#define QSPI_RW_CHUNK_MAX_BYTES     (64U * 1024U)
#define QSPI_VERIFY_SCRATCH_BYTES   256U
/* USER CODE END Includes */

extern QSPI_HandleTypeDef hqspi;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_QUADSPI_Init(void);

/* USER CODE BEGIN Prototypes */
uint32_t QSPI_ReadJEDEC_ID(uint8_t *mid, uint8_t *mem_type, uint8_t *capacity);
uint8_t  QSPI_CheckFlashSize(void); /* returns 0 on OK, non-zero on mismatch */
uint32_t QSPI_ReadDeviceID_90(uint8_t *mid, uint8_t *devid);
uint8_t  QSPI_IsMemoryMapped(void); /* returns 1 if memory-mapped entered */
uint8_t  QSPI_EnableMemoryMappedFast(void); /* force re-enter fast (quad/standard) memory-mapped if disabled at build */
uint8_t  QSPI_AppHeaderValid(const qspi_app_header_t *hdr); /* validate header when enabled */
uint8_t  QSPI_ProgramPatternIfBlank(void); /* optional pattern writer */
uint8_t  QSPI_LastCheckStatus(void); /* result code from last init check */
uint8_t  QSPI_EraseRange(uint32_t address, uint32_t length); /* erase contiguous sectors */
uint8_t  QSPI_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t length); /* program data in pages */
uint8_t  QSPI_ReadBuffer(uint32_t address, uint8_t *data, uint32_t length); /* read data through command mode */
uint32_t QSPI_CalculateCRC32(const uint8_t *data, uint32_t length); /* software CRC32 */
uint8_t  QSPI_VerifyCRC32(uint32_t address, uint32_t length, uint32_t expected_crc, uint32_t *actual_crc);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */
