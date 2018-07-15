#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#include "extraFunctions.h"

static void master(int size, int num_procs){
    int WORKTAG=1;
    MPI_Status status;
    //grab the total num of processes
    
    //int **input=(int **)input1;
    int chunk_size=size/num_procs;
    int *sub_arr=(int*) malloc( chunk_size*2 * sizeof(int) );
    int **input=(int**) malloc( 2 * size  * sizeof(int));
    for(int i=0;i<2;i++)
	input[i]=(int*) malloc( size  * sizeof(int));

    int *ranked=(int*) malloc( size * sizeof(int) );
    int index=-1,index_prev=-1;
    input=init_vecA(size,num_procs);
    //print_vecA(input,size);
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
    //print_vecb(sub_arr,chunk_size*2);
    
    double start_time,stop_time;
    start_time = MPI_Wtime();

    int present=0,start=1;
    for(int i=start;i<2*chunk_size;i+=2){
       if(sub_arr[i]==0){
         ranked[++index]=sub_arr[i-1];
         sub_arr[i-1]=sub_arr[start-1];
         sub_arr[i]=sub_arr[start];
         start+=2;
         index_prev=index;
         for(int j=1;j<num_procs;j++){
              MPI_Ssend(&index,1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD); 
              MPI_Ssend(&ranked[index],1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD);
              //printf("process0 sending index=%d to j=%d\n",index,j);
         }
         present=1;        
       }
    }
    if(present==0){
      //do{
        MPI_Recv(&index,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        MPI_Recv(&ranked[index],1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        //printf("process0 received index=%d\n",index);
      //}while(!(index-index_prev==1));
      //index_prev=index;
    }
    do{
      present=0;
      for(int i=start;i<2*chunk_size;i+=2){
       if(sub_arr[i]==ranked[index]){
         ranked[++index]=sub_arr[i-1];
         sub_arr[i-1]=sub_arr[start-1];
         sub_arr[i]=sub_arr[start];
         start+=2;
         index_prev=index;
         for(int j=1;j<num_procs;j++){
              MPI_Ssend(&index,1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD); 
              MPI_Ssend(&ranked[index],1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD);
              //printf("process0 sending index=%d to j=%d\n",index,j);
         }
         present=1;         
       }
      }
      if(present==0){
        //do{
          MPI_Recv(&index,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
          MPI_Recv(&ranked[index],1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
          //printf("process0 received index=%d\n",index);
        //}while(!(index-index_prev==1));
        //index_prev=index;
      }
    }while(index!=(size-1));


    stop_time = MPI_Wtime();
    printf("Total time (sec): %f\n", stop_time - start_time);
    
    printf("Ranked list is as follows :\n");
    for(int i=0;i<size;i++)
       printf("%d ",ranked[i]);
    printf("\n");
}

static void slave(int size, int my_rank){
    int num_procs;
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int chunk_size=size/num_procs;
    int *sub_arr=(int*) malloc( chunk_size*2 * sizeof(int) );

    int *ranked=(int*) malloc( size * sizeof(int) );
    int index=-1,index_prev=-1;
    MPI_Recv(sub_arr,chunk_size*2,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
    //printf("process%d :\n",my_rank);
    //print_vecb(sub_arr,chunk_size*2);

    int present=0,start=1, WORKTAG=1;
    for(int i=start;i<2*chunk_size;i+=2){
       if(sub_arr[i]==0){
         ranked[++index]=sub_arr[i-1];
         sub_arr[i-1]=sub_arr[start-1];
         sub_arr[i]=sub_arr[start];
         start+=2;
         index_prev=index;
         for(int j=0;j<num_procs;j++){
            if(j!=my_rank){
              MPI_Ssend(&index,1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD); 
              MPI_Ssend(&ranked[index],1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD);
              //printf("process%d sending index=%d to j=%d\n",my_rank,index,j);
            } 
         }
         present=1;         
       }
    }
    if(present==0){
      //do{
        MPI_Recv(&index,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        MPI_Recv(&ranked[index],1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        //printf("process%d received index=%d\n",my_rank,index);
      //}while(!(index-index_prev==1));
      //index_prev=index;
    }
    do{
      present=0;
      for(int i=start;i<2*chunk_size;i+=2){
       if(sub_arr[i]==ranked[index]){
         ranked[++index]=sub_arr[i-1];
         sub_arr[i-1]=sub_arr[start-1];
         sub_arr[i]=sub_arr[start];
         start+=2;
         index_prev=index;
         for(int j=0;j<num_procs;j++){
            if(j!=my_rank){
              MPI_Ssend(&index,1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD); 
              MPI_Ssend(&ranked[index],1,MPI_INT,j,WORKTAG,MPI_COMM_WORLD);
              //printf("process%d sending index=%d to j=%d\n",my_rank,index,j);
            } 
         }
         present=1;         
       }
      }
      if(present==0){
        //do{
          MPI_Recv(&index,1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
          MPI_Recv(&ranked[index],1,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
          //printf("process%d received index=%d\n",my_rank,index);
        //}while(!(index-index_prev==1));
        //index_prev=index;
      }
    }while(index!=(size-1));

    //printf("process%d count=%d\n",my_rank,count);
}

int main(int argc, char *argv[])
{
    // Declare process-related vars (note: each process has its own copy)
    // and initialize MPI
    int my_rank,num_procs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //grab this process's rank

    int size_of_matrix=num_procs*60;
    srand(time(NULL));

    if(my_rank==0){
	master(size_of_matrix,num_procs);
    }
    else
	slave(size_of_matrix,my_rank);

    // Shutdown MPI (important - don't forget!)
    printf("process%d terminating...\n",my_rank);
    MPI_Finalize();

    return 0;
}
