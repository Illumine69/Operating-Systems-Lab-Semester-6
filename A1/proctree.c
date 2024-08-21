/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 1 : Introduction to multi-process applications
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>

#define MAX 1024

int main(int argc, char* argv[]){

    int indent = 0;                                 // stores indentation level
    if(argc <= 1){                                  // if no node name is given
        printf("Run with a node name\n");
        exit(-1);
    }
    else if(argc >= 3){                             // if indentation level is given
        indent = atoi(argv[2]);
    }
    
    FILE* fp = fopen("treeinfo.txt","r");
    char temp[MAX];
    char* city;

    while(fgets(temp, MAX, fp)){                    // stores the line in temp
        city = strtok(temp, " ");                   // get first word in temp
        if(strcmp(city, argv[1]) == 0){             // if word is same as node name, line is found
            break;
        }
    }
    if(strcmp(city, argv[1]) != 0){                 // if city is not found, send error
        printf("City %s not found\n",argv[1]);
        return 0;
    }

    for(int i = 0; i < indent;i++){                 // print indentation
        printf("\t");
    }

    pid_t pid = getpid();
    printf("%s (%d)\n",city,pid);                   // print city name and pid
    
    char* child = strtok(NULL, " ");
    int i = 0;
    pid_t child_pid;
    while(child != NULL){
        if(i++ < 1) {                               // ignore first word as it is the number of childs
            child = strtok(NULL, " ");
            continue;
        }

        child_pid = fork();
        if(child_pid == 0){                         // if child process
            char* args[4];
            args[0] = (char*)malloc(11*sizeof(char)); strcpy(args[0],"./proctree");
            if(child[strlen(child)-1] == '\n'){     // ensure that name does not have a newline
                child[strlen(child)-1] = '\0';
            }
            args[1] = (char*)malloc(sizeof(child)); strcpy(args[1],child);
            args[2] = (char*)malloc(4*sizeof(char)); sprintf(args[2],"%d",indent+1);        // store the next level of indentation
            args[3] = NULL;
            execvp(args[0], args);                  // $ ./proctree <child> <indentation>
        }
        else{                                       // if parent process
            waitpid(child_pid, NULL, 0);    
        }
        child = strtok(NULL, " ");                  // get next child
    }
    fclose(fp);
}