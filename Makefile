BUILD_DIR = build

.PHONY: all configure clean \
	run-primitive run-struct run-nested-struct run-nested-pointer-struct \
	run-client run-server

all: configure
	cmake --build $(BUILD_DIR)

configure:
	cmake -S . -B $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

# --- serdes ---
run-primitive: all
	./$(BUILD_DIR)/serdes/primitive_ds_ser_deser

run-struct: all
	./$(BUILD_DIR)/serdes/struct_ds_ser_deser

run-nested-struct: all
	./$(BUILD_DIR)/serdes/nested_struct_ds_ser_deser

run-nested-pointer-struct: all
	./$(BUILD_DIR)/serdes/nested_pointer_struct_ds_ser_deser

# --- networking ---
run-client: all
	./$(BUILD_DIR)/networking/client

run-server: all
	./$(BUILD_DIR)/networking/server
