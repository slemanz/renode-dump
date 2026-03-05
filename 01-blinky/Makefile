CC = arm-none-eabi-gcc
TARGET = flash

MACH = -mcpu=cortex-m4
FLOAT = -mfloat-abi=hard -mfpu=fpv4-sp-d16

BUILD_DIR = Build
ROOT_DIR  = ..

LINKER   = -T $(ROOT_DIR)/general/drivers/Device/linkers/STM32F411.ld
MAP_FILE = -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

CFLAGS = -c $(MACH) -mthumb $(FLOAT) -std=gnu99 -Wall -O0 -g3 \
         --specs=nano.specs -ffunction-sections -fdata-sections

LDFLAGS = $(MACH) -mthumb $(FLOAT) --specs=nosys.specs $(LINKER) $(MAP_FILE) -g3 \
          -Wl,--gc-sections -static --specs=nano.specs \
          -Wl,--start-group -lc -lm -Wl,--end-group \
          -Wl,--print-memory-usage

OBJCOPY = arm-none-eabi-objcopy

###########################################
#                 INCLUDES
###########################################

INCLUDES += -I $(ROOT_DIR)/general/drivers/Inc/

############################################
#               SOURCE FILES
############################################

OBJS     += $(BUILD_DIR)/main.o
OBJS     += $(BUILD_DIR)/syscalls.o
OBJS     += $(BUILD_DIR)/startup.o

DRIVERS  += $(BUILD_DIR)/driver_clock.o
DRIVERS  += $(BUILD_DIR)/driver_gpio.o
DRIVERS  += $(BUILD_DIR)/driver_systick.o
DRIVERS  += $(BUILD_DIR)/driver_interrupt.o

PREREQ = $(BUILD_DIR)

all: $(PREREQ) $(BUILD_DIR)/$(TARGET).elf

$(PREREQ):
	mkdir -p $(PREREQ)

$(BUILD_DIR)/%.o: Src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/%.o: $(ROOT_DIR)/general/drivers/Src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/%.o: $(ROOT_DIR)/general/drivers/Device/system/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/%.o: $(ROOT_DIR)/general/drivers/Device/startup/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(DRIVERS)
	$(CC) $(LDFLAGS) -o $@ $^
	$(OBJCOPY) -O binary $@ $(BUILD_DIR)/$(TARGET).bin
	arm-none-eabi-size $@

clean:
	rm -rf $(BUILD_DIR)/*.map $(BUILD_DIR)/*.o

.PHONY: all clean