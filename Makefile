CC=gcc
CFLAGS+=-std=c99 -Wall -I. -O3 -lrt

DEPS = fq.h st.h rbt.h
OBJ1 = fq.o st.o rbt.o rbt_test.o
OBJ2 = fq.o rbt.o rbt_example.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
all: rbt_test rbt_example

rbt_test: $(OBJ1)
	$(CC) -o $@ $^ $(CFLAGS)
rbt_example: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf *.o *~ core rbt_test rbt_example
