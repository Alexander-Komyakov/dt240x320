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

    game_pong(spi);
}
