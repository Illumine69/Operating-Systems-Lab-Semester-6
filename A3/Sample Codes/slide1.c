#include <stdio.h>
#include <unistd.h>

#define MAX 20

int main(){
    int p[2];
    char* msg = "hello";
    char buff[MAX];
    pipe(p);
    int pid = fork();
    if(pid > 0){
        write(p[1], msg, MAX);
    }
    else{
        sleep(1);
        write(p[1], msg, MAX);
        for(int i=0;i<2;i++){
            read(p[0], buff, MAX);
            printf("%s\n", buff);
        }
    }
}