.PHONY: build test benchmark bench-py clean

BUILD_TYPE ?= Release
VENV ?= .venv
PY := $(VENV)/bin/python

build:
	conan install . --build=missing
	cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/$(BUILD_TYPE)/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build build -j

test: build
	ctest --test-dir build --output-on-failure

benchmark: build
	./build/tree_benchmark

$(VENV):
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install -q -r bench/pybench/requirements.txt

bench-py: $(VENV)
	$(PY) bench/pybench/bench.py --json bench/results/sklearn.json

clean:
	rm -rf build $(VENV)
