#include "extraFunctions.h"
#define MAX_RAND 100

int** init_vecA(int size,int procs)
{
    int i;
    int **vec=(int**) malloc( 2 * size * sizeof(int) );
    vec[0]=(int*) malloc( size * sizeof(int) );
    vec[1]=(int*) malloc( size * sizeof(int) );
    for (i = 0; i < size-1; i++)
    {
        vec[0][i] = i+1;
        if((i+1+(size/procs))<size+1){
            vec[1][i]= i+1+(size/procs);
        }else
        {
            vec[1][i]= ((i+1+(size/procs))%(size))+1;
        }
    }
    vec[0][size-1]=size;
    vec[1][i]=0;
    return vec;
}


int* init_vecb(int size)
{
    int i;
    int *vec=(int*) malloc( size * sizeof(int) );
    for (i = 0; i < size; i++)
    {
        //vec[i]=1;
        vec[i] = rand() % MAX_RAND;
    }
    return vec;  
}

void print_vecA(int **vec, int size)
{
    printf("Initial vector 'A' : \n");
    
    int i;
    for (i = 0; i < 2; i++)
    {
        for(int j=0;j<size;j++)
           printf("%d ", vec[i][j]);
        printf("\n");
    }
    printf("\n\n");
}

void print_vecb(int *vec, int size)
{
    printf("Initial vector 'b' : \n");
    
    int i;
    for (i = 0; i < size; i++)
    {
        printf("%d ", vec[i]);
    }
    printf("\n\n");
}

int compute_element(int *vec, int size)
{
    int row_value=0;
    for(int i=0;i<size;i+=2)
       row_value+=(vec[i]*vec[i+1]);

    return row_value;
}

int sum_elements(int *vec,int size){
    int sum=0;
    for(int i=0;i<size;i++)
       sum+=vec[i];

    return sum;
}
