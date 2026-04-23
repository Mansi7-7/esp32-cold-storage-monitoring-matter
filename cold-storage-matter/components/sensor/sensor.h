/*
 * sensor.h — Sensor abstraction for DHT22 / SHT31
 * Cold Storage Monitoring | DigitalMonk
 * https://digitalmonk.biz
 */

#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the temperature/humidity sensor
 *        Supports DHT22 or SHT31 — configure in menuconfig
 */
void sensor_init(void);

/**
 * @brief Read temperature and humidity from sensor
 *
 * @param temperature  Pointer to store temperature value in Celsius
 * @param humidity     Pointer to store relative humidity in percentage
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t sensor_read(float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif
