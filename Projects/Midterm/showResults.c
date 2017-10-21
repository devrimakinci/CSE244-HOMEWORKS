
/**
* Yazan: Devrim AKINCI
* Numara: 141044052 
*
**/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define RESULTNAME "result.txt"
#define LOGFILE "showResult.log"
#define DIRECTORYNAME "log"
#define PIDNAME "pid.txt"
#define TICKSFILE "ticks.txt"
#define BUFFSIZE 255
#define KILL -1
#define SIGNAL 0
#define MICRO 1000000
#define MILI 1000

struct client{
    int pid;
    double result1;
    double time1;
    double result2;
    double time2;
};

void handler(int signum);
int readResultFile(const char* filename, struct client *c);
void creatingLogFile(const char* logname, int pid, double result1, double result2, double time1, double time2);
void writeErrorMessageLogFile(const char* logname, double time,int signum,int chooseFlag);
void writeToPID(const char* filename);
int* split(char* string,char ch,int *size);
void readPidFile(const char* filename);
int readTicks(const char* filename);
double getdiff(struct timeval start, struct timeval end);
int msleep(long miliseconds);

char str[BUFFSIZE]={0};
char pidStr[BUFFSIZE]={0};
int count = 1;
struct timeval start,end;

int main(int argc, char* argv[]){

    gettimeofday(&start,NULL);
    struct client client1;
    int ticks;
    struct sigaction sa;
    writeToPID(PIDNAME);
    ticks = readTicks(TICKSFILE);
    sa.sa_handler = &handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }
    if (sigaction(SIGUSR1,&sa,NULL) < 0){
        perror ("sigaction");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

    while(1) {
        readResultFile(RESULTNAME, &client1);
        chdir(DIRECTORYNAME);
        creatingLogFile(LOGFILE,client1.pid,client1.result1,client1.result2,client1.time1,client1.time2);
        chdir("..");
        if(client1.result1 == 0.00){
            printf("%d %.2f(Shifted Det:0.00) %.2f\n", client1.pid, client1.result1, client1.result2);
        }
        else
            printf("%d %.2f %.2f\n", client1.pid, client1.result1, client1.result2);
        msleep(ticks);
    }
    return 0;
}

double getdiff(struct timeval start, struct timeval end){
    double diff=0;
    diff=(end.tv_sec-start.tv_sec) * MICRO + (end.tv_usec-start.tv_usec);
    return diff / MILI;
}

void handler(int signum){
    if (signum == SIGTERM){
        gettimeofday(&end,NULL);
        double time;
        time = getdiff(start,end);
        chdir(DIRECTORYNAME);
        writeErrorMessageLogFile(LOGFILE,time,signum,KILL);
        chdir("..");
        exit(1);
    }
    else if (signum == SIGINT){
        gettimeofday(&end,NULL);
        double time;
        int *pidArr;
        int size,i;
        time = getdiff(start,end);
        chdir(DIRECTORYNAME);
        writeErrorMessageLogFile(LOGFILE,time,signum,SIGNAL);
        chdir("..");
        readPidFile(PIDNAME);
        pidArr = split(pidStr,' ',&size);
        for(i=0; i<size; i++) {
            if (pidArr[i] != getpid()) {
                kill(pidArr[i], SIGTERM);
            }
        }
        free(pidArr);
        exit(1);
    }
}

void readPidFile(const char* filename){
    FILE *fp;
    fp = fopen(filename,"r");
    if(fp == NULL){
        fprintf(stderr,"Failed to open file!\n");
        exit(1);
    }
    fgets(pidStr,BUFFSIZE,fp);
    fclose(fp);
}

int* split(char* string,char ch,int *size){
    int i,val,j,k;
    char *splitStr;
    int *pidArray;
    *size = 0;
    j=0;
    k=0;
    for(i=0; string[i] != '\0'; i++){
        if(string[i] == ch)
            (*size)++;
    }
    splitStr = (char*) malloc((*size +1) * sizeof(char));
    pidArray = (int*) malloc((*size + 1) * sizeof(int));
    for(i=0; string[i] != '\0'; i++){
        while(string[i] != ch){
            if(string[i] == '\0')
                break;
            splitStr[k] = string[i];
            i++;
            k++;
        }
        k=0;
        val = atoi(splitStr);
        pidArray[j] = val;
        j++;
    }
    free(splitStr);
    return pidArray;
}

void writeErrorMessageLogFile(const char* logname, double time,int signum,int chooseFlag){
    FILE *fp;
    fp = fopen(logname,"a");
    if (fp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    if(chooseFlag == KILL){
        fprintf(fp,"%s signal kills this program!\n",strsignal(signum));
        fprintf(fp,"%s signal generated time:%.5f ms\n",strsignal(signum),time);
    }
    else if (chooseFlag == SIGNAL){
        fprintf(fp,"%s(CTRL+C) signal is received!\n",strsignal(signum));
        fprintf(fp,"%s signal generated time:%.5f ms\n",strsignal(signum),time);
    }
    fclose(fp);
}

void writeToPID(const char* filename){
    FILE *fp;
    fp = fopen(filename,"a");
    if(fp == NULL){
        fprintf(stderr,"Failed to creating text file!\n");
    }
    fprintf(fp,"%d ",getpid());
    fclose(fp);

}

int readResultFile(const char* filename, struct client *c){
    FILE *fp;
    int val;
    fp = fopen(filename,"r");
    if(fp == NULL){
        fprintf(stderr,"Failed to open file!\n");
        exit(1);
    }
    val = fscanf(fp, "%d %lf %lf %d %lf %lf", &c->pid, &c->result1, &c->time1, &c->pid, &c->result2, &c->time2);
    fclose(fp);
    return val;
}

void creatingLogFile(const char* logname, int pid, double result1, double result2, double time1, double time2){
    FILE *fp;
    fp = fopen(logname,"a");
    if (fp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    sprintf(str,"m%d",count);
    count++;
    fprintf(fp,"%s, %d\n%.2f, %.5f\n%2f, %.5f\n",str,pid,result1,time1,result2,time2);
    fclose(fp);
}

int readTicks(const char *filename){
    FILE *fp;
    int tick;
    fp = fopen(filename,"r");
    if (fp == NULL){
        fprintf(stderr,"Failed to open file!\n");
        exit(1);
    }
    fscanf(fp,"%d",&tick);
    fclose(fp);
    return tick;
}

int msleep(long miliseconds) {
    struct timespec start, end;

    if(miliseconds > MILI - 1){
        start.tv_sec = (int)(miliseconds / MILI);
        start.tv_nsec = (miliseconds - ((long)start.tv_sec * MILI)) * MICRO;
    }
    else{
        start.tv_sec = 0;
        start.tv_nsec = miliseconds * MICRO;
    }
    return nanosleep(&start , &end);
}
