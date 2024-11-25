FLAGS=-O2 -fpermissive -fopenmp

CC=g++

RM=rm -f

EXEC=bucketsort

all: $(EXEC)

$(EXEC): main.c bucketsort.c bucketsort.h
	$(CC) $(FLAGS) bucketsort.c -c -o bucketsort.o
	$(CC) $(FLAGS) main.c -c -o main.o
	$(CC) $(FLAGS) main.o bucketsort.o -o $(EXEC)

run:
	./$(EXEC)

clean:
	$(RM) main.o bucketsort.o $(EXEC)
