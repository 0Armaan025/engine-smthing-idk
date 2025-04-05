all:  
	g++ main.cpp glad/src/glad.c -o main -Iglad/include -ISDL2/include -LSDL2/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -lopengl32
