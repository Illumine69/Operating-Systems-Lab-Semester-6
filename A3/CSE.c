/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 3 : IPC using pipe and dup
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 100

int main(int argc, char* argv[]){

    if(argc > 1){
        int ifd1 = atoi(argv[2]);
        int ofd1 = atoi(argv[3]);
        int ifd2 = atoi(argv[4]);
        int ofd2 = atoi(argv[5]);
        if(strcmp(argv[1],"C") == 0){     // Command-input child (C)

           // closing unused file descriptors
           close(ifd1);                   
           close(ofd2);
           
           // storing for future use
           int std_in = dup(STDIN_FILENO);
           int std_out = dup(STDOUT_FILENO);

           int swaprole = 0;
           char buff[MAX];
           while(1){
                while(swaprole == 0){                   // currently in the command mode
                    dup2(ofd1, STDOUT_FILENO);          // output to be piped to second process
                    dup2(std_in, STDIN_FILENO);         // input is the standard input

                    write(std_out, "Enter command> ", 14); 
                    memset(buff, '\0', MAX);
                    fgets(buff, MAX, stdin);        

                    printf("%s", buff);                 // send the buff via pipe
                    fflush(stdout);

                    if(strcmp(buff,"exit\n")==0){
                        exit(0);
                    }

                    if(strcmp(buff, "swaprole\n")==0){
                        swaprole = 1;
                    }
                }

                // Execute mode entered
                dup2(ifd2, STDIN_FILENO);               // pipe input
                dup2(std_out, STDOUT_FILENO);           // output is the standard output

                printf("Waiting for command> ");
                fflush(stdout);

                memset(buff, '\0', MAX);
                fgets(buff, MAX, stdin);

                printf("%s", buff);
                fflush(stdout);

                if(strcmp(buff, "exit\n")==0){
                    exit(0);
                }

                if(strcmp(buff, "swaprole\n")==0){
                    swaprole = 0;
                    continue;
                }

                int pid = fork();

                if(pid == 0){                   // Grand child prcess
                    // set the command execution output and input to be in parent process
                    dup2(std_in, STDIN_FILENO);
                    dup2(std_out, STDOUT_FILENO);

                    char* command[MAX];
                    char* word = strtok(buff, " ");
                    int i=0;
                    while(word != NULL){
                        command[i] = word;
                        word = strtok(NULL," ");
                        i++;
                    }
                    command[i-1][strlen(command[i-1])-1] = '\0';        // last character is newline which must be removed
                    command[i]=NULL;

                    if(execvp(command[0],command)==-1){
                        perror("execvp failed");
                        exit(EXIT_FAILURE);
                    }
                }
                else{
                    waitpid(pid, NULL, WUNTRACED);
                    fflush(stdout);

                    if(strcmp(buff, "swaprole\n")==0){
                        swaprole = 0;
                    }
                }
           }
        }
        else{                   // Execute-command child (E)

            // closing unused file descriptors
            close(ofd1);
            close(ifd2);

            // storing for future use
            int std_in = dup(STDIN_FILENO);
            int std_out = dup(STDOUT_FILENO);

            char buff[MAX];
            int swaprole = 1;
            while(1){

                while(swaprole == 1){                       // currently in the execute mode
                    dup2(ifd1, STDIN_FILENO);               // pipe input
                    dup2(std_out, STDOUT_FILENO);           // output is the standard output

                    printf("Waiting for command> ");
                    fflush(stdout);

                    memset(buff, '\0', MAX);
                    fgets(buff, MAX, stdin);

                    printf("%s", buff);
                    fflush(stdout);

                    if(strcmp(buff, "exit\n")==0){
                        exit(0);
                    }
                    if(strcmp(buff, "swaprole\n")==0){
                        swaprole = 0;
                        break;
                    }

                    int pid = fork();

                    if(pid == 0){                           // Grand child process
                        // set the command execution output and input to be in parent process
                        dup2(std_in, STDIN_FILENO);
                        dup2(std_out, STDOUT_FILENO);

                        char* command[MAX];
                        char* word = strtok(buff, " ");
                        int i=0;
                        while(word != NULL){
                            command[i] = word;
                            word = strtok(NULL," ");
                            i++;
                        }
                        
                        command[i-1][strlen(command[i-1])-1] = '\0';        // last character is newline which must be removed
                        command[i]=NULL;

                        if(execvp(command[0],command)==-1){
                            perror("execvp failed");
                            exit(EXIT_FAILURE);
                        }
                    }
                    else{
                        waitpid(pid, NULL, WUNTRACED);
                        fflush(stdout);
                    }
                }
                
                // command mode entered
                dup2(ofd2, STDOUT_FILENO);          // output to be piped to first process
                dup2(std_in, STDIN_FILENO);         // input is the standard input

                write(std_out, "Enter command> ", 14);
                memset(buff, '\0', MAX);
                fgets(buff, MAX, stdin);

                printf("%s", buff);                 // send the buff via pipe
                fflush(stdout);

                if(strcmp(buff, "exit\n")==0){
                    exit(0);
                }
                if(strcmp(buff, "swaprole\n")==0){
                    swaprole = 1;
                }
            }
        }
    }
    else{
        int fd1[2],fd2[2];  //Pipes
        pipe(fd1);
        pipe(fd2);
        printf("+++ CSE in supervisor mode: Started\n");
        printf("+++ CSE in supervisor mode: pfd = [%d %d]\n",fd1[0], fd1[1]);
        int pid1,pid2;
        printf("+++ CSE in supervisor mode: Forking first child in command-input mode\n");
        pid1 = fork();
        if(pid1 == 0){          // First Child Process
            char* args[9];
            args[0] = (char*)malloc(6*sizeof(char)); strcpy(args[0],"xterm");
            args[1] = (char*)malloc(3*sizeof(char));strcpy(args[1],"-T");
            args[2] = (char*)malloc(12*sizeof(char));strcpy(args[2],"First Child");
            args[3] = (char*)malloc(3*sizeof(char));strcpy(args[3],"-e");
            args[4] = (char*)malloc(6*sizeof(char));strcpy(args[4],"./CSE");
            args[5] = (char*)malloc(2*sizeof(char));strcpy(args[5],"C");
            args[6] = (char*)malloc(4*sizeof(char));sprintf(args[6],"%d",fd1[0]);
            args[7] = (char*)malloc(4*sizeof(char));sprintf(args[7],"%d",fd1[1]);
            args[8] = (char*)malloc(4*sizeof(char));sprintf(args[8],"%d",fd2[0]);
            args[9] = (char*)malloc(4*sizeof(char));sprintf(args[9],"%d",fd2[1]);
            args[10] = NULL;
            execvp(args[0],args);
        }
        else{   
            printf("+++ CSE in supervisor mode: Forking second child in execute mode\n");
            pid2 = fork();
            if(pid2 == 0){      // Second Child Process
                char* args[9];
                args[0] = (char*)malloc(6*sizeof(char)); strcpy(args[0],"xterm");
                args[1] = (char*)malloc(3*sizeof(char));strcpy(args[1],"-T");
                args[2] = (char*)malloc(13*sizeof(char));strcpy(args[2],"Second Child");
                args[3] = (char*)malloc(3*sizeof(char));strcpy(args[3],"-e");
                args[4] = (char*)malloc(6*sizeof(char));strcpy(args[4],"./CSE");
                args[5] = (char*)malloc(2*sizeof(char));strcpy(args[5],"E");
                args[6] = (char*)malloc(4*sizeof(char));sprintf(args[6],"%d",fd1[0]);
                args[7] = (char*)malloc(4*sizeof(char));sprintf(args[7],"%d",fd1[1]);
                args[8] = (char*)malloc(4*sizeof(char));sprintf(args[8],"%d",fd2[0]);
                args[9] = (char*)malloc(4*sizeof(char));sprintf(args[9],"%d",fd2[1]);
                args[10] = NULL;
                execvp(args[0],args);
            }
            else{               // Parent Process

                close(fd1[0]);
                close(fd1[1]);
                close(fd2[0]);
                close(fd2[1]);
                waitpid(pid1,NULL,WUNTRACED);
                waitpid(pid2,NULL,WUNTRACED);

                printf("+++ CSE in supervisor mode: First child terminated\n");
                printf("+++ CSE in supervisor mode: Second child terminated\n");
            }
        }
    }
    return 0;
}