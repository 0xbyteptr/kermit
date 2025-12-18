.PHONY all: build

clean:
	rm -rf build

build: clean
	mkdir -p build
	cd build && cmake .. && make