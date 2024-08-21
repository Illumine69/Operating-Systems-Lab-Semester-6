#include <stdio.h>
#include <unistd.h>
#define MAX 20

int main(){
    FILE* fp;
    int status;
    char path[MAX];
    fp = popen("ls -l", "r");
    if(fp == NULL){
        printf("popen error\n");
        return 1;
    }
    while(fgets(path, MAX, fp) != NULL){
        printf("%s", path);
    }
    status = pclose(fp);
}