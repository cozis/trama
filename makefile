ifeq ($(OS),Windows_NT)
	OSNAME = windows
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    	ARCH = AMD64
    else ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    	ARCH = AMD64
	else ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        ARCH = IA32
	endif
else
	UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OSNAME = linux
    else ifeq ($(UNAME_S),Darwin)
        OSNAME = osx
    endif

    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        ARCH = AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        ARCH = IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        ARCH = ARM
    endif
endif

ifeq ($(OSNAME),windows)
	PLATFORM_FLAGS = -lws2_32
	EXT = .exe
else ifeq ($(OSNAME),linux)
	PLATFORM_FLAGS = -pthread -ldl
	EXT =
else
endif

all: trama$(EXT)

sqlite3.o: src/core/sqlite3.c src/core/sqlite3.h
	gcc -c $< -o $@ -Wall -Wextra

trama$(EXT): sqlite3.o $(wildcard src/*/*.c src/*/*.h)
	gcc sqlite3.o src/main.c src/serve/template.c src/core/core.c src/serve/serve.c src/serve/router.c src/serve/tinytemplate.c -o trama$(EXT) -lchttp $(PLATFORM_FLAGS) -g -Wall -Wextra -I3p/chttp/include/ -L3p/chttp/

clean:
	rm trama trama.exe sqlite3.o