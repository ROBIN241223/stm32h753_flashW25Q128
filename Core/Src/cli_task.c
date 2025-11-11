/*
 * FreeRTOS CLI Task - Console Interface
 * STM32H753 + W25Q128 Flash
 * 
 * NOTE: UART is not yet configured in this project.
 * CLI commands are registered but task just sleeps.
 * To enable console, configure UART in CubeMX first.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include "main.h"
#include <string.h>

#define CLI_COMMAND_MAX_INPUT_SIZE   128
#define CLI_COMMAND_MAX_OUTPUT_SIZE  512

/* Forward declarations */
extern void vRegisterSampleCLICommands(void);
extern void vRegisterCustomCLICommands(void);

void vCLITask(void *pvParameters)
{
    (void)pvParameters;
    
    /* Register all CLI commands */
    vRegisterSampleCLICommands();
    vRegisterCustomCLICommands();
    
    /* CLI infrastructure is ready */
    /* Task sleeps until UART is configured */
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
