#ifndef __SWO_H__
#define __SWO_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/* Initialize SWO (ITM+TPIU) for NRZ output on TRACESWO pin.
 * cpu_hz: current SystemCoreClock
 * swo_baud: desired SWO baud (e.g., 2,000,000)
 */
void SWO_Init(uint32_t cpu_hz, uint32_t swo_baud);

/* Debugger connection detection via DHCSR.C_DEBUGEN */
bool Debug_IsConnected(void);

/* Low-level write to ITM port 0 (non-blocking). Returns ch or -1 if not ready. */
int SWO_WriteChar(int ch);

/* Convenience printers */
void SWO_Print(const char *s);
int  SWO_Printf(const char *fmt, ...);

/* Retarget for newlib _write via syscalls.c */
int __io_putchar(int ch);

#endif /* __SWO_H__ */
