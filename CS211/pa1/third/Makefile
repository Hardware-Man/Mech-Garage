all: third

third: third.c
	gcc -Werror -Wall -fsanitize=address -std=c11 third.c -o third

clean:
	rm -f third
