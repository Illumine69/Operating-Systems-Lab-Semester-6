/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 4 : Shared Memory without Synchronization
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

int main(){
    int n,t;
    printf("n = ");
    scanf(" %d",&n);

    printf("t = ");
    scanf("%d",&t);

    int shmid = shmget(IPC_PRIVATE,2*sizeof(int),0777|IPC_CREAT);
    int *m = (int *) shmat(shmid,0,0);
    m[0] = 0;
    m[1] = 0;

    srand((unsigned int)time(NULL));
    for(int i=0;i<n;i++){
        if(fork() == 0){    // Child Process
            int c = i+1;        // child number
            printf("\t\t\t\t\t\tConsumer %d is alive\n",c);
            int item_total = 0;
            int item_count = 0;

            while(1){
                if(m[0] == -1){
                    break;
                }
                if(m[0] == c){
                    item_count++;
                    item_total += m[1];
                    #ifdef VERBOSE
                    printf("\t\t\t\t\t\tConsumer %d reads %d\n",c,m[1]);
                    #endif
                    m[0] = 0;
                }
            }

            printf("\t\t\t\t\t\tConsumer %d has read %d items: Checksum = %d\n", c, item_count, item_total);

            shmdt(m);
            exit(0);
        }
    }

    // Parent Process
    printf("Producer is alive\n");
    int item, c;
    int consumer_items[n+1][2];     // first column is count, second is sum
    memset(consumer_items,0,sizeof(consumer_items));

    
    for(int i=0;i < t;i++){
        item = 100 + rand()%900;
        c = 1 + rand()%n;
        while(m[0] != 0);
        m[0] = c;

        // sleep for random time between 1 and 10 microseconds
        #ifdef SLEEP
        int time = 1 + rand()%10;
        usleep(time);
        #endif

        m[1] = item;
        consumer_items[c][0]++;
        consumer_items[c][1] += item;

        #ifdef VERBOSE
        printf("Producer produces %d for consumer %d\n",item,c);
        #endif

    }

    while(m[0] != 0);
    m[0] = -1;

    for(int i=0;i<n;i++){
        wait(NULL);
    }
    printf("Producer has produced %d items\n",t);
    for(int i=1;i<=n;i++){
        printf("%d items for Consumer %d: Checksum = %d\n", consumer_items[i][0],i,consumer_items[i][1]);
    }

    shmdt(m);
    shmctl(shmid,IPC_RMID,0);

}