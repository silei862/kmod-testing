CC			=gcc
CFLAGS		=-O2
LDFLAGS		=
KMOD_DIR	=kmod

BIN_CMOSRW	=cmosedit
OBJ_CMOSRW	=cmosedit.o
SRC_CMOSRW	=cmosedit.c

OBJ_LIBCMOS	=cmosio.o cmosinfo.o
SRC_LIBCMOS =cmosio.c cmosio.h cmosinfo.c cmosinfo.h

BIN			=$(BIN_CMOSRW)  
OBJ			=$(OBJ_CMOSRW) $(OBJ_LIBCMOS)

$(BIN)			:$(OBJ)
	$(CC) $(CFLAGS) -o $(BIN_CMOSRW) $(OBJ)
	cd $(KMOD_DIR) && $(MAKE)

$(OBJ_CMOSRW)	:$(SRC_CMOSRW)

$(OBJ_LIBCMOS)	:$(SRC_LIBCMOS)

.PHONY 		:clean
clean:
	cd $(KMOD_DIR) && $(MAKE) clean
	rm -f $(BIN) $(OBJ)

