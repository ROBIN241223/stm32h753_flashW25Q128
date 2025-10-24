# Dual-Stage Boot (Internal Flash Stub + External QSPI XIP)

## Mục tiêu
Chạy một boot stub nhỏ trong internal Flash (0x0800_0000) để:
1. Khởi tạo clock, QSPI và bật chế độ Memory-Mapped (XIP) cho Winbond W25Q128 tại 0x9000_0000.
2. Kiểm tra vector table external (stack & reset handler) hợp lệ.
3. Nhảy vào ứng dụng chính đã link ở 0x9000_0000.

## Thành phần chính
- Linker stub: `STM32H753IIKX_INTFLASH.ld` (FLASH = 0x08000000, EXTFLASH chỉ tham chiếu).
- Stub source: `Core/Src/boot_xip_stub.c` (định nghĩa `main()` khi `INTERNAL_BOOT_STUB` bật).
- Ứng dụng chính vẫn link bằng `STM32H753IIKX_FLASH.ld` (FLASH = external 0x90000000).

## Build stub
Hai cách:

### 1. Makefile phẳng
```
make -f Makefile.stubflat stub_hex
```
Kết quả: `build/h753stub.elf`, `.hex`, `.bin`.

### 2. Batch script (đề xuất cho Windows)
```
build_stub.bat
```
Kết quả tương tự.

## Flash thứ tự
1. Nạp stub nội flash:
```
STM32_Programmer_CLI -c port=SWD -w build/h753stub.hex -v -rst
```
2. Nạp ứng dụng external QSPI:
   - Cách A (External Loader): Chọn loader tương thích W25Q128 trong STM32CubeProgrammer rồi nạp `build/h753duc.hex` (link 0x90000000).
   - Cách B (Tương lai): Stub tự nhận file qua UART/… rồi ghi QSPI (chưa triển khai).
3. Reset MCU: Stub -> init QSPI -> jump app.

## Kiểm tra jump
Trong debug, quan sát:
- `QSPI_IsMemoryMapped()` trả 1.
- Vector tại 0x90000004 (reset handler) khác 0xFFFFFFFF và nằm trong vùng 0x9000_0000..0x900FFFFF (tuỳ kích thước app).

## Khi nào cần External Loader
Chỉ khi bạn muốn nạp / verify QSPI trực tiếp từ PC mà **không chạy stub**. Loader cung cấp hàm low-level cho CubeProgrammer điều khiển QSPI qua SWD.

## Gợi ý mở rộng
- Thêm magic header + CRC ở đầu external image.
- Hỗ trợ command UART để update firmware.
- Chuyển sang Fast Quad Read (0x6B/0xEB) sau khi xác nhận ổn định.

## Macro quan trọng
- `INTERNAL_BOOT_STUB` (stub build): bật `main()` trong `boot_xip_stub.c`.
- `QSPI_ENABLE_XIP=1` đảm bảo vào memory-mapped trước khi jump.
- `QSPI_ENABLE_DEMO=0` tránh erase/program test flash.

## Cảnh báo
Không bật những thao tác erase demo trong stub vì có thể xoá vùng vector app.
