all:
	gcc -o s snake_terminal.c -lncurses
	gcc -o s1 snake1.c -lncurses

clean:
	rm s s1