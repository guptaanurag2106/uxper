all: fibonacci echo wc

fibonacci: fibonacci.asm
	nasm -felf64 fibonacci.asm && gcc -o fibonacci fibonacci.o && ./fibonacci 10

echo: echo.asm
	nasm -felf64 echo.asm && gcc -o echo echo.o && ./echo echoing stuff

wc: wc.asm
	nasm -felf64 wc.asm && gcc -o wc wc.o && ./wc misc.c

clean:
	rm -f *.o echo wc fibonacci misc

.PHONY: all clean
