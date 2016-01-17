APP=shapes
APP2=shapes_part2
GCC_OPTIONS=-Wall -pedantic -I include
GL_OPTIONS=-lGLEW -lGL -lglut
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

build: $(APP) $(APP2)

clean:
	@rm -fr $(APP) $(APP2)

run: build
	@./$(APP)
	@./$(APP2)

$(APP) $(APP2): 
	g++ $@.cpp Common/InitShader.o $(OPTIONS) -o $@

