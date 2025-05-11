#include "sonic.h"

void sonic(spi_device_handle_t spi)
{
    init_gpio_display();
    reset_display();

    spi_init(&spi);

    const size_t data = 1;
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, data);  //16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0xB0;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    fill_screen(spi, 0xFFFF);

/*
    Fill more rectangle
*/
/*
    fill_rect(spi, 0, 0, 32, 40, 0x0000);
    fill_rect(spi, 0, 40, 32, 40, 0x1111);
    fill_rect(spi, 0, 80, 32, 40, 0x2222);
    fill_rect(spi, 0, 120, 32, 40, 0x3333);
    fill_rect(spi, 32, 0, 32, 40, 0x4444);
    fill_rect(spi, 32, 40, 32, 40, 0x5555);
    fill_rect(spi, 32, 80, 32, 40, 0x6666);
    fill_rect(spi, 32, 120, 32, 40, 0x7777);
    fill_rect(spi, 64, 0, 32, 40, 0x8888);
    fill_rect(spi, 64, 40, 32, 40, 0x9999);
    fill_rect(spi, 64, 80, 32, 40, 0xAAAA);
    fill_rect(spi, 64, 120, 32, 40, 0xBBBB);
    fill_rect(spi, 96, 0, 32, 40, 0xCCCC);
    fill_rect(spi, 96, 40, 32, 40, 0xDDDD);
    fill_rect(spi, 96, 80, 32, 40, 0xEEEE);
    fill_rect(spi, 96, 120, 32, 40, 0xFFFF);
*/

	while (1)
	{
    	draw_image(spi, &image_sonic1);
	    vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
    	draw_image(spi, &image_sonic2);
		vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
        draw_image(spi, &image_sonic3);
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
        draw_image(spi, &image_sonic4);
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
        draw_image(spi, &image_sonic5);
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
        draw_image(spi, &image_sonic6);
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Пауза 100 мс
	}

/*

    Vertical Scrolling

*/
/*
    uint16_t tfa = 0;
    uint16_t vsa = 320;
    uint16_t bfa = 0;
    uint16_t ssa = 0;
    uint16_t i = 1;
    while (1) {
        if (i == 320) { i = 1; }; i++;
        ssa = 0+i;
        vertical_scroll(spi, &tfa, &vsa, &bfa, &ssa);
        //fill_screen(spi, 0xCCCC+i);
        vTaskDelay(10 / portTICK_PERIOD_MS); // Задержка для видимости прокрутки
        printf("ok\n");
    }
*/
}


