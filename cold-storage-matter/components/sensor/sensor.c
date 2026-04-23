/*
 * sensor.c — DHT22 / SHT31 Sensor Driver
 * Cold Storage Monitoring | DigitalMonk
 * https://digitalmonk.biz
 *
 * Select sensor type in sdkconfig:
 *   CONFIG_SENSOR_DHT22  — for DHT22
 *   CONFIG_SENSOR_SHT31  — for SHT31 (I2C)
 */

#include "sensor.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "sensor";

// -------------------------------------------------------
// Pin Configuration — Change as per your hardware wiring
// -------------------------------------------------------
#define DHT22_GPIO_PIN      GPIO_NUM_4     // DATA pin for DHT22
#define SHT31_I2C_SDA       GPIO_NUM_21    // SDA pin for SHT31
#define SHT31_I2C_SCL       GPIO_NUM_22    // SCL pin for SHT31
#define SHT31_I2C_PORT      I2C_NUM_0
#define SHT31_I2C_ADDR      0x44           // Default SHT31 I2C address
#define SHT31_I2C_FREQ      100000         // 100kHz

// -------------------------------------------------------
// SHT31 Commands
// -------------------------------------------------------
#define SHT31_CMD_MEASURE_HIGH  0x2C06     // Single shot, high repeatability
#define SHT31_CMD_SOFT_RESET    0x30A2

// -------------------------------------------------------
// DHT22 Bit-bang protocol
// -------------------------------------------------------
#define DHT22_TIMEOUT_US    100

static int dht22_wait_level(int level, int timeout_us)
{
    int elapsed = 0;
    while (gpio_get_level(DHT22_GPIO_PIN) != level) {
        if (elapsed++ > timeout_us) return -1;
        esp_rom_delay_us(1);
    }
    return elapsed;
}

static esp_err_t dht22_read_raw(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};

    // Send start signal
    gpio_set_direction(DHT22_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT22_GPIO_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT22_GPIO_PIN, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(DHT22_GPIO_PIN, GPIO_MODE_INPUT);

    // Wait for DHT22 response
    if (dht22_wait_level(0, DHT22_TIMEOUT_US) < 0) return ESP_FAIL;
    if (dht22_wait_level(1, DHT22_TIMEOUT_US) < 0) return ESP_FAIL;
    if (dht22_wait_level(0, DHT22_TIMEOUT_US) < 0) return ESP_FAIL;

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        if (dht22_wait_level(1, DHT22_TIMEOUT_US) < 0) return ESP_FAIL;
        int high_time = dht22_wait_level(0, DHT22_TIMEOUT_US);
        if (high_time < 0) return ESP_FAIL;

        data[i / 8] <<= 1;
        if (high_time > 40) data[i / 8] |= 1; // >40us = bit 1
    }

    // Checksum verification
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        ESP_LOGE(TAG, "DHT22 checksum mismatch");
        return ESP_ERR_INVALID_CRC;
    }

    *humidity    = ((data[0] << 8) | data[1]) / 10.0f;
    *temperature = (((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) *temperature = -(*temperature); // Negative temp

    return ESP_OK;
}

// -------------------------------------------------------
// SHT31 I2C Functions
// -------------------------------------------------------
static esp_err_t sht31_send_cmd(uint16_t cmd)
{
    uint8_t buf[2] = {(uint8_t)(cmd >> 8), (uint8_t)(cmd & 0xFF)};
    return i2c_master_write_to_device(
        SHT31_I2C_PORT, SHT31_I2C_ADDR,
        buf, 2, pdMS_TO_TICKS(100)
    );
}

static esp_err_t sht31_read_raw(float *temperature, float *humidity)
{
    uint8_t buf[2] = {
        (uint8_t)(SHT31_CMD_MEASURE_HIGH >> 8),
        (uint8_t)(SHT31_CMD_MEASURE_HIGH & 0xFF)
    };

    esp_err_t ret = i2c_master_write_to_device(
        SHT31_I2C_PORT, SHT31_I2C_ADDR,
        buf, 2, pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(20)); // Measurement time

    uint8_t data[6] = {0};
    ret = i2c_master_read_from_device(
        SHT31_I2C_PORT, SHT31_I2C_ADDR,
        data, 6, pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) return ret;

    // Parse temperature and humidity
    uint16_t raw_temp = (data[0] << 8) | data[1];
    uint16_t raw_hum  = (data[3] << 8) | data[4];

    *temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    *humidity    = 100.0f * ((float)raw_hum / 65535.0f);

    return ESP_OK;
}

// -------------------------------------------------------
// Public API
// -------------------------------------------------------
void sensor_init(void)
{
#ifdef CONFIG_SENSOR_DHT22
    ESP_LOGI(TAG, "Initializing DHT22 on GPIO %d", DHT22_GPIO_PIN);
    gpio_reset_pin(DHT22_GPIO_PIN);
    gpio_set_direction(DHT22_GPIO_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(DHT22_GPIO_PIN, GPIO_PULLUP_ONLY);

#elif CONFIG_SENSOR_SHT31
    ESP_LOGI(TAG, "Initializing SHT31 on I2C SDA=%d SCL=%d", SHT31_I2C_SDA, SHT31_I2C_SCL);

    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = SHT31_I2C_SDA,
        .scl_io_num       = SHT31_I2C_SCL,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SHT31_I2C_FREQ,
    };
    i2c_param_config(SHT31_I2C_PORT, &conf);
    i2c_driver_install(SHT31_I2C_PORT, conf.mode, 0, 0, 0);

    // Soft reset
    sht31_send_cmd(SHT31_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(10));
#endif

    ESP_LOGI(TAG, "✅ Sensor initialized");
}

esp_err_t sensor_read(float *temperature, float *humidity)
{
    if (!temperature || !humidity) return ESP_ERR_INVALID_ARG;

#ifdef CONFIG_SENSOR_DHT22
    return dht22_read_raw(temperature, humidity);

#elif CONFIG_SENSOR_SHT31
    return sht31_read_raw(temperature, humidity);

#else
    // Simulation mode for testing without hardware
    ESP_LOGW(TAG, "⚠️  No sensor configured — using simulated values");
    *temperature = -18.5f + ((float)(esp_random() % 100) / 100.0f);
    *humidity    = 60.0f  + ((float)(esp_random() % 200) / 100.0f);
    return ESP_OK;
#endif
}
