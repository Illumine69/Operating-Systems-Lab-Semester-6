/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 7 : Design your own thread library
File: foothread.h
*/

#ifndef FOOTHREAD_H
#define FOOTHREAD_H

#define FOOTHREAD_THREADS_MAX 100
#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152
#define FOOTHREAD_JOINABLE 0
#define FOOTHREAD_DETACHED 1
#define FOOTHREAD_ATTR_INITIALIZER (foothread_attr_t){ FOOTHREAD_DEFAULT_STACK_SIZE, FOOTHREAD_DETACHED}
#define LOCKED 1
#define UNLOCKED 0
#define P(s) semop(s, &pop, 1) /* pop is the structure we pass for doing the P(s) operation */
#define V(s) semop(s, &vop, 1) /* vop is the structure we pass for doing the V(s) operation */

typedef struct{
    int id;
    void* stack;
} foothread_t;

typedef struct{
    int stack_size;
    int join_type;
} foothread_attr_t;

typedef struct{
    int semid;      // semaphore id
    int state;      // 0 for unlocked, 1 for locked
    int locking_thread;    // id of the thread that has locked the mutex
} foothread_mutex_t;

typedef struct{
    int semid;      // semaphore id
    int count;      // number of threads that have reached the barrier
    int waiting;    // number of threads that are waiting at the barrier
    int count_sem;  // semaphore for count
} foothread_barrier_t;

void foothread_create(foothread_t *, foothread_attr_t *, int (*)(void *), void *);
void foothread_attr_setjointype ( foothread_attr_t * , int );
void foothread_attr_setstacksize ( foothread_attr_t * , int );
void foothread_exit ();

void foothread_mutex_init ( foothread_mutex_t * );
void foothread_mutex_lock ( foothread_mutex_t * );
void foothread_mutex_unlock ( foothread_mutex_t * );
void foothread_mutex_destroy ( foothread_mutex_t * );

void foothread_barrier_init ( foothread_barrier_t * , int );
void foothread_barrier_wait ( foothread_barrier_t * );
void foothread_barrier_destroy ( foothread_barrier_t * );

#endif