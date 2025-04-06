#!/bin/bash

# Проверяем наличие ttyUSB0
if [ ! -c "/dev/ttyUSB0" ]; then
    echo "Ошибка: Устройство /dev/ttyUSB0 не найдено. Подключите ESP32 и проверьте права доступа."
    exit 1
fi

# Запускаем прошивку через Docker контейнер ESP-IDF
docker run --rm -it \
    --device=/dev/ttyUSB0 \
    -v /opt/esp32-project:/opt/esp32-project \
    espressif/idf:release-v5.4 \
    esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z \
        0x1000 /opt/esp32-project/bootloader.bin \
        0x8000 /opt/esp32-project/partition-table.bin \
        0x10000 /opt/esp32-project/dt240x320.bin
