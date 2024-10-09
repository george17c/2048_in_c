CC=gcc
OUT=2048

build:
	$(CC) -Wall -g 2048.c -o $(OUT) -lncurses

run:
	./$(OUT)

clean:
	rm -f $(OUT)


