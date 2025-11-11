/* Boot stub to run from internal FLASH and jump to external QSPI mapped application.
 * Build this with linker script STM32H753IIKX_INTFLASH.ld and define INTERNAL_BOOT_STUB.
 */

#ifdef INTERNAL_BOOT_STUB

#include "main.h"
#include "quadspi.h"
#include "gpio.h"

/* Minimal prototypes (original versions live in main.c for external app). For the stub we can reuse the
 * generated full SystemClock_Config by providing an extern if we link main.c, but we exclude main.c to
 * avoid duplicate symbols. Instead implement a trimmed clock setup adequate for QSPI and core run. */

void Error_Handler(void);
void SystemClock_Config(void);

#ifndef EXT_APP_BASE
#define EXT_APP_BASE 0x90000000UL
#endif

typedef void (*AppEntry_t)(void);

static int QSPI_AppVectorValid(void)
{
    uint32_t sp = *(uint32_t*)EXT_APP_BASE;
    /* Basic stack pointer range check: SRAM regions start at 0x20000000 / 0x24000000 etc */
    if ((sp & 0xFF000000UL) != 0x20000000UL && (sp & 0xFF000000UL) != 0x24000000UL)
        return 0;
    uint32_t reset = *(uint32_t*)(EXT_APP_BASE + 4);
    if ((reset < EXT_APP_BASE) || (reset > (EXT_APP_BASE + 0x01000000UL)))
        return 0;
#if QSPI_ENABLE_APP_HEADER_CHECK
    /* Header is placed just before vector table or at base? Define convention: header at EXT_APP_BASE + 0x200 (example) could differ.
       For simplicity assume header located immediately after vector table (offset 0x200 is common in some schemes, but we'll use 0x100).
       Here choose offset 0x100 to avoid clobbering initial vectors if aligning with generated image; adjust as needed. */
    const qspi_app_header_t *hdr = (const qspi_app_header_t*)(EXT_APP_BASE + 0x100);
    if (!QSPI_AppHeaderValid(hdr)) return 0;
    /* (Optional) CRC verification can be added reading hdr->length bytes after header. */
#endif
    return 1;
}

static void JumpToExternalApp(void)
{
    uint32_t sp = *(uint32_t*)EXT_APP_BASE;
    uint32_t reset = *(uint32_t*)(EXT_APP_BASE + 4);

    __disable_irq();
    SCB->VTOR = EXT_APP_BASE; /* Redirect vector table */
    __DSB(); __ISB();
    __set_MSP(sp);
    AppEntry_t entry = (AppEntry_t)reset;
    __enable_irq();
    entry();
}

void QSPI_XIP_BootExternal(void)
{
    /* Peripherals already initialized before calling this. QUADSPI should be in memory-mapped mode. */
    if (!QSPI_IsMemoryMapped()) {
        /* If not mapped (e.g. QSPI_ENABLE_XIP was 0) we cannot jump safely. */
        return;
    }
    if (!QSPI_AppVectorValid()) {
        /* Invalid vector table in external flash: stay in stub */
        return;
    }
    JumpToExternalApp();
}

/* Minimal stub main: initialize system then jump */
int main(void)
{
    /* Enable caches prior to HAL_Init if desired (already done in original main) */
    SCB_EnableICache();
    SCB_EnableDCache();
    HAL_Init();
    SystemClock_Config(); /* Minimal clock init for stub */
    /* Basic GPIO (optional LED) */
    MX_GPIO_Init();
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, LED_OFF_LEVEL);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, LED_OFF_LEVEL);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, LED_OFF_LEVEL);
    /* QUADSPI init will perform JEDEC check and enter memory-mapped if QSPI_ENABLE_XIP=1 */
    MX_QUADSPI_Init();
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, LED_ON_LEVEL); /* Indicate boot stub active */

    // QSPI_XIP_BootExternal();

    /* Fallback loop: blink LED slowly if jump failed */
    while (1) {
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
        HAL_Delay(500);
    }
}

/* Simplified clock configuration: enable HSE, configure PLL1 for a moderate SYSCLK.
 * If the exact frequencies of the full app are required, replicate that code here. */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    /* Match external app values for consistency */
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 60;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|
                                 RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2|
                                 RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    // __disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_5);
        HAL_Delay(500);
    }
}

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}

#endif /* INTERNAL_BOOT_STUB */
