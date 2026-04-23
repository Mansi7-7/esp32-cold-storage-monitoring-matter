cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    components
    $ENV{ESP_MATTER_PATH}/components
    $ENV{ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/config/esp32/components
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cold_storage_matter)
