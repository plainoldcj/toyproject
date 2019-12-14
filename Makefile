CFLAGS=\
-I./src\
-I./third_party/glew\
-DGLEW_STATIC\
-D__REFLECTED__=

LIBS=-lSDL2main -lSDL2

include platform.mk

SRC:=$(shell find src/game -name '*.c')
OBJ:=$(patsubst src/%.c,build/%.o,$(SRC))

build_dir:
	mkdir -p build/game
	mkdir -p build/reflect
	mkdir -p build/third_party
.PHONY: build_dir

build/%.o: src/%.c
	gcc -o $@ $(CFLAGS) -Wall -g -c $^ $(DEFINES)

build/third_party/glew.o: third_party/glew/glew.c
	gcc $(CFLAGS) -o $@ -c $^

build/%.lex.c: src/%.flex
	mkdir -p $(dir $@)
	flex --outfile $@ $^

build/%.o: build/%.c
	gcc $(CFLAGS) -c -o $@ $^

build/reflect_app: build/reflect/reflect_parser.o build/reflect/reflect_parser.lex.o
	gcc -o $@ $^

build/reflected.c: build/reflect_app
	./reflect.sh

build/project: $(OBJ) build/third_party/glew.o build/reflected.o
	gcc -o $@ $^ $(LIBS)

all: build_dir build/project
.PHONY: all

clean:
	rm -r build
.PHONY: clean
