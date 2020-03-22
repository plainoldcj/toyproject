set(SOURCES
alloc.c
alloc.h
alloc_tests.c
alloc_tests.h
app_states.c
app_states.h
as_editor.c
as_game.c
as_menu.c
assets.c
assets.h
bomb_system.c
camera_system.c
chest_system.c
common.c
common.h
draw_system.c
editor.c
editor.h
entity.c
entity.h
explosion_system.c
font_baker.c
font_baker.h
font.h
font_renderer.c
font_renderer.h
game.c
game.h
grid.c
grid.h
json_common.c
json_common.h
json_reader.c
json_reader.h
json_tests.c
json_tests.h
json_writer.c
json_writer.h
main.c
material_manager.c
material_manager.h
math.c
math.h
math_tests.c
math_tests.h
physics_system.c
player_system.c
reflect.c
renderer.c
renderer.h
shared_game.c
shared_game.h
strsplit.c
strsplit.h
strsplit_tests.c
tga_image.c
tga_image.h
trigger_system.c
ui.c
ui.h
unit_tests.c)

set(GENERATED_SOURCES
${CMAKE_BINARY_DIR}/reflected.c
${CMAKE_CURRENT_BINARY_DIR}/game_unit_tests.c
)

find_package(SDL2 REQUIRED)

add_definitions(-D'__REFLECTED__=')
add_definitions(-D'__REFL_ATTRIB__\(...\)=')

add_executable(project ${SOURCES} ${GENERATED_SOURCES})

target_link_libraries(project glew)
target_link_libraries(project ${SDL2_LIBRARIES})
target_link_libraries(project m GL GLU)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/reflected.c
	COMMAND ./reflect.sh
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS reflect_app)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/game_unit_tests.c
	COMMAND python3 ./src/build/test_parser.py ${CMAKE_CURRENT_BINARY_DIR}/game_unit_tests.c src/game
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
