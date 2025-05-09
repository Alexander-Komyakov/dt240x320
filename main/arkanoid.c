#include "arkanoid.h"

// Массив цветов кирпичей
const uint16_t brick_colors[BRICK_ROWS] = {
    0xF800, 0x07E0, 0x001F, 0xFFE0, 
    0xF81F, 0x07FF, 0xAFE5, 0xFF07
};

static bool is_colliding(GameObject a, GameObject b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void print_bricks_matrix(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    printf("Матрица кирпичей:\n");
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            printf("%c", bricks[row][col].active ? (bricks[row][col].unbreakable ? '#' : 'X') : '.');
        }
        printf("\n");
    }
}


void generate_random_bricks(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    // Очистка поля
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].active = false;
            bricks[row][col].unbreakable = false;
        }
    }

    // Генерация 1 фигуры (тестовый режим)
    int shapes = 1;
    printf("\n=== Новый раунд ===\nГенерируем %d фигур:\n", shapes);

    // Сначала генерируем фигуры
    for (int s = 0; s < shapes; s++) {
//        int start_row = rand() % (BRICK_ROWS - 6);
//        int start_col = rand() % (BRICK_COLS - 8);

		int max_figure_height = 8; // Максимальная высота самой большой фигуры
		int max_figure_width = 6;  // Максимальная ширина самой большой фигуры
		int start_row = rand() % (BRICK_ROWS - max_figure_height + 1);
		int start_col = rand() % (BRICK_COLS - max_figure_width + 1);
        ShapeType shape_type = rand() % SHAPE_TOTAL_COUNT;

        switch (shape_type) {
            case SHAPE_SNAKE: {
                if (rand() % 2 == 0) {
			        printf("%d. Змейка (классическая) в [%d,%d]\n", s+1, start_row, start_col);
			        
			        // Шаблон классической змейки (17 точек)
			        int pattern[][2] = {
			            // Горизонтальная часть 1 (6 блоков вправо)
			            {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5},
			            // Вертикальная часть 1 (2 блока вниз)
			            {1,5}, {2,5},
			            // Горизонтальная часть 2 (6 блоков влево)
			            {2,4}, {2,3}, {2,2}, {2,1}, {2,0},
			            // Вертикальная часть 2 (2 блока вниз)
			            {3,0}, {4,0},
			            // Хвост (3 блока вправо)
			            {4,1}, {4,2}
			        };
			        
			        int points_count = sizeof(pattern) / sizeof(pattern[0]);
			        
			        for (int i = 0; i < points_count; i++) {
			            int r = start_row + pattern[i][0];
			            int c = start_col + pattern[i][1];
			            if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			                bricks[r][c].active = true;
			            }
			        }
	            } else {
                    printf("%d. Змейка (компактная) в [%d,%d]\n", s+1, start_row, start_col);
                    // Компактная змейка
                    int pattern[5][2] = {{0,0}, {0,1}, {1,1}, {1,2}, {2,2}};
                    for (int i = 0; i < 5; i++) {
                        int r = start_row + pattern[i][0];
                        int c = start_col + pattern[i][1];
						if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) { // !bricks - проверка чтобы блоки не создавались
						    bricks[r][c].active = true;                                 // там где они уже созданы при генерации
						}
                    }
                }
                break;
            }
            case HORIZONTAL_LINE: {
				int length = 4 + rand() % 4; // Случайная длина от 4 до 7
			    printf("%d. Горизонтальная линия (длина %d) в [%d,%d]\n", 
			          s+1, length, start_row, start_col);

			    for (int i = 0; i < length; i++) {
			        int r = start_row;
			        int c = start_col + i;
			        if (c < BRICK_COLS && !bricks[r][c].active) {  // Проверка границ
			            bricks[r][c].active = true;
			        }
			    }
			    break;
            }
            case VERTICAL_LINE: {
                int length = 4 + rand() % 4; // Случайная длина от 4 до 7
                printf("%d. Вертикальная линия (длина %d) в [%d,%d]\n", 
                      s+1, length, start_row, start_col);

                for (int i = 0; i < length; i++) {
                    int r = start_row + i;
                    int c = start_col;
                    if (c < BRICK_ROWS && !bricks[r][c].active) {  // Проверка границ
                        bricks[r][c].active = true;
                    }
                }
                break;
            }
			case SHAPE_SQUARE: {
			    int size = 3 + rand() % 4; // Случайный размер от 3x3 до 6x6
			    printf("%d. Квадрат %dx%d в [%d,%d]\n", s+1, size, size, start_row, start_col);
			    
			    for (int i = 0; i < size; i++) {
			        for (int j = 0; j < size; j++) {
			            int r = start_row + i;
			            int c = start_col + j;
			            if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			                bricks[r][c].active = true;
			            }
			        }
			    }
			    break;
			}
			case SHAPE_SPIDER: {
			    printf("%d. Паук в [%d,%d]\n", s+1, start_row, start_col);
			    
			    // Вертикальная линия (8 блоков)
			    for (int i = 0; i < 8; i++) {
			        int r = start_row + i;
			        int c = start_col + 2; // Центральная колонка
			        if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }

			    // Горизонтальные линии (по 3 блока влево и вправо)
			    // Нижняя (1-я позиция снизу)
			    for (int j = -1; j <= 1; j++) {
			        int r = start_row + 0;
			        int c = start_col + 2 + j;
			        if (r < BRICK_ROWS && c >= 0 && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    // Средняя (3-я позиция снизу)
			    for (int j = -1; j <= 1; j++) {
			        int r = start_row + 2;
			        int c = start_col + 2 + j;
			        if (r < BRICK_ROWS && c >= 0 && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    // Верхняя (5-я позиция снизу)
			    for (int j = -1; j <= 1; j++) {
			        int r = start_row + 4;
			        int c = start_col + 2 + j;
			        if (r < BRICK_ROWS && c >= 0 && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    break;
			}
			case SHAPE_LADDER: {
			    printf("%d. Лесенка в [%d,%d]\n", s+1, start_row, start_col);
			    
			    int pattern[][2] = {
			        {0,0}, {1,0}, {1,1}, {2,1}, {2,2}, {3,2}, {3,3}
			    };
			    int points_count = sizeof(pattern) / sizeof(pattern[0]);
			    
			    for (int i = 0; i < points_count; i++) {
			        int r = start_row + pattern[i][0];
			        int c = start_col + pattern[i][1];
			        if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    break;
			}
			case SHAPE_CROSS: {
			    printf("%d. Крест в [%d,%d]\n", s+1, start_row, start_col);
			    
			    // Вертикальная линия (5 блоков)
			    for (int i = 0; i < 5; i++) {
			        int r = start_row + i;
			        int c = start_col + 2;
			        if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    
			    // Горизонтальная линия (5 блоков)
			    for (int j = 0; j < 5; j++) {
			        int r = start_row + 2;
			        int c = start_col + j;
			        if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			            bricks[r][c].active = true;
			        }
			    }
			    break;
			}
			case SHAPE_TRIANGLE: {
			    printf("%d. Треугольник в [%d,%d]\n", s+1, start_row, start_col);
			    
			    int size = 3 + rand() % 3; // Размер от 3 до 5
			    for (int i = 0; i < size; i++) {
			        for (int j = 0; j <= i; j++) {
			            int r = start_row + i;
			            int c = start_col + j;
			            if (r < BRICK_ROWS && c < BRICK_COLS && !bricks[r][c].active) {
			                bricks[r][c].active = true;
			            }
			        }
			    }
			    break;
			}
			case SHAPE_SNOWFLAKE: {
			    printf("%d. Снежинка в [%d,%d]\n", s+1, start_row, start_col);
			    
			    // Центральный блок
			    int center_r = start_row + 2;
			    int center_c = start_col + 2;
			    if (center_r < BRICK_ROWS && center_c < BRICK_COLS && !bricks[center_r][center_c].active) {
			        bricks[center_r][center_c].active = true;
			    }
			    
			    // Лучи (по 3 блока в каждом направлении)
			    for (int i = 0; i < 3; i++) {
			        // Верхний луч
			        if (center_r - i >= 0) {
			            bricks[center_r - i][center_c].active = true;
			        }
			        // Нижний луч
			        if (center_r + i < BRICK_ROWS) {
			            bricks[center_r + i][center_c].active = true;
			        }
			        // Левый луч
			        if (center_c - i >= 0) {
			            bricks[center_r][center_c - i].active = true;
			        }
			        // Правый луч
			        if (center_c + i < BRICK_COLS) {
			            bricks[center_r][center_c + i].active = true;
			        }
			        // Диагональные лучи
			        if (center_r - i >= 0 && center_c - i >= 0) {
			            bricks[center_r - i][center_c - i].active = true;
			        }
			        if (center_r - i >= 0 && center_c + i < BRICK_COLS) {
			            bricks[center_r - i][center_c + i].active = true;
			        }
			        if (center_r + i < BRICK_ROWS && center_c - i >= 0) {
			            bricks[center_r + i][center_c - i].active = true;
			        }
			        if (center_r + i < BRICK_ROWS && center_c + i < BRICK_COLS) {
			            bricks[center_r + i][center_c + i].active = true;
			        }
			    }
			    break;
			}
            default: {
                printf("%d. Случайный блок в [%d,%d]\n", s+1, start_row, start_col);
                // Генерируем случайный блок 2x2
                for (int i = 0; i < 2 && (start_row + i) < BRICK_ROWS; i++) {
                    for (int j = 0; j < 2 && (start_col + j) < BRICK_COLS; j++) {
  				    	if (!bricks[start_row + i][start_col + j].active) {
			    	        bricks[start_row + i][start_col + j].active = true;
				        }
                    }
                }
                break;
            }
        }
    }

    // Теперь генерируем неразрушаемые блоки, ИСКЛЮЧАЯ активные фигуры
    int unbreakable_count = (BRICK_ROWS * BRICK_COLS) * (0.1 + (rand() % 11) * 0.01);
    for (int i = 0; i < unbreakable_count; i++) {
        int row, col;
        do {
            row = rand() % BRICK_ROWS;
            col = rand() % BRICK_COLS;
        } while (bricks[row][col].active); // Повторяем, пока не найдём неактивный кирпич

        bricks[row][col].active = true;
        bricks[row][col].unbreakable = true;
    }

    // Установка параметров кирпичей (цвета и координаты)
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
                bricks[row][col].y = BRICK_TOP_MARGIN + row * (BRICK_HEIGHT + BRICK_MARGIN);
                bricks[row][col].width = BRICK_WIDTH;
                bricks[row][col].height = BRICK_HEIGHT;
                bricks[row][col].color = bricks[row][col].unbreakable ? 0x7BEF : brick_colors[row % BRICK_ROWS];
            }
        }
    }

    printf("==================\n");
    print_bricks_matrix(bricks);
}


void draw_bricks(spi_device_handle_t spi, Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                fill_rect(spi, bricks[row][col].x, bricks[row][col].y, 
                         bricks[row][col].width, bricks[row][col].height, 
                         bricks[row][col].color);
            }
        }
    }
}

bool all_bricks_destroyed(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active && !bricks[row][col].unbreakable) {
                return false;
            }
        }
    }
    return true;
}

void show_round_screen(spi_device_handle_t spi, uint16_t round, uint8_t lives, float speed) {
    fill_screen(spi, 0x0000);

    // Базовый текст "РАУНД "
    const uint16_t round_text_prefix[] = {u'Р', u'А', u'У', u'Н', u'Д', u' ', 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2 - 20, round_text_prefix, 0xFFFF);
    
    // Отображение номера раунда (1 или 2 цифры)
    if (round < 10) {
        const uint16_t round_num[] = {u'0' + round, 0};
        draw_text(spi, DISPLAY_WIDTH/2 + 15, DISPLAY_HEIGHT/2 - 20, round_num, 0xFFFF);
    } else {
        const uint16_t round_num[] = {u'0' + (round / 10), u'0' + (round % 10), 0};
        draw_text(spi, DISPLAY_WIDTH/2 + 15, DISPLAY_HEIGHT/2 - 20, round_num, 0xFFFF);
    }


/*    
    const uint16_t round_text[] = {u'Р', u'А', u'У', u'Н', u'Д', u' ', u'0' + round, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2 - 20, round_text, 0xFFFF);

*/
    const uint16_t lives_text[] = {u'Ж', u'И', u'З', u'Н', u'И', u':', u'0' + lives, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2, lives_text, 0xFFFF);
    
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void game_arkanoid(spi_device_handle_t spi) {
    unsigned int seed = xTaskGetTickCount();
    printf("Инициализация генератора случайных чисел: seed=%u\n", seed);
    srand(seed);
    
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    
    Brick bricks[BRICK_ROWS][BRICK_COLS];
    GameObject player = {DISPLAY_WIDTH/2 - 20, 230, 40, 10, 0xFFFF};
    GameObject ball = {DISPLAY_WIDTH/2, player.y - 10, 10, 10, 0xFFFF};

	int received_button = 0;         // Код полученной кнопки
	uint8_t speed = 3;              // Скорость движения платформы
	float ball_speed_x = 0;         // Горизонтальная скорость мяча
	float ball_speed_y = 0;         // Вертикальная скорость мяча
	float base_speed = 1.5f;        // Базовая скорость мяча
	uint8_t speed_hits = 0;         // Счётчик ударов для увеличения скорости
	uint16_t prev_player_x = player.x;  // Предыдущая позиция платформы (для отрисовки)
	int16_t prev_ball_x = ball.x;     // Предыдущая X-позиция мяча
	int16_t prev_ball_y = ball.y;     // Предыдущая Y-позиция мяча
	uint8_t lives = 5;               // Количество жизней
	uint16_t round = 1;               // Текущий раунд
	bool ball_active = false;        // Флаг, что мяч в движении
	bool red_button_enabled = true;  // Флаг активности красной кнопки
	bool game_paused = false;       // Флаг паузы

    // Первый раунд - стандартное расположение
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
            bricks[row][col].y = row * (BRICK_HEIGHT + BRICK_MARGIN) + BRICK_TOP_MARGIN;
            bricks[row][col].width = BRICK_WIDTH;
            bricks[row][col].height = BRICK_HEIGHT;
            bricks[row][col].color = brick_colors[row];
            bricks[row][col].active = true;
            bricks[row][col].unbreakable = (col % 5 == 0); // Каждый 5 ряд неразбиваемый
            if (bricks[row][col].unbreakable) {
                bricks[row][col].color = 0x7BEF;
            }
        }
    }
    
    show_round_screen(spi, round, lives, base_speed);
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
    draw_bricks(spi, bricks);

    while (1) {
        // Обработка паузы
        if (gpio_get_level(BUTTON_WHITE) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            if (gpio_get_level(BUTTON_WHITE) == 0) {
                game_paused = !game_paused;
                
                if (game_paused) {
                    fill_screen(spi, 0x0000);
                    draw_text(spi, DISPLAY_WIDTH/2 - 10, DISPLAY_HEIGHT/2 - 20, 
                             u"ПАУЗА", 0xFFFF);
                    draw_text(spi, DISPLAY_WIDTH/2 - 115, DISPLAY_HEIGHT/2 + 30, 
                             u"ЧТОБЫ ПРОДОЛЖИТЬ НАЖМИТЕ БЕЛУЮ КНОПКУ", 0xFFFF);
                } else {
                    fill_screen(spi, 0x0000);
                    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    draw_bricks(spi, bricks);
                }
                
                while (gpio_get_level(BUTTON_WHITE) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        // Пропуск раунда
        if (gpio_get_level(BUTTON_YELLOW) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            if (gpio_get_level(BUTTON_YELLOW) == 0) {
                for (int row = 0; row < BRICK_ROWS; row++) {
                    for (int col = 0; col < BRICK_COLS; col++) {
                        if (!bricks[row][col].unbreakable) {
                            bricks[row][col].active = false;
                        }
                    }
                }
                while (gpio_get_level(BUTTON_YELLOW) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        if (game_paused) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // Основной игровой цикл
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_LEFT) {
                player.x = (player.x > speed) ? player.x - speed : 0;
                fill_rect(spi, player.x + player.width, player.y, prev_player_x - player.x, player.height, 0x0000);
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RIGHT) {
                player.x = (player.x < DISPLAY_WIDTH - player.width - speed) ?
                          player.x + speed : DISPLAY_WIDTH - player.width;
                fill_rect(spi, prev_player_x, player.y, player.x - prev_player_x, player.height, 0x0000);
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RED && red_button_enabled) {
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = base_speed;
                    ball_speed_y = -base_speed;
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    ball_active = true;
                    red_button_enabled = false;
                }
            }

            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            prev_player_x = player.x;
        }

		// Движение мяча
        if (ball_active) {
		    prev_ball_x = ball.x;  // Запоминаем, где был мячик
		    prev_ball_y = ball.y;  // (чтобы потом стереть его след)
		    ball.x += (int)ball_speed_x;  // Двигаем мячик вправо/влево
		    ball.y += (int)ball_speed_y;  // Двигаем мячик вверх/вниз


            // Обработка столкновений мяча с кирпичами
			bool brick_hit = false;  // Пока не знаем, попали или нет
			for (int row = 0; row < BRICK_ROWS; row++) {  // Проверяем все ряды кирпичиков
			    for (int col = 0; col < BRICK_COLS; col++) {  // Проверяем все кирпичики в ряду
			        if (bricks[row][col].active &&  // Если кирпичик есть...
			            is_colliding(ball, (GameObject){  // ...и мячик его задел
			                bricks[row][col].x, 
			                bricks[row][col].y, 
			                bricks[row][col].width, 
			                bricks[row][col].height, 
			                0})) {
			            brick_hit = true;  // Ура, попали!
			                        
                        //  Запоминаем, откуда прилетел мячик
                        float prev_ball_x = ball.x - ball_speed_x; // Где был мячик до удара?
                        float prev_ball_y = ball.y - ball_speed_y;
                        
						// Обработка столкновения с кирпичом
						if (bricks[row][col].unbreakable) {
						    // Для неразрушаемых блоков просто делаем отскок
						    // Нет необходимости перерисовывать - они не меняются
						} else {
						    // Разрушаемые блоки - деактивируем и стираем
						    bricks[row][col].active = false;
						    fill_rect(spi, bricks[row][col].x, bricks[row][col].y,
						             bricks[row][col].width, bricks[row][col].height,
						             0x0000);
						}

						// Определяем, откуда прилетел мячик
                        bool from_left = prev_ball_x + ball.width <= bricks[row][col].x;  // Слева?
                        bool from_right = prev_ball_x >= bricks[row][col].x + bricks[row][col].width;  // Справа?
                        bool from_top = prev_ball_y + ball.height <= bricks[row][col].y;  // Сверху?
                        bool from_bottom = prev_ball_y >= bricks[row][col].y + bricks[row][col].height;  // Снизу?


                        // Гибридная система обработки столкновений
                        if (from_left || from_right) {
                            // Горизонтальное столкновение
                            if (from_left) {
                                ball.x = bricks[row][col].x - ball.width - 0.3f; // Отскакивает влево
                            } else {
                                ball.x = bricks[row][col].x + bricks[row][col].width + 0.3f; // Отскакивает вправо
                            }
                            ball_speed_x = -ball_speed_x; // Меняем направление скорости
                        }
                        
                        if (from_top || from_bottom) {
                            // Вертикальное столкновение
                            if (from_top) {
                                ball.y = bricks[row][col].y - ball.height - 0.3f; // Отскакивает вверх
								if ((int16_t)ball.y < 0) ball.y = 0; // Не вылетает за верх экрана
                            } else {
                                ball.y = bricks[row][col].y + bricks[row][col].height + 0.3f; // Отскакивает вниз
                            }
                            ball_speed_y = -ball_speed_y; // Меняем направление скорости
                        }

                        // Обработка угловых столкновений (когда не ясно направление)
                        if (!(from_left || from_right || from_top || from_bottom)) {
                            // Используем более мягкую версию углового отскока
                            float dx = (ball.x + ball.width/2) - (bricks[row][col].x + bricks[row][col].width/2);
                            float dy = (ball.y + ball.height/2) - (bricks[row][col].y + bricks[row][col].height/2);
                            
                            if (fabs(dx) > fabs(dy)) {
                                if (dx > 0) {
                                    ball.x = bricks[row][col].x + bricks[row][col].width + 0.3f;
                                } else {
                                    ball.x = bricks[row][col].x - ball.width - 0.3f;
                                    if ((int16_t)ball.x < 0) ball.x = 0;
                                }
                                ball_speed_x = -ball_speed_x;
                            } else {
                                if (dy > 0) {
                                    ball.y = bricks[row][col].y + bricks[row][col].height + 0.3f;
                                } else {
                                    ball.y = bricks[row][col].y - ball.height - 0.3f;
									if ((int16_t)ball.y < 0) ball.y = 0;
                                }
                                ball_speed_y = -ball_speed_y;
                            }
                        }

                        // Ограничение скорости после столкновения
                        const float MAX_BALL_SPEED = 5.0f;
                        if (fabs(ball_speed_x) > MAX_BALL_SPEED) {
                            ball_speed_x = copysignf(MAX_BALL_SPEED, ball_speed_x);
                        }
                        if (fabs(ball_speed_y) > MAX_BALL_SPEED) {
                            ball_speed_y = copysignf(MAX_BALL_SPEED, ball_speed_y);
                        }

                        // После обработки столкновения выходим из циклов
                        goto collision_processed;
                    }
                }
            }
            collision_processed:

/*
			// Проверка координаты движения мяча
			printf ("x: %d, y: %d\n", ball.x, ball.y);
*/

            // Если было столкновение с кирпичом, перерисовываем мяч в новой позиции
            if (brick_hit) {
                fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                prev_ball_x = ball.x;
                prev_ball_y = ball.y;
                continue; // Пропускаем остальную обработку для этого кадра
            }

            if (is_colliding(player, ball)) {
                ball.y = player.y - ball.height;
                int hit_pixel = (ball.x + ball.width/2) - (player.x + player.width/2);
                float hit_norm = (float)hit_pixel / (player.width/2.0f);
                const float center_speed = 1.25f;
                const float edge_speed = 2.25f;
                const float speed_range = edge_speed - center_speed;
                float speed_factor = fabsf(hit_norm);
                float current_speed = center_speed + (speed_range * speed_factor);
                ball_speed_x = copysignf(current_speed, hit_norm);
                ball_speed_y = -(1.25f + speed_factor * 0.75f);
                
                speed_hits++;
                if (speed_hits % SPEED_INCREASE_INTERVAL == 0) {
                    base_speed += 0.25f;
                }
            }


            // Границы экрана
			if (ball.y <= 0) {
			    ball.y = 0; // Фиксация для верхней границы (по аналогии)
			    ball_speed_y = fabs(ball_speed_y);
			}

			if (ball.x <= 0) {
			    ball.x = 0;
			    ball_speed_x = -ball_speed_x;
			} 
			else if (ball.x + ball.width >= DISPLAY_WIDTH) {
			    ball.x = DISPLAY_WIDTH - ball.width;
			    ball_speed_x = -ball_speed_x;
			}

            // Проверка проигрыша
            if (ball.y + ball.height >= DISPLAY_HEIGHT) {
                fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                fill_rect(spi, ball.x, ball.y, ball.width, ball.height, 0x0000);
                
                lives--;
                fill_rect(spi, DISPLAY_WIDTH - 30, 10, 20, 8, 0x0000);
                const uint16_t lives_count[] = {u'0' + lives, 0};
                draw_text(spi, DISPLAY_WIDTH - 30, 10, lives_count, 0xFFFF);
                
                if (lives > 0) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = 0;
                    ball_speed_y = 0;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    ball_active = false;
                    red_button_enabled = true;
                    base_speed = 1.5f;
                    speed_hits = 0;
                } else {
                    const uint16_t game_over[] = {u'К', u'О', u'Н', u'Е', u'Ц', u' ', u'И', u'Г', u'Р', u'Ы', 0};
                    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2, game_over, 0xFFFF);
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    return;
                }
            }
            else {
                if (all_bricks_destroyed(bricks)) {
                    round++;
                    speed_hits = 0;
                    
                    generate_random_bricks(bricks);
                    show_round_screen(spi, round, lives, base_speed);
                    
                    player.x = DISPLAY_WIDTH/2 - 20;
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

                    ball_speed_x = 0;
                    ball_speed_y = 0;
                    ball_active = false;
                    red_button_enabled = true;
                    
                    fill_screen(spi, 0x0000);
                    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    draw_bricks(spi, bricks);
                }
                else {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                }
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
