#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX 20

int main(){
    int old,new;
    old = open("input.txt", O_WRONLY | O_CREAT);
    close(1);
    new = dup(old);
    printf("hello world\n");
}