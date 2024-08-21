#include <stdio.h>
#include <unistd.h>
#define MAX 20

main(){
    char* msg="hello";
    char buff[MAX];
    int p[2];
    pipe(p);
    int pid = fork();
    if(pid==0){
        printf("child exiting\n");
    }
    else{
        sleep(1);
        close(p[0]);
        printf("%d", write(p[1], msg, MAX));
    }
}