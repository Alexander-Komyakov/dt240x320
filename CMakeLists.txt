# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dt240x320)

# Настройки CPack
set(CPACK_PACKAGE_CONTACT "Alexander Komyakov <sanya202018@gmail.com>")
set(CPACK_PACKAGE_NAME "esp32-dt240x320")
set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "ESP32 firmware package")
set(CPACK_PACKAGE_MAINTAINER "Alexander Komyakov <sanya202018@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3, esptool")
set(CPACK_GENERATOR "DEB")
set(CPACK_SET_DESTDIR ON)


# Ищем esptool.py в системе
find_program(ESPTOOL esptool.py PATHS /usr/bin /usr/local/bin REQUIRED)

# Копируем в пакет
install(FILES "${ESPTOOL}"
        DESTINATION "/opt/esp32-tools"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

# Установочные правила
install(FILES 
    "${CMAKE_BINARY_DIR}/bootloader/bootloader.bin"
    "${CMAKE_BINARY_DIR}/partition_table/partition-table.bin"
    "${CMAKE_BINARY_DIR}/dt240x320.bin"
    DESTINATION "/opt/esp32-project"
)

install(PROGRAMS "${CMAKE_SOURCE_DIR}/scripts/flash_esp32.sh"
    DESTINATION "/usr/bin"
    RENAME "esp32-project-flash"
)
include(CPack)
