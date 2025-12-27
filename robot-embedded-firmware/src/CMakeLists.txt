
set(FOC_FILES
    foc/common/pid.cpp
    foc/common/time_utils.cpp
    foc/common/foc_utils.cpp
    foc/common/lowpass_filter.cpp
    foc/common/base_classes/CurrentSense.cpp
    foc/common/base_classes/Sensor.cpp
    foc/common/base_classes/FOCMotor.cpp
    foc/communication/Commander.cpp
    foc/communication/SimpleFOCDebug.cpp
#    foc/current_sense/GenericCurrentSense.cpp
#    foc/current_sense/LowsideCurrentSense.cpp
    foc/current_sense/hardware_specific/esp32/esp32_mcu.cpp
    foc/current_sense/hardware_specific/esp32/esp32_mcpwm_mcu.cpp
    foc/current_sense/hardware_specific/esp32/esp32_adc_driver.cpp
#    foc/sensors/GenericSensor.cpp
#    foc/sensors/Encoder.cpp
#    foc/sensors/HallSensor.cpp
    foc/sensors/MagneticSensorI2C.cpp
    foc/drivers/BLDCDriver3PWM.cpp
    foc/drivers/hardware_specific/esp32/esp32_mcpwm_mcu.cpp
    foc/drivers/hardware_specific/esp32/esp32_driver_mcpwm.cpp
    foc/angle_sensor/as5600.cpp

    foc/BLDCMotor.cpp
)


idf_component_register(SRCS
    main.cpp
    wifi.c
    servos.cpp
    battery.cpp
    AttitudeSensor.cpp
    STSServoDriver.cpp
    LQRController.cpp
    mpu6050.c
    robot.cpp

    ble.c
    ble/gap.c
    ble/gatt_svc.c
    ble/heart_rate_mock.c
    ble/led.c

    esp/io.cpp
    esp/adc.cpp
    esp/gpio.cpp
    esp/misc.cpp
    esp/serial.cpp

    ${FOC_FILES}

    PRIV_REQUIRES nvs_flash spi_flash esp_adc esp_driver_gpio esp_timer esp_wifi freertos bt
    INCLUDE_DIRS "." "../include"
)

# Tell the linker that we want to redefine the function named `esp_restart`.
# We must now define a function named __wrap_esp_restart, which has the same signature as the former implementation.
#target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=esp_restart")
