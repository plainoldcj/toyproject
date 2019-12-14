CFLAGS=\
-I./src\
-I./third_party/glew\
-DGLEW_STATIC\
-D__REFLECTED__=

LIBS=\
-lSDL2main\
-lSDL2

# See https://vim.fandom.com/wiki/Accessing_the_system_clipboard.
# This assumes that OS environment variable is only set on Windows.
ifdef OS
	LIBS+=\
	-lgdi32\
	-lwinmm\
	-limm32\
	-lole32\
	-loleaut32\
	-lversion\
	-lsetupapi\
	-lopengl32

	DEFINES=-DPLATFORM_WINDOWS
else
	LIBS+=\
	-lGL\
	-lGLU\
	-lm

	DEFINES=-DPLATFORM_LINUX
endif

SRC=\
src/game/alloc.c\
src/game/alloc_tests.c\
src/game/common.c\
src/game/draw_system.c\
src/game/editor.c\
src/game/entity.c\
src/game/grid.c\
src/game/main.c\
src/game/math.c\
src/game/math_tests.c\
src/game/reflect.c\
src/game/renderer.c\
src/game/shared_game.c

OBJ_GEN=\
build/reflected.o

THIRD_PARTY_SRC=\
third_party/glew/glew.c

build_dir:
	mkdir -p build
.PHONY: build_dir

build/%.lex.c: src/%.flex
	mkdir -p $(dir $@)
	flex --outfile $@ $^

build/%.lex.o: build/%.lex.c
	gcc -c -Isrc -o $@ $^

build/%.o: build/%.c
	gcc $(CFLAGS) -c -o $@ $^

FLEX=$(wildcard src/reflect/*.flex)
FLEX_OBJ=$(patsubst src/reflect/%.flex,build/reflect/%.lex.o,$(FLEX))

build/reflect_app: src/reflect/reflect_parser.c $(FLEX_OBJ)
	echo $(FLEX_OBJ)
	gcc -Isrc --std=c90 -g -o $@ $^

build/reflected.c: build/reflect_app
	./reflect.sh

build/project: $(SRC) $(THIRD_PARTY_SRC) $(OBJ_GEN)
	gcc $(CFLAGS) -Wall -g $^ $(DEFINES) $(LIBS) -o $@

all: build_dir build/project
.PHONY: all

clean:
	rm -r build
.PHONY: clean
