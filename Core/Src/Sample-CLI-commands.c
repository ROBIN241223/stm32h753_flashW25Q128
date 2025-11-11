/*
 * FreeRTOS+CLI Sample Commands
 * Customized for STM32H753 + W25Q128 Flash
 */

#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

#include <string.h>
#include <stdio.h>

#ifndef configINCLUDE_TRACE_RELATED_CLI_COMMANDS
    #define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
    #define configINCLUDE_QUERY_HEAP_COMMAND 1
#endif

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the run-time-stats command.
 */
#if( configGENERATE_RUN_TIME_STATS == 1 )
    static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif /* configGENERATE_RUN_TIME_STATS */

/*
 * Implements the query-heap command.
 */
#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
    static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

/* Structure that defines the "task-stats" command line command. */
static const CLI_Command_Definition_t xTaskStats =
{
    "task-stats", /* The command string to type. */
    "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n\r\n",
    prvTaskStatsCommand, /* The function to run. */
    0 /* No parameters are expected. */
};

#if( configGENERATE_RUN_TIME_STATS == 1 )
    /* Structure that defines the "run-time-stats" command line command. */
    static const CLI_Command_Definition_t xRunTimeStats =
    {
        "run-time-stats", /* The command string to type. */
        "\r\nrun-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n\r\n",
        prvRunTimeStatsCommand, /* The function to run. */
        0 /* No parameters are expected. */
    };
#endif /* configGENERATE_RUN_TIME_STATS */

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
    /* Structure that defines the "query-heap" command line command. */
    static const CLI_Command_Definition_t xQueryHeap =
    {
        "query-heap",
        "\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n\r\n",
        prvQueryHeapCommand, /* The function to run. */
        0 /* The user can enter any number of commands. */
    };
#endif /* configINCLUDE_QUERY_HEAP_COMMAND */

/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands( void )
{
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &xTaskStats );

    #if( configGENERATE_RUN_TIME_STATS == 1 )
    {
        FreeRTOS_CLIRegisterCommand( &xRunTimeStats );
    }
    #endif

    #if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
    {
        FreeRTOS_CLIRegisterCommand( &xQueryHeap );
    }
    #endif
}
/*-----------------------------------------------------------*/

static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *const pcHeader = "Task          State  Priority  Stack  Num\r\n*******************************************\r\n";

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Generate a table of task stats. */
    strcpy( pcWriteBuffer, pcHeader );
    vTaskList( pcWriteBuffer + strlen( pcHeader ) );

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )

    static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
    {
        /* Remove compile time warnings about unused parameters, and check the
        write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        write buffer length is adequate, so does not check for buffer overflows. */
        ( void ) pcCommandString;
        ( void ) xWriteBufferLen;
        configASSERT( pcWriteBuffer );

        sprintf( pcWriteBuffer, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

        /* There is no more data to return after this single string, so return
        pdFALSE. */
        return pdFALSE;
    }

#endif /* configINCLUDE_QUERY_HEAP */
/*-----------------------------------------------------------*/

#if( configGENERATE_RUN_TIME_STATS == 1 )

    static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
    {
    const char *const pcHeader = "Task            Abs Time      % Time\r\n****************************************\r\n";

        /* Remove compile time warnings about unused parameters, and check the
        write buffer is not NULL.  NOTE - for simplicity, this example assumes the
        write buffer length is adequate, so does not check for buffer overflows. */
        ( void ) pcCommandString;
        ( void ) xWriteBufferLen;
        configASSERT( pcWriteBuffer );

        /* Generate a table of task stats. */
        strcpy( pcWriteBuffer, pcHeader );
        vTaskGetRunTimeStats( pcWriteBuffer + strlen( pcHeader ) );

        /* There is no more data to return after this single string, so return
        pdFALSE. */
        return pdFALSE;
    }

#endif /* configGENERATE_RUN_TIME_STATS */
