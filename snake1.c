// demo.c
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define DELAY 		80000		// 1ms tick 
#define MAXLEN		100
#define MAXTURNQ	50

typedef enum{
	N,
	E,
	W,
	S
}dir;

static char get_bodychar(int x,dir d);

struct block{
	char c;
	int ishead;
	int x;
	int y;
	dir d;
	struct block *n;
};

struct snake{
	struct block *head;
	struct block *tail;
	int len;
};

struct context{
	struct snake s;
	struct block f;
};

void draw(void *ctx)
{
	struct context *p = (struct context *)ctx;

	clear(); // Clear the screen of all
 	
	struct snake *s = &(p->s);
	struct block *f = &(p->f);

	mvprintw(f->y,f->x,&(f->c));
 	
 	static char c;
 
 	struct block *cur = s->head;
 	for(; cur != NULL;cur = cur->n){
 		c = get_bodychar(cur->ishead,cur->d);
 		mvprintw(cur->y,cur->x,&c);
 	}
 	
 	refresh();
}

void end(){
	while(1){
		sleep(1);
	}
}

static void get_newpos(dir d,int *x,int *y)
{
	switch(d){
	case N: (*y)--; break;
	case E: (*x)++; break;
	case W: (*x)--; break;
	case S: (*y)++; break;
	}
	#if 0
	if(*x >= 80 || *x < 0 || *y >= 48 || *y < 0){
		end();
	}
	#endif
	if(*x >= 80)	*x = 0;
	if(*x < 0)		*x = 79;
	if(*y >= 48)	*y = 0;
	if(*y < 0)		*y = 47;
}

static char get_bodychar(int ishead,dir d)
{
	char c;
	if(ishead == 1){
		switch(d){
		case N: c = '^'; break;
		case E: c = '>'; break;
		case W: c = '<'; break;
		case S: c = 'v'; break;
		}
	}else{
		switch(d){
		case N: c = '$'; break;
		case E: c = '$'; break;
		case W: c = '$'; break;
		case S: c = '$'; break;
		}
	}
	return c;
}

void snake_addf(struct snake *s,int x,int y,dir d)
{
	struct block *t = malloc(sizeof(struct block));
	t->x = x;
	t->y = y;
	t->d = d;

	t->ishead = 1;	// new node will always be head

	if(s->head == NULL){
		t->n = NULL;
		t->c = get_bodychar(1,t->d);
		s->head = s->tail = t;
	}else{
		s->head->ishead = 0;
		s->head->c = get_bodychar(0,s->head->d);
		t->n = s->head;
		s->head = t;
	}
}

void snake_pop_tail(struct snake *s)
{
	if(s == NULL){
		return;
	}
	struct block *cur = s->head;
	if(cur == NULL){
		return;
	}
	if(cur == s->tail){
		free(cur);
		s->head = s->tail = NULL;
		return;
	}
	while(cur->n != s->tail){
		cur = cur->n;
	}

	s->tail = cur;
	free(cur->n);
	cur->n = NULL;
}



static void snake_init(struct snake *s)
{
	s->len = 3;
 	
	for(int i = 0;i<s->len;i++){
		snake_addf(s,i,0,E);
	}
}



int main(int argc, char *argv[]) {

 	initscr();
 	cbreak();
 	nodelay(stdscr,TRUE);
 	keypad(stdscr, TRUE);
 	scrollok(stdscr, TRUE);
 	noecho();

 	curs_set(FALSE);

 	srand(time(NULL));

 	struct context ctx = {  		
 		.f = { .c = '*', .x = 50, .y=30 }
 	};
	
	snake_init(&ctx.s);

	
	while(1) {
		
		struct block *f = &ctx.f;
		struct snake *s = &ctx.s;
		struct block *h = s->head;
		int nx = h->x;
		int ny = h->y;

		get_newpos(h->d,&nx,&ny);

		int c = getch();
		switch(c){
		case KEY_UP: 
				if(h->d == S){
					snake_addf(s,nx,ny,S);
				}else{
					snake_addf(s,nx,ny,N);
				}
		break;	
		case KEY_RIGHT: 
				if(h->d == W){
					snake_addf(s,nx,ny,W);
				}else{
					snake_addf(s,nx,ny,E);
				}
		break;
		case KEY_LEFT: 
				if(h->d == E){
					snake_addf(s,nx,ny,E);
				}else{
					snake_addf(s,nx,ny,W);
				}
		break;
		case KEY_DOWN: 
				if(h->d == N){
					snake_addf(s,nx,ny,N);
				}else{
					snake_addf(s,nx,ny,S);
				}
		break;
		case ERR:
				snake_addf(s,nx,ny,h->d);
		break;
		}

		struct block *walk = s->head->n;
		while(walk != NULL){
			if(s->head->x == walk->x && s->head->y == walk->y){
				end();
			}
			walk = walk->n;
		}
		if(s->head->x == f->x && s->head->y == f->y){
			int xpos = rand()%80;
			int ypos = rand()%30;

			f->x = xpos;
			f->y = ypos;
		}else{
			snake_pop_tail(s);
		}
        
        
 		draw(&ctx);
		
 		usleep(DELAY); // Shorter delay between movements
 	}

	endwin(); // Restore normal terminal behavior
}

