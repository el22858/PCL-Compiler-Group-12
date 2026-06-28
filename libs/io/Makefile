AS=nasm
ASFLAGS=-f elf64

ASS=$(wildcard *.s)
OBJ=$(ASS:.s=.o)

%.o: %.s
	@$(AS) $(ASFLAGS) -o $@ $^

all: writeString.o writeInteger.o writeChar.o

clean:
	@rm -f *.o