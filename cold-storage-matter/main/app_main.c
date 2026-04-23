/*
 * Cold Storage Monitoring Using Matter Protocol
 * Developed by DigitalMonk — https://digitalmonk.biz
 * Case Study: https://digitalmonk.biz/cold-storage-monitoring-using-matter-protocol/
 *
 * Hardware: ESP32 + DHT22/SHT31 Sensor
 * Protocol: Matter over Wi-Fi / Thread
 * Platforms: Apple Home, Google Home, Samsung SmartThings
 */

#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include <app_priv.h>
#include <app_reset.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sensor.h"

static const char *TAG = "cold_storage_matter";

// Matter Endpoint IDs
static uint16_t temperature_endpoint_id   = 0;
static uint16_t humidity_endpoint_id      = 0;

// Alert Thresholds (configurable)
#define TEMP_ALERT_HIGH_C       (-5)    // Alert if above -5°C
#define TEMP_ALERT_LOW_C        (-25)   // Alert if below -25°C
#define HUMIDITY_ALERT_HIGH_PCT (85)    // Alert if above 85% RH

// Matter attribute update callback
static void app_attribute_update_cb(
    esp_matter::callback_type_t type,
    uint16_t endpoint_id,
    uint32_t cluster_id,
    uint32_t attribute_id,
    esp_matter_attr_val_t *val,
    void *priv_data)
{
    if (type == esp_matter::PRE_UPDATE) {
        ESP_LOGI(TAG, "Attribute update: endpoint=%d cluster=0x%04" PRIx32 " attr=0x%04" PRIx32,
                 endpoint_id, cluster_id, attribute_id);
    }
}

// Matter event handler
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "✅ Matter Commissioning complete!");
            break;
        case chip::DeviceLayer::DeviceEventType::kInternetConnectivityChange:
            if (event->InternetConnectivityChange.IPv4 == chip::DeviceLayer::kConnectivity_Established) {
                ESP_LOGI(TAG, "🌐 IPv4 connectivity established");
            }
            break;
        default:
            break;
    }
}

// Task: Read sensor and update Matter attributes
static void sensor_update_task(void *pvParameters)
{
    float temperature = 0.0f;
    float humidity    = 0.0f;

    while (1) {
        // Read from DHT22 / SHT31
        esp_err_t ret = sensor_read(&temperature, &humidity);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "🌡 Temp: %.2f°C  💧 Humidity: %.2f%%", temperature, humidity);

            // Matter uses int16 with factor 100 for temperature (millidegrees)
            int16_t matter_temp     = (int16_t)(temperature * 100);
            uint16_t matter_humidity = (uint16_t)(humidity * 100);

            // Update Temperature attribute in Matter
            esp_matter::attribute::report(
                temperature_endpoint_id,
                chip::app::Clusters::TemperatureMeasurement::Id,
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id,
                &matter_temp,
                sizeof(matter_temp)
            );

            // Update Humidity attribute in Matter
            esp_matter::attribute::report(
                humidity_endpoint_id,
                chip::app::Clusters::RelativeHumidityMeasurement::Id,
                chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Id,
                &matter_humidity,
                sizeof(matter_humidity)
            );

            // Check thresholds and log alerts
            if (temperature > TEMP_ALERT_HIGH_C) {
                ESP_LOGW(TAG, "🔴 ALERT: Temperature too HIGH! %.2f°C > %d°C", temperature, TEMP_ALERT_HIGH_C);
            }
            if (temperature < TEMP_ALERT_LOW_C) {
                ESP_LOGW(TAG, "🔴 ALERT: Temperature too LOW! %.2f°C < %d°C", temperature, TEMP_ALERT_LOW_C);
            }
            if (humidity > HUMIDITY_ALERT_HIGH_PCT) {
                ESP_LOGW(TAG, "💧 ALERT: Humidity too HIGH! %.2f%% > %d%%", humidity, HUMIDITY_ALERT_HIGH_PCT);
            }

        } else {
            ESP_LOGE(TAG, "❌ Sensor read failed: %s", esp_err_to_name(ret));
        }

        // Read every 30 seconds
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

void app_main()
{
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " Cold Storage Monitor — Matter Protocol");
    ESP_LOGI(TAG, " By DigitalMonk | https://digitalmonk.biz");
    ESP_LOGI(TAG, "==============================================");

    // Initialize sensor (DHT22 or SHT31)
    sensor_init();

    // Initialize Matter node
    esp_matter::node::config_t node_config;
    esp_matter::node_t *node = esp_matter::node::create(
        &node_config,
        app_attribute_update_cb,
        app_event_cb
    );

    // ---- Temperature Sensor Endpoint ----
    esp_matter::endpoint::temperature_sensor::config_t temp_config;
    esp_matter::endpoint_t *temp_ep = esp_matter::endpoint::temperature_sensor::create(
        node, &temp_config, ENDPOINT_FLAG_NONE, NULL
    );
    temperature_endpoint_id = esp_matter::endpoint::get_id(temp_ep);
    ESP_LOGI(TAG, "Temperature endpoint ID: %d", temperature_endpoint_id);

    // ---- Humidity Sensor Endpoint ----
    esp_matter::endpoint::humidity_sensor::config_t hum_config;
    esp_matter::endpoint_t *hum_ep = esp_matter::endpoint::humidity_sensor::create(
        node, &hum_config, ENDPOINT_FLAG_NONE, NULL
    );
    humidity_endpoint_id = esp_matter::endpoint::get_id(hum_ep);
    ESP_LOGI(TAG, "Humidity endpoint ID: %d", humidity_endpoint_id);

    // Start Matter stack
    esp_matter::start(app_event_cb);

    // Enable Matter console (for debugging)
#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::init();
#endif

    // Start sensor reading task
    xTaskCreate(
        sensor_update_task,
        "sensor_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "✅ System initialized. Monitoring started.");
}
