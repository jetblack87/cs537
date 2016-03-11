APP=shapes
GCC_OPTIONS=-Wall -Wno-unused-function -pedantic -I include
GL_OPTIONS=-lGLEW -lGL -lglut
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

run_all:
	@make APP=shapes_a7 clean run
	@make APP=shapes_a6 clean run	

build: $(APP)

clean:
	@rm -fr $(APP)

run: build
	@./$(APP)

$(APP): 
	@g++ $@.cpp Common/InitShader.o $(OPTIONS) -o $@

