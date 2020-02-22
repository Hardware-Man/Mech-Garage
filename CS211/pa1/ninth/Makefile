all: ninth

ninth: ninth.c
	gcc -g -Werror -Wall -fsanitize=address -std=c11 ninth.c -o ninth

clean:
	rm -f ninth
