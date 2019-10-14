CFLAGS=-lglut  -lGL -lm -lstdc++ -lGLU -O3 -no-pie
CC=g++ 
it:pt-finder.cpp  Makefile
	$(CC)pt-finder.cpp -o pt-finder $(CFLAGS)