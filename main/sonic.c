#include "sonic.h"

// Структура с нулевыми значениями x, y
Image current_fighter = {0};
Image current_sonic = {0};
Image current_pikachu = {0};

// Буфер для композитного изображения
static uint16_t *composite_buffer = NULL;
static size_t composite_buffer_size = 0;

// Глобальное смещение для скроллинга
static int global_scroll_offset = 0;

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

// Получение пикселя из объединенной сцены (три фона как один уровень)
static uint16_t get_scene_pixel(int x, int y) {
    int total_width = image_background.width + image_background2.width + image_background3.width;
    
    // Обеспечиваем циклический скролл
    x = x % total_width;
    if (x < 0) x += total_width;
    
    if (x < image_background.width) {
        // Первый фон
        return image_background.pixels[y * image_background.width + x];
    } else if (x < image_background.width + image_background2.width) {
        // Второй фон
        int bg2_x = x - image_background.width;
        return image_background2.pixels[y * image_background2.width + bg2_x];
    } else {
        // Третий фон
        int bg3_x = x - image_background.width - image_background2.width;
        return image_background3.pixels[y * image_background3.width + bg3_x];
    }
}

// Создание композитного кадра (фон + персонажи)
void prepare_composite_frame(int scroll_offset) {
    // Копируем фон со смещением из объединенной сцены
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int scene_x = x + scroll_offset;
            composite_buffer[y * DISPLAY_WIDTH + x] = get_scene_pixel(scene_x, y);
        }
    }

    // Рисуем персонажей поверх фона
    draw_character(&current_fighter);
}

// Функция отрисовки экрана
void render_display(spi_device_handle_t spi) {
    // Подготавливаем кадр со смещением
    prepare_composite_frame(global_scroll_offset);

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
    const uint8_t move_speed = 8;
    const int scroll_speed = 8;
    const int scroll_threshold = 50; // Расстояние от края экрана для начала скроллинга

    // Инициализация буфера
    init_composite_buffer(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    while (1) {
        // 1. Проверяем состояние кнопок
        left_pressed = (gpio_get_level(BUTTON_LEFT) == 0);
        right_pressed = (gpio_get_level(BUTTON_RIGHT) == 0);
        red_button_pressed = (gpio_get_level(BUTTON_RED) == 0) && red_button_enabled;

        // 2. Обработка состояний
        if (red_button_pressed && current_state != STATE_SHOOTING) {
            current_state = STATE_SHOOTING;
            current_frame = 0;
            frame_counter = 0;
            red_button_enabled = false;
        }
        else if (current_state == STATE_SHOOTING) {
            if (frame_counter++ >= shot_frame_delay) {
                frame_counter = 0;
                current_frame++;
                
                if (current_frame >= sizeof(fighter_shot_frames)/sizeof(fighter_shot_frames[0])) {
                    current_state = STATE_IDLE;
                    current_frame = 0;
                    red_button_enabled = true;
                }
            }
        } 
        else if (left_pressed || right_pressed) {
            current_state = STATE_MOVING;
            
            if (left_pressed) {
                // Движение влево
                if (fighter_x > move_speed) {
                    fighter_x -= move_speed;
                } else if (fighter_x > 0) {
                    fighter_x = 0;
                }
                
                // Скроллим фон влево, если персонаж у левого края
                if (fighter_x <= scroll_threshold) {
                    global_scroll_offset -= scroll_speed; // МИНУС - фон движется влево
                }
            }
            else if (right_pressed) {
                // Движение вправо
                int max_x = DISPLAY_WIDTH - fighter_move_frames[0]->width;
                if (fighter_x < max_x - move_speed) {
                    fighter_x += move_speed;
                } else if (fighter_x < max_x) {
                    fighter_x = max_x;
                }
                
                // Скроллим фон вправо, если персонаж у правого края
                if (fighter_x >= max_x - scroll_threshold) {
                    global_scroll_offset += scroll_speed; // ПЛЮС - фон движется вправо
                }
            }
            
            // Циклический скролл фона
            int total_width = image_background.width + image_background2.width + image_background3.width;
            if (global_scroll_offset >= total_width) {
                global_scroll_offset -= total_width;
            } else if (global_scroll_offset < 0) {
                global_scroll_offset += total_width;
            }
            
            if (frame_counter++ >= move_frame_delay) {
                frame_counter = 0;
                current_frame = (current_frame + 1) % (sizeof(fighter_move_frames)/sizeof(fighter_move_frames[0]));
            }
        }
        else {
            current_state = STATE_IDLE;
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

        // 5. Отрисовка экрана
        render_display(spi);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void game_sonic(spi_device_handle_t spi) {
    // Инициализация GPIO для кнопок
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_LEFT) | (1ULL << BUTTON_RIGHT) | (1ULL << BUTTON_RED),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    xTaskCreate(task_animation, "animation_task", 4096, spi, 1, NULL);
}
