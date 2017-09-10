CC = gcc
CFLAGS = -Wall -fPIC -pthread

build: libscheduler.so

libscheduler.so: so_scheduler.o utils.o queue.o
	$(CC) $(CFLAGS) -shared $^ -o $@

so_scheduler.o: so_scheduler.c
utils.o: utils.c
queue.o: queue.c

clean:
	rm -f *.o *.so
