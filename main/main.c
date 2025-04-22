#include "main.h"
#include "button.h"
#include "esp_log.h"
#include "display.h"
#include "image_structure.h"


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

    send_command(spi, 0x13);

    uint8_t madctl_value = 0x70;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    init_gpio_button();
    // Создаем задачу для обработки нажатий
    xTaskCreate(button_task, "button_task", 2048, NULL, 1, NULL);


	// Передвижение персонажа(квадрата)
    int received = 0;

    image_kunglao_1.x = 100;
    image_kunglao_1.y = 100;
	int prev_x = image_kunglao_1.x, prev_y = image_kunglao_1.y;
    uint8_t speed = 3;
	draw_image(spi, &image_background_forest_1);

/*
    Vertical Scrolling
    uint16_t tfa = 0;
    uint16_t vsa = 320;
    uint16_t bfa = 0;
    uint16_t ssa = 0;
    uint16_t i = 1;
*/
	while (1) {
/*
        if (i == 320) { i = 1; }; i++;
        ssa = 0+i;
        vertical_scroll(spi, &tfa, &vsa, &bfa, &ssa);
*/
	    if (xStreamBufferReceive(xStreamBuffer, &received, sizeof(received), 0) > 0) {
			if (received == BUTTON_UP) {
				image_kunglao_1.y -= speed;
			}
            if (received == BUTTON_DOWN) {
                image_kunglao_1.y += speed;
            } 
            if (received == BUTTON_LEFT) {
                image_kunglao_1.x -= speed;
            } 
            if (received == BUTTON_RIGHT) {
                image_kunglao_1.x += speed;
            } 

	        // Ограничиваем координаты
	        image_kunglao_1.x = ((uint16_t) (image_kunglao_1.x + speed) <= speed) ? 0 : (image_kunglao_1.x > DISPLAY_WIDTH - 32) ? DISPLAY_WIDTH - 32 : image_kunglao_1.x;
	        image_kunglao_1.y = ((uint16_t) (image_kunglao_1.y + speed) <= speed) ? 0 : (image_kunglao_1.y > DISPLAY_HEIGHT - 40) ? DISPLAY_HEIGHT - 40 : image_kunglao_1.y;

	        // Стираем старый квадрат (заливаем белым)
            //if (prev_x != image_kunglao_1.x || prev_y != image_kunglao_1.y) {
	        //    fill_rect(spi, prev_x, prev_y, 32, 40, 0xFFFF);
            //}
	        
			// Рисуем нового кунглао
			draw_image_background(spi, &image_kunglao_1, image_background_forest_pixels_1);
	        
	        prev_x = image_kunglao_1.x;
	        prev_y = image_kunglao_1.y;
            //fill_screen(spi, 0x0000);
            //fill_screen(spi, 0xFFFF);
            //fill_screen(spi, 0xCCCC);
            //fill_screen(spi, 0xBBBB);
	    }
	    vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
	}
}

