build/reflect_app: $(filter build/reflect/%,$(OBJ))
	gcc -o $@ $^ $(LIBS)
