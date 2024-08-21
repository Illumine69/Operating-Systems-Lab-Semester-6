#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX 20

int main(){
    int file_desc = open("dup.txt", O_WRONLY | O_APPEND);
    if(file_desc < 0){
        printf("Error opening the file\n");
    }

    int copy_desc = dup(file_desc);

    write(copy_desc, "This will be the output to the file named dup.txt\n",50);
    write(file_desc, "This will also be the output to the file named dup.txt\n",55);
    return 0;
}