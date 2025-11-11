#!/bin/bash
# Script to build complete firmware: Bootloader + Application
# Bootloader at 0x08000000 (internal flash)
# Application at 0x90000000 (external QSPI flash)

set -e

echo "======================================"
echo "Building Dual-Stage Firmware"
echo "======================================"

# Step 1: Build Bootloader
echo ""
echo "[1/4] Building Bootloader..."
make -f Makefile.bootloader clean
make -f Makefile.bootloader -j$(nproc)

if [ ! -f "build/bootloader/bootloader.hex" ]; then
    echo "ERROR: Bootloader build failed!"
    exit 1
fi

echo "✓ Bootloader built successfully"

# Step 2: Build Application (main firmware)
echo ""
echo "[2/4] Building Application..."
make clean
make -j$(nproc)

if [ ! -f "build/h753duc.hex" ]; then
    echo "ERROR: Application build failed!"
    exit 1
fi

echo "✓ Application built successfully"

# Step 3: Merge HEX files
echo ""
echo "[3/4] Merging Bootloader + Application..."

# srec_cat is part of srecord package
# Install: sudo apt-get install srecord (Ubuntu/Debian)
# or: brew install srecord (macOS)

if ! command -v srec_cat &> /dev/null; then
    echo "ERROR: srec_cat not found. Please install srecord package."
    echo "  Ubuntu/Debian: sudo apt-get install srecord"
    echo "  macOS: brew install srecord"
    exit 1
fi

# Merge bootloader (0x08000000) and application (0x90000000)
srec_cat \
    build/bootloader/bootloader.hex -Intel \
    build/h753duc.hex -Intel \
    -o build/complete_firmware.hex -Intel

echo "✓ Firmware merged successfully"

# Step 4: Generate binary files
echo ""
echo "[4/4] Generating binary files..."

# Convert merged HEX to BIN
arm-none-eabi-objcopy -I ihex -O binary \
    build/complete_firmware.hex \
    build/complete_firmware.bin

echo "✓ Binary files generated"

# Display file sizes
echo ""
echo "======================================"
echo "Build Summary:"
echo "======================================"
echo "Bootloader:"
ls -lh build/bootloader/bootloader.{hex,bin} | awk '{print "  " $9 ": " $5}'
echo ""
echo "Application:"
ls -lh build/h753duc.{hex,bin} | awk '{print "  " $9 ": " $5}'
echo ""
echo "Complete Firmware:"
ls -lh build/complete_firmware.{hex,bin} | awk '{print "  " $9 ": " $5}'
echo ""
echo "======================================"
echo "✓ Build complete!"
echo "======================================"
echo ""
echo "Flash commands:"
echo "  1. Flash complete firmware:"
echo "     st-flash --format ihex write build/complete_firmware.hex"
echo ""
echo "  2. Or flash separately:"
echo "     st-flash write build/bootloader/bootloader.bin 0x08000000"
echo "     st-flash write build/h753duc.bin 0x90000000"
echo ""
