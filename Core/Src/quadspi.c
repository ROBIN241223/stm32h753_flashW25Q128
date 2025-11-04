/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.c
  * @brief   This file provides code for the configuration
  *          of the QUADSPI instances.
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
/* Includes ------------------------------------------------------------------*/
#include "quadspi.h"

/* USER CODE BEGIN 0 */
static uint8_t QSPI_WriteEnable(void);
uint8_t QSPI_AutoPollingMemReady(void);
static uint8_t QSPI_EnableMemoryMapped(void);
static uint8_t QSPI_EraseSector(uint32_t address);
static uint8_t QSPI_PageProgram(uint32_t address, const uint8_t *data, uint32_t length);
void QSPI_FlashDemo(void); /* public demo (could be moved to user code) */
void QSPI_IndirectModeDemo(void); /* public demo using Indirect Mode only */
static uint8_t QSPI_SetQEBit(void);
static uint8_t QSPI_EnableMemoryMappedQuad(void);
static uint8_t QSPI_ReadStatusReg(uint8_t *sr1);
static uint8_t QSPI_ReadStatusReg2(uint8_t *sr2); /* some Winbond parts have SR2 QE bit */
static uint8_t QSPI_CheckRange(uint32_t address, uint32_t length);
static uint8_t QSPI_ReadChunk(uint32_t address, uint8_t *data, uint32_t length);
static uint8_t QSPI_EnsureQuadReady(void);
static void     QSPI_CRC32_EnsureTable(void);
static uint32_t QSPI_CRC32_Update(uint32_t crc, const uint8_t *data, uint32_t length);
/* Track whether we entered memory-mapped mode and capture first word for debugging */
static volatile uint8_t qspi_memory_mapped_entered = 0;
static volatile uint32_t qspi_first_word = 0;
static uint8_t qspi_qe_set = 0;
static uint32_t qspi_crc32_table[256];
static uint8_t qspi_crc32_table_ready = 0;
static volatile uint8_t qspi_last_check_status = 0;

QSPI_HandleTypeDef hqspi;
MDMA_HandleTypeDef hmdma_quadspi_fifo_th;

/* QUADSPI init function */
void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 2;
  hqspi.Init.FifoThreshold = 32;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 23;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */
  /* Optional: verify external flash JEDEC ID & size */
  qspi_last_check_status = QSPI_CheckFlashSize();
  if (qspi_last_check_status != 0U) {
    /* Size mismatch or ID read failure */
    Error_Handler();
  }
#if QSPI_ENABLE_QUAD
  if (QSPI_EnsureQuadReady() != 0) {
    Error_Handler();
  }
#endif
#if QSPI_ENABLE_XIP
  /* Optional early pattern fill (only small region) */
#if QSPI_ENABLE_PATTERN_FILL
  QSPI_ProgramPatternIfBlank();
#endif
  if (
#if QSPI_ENABLE_QUAD
      QSPI_EnableMemoryMappedQuad() != 0
#else
      QSPI_EnableMemoryMapped() != 0
#endif
  ) {
    Error_Handler();
  }
  else {
    qspi_memory_mapped_entered = 1;
    qspi_first_word = *(uint32_t*)0x90000000UL; /* capture for debugger */
  }
#endif

#if QSPI_ENABLE_DEMO
  QSPI_FlashDemo();
#endif

  /* USER CODE END QUADSPI_Init 2 */

}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PG6     ------> QUADSPI_BK1_NCS
    PF10     ------> QUADSPI_CLK
    PD13     ------> QUADSPI_BK1_IO3
    PD12     ------> QUADSPI_BK1_IO1
    PD11     ------> QUADSPI_BK1_IO0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_12|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* QUADSPI MDMA Init */
    /* QUADSPI_FIFO_TH Init */
    hmdma_quadspi_fifo_th.Instance = MDMA_Channel0;
    hmdma_quadspi_fifo_th.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
    hmdma_quadspi_fifo_th.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_quadspi_fifo_th.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_quadspi_fifo_th.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_quadspi_fifo_th.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_quadspi_fifo_th.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    hmdma_quadspi_fifo_th.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_quadspi_fifo_th.Init.BufferTransferLength = 32;
    hmdma_quadspi_fifo_th.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.SourceBlockAddressOffset = 0;
    hmdma_quadspi_fifo_th.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_quadspi_fifo_th) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_MDMA_ConfigPostRequestMask(&hmdma_quadspi_fifo_th, 0, 0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(qspiHandle,hmdma,hmdma_quadspi_fifo_th);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PG6     ------> QUADSPI_BK1_NCS
    PF10     ------> QUADSPI_CLK
    PD13     ------> QUADSPI_BK1_IO3
    PD12     ------> QUADSPI_BK1_IO1
    PD11     ------> QUADSPI_BK1_IO0
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_13|GPIO_PIN_12|GPIO_PIN_11);

    /* QUADSPI MDMA DeInit */
    HAL_MDMA_DeInit(qspiHandle->hmdma);
  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/* Low-level helpers ------------------------------------------------------- */
static uint8_t QSPI_WaitWhileBusy(uint32_t timeout)
{
  QSPI_CommandTypeDef sCommand = {0};
  QSPI_AutoPollingTypeDef sConfig = {0};
  sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction = W25Q128_CMD_READ_STATUS_REG;
  sCommand.AddressMode = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = QSPI_DATA_1_LINE;
  sCommand.DummyCycles = 0;
  sConfig.Match = 0x00; /* WIP=0 */
  sConfig.Mask = 0x01;  /* WIP bit */
  sConfig.MatchMode = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval = 0x10;
  sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
  if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, timeout) != HAL_OK)
    return 1;
  return 0;
}

static uint8_t QSPI_WriteEnable(void)
{
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = W25Q128_CMD_WRITE_ENABLE;
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_NONE;
  cmd.DummyCycles = 0;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 1;
  return 0;
}

static uint8_t QSPI_CheckRange(uint32_t address, uint32_t length)
{
  if (length == 0) return 0;
  if (address >= MEMORY_FLASH_SIZE) return 1;
  if (length > (MEMORY_FLASH_SIZE - address)) return 1;
  return 0;
}

static uint8_t QSPI_EnsureQuadReady(void)
{
#if !QSPI_ENABLE_QUAD
  return 0;
#else
  if (!qspi_qe_set) {
    if (QSPI_SetQEBit()) return 1;
  }
  return 0;
#endif
}

static uint8_t QSPI_ReadChunk(uint32_t address, uint8_t *data, uint32_t length)
{
  if (length == 0) return 0;
  if (!data) return 1;
#if QSPI_ENABLE_QUAD
  if (QSPI_EnsureQuadReady()) return 2;
#endif
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
#if QSPI_ENABLE_QUAD
  cmd.Instruction = 0x6B; /* Fast Read Quad Output */
  cmd.DataMode = QSPI_DATA_4_LINES;
  cmd.DummyCycles = QSPI_QUAD_DUMMY_CYCLES;
#else
  cmd.Instruction = W25Q128_CMD_READ_DATA;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.DummyCycles = 0;
#endif
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_24_BITS;
  cmd.Address = address;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
  cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  cmd.NbData = length;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 3;
  if (HAL_QSPI_Receive(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 4;
  return 0;
}

static void QSPI_CRC32_EnsureTable(void)
{
  if (qspi_crc32_table_ready) return;
  for (uint32_t i = 0; i < 256U; ++i) {
    uint32_t crc = i;
    for (uint32_t bit = 0; bit < 8U; ++bit) {
      if (crc & 1U) crc = (crc >> 1) ^ 0xEDB88320UL;
      else crc >>= 1;
    }
    qspi_crc32_table[i] = crc;
  }
  qspi_crc32_table_ready = 1;
}

static uint32_t QSPI_CRC32_Update(uint32_t crc, const uint8_t *data, uint32_t length)
{
  if (!data || length == 0) return crc;
  for (uint32_t i = 0; i < length; ++i) {
    uint8_t idx = (uint8_t)((crc ^ data[i]) & 0xFFU);
    crc = (crc >> 8) ^ qspi_crc32_table[idx];
  }
  return crc;
}

static uint8_t QSPI_EraseSector(uint32_t address)
{
  if (QSPI_WriteEnable()) return 1;
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = W25Q128_CMD_SECTOR_ERASE;
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_24_BITS;
  cmd.Address = address;
  cmd.DataMode = QSPI_DATA_NONE;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 2;
  if (QSPI_WaitWhileBusy(5000)) return 3;
  return 0;
}

static uint8_t QSPI_PageProgram(uint32_t address, const uint8_t *data, uint32_t length)
{
  if (length == 0 || length > MEMORY_PAGE_SIZE) return 1;
  if (QSPI_WriteEnable()) return 2;
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = W25Q128_CMD_PAGE_PROGRAM;
  cmd.AddressMode = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize = QSPI_ADDRESS_24_BITS;
  cmd.Address = address;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = length;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 3;
  if (HAL_QSPI_Transmit(&hqspi, (uint8_t*)data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 4;
  if (QSPI_WaitWhileBusy(100)) return 5;
  return 0;
}

static __attribute__((unused)) uint8_t QSPI_EnableMemoryMapped(void)
{
  QSPI_CommandTypeDef sCommand = {0};
  QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};
  /* Use standard READ (0x03) first; can switch to fast quad read later. */
  sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction = W25Q128_CMD_READ_DATA;
  sCommand.AddressMode = QSPI_ADDRESS_1_LINE;
  sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = QSPI_DATA_1_LINE;
  sCommand.DummyCycles = 0;
  sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;

  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) {
    return 1;
  }
  return 0;
}

static uint8_t QSPI_EnableMemoryMappedQuad(void)
{
#if QSPI_ENABLE_QUAD
  /* Ensure QE bit set (Winbond: Status Reg 2 bit 1) */
  if (QSPI_EnsureQuadReady()) return 1;
  QSPI_CommandTypeDef sCommand = {0};
  QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};
  /* Fast Read Quad Output 0x6B: Instruction(1) + Addr(3) + Dummy + Data(4 lines) */
  sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction = 0x6B; /* FAST_READ_QUAD_OUTPUT */
  sCommand.AddressMode = QSPI_ADDRESS_1_LINE; /* address still single line for this mode */
  sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = QSPI_DATA_4_LINES;
  sCommand.DummyCycles = QSPI_QUAD_DUMMY_CYCLES;
  sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
  sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) return 2;
  return 0;
#else
  return QSPI_EnableMemoryMapped();
#endif
}

static __attribute__((unused)) uint8_t QSPI_ReadStatusReg(uint8_t *sr1)
{
  QSPI_CommandTypeDef cmd = {0};
  uint8_t val=0;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = W25Q128_CMD_READ_STATUS_REG; /* 0x05 */
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 1;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 1;
  if (HAL_QSPI_Receive(&hqspi, &val, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 2;
  if (sr1) *sr1 = val;
  return 0;
}

static uint8_t QSPI_ReadStatusReg2(uint8_t *sr2)
{
  /* Winbond SR2 is 0x35 read */
  QSPI_CommandTypeDef cmd = {0};
  uint8_t val=0;
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = 0x35; /* Read Status Register-2 */
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 1;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 1;
  if (HAL_QSPI_Receive(&hqspi, &val, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 2;
  if (sr2) *sr2 = val;
  return 0;
}

static uint8_t QSPI_WriteStatusReg2(uint8_t sr2)
{
  if (QSPI_WriteEnable()) return 1;
  QSPI_CommandTypeDef cmd = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction = 0x31; /* Write Status Register-2 */
  cmd.AddressMode = QSPI_ADDRESS_NONE;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode = QSPI_DATA_1_LINE;
  cmd.NbData = 1;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 2;
  if (HAL_QSPI_Transmit(&hqspi, &sr2, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 3;
  if (QSPI_WaitWhileBusy(100)) return 4;
  return 0;
}

static uint8_t QSPI_SetQEBit(void)
{
#if !QSPI_ENABLE_QUAD
  return 0;
#else
  uint8_t sr2;
  if (QSPI_ReadStatusReg2(&sr2)) return 1;
  if ((sr2 & (1U<<1)) == 0U) {
    sr2 |= (1U<<1);
    if (QSPI_WriteStatusReg2(sr2)) return 2;
    if (QSPI_ReadStatusReg2(&sr2)) return 3;
    if ((sr2 & (1U<<1)) == 0U) return 4;
  }
  qspi_qe_set = 1;
  return 0;
#endif
}

void QSPI_FlashDemo(void)
{
  /* Demo: erase first sector (0x000000), program one page, read-back verify. */
  static const uint32_t test_addr = QSPI_DEMO_SECTOR_ADDR;
  static uint8_t page_tx[MEMORY_PAGE_SIZE];
  static uint8_t page_rx[MEMORY_PAGE_SIZE];
  for (uint32_t i = 0; i < MEMORY_PAGE_SIZE; ++i) page_tx[i] = (uint8_t)(i ^ 0x5A);
  if (QSPI_EraseSector(test_addr) != 0) return; /* silently abort demo */
  if (QSPI_PageProgram(test_addr, page_tx, MEMORY_PAGE_SIZE) != 0) return;

  /* Exit memory mapped? Not needed; we can directly memcpy from mapped region 0x90000000 + test_addr */
  uint8_t *mapped = (uint8_t*)(0x90000000UL + test_addr);
  for (uint32_t i = 0; i < MEMORY_PAGE_SIZE; ++i) page_rx[i] = mapped[i];
  for (uint32_t i = 0; i < MEMORY_PAGE_SIZE; ++i) {
    if (page_rx[i] != page_tx[i]) {
      /* Verification failed: handle as needed (toggle LED, etc.) */
      return;
    }
  }
  /* If reached here: demo success */
}
uint32_t QSPI_ReadJEDEC_ID(uint8_t *mid, uint8_t *mem_type, uint8_t *capacity)
{
  QSPI_CommandTypeDef sCommand = {0};
  uint8_t id[3] = {0};

  sCommand.Instruction = W25Q128_CMD_JEDEC_ID; /* JEDEC ID (Manufacturer, Memory Type, Capacity) */
  sCommand.AddressMode = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = QSPI_DATA_1_LINE;
  sCommand.DummyCycles = 0;
  sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
  sCommand.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  sCommand.NbData = 3;

  if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 1;
  if (HAL_QSPI_Receive(&hqspi, id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 2;
  if (mid) *mid = id[0];
  if (mem_type) *mem_type = id[1];
  if (capacity) *capacity = id[2];
  return 0;
}

uint8_t QSPI_CheckFlashSize(void)
{
  uint8_t mid, mtype, cap;
  if (QSPI_ReadJEDEC_ID(&mid, &mtype, &cap)) return 1; /* read fail */
  if (mid != W25Q_MANUFACTURER_ID) return 2; /* wrong manufacturer */
  if (!((mtype == W25Q128_MEM_TYPE_40) || (mtype == W25Q128_MEM_TYPE_60))) return 3; /* unexpected mem type */
  if (cap != W25Q128_CAPACITY_ID) return 4; /* not 128Mbit */

  /* Compute size from capacity code per Winbond formula: size = 1 << cap when cap >= 0x15 (0x18 -> 2^(0x18-0x11+?) )
     Simpler: For common parts we can derive directly: 0x18 => 128Mbit => 16MB. */
  if (MEMORY_FLASH_SIZE != 0x01000000UL) return 5; /* our macro inconsistent */
  if (hqspi.Init.FlashSize != (W25Q128_ADDRESS_BITS - 1)) return 6; /* controller config mismatch */
  return 0; /* OK */
}

uint8_t QSPI_IsMemoryMapped(void)
{
  return qspi_memory_mapped_entered;
}

uint8_t QSPI_EnableMemoryMappedFast(void)
{
  /* Re-enter mapping (useful if user disabled at build) */
  if (qspi_memory_mapped_entered) return 0;
#if QSPI_ENABLE_QUAD
  if (QSPI_EnableMemoryMappedQuad()) return 1;
#else
  if (QSPI_EnableMemoryMapped()) return 1;
#endif
  qspi_memory_mapped_entered = 1;
  qspi_first_word = *(uint32_t*)0x90000000UL;
  return 0;
}

uint8_t QSPI_ProgramPatternIfBlank(void)
{
#if !QSPI_ENABLE_PATTERN_FILL
  return 0;
#else
  /* Simple heuristic: read first 16 bytes; if all 0xFF, assume blank and program pattern length */
  uint8_t *mapped = (uint8_t*)0x90000000UL;
  int blank = 1;
  for (int i=0;i<16;i++) if (mapped[i] != 0xFF) { blank = 0; break; }
  if (!blank) return 0;
  uint32_t remaining = QSPI_PATTERN_LEN_BYTES;
  if (remaining == 0) return 0;
  if (remaining > MEMORY_FLASH_SIZE) remaining = MEMORY_FLASH_SIZE;
  /* Pattern: byte = (addr & 0xFF) ^ 0xA5 */
  uint8_t page[MEMORY_PAGE_SIZE];
  uint32_t addr = 0;
  while (remaining)
  {
    uint32_t chunk = (remaining > MEMORY_PAGE_SIZE) ? MEMORY_PAGE_SIZE : remaining;
    for (uint32_t i=0;i<chunk;i++) page[i] = (uint8_t)(((addr+i)&0xFF) ^ 0xA5);
    if (QSPI_EraseSector(addr & ~(MEMORY_SECTOR_SIZE-1))) return 1; /* naive: erase each sector when first page hits it */
    if (QSPI_PageProgram(addr, page, chunk)) return 2;
    addr += chunk;
    remaining -= chunk;
  }
  return 0;
#endif
}

uint8_t QSPI_AppHeaderValid(const qspi_app_header_t *hdr)
{
#if !QSPI_ENABLE_APP_HEADER_CHECK
  (void)hdr; return 1; /* treat as valid if feature disabled */
#else
  if (!hdr) return 0;
  if (hdr->magic != QSPI_APP_MAGIC) return 0;
  if (hdr->length == 0 || hdr->length > (MEMORY_FLASH_SIZE - sizeof(qspi_app_header_t))) return 0;
  /* CRC check left to boot stub (reading over mapped region) - here we only do structural check */
  return 1;
#endif
}

uint32_t QSPI_ReadDeviceID_90(uint8_t *mid, uint8_t *devid)
{
  QSPI_CommandTypeDef cmd = {0};
  uint8_t rx[2] = {0};
  cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  cmd.Instruction     = W25Q128_CMD_DEVICE_ID; /* 0x90 */
  cmd.AddressMode     = QSPI_ADDRESS_1_LINE;
  cmd.AddressSize     = QSPI_ADDRESS_24_BITS;
  cmd.Address         = 0x000000; /* dummy address */
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode        = QSPI_DATA_1_LINE;
  cmd.DummyCycles     = 0;
  cmd.NbData          = 2;
  if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 1;
  if (HAL_QSPI_Receive(&hqspi, rx, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    return 2;
  if (mid)   *mid   = rx[0];
  if (devid) *devid = rx[1];
  return 0;
}

uint8_t QSPI_EraseRange(uint32_t address, uint32_t length)
{
  if (length == 0) return 0;
  if ((address % MEMORY_SECTOR_SIZE) != 0U) return 1;
  if ((length % MEMORY_SECTOR_SIZE) != 0U) return 2;
  if (QSPI_CheckRange(address, length)) return 3;
  for (uint32_t offset = 0; offset < length; offset += MEMORY_SECTOR_SIZE) {
    if (QSPI_EraseSector(address + offset)) return 4;
  }
  return 0;
}

uint8_t QSPI_WriteBuffer(uint32_t address, const uint8_t *data, uint32_t length)
{
  if (length == 0) return 0;
  if (!data) return 1;
  if (QSPI_CheckRange(address, length)) return 2;
  uint32_t remaining = length;
  uint32_t current = address;
  const uint8_t *cursor = data;
  while (remaining) {
    uint32_t page_offset = current % MEMORY_PAGE_SIZE;
    uint32_t chunk = MEMORY_PAGE_SIZE - page_offset;
    if (chunk > remaining) chunk = remaining;
    if (QSPI_PageProgram(current, cursor, chunk)) return 3;
    current += chunk;
    cursor += chunk;
    remaining -= chunk;
  }
  return 0;
}

uint8_t QSPI_ReadBuffer(uint32_t address, uint8_t *data, uint32_t length)
{
  if (length == 0) return 0;
  if (!data) return 1;
  if (QSPI_CheckRange(address, length)) return 2;
  uint32_t remaining = length;
  uint32_t current = address;
  uint8_t *cursor = data;
  while (remaining) {
    uint32_t chunk = (remaining > QSPI_RW_CHUNK_MAX_BYTES) ? QSPI_RW_CHUNK_MAX_BYTES : remaining;
    if (QSPI_ReadChunk(current, cursor, chunk)) return 3;
    current += chunk;
    cursor += chunk;
    remaining -= chunk;
  }
  return 0;
}

uint32_t QSPI_CalculateCRC32(const uint8_t *data, uint32_t length)
{
  if (length == 0) return 0x00000000UL;
  if (!data) return 0x00000000UL;
  QSPI_CRC32_EnsureTable();
  uint32_t crc = 0xFFFFFFFFUL;
  crc = QSPI_CRC32_Update(crc, data, length);
  return crc ^ 0xFFFFFFFFUL;
}

uint8_t QSPI_VerifyCRC32(uint32_t address, uint32_t length, uint32_t expected_crc, uint32_t *actual_crc)
{
  if (length == 0) {
    if (actual_crc) *actual_crc = 0x00000000UL;
    return (expected_crc == 0x00000000UL) ? 0 : 1;
  }
  if (QSPI_CheckRange(address, length)) return 2;
  QSPI_CRC32_EnsureTable();
  uint32_t crc = 0xFFFFFFFFUL;
  uint32_t remaining = length;
  uint32_t current = address;
  uint8_t scratch[QSPI_VERIFY_SCRATCH_BYTES];
  while (remaining) {
    uint32_t chunk = (remaining > (uint32_t)sizeof(scratch)) ? (uint32_t)sizeof(scratch) : remaining;
    if (QSPI_ReadChunk(current, scratch, chunk)) return 3;
    crc = QSPI_CRC32_Update(crc, scratch, chunk);
    current += chunk;
    remaining -= chunk;
  }
  crc ^= 0xFFFFFFFFUL;
  if (actual_crc) *actual_crc = crc;
  return (crc == expected_crc) ? 0 : 4;
}

uint8_t QSPI_LastCheckStatus(void)
{
  return qspi_last_check_status;
}

/* Demo function using Indirect Mode only (no Memory Mapped) */
/* This demonstrates basic QSPI operations using Indirect Mode as described */
void QSPI_IndirectModeDemo(void)
{
  /* Example: Read JEDEC ID using Indirect Mode */
  uint8_t mid, mtype, cap;
  if (QSPI_ReadJEDEC_ID(&mid, &mtype, &cap) == 0) {
    /* Successfully read JEDEC ID: mid=0xEF for Winbond, mtype=0x40/0x60, cap=0x18 for 128Mbit */
    /* You can log or use these values for verification */
  } else {
    /* JEDEC ID read failed - check QSPI connection */
    return;
  }

  /* Example: Write Enable (WREN) - prerequisite for write/erase */
  if (QSPI_WriteEnable() != 0) {
    /* Write Enable failed */
    return;
  }

  /* Example: Erase a sector (4KB) at address 0x000000 */
  if (QSPI_EraseSector(0x000000) != 0) {
    /* Sector erase failed */
    return;
  }

  /* Example: Program a page (256 bytes) with test data */
  uint8_t test_data[MEMORY_PAGE_SIZE];
  for (uint32_t i = 0; i < MEMORY_PAGE_SIZE; ++i) {
    test_data[i] = (uint8_t)(i & 0xFF); /* Simple pattern */
  }
  if (QSPI_PageProgram(0x000000, test_data, MEMORY_PAGE_SIZE) != 0) {
    /* Page program failed */
    return;
  }

  /* Example: Read back the data using Indirect Mode */
  uint8_t read_data[MEMORY_PAGE_SIZE];
  if (QSPI_ReadChunk(0x000000, read_data, MEMORY_PAGE_SIZE) != 0) {
    /* Read failed */
    return;
  }

  /* Verify the data */
  for (uint32_t i = 0; i < MEMORY_PAGE_SIZE; ++i) {
    if (read_data[i] != test_data[i]) {
      /* Verification failed */
      return;
    }
  }

  /* Example: Read Status Register to check if busy */
  uint8_t sr1;
  if (QSPI_ReadStatusReg(&sr1) == 0) {
    if (sr1 & 0x01) {
      /* Flash is busy - wait or handle */
    }
  }

  /* Demo completed successfully */
}

/* USER CODE END 1 */
