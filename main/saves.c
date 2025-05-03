#include "saves.h"
#include "esp_log.h"


void nvs_init(nvs_handle_t* nvs_handle) {
    esp_err_t ret;
    nvs_flash_deinit();
    // Инициализация с восстановлением
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    nvs_open("gamesaved", NVS_READWRITE, nvs_handle);
}

void nvs_save_uint8_t(nvs_handle_t nvs_handle, uint8_t number, char* strnumber) {
    // записываем число
    nvs_set_u8(nvs_handle, strnumber, number);
    // выполняем комит
    nvs_commit(nvs_handle);
}

uint8_t nvs_load_uint8_t(nvs_handle_t nvs_handle, char* strnumber) {
    uint8_t number = 0;
    // читаем наше число
    esp_err_t err = nvs_get_u8(nvs_handle, strnumber, &number);
    if (err != ESP_OK) {
        // если нечего читать, то возвращаем 0
        ESP_LOGE("NVS", "Ошибка коммита: %s", esp_err_to_name(err));
        printf("ОШИБКА загрузил число с именем %s: %d\n", strnumber, number);
        return 0;
    }
    return number;
}
