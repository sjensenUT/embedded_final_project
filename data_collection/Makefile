.PHONY: all clean

CC = gcc
#CFLAGS = $(DEBUG_FLAG)

all: bb_measure

bb_measure: bb_measure.o ../timing-harness/run_test.o ../timing-harness/rdpmc.o ../timing-harness/libharness.a
	$(CC) -o $@ $^ -lrt


bb_measure.o: bb_measure.c

clean:
	rm -f bb_measure.o bb_measure
