#include "dacha.h"

void game_dacha(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    draw_image(spi, &image_dacha);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    draw_image(spi, &image_dacha_evening);

    int received_button;

    // Переменные для скроллинга
    uint16_t ssa = 0;
    float speed = 0;
    float car_position = 0; // Точная позиция float
    float scroll_position = 0; // Точная позиция скролла float
    
    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (speed < 1) {
                speed += 0.08f; // 'f' для float
            }
        }
        if (speed > 0) {
            speed -= 0.02f;
            if (speed < 0) speed = 0; // Защита от отрицательной скорости
        }

        // ОБНОВЛЕНИЕ ПОЗИЦИИ МАШИНКИ
        car_position += (speed * 2.0f);
        
        // ЦИКЛИЧЕСКАЯ КОРРЕКЦИЯ (0-320)
        while (car_position >= DISPLAY_WIDTH) {
            car_position -= DISPLAY_WIDTH;
        }
        while (car_position < 0) {
            car_position += DISPLAY_WIDTH;
        }
        
        // ПРЕОБРАЗОВАНИЕ В INT ДЛЯ ОТРИСОВКИ
        image_dacha_car.x = (int)(car_position + 0.5f); // Округление до ближайшего

        // ОБНОВЛЕНИЕ СКРОЛЛИНГА
        scroll_position -= speed;
        
        // ЦИКЛИЧЕСКАЯ КОРРЕКЦИЯ СКРОЛЛА (0-320)
        while (scroll_position >= DISPLAY_WIDTH) {
            scroll_position -= DISPLAY_WIDTH;
        }
        while (scroll_position < 0) {
            scroll_position += DISPLAY_WIDTH;
        }
        
        // ПРЕОБРАЗОВАНИЕ В INT ДЛЯ СКРОЛЛА
        ssa = (uint16_t)(scroll_position + 0.5f); // Округление

        printf("Iter: x=%d (%.2f), scroll=%d (%.2f), speed=%.2f\n", 
               image_dacha_car.x, car_position, ssa, scroll_position, speed);
        
        draw_image_background(spi, &image_dacha_car, image_dacha_evening_pixels, 0xFFFF);
        vertical_scroll(spi, 0, 320, 0, ssa);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
