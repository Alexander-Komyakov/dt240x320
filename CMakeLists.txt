# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dt240x320)


if(UNIX AND NOT APPLE)
    include(InstallRequiredSystemLibraries)
    
    set(CPACK_PACKAGE_NAME "esp32-project")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_DESCRIPTION "ESP32 firmware package")
    set(CPACK_PACKAGE_MAINTAINER "Alexander Komyakov <sanya202018@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "python3, esptool")
    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    
    # Указываем какие файлы куда устанавливать
    install(
        DIRECTORY ${CMAKE_BINARY_DIR}/
        DESTINATION /opt/esp32-project
        FILES_MATCHING PATTERN "*.bin"
    )
    
    # Устанавливаем скрипт для прошивки
    install(
        PROGRAMS ${CMAKE_SOURCE_DIR}/scripts/flash_esp32.sh
        DESTINATION /usr/bin
        RENAME esp32-project-flash
    )
    
    include(CPack)
endif()
