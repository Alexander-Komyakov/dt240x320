#include "button.h"

// Стрим для передачи нажатых клавишь
StreamBufferHandle_t xStreamBuffer;

// Задача нажатия кнопок
void button_task(void *pvParameter) {
    int pins[] = BUTTON_PINS;
    int num_pins = 9; // 9 кнопок

    for ( ;; ) {
        for (int i = 0; i < num_pins; i++) {
            if (gpio_get_level(pins[i]) == 0) { // Если кнопка нажата (LOW, так как подтяжка к VCC)
                xStreamBufferSend(xStreamBuffer, &pins[i], sizeof(pins[i]), 0);
                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)); // Задержка для устранения дребезга
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Небольшая задержка для снижения нагрузки на CPU
    }
}

void init_gpio_button() {
    // Настраиваем пины как входы с подтягивающими резисторами
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // Отключаем прерывания
    io_conf.mode = GPIO_MODE_INPUT;       // Режим ввода
    io_conf.pin_bit_mask = 0;             // Очищаем маску

    // Добавляем пины в маску
    uint64_t button_pins = 0;
    int pins[] = BUTTON_PINS;
    for (int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
        button_pins |= (1ULL << pins[i]);
    }
    io_conf.pin_bit_mask = button_pins;

    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Включаем подтягивающие резисторы
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}
