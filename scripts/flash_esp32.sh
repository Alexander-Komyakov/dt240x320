#!/bin/bash
esptool.py --port /dev/ttyUSB0 write_flash \
    0x1000 /opt/esp32-project/bootloader.bin \
    0x8000 /opt/esp32-project/partition-table.bin \
    0x10000 /opt/esp32-project/firmware.bin
