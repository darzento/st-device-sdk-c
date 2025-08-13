target_sources(iotcore
        PRIVATE
        iot_bsp_debug_esp32c3.c
        iot_bsp_fs_esp32c3.c
        iot_bsp_nv_data_esp32c3.c
        iot_bsp_random_esp32c3.c
        iot_bsp_system_esp32c3.c
        iot_bsp_wifi_esp32c3.c
        )

if(CONFIG_STDK_IOT_CORE_EASYSETUP_BLE)
target_sources(iotcore
        PRIVATE
        iot_bsp_ble_esp32c3.c
        )
endif()
