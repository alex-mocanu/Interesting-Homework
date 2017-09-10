CC=g++
CFLAGS=-g -Wall -std=c++11 -D FEP_OPENCL

LIB=-L/usr/local/cuda-8.0/lib64/

all:	accpop

accpop: helper.cpp helper.hpp utils.cpp host.cpp
	$(CC) $(CFLAGS) $(LIB) host.cpp utils.cpp helper.cpp -lOpenCL -o accpop

clean:
	rm -f accpop
