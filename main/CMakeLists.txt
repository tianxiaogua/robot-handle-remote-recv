file(GLOB_RECURSE SOURCES 
	main.c
	../APP/app/*.c
	../APP/driver/*.c
	../APP/driver/tools/*.c
	../APP/component/wifi/wifi.c
	../APP/component/nvs/nvs.c 
	../APP/component/softAP/softAP.c 
)
 
set(inc
"."
../APP/app
../APP/driver
../APP/driver/tools
../APP/component/wifi
../APP/component/nvs 
../APP/component/softAP
)
 
idf_component_register(
	SRCS ${SOURCES} 
	INCLUDE_DIRS ${inc}
#	REQUIRES driver
#	REQUIRES esp_adc_cal
	)


#idf_component_register(
#    SRCS "tusb_hid_example_main.c"
#    INCLUDE_DIRS "."
#    REQUIRES driver
#    )
