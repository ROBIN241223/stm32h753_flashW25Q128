# Hướng dẫn Build và Flash Firmware với Bootloader

## Kiến trúc hệ thống

```
┌─────────────────────────────────────────┐
│  Internal Flash (0x08000000)            │
│  ┌───────────────────────────────────┐  │
│  │  Bootloader (boot_xip_stub.c)     │  │
│  │  - Khởi tạo QSPI                  │  │
│  │  - Enable memory-mapped mode      │  │
│  │  - Jump to external application   │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
                    │
                    │ Jump to 0x90000000
                    ▼
┌─────────────────────────────────────────┐
│  External QSPI Flash (0x90000000)       │
│  ┌───────────────────────────────────┐  │
│  │  Main Application (main.c)        │  │
│  │  - FreeRTOS tasks                 │  │
│  │  - Application logic              │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

## Cài đặt công cụ cần thiết

### Ubuntu/Debian:
```bash
sudo apt-get install srecord
```

### macOS:
```bash
brew install srecord
```

## Cách build firmware

### Option 1: Build tự động (khuyến nghị)
```bash
./build_complete.sh
```

Script này sẽ:
1. Build bootloader (từ internal flash)
2. Build application (cho external flash)
3. Merge 2 file HEX thành 1 file duy nhất
4. Tạo file binary

### Option 2: Build thủ công

#### Bước 1: Build Bootloader
```bash
make -f Makefile.bootloader clean
make -f Makefile.bootloader -j$(nproc)
```

Kết quả: `build/bootloader/bootloader.hex` và `.bin`

#### Bước 2: Build Application
```bash
make clean
make -j$(nproc)
```

Kết quả: `build/h753duc.hex` và `.bin`

#### Bước 3: Merge 2 file HEX
```bash
srec_cat \
    build/bootloader/bootloader.hex -Intel \
    build/h753duc.hex -Intel \
    -o build/complete_firmware.hex -Intel
```

## Cách nạp firmware

### Option 1: Nạp file HEX đã merge (đơn giản nhất)
```bash
st-flash --format ihex write build/complete_firmware.hex
```

### Option 2: Nạp riêng từng phần
```bash
# Nạp bootloader vào internal flash
st-flash write build/bootloader/bootloader.bin 0x08000000

# Nạp application vào external QSPI flash
st-flash write build/h753duc.bin 0x90000000
```

**Lưu ý:** Option 2 yêu cầu external loader cho QSPI hoặc bootloader phải được nạp trước để hỗ trợ ghi QSPI.

## Quy trình hoạt động

1. **Power On / Reset**
   - CPU bắt đầu từ 0x08000000 (internal flash)
   - Vector table trỏ đến bootloader

2. **Bootloader thực thi**
   - Khởi tạo system clock
   - Khởi tạo QSPI peripheral
   - Detect W25Q128 flash chip
   - Enable memory-mapped mode (QSPI XIP)

3. **Kiểm tra application**
   - Đọc vector table tại 0x90000000
   - Kiểm tra stack pointer hợp lệ
   - Kiểm tra reset handler hợp lệ

4. **Jump to application**
   - Disable interrupts
   - Set VTOR = 0x90000000
   - Set MSP = stack pointer từ external flash
   - Jump đến reset handler của application
   - Enable interrupts
   - Application chạy từ external QSPI flash

## Debug

### Kiểm tra bootloader có chạy không:
```c
// Thêm vào boot_xip_stub.c trong hàm main()
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // LED ON
HAL_Delay(1000);
HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET); // LED OFF
```

### Kiểm tra QSPI memory-mapped:
Trong debugger, kiểm tra:
- Register QUADSPI->CCR bit[26] = 1 (functional mode = memory-mapped)
- Đọc 0x90000000 phải trả về dữ liệu hợp lệ (không phải 0xFFFFFFFF)

### Kiểm tra application có được nạp đúng:
```bash
# Đọc 8 bytes đầu tiên từ QSPI flash (stack pointer + reset handler)
st-flash read /tmp/qspi_header.bin 0x90000000 8
hexdump -C /tmp/qspi_header.bin
```

## Troubleshooting

### Lỗi: Application không chạy sau khi jump
- Kiểm tra linker script của application sử dụng `ORIGIN = 0x90000000`
- Kiểm tra QSPI đã vào memory-mapped mode chưa
- Kiểm tra vector table hợp lệ

### Lỗi: QSPI không khởi tạo được
- Kiểm tra kết nối phần cứng
- Kiểm tra JEDEC ID (0xEF4018 cho W25Q128)
- Kiểm tra clock configuration

### Lỗi: st-flash không nhận QSPI address
- Nạp bootloader trước: `st-flash write build/bootloader/bootloader.bin 0x08000000`
- Hoặc sử dụng external loader cho STM32CubeProgrammer

## Files quan trọng

- `Makefile.bootloader` - Build bootloader
- `Makefile` - Build application
- `build_complete.sh` - Script tự động build và merge
- `STM32H753IIKX_INTFLASH.ld` - Linker script cho bootloader (internal flash)
- `STM32H753XX_FLASH.ld` - Linker script cho application (external QSPI)
- `Core/Src/boot_xip_stub.c` - Source code bootloader

## Cấu trúc thư mục build

```
build/
├── bootloader/
│   ├── bootloader.elf
│   ├── bootloader.hex
│   ├── bootloader.bin
│   └── bootloader.map
├── h753duc.elf
├── h753duc.hex
├── h753duc.bin
├── h753duc.map
├── complete_firmware.hex  ← File này để nạp
└── complete_firmware.bin
```

## Mẹo

1. **Sử dụng file .hex merged** - Dễ dàng nhất, 1 lệnh nạp xong cả bootloader + app
2. **Debug bootloader riêng** - Build và nạp chỉ bootloader để test
3. **Update application** - Chỉ cần rebuild và nạp lại application, không động bootloader
4. **Backup firmware** - Lưu lại file `complete_firmware.hex` để nạp lại sau này
