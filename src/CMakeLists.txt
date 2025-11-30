idf_component_register(SRCS
    "main.c"
    PRIV_REQUIRES esp_wifi nvs_flash
    INCLUDE_DIRS "." "../include")

# Tell the linker that we want to redefine the function named `esp_restart`.
# We must now define a function named __wrap_esp_restart, which has the same signature as the former implementation.
target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_restart")

