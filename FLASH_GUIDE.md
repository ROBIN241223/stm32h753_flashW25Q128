# Hướng dẫn Flash Firmware qua ST-Link V3

## 🔧 Yêu cầu

- ST-Link V3 hoặc ST-Link V2
- USB cable
- STM32H753 board
- `st-flash` tool (đã cài: `sudo apt install stlink-tools`)

## 🚀 Cách Flash nhanh

### Option 1: Dùng script tương tác (Khuyến nghị)

```bash
./flash.sh
```

Menu sẽ hiện:
```
1) Flash Application only
2) Flash Bootloader only  
3) Flash Complete Firmware (merged)
4) Flash to External QSPI
5) Flash and Open Serial Monitor
```

### Option 2: Dùng lệnh trực tiếp

```bash
# Flash application vào internal flash
st-flash --reset write build/h753duc.bin 0x08000000

# Flash bootloader
st-flash --reset write build/bootloader/bootloader.bin 0x08000000

# Flash complete firmware (HEX)
st-flash --format ihex --reset write build/complete_firmware.hex
```

### Option 3: Dùng VS Code Tasks

1. Nhấn `Ctrl+Shift+P`
2. Chọn `Tasks: Run Task`
3. Chọn:
   - **"Flash with ST-Link (Interactive)"** - Menu tương tác
   - **"Build and Flash"** - Build rồi flash luôn
   - **"Flash Complete Firmware (HEX)"** - Flash firmware đã merge

## 📋 Quy trình Build và Flash đầy đủ

### Cho Application thường:

```bash
# 1. Build
make clean
make -j4

# 2. Flash
st-flash --reset write build/h753duc.bin 0x08000000

# 3. Mở serial monitor (nếu cần)
minicom -D /dev/ttyUSB0 -b 115200
```

### Cho Bootloader + Application:

```bash
# 1. Build all
./build_complete.sh

# 2. Flash merged firmware
st-flash --format ihex --reset write build/complete_firmware.hex

# 3. Test
minicom -D /dev/ttyUSB0 -b 115200
```

## 🐛 Troubleshooting

### Lỗi: "st-flash: not found"
```bash
sudo apt install stlink-tools
```

### Lỗi: "No ST-LINK detected"
```bash
# Kiểm tra kết nối
st-info --probe

# Thử reset ST-Link
sudo systemctl restart udev
```

### Lỗi: Permission denied
```bash
# Thêm user vào group dialout
sudo usermod -a -G dialout $USER

# Hoặc chạy với sudo
sudo st-flash write build/h753duc.bin 0x08000000
```

### Lỗi: Flash không nhận QSPI address (0x90000000)
- Cần flash bootloader trước
- Hoặc dùng STM32CubeProgrammer với External Loader

## 🎯 STM32CubeProgrammer (Alternative)

Nếu muốn dùng GUI:

```bash
# Mở STM32CubeProgrammer
STM32_Programmer_CLI

# Flash qua CLI
STM32_Programmer_CLI -c port=SWD -w build/h753duc.bin 0x08000000 -v -rst
```

## 📊 Kiểm tra kết quả

```bash
# Đọc memory
st-flash read output.bin 0x08000000 0x10000

# Verify
st-flash --reset --verify write build/h753duc.bin 0x08000000
```

## 🔄 Flash qua OpenOCD (Advanced)

```bash
# Start OpenOCD
openocd -f interface/stlink-v2.cfg -f target/stm32h7x.cfg

# Trong terminal khác
telnet localhost 4444
> reset halt
> flash write_image erase build/h753duc.bin 0x08000000
> reset run
> exit
```

## ✅ Checklist Flash thành công

- [ ] ST-Link kết nối (LED nhấp nháy)
- [ ] Build không có lỗi (make thành công)
- [ ] Flash thành công (100% complete)
- [ ] Reset MCU (hoặc power cycle)
- [ ] LED/UART hoạt động đúng
- [ ] CLI console hiện prompt `>`

## 📱 FreeRTOS+CLI Test

Sau khi flash, kết nối UART:

```bash
minicom -D /dev/ttyUSB0 -b 115200
```

Thử các lệnh:
```
> help
> task-stats
> query-heap
> version
> uptime
> flash-info
> clear
```

## 🎨 VS Code Workflow

**Build:**
- `Ctrl+Shift+B` → "Build Firmware"

**Flash:**
- `Ctrl+Shift+P` → "Tasks: Run Task" → "Flash with ST-Link"

**Build + Flash + Monitor:**
- `Ctrl+Shift+P` → "Tasks: Run Task" → "Build and Flash"
- Sau đó chọn option 5 trong menu flash

## 🔧 Config ST-Link trong VS Code

Đã config sẵn trong `.vscode/tasks.json`:
- ✅ Build and Flash
- ✅ Flash Complete Firmware  
- ✅ Flash Bootloader Only
- ✅ Interactive Flash Menu

## 💡 Tips

1. **Luôn dùng `--reset` flag** để reset MCU sau flash
2. **Verify flash** bằng `--verify` nếu nghi ngờ lỗi
3. **Flash chậm** → Kiểm tra dây USB và ST-Link firmware
4. **External flash** cần bootloader hoặc external loader
5. **Debug** → Dùng st-info để check ST-Link status

## 📞 Support

Nếu gặp vấn đề:
```bash
# Check ST-Link info
st-info --probe
st-info --version

# Check MCU info
st-info --descr
```
