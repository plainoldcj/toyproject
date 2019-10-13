CFLAGS=\
-I./third_party/glew\
-DGLEW_STATIC

WINLIBS=\
-lgdi32\
-lwinmm\
-limm32\
-lole32\
-loleaut32\
-lversion\
-lsetupapi\
-lopengl32

LIBS=\
-lSDL2main\
-lSDL2

SRC=\
src/common.c\
src/main.c\
src/renderer.c

THIRD_PARTY_SRC=\
third_party/glew/glew.c

bin/project: $(SRC) $(THIRD_PARTY_SRC)
	gcc $(CFLAGS) -g $^ $(LIBS) $(WINLIBS) -o $@
