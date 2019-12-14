build/project: $(filter build/game/%,$(OBJ)) build/third_party/glew.o build/reflected.o
	gcc -o $@ $^ $(LIBS)
