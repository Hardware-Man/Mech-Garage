all: seventh

seventh: seventh.c
	gcc -g -Werror -Wall -fsanitize=address -std=c11 seventh.c -o seventh

clean:
	rm -f seventh
