CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lm

SRC = main.c queue.c data.c bestfit.c
OBJ = $(SRC:.c=.o)
 
EXE = allocate

$(EXE): $(OBJ)
	gcc -Wall -o allocate $(OBJ) $(LDLIBS)

format:
	clang-format -style=file -i *.c

main.o: main.c queue.h data.h bestfit.h

queue.o: queue.c queue.h data.h

data.o: data.c data.h

bestfit.o: bestfit.c bestfit.h queue.h

clean:
	@rm -f *.o $(EXE)