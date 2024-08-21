/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 5 : Mutual exclusion and synchronization using semaphores
File: worker.c
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

int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Usage: %s <n> <i>\n", argv[0]);
        exit(1);
    }
    int n = atoi(argv[1]);
    int id = atoi(argv[2]);

    key_t keyA = ftok("/tmp", START);
    key_t keyT = ftok("/tmp", START+1);
    key_t keyidx = ftok("/tmp", START+2);
    int shmidA = shmget(keyA, n*n*sizeof(int), 0777);
    int shmidT = shmget(keyT, n*sizeof(int), 0777);
    int shmidx = shmget(keyidx, sizeof(int), 0777);

    int *A = (int *)malloc(n*n*sizeof(int));
    int *T = (int *)malloc(n*sizeof(int));
    int *idx = (int *)malloc(sizeof(int));

    A = (int *)shmat(shmidA, 0, 0);
    T = (int *)shmat(shmidT, 0, 0);
    idx = (int *)shmat(shmidx, 0, 0);

    key_t ketmtx = ftok("/tmp", START+3);
    key_t keyntfy = ftok("/tmp", START+4);
    int mtx = semget(ketmtx, 1, 0777);
    int ntfy = semget(keyntfy, 1, 0777);

    struct sembuf pop, vop;
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; vop.sem_op = 1;

    int sync[n];
    for(int i=0;i<n;i++){
        key_t keysync = ftok("/tmp", START+5+i);
        sync[i] = semget(keysync, 1, 0777);
    }

    // Worker id waits for dependent modules to complete
    for(int i=0;i<n;i++){
        if(A[i*n + id] == 1){
            P(sync[id]);
        }
    }

    // Worker id enters the critical section
    P(mtx);
    T[*idx] = id;
    *idx = (*idx + 1);
    V(mtx);
    // Worker id exits the critical section

    // Worker id notifies the dependent modules
    for(int i=0;i<n;i++){
        if(A[id*n + i] == 1){
            V(sync[i]);
        }
    }

    // Worker id notifies the boss
    V(ntfy);
    
    shmdt(A);
    shmdt(T);
    shmdt(idx);

    return 0;
}
