/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 7 : Design your own thread library
File: computesum.c
*/

#include <fcntl.h>
#include <foothread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX FOOTHREAD_THREADS_MAX
int P[MAX], SUM[MAX], isParent[MAX], barrierSet[MAX];
int root, n;
foothread_barrier_t barrier[MAX], initLeaf, sumComplete;
foothread_mutex_t getLeaf, printSum;

int computeSum(void *arg) {
    int i = (int)arg;
    if (!isParent[i]) {
        foothread_mutex_lock(&getLeaf);
        printf("Leaf node %2d :: Enter a positive integer: ", i);
        scanf(" %d", &SUM[i]);
        foothread_mutex_unlock(&getLeaf);
        SUM[P[i]] += SUM[i];
        foothread_barrier_wait(&initLeaf);      // Wait till all leafs have entered their values
        foothread_barrier_wait(&barrier[P[i]]);     // Wait till all children have entered their values
    } else {
        foothread_barrier_wait(&initLeaf);
        foothread_barrier_wait(&barrier[i]);
        foothread_mutex_lock(&printSum);
        printf("Internal node %2d gets the partial sum %d from its children\n", i, SUM[i]);
        foothread_mutex_unlock(&printSum);
        if (i != root) {
            SUM[P[i]] += SUM[i];
            foothread_barrier_wait(&barrier[P[i]]);
        }
    }
    if (i == root) {
        printf("Sum at root (node %2d) = %d\n", root, SUM[root]);
    }
    foothread_barrier_wait(&sumComplete);
    foothread_exit();
    return 0;
}

int main() {
    fflush(stdout);
    FILE *fd = fopen("tree.txt", "r");
    char *line = malloc(100 * sizeof(char));
    int i = 0, leafCount = 0;
    memset(isParent, 0, sizeof(isParent));
    memset(barrierSet, 0, sizeof(barrier));
    while (fgets(line, 100, fd) != NULL) {
        if (i == 0) {
            n = atoi(line);
            if (n > MAX) {
                printf("Number of nodes exceeds the limit\n");
                return 0;
            }
        } else {
            int *arr = (int *)malloc(2 * sizeof(int));
            char *token = strtok(line, " ");
            int j = 0;
            while (token != NULL) {
                arr[j] = atoi(token);
                j++;
                token = strtok(NULL, " ");
            }
            P[arr[0]] = arr[1];
            if (arr[0] == arr[1]) {
                root = arr[0];
            } else {
                isParent[arr[1]]++;
            }
        }
        i++;
    }
    for (int i = 0; i < n; i++) {
        if (!isParent[i]) {
            leafCount++;
        }
    }

    foothread_barrier_init(&initLeaf, n);
    foothread_barrier_init(&sumComplete, n + 1);
    foothread_t threads[n];
    foothread_attr_t attr[n];
    foothread_mutex_init(&getLeaf);
    foothread_mutex_init(&printSum);

    for (int i = 0; i < n; i++) {
        if (barrierSet[P[i]] == 0) {        // Barrier not set for parent
            foothread_barrier_init(&barrier[P[i]], isParent[P[i]] + 1);
            barrierSet[P[i]] = 1;
        }
        if (isParent[i] && barrierSet[i] == 0) {        // Barrier not set for internal node
            foothread_barrier_init(&barrier[i], isParent[i] + 1);
            barrierSet[i] = 1;
        }
        foothread_attr_setjointype(&attr[i], FOOTHREAD_JOINABLE);
        foothread_attr_setstacksize(&attr[i], 2097152);
        foothread_create(&threads[i], &attr[i], computeSum, (void *)i);
    }

    foothread_barrier_wait(&sumComplete);       // Wait till all threads have completed to destroy the barriers and mutexes
    for (int i = 0; i < n; i++) {
        if (barrierSet[i]) {
            foothread_barrier_destroy(&barrier[i]);
        }
    }
    foothread_barrier_destroy(&initLeaf);
    foothread_barrier_destroy(&sumComplete);
    foothread_mutex_destroy(&getLeaf);
    foothread_mutex_destroy(&printSum);
    foothread_exit();
}