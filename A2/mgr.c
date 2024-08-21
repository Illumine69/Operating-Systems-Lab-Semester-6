/*
Name: Sanskar Mittal
Roll No.: 21CS10057
Semester: 6
Lab Assignment: 2 : IPC using signals
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <signal.h>


int pid[11], pgid[11], status[11];      // stores pid, pgid and status of each job
char job_name[11];                      // stores name of each job
int total_jobs = 1;                     // keeps track of total number of jobs
int is_running = 0;                     // keeps track of whether a job is running or not
int cur_job = 0;                        // keeps track of current job

void sigHandler(int signum){
    if(is_running){
        switch (signum){
            case SIGINT: {
                kill(pid[cur_job],SIGINT);
                status[cur_job] = 3;
                printf("\n");
                break;
            }
            case SIGTSTP: {
                kill(pid[cur_job],SIGTSTP);
                status[cur_job] = 1;
                printf("\n");
                break;
            }
        }
    }
    else{
        printf("\nmgr> ");
        fflush(stdout);
    }
}

int main(){
    pid[0] = getpid();
    pgid[0] = getpgid(pid[0]);
    for(int i=0;i<11;i++){
        status[0] = 0;
    }

    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);

    while(1){
        char command;
        printf("mgr> ");
        scanf(" %c",&command);
        switch (command)  { 
            case 'p':{
                printf("NO\tPID\t\tPGID\t\tSTATUS\t\tNAME\n");    
                for(int i=0;i < total_jobs;i++){
                    char* cur_job_name = (char*)malloc(sizeof(char)*100);
                    char* cur_job_status = (char*)malloc(sizeof(char)*100);

                    if(i == 0){
                        strcpy(cur_job_name,"\tmgr");
                        cur_job_name[4] = '\0';
                    }
                    else{
                        strcpy(cur_job_name,"job ");
                        cur_job_name[4] = job_name[i];
                        cur_job_name[5] = '\0';
                    }

                    if(status[i] == 0){
                        sprintf(cur_job_status,"SELF");
                    }
                    else if(status[i] == 1){
                        sprintf(cur_job_status,"SUSPENDED");
                    }
                    else if(status[i] == 2){
                        sprintf(cur_job_status,"KILLED\t");
                    }
                    else if(status[i] == 3){
                        sprintf(cur_job_status,"TERMINATED");
                    }
                    else{
                        sprintf(cur_job_status,"FINISHED");
                    }

                    printf("%d\t%d\t\t%d\t\t%s\t%s\n",i,pid[i],pgid[i],cur_job_status,cur_job_name);
                    free(cur_job_name);
                    free(cur_job_status);
                }
                break;
            }
            case 'r':{
                is_running = 1;
                cur_job = total_jobs;              // current job is the new job being created

                if(total_jobs < 11){
                    srand((unsigned int)time(NULL));
                    char alphabet = 'A' + rand() % 26;
                    int process_pid = fork();
                    if(process_pid == 0){                       // CHILD PROCESS
                        int c_pid = getpid();
                        setpgid(c_pid,c_pid);
                        char* flag = (char *)malloc(sizeof(char)*2);
                        flag[0] = alphabet;
                        flag[1] = '\0';
                        execlp("./job","./job",flag,NULL);      // run job.c
                    }
                    else{                                       // PARENT PROCESS
                        pid[cur_job] = process_pid;
                        setpgid(process_pid,process_pid);
                        pgid[cur_job] = pid[cur_job];
                        job_name[cur_job] = alphabet;

                        int sig_status;
                        waitpid(pid[cur_job], &sig_status,WUNTRACED);

                        if(WIFSTOPPED(sig_status)){             // if child process is suspended
                            status[cur_job] = 1;           
                        }
                        else if(WIFSIGNALED(sig_status)){       // if child process is terminated
                            status[cur_job] = 3;
                        }
                        else if(WIFEXITED(sig_status)){         // if child process is finished
                            status[cur_job] = 4;
                        }
                    }
                }
                else{
                    printf("Process table is full. Quitting...\n");
                    for(int i=1;i<total_jobs;i++){              // kill all jobs
                        kill(pid[i],SIGINT);
                    }
                    exit(-1);
                }
                total_jobs++;
                is_running = 0;
                break;
            }
            case 'c':{
                int is_suspended = 0;                   // flag to check if there is any suspended job
                for(int i=1;i<total_jobs;i++){
                    if(status[i] == 1){
                        if(is_suspended == 0){
                            printf("Suspended jobs: ");
                            is_suspended = 1;
                            printf(" %d", i);
                        }
                        else{
                            printf(", %d", i);
                        }
                    }
                }
                if(is_suspended){
                    printf(" (Pick one): ");
                    int job_no;
                    scanf(" %d",&job_no);
                    while(status[job_no] != 1){             // loop until a valid job number is entered
                        printf("Enter a valid job number: ");
                        scanf(" %d",&job_no);
                    }
                    cur_job = job_no;
                    is_running = 1;                         // set flag to indicate that a job is running
                    kill(pid[cur_job],SIGCONT);                 
                    
                    int sig_status;
                    waitpid(pid[cur_job], &sig_status,WUNTRACED);
                    if(WIFSTOPPED(sig_status)){
                        status[cur_job] = 1;           
                    }
                    else if(WIFSIGNALED(sig_status)){
                        status[cur_job] = 3;
                    }
                    else if(WIFEXITED(sig_status)){
                        status[cur_job] = 4;
                    }
                    is_running = 0;
                }
                break;
            }
            case 'k':{
                int is_suspended = 0;
                for(int i=1;i<total_jobs;i++){
                    if(status[i] == 1){
                        if(is_suspended == 0){
                            printf("Suspended jobs: ");
                            is_suspended = 1;
                            printf(" %d", i);
                        }
                        else{
                            printf(", %d", i);
                        }
                    }
                }
                if(is_suspended){
                    printf(" (Pick one): ");
                    int job_no;
                    scanf(" %d",&job_no);
                    while(status[job_no] != 1){
                        printf("\nEnter a valid job number: ");
                        scanf(" %d",&job_no);
                    }
                    kill(pid[job_no],SIGINT);
                    status[job_no] = 2;
                }
                break;
            }
            case 'h':{
                printf("Command : Action\n");
                printf("   c    : Continue a suspended job\n");
                printf("   h    : Print this help message\n");
                printf("   k    : Kill a suspended job\n");
                printf("   p    : Print the process table\n");
                printf("   q    : Quit\n");
                printf("   r    : Run a new job\n");
                break;
            }
            case 'q':{
                for(int i=1;i<total_jobs;i++){
                    kill(pid[i],SIGINT);
                }
                exit(0);
                break;
            }
            default: {
                printf("Enter a valid command!\n");
                break;
            }
        }
    }
    return 0;
}