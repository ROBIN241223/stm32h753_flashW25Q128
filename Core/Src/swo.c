#include "swo.h"
#include "stm32h7xx.h"
#include <stdio.h>

bool Debug_IsConnected(void)
{
  return (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0;
}

void SWO_Init(uint32_t cpu_hz, uint32_t swo_baud)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  ITM->LAR = 0xC5ACCE55;
  DWT->LAR = 0xC5ACCE55;

  ITM->TCR = 0;
  ITM->TER = 0;

  uint32_t acpr = (swo_baud == 0U) ? 0U : ((cpu_hz / swo_baud) - 1U);
  TPI->ACPR = acpr;

  TPI->SPPR = 0x00000002U;
  TPI->FFCR = 0x00000100U;

  DWT->CTRL |= 1U;

  ITM->TPR = 0U;
  ITM->TCR = ITM_TCR_ITMENA_Msk |
         ITM_TCR_TSENA_Msk  |
         ITM_TCR_SWOENA_Msk |
         ITM_TCR_SYNCENA_Msk |
         (1U << ITM_TCR_TraceBusID_Pos);

  ITM->TER = 1U;
}

int SWO_WriteChar(int ch)
{
  if ((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0U) return -1;
  if ((ITM->TER & 1U) == 0U) return -1;
  ITM->PORT[0].u8 = (uint8_t)ch;
  return ch;
}

void SWO_Print(const char *s)
{
  if (!s) return;
  while (*s) {
    SWO_WriteChar((uint8_t)*s++);
  }
}

int SWO_Printf(const char *fmt, ...)
{
  char buf[256];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  if (n > 0) {
    if (n > (int)sizeof(buf)) {
      n = sizeof(buf);
    }
    for (int i = 0; i < n; ++i) {
      SWO_WriteChar(buf[i]);
    }
  }
  return n;
}

int __io_putchar(int ch)
{
  if (Debug_IsConnected()) {
    SWO_WriteChar(ch);
  }
  return ch;
}
