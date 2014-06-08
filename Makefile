# Copyright © 2013 Paulo Roberto Urio <paulourio@gmail.com> 
CMD=./build/simulate test.in

MAKEFLAGS += --no-print-directory

.PHONY: all clean release debug lint

all: debug

release: build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	cd build && make -j9

debug: build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	@#cd build && scan-build -v make -j9
	cd build && make -j9

build: 
	mkdir build

valgrind: debug
	valgrind --leak-check=full $(CMD)

run: debug
	$(CMD)

memcheck: debug
	valgrind --leak-check=full $(CMD)

lint:
	cpplint --filter=-whitespace/labels,-whitespace/braces,-build/include,-readability/streams *.cc *.h

clean:
	$(RM) -R build/

