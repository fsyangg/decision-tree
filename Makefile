.PHONY: build test benchmark clean

BUILD_TYPE ?= Release

build:
	conan install . --build=missing
	cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/$(BUILD_TYPE)/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build build -j

test: build
	ctest --test-dir build --output-on-failure

benchmark: build
	./build/tree_benchmark

clean:
	rm -rf build
