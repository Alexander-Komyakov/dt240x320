Для сохранения в играх мы используем простой подход с переменными.
Сохраняем и выгружаем значения переменных типа uint8_t, uint16_t, uint32_t и char [32]
Для добавления этого функционала в хидер файле делаем `#include "saves.h"`
Сохранение: 
```
uint8_t level;
save_nvs_u8("level", level);`
```
void save_nvs_u8(const char* key, uint8_t value);
void save_nvs_u16(const char* key, uint16_t value);
void save_nvs_u32(const char* key, uint32_t value);
void save_nvs_str(const char* key, const char* value);

Загрузка:
```
uint8_t level = load_nvs_u8("level");

char level[32];
load_nvs_str("level", &level);
```
uint8_t load_nvs_u8(const char* strnumber);
uint16_t load_nvs_u16(const char* strnumber);
uint32_t load_nvs_u32(const char* strnumber);
bool load_nvs_str(const char* key, char* out_value);

