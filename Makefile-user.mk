## Local build configuration
## Parameters configured here will override default and ENV values.
## Uncomment and change examples:

## Add your source directories here separated by space
# MODULES = app
# EXTRA_INCDIR = include

## ESP_HOME sets the path where ESP tools and SDK are located.
## Windows:
# ESP_HOME = c:/Espressif

## MacOS / Linux:
# ESP_HOME = /opt/esp-open-sdk
ESP_HOME = $(HOME)/git/esp-open-sdk

## SMING_HOME sets the path where Sming framework is located.
## Windows:
# SMING_HOME = c:/tools/sming/Sming 

## MacOS / Linux
# SMING_HOME = /opt/sming/Sming
SMING_HOME = $(HOME)/git/Sming/Sming

## COM port parameter is reqruied to flash firmware correctly.
## Windows: 
# COM_PORT = COM3

## MacOS / Linux:
# COM_PORT = /dev/tty.usbserial
COM_PORT = /dev/ttyS0

## Com port speed
# COM_SPEED	= 115200
COM_SPEED ?= 115200

# SPI_MODE: qio, qout, dio, dout
## Configure flash parameters (for ESP12-E and other new boards):
# SPI_MODE = dio
#
## *** Questa impostazione riguarda l'ultima versione di ESP8266 ESP-01, V3.0 ? ***
SPI_MODE ?= dout

## SPIFFS options
DISABLE_SPIFFS = 1
# SPIFF_FILES = files

# esptool
ESPTOOL ?= /home/pi/git/esptool/esptool.py
# esptool2 path
ESPTOOL2 ?= /home/pi/git/esptool2/esptool2

# Default COM port speed (used for flashing)
COM_SPEED_ESPTOOL ?= $(COM_SPEED)

## MQTT Parameters
ifdef MQTT_URL
	USER_CFLAGS += -DMQTT_URL=\"$(MQTT_URL)\" 
endif

# We need rBoot in order to be able to run bigger Flash roms.

#### overridable rBoot options ####
## use rboot build mode
RBOOT_ENABLED ?= 1
## enable big flash support (for multiple roms, each in separate 1mb block of flash)
RBOOT_BIG_FLASH ?= 1
## two rom mode (where two roms sit in the same 1mb block of flash)
#RBOOT_TWO_ROMS  ?= 1
## size of the flash chip
SPI_SIZE  ?= 4M

## WIFI
ifdef WIFI_SSID
	USER_CFLAGS += -DWIFI_SSID=\"$(WIFI_SSID)\" 
endif
ifdef WIFI_PWD
	USER_CFLAGS += -DWIFI_PWD=\"$(WIFI_PWD)\" 
endif
