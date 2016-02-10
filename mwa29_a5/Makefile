APP=shapes
GCC_OPTIONS=-Wall -Wno-unused-function -pedantic -I include
GL_OPTIONS=-lGLEW -lGL -lglut
OPTIONS=$(GCC_OPTIONS) $(GL_OPTIONS)

build: $(APP)

clean:
	@rm -fr $(APP)

run: build
	@./$(APP)

$(APP): 
	@g++ $@.cpp Common/InitShader.o $(OPTIONS) -o $@

