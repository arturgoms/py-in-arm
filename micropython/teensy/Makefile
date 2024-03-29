# Select the board to build for: if not given on the command line,
# then default to TEENSY_3.1.
BOARD ?= TEENSY_3.1
ifeq ($(wildcard boards/$(BOARD)/.),)
$(error Invalid BOARD specified)
endif

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

include ../py/mkenv.mk
-include mpconfigport.mk
include boards/$(BOARD)/mpconfigboard.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h $(BUILD)/pins_qstr.h

# include py core make definitions
include ../py/py.mk

# If you set USE_ARDUINO_TOOLCHAIN=1 then this makefile will attempt to use
# the toolchain that comes with Teensyduino
ifeq ($(USE_ARDUINO_TOOLCHAIN),)
USE_ARDUINO_TOOLCHAIN = 0
endif

ifeq ($(USE_ARDUINO_TOOLCHAIN),1)
ifeq ($(ARDUINO),)
$(error USE_ARDUINO_TOOLCHAIN requires that ARDUINO be set)
endif
endif

# If you use teensy_loader_cli
# from https://github.com/PaulStoffregen/teensy_loader_cli.git
# then you no longer need to install arduino and teensyduino.

ifeq ($(USE_ARDUINO_TOOLCHAIN),1)
$(info Using ARDUINO toolchain)
CROSS_COMPILE = $(ARDUINO)/hardware/tools/arm/bin/arm-none-eabi-
USE_TEENSY_LOADER_CLI = 0
else
$(info Using toolchain from PATH)
CROSS_COMPILE = arm-none-eabi-
USE_TEENSY_LOADER_CLI = 1
endif

CFLAGS_TEENSY = -DUSB_SERIAL -DARDUINO=10611
CFLAGS_CORTEX_M = -mthumb -mtune=cortex-m4 -mcpu=cortex-m4  -fsingle-precision-constant -Wdouble-promotion
CFLAGS_MCU_mk20 = $(CFLAGS_CORTEX_M) -D__MK20DX256__ -msoft-float -mfloat-abi=soft
CFLAGS_MCU_mk64 = $(CFLAGS_CORTEX_M) -D__MK64FX512__ -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS_MCU_mk66 = $(CFLAGS_CORTEX_M) -D__MK66FX1M0__ -mfloat-abi=hard -mfpu=fpv4-sp-d16
# CFLAGS_BOARD comes from mpconfigboard.mk

LDFLAGS_MCU_mk20 = -msoft-float -mfloat-abi=soft
LDFLAGS_MCU_mk66 = -mfloat-abi=hard -mfpu=fpv4-sp-d16

INC += -I.
INC += -I..
INC += -I../stmhal
INC += -I../lib/mp-readline
INC += -I$(BUILD)
INC += -Icore
INC += -Iboards/$(BOARD)
INC += -I../lib/timeutils

CFLAGS = $(INC) -Wall -Wpointer-arith -ansi -std=gnu99 -nostdlib $(CFLAGS_CORTEX_M4)
CFLAGS += $(CFLAGS_TEENSY) $(CFLAGS_MCU_$(MCU_SERIES)) $(CFLAGS_BOARD)
LDFLAGS = -nostdlib -T $(LD_FILE) $(LDFLAGS_MCU_$(MCU_SERIES)) -Wl,--defsym=__rtc_localtime=$(shell $(PYTHON) ./rtc_localtime.py)

CFLAGS += -DSTM32_HAL_H='"stm32hal.h"'
CFLAGS += -DMICROPY_HW_HAS_SDCARD=$(MICROPY_HW_HAS_SDCARD)

ifeq ($(USE_ARDUINO_TOOLCHAIN),1)

LIBGCC_FILE_NAME = $(ARDUINO)/hardware/tools/arm-none-eabi/lib/gcc/arm-none-eabi/4.7.2/thumb2/libgcc.a
LIBM_FILE_NAME   = $(ARDUINO)/hardware/tools/arm-none-eabi/arm-none-eabi/lib/thumb2/libm.a
LIBC_FILE_NAME   = $(ARDUINO)/hardware/tools/arm-none-eabi/arm-none-eabi/lib/thumb2/libc.a

else

LIBGCC_FILE_NAME = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LIBM_FILE_NAME   = $(shell $(CC) $(CFLAGS) -print-file-name=libm.a)
LIBC_FILE_NAME   = $(shell $(CC) $(CFLAGS) -print-file-name=libc.a)

endif

#$(info %%%%% LIBGCC_FILE_NAME = $(LIBGCC_FILE_NAME))
#$(info %%%%% LIBM_FILE_NAME = $(LIBM_FILE_NAME))
#$(info %%%%% LIBC_FILE_NAME = $(LIBC_FILE_NAME))

#$(info %%%%% dirname LIBGCC_FILE_NAME = $(dir $(LIBGCC_FILE_NAME)))
#$(info %%%%% dirname LIBM_FILE_NAME = $(dir $(LIBM_FILE_NAME)))
#$(info %%%%% dirname LIBC_FILE_NAME = $(dir $(LIBC_FILE_NAME)))

LIBS  = -L $(dir $(LIBM_FILE_NAME)) -lm
LIBS += -L $(dir $(LIBC_FILE_NAME)) -lc
LIBS += -L $(dir $(LIBGCC_FILE_NAME)) -lgcc

#Debugging/Optimization
ifdef DEBUG
CFLAGS += -Og -ggdb
else
CFLAGS += -Os #-DNDEBUG
endif
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections

USE_FROZEN = 1
USE_MEMZIP = 0

SRC_C = \
	hal_ftm.c \
	hal_gpio.c \
	help.c \
	main.c \
	adc.c \
	lcd.c \
	led.c \
	modmachine.c \
	modpyb.c \
	pin_defs_teensy.c \
	reg.c \
	rng.c \
	teensy_hal.c \
	timer.c \
	uart.c \
	usb.c \

STM_SRC_C = $(addprefix stmhal/,\
	bufhelper.c \
	gccollect.c \
	input.c \
	irq.c \
	pendsv.c \
	pin.c \
	pin_named_pins.c \
	pybstdio.c \
	)

STM_SRC_S = $(addprefix stmhal/,\
	gchelper.s \
	)

LIB_SRC_C = $(addprefix lib/,\
	libc/string0.c \
	mp-readline/readline.c \
	timeutils/timeutils.c \
	utils/pyexec.c \
	utils/pyhelp.c \
	)

SRC_TEENSY = $(addprefix core/,\
	mk20dx128.c \
	pins_teensy.c \
	analog.c \
	usb_desc.c \
	usb_dev.c \
	usb_mem.c \
	usb_serial.c \
	yield.c \
	)

ifeq ($(MICROPY_HW_HAS_SDCARD),1)
SRC_C += \
	sdcard.c \
	fatfs_port.c \
	sdhc.c \

STM_SRC_C += $(addprefix stmhal/,\
	builtin_open.c \
	moduos.c \
	)

LIB_SRC_C += $(addprefix lib/,\
	fatfs/ff.c \
	fatfs/option/ccsbcs.c \
	)
endif

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(STM_SRC_C:.c=.o) $(STM_SRC_S:.s=.o) $(SRC_TEENSY:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(BUILD)/pins_$(BOARD).o

all: hex
hex: $(BUILD)/micropython.hex

ifeq ($(USE_MEMZIP),1)
SRC_C += \
	lib/memzip/import.c \
	lib/memzip/lexermemzip.c \
	lib/memzip/memzip.c \

OBJ += $(BUILD)/memzip-files.o

MAKE_MEMZIP = ../lib/memzip/make-memzip.py
ifeq ($(MEMZIP_DIR),)
MEMZIP_DIR = scripts
endif

$(BUILD)/memzip-files.o: $(BUILD)/memzip-files.c
	$(call compile_c)

$(BUILD)/memzip-files.c: $(shell find ${MEMZIP_DIR} -type f)
	@$(ECHO) "Creating $@"
	$(Q)$(PYTHON) $(MAKE_MEMZIP) --zip-file $(BUILD)/memzip-files.zip --c-file $@ $(MEMZIP_DIR)

endif # USE_MEMZIP

ifeq ($(USE_FROZEN),1)

ifeq ($(FROZEN_DIR),)
FROZEN_DIR = scripts
endif

CFLAGS += -DMICROPY_MODULE_FROZEN_STR

ifeq ($(MICROPY_HW_HAS_SDCARD),1)
SRC_C += stmhal/import.c
else
SRC_C += lexerfrozen.c
endif
SRC_C += \
	$(BUILD)/frozen.c

endif # USE_FROZEN

ifeq ($(USE_TEENSY_LOADER_CLI),1)
post_compile: $(BUILD)/micropython.hex
	$(ECHO) "Preparing $@ for upload"
	$(Q)teensy_loader_cli -v -mmcu=$(MCU) -w -s $<

reboot:
else
ifeq ($(ARDUINO),)
post_compile: $(BUILD)/micropython.hex
	$(ECHO) "Please define ARDUINO (where TeensyDuino is installed)"
	exit 1

reboot:
	$(ECHO) "Please define ARDUINO (where TeensyDuino is installed)"
	exit 1

else
TOOLS_PATH = $(ARDUINO)/hardware/tools

post_compile: $(BUILD)/micropython.hex
	$(ECHO) "Preparing $@ for upload"
	$(Q)$(TOOLS_PATH)/teensy_post_compile -file="$(basename $(<F))" -path="$(abspath $(<D))" -tools="$(TOOLS_PATH)"

reboot:
	$(ECHO) "REBOOT"
	-$(Q)$(TOOLS_PATH)/teensy_reboot

endif	# ARDUINO
endif	# USE_TEENSY_LOADER_CLI

.PHONY: deploy
deploy: post_compile reboot

$(BUILD)/micropython.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -o "$@" -Wl,-Map,$(@:.elf=.map) $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/%.hex: $(BUILD)/%.elf
	$(ECHO) "HEX $<"
	$(Q)$(OBJCOPY) -O ihex -R .eeprom "$<" "$@"

MAKE_PINS = boards/make-pins.py
BOARD_PINS = boards/$(BOARD)/pins.csv
PREFIX_FILE = boards/mk20dx256_prefix.c
GEN_PINS_SRC = $(BUILD)/pins_$(BOARD).c
GEN_PINS_HDR = $(HEADER_BUILD)/pins.h
GEN_PINS_QSTR = $(BUILD)/pins_qstr.h
GEN_PINS_AF_CONST = $(HEADER_BUILD)/pins_af_const.h
GEN_PINS_AF_PY = $(BUILD)/pins_af.py

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(STM_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

# Making OBJ use an order-only depenedency on the generated pins.h file
# has the side effect of making the pins.h file before we actually compile
# any of the objects. The normal dependency generation will deal with the
# case when pins.h is modified. But when it doesn't exist, we don't know
# which source files might need it.
$(OBJ): | $(HEADER_BUILD)/pins.h

# Use a pattern rule here so that make will only call make-pins.py once to make
# both pins_$(BOARD).c and pins.h
$(BUILD)/%_$(BOARD).c $(HEADER_BUILD)/%.h $(HEADER_BUILD)/%_af_const.h $(BUILD)/%_qstr.h: boards/$(BOARD)/%.csv $(MAKE_PINS) $(AF_FILE) $(PREFIX_FILE) | $(HEADER_BUILD)
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(MAKE_PINS) --board $(BOARD_PINS) --af $(AF_FILE) --prefix $(PREFIX_FILE) --hdr $(GEN_PINS_HDR) --qstr $(GEN_PINS_QSTR) --af-const $(GEN_PINS_AF_CONST) --af-py $(GEN_PINS_AF_PY) > $(GEN_PINS_SRC)

$(BUILD)/pins_gen.o: $(BUILD)/pins_gen.c
	$(call compile_c)

$(BUILD)/%.pp: $(BUILD)/%.c
	$(ECHO) "PreProcess $<"
	$(Q)$(CC) $(CFLAGS) -E -Wp,-C,-dD,-dI -o $@ $<

include ../py/mkrules.mk
