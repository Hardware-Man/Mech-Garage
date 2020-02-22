all: eighth

eighth: eighth.c
	gcc -g -Werror -Wall -fsanitize=address -std=c11 eighth.c -o eighth

clean:
	rm -f eighth
