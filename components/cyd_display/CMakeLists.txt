if(IDF_TARGET STREQUAL "esp32")
    set(CYD_REQUIRES esp_lcd esp_driver_spi esp_driver_gpio esp_wifi freertos
        nvs_flash lvgl__lvgl atanisoft__esp_lcd_touch_xpt2046 cmd_router)
else()
    set(CYD_REQUIRES "")
endif()

idf_component_register(SRCS "cyd_display.c"
    INCLUDE_DIRS "include" "../../include"
    REQUIRES ${CYD_REQUIRES})

target_compile_options(${COMPONENT_LIB} PRIVATE -Os)
