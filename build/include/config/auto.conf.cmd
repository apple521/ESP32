deps_config := \
	/home/cjy/esp/esp-idf/components/app_trace/Kconfig \
	/home/cjy/esp/esp-idf/components/aws_iot/Kconfig \
	/home/cjy/esp/esp-idf/components/bt/Kconfig \
	/home/cjy/esp/esp-idf/components/driver/Kconfig \
	/home/cjy/esp/esp-idf/components/esp32/Kconfig \
	/home/cjy/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/cjy/esp/esp-idf/components/esp_event/Kconfig \
	/home/cjy/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/cjy/esp/esp-idf/components/esp_http_server/Kconfig \
	/home/cjy/esp/esp-idf/components/ethernet/Kconfig \
	/home/cjy/esp/esp-idf/components/fatfs/Kconfig \
	/home/cjy/esp/esp-idf/components/freemodbus/Kconfig \
	/home/cjy/esp/esp-idf/components/freertos/Kconfig \
	/home/cjy/esp/esp-idf/components/heap/Kconfig \
	/home/cjy/esp/esp-idf/components/libsodium/Kconfig \
	/home/cjy/esp/esp-idf/components/log/Kconfig \
	/home/cjy/esp/esp-idf/components/lwip/Kconfig \
	/home/cjy/esp/esp-idf/components/mbedtls/Kconfig \
	/home/cjy/esp/esp-idf/components/mdns/Kconfig \
	/home/cjy/esp/esp-idf/components/mqtt/Kconfig \
	/home/cjy/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/cjy/esp/esp-idf/components/openssl/Kconfig \
	/home/cjy/esp/esp-idf/components/pthread/Kconfig \
	/home/cjy/esp/esp-idf/components/spi_flash/Kconfig \
	/home/cjy/esp/esp-idf/components/spiffs/Kconfig \
	/home/cjy/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/cjy/esp/esp-idf/components/vfs/Kconfig \
	/home/cjy/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/cjy/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/cjy/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/c/msys32/home/cjy/esp/smart_config/main/Kconfig.projbuild \
	/home/cjy/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/cjy/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
