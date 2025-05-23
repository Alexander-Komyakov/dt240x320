#include "main.h"
#include "button.h"
#include "esp_log.h"
#include "display.h"
#include "image_structure.h"
#include "pong.h"
#include "doodle.h"
#include "arkanoid.h"
#include "sonic.h"
#include "saves.h"
#include "menu.h"


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
    xTaskCreate(storage_task, "storage_task", 4096, NULL, 1, NULL);
    uint8_t number_game = menu(spi);
    if (number_game == 0) game_pong(spi);
    else if (number_game == 1) game_doodle(spi);
    else if (number_game == 2) game_arkanoid(spi);
    else if (number_game == 3) sonic(spi);
}

