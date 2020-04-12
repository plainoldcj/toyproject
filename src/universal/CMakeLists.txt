set(SOURCES
cmdline.c
cmdline.h
game_api.h
graphics.h
host.c
host.h
reflect.h
strsplit.c
strsplit.h
strsplit_tests.c
unit_tests.c
unit_tests.h
universal.h)

add_library(universal STATIC ${SOURCES})
