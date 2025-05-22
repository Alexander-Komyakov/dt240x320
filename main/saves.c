#include "saves.h"
#include "esp_log.h"


// Создаём очередь
//static QueueHandle_t nvs_queue = xQueueCreate(4, sizeof(nvs_data_t));
//static nvs_handle_t nvs_handle_storage;
//static QueueHandle_t nvs_queue;
//static nvs_handle_t nvs_handle_storage;
QueueHandle_t nvs_queue;
nvs_handle_t nvs_handle_storage;

void nvs_init() {
    esp_err_t ret;
    nvs_flash_deinit();
    // Инициализация с восстановлением
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    printf("open nvs\n");
    nvs_open("gamesaved", NVS_READWRITE, &nvs_handle_storage);
    nvs_queue = xQueueCreate(4, sizeof(nvs_data_t));
}

void storage_task(void* arg) {
    nvs_init();
    while (1) {
        nvs_data_t data;
        if (xQueueReceive(nvs_queue, &data, portMAX_DELAY)) {
            if (data.type == 0) {
                nvs_set_u8(nvs_handle_storage, data.key, data.value.u8);
            } else if (data.type == 1) {
                nvs_set_u16(nvs_handle_storage, data.key, data.value.u16);
            } else if (data.type == 2) {
                nvs_set_u32(nvs_handle_storage, data.key, data.value.u32);
            } else if (data.type == 3) {
                nvs_set_str(nvs_handle_storage, data.key, data.value.str);
            }
            nvs_commit(nvs_handle_storage);
        }
    }
    
    nvs_close(nvs_handle_storage);
    vTaskDelete(NULL);
}

void save_nvs_u8(const char* key, uint8_t value) {
    nvs_data_t data = {
        .type = 0,
        .value.u8 = value
    };
    strncpy(data.key, key, sizeof(data.key) - 1);
    data.key[sizeof(data.key) - 1] = '\0';
    xQueueSend(nvs_queue, &data, portMAX_DELAY);
}

void save_nvs_u16(const char* key, uint16_t value) {
    nvs_data_t data = {
        .type = 1,
        .value.u16 = value
    };
    strncpy(data.key, key, sizeof(data.key) - 1);
    data.key[sizeof(data.key) - 1] = '\0';
    xQueueSend(nvs_queue, &data, portMAX_DELAY);
}

void save_nvs_u32(const char* key, uint32_t value) {
    nvs_data_t data = {
        .type = 2,
        .value.u32 = value
    };
    strncpy(data.key, key, sizeof(data.key) - 1);
    data.key[sizeof(data.key) - 1] = '\0';
    xQueueSend(nvs_queue, &data, portMAX_DELAY);
}

void save_nvs_str(const char* key, const char* value) {
    nvs_data_t data = {
        .type = 3  // Тип "строка"
    };
    
    // Копируем ключ
    strncpy(data.key, key, sizeof(data.key) - 1);
    data.key[sizeof(data.key) - 1] = '\0';
    
    // Копируем строку
    strncpy(data.value.str, value, MAX_STR_LEN - 1);
    data.value.str[MAX_STR_LEN - 1] = '\0';
    
    if (xQueueSend(nvs_queue, &data, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE("NVS", "Ошибка отправки в очередь");
    }
}

uint8_t load_nvs_u8(const char* strnumber) {
    uint8_t number = 0;
    // читаем наше число
    esp_err_t err = nvs_get_u8(nvs_handle_storage, strnumber, &number);
    if (err != ESP_OK) {
        // если нечего читать, то возвращаем 0
        ESP_LOGE("NVS", "Ошибка коммита: %s", esp_err_to_name(err));
        printf("ОШИБКА загрузил число с именем %s: %d\n", strnumber, number);
        return 0;
    }
    return number;
}
uint16_t load_nvs_u16(const char* strnumber) {
    uint16_t number = 0;
    // читаем наше число
    esp_err_t err = nvs_get_u16(nvs_handle_storage, strnumber, &number);
    if (err != ESP_OK) {
        // если нечего читать, то возвращаем 0
        ESP_LOGE("NVS", "Ошибка коммита: %s", esp_err_to_name(err));
        printf("ОШИБКА загрузил число с именем %s: %d\n", strnumber, number);
        return 0;
    }
    return number;
}
uint32_t load_nvs_u32(const char* strnumber) {
    uint32_t number = 0;
    // читаем наше число
    esp_err_t err = nvs_get_u32(nvs_handle_storage, strnumber, &number);
    if (err != ESP_OK) {
        // если нечего читать, то возвращаем 0
        ESP_LOGE("NVS", "Ошибка коммита: %s", esp_err_to_name(err));
        return 0;
    }
    return number;
}

bool load_nvs_str(const char* key, char* out_value) {
    size_t required_size = MAX_STR_LEN;
    esp_err_t err = nvs_get_str(nvs_handle_storage, key, out_value, &required_size);
    
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Ошибка чтения строки %s: %s", key, esp_err_to_name(err));
        out_value[0] = '\0';  // Возвращаем пустую строку при ошибке
        return false;
    }
    return true;
}
