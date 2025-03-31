all:
	g++ main.cpp -o main -I SDL2/include -L SDL2/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image
