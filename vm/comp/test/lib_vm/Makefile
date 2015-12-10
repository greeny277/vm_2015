# $Id: Makefile 10011 2014-10-15 16:08:01Z raimue $

CFLAGS=-O2 -Wall -Werror

COMBINE_ROM_OBJ=combine_rom.o

all: combine_rom


combine_rom: $(COMBINE_ROM_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) combine_rom

.PHONY: clean
