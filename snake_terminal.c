// demo.c
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define DELAY 1000		// 1ms tick 
#define MAXLEN		100
#define MAXTURNQ	50

typedef enum{
	N,
	E,
	W,
	S
}dir;

static char get_bodychar(int x,dir d);
static void end();
static void rem_turn();


struct block{
	char c;
	int ishead;
	int x;
	int y;
	dir d;
}block;

struct snake{
	struct block body[MAXLEN];
	int len;
}snake;

struct turn{
	int active;
	int x;
	int y;
	dir old;
	dir new;
}turn;

struct turn tq[MAXTURNQ];
struct tuq{
	int start;
	int end;
	int count;
};
static struct tuq tuq = { .start = 0, .end = 0, .count = 0};


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
 
 	for(int i = 0; i< s->len;i++){
 		c = get_bodychar(s->body[i].ishead,s->body[i].d);
 		mvprintw(s->body[i].y,s->body[i].x,&c);
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
	if(*x >= 80 || *x < 0 || *y >= 48 || *y < 0){
		end();
	}
}

void update(void *ctx)
{
	struct context *p = (struct context *)ctx;

	struct snake *s = &(p->s);
	struct block *f = &(p->f);


	static int f_init = 1;
	if(f_init){
		f_init = 0;
		int xpos = rand()%80;
		int ypos = rand()%30;

		f->x = xpos;
		f->y = ypos;
	}
	

	static long up_s = 0;
	if(up_s++ > 50){

		int k = 0;
		// find hits with turn for every point of snake
		for(int i = 0; i< s->len;i++){
			
			if(i == 0){
				// check if head has hit fruit
				if(s->body[i].x == f->x && s->body[i].y == f->y){
					f_init = 1;
					s->len = s->len + 1;
					int k = s->len - 1;
					int xp = s->body[k-1].x;
					int yp = s->body[k-1].y;

					switch(s->body[k-1].d){
					case N: s->body[k].x = xp;s->body[k].y = yp+1 ;s->body[k].d = N; break;
					case E: s->body[k].x = xp-1;s->body[k].y = yp;s->body[k].d = E; break;
					case W: s->body[k].x = xp+1;s->body[k].y = yp;s->body[k].d = W; break;
					case S: s->body[k].x = xp;s->body[k].y = yp-1;s->body[k].d = S; break;
					}
					
				}
			}
			for(int j = 0; j<MAXTURNQ; j++)
				
				if(tq[j].active == 1){	// check if turn is active
					if(s->body[i].x == tq[j].x && s->body[i].y == tq[j].y){
 						s->body[i].d = tq[j].new;

 						// make turn inactive if last point in body
 						if(i == (s->len - 1)){
 							tq[j].active = 0;
 						}
				}	
 			}
 			get_newpos(s->body[i].d,&(s->body[i].x),&(s->body[i].y));
		}
		for(int i = 1;i< s->len; i++){
			// check head collision with body
			if(s->body[0].x == s->body[i].x && s->body[0].y == s->body[i].y){
				end();
			}
		}
	
		up_s =0;
	}
	
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

static void snake_init(struct snake *s)
{
	s->len = 5;
 	s->body[0].ishead = 1;

	for(int i = 0;i<s->len;i++){
		s->body[i].d = E;
		s->body[i].c = get_bodychar(s->body[i].ishead,s->body[i].d);
		s->body[i].x = s->len-i;
		s->body[i].y = 0;
	}
}

static void rem_turn()
{
	if(tuq.count != 0){
		tuq.start = (tuq.start + 1)%MAXTURNQ;
		tuq.count--;
	}
}

static void add_turn(int x,int y,dir old,dir new)
{
	int idx = tuq.end;
	tq[idx].active = 1;
	tq[idx].x = x;
	tq[idx].y = y;
	tq[idx].old = old;
	tq[idx].new = new;
	
	tuq.end = (tuq.end + 1)%MAXTURNQ;
	tuq.count++;
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
	
	snake_init(&(ctx.s));

	
	while(1) {
		
		int c = getch();
		int posx = ctx.s.body[0].x;
		int posy = ctx.s.body[0].y;
		dir old = ctx.s.body[0].d;

		switch(c){
		case KEY_UP: 
				if(old != N && old != S){
					add_turn(posx,posy,old,N);
				}  
		break;	
		case KEY_RIGHT: 
				if(old != E && old != W){
					add_turn(posx,posy,old,E);
				}  
		break;
		case KEY_LEFT: 
				if(old != W && old != E){
					add_turn(posx,posy,old,W);
				}  
		break;
		case KEY_DOWN: 
				if(old != S && old != N){
					add_turn(posx,posy,old,S);
				}  
		break;
		case ERR:
		break;
		}
        	
        update(&ctx);
 		draw(&ctx);
		
 		usleep(DELAY); // Shorter delay between movements
 	}

	endwin(); // Restore normal terminal behavior
}

