#include "button.h"

#define BUTTON_PIN ADC1_CHANNEL_0

// Стрим для передачи нажатых клавишь
StreamBufferHandle_t xStreamBuffer;

// Задача нажатия кнопок
void button_task(void *pvParameter) {
    int pins[] = BUTTON_PINS;
    int num_pins = 9; // 9 кнопок

#ifdef ENABLE_ANALOG_CONTROL
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // GPIO36 (sensor_vp)
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
#endif

    for ( ;; ) {
#ifdef ENABLE_ANALOG_CONTROL
        int raw2 = adc1_get_raw(ADC1_CHANNEL_0);
        int raw1 = adc1_get_raw(ADC1_CHANNEL_3);
        
        if (raw1 < 800) {
            printf("ADC1: %d\n", raw1);
            xStreamBufferSend(xStreamBuffer, &pins[6], sizeof(pins[6]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw1 < 1500) {
            printf("ADC1: %d\n", raw1);
            xStreamBufferSend(xStreamBuffer, &pins[5], sizeof(pins[5]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw1 < 2400) {
            printf("ADC1: %d\n", raw1);
            xStreamBufferSend(xStreamBuffer, &pins[2], sizeof(pins[2]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw1 < 3500) {
            printf("ADC1: %d\n", raw1);
            xStreamBufferSend(xStreamBuffer, &pins[3], sizeof(pins[3]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        }

        if (raw2 < 800) {
            printf("ADC2: %d\n", raw2);
            xStreamBufferSend(xStreamBuffer, &pins[1], sizeof(pins[1]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw2 < 1500) {
            printf("ADC2: %d\n", raw2);
            xStreamBufferSend(xStreamBuffer, &pins[8], sizeof(pins[8]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw2 < 2400) {
            printf("ADC2: %d\n", raw2);
            xStreamBufferSend(xStreamBuffer, &pins[4], sizeof(pins[4]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw2 < 3100) {
            printf("ADC2: %d\n", raw2);
            xStreamBufferSend(xStreamBuffer, &pins[7], sizeof(pins[7]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        } else if (raw2 < 3900) {
            printf("ADC2: %d\n", raw2);
            xStreamBufferSend(xStreamBuffer, &pins[0], sizeof(pins[0]), 0);
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        }
#endif

        for (int i = 0; i < num_pins; i++) {
            if (gpio_get_level(pins[i]) == 0) { // Если кнопка нажата (LOW, так как подтяжка к VCC)
                printf("pins: %d\n", i);
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
