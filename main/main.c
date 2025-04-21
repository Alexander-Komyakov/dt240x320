#include "main.h"
#include "button.h"
#include "esp_log.h"
#include "display.h"


void app_main(void)
{
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    init_gpio_display();
    reset_display();
    spi_device_handle_t spi;

    spi_init(&spi);

    const size_t data = 1;
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, data);  //16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0x70;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    init_gpio_button();
    // Создаем задачу для обработки нажатий
    xTaskCreate(button_task, "button_task", 2048, NULL, 1, NULL);


	// Передвижение персонажа(квадрата)
    int received = 0;
	int x_pos = 96, y_pos = 120;


	int prev_x = x_pos, prev_y = y_pos;

	// Заливаем весь экран белым (только при старте)
	fill_screen(spi, 0xFFFF);

	while (1) {
	    if (xStreamBufferReceive(xStreamBuffer, &received, sizeof(received), 0) > 0) {
			if (received == BUTTON_UP) {
				y_pos -= 3;
			}
            if (received == BUTTON_DOWN) {
                y_pos += 3;
            } 
            if (received == BUTTON_LEFT) {
                x_pos -= 3;
            } 
            if (received == BUTTON_RIGHT) {
                x_pos += 3;
            } 

	        // Ограничиваем координаты
	        x_pos = (x_pos < 0) ? 0 : (x_pos > DISPLAY_WIDTH - 32) ? DISPLAY_WIDTH - 32 : x_pos;
	        y_pos = (y_pos < 0) ? 0 : (y_pos > DISPLAY_HEIGHT - 40) ? DISPLAY_HEIGHT - 40 : y_pos;

	        // Стираем старый квадрат (заливаем белым)
            if (prev_x != x_pos || prev_y != y_pos) {
	            fill_rect(spi, prev_x, prev_y, 32, 40, 0xFFFF);
            }
	        
	        // Рисуем новый квадрат (чёрный)
	        fill_rect(spi, x_pos, y_pos, 32, 40, 0x0000);
	        
	        prev_x = x_pos;
	        prev_y = y_pos;
	    }
	    vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
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

