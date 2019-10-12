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
src/main.c

bin/project: $(SRC)
	gcc -g $^ $(LIBS) $(WINLIBS) -o $@
