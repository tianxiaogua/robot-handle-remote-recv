file(GLOB_RECURSE SOURCES 
	main.c
	../APP/app/*.c
	../APP/driver/*.c
	../APP/driver/tools/*.c
#	../APP/LCD/*.c
#	../APP/PCA9557/*.c
#	../APP/MPU6050/*.c
)

set(inc
"."
../APP/app
../APP/driver
../APP/driver/tools
#../APP/LCD
#../APP/PCA9557
#../APP/MPU6050
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
