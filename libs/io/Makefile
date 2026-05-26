AS=nasm
ASFLAGS=-f elf64

ASS=$(wildcard *.s)
OBJ=$(ASS:.s=.o)

%.o: %.s
	@$(AS) $(ASFLAGS) -o $@ $^

all: $(OBJ)

clean:
	@rm -f *.o
