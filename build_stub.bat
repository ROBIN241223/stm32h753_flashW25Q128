@echo off
set NAME=h753stub
set OUTDIR=build\stub_flat
if not exist build mkdir build
if not exist %OUTDIR% mkdir %OUTDIR%
set CC=arm-none-eabi-gcc
set OBJCOPY=arm-none-eabi-objcopy
set DEFS=-DINTERNAL_BOOT_STUB=1 -DQSPI_ENABLE_XIP=1 -DQSPI_ENABLE_DEMO=0 -DDATA_CACHE_ENABLE -DINSTRUCTION_CACHE_ENABLE -DCORE_CM7
set INCLUDES=-ICore/Inc -IDrivers/CMSIS/Include -IDrivers/CMSIS/Device/ST/STM32H7xx/Include -IDrivers/STM32H7xx_HAL_Driver/Inc -IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy
set CFLAGS=-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -O2 -ffunction-sections -fdata-sections %DEFS% %INCLUDES%

set CORE=boot_xip_stub system_stm32h7xx stm32h7xx_it stm32h7xx_hal_msp gpio quadspi mdma tim sysmem syscalls
set HAL=stm32h7xx_hal stm32h7xx_hal_cortex stm32h7xx_hal_dma stm32h7xx_hal_dma_ex stm32h7xx_hal_exti stm32h7xx_hal_flash stm32h7xx_hal_flash_ex stm32h7xx_hal_gpio stm32h7xx_hal_hsem stm32h7xx_hal_mdma stm32h7xx_hal_pwr stm32h7xx_hal_pwr_ex stm32h7xx_hal_qspi stm32h7xx_hal_rcc stm32h7xx_hal_rcc_ex stm32h7xx_hal_tim stm32h7xx_hal_tim_ex stm32h7xx_ll_delayblock

echo Compiling core sources...
for %%f in (%CORE%) do (
  %CC% %CFLAGS% -c Core/Src/%%f.c -o %OUTDIR%/%%f.o || goto :err
)
echo Compiling HAL sources...
for %%f in (%HAL%) do (
  %CC% %CFLAGS% -c Drivers/STM32H7xx_HAL_Driver/Src/%%f.c -o %OUTDIR%/%%f.o || goto :err
)
echo Assembling startup...
%CC% -mcpu=cortex-m7 -mthumb -c Core/Startup/startup_stm32h753iikx.s -o %OUTDIR%/startup.o || goto :err

echo Linking...
%CC% %OUTDIR%\*.o -TSTM32H753IIKX_INTFLASH.ld -Wl,-Map=build\%NAME%.map -Wl,--gc-sections --specs=nosys.specs --specs=nano.specs -o build\%NAME%.elf || goto :err
echo Objcopy HEX...
%OBJCOPY% -O ihex build\%NAME%.elf build\%NAME%.hex || goto :err
echo Objcopy BIN...
%OBJCOPY% -O binary build\%NAME%.elf build\%NAME%.bin || goto :err
echo SUCCESS: build\%NAME%.elf / .hex / .bin ready.
goto :eof

:err
echo ERROR during stub build.
exit /b 1
