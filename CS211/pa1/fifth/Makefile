all: fifth

fifth: fifth.c
	gcc -Werror -Wall -fsanitize=address -std=c11 fifth.c -o fifth

clean:
	rm -f fifth
