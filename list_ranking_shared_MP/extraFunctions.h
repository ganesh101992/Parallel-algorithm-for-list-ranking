#ifndef EXTRAFUNCTIONS
#define EXTRAFUNCTIONS

#include <stdio.h>
#include <stdlib.h>

int compute_element(int *vec, int size);
int sum_elements(int *vec,int size);
int** init_vecA(int size,int procs);
int* init_vecb(int size);
void print_vecA(int **vec, int size);
void print_vecb(int *vec, int size);

#endif
