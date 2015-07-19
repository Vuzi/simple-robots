# Simple robot common Makefile

CC= gcc
CFLAGS= --std=c99
INCLUDES= ./includes
LDFLAGS= -lpthread

LIBDIR= lib
OBJDIR= obj

LIB= $(LIBDIR)/common.a

SRC= $(wildcard *.c)
OBJ= $(patsubst %.c, $(OBJDIR)/%.o, $(SRC))

all: directories msg $(LIB) 

msg:
	@echo "         Simple Robot common library building..."

directories:
	@mkdir -p ${OBJDIR} ${LIBDIR}

$(LIB): $(OBJ)
	@ar rcs $@ $^

$(OBJDIR)/%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) -I $(INCLUDES)
	

.PHONY: clean mrproper

clean:
	@rm -rf $(OBJ)

mrproper: clean
	@rm -rf $(LIB) $(OBJ)
