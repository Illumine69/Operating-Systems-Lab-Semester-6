#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX 20

int main(){
    int fd[2];
    char buf[20];
    pipe(fd);

    int pid = fork();
    if(pid == 0){
        close(1);
        dup(fd[1]);
        printf("hello world\n");
    }
    else{
        read(fd[0], buf, MAX);
        printf("%s\n", buf);
    }
}