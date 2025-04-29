#include "main.h"
#include "button.h"
#include "esp_log.h"
#include "display.h"
#include "image_structure.h"
#include "pong.h"


void app_main(void)
{
    init_gpio_display();
    reset_display();
    spi_device_handle_t spi;

    spi_init(&spi);

    init_display(spi);
    init_gpio_button();
    // Создаем задачу для обработки нажатий
    xTaskCreate(button_task, "button_task", 2048, NULL, 1, NULL);

    fill_screen(spi, 0xFFFF);

    draw_border(spi, &image_pong_preview, 8, 0xAAAA);

    draw_image(spi, &image_pong_preview);
    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    image_pong_preview.x = 223;
    draw_image(spi, &image_pong_preview);

    image_pong_preview.y = 140;
    draw_image(spi, &image_pong_preview);
    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    image_pong_preview.x = 31;

    draw_image(spi, &image_pong_preview);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    game_pong(spi);
}
