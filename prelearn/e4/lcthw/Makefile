CFLAGS=-Wall -g

all: ex19

ex19: object.o game_logic.o

clean:
	rm -f ex1
	rm -f object.o

run: ex19
	./ex19

test: ex19t ex19
	./ex19t | ./ex19