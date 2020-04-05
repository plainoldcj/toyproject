set(SOURCES
reflect.h
strsplit.c
strsplit.h
strsplit_tests.c
unit_tests.c
unit_tests.h
universal.h)

add_library(universal STATIC ${SOURCES})
