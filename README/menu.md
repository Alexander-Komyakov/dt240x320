Для работы меню есть два файла: menu.c menu.h

Всего 6 слотов для игр.

1. Создаем превью для игры 65x60 и конвертируем согласно инструкции: https://gitlab.3dprintlite.ru/infra/converter-bmp-array/-/blob/main/README.md?ref_type=heads
2. Выбираем свободный слот и запоминаем его номер. Считаем слева направо, сверху вниз от нуля, а то есть 0 1 2 3 4 5.
3. В коде menu.c изменяем соответствующую картинку:
```
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
    draw_image(spi, &image_pong_preview);

    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 224;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.y = 140;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 30;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_GAME_COLOR);
```
Изменили второе превью на новое в соответствии с названием картинки из первого этапа конвертации
```
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
    draw_image(spi, &image_pong_preview);

    image_pong_preview.x = 127;
    draw_image(spi, &image_arkanoid_preview);
    draw_border(spi, &image_arkanoid_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 224;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.y = 140;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 30;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_GAME_COLOR);
```

В main.c добавляем вызов функции по нашему номеру пункта в menu. Аналогично `game_pong();`
