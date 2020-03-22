find_package(FLEX REQUIRED)

set(SOURCES
reflect_local.h
reflect_parser.c
term.h
)

flex_target(REFLECT_LEXER reflect_parser.flex ${CMAKE_CURRENT_BINARY_DIR}/reflect_parser.lex.c)

add_executable(reflect_app ${FLEX_REFLECT_LEXER_OUTPUTS} ${SOURCES})
