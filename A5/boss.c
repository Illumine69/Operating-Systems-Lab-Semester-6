/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 5 : Mutual exclusion and synchronization using semaphores
File: boss.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)
#define START 0

int main(){

    // Read the graph from the file
    FILE* graph = fopen("graph.txt", "r");

    // Read the number of vertices
    int n;
    fscanf(graph, "%d", &n);

    int *A, *T, *idx;
    A = (int *)malloc(n*n*sizeof(int));     // Adjacency matrix
    T = (int *)malloc(n*sizeof(int));       // Topological sort array
    idx = (int *)malloc(sizeof(int));       // Index of the next vertex to be added to the topological sort array

    int shmidA, shmidT, shmidx;
    int mtx, sync[n], ntfy;
    struct sembuf pop, vop;

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    key_t keyA = ftok("/tmp", START);
    key_t keyT = ftok("/tmp", START+1);
    key_t keyidx = ftok("/tmp", START+2);
    shmidA = shmget(keyA, n*n*sizeof(int), 0777|IPC_CREAT);
    shmidT = shmget(keyT, n*sizeof(int), 0777|IPC_CREAT);
    shmidx = shmget(keyidx, sizeof(int), 0777|IPC_CREAT);

    A = (int *)shmat(shmidA, 0, 0);
    T = (int *)shmat(shmidT, 0, 0);
    idx = (int *)shmat(shmidx, 0, 0);

    // Read the adjacency matrix
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            fscanf(graph, " %d", &A[i*n + j]);
        }
    }

    key_t ketmtx = ftok("/tmp", START+3);
    mtx = semget(ketmtx, 1, 0777|IPC_CREAT);
    semctl(mtx, 0, SETVAL, 1);      // Assigned 1 to the mutex to allow the first worker to enter the critical section

    key_t keyntfy = ftok("/tmp", START+4);
    ntfy = semget(keyntfy, 1, 0777|IPC_CREAT);
    semctl(ntfy, 0, SETVAL, 0);     // Assigned 0 to the notification semaphore to block the boss initially

    // Create n semaphores for synchronization
    for(int i=0;i<n;i++){
        key_t keysync = ftok("/tmp", START+5+i);
        sync[i] = semget(keysync, 1, 0777|IPC_CREAT);
        semctl(sync[i], 0, SETVAL, 0);
    }

    // Starting index for the topological sort array
    *idx = 0;
    
    printf("+++ Boss: Setup done...\n");

    // Wait for the workers to complete the topological sort
    for(int i=0;i<n;i++){
        P(ntfy);
    }

    printf("+++ Topological sorting of the vertices\n");
    for(int i=0;i<n;i++){
        printf("%d\t", T[i]);
    }

    // Check whether topological sort is followed
    int isWrong = 0;
    for(int i=0;i<n;i++){
        for(int j=0;j<i;j++){
            if(A[T[i]*n + T[j]] == 1){      // If there is an edge from T[i] to T[j]; i > j
                printf("There is an edge from %d to %d\n", T[i], T[j]);
                isWrong = 1;
            }
        }
    } 
    if(isWrong){
        printf("+++ Boss: These many precedence constrains are violated...\n");
    }
    else{
        printf("\n+++ Boss: Well done, my team...\n");
    }

    shmdt(A); shmctl(shmidA, IPC_RMID, 0);
    shmdt(T); shmctl(shmidT, IPC_RMID, 0);
    shmdt(idx); shmctl(shmidx, IPC_RMID, 0);
    semctl(mtx, 0, IPC_RMID, 0);
    semctl(ntfy, 0, IPC_RMID, 0);

    return 0;
}