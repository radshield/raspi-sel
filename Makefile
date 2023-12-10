all: logger
.PHONY: clean logger

logger: logger/record.c
	cmake -S logger -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cd build && ninja

clean:
	rm -rf build
