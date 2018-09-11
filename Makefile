all:
	mkdir -p build
	gcc src/main.c -o build/main -lSDL2 -lSDL2_image -g

run: all
	./build/main
