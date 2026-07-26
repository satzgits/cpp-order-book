CXX ?= g++
BUILD_DIR ?= build
CMAKE = cmake

.PHONY: all build test clean

all: build

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_CXX_COMPILER=$(CXX)
	cd $(BUILD_DIR) && $(CMAKE) --build .

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

run: build
	$(BUILD_DIR)/order_book

clean:
	rm -rf $(BUILD_DIR)
