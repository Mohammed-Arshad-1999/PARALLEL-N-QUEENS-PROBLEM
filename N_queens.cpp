#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>
#include<getopt.h>
#define MAX_DIMENSIONS 25
#define MAX_THREADS 9
typedef struct
{
int size;
int top;
int matrix[MAX_DIMENSIONS][MAX_DIMENSIONS];
int stack[MAX_DIMENSIONS];
}board_t;
void* create_start(void*);
void init(board_t*);
void scan(board_t*,int,int);
void mark_attacks(board_t*,int,int);
void print_board(board_t*);
void parse_options(int,char**,int*,int*,int*);
pthread_mutex_t SOL_MUTEX;
pthread_mutex_t JOB_MUTEX;
long SOLUTIONS=0;
int CURR_BOARD_INDEX=-1;
int THREADS=1;
int SIZE=4;
int QUIET=0;
board_t *BOARD[MAX_DIMENSIONS];
int main(int argc, char **argv)
{
 int i,rc;
 char s;
 clock_t start,end;
 start=clock();
 srand(time(NULL));
 pthread_t thread_id[MAX_THREADS];
 pthread_mutex_init(&SOL_MUTEX,NULL);
 pthread_mutex_init(&JOB_MUTEX,NULL);
 parse_options(argc,argv,&THREADS,&SIZE,&QUIET);
 if(SIZE==0)
 {
 SOLUTIONS=1;
 }
 else
 {
 for(i=1;i<THREADS;i++)
 rc=pthread_create(&thread_id[i-1],NULL,create_start,(void *)SIZE);
 create_start((void *)SIZE);
 for(i=0;i<THREADS-1;i++)
 pthread_join(thread_id[i],NULL);
 }
 pthread_mutex_destroy(&SOL_MUTEX);
 pthread_mutex_destroy(&JOB_MUTEX);
 for(i=0;i<SIZE;i++)
 free(BOARD[i]);
 s=(SOLUTIONS==1)?'\0':'s';
 printf("%d solution%c found\n",SOLUTIONS,s);
 end=clock();
 double time=((double)(end-start))/CLOCKS_PER_SEC;
 printf("total time is %f",time);
 return 0;
}
void* create_start(void* numb_rows)
{
 int my_board_index;
 while(CURR_BOARD_INDEX<(int)numb_rows-1)
 {
 pthread_mutex_lock(&JOB_MUTEX);
 my_board_index=++CURR_BOARD_INDEX;
 pthread_mutex_unlock(&JOB_MUTEX);
 BOARD[my_board_index]=(board_t *)malloc(sizeof(board_t));
 BOARD[my_board_index]->size=(int)numb_rows;
 init(BOARD[my_board_index]);
 scan(BOARD[my_board_index],my_board_index,0);
 }
}
void scan(board_t* board,int start_row,int col)
{
 int i,r,c;
 int row;
 int copy[MAX_DIMENSIONS][MAX_DIMENSIONS];
 for(r=0;r<board->size;r++)
 {
 for (c=0;c<board->size;c++)
 copy[r][c]=board->matrix[r][c];
 }
 if(col==board->size)
 {
 pthread_mutex_lock(&SOL_MUTEX);
 SOLUTIONS++;
 if(!QUIET)
 {
 print_board(board);
printf("\n");
 }
 pthread_mutex_unlock(&SOL_MUTEX);
 return;
 }
 for(row=start_row;row<board->size;row++)
 {
 if(board->matrix[row][col]==1)
 {
 board->stack[board->top++]=row;
 mark_attacks(board,row,col);
 scan(board,0,col+1);
 if(col==0)
 return;
 for(r=0;r<board->size;r++)
{
 for(c=0;c<board->size;c++)
 board->matrix[r][c]=copy[r][c];
 }
 board->top--;
 }
 }
 return;
}
void mark_attacks(board_t* board,int row, int col)
{
 int up=row;
 int down=row;
 int c;
 board->matrix[row][col]=8;
 for(c=col+1;c<board->size;c++)
 {
 --up;
 ++down;
 board->matrix[row][c]=0;
 if(up>=0)
 board->matrix[up][c]=0;
 if(down<board->size)
 board->matrix[down][c]=0;
 }
}
void print_board(board_t* board)
{
 int row,col;
 for(row=0;row<board->size;row++)
 {
 for(col=0;col<board->size;col++)
 {
 if(board->matrix[row][col]==8)
{
printf(" Q ");
}
else
{
printf(" . ");
}
 }
 printf("\n");
 }
}
void init (board_t* board)
{
 int row,col;
 for(row=0;row<board->size;row++)
 {
 for(col=0;col<board->size;col++)
 board->matrix[row][col]=1;
 }
}
void parse_options(int argc,char** argv,int* t,int* n,int* q)
{
 int c;
 while(c!=-1)
 {
 c=getopt(argc,argv,"t:n:qh");
 switch(c)
 {
 case 't':
 *t=atoi(optarg);
 break;
 case 'n':
 *n=atoi(optarg);
 break;
 case 'q':
 *q=1;
 break;
 case 'h':
 printf("USAGE: p1 [-t threads -n size -q]\n");
 printf("-t threads (default 1, max 9)\n");
 printf("-n size(default 4, max 25)\n");
 printf("-q(only print # solutions)\n");
 exit(0);
 }
 }
 *n=*n>25?25:*n;
 *t=*t>9?9:*t;
}