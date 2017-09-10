CC = gcc
CFLAGS = -Wall -g -fPIC

build: libvmsim.so

libvmsim.so: vmsim.o common_lin.o utils.o
	$(CC) -Wall -shared $^ -o $@

vmsim.o: vmsim.c
common_lin.o: common_lin.c
utils.o: utils.c

clean:
	rm -f *.o *.so ram* swap*
