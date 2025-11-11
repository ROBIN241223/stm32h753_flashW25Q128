/*
 * Custom CLI Commands for STM32H753 + W25Q128 Flash
 */

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>

/* Command: flash-info */
static BaseType_t prvFlashInfoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    
    snprintf(pcWriteBuffer, xWriteBufferLen,
        "W25Q128 Flash Memory Information:\r\n"
        "  Type:       W25Q128JV\r\n"
        "  Size:       16 MB (128 Mbit)\r\n"
        "  Base Addr:  0x90000000\r\n"
        "  Sectors:    256 x 64KB\r\n"
        "  Blocks:     2048 x 4KB\r\n"
        "  Pages:      65536 x 256B\r\n"
        "  Interface:  QSPI (Quad SPI)\r\n"
        "  Mode:       Memory-Mapped XIP\r\n\r\n");
    
    return pdFALSE;
}

static const CLI_Command_Definition_t xFlashInfoCommand =
{
    "flash-info",
    "\r\nflash-info:\r\n Shows W25Q128 flash memory information\r\n\r\n",
    prvFlashInfoCommand,
    0
};

/* Command: reboot */
static BaseType_t prvRebootCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    
    strncpy(pcWriteBuffer, "Rebooting system...\r\n", xWriteBufferLen);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    NVIC_SystemReset();
    
    return pdFALSE;
}

static const CLI_Command_Definition_t xRebootCommand =
{
    "reboot",
    "\r\nreboot:\r\n Reboot the MCU\r\n\r\n",
    prvRebootCommand,
    0
};

/* Command: version */
static BaseType_t prvVersionCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    
    snprintf(pcWriteBuffer, xWriteBufferLen,
        "STM32H753 Firmware\r\n"
        "  MCU:        STM32H753IIK6\r\n"
        "  Core:       ARM Cortex-M7\r\n"
        "  Frequency:  480 MHz\r\n"
        "  FreeRTOS:   V10.x\r\n"
        "  Build Date: " __DATE__ " " __TIME__ "\r\n\r\n");
    
    return pdFALSE;
}

static const CLI_Command_Definition_t xVersionCommand =
{
    "version",
    "\r\nversion:\r\n Shows firmware version information\r\n\r\n",
    prvVersionCommand,
    0
};

/* Command: uptime */
static BaseType_t prvUptimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    
    TickType_t xUptime = xTaskGetTickCount();
    uint32_t ulSeconds = xUptime / configTICK_RATE_HZ;
    uint32_t ulMinutes = ulSeconds / 60;
    uint32_t ulHours = ulMinutes / 60;
    uint32_t ulDays = ulHours / 24;
    
    snprintf(pcWriteBuffer, xWriteBufferLen,
        "System Uptime:\r\n"
        "  %lu days, %lu hours, %lu minutes, %lu seconds\r\n"
        "  Total ticks: %lu\r\n\r\n",
        ulDays,
        ulHours % 24,
        ulMinutes % 60,
        ulSeconds % 60,
        (unsigned long)xUptime);
    
    return pdFALSE;
}

static const CLI_Command_Definition_t xUptimeCommand =
{
    "uptime",
    "\r\nuptime:\r\n Shows system uptime\r\n\r\n",
    prvUptimeCommand,
    0
};

/* Command: clear */
static BaseType_t prvClearCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    
    /* ANSI escape code to clear screen and move cursor to home */
    strncpy(pcWriteBuffer, "\033[2J\033[H", xWriteBufferLen);
    
    return pdFALSE;
}

static const CLI_Command_Definition_t xClearCommand =
{
    "clear",
    "\r\nclear:\r\n Clear the screen\r\n\r\n",
    prvClearCommand,
    0
};

/* Register custom commands */
void vRegisterCustomCLICommands(void)
{
    FreeRTOS_CLIRegisterCommand(&xFlashInfoCommand);
    FreeRTOS_CLIRegisterCommand(&xRebootCommand);
    FreeRTOS_CLIRegisterCommand(&xVersionCommand);
    FreeRTOS_CLIRegisterCommand(&xUptimeCommand);
    FreeRTOS_CLIRegisterCommand(&xClearCommand);
}
