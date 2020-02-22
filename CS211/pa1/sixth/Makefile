all: sixth

sixth: sixth.c
	gcc -g -Werror -Wall -fsanitize=address -std=c11 sixth.c -o sixth

clean:
	rm -f sixth
