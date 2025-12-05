
set(FOC_FILES
    "foc/common/pid.cpp"
    "foc/common/time_utils.cpp"
    "foc/common/lowpass_filter.cpp"
    "foc/common/base_classes/CurrentSense.cpp"
    "foc/common/base_classes/Sensor.cpp"
    "foc/common/base_classes/FOCMotor.cpp"
    "foc/communication/Commander.cpp"
    "foc/communication/SimpleFOCDebug.cpp"
    "foc/current_sense/GenericCurrentSense.cpp"
    "foc/current_sense/LowsideCurrentSense.cpp"
    "foc/sensors/GenericSensor.cpp"
    "foc/sensors/Encoder.cpp"
    "foc/sensors/HallSensor.cpp"
    "foc/BLDCMotor.cpp")


idf_component_register(SRCS
    main.cpp
    wifi.c
    servos.cpp
    battery.cpp

    STSServoDriver.cpp

    esp/io.cpp
    esp/gpio.cpp
    esp/misc.cpp
    esp/serial.cpp

    #    ${FOC_FILES}

    PRIV_REQUIRES nvs_flash spi_flash esp_adc esp_driver_gpio esp_timer esp_wifi freertos
    INCLUDE_DIRS "." "../include"
)

# Tell the linker that we want to redefine the function named `esp_restart`.
# We must now define a function named __wrap_esp_restart, which has the same signature as the former implementation.
#target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_restart")
