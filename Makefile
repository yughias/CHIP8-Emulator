all:
	gcc -Iinclude -Llib src/*.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lole32 -lComdlg32