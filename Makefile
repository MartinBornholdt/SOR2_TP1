.PHONY: all clean

SRC = 2b.c 3a.c 4b.c 4c.c read_boot.c
BIN = $(SRC:.c=)

clean:
	rm -f $(BIN) $(OBJ)

all: 
	gcc 2b.c -o 2b
	gcc 3a.c -o 3a
	gcc 4b.c -o 4b
	gcc 4c.c -o 4c
	gcc read_boot.c -o read_boot
