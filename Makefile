TARGET := stm32f103rct6_app
BUILD_DIR := build
OUTPUT_DIR := output

CC := arm-none-eabi-gcc
AS := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size
OPENOCD ?= openocd
ST_FLASH ?= st-flash

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
COMMON_FLAGS := $(CPU_FLAGS) -std=c11 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wno-unused-parameter -g3 -O0
DEFINES := -DSTM32F103xE -DUSE_HAL_DRIVER

INCLUDES := \
	-Iapp/include \
	-Idrivers/interface/include \
	-Idrivers/bsp/gpio/include \
	-Idrivers/bsp/led/include \
	-Idrivers/bsp/uart/include \
	-Iplatform/chips/stm32f1/include \
	-Iplatform/cmsis/core/include \
	-Iplatform/cmsis/device/st/stm32f1xx/include \
	-Iplatform/hal/stm32f1/include \
	-Iplatform/boards/stm32f103rct6_generic/include \
	-Irtos/freertos/config \
	-Irtos/wrappers/include \
	-Ithird_party/freertos/FreeRTOS-Kernel-main/include \
	-Ithird_party/freertos/FreeRTOS-Kernel-main/portable/GCC/ARM_CM3

C_SOURCES := \
	app/src/app.c \
	app/src/app_event.c \
	app/src/app_key.c \
	app/src/app_key_demo.c \
	app/src/app_key_input.c \
	app/src/app_led_demo.c \
	app/src/app_service.c \
	app/src/app_timer.c \
	app/src/app_task.c \
	app/src/log.c \
	app/src/app_system.c \
	app/src/main.c \
	drivers/bsp/gpio/src/bsp_key.c \
	drivers/bsp/led/src/bsp_led.c \
	drivers/bsp/uart/src/bsp_uart.c \
	platform/boards/stm32f103rct6_generic/src/board.c \
	platform/boards/stm32f103rct6_generic/src/board_irq.c \
	platform/boards/stm32f103rct6_generic/src/stm32f1xx_it.c \
	platform/cmsis/device/st/stm32f1xx/src/system_stm32f1xx.c \
	platform/hal/stm32f1/src/stm32f1xx_hal.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_cortex.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_dma.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_exti.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_flash.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_flash_ex.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_gpio.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_pwr.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_rcc.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_rcc_ex.c \
	platform/hal/stm32f1/src/stm32f1xx_hal_uart.c \
	rtos/wrappers/src/rtos_hooks.c \
	third_party/freertos/FreeRTOS-Kernel-main/list.c \
	third_party/freertos/FreeRTOS-Kernel-main/queue.c \
	third_party/freertos/FreeRTOS-Kernel-main/tasks.c \
	third_party/freertos/FreeRTOS-Kernel-main/timers.c \
	third_party/freertos/FreeRTOS-Kernel-main/portable/GCC/ARM_CM3/port.c \
	third_party/freertos/FreeRTOS-Kernel-main/portable/MemMang/heap_4.c

ASM_SOURCES := \
	platform/startup/stm32f103rct6/startup_stm32f103xe.s

OBJECTS := $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o)) $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))
DEPFILES := $(OBJECTS:.o=.d)

LDSCRIPT := toolchain/linker/stm32f103rct6_flash.ld
LDFLAGS := $(CPU_FLAGS) -T$(LDSCRIPT) -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections -Wl,-Map=$(OUTPUT_DIR)/$(TARGET).map
OPENOCD_INTERFACE_CFG := toolchain/openocd/stlink.cfg
OPENOCD_TARGET_CFG := toolchain/openocd/stm32f103rct6.cfg
OPENOCD_ARGS := -f $(OPENOCD_INTERFACE_CFG) -f $(OPENOCD_TARGET_CFG)

.PHONY: all clean size flash flash-openocd erase erase-openocd debug-server reset

all: $(OUTPUT_DIR)/$(TARGET).elf $(OUTPUT_DIR)/$(TARGET).bin $(OUTPUT_DIR)/$(TARGET).hex size

$(OUTPUT_DIR)/$(TARGET).elf: $(OBJECTS) | $(OUTPUT_DIR)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(OUTPUT_DIR)/$(TARGET).bin: $(OUTPUT_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(OUTPUT_DIR)/$(TARGET).hex: $(OUTPUT_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(DEFINES) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(CPU_FLAGS) -x assembler-with-cpp $(DEFINES) $(INCLUDES) -c $< -o $@

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

size: $(OUTPUT_DIR)/$(TARGET).elf
	$(SIZE) $<

flash: $(OUTPUT_DIR)/$(TARGET).bin
	$(ST_FLASH) write $(OUTPUT_DIR)/$(TARGET).bin 0x08000000
	$(ST_FLASH) reset

flash-openocd: $(OUTPUT_DIR)/$(TARGET).elf
	$(OPENOCD) $(OPENOCD_ARGS) -c "program $(OUTPUT_DIR)/$(TARGET).elf verify reset exit"

erase:
	$(ST_FLASH) erase

erase-openocd:
	$(OPENOCD) $(OPENOCD_ARGS) -c "init" -c "reset init" -c "stm32f1x mass_erase 0" -c "reset run" -c "shutdown"

debug-server:
	$(OPENOCD) $(OPENOCD_ARGS)

reset:
	$(ST_FLASH) reset

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

-include $(DEPFILES)
