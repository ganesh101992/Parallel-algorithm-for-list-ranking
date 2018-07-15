#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#include "extraFunctions.h"

int main(int argc, char *argv[])
{
    // Declare process-related vars (note: each process has its own copy)
    // and initialize MPI
    int my_rank,num_procs,index=-1,chunk_size,inserted;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //grab this process's rank

    int size_of_matrix=num_procs*100;
    srand(time(NULL));
    double start_time,stop_time;
        
    chunk_size=size_of_matrix/num_procs;
    int *sub_arr=(int*) malloc( chunk_size*2 * sizeof(int) );

    int *ranked; MPI_Win win,win1;
    ranked=(int*) malloc( size_of_matrix  * sizeof(int));
    for(int i=0;i<size_of_matrix;i++)
       ranked[i]=-1;
    MPI_Win_create(ranked, size_of_matrix, sizeof(int),MPI_INFO_NULL,MPI_COMM_WORLD, &win);
    MPI_Win_create(&index, 1, sizeof(int),MPI_INFO_NULL,MPI_COMM_WORLD, &win1);

    if(my_rank==0){
        int WORKTAG=1;
        //grab the total num of processes
    
        int **input=(int**) malloc( 2 * size_of_matrix  * sizeof(int));
        for(int i=0;i<2;i++)
           input[i]=(int*) malloc( size_of_matrix  * sizeof(int));
        input=init_vecA(size_of_matrix,num_procs);
        //print_vecA(input,size_of_matrix);
        for(int i=1;i<num_procs;i++){
	   int col=0;
	   for(int j=i*chunk_size;j<((i+1)*chunk_size);j++){
	    sub_arr[col]=input[0][j];
	    sub_arr[col+1]=input[1][j];
	    col+=2;
	   }
        //printf("Sending initial work...\n");
	MPI_Send(sub_arr,chunk_size*2,MPI_INT,i,WORKTAG,MPI_COMM_WORLD);       
        }
	int col=0;
	for(int j=0;j<(chunk_size);j++){
	    sub_arr[col]=input[0][j];
	    sub_arr[col+1]=input[1][j];
	    col+=2;
	}
    }
    else{
        MPI_Status status;

        MPI_Recv(sub_arr,chunk_size*2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    }

    start_time = MPI_Wtime();
    int start=1;

    MPI_Win_fence((MPI_MODE_NOPRECEDE), win1);
    MPI_Get(&index, 1, MPI_INT, 0, 0, 1, MPI_INT, win1);
    MPI_Win_fence(( MPI_MODE_NOSUCCEED), win1); 
    do{
      inserted=0;
      //printf("process %d read index=%d\n",my_rank,index);

      if(index==-1){
        MPI_Win_fence(MPI_MODE_NOPRECEDE, win);
        for(int i=start;i<2*chunk_size;i+=2)
           if(sub_arr[i]==0){
              index++;
              MPI_Put(&sub_arr[i-1], 1, MPI_INT, 0, index, 1, MPI_INT, win);
              inserted=1;

              sub_arr[i-1]=sub_arr[start-1];
              sub_arr[i]=sub_arr[start];
              start+=2;
              //printf("process %d inserted value=%d\n", my_rank, sub_arr[i-1]);
           }
        MPI_Win_fence((MPI_MODE_NOSTORE |MPI_MODE_NOSUCCEED),win);

        MPI_Win_fence(MPI_MODE_NOPRECEDE, win1);
        if(inserted==1)
          MPI_Put(&index, 1, MPI_INT, 0, 0, 1, MPI_INT, win1);
        MPI_Win_fence((MPI_MODE_NOSTORE |MPI_MODE_NOSUCCEED), win1);           
      }
      else{
           int element=-1;
           MPI_Win_fence((MPI_MODE_NOPRECEDE), win);
           MPI_Get(&element, 1, MPI_INT, 0, index, 1, MPI_INT, win);
           MPI_Win_fence(( MPI_MODE_NOSUCCEED), win);

           MPI_Win_fence(MPI_MODE_NOPRECEDE, win);
           for(int i=start;i<2*chunk_size;i+=2)
              if(sub_arr[i]==element){
                index++;
                MPI_Put(&sub_arr[i-1], 1, MPI_INT, 0, index, 1, MPI_INT, win);
                inserted=1;

                element=sub_arr[i-1];
                sub_arr[i-1]=sub_arr[start-1];
                sub_arr[i]=sub_arr[start];
                //i=start;
                start+=2;
                //printf("process %d inserted value=%d\n", my_rank, sub_arr[i-1]);
              }
           MPI_Win_fence((MPI_MODE_NOSTORE |MPI_MODE_NOSUCCEED), win);

           MPI_Win_fence(MPI_MODE_NOPRECEDE, win1);
           if(inserted==1)
             MPI_Put(&index, 1, MPI_INT, 0, 0, 1, MPI_INT, win1);
           MPI_Win_fence((MPI_MODE_NOSTORE |MPI_MODE_NOSUCCEED), win1); 
      }
      MPI_Win_fence((MPI_MODE_NOPRECEDE), win1);
      MPI_Get(&index, 1, MPI_INT, 0, 0, 1, MPI_INT, win1);
      MPI_Win_fence(( MPI_MODE_NOSUCCEED), win1); 
    }while((index)!=(size_of_matrix-1));

    stop_time = MPI_Wtime();

    MPI_Win_free(&win);
    MPI_Win_free(&win1);  

    if(my_rank==0){
       printf("Total time (sec): %f\n", stop_time - start_time);
       printf("The ranked list is as follows :\n");
       for(int i=0;i<size_of_matrix;i++)
	  printf("%d ",ranked[i]);
       printf("\n");
       //printf("process0 reading updated data %d\n",ranked[0]);
    }

    // Shutdown MPI (important - don't forget!)
    printf("process%d terminating...\n",my_rank);
    MPI_Finalize();

    return 0;
}
