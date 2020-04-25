set(SOURCES
app_event.h
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
universal.h
vertex.h)

add_library(universal STATIC ${SOURCES})
