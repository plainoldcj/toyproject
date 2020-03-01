build/project: $(filter build/game/%,$(OBJ)) build/third_party/glew.o build/reflected.o build/game_unit_tests.o
	@gcc -o $@ $^ $(LIBS)
	@echo Linking game
