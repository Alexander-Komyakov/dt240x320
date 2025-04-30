#include "doodle.h"
#include "font.h"

void game_doodle(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_screen_gradient(spi, 0x0000, 0xFFFF);

    int received_button;
    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}
