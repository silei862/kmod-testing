CC		=gcc
CFLAGS	=-O3
SUBDIR	=kmod
BIN	=psex
OBJ	=
SRC	=psex.c

$(BIN)	:$(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)
	cd $(SUBDIR) && $(MAKE)

.PHONY:clean
clean:
	cd $(SUBDIR) && $(MAKE) clean
	rm -f $(BIN) $(OBJ)
