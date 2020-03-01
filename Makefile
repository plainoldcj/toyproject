CFLAGS=\
-I./src\
-I./third_party/glew\
-I./third_party/stb\
-DGLEW_STATIC\
-D'__REFLECTED__='\
-D'__REFL_ATTRIB__(...)='

LIBS=-lSDL2main -lSDL2

include platform.mk

build_dir:
	mkdir -p build/game
	mkdir -p build/reflect
	mkdir -p build/third_party
.PHONY: build_dir

build/%.o: src/%.c
	@gcc -o $@ $(CFLAGS) -Wall -g -c $^ $(DEFINES)
	@echo Compiling $^

build/third_party/glew.o: third_party/glew/glew.c
	@gcc $(CFLAGS) -o $@ -c $^
	@echo Compiling third party $^

build/%.lex.c: src/%.flex
	mkdir -p $(dir $@)
	flex --outfile $@ $^

build/%.o: build/%.c
	@gcc $(CFLAGS) -c -o $@ $^
	@echo Compiling generated $^

build/reflected.c: build/reflect_app
	./reflect.sh

build/game_unit_tests.c:
	python3 ./src/build/test_parser.py $@ src/game
.PHONY: build/game_unit_tests.c

SRC:=$(shell find src -name '*.c')
FLEX:=$(shell find src -iname '*.flex')

OBJ:=$(patsubst src/%.c,build/%.o,$(SRC))
OBJ+=$(patsubst src/%.flex,build/%.lex.o,$(FLEX))

include src/reflect/reflect.mk
include src/game/game.mk

all: build_dir build/project
.PHONY: all

clean:
	rm -r build
.PHONY: clean
