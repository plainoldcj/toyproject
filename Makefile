CFLAGS=\
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
src/alloc.c\
src/alloc_tests.c\
src/common.c\
src/draw_system.c\
src/editor.c\
src/entity.c\
src/grid.c\
src/main.c\
src/math.c\
src/math_tests.c\
src/reflect.c\
src/renderer.c\
src/shared_game.c

OBJ_GEN=\
build/reflected.o

THIRD_PARTY_SRC=\
third_party/glew/glew.c

build_dir:
	mkdir -p build
.PHONY: build_dir

build/%.lex.c: src/%.flex
	flex --outfile $@ $^

build/%.lex.o: build/%.lex.c
	gcc -c -Isrc -o $@ $^

build/%.o: build/%.c
	gcc $(CFLAGS) -c -o $@ $^

build/reflect: src/reflect_parser.c build/reflect_parser.lex.o
	gcc --std=c90 -g -o $@ $^

build/reflected.c: build/reflect
	./reflect.sh

build/project: $(SRC) $(THIRD_PARTY_SRC) $(OBJ_GEN)
	gcc $(CFLAGS) -Wall -g $^ $(DEFINES) $(LIBS) -o $@

all: build_dir build/reflect build/project
.PHONY: all
