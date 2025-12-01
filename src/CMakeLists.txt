idf_component_register(SRCS
    main.c
    wifi.c

    battery.c

    PRIV_REQUIRES esp_wifi nvs_flash spi_flash cxx esp_adc esp_driver_gpio freertos
    INCLUDE_DIRS "." "../include" # "../include/cores"
)

# Tell the linker that we want to redefine the function named `esp_restart`.
# We must now define a function named __wrap_esp_restart, which has the same signature as the former implementation.
#target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_restart")
