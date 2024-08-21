/*
Name: Sanskar Mittal
Roll Number: 21CS10057
Semester: 6
Lab Assignment: 6 : Mutithreaded applications using pthread
File: session.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <event.h>
#include <pthread.h>
#include <time.h>
#define MAX 50      // Maximum number of reporters

int curTime = 0;
int doctorBusyTime = 0;
int doctorLeft = 0;
int newVisitor = 0;
int leaveDoctor = 0;
int reporterSignal = 0;
int patientSignal = 0;
int salesrepSignal = 0;
pthread_mutex_t mtx_counts;
pthread_cond_t cond_counts;
pthread_mutex_t mtx_doctor;
pthread_cond_t cond_doctor;
pthread_mutex_t mtx_reporter;
pthread_cond_t cond_reporter;
pthread_cond_t cond_reporter_2;
pthread_mutex_t mtx_patient;
pthread_cond_t cond_patient;
pthread_cond_t cond_patient_2;
pthread_mutex_t mtx_salesrep;
pthread_cond_t cond_salesrep;
pthread_cond_t cond_salesrep_2;
int last_token_no_patient = 0;
int last_token_no_salesrep = 0;
int last_token_no_reporter = 0;
int no_of_waiting_patient = 0;
int no_of_waiting_salesrep = 0;
int no_of_waiting_reporter = 0;

int patient_duration[MAX], salesrep_duration[MAX], reporter_duration[MAX];

// Calculate the time in 12-hour format
void calculate_time(int time, int* cur_hour, int* cur_minute, char* am_pm){
    int hour = (((9 + time / 60) % 24) + 24) % 24;
    int minute = time % 60;
    *cur_hour = hour, *cur_minute = minute;
    *am_pm = 'a';
    if(hour <= 12){
        if(minute < 0){
            *cur_hour = hour - 1;
            *cur_minute = 60 + minute;
        }
        if(hour == 12){
            *am_pm = 'p';
        }
    }
    else{
        *am_pm = 'p';
        if(minute < 0){
            *cur_hour = hour - 1;
            *cur_minute = 60 + minute;
        }
        if(*cur_hour != 12){
            *cur_hour = *cur_hour - 12;
        }
    }
}

void* doctor(void* param){
    int cur_hour, cur_minute;
    char am_pm;
    while(1){
        pthread_mutex_lock(&mtx_doctor);
        while(newVisitor == 0 && leaveDoctor == 0){
            pthread_cond_wait(&cond_doctor, &mtx_doctor);
        }
        calculate_time(curTime, &cur_hour, &cur_minute, &am_pm);
        if(leaveDoctor == 1){
            printf("[%02d:%02d%cm] Doctor leaves\n", cur_hour, cur_minute, am_pm);
            doctorLeft = 1;
            pthread_cond_signal(&cond_doctor);
            pthread_mutex_unlock(&mtx_doctor);
            break;
        }
        printf("[%02d:%02d%cm] Doctor has next visitor\n", cur_hour, cur_minute, am_pm);
        newVisitor--;
        pthread_cond_signal(&cond_doctor);
        pthread_mutex_unlock(&mtx_doctor);
    }
    pthread_exit(0);
}

void* reporter(void* param){
    int cur_hour1, cur_minute1, cur_hour2, cur_minute2;
    char am_pm1, am_pm2;
    pthread_mutex_lock(&mtx_reporter);
    while(reporterSignal == 0){
        pthread_cond_wait(&cond_reporter, &mtx_reporter);
    }
    reporterSignal = 0;
    int reporter_num = last_token_no_reporter - no_of_waiting_reporter;
    calculate_time(curTime, &cur_hour1, &cur_minute1, &am_pm1);
    calculate_time(curTime + reporter_duration[reporter_num], &cur_hour2, &cur_minute2, &am_pm2);
    printf("[%02d:%02d%cm - %02d:%02d%cm] Reporter %d is in doctor's chamber\n", cur_hour1, cur_minute1, am_pm1, cur_hour2, cur_minute2, am_pm2, reporter_num);
    pthread_cond_signal(&cond_reporter_2);
    pthread_mutex_unlock(&mtx_reporter);
    pthread_exit(0);
}

void* patient(void* param){
    int cur_hour1, cur_minute1, cur_hour2, cur_minute2;
    char am_pm1, am_pm2;
    pthread_mutex_lock(&mtx_patient);
    while(patientSignal == 0){
        pthread_cond_wait(&cond_patient, &mtx_patient);
    }
    patientSignal = 0;
    int patient_num = (last_token_no_patient > 25 ? 25 : last_token_no_patient) - no_of_waiting_patient;
    calculate_time(curTime, &cur_hour1, &cur_minute1, &am_pm1);
    calculate_time(curTime + patient_duration[patient_num], &cur_hour2, &cur_minute2, &am_pm2);
    printf("[%02d:%02d%cm - %02d:%02d%cm] Patient %d is in doctor's chamber\n", cur_hour1, cur_minute1, am_pm1, cur_hour2, cur_minute2, am_pm2, patient_num);
    pthread_cond_signal(&cond_patient_2);
    pthread_mutex_unlock(&mtx_patient);
    pthread_exit(0);
}

void* salesrep(void* param){
    int cur_hour1, cur_minute1, cur_hour2, cur_minute2;
    char am_pm1, am_pm2;
    pthread_mutex_lock(&mtx_salesrep);
    while(salesrepSignal == 0){
        pthread_cond_wait(&cond_salesrep, &mtx_salesrep);
    }
    salesrepSignal = 0;
    int salesrep_num = (last_token_no_salesrep > 3 ? 3 : last_token_no_salesrep) - no_of_waiting_salesrep;
    calculate_time(curTime, &cur_hour1, &cur_minute1, &am_pm1);
    calculate_time(curTime + salesrep_duration[salesrep_num], &cur_hour2, &cur_minute2, &am_pm2);
    printf("[%02d:%02d%cm - %02d:%02d%cm] Sales representative %d is in doctor's chamber\n", cur_hour1, cur_minute1, am_pm1, cur_hour2, cur_minute2, am_pm2, salesrep_num);
    pthread_cond_signal(&cond_salesrep_2);
    pthread_mutex_unlock(&mtx_salesrep);
    pthread_exit(0);
}

int main(){
    char* filename = "arrival.txt";
    eventQ E = initEQ(filename);

    pthread_mutex_init(&mtx_counts, NULL);
    pthread_cond_init(&cond_counts, NULL);
    pthread_mutex_init(&mtx_doctor, NULL);
    pthread_cond_init(&cond_doctor, NULL);
    pthread_mutex_init(&mtx_reporter, NULL);
    pthread_cond_init(&cond_reporter, NULL);
    pthread_cond_init(&cond_reporter_2, NULL);
    pthread_mutex_init(&mtx_patient, NULL);
    pthread_cond_init(&cond_patient, NULL);
    pthread_cond_init(&cond_patient_2, NULL);
    pthread_mutex_init(&mtx_salesrep, NULL);
    pthread_cond_init(&cond_salesrep, NULL);
    pthread_cond_init(&cond_salesrep_2, NULL);

    pthread_t doctor_t;
    pthread_attr_t doctor_attr;
    pthread_attr_init(&doctor_attr);
    pthread_create(&doctor_t, &doctor_attr, doctor, NULL);

    event e_doctor = (event) {'D',0, 0};        // Doctor starts at time 0
    E = addevent(E, e_doctor);

    while(emptyQ(E) == 0){
        event e = nextevent(E);
        E = delevent(E);
        curTime = e.time;
        int cur_hour, cur_minute;
        char am_pm;
        calculate_time(curTime, &cur_hour, &cur_minute, &am_pm);
        if(e.type == 'D'){
            pthread_mutex_lock(&mtx_doctor);

            if(last_token_no_patient >= 25 && last_token_no_salesrep >= 3 && no_of_waiting_patient == 0 && no_of_waiting_salesrep == 0 && no_of_waiting_reporter == 0){     // Leave condition for doctor
                leaveDoctor = 1;
                pthread_cond_signal(&cond_doctor);
                pthread_mutex_unlock(&mtx_doctor);

                pthread_mutex_lock(&mtx_doctor);
                while(doctorLeft == 0){
                    pthread_cond_wait(&cond_doctor, &mtx_doctor);
                }
                pthread_mutex_unlock(&mtx_doctor);
            }
            else{
                if(no_of_waiting_reporter > 0){
                    no_of_waiting_reporter--;
                    newVisitor++;
                    pthread_cond_signal(&cond_doctor);
                    pthread_mutex_unlock(&mtx_doctor);

                    pthread_mutex_lock(&mtx_doctor);
                    while(newVisitor > 0){
                        pthread_cond_wait(&cond_doctor, &mtx_doctor);
                    }
                    pthread_mutex_unlock(&mtx_doctor);

                    event e_doctor = {'D',curTime + reporter_duration[last_token_no_reporter - no_of_waiting_reporter], 0};
                    E = addevent(E, e_doctor);

                    pthread_mutex_lock(&mtx_reporter);
                    reporterSignal = 1;
                    pthread_cond_signal(&cond_reporter);
                    pthread_mutex_unlock(&mtx_reporter);

                    pthread_mutex_lock(&mtx_reporter);
                    while(reporterSignal == 1){
                        pthread_cond_wait(&cond_reporter_2, &mtx_reporter);
                    }
                    doctorBusyTime = curTime + reporter_duration[last_token_no_reporter - no_of_waiting_reporter];
                    pthread_mutex_unlock(&mtx_reporter);
                }
                else if(no_of_waiting_patient > 0){
                    no_of_waiting_patient--;
                    newVisitor++;
                    pthread_cond_signal(&cond_doctor);
                    pthread_mutex_unlock(&mtx_doctor);

                    pthread_mutex_lock(&mtx_doctor);
                    while(newVisitor > 0){
                        pthread_cond_wait(&cond_doctor, &mtx_doctor);
                    }
                    pthread_mutex_unlock(&mtx_doctor);

                    int patient_num = (last_token_no_patient > 25 ? 25 : last_token_no_patient) - no_of_waiting_patient;
                    event e_doctor = {'D',curTime + patient_duration[patient_num], 0};
                    E = addevent(E, e_doctor);

                    pthread_mutex_lock(&mtx_patient);
                    patientSignal = 1;
                    pthread_cond_signal(&cond_patient);
                    pthread_mutex_unlock(&mtx_patient);

                    pthread_mutex_lock(&mtx_patient);
                    while(patientSignal == 1){
                        pthread_cond_wait(&cond_patient_2, &mtx_patient);
                    }
                    doctorBusyTime = curTime + patient_duration[last_token_no_patient - no_of_waiting_patient];
                    pthread_mutex_unlock(&mtx_patient);
                }
                else if(no_of_waiting_salesrep > 0){
                    no_of_waiting_salesrep--;
                    newVisitor++;
                    pthread_cond_signal(&cond_doctor);
                    pthread_mutex_unlock(&mtx_doctor);

                    pthread_mutex_lock(&mtx_doctor);
                    while(newVisitor > 0){
                        pthread_cond_wait(&cond_doctor, &mtx_doctor);
                    }
                    pthread_mutex_unlock(&mtx_doctor);
                    int salesrep_num = (last_token_no_salesrep > 3 ? 3 : last_token_no_salesrep) - no_of_waiting_salesrep;
                    event e_doctor = {'D',curTime + salesrep_duration[salesrep_num],0};
                    E = addevent(E, e_doctor);

                    pthread_mutex_lock(&mtx_salesrep);
                    salesrepSignal = 1;
                    pthread_cond_signal(&cond_salesrep);
                    pthread_mutex_unlock(&mtx_salesrep);

                    pthread_mutex_lock(&mtx_salesrep);
                    while(salesrepSignal == 1){
                        pthread_cond_wait(&cond_salesrep_2, &mtx_salesrep);
                    }
                    doctorBusyTime = curTime + salesrep_duration[last_token_no_salesrep - no_of_waiting_salesrep];
                    pthread_mutex_unlock(&mtx_salesrep);
                }
                else{
                    // no one scheduled doctor
                    pthread_mutex_unlock(&mtx_doctor);
                }
            }
        }
        else if(e.type == 'R'){
            last_token_no_reporter++;
            reporter_duration[last_token_no_reporter] = e.duration;
            
            printf("\t[%02d:%02d%cm] Reporter %d arrives\n",cur_hour, cur_minute, am_pm, last_token_no_reporter);
            if(leaveDoctor == 0){
                if(no_of_waiting_patient == 0 && no_of_waiting_reporter == 0 && no_of_waiting_salesrep == 0){
                    if(curTime >= doctorBusyTime){      // If doctor is free, schedule the reporter
                        event e_doctor = {'D',curTime,0};
                        E = addevent(E, e_doctor);
                    }
                }
                no_of_waiting_reporter++;
                pthread_t reporter_t;
                pthread_attr_t reporter_attr;
                pthread_attr_init(&reporter_attr);

                pthread_create(&reporter_t, &reporter_attr, reporter, NULL);
                pthread_detach(reporter_t);
            }
            else{
                printf("\t[%02d:%02d%cm] Reporter %d leaves (session over)\n",cur_hour, cur_minute, am_pm, last_token_no_reporter);
            }
        }
        else if(e.type == 'P'){
            last_token_no_patient++;
            patient_duration[last_token_no_patient] = e.duration;
            printf("\t[%02d:%02d%cm] Patient %d arrives\n",cur_hour, cur_minute, am_pm, last_token_no_patient);
            if(last_token_no_patient <= 25){
                if(no_of_waiting_patient == 0 && no_of_waiting_reporter == 0 && no_of_waiting_salesrep == 0){
                    if(curTime >= doctorBusyTime){      // If doctor is free, schedule the patient
                        event e_doctor = {'D',curTime,0};
                        E = addevent(E, e_doctor);
                    }
                }
                no_of_waiting_patient++;
                pthread_t patient_t;
                pthread_attr_t patient_attr;
                pthread_attr_init(&patient_attr);

                pthread_create(&patient_t, &patient_attr, patient, NULL);
                pthread_detach(patient_t);
            }
            else if(leaveDoctor == 1){
                printf("\t[%02d:%02d%cm] Patient %d leaves (session over)\n",cur_hour, cur_minute, am_pm, last_token_no_patient);
            }
            else{
                printf("\t[%02d:%02d%cm] Patient %d leaves (quota full)\n", cur_hour, cur_minute, am_pm, last_token_no_patient);
            }
        }
        else if(e.type == 'S'){
            last_token_no_salesrep++;
            salesrep_duration[last_token_no_salesrep] = e.duration;
            printf("\t[%02d:%02d%cm] Sales representative %d arrives\n", cur_hour, cur_minute, am_pm, last_token_no_salesrep);
            if(last_token_no_salesrep <= 3){
                if(no_of_waiting_patient == 0 && no_of_waiting_reporter == 0 && no_of_waiting_salesrep == 0){
                    if(curTime >= doctorBusyTime){      // If doctor is free, schedule the salesrep
                        event e_doctor = {'D',curTime,0};
                        E = addevent(E, e_doctor);
                    }
                }
                no_of_waiting_salesrep++;
                pthread_t salesrep_t;
                pthread_attr_t salesrep_attr;
                pthread_attr_init(&salesrep_attr);

                pthread_create(&salesrep_t, &salesrep_attr, salesrep, NULL);
                pthread_detach(salesrep_t);
            }
            else if(leaveDoctor == 1){
                printf("\t[%02d:%02d%cm] Sales representative %d leaves (session over)\n", cur_hour, cur_minute, am_pm, last_token_no_salesrep);
            }
            else{
                printf("\t[%02d:%02d%cm] Sales representative %d leaves (quota full)\n", cur_hour, cur_minute, am_pm, last_token_no_salesrep);
            }
        }
    }
}