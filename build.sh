gcc -c midi.c
g++ -g -L./custom_freeglut arrange.cpp x11.cpp midi.o minilisp/bignum.o minilisp/reader.o minilisp/minilisp.o -I/usr/local/include/GLV -I/usr/local/include/lyd-0.0/ -llyd-0.0 -L/usr/local/lib/ -lsndfile -lGLV -lGL -lGLU -lglut -lGLEW -lpthread -lX11 -lXt -ljack -std=gnu++11 -Wno-write-strings -fpermissive -o produce
