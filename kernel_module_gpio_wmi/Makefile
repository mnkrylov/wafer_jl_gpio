obj-m += wafer_jl_gpio_wmi.o

# Используем временную папку для хранения временных файлов
TEMP_DIR := ./tmp
BIN_DIR := ./build

all:
	mkdir -p $(TEMP_DIR) $(BIN_DIR)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	mv *.o *.ko *.mod.c *.order *.symvers *.mod $(TEMP_DIR) 2>/dev/null || true
	mv $(TEMP_DIR)/*.ko $(BIN_DIR)/

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf $(TEMP_DIR) $(BIN_DIR)
