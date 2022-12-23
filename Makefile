BUILD_DIR = build

.PHONY: all configure clean run-primitive run-struct

all: configure
	cmake --build $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

run-primitive: all
	./$(BUILD_DIR)/primitive_ds_ser_deser

run-struct: all
	./$(BUILD_DIR)/struct_ds_ser_deser

run-nested-struct: all
	./$(BUILD_DIR)/nested_struct_ds_ser_deser
