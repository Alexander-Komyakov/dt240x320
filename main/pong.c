#include "pong.h"
#include <string.h>
#include "driver/gpio.h"
#include <stdlib.h>

// Простой шрифт 5x8 (цифры и некоторые буквы)
const uint8_t font_5x8[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7F, 0x09, 0x09, 0x09, 0x7F}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // :
};

// Функция вывода символа
void draw_char(spi_device_handle_t spi, uint16_t x, uint16_t y, char c, uint16_t color) {
    uint8_t index = 0;
    
    if (c >= '0' && c <= '9') {
        index = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        index = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        index = c - 'a' + 10;
    } else if (c == ' ') {
        index = 36;
    } else if (c == '!') {
        index = 37;
    } else if (c == ':') {
        index = 38;
    } else {
        return;
    }

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t pixels = font_5x8[index][col];
        for (uint8_t row = 0; row < 8; row++) {
            if (pixels & (1 << row)) {
                fill_rect(spi, x + col, y + row, 1, 1, color);
            }
        }
    }
}

// Функция вывода строки
void draw_text(spi_device_handle_t spi, uint16_t x, uint16_t y, const char *text, uint16_t color) {
    while (*text) {
        draw_char(spi, x, y, *text, color);
        x += 6;
        text++;
    }
}

// Функция проверки столкновения
bool is_colliding(struct Player a, struct Player b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void game_pong(spi_device_handle_t spi) {
restart_game:
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    // Инициализация объектов
    struct Player player = {10, DISPLAY_HEIGHT/2 - 20, 10, 40, 0xFFFF};
    struct Player bot = {DISPLAY_WIDTH - 20, DISPLAY_HEIGHT/2 - 20, 10, 40, 0xFFFF};
    struct Player ball = {DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2, 10, 10, 0xFFFF};

    uint8_t player_score = 0;
    uint8_t bot_score = 0;
    bool game_over = false;
    int received_button = 0;

    // Настройки скорости
    uint8_t speed = 3;
    int8_t ball_speed_x = 3;
    int8_t ball_speed_y = 3;
    uint8_t bot_reaction_speed = 2;
    uint8_t bot_attack_speed = 2;

    // Переменные для частичной перерисовки
    uint16_t prev_player_y = player.y;
    uint16_t prev_bot_y = bot.y;
    uint16_t prev_ball_x = ball.x;
    uint16_t prev_ball_y = ball.y;

    // Переменные для ИИ бота
    int16_t ball_target_y;
    int16_t bot_center;
    int16_t distance_to_ball;
    uint8_t current_bot_speed;
    int16_t predicted_y;

    // Переменные для обработки столкновений
    int hit_pos;

    // Переменная для отображения счета
    char score_text[32];

    // Первоначальная отрисовка
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

    while (!game_over) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            // Полностью стираем предыдущую позицию
            fill_rect(spi, player.x, prev_player_y, player.width, player.height, 0x0000);
            
            if (received_button == BUTTON_UP) {
                player.y = (player.y > speed) ? player.y - speed : 0;
            }
            else if (received_button == BUTTON_DOWN) {
                player.y = (player.y < DISPLAY_HEIGHT - player.height - speed) ? 
                          player.y + speed : DISPLAY_HEIGHT - player.height;
            }

            // Рисуем новую позицию
            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            prev_player_y = player.y;
        }

        // Улучшенный ИИ бота
        ball_target_y = ball.y + (ball.height/2);
        bot_center = bot.y + (bot.height/2);
        distance_to_ball = abs(ball_target_y - bot_center);

        if (ball_speed_x > 0) {
            current_bot_speed = (distance_to_ball > 20) ? bot_attack_speed : bot_reaction_speed;
            
            if (ball_target_y < bot_center - 5) {
                bot.y = (bot.y > current_bot_speed) ? bot.y - current_bot_speed : 0;
            } 
            else if (ball_target_y > bot_center + 5) {
                bot.y = (bot.y < DISPLAY_HEIGHT - bot.height - current_bot_speed) ? 
                        bot.y + current_bot_speed : DISPLAY_HEIGHT - bot.height;
            }
            
            // Предсказание траектории
            if (distance_to_ball < 15 && ball.x > DISPLAY_WIDTH/2) {
                predicted_y = ball.y + (ball_speed_y * (DISPLAY_WIDTH - ball.x) / ball_speed_x);
                if (predicted_y > 0 && predicted_y < DISPLAY_HEIGHT - bot.height) {
                    ball_target_y = predicted_y;
                }
            }
        }

        // Отрисовка бота с полной очисткой
        fill_rect(spi, bot.x, prev_bot_y, bot.width, bot.height, 0x0000);
        fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
        prev_bot_y = bot.y;

        // Движение мяча
        prev_ball_x = ball.x;
        prev_ball_y = ball.y;
        ball.x += ball_speed_x;
        ball.y += ball_speed_y;

        // Проверка столкновений
        if (is_colliding(player, ball)) {
            ball.x = player.x + player.width;
            ball_speed_x = abs(ball_speed_x);
            hit_pos = (ball.y + ball.height/2) - (player.y + player.height/2);
            ball_speed_y = hit_pos / 3;
            if (ball_speed_y > 4) ball_speed_y = 4;
            if (ball_speed_y < -4) ball_speed_y = -4;
        }

        if (is_colliding(bot, ball)) {
            ball.x = bot.x - ball.width;
            ball_speed_x = -abs(ball_speed_x);
            hit_pos = (ball.y + ball.height/2) - (bot.y + bot.height/2);
            ball_speed_y = hit_pos / 2;
            if (ball_speed_y > 5) ball_speed_y = 5;
            if (ball_speed_y < -5) ball_speed_y = -5;
        }

        // Отскок от границ
        if (ball.y <= 0 || ball.y + ball.height >= DISPLAY_HEIGHT) {
            ball_speed_y = -ball_speed_y;
        }

        // Обработка гола
        if (ball.x <= 0 || ball.x + ball.width >= DISPLAY_WIDTH) {
            if (ball.x <= 0) bot_score++;
            else player_score++;

            snprintf(score_text, sizeof(score_text), "SCORE %d:%d", player_score, bot_score);
            fill_screen(spi, 0x0000);
            draw_text(spi, DISPLAY_WIDTH/2 - 30, DISPLAY_HEIGHT/2 - 10, score_text, 0xFFFF);
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            // Сброс позиций
            ball.x = DISPLAY_WIDTH/2;
            ball.y = DISPLAY_HEIGHT/2;
            player.y = DISPLAY_HEIGHT/2 - player.height/2;
            bot.y = DISPLAY_HEIGHT/2 - bot.height/2;
            
            // Обновляем предыдущие позиции после сброса
            prev_player_y = player.y;
            prev_bot_y = bot.y;
            prev_ball_x = ball.x;
            prev_ball_y = ball.y;

            ball_speed_x = (ball.x <= 0) ? 2 : 2;
            ball_speed_y = (rand() % 5) - 2;

            // Полная перерисовка
            fill_screen(spi, 0x0000);
            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
            fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
            
            continue;
        }

        // Проверка на победу
        if (player_score >= 10 || bot_score >= 10) {
            fill_screen(spi, 0x0000);
            if (player_score >= 10) {
                draw_text(spi, DISPLAY_WIDTH/2 - 40, DISPLAY_HEIGHT/2 - 20, "Povezlo, Povezlo...!", 0x07E0);
            } else {
                draw_text(spi, DISPLAY_WIDTH/2 - 10, DISPLAY_HEIGHT/2 - 20, "SOSAL!", 0xF800);
            }
            
            draw_text(spi, DISPLAY_WIDTH/2 - 100, DISPLAY_HEIGHT/2 + 10, "Press RED/A Button to restart game", 0xFFFF);
            
            // Ожидание нажатия кнопки RED
            while (1) {
                if (gpio_get_level(BUTTON_RED) == 0) {
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                    if (gpio_get_level(BUTTON_RED) == 0) {
                        vStreamBufferDelete(xStreamBuffer);
                        goto restart_game;
                    }
                }
                vTaskDelay(50 / portTICK_PERIOD_MS);
            }
        }

        // Отрисовка мяча
        fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
        fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
