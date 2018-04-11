CC=gcc
CFLAGS=-I.

all: parallel_min_max zombie_process process_memory parallel_sum


parallel_sum : parallel_sum.c utils.o utils.h
	$(CC) utils.o parallel_sum.c -o parallel_sum -lpthread -L./libsum -lsum $(CFLAGS)
	
process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

zombie_process : zombie_process.c
	$(CC) -o zombie_process zombie_process.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o parallel_min_max zombie_process parallel_sum process_memory