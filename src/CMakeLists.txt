idf_component_register(SRCS
    main.cpp
    wifi.c

    battery.cpp

    STSServoDriver.cpp

    esp/io.cpp
    esp/gpio.cpp
    esp/misc.cpp
    esp/serial.cpp

    PRIV_REQUIRES nvs_flash spi_flash esp_adc esp_driver_gpio esp_timer esp_wifi freertos
    INCLUDE_DIRS "." "../include"
)

# Tell the linker that we want to redefine the function named `esp_restart`.
# We must now define a function named __wrap_esp_restart, which has the same signature as the former implementation.
#target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_restart")
