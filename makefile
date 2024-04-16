ASM=nasm
SRC_DIR=src
BUILD_DIR=build
C_COMP=gcc



.PHONY: floppy_image kernel bootloader tool_fat_read always






tool_fat_read: $(BUILD_DIR)/tools/FAT_reader

$(BUILD_DIR)/tools/FAT_reader: always
	$(C_COMP) ./$(SRC_DIR)/tools/fat_reader.c -o ./$(BUILD_DIR)/tools/FAT_reader 

floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main_floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "Label" $(BUILD_DIR)/main_floppy.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main_floppy.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main_floppy.img $(BUILD_DIR)/kernel.bin "::kernel.bin"


bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: always
	$(ASM) ./$(SRC_DIR)/bootloader/boot.asm -f bin -o ./$(BUILD_DIR)/bootloader.bin


kernel: $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.bin: always
	$(ASM) ./$(SRC_DIR)/kernel/main.asm -f bin -o ./$(BUILD_DIR)/kernel.bin



always:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/tools

clean:
	rm -rf $(BUILD_DIR)/*
