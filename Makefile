CFLAGS=\
-I./third_party/glew\
-DGLEW_STATIC

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
	-lGL

	DEFINES=-DPLATFORM_LINUX
endif

SRC=\
src/common.c\
src/main.c\
src/renderer.c

THIRD_PARTY_SRC=\
third_party/glew/glew.c

bin/project: $(SRC) $(THIRD_PARTY_SRC)
	gcc $(CFLAGS) -Wall -g $^ $(DEFINES) $(LIBS) -o $@
