#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H 

#include "FreeRTOS.h"

#define configCOMMAND_INT_MAX_OUTPUT_SIZE 512

typedef BaseType_t (*pdCOMMAND_LINE_CALLBACK)( char * pcWriteBuffer, size_t xWriteBufferLen, const char * pcCommandString );

typedef struct xCOMMAND_LINE_INPUT
{
    const char * const pcCommand;
    const char * const pcHelpString;
    const pdCOMMAND_LINE_CALLBACK pxCommandInterpreter;
    int8_t cExpectedNumberOfParameters;
} CLI_Command_Definition_t;

#define xCommandLineInput CLI_Command_Definition_t

BaseType_t FreeRTOS_CLIRegisterCommand( const CLI_Command_Definition_t * const pxCommandToRegister );

BaseType_t FreeRTOS_CLIProcessCommand( const char * const pcCommandInput, char * pcWriteBuffer, size_t xWriteBufferLen );

char * FreeRTOS_CLIGetOutputBuffer( void );

const char * FreeRTOS_CLIGetParameter( const char * pcCommandString, UBaseType_t uxWantedParameter, BaseType_t * pxParameterStringLength );

extern void vUARTCommandConsoleStart( uint16_t usStackSize, UBaseType_t uxPriority );
extern void vRegisterSampleCLICommands( void );
extern void vRegisterCustomCLICommands( void );

#endif
