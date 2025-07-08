#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "driver/gpio.h"

// Определяем пины для клавиш
#define BUTTON_PINS {GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_19, GPIO_NUM_21, GPIO_NUM_22, GPIO_NUM_25, GPIO_NUM_32}

// Задержка для устранения дребезга клавишь (в миллисекундах)
#define DEBOUNCE_DELAY_MS 5

// Назначение клавиш
#define BUTTON_LEFT    GPIO_NUM_19
#define BUTTON_RIGHT   GPIO_NUM_14
#define BUTTON_UP      GPIO_NUM_13
#define BUTTON_DOWN    GPIO_NUM_25
#define BUTTON_RED     GPIO_NUM_17
#define BUTTON_WHITE   GPIO_NUM_22
#define BUTTON_YELLOW  GPIO_NUM_21
#define BUTTON_BLUE    GPIO_NUM_16
#define BUTTON_CENTER  GPIO_NUM_32

#define STREAM_BUF_SIZE 32

void init_gpio_button();
void button_task(void *pvParameter);

extern StreamBufferHandle_t xStreamBuffer;
