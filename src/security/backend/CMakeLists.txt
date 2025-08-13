if(${CONFIG_STDK_IOT_CORE_SECURITY_BACKEND_SOFTWARE})
	add_subdirectory(software)
elseif(${CONFIG_STDK_IOT_CORE_SECURITY_BACKEND_HARDWARE})
	add_subdirectory(hardware)
endif()

target_sources(iotcore
	PRIVATE
	iot_security_be_bsp.c
)

