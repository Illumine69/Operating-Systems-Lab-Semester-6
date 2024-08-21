/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 7 : Design your own thread library
File: foothread.c
*/

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <foothread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int total_active_threads = 0;

struct sembuf pop = {0, -1, 0}, vop = {0, 1, 0};

typedef struct{
    int t_id;
    int join_type;
    int stack_size;
    int semid;
}_thread_table;

// Initialising the thread table
_thread_table thread_table[FOOTHREAD_THREADS_MAX] = {[0 ... (FOOTHREAD_THREADS_MAX-1)] = {-1, -1, -1}};

void foothread_attr_setjointype ( foothread_attr_t *t_attr , int type){
    t_attr->join_type = type;
}

void foothread_attr_setstacksize ( foothread_attr_t *t_attr , int size){
    t_attr->stack_size = size;
}

void foothread_create(foothread_t *t_id, foothread_attr_t *t_attr, int (*fn)(void *), void *arg) {

    if(total_active_threads >= FOOTHREAD_THREADS_MAX){
        printf("Maximum threads limit reached\n");
        exit(0);
    }

    if(t_attr == NULL){
        t_attr = (foothread_attr_t*)malloc(sizeof(foothread_attr_t));
        *t_attr = FOOTHREAD_ATTR_INITIALIZER;
    }

    t_id->stack = malloc(t_attr->stack_size);

    int semid;
    if(t_attr->join_type == FOOTHREAD_JOINABLE){
        semid = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
        semctl(semid, 0, SETVAL, 0);
    }

    t_id->id = clone(fn, t_id->stack + t_attr->stack_size, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT | CLONE_THREAD, arg);
    
    total_active_threads++;
    int i = t_id->id%FOOTHREAD_THREADS_MAX;     // Hashing the thread id to get the index in the thread table
    thread_table[i].t_id = t_id->id;
    thread_table[i].join_type = t_attr->join_type;
    thread_table[i].stack_size = t_attr->stack_size;
    if(t_attr->join_type == FOOTHREAD_JOINABLE){
        thread_table[i].semid = semid;
    }
}

void foothread_exit(){
    int t_id = gettid();
    int p_id = getpid();
    
    if(t_id == p_id){   // Leader Thread
        for(int i=0; i<FOOTHREAD_THREADS_MAX; i++){
            if(thread_table[i].t_id != -1 && thread_table[i].join_type == FOOTHREAD_JOINABLE){
                P(thread_table[i].semid);       // Waiting for all joinable threads to exit
            }
        }
        // Cleaning the thread table
        for(int i=0;i<FOOTHREAD_THREADS_MAX;i++){
            if(thread_table[i].t_id != -1){
                if(thread_table[i].join_type == FOOTHREAD_JOINABLE){
                    semctl(thread_table[i].semid, 0, IPC_RMID, 0);
                }
                thread_table[i].t_id = -1;
                thread_table[i].join_type = -1;
                thread_table[i].stack_size = -1;
            }
        }
        exit(0);
    }
    else{   // Follower Thread
        for(int i=0; i<FOOTHREAD_THREADS_MAX; i++){
            if(thread_table[i].t_id == t_id){
                if(thread_table[i].join_type == FOOTHREAD_JOINABLE){
                    V(thread_table[i].semid);    // Signalling the leader thread that this thread has exited
                }
                break;
            }
        }
    }
}

void foothread_mutex_init ( foothread_mutex_t * m){

    int semid = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    semctl(semid, 0, SETVAL, 1);
    m->semid = semid;
    m->state = UNLOCKED;
    m->locking_thread = -1;
    
}

void foothread_mutex_lock ( foothread_mutex_t * m ){
    pid_t t_id = gettid();
    P(m->semid);
    m->state = LOCKED;
    m->locking_thread = t_id;
}   

void foothread_mutex_unlock ( foothread_mutex_t * m){
    pid_t t_id = gettid();
    if(m->locking_thread != t_id){
        printf("Thread %d is not the owner of the lock\n", t_id);
        exit(0);
    }
    if(m->state == UNLOCKED){
        printf("Mutex is already unlocked\n");
        exit(0);
    }
    m->state = UNLOCKED;
    m->locking_thread = -1;
    V(m->semid);
}

void foothread_mutex_destroy ( foothread_mutex_t * m){
    if(m->state == LOCKED){
        printf("Mutex is locked, cannot destroy. Unlock the mutex first\n");
        exit(0);
    }
    semctl(m->semid, 0, IPC_RMID, 0);
}

void foothread_barrier_init ( foothread_barrier_t * b_id, int count){
    int semid = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    semctl(semid, 0, SETVAL, 0);
    b_id->semid = semid;
    b_id->count = count;
    b_id->waiting = 0;
    b_id->count_sem = semget(IPC_PRIVATE, 1, 0777|IPC_CREAT);
    semctl(b_id->count_sem, 0, SETVAL, 1);
}

void foothread_barrier_wait ( foothread_barrier_t * b_id){
    P(b_id->count_sem);                         // Locking the count semaphore to ensure that waiting is updated atomically
    b_id->waiting++;
    if(b_id->waiting < b_id->count){            // If all threads have not reached the barrier
        V(b_id->count_sem);
        P(b_id->semid);
    }
    else{                                       // Final thread has reached the barrier
        V(b_id->count_sem);
        for(int i=1; i<b_id->count; i++){       // Signalling all the waiting threads
            V(b_id->semid);
        }
        b_id->waiting = 0;
    }
}

void foothread_barrier_destroy ( foothread_barrier_t * b_id){
    semctl(b_id->semid, 0, IPC_RMID, 0);
    semctl(b_id->count_sem, 0, IPC_RMID, 0);
}



