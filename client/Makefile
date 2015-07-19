# Simple robot client Makefile

CC= gcc
CFLAGS= --std=c99
INCLUDES= ../common/includes
LDFLAGS= -lpthread
COMMON= ../common/lib/common.a

BINDIR= bin
OBJDIR= obj

BIN= $(BINDIR)/client

SRC= $(wildcard *.c)
OBJ= $(patsubst %.c, $(OBJDIR)/%.o, $(SRC))

all: directories msg $(BIN)

msg:
	@echo "         Simple Robot Client building..."

directories:
	@mkdir -p ${OBJDIR} ${BINDIR}

$(BIN): $(OBJ)
	@$(CC) -o $@ $^ $(CFLAGS) $(COMMON) $(LDFLAGS) -I $(INCLUDES)

$(OBJDIR)/%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) -I $(INCLUDES)

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(BIN) $(OBJ)
