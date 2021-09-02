all:	src/flash_bash.c
	gcc -g -lwiringPi -Wall -o flash_bash src/flash_bash.c

clean:
	rm -rf flash_bash flash_bash.dSYM/