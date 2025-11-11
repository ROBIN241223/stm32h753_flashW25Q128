#!/bin/bash
# Script nạp firmware qua ST-Link V3
# Hỗ trợ cả bootloader và application

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN} STM32H753 Firmware Flash Tool${NC}"
echo -e "${GREEN} ST-Link V3 Programmer${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Kiểm tra ST-Link connection
echo "Checking ST-Link connection..."
if st-info --probe > /dev/null 2>&1; then
    echo -e "${GREEN}✓ ST-Link V3 detected${NC}"
    st-info --probe
else
    echo -e "${RED}✗ ST-Link not found!${NC}"
    echo "Please connect ST-Link V3 and try again"
    exit 1
fi

echo ""
echo "Select flash option:"
echo "  1) Flash Application only (h753duc.bin -> 0x08000000)"
echo "  2) Flash Bootloader only (bootloader.bin -> 0x08000000)"
echo "  3) Flash Complete Firmware (bootloader + app merged HEX)"
echo "  4) Flash Application to External QSPI (0x90000000)"
echo "  5) Flash and Open Serial Monitor"
echo ""
read -p "Enter choice [1-5]: " choice

case $choice in
    1)
        echo -e "${YELLOW}Flashing Application...${NC}"
        if [ ! -f "build/h753duc.bin" ]; then
            echo -e "${RED}Error: build/h753duc.bin not found${NC}"
            echo "Run 'make' first to build the project"
            exit 1
        fi
        
        st-flash --reset write build/h753duc.bin 0x08000000
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Application flashed successfully${NC}"
        else
            echo -e "${RED}✗ Flash failed${NC}"
            exit 1
        fi
        ;;
        
    2)
        echo -e "${YELLOW}Flashing Bootloader...${NC}"
        if [ ! -f "build/bootloader/bootloader.bin" ]; then
            echo -e "${RED}Error: build/bootloader/bootloader.bin not found${NC}"
            echo "Run 'make -f Makefile.bootloader' first"
            exit 1
        fi
        
        st-flash --reset write build/bootloader/bootloader.bin 0x08000000
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Bootloader flashed successfully${NC}"
        else
            echo -e "${RED}✗ Flash failed${NC}"
            exit 1
        fi
        ;;
        
    3)
        echo -e "${YELLOW}Flashing Complete Firmware (HEX)...${NC}"
        if [ ! -f "build/complete_firmware.hex" ]; then
            echo -e "${RED}Error: build/complete_firmware.hex not found${NC}"
            echo "Run './build_complete.sh' first"
            exit 1
        fi
        
        st-flash --format ihex --reset write build/complete_firmware.hex
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Complete firmware flashed successfully${NC}"
        else
            echo -e "${RED}✗ Flash failed${NC}"
            exit 1
        fi
        ;;
        
    4)
        echo -e "${YELLOW}Flashing to External QSPI...${NC}"
        echo -e "${YELLOW}Note: Bootloader must be flashed first!${NC}"
        if [ ! -f "build/h753duc.bin" ]; then
            echo -e "${RED}Error: build/h753duc.bin not found${NC}"
            exit 1
        fi
        
        st-flash write build/h753duc.bin 0x90000000
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Application flashed to QSPI successfully${NC}"
        else
            echo -e "${RED}✗ Flash to QSPI failed${NC}"
            echo "You may need external loader for QSPI flash"
            exit 1
        fi
        ;;
        
    5)
        echo -e "${YELLOW}Flashing and opening serial monitor...${NC}"
        if [ ! -f "build/h753duc.bin" ]; then
            echo -e "${RED}Error: build/h753duc.bin not found${NC}"
            exit 1
        fi
        
        st-flash --reset write build/h753duc.bin 0x08000000
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Flash successful${NC}"
            echo ""
            echo "Opening serial monitor..."
            sleep 2
            
            # Detect serial port
            if [ -e "/dev/ttyUSB0" ]; then
                SERIAL_PORT="/dev/ttyUSB0"
            elif [ -e "/dev/ttyACM0" ]; then
                SERIAL_PORT="/dev/ttyACM0"
            else
                echo -e "${RED}No serial port found${NC}"
                exit 1
            fi
            
            echo "Serial port: $SERIAL_PORT"
            echo "Baudrate: 115200"
            echo "Press Ctrl+A then K to exit"
            echo ""
            
            # Try different terminal programs
            if command -v minicom &> /dev/null; then
                minicom -D $SERIAL_PORT -b 115200
            elif command -v screen &> /dev/null; then
                screen $SERIAL_PORT 115200
            elif command -v picocom &> /dev/null; then
                picocom -b 115200 $SERIAL_PORT
            else
                echo -e "${YELLOW}No serial terminal found${NC}"
                echo "Install: sudo apt install minicom"
            fi
        else
            echo -e "${RED}✗ Flash failed${NC}"
            exit 1
        fi
        ;;
        
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN} Done!${NC}"
echo -e "${GREEN}========================================${NC}"
