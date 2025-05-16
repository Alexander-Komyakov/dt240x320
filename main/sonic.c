#include "sonic.h"

// Структура с нулевыми значениями x, y
Image current_fighter = {0};
Image current_sonic = {0};
Image current_pikachu = {0};

// Буфер для композитного изображения
static uint16_t *composite_buffer = NULL;
static size_t composite_buffer_size = 0;

// Инициализация буфера
void init_composite_buffer(uint16_t width, uint16_t height) {
    size_t needed_size = width * height * sizeof(uint16_t);
    if (composite_buffer_size < needed_size) {
        if (composite_buffer) free(composite_buffer);
        composite_buffer = heap_caps_malloc(needed_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
        composite_buffer_size = needed_size;
    }
}

// Отрисовка персонажа на композитном буфере
void draw_character(const Image *character) {
    for (int y = 0; y < character->height; y++) {
        for (int x = 0; x < character->width; x++) {
            uint16_t pixel = character->pixels[y * character->width + x];
            if (pixel != TRANSPARENT_COLOR) {
                int buf_x = character->x + x;
                int buf_y = character->y + y;
                if (buf_x >= 0 && buf_x < DISPLAY_WIDTH && 
                    buf_y >= 0 && buf_y < DISPLAY_HEIGHT) {
                    composite_buffer[buf_y * DISPLAY_WIDTH + buf_x] = pixel;
                }
            }
        }
    }
}

// Создание композитного кадра (фон + персонажи)
void prepare_composite_frame(uint16_t scroll_offset) {
    // Копируем фон со смещением
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int src_x = (x + scroll_offset) % DISPLAY_WIDTH;
            composite_buffer[y * DISPLAY_WIDTH + x] = image_background.pixels[y * DISPLAY_WIDTH + src_x];
        }
    }

    // Рисуем персонажей поверх фона
    draw_character(&current_fighter);
}

// Функция вращения экрана
void rotate_display(spi_device_handle_t spi, uint16_t speed) {
    static uint16_t scroll_offset = 0;
    scroll_offset = (scroll_offset + speed) % DISPLAY_WIDTH;

    // Подготавливаем кадр со смещением
    prepare_composite_frame(scroll_offset);

    // Отправляем на дисплей
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {0, 0, (DISPLAY_WIDTH-1) >> 8, (DISPLAY_WIDTH-1) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, 0, (DISPLAY_HEIGHT-1) >> 8, (DISPLAY_HEIGHT-1) & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);
    send_data16b(spi, composite_buffer, DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

// Задача анимации
void task_animation(void *pvParameters) {
    spi_device_handle_t spi = (spi_device_handle_t)pvParameters;
    
    // Анимационные кадры
    const Image *fighter_idle_frames[] = {&image_fighter_stay1};
    const Image *fighter_move_frames[] = {&image_fighter_move1, &image_fighter_move2,
                                        &image_fighter_move3, &image_fighter_move4,
                                        &image_fighter_move5};
    const Image *fighter_shot_frames[] = {&image_fighter_shot1};
//, &image_fighter_shot2};

    // Состояние персонажа
    typedef enum {
        STATE_IDLE,
        STATE_MOVING,
        STATE_SHOOTING
    } FighterState;
    
    FighterState current_state = STATE_IDLE;
    bool left_pressed = false;
    bool right_pressed = false;
    bool red_button_pressed = false;
    bool red_button_enabled = true;
    
    uint8_t current_frame = 0;
    uint8_t frame_counter = 0;
    const uint8_t move_frame_delay = 1;
    const uint8_t shot_frame_delay = 1;

    // Позиция и движение
    uint16_t fighter_x = FIGHTER_X;
    uint16_t fighter_y = FIGHTER_Y;
    const uint8_t move_speed = 5;
    uint16_t scroll_speed = 0;
    const uint16_t moving_scroll_speed = 6;

    // Инициализация буфера
    init_composite_buffer(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    while (1) {
        // 1. Проверяем состояние кнопок
        left_pressed = (gpio_get_level(BUTTON_LEFT) == 0);
        right_pressed = (gpio_get_level(BUTTON_RIGHT) == 0);
        red_button_pressed = (gpio_get_level(BUTTON_RED) == 0) && red_button_enabled;

        // 2. Обработка состояний
        if (red_button_pressed && current_state != STATE_SHOOTING) {
            // Начало анимации удара
            current_state = STATE_SHOOTING;
            current_frame = 0;
            frame_counter = 0;
            red_button_enabled = false;
        }
        else if (current_state == STATE_SHOOTING) {
            // Продолжаем анимацию удара
            if (frame_counter++ >= shot_frame_delay) {
                frame_counter = 0;
                current_frame++;
                
                if (current_frame >= sizeof(fighter_shot_frames)/sizeof(fighter_shot_frames[0])) {
                    // Завершение анимации удара
                    current_state = STATE_IDLE;
                    current_frame = 0;
                    red_button_enabled = true;
                }
            }
        } 
        else if (left_pressed || right_pressed) {
            // Движение
            current_state = STATE_MOVING;
            
            if (left_pressed) {
                fighter_x = (fighter_x > move_speed) ? fighter_x - move_speed : 0;
                scroll_speed = moving_scroll_speed;
            }
            else if (right_pressed) {
                fighter_x = (fighter_x < DISPLAY_WIDTH - fighter_move_frames[0]->width - move_speed) 
                          ? fighter_x + move_speed 
                          : DISPLAY_WIDTH - fighter_move_frames[0]->width;
                scroll_speed = moving_scroll_speed;
            }
            
            if (frame_counter++ >= move_frame_delay) {
                frame_counter = 0;
                current_frame = (current_frame + 1) % (sizeof(fighter_move_frames)/sizeof(fighter_move_frames[0]));
            }
        }
        else {
            // Стояние
            current_state = STATE_IDLE;
            scroll_speed = 0;
            current_frame = 0;
            frame_counter = 0;
        }

        // 3. Выбор текущего кадра анимации
        switch (current_state) {
            case STATE_SHOOTING:
                current_fighter = *fighter_shot_frames[current_frame];
                break;
            case STATE_MOVING:
                current_fighter = *fighter_move_frames[current_frame];
                break;
            case STATE_IDLE:
            default:
                current_fighter = *fighter_idle_frames[0];
                break;
        }

        // 4. Установка позиции персонажа
        current_fighter.x = fighter_x;
        current_fighter.y = fighter_y;

        // 5. Вращение экрана (если есть движение)
        rotate_display(spi, (current_state == STATE_MOVING) ? scroll_speed : 0);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sonic(spi_device_handle_t spi) {
    init_gpio_display();
    reset_display();
    spi_init(&spi);

    // Инициализация дисплея
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, 1);
    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0x60;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    xTaskCreate(task_animation, "animation_task", 4096, spi, 1, NULL);
}
