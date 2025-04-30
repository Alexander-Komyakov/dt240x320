#include "pong.h"
#include "font.h"


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

    uint16_t player_score = 0;
    uint16_t bot_score = 0;
    int received_button = 0;
    uint8_t level = 0;
    uint16_t p_text[4], b_text[4];
    uint8_t p_pos = 0, b_pos = 0;

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

    // Первоначальная отрисовка
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                player.y = (player.y > speed) ? player.y - speed : 0;
                fill_rect(spi, player.x, player.y + player.height, player.width, prev_player_y - player.y, 0x0000);
            }
            else if (received_button == BUTTON_DOWN) {
                player.y = (player.y < DISPLAY_HEIGHT - player.height - speed) ? 
                          player.y + speed : DISPLAY_HEIGHT - player.height;
                fill_rect(spi, player.x, prev_player_y, player.width, player.y - prev_player_y, 0x0000);
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

        // Отрисовка бота с частичной очисткой
        if (prev_bot_y != bot.y) {
            if (bot.y > prev_bot_y) {
                // Движение вниз - стираем верхнюю часть
                fill_rect(spi, bot.x, prev_bot_y, bot.width, bot.y - prev_bot_y, 0x0000);
            } else {
                // Движение вверх - стираем нижнюю часть
                fill_rect(spi, bot.x, bot.y + bot.height, bot.width, prev_bot_y - bot.y, 0x0000);
            }
            
            // Рисуем новую позицию
            fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
            prev_bot_y = bot.y;
        }

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

            fill_screen(spi, 0x0000);
            draw_text(spi, DISPLAY_WIDTH/2 - 30, DISPLAY_HEIGHT/2 - 10, u"СЧЕТ ", 0xFFFF);

            // Формируем текст для игрока
            if (player_score >= 100) p_text[p_pos++] = u'0' + (player_score / 100);
            if (player_score >= 10 || p_pos > 0) p_text[p_pos++] = u'0' + ((player_score / 10) % 10);
            p_text[p_pos++] = u'0' + (player_score % 10);
            p_text[p_pos] = 0;
            
            // Формируем текст для бота
            if (bot_score >= 100) b_text[b_pos++] = u'0' + (bot_score / 100);
            if (bot_score >= 10 || b_pos > 0) b_text[b_pos++] = u'0' + ((bot_score / 10) % 10);
            b_text[b_pos++] = u'0' + (bot_score % 10);
            b_text[b_pos] = 0;

            draw_text(spi, DISPLAY_WIDTH/2 + 10, DISPLAY_HEIGHT/2 - 10, p_text, 0xFFFF);
            draw_text(spi, DISPLAY_WIDTH/2 + 30, DISPLAY_HEIGHT/2 - 10, b_text, 0xFFFF);
            p_pos = 0, b_pos = 0;
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            // Проверка на победу
            if (player_score == 10 || player_score == 25 || player_score == 40 ||
                bot_score == 10 || bot_score == 25 || bot_score == 40) {
                fill_screen(spi, 0x0000);
                // Поражение
                if (bot_score > player_score) {
                    draw_text(spi, DISPLAY_WIDTH/2 - 40, DISPLAY_HEIGHT/2 - 20, u"ВЫ ПРОИГРАЛИ", 0xF800);
                    draw_text(spi, DISPLAY_WIDTH/2 - 120, DISPLAY_HEIGHT/2 + 10, u"НАЖМИТЕ КРАСНУЮ КНОПКУ ДЛЯ ПЕРЕЗАПУСКА ИГРЫ", 0xFFFF);
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
                else if (player_score == 10 && level == 0) {
                    draw_text(spi, DISPLAY_WIDTH/2 - 40, DISPLAY_HEIGHT/2 - 20, u"СРЕДНЯЯ СЛОЖНОСТЬ", 0x07E0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    bot_reaction_speed = 3;
                    bot_attack_speed = 3;
                    level = 1;
                } else if (player_score == 25 && level == 1) {
                    draw_text(spi, DISPLAY_WIDTH/2 - 40, DISPLAY_HEIGHT/2 - 20, u"ВЫСОКАЯ СЛОЖНОСТЬ", 0x07E0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    speed = 4;
                    bot_reaction_speed = 4;
                    bot_attack_speed = 4;
                    level = 2;
                } else if (player_score == 40 && level == 2) {
                    draw_text(spi, DISPLAY_WIDTH/2 - 50, DISPLAY_HEIGHT/2 - 20, u"НЕВОЗМОЖНАЯ СЛОЖНОСТЬ", 0x07E0);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    speed = 5;
                    bot_reaction_speed = 5;
                    bot_attack_speed = 5;
                    level = 3;
                }
            }

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


        // Отрисовка мяча
        fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
        fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
