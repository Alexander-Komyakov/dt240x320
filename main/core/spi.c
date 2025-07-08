#include "spi.h"


void reset_display(void) {
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void init_gpio_display() {
	gpio_reset_pin( PIN_NUM_CS );
	gpio_set_direction( PIN_NUM_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( PIN_NUM_CS, 0 );

	gpio_reset_pin( PIN_NUM_DC );
	gpio_set_direction( PIN_NUM_DC, GPIO_MODE_OUTPUT );
    gpio_set_level( PIN_NUM_DC, 0 );

	gpio_reset_pin( PIN_NUM_RST );
	gpio_set_direction( PIN_NUM_RST, GPIO_MODE_OUTPUT );
}

void spi_init(spi_device_handle_t *spi) {
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 153600, //320*240*2
    };

    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Failed to initialize bus: %s", esp_err_to_name(ret));
    }

    spi_device_interface_config_t spicfg = {
        .clock_speed_hz = 40000000,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
        .mode = 0,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ret = spi_bus_add_device(HSPI_HOST, &spicfg, spi);
    if (ret != ESP_OK) { printf("SPI bus add device failed!\n");
    }
}

void send_command(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = 8;
    trans.tx_buffer = &cmd;
    gpio_set_level(PIN_NUM_DC, 0);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending command: %s (0x%02X)\n", esp_err_to_name(ret), cmd);
    }
}

void send_command_no_dc(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = 8;
    trans.tx_buffer = &cmd;
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending command: %s (0x%02X)\n", esp_err_to_name(ret), cmd);
    }
}

void send_data(spi_device_handle_t spi, const uint8_t *data, size_t length) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = length*8; // Длина в битах
    trans.tx_buffer = data; // Указатель на данные для передачи
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending data: %s\n", esp_err_to_name(ret));
    }
}

void send_data16b(spi_device_handle_t spi, const uint16_t *data, size_t length) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = length*16; // Длина в битах
    trans.tx_buffer = data; // Указатель на данные для передачи
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending data: %s\n", esp_err_to_name(ret));
    }
}
