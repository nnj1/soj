osx:
	g++ -std=c++11 main.cpp -o bin/dank -I mac/include -L mac/lib -l SDL2-2.0.0 -l SDL2_ttf-2.0.0 -l SDL2_image-2.0.0 -l SDL2_mixer-2.0.0
mingw:
	g++ -std=c++11 main.cpp -o bin/dank -I windows/include -L windows/lib -l SDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lmingw32 -lSDL2main -lSDL2
