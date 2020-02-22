all: fourth

fourth: fourth.c
	gcc -Werror -Wall -fsanitize=address -std=c11 fourth.c -o fourth

clean:
	rm -f fourth
