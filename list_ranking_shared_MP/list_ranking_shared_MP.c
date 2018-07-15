#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "omp.h"
#include "extraFunctions.h"

int main(int argc, char *argv[])
{
    int size_of_matrix=omp_get_max_threads()*75;
    int chunk_size=size_of_matrix/omp_get_max_threads();
    srand(time(NULL));
    double start_time,stop_time;
    int **input=(int**) malloc( 2 * size_of_matrix  * sizeof(int));
    for(int i=0;i<2;i++)
       input[i]=(int*) malloc( size_of_matrix  * sizeof(int));
    int *ranked=(int*) malloc( size_of_matrix  * sizeof(int));
    for(int i=0;i<size_of_matrix;i++)
       ranked[i]=0;

    input=init_vecA(size_of_matrix,omp_get_max_threads());
    //print_vecA(input,size_of_matrix);

    start_time = omp_get_wtime();
    int index=-1,start=0;
    #pragma omp parallel shared(index) firstprivate(start)
    {
        //printf("Hello inside the pragma from thread id %d (%d threads running).\n",omp_get_thread_num(), omp_get_num_threads());
        start=omp_get_thread_num()*chunk_size;
        int limit=(omp_get_thread_num()+1)*chunk_size;
        //printf("for thread%d start=%d limit=%d\n",omp_get_thread_num(),start,limit);
        for(int j=start;j<limit;j++)
           if(input[1][j]==0){
	      ranked[++index]=input[0][j];
              input[0][j]=input[0][start];
              input[1][j]=input[1][start];
              start++;
           }
        do{
          if(index!=-1)
          for(int j=start;j<limit;j++)
             if(input[1][j]==ranked[index]){
	        ranked[++index]=input[0][j];
                input[0][j]=input[0][start];
                input[1][j]=input[1][start];
                start++;
             }         
        }while(index!=size_of_matrix-1);
    }
    stop_time = omp_get_wtime();
    printf("Total execution time (sec): %f\n", stop_time - start_time);


    printf("The ranked list is as follows :\n");
    for(int j=0;j<size_of_matrix;j++)
       printf("%d ",ranked[j]);
    printf("\n");       

    return EXIT_SUCCESS;
}
