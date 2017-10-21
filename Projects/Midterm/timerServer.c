
/**
* Yazan: Devrim AKINCI
* Numara: 141044052 
*
**/


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>

#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /*Fifo icin izinler */
#define MICRO 1000000
#define MILI 1000
#define BUFFSIZE 255
#define KILL -1
#define SIGNAL 0
#define LOGNAME "timerServer.log"
#define FILENAME "server.txt"
#define RESULTFILE "result.txt"
#define TICKSFILE "ticks.txt"
#define CLIENTFIFONAME "myclientfifo"
#define PIDNAME "pid.txt"
#define DIRECTORYNAME "log"

void handler(int signum);
int readFromFifo (const char* fifoname);
double** creatingMatrix(int value);
double determinant(double **matrix, int size);
void swapRows(double** matrix,int row1,int row2,int size);
void writeToFifoMatrix(const char* fifoname,double** matrix,int size);
double getdiff(struct timeval start, struct timeval end);
void creatingLogFile(const char* logName, double time, int clientPid, double det);
void writeToText(const char* filename,int size);
int msleep(long miliseconds);
void freeMatrix(double** matrix, int size);
void writeToPID(const char* filename);
void readPidFile(const char* filename);
int* split(char* string,char ch,int *size);
void writeErrorMessageLogFile(const char* logname, double time,int signum,int chooseFlag);
void writeToTicks(const char* filename, int ticks);

char* fifoName;
int sizeMatrix;
int ticks;
int exitFlag = 0;
char pidStr[BUFFSIZE]={0};
struct timeval startTimeVal,endTimeVal;

int main(int argc, char* argv[]) {

    /*USAGE*/
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ticks in ms> <n> <mainfifoname>\n", argv[0]);
        return 1;
    }

    ticks = atoi(argv[1]);
    sizeMatrix = 2* (atoi(argv[2]));
    fifoName = argv[3];

    int res,stat;
    struct sigaction sa;

    if (sizeMatrix <= 0){
        fprintf(stderr,"Matrix size must be greater than zero!\n");
        exit(1);
    }

    gettimeofday(&startTimeVal,NULL);

    if(mkfifo(fifoName,FIFO_PERMS) < 0){
        fprintf(stderr,"Failed fifo!\n");
        return -1;
    }
    if(mkfifo(CLIENTFIFONAME,FIFO_PERMS) < 0){
        fprintf(stderr,"Failed fifo!\n");
        exit(1);
    }
    stat = mkdir(DIRECTORYNAME,0777);
    if(stat == 1){
        fprintf(stderr,"Failed to create directory!\n");
        return -1;
    }
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
    writeToText(FILENAME,sizeMatrix);
    writeToPID(PIDNAME);
    writeToTicks(TICKSFILE,ticks);
    while(!exitFlag){
        msleep(ticks);
    }

    return 0;
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

void handler(int signum){
    if (signum == SIGUSR1){
        int clientPID;
        struct timeval start,end;
        struct timezone startTimezone,endTimezone;
        double det,time;
        double **array;
        pid_t child;
        clientPID = readFromFifo(fifoName);
        child = fork();
        if(child < 0){
            fprintf(stderr,"Fork failed!\n");
            exit(1);
        }
        if(child == 0) {
            gettimeofday(&start,&startTimezone);
            array = creatingMatrix(sizeMatrix);
            gettimeofday(&end,&endTimezone);
            time = getdiff (start,end);
            det = determinant(array, sizeMatrix);
            if (det == 0.0)
                array = creatingMatrix(sizeMatrix);
            chdir(DIRECTORYNAME);
            creatingLogFile(LOGNAME,time,clientPID,det);
            chdir("..");
            writeToFifoMatrix(CLIENTFIFONAME,array,sizeMatrix);
            exit(1);
        }
        else{
            wait(NULL);
        }
    }
    else if (signum == SIGINT) {
        gettimeofday(&endTimeVal,NULL);
        int *pidArr;
        int size,i;
        double time;
        time = getdiff(startTimeVal,endTimeVal);
        chdir(DIRECTORYNAME);
        writeErrorMessageLogFile(LOGNAME,time,signum,SIGNAL);
        chdir("..");
        readPidFile(PIDNAME);
        pidArr = split(pidStr,' ',&size);
        for(i=0; i<size; i++){
            if(pidArr[i] != getpid()){
                kill(pidArr[i],SIGTERM);
            }
        }
        exitFlag = 1;
        free(pidArr);
        unlink(fifoName);
        unlink(CLIENTFIFONAME);
        unlink(FILENAME);
        unlink(RESULTFILE);
        unlink(PIDNAME);
        unlink(TICKSFILE);
        exit(1);
    }
    else if (signum == SIGTERM){
        gettimeofday(&endTimeVal,NULL);
        double time;
        time = getdiff(startTimeVal,endTimeVal);
        chdir(DIRECTORYNAME);
        writeErrorMessageLogFile(LOGNAME,time,signum,KILL);
        chdir("..");
        unlink(fifoName);
        unlink(CLIENTFIFONAME);
        unlink(FILENAME);
        unlink(RESULTFILE);
		unlink(PIDNAME);
        unlink(TICKSFILE);
        exit(1);
    }
    else{
        fprintf(stdout,"Program will be continue!\n");
    }
    msleep(ticks);
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

double getdiff(struct timeval start, struct timeval end){
    double diff=0;
    diff=(end.tv_sec-start.tv_sec) * MICRO + (end.tv_usec-start.tv_usec);
    return diff / MILI;
}

int readFromFifo (const char* fifoname) {
    int result;
    int fd;
    fd = open(fifoname,O_RDONLY);
    if (fd < 0){
        fprintf(stderr,"Failed to open fifo for reading!\n");
        exit(1);
    }
    if (read(fd,&result,sizeof(int)) < 0){
        fprintf(stderr,"Failed to read!\n");
        exit(1);
    }
    close(fd);
    return result;
}

void creatingLogFile(const char* logName, double time, int clientPid, double det){
    FILE* fp;
    fp = fopen(logName,"a");
    if (fp == NULL){
        fprintf(stderr,"Failed to create log file!\n");
        exit(1);
    }
    fprintf(fp,"Generated Matris Time: %.2lf ms Client PID: %d Determinat: %.2lf\n",time,clientPid,det);
    fclose(fp);
}

void writeToText(const char* filename, int size){
    FILE *fp;
    fp = fopen(filename,"w");
    if(fp == NULL){
        fprintf(stderr,"Failed to creating text file!\n");
    }
    fprintf(fp,"%d %d",getpid(),size);
    fclose(fp);
}

void writeToTicks(const char* filename, int ticks){
    FILE *fp;
    fp = fopen(filename,"w");
    if(fp == NULL){
        fprintf(stderr,"Failed to creating text file!\n");
    }
    fprintf(fp,"%d",ticks);
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

void writeToFifoMatrix(const char* fifoname, double** matrix,int size){
    int fd,i,j;
    double val;
    fd = open(fifoname,O_WRONLY);
    if (fd < 0){
        fprintf(stderr,"Failed to open fifo for writing matrix!\n");
        exit(1);
    }
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            val = matrix[i][j];
            if (write(fd,&val,sizeof(double)) < 0){
                fprintf(stderr,"Failed to write!\n");
                exit(1);
            }
        }
    }
    close(fd);
}

double** creatingMatrix(int value){
    int number;
    double **matrix;
    int i,j;
    matrix = (double**)calloc(value,sizeof(double *));
    for(i=0; i< value; i++)
        matrix[i] = (double*)calloc(value,sizeof(double));
    srand(time(NULL));
    for (i = 0;  i < value ; i++) {
        for (j = 0; j < value; j++) {
            number = (rand() % 10) + 1;
            matrix[i][j] = number;
        }
    }
    return matrix;
}

double determinant(double **matrix, int size){
    int i,j,k;
    double** temp;
    temp = (double**)calloc(size,sizeof(double *));
    for(i=0; i< size; i++)
        temp[i] = (double*)calloc(size,sizeof(double));
    double ratio,result;
    for(i=0; i< size; i++){
        for(j=0; j<size; j++){
            temp[i][j] = matrix[i][j];
        }
    }
    for(j = 0; j < size; j++){
        if(temp[j][j] == 0.0){
            swapRows(temp,j,j+1,size);
        }
        for(i = j+1; i < size; i++){
            ratio = temp[i][j]/temp[j][j];
            for(k = j; k < size; k++){
                temp[i][k] -= ratio * temp[j][k];
            }
        }
    }
    result = 1;
    for(i = 0; i < size; i++)
        result *= temp[i][i];
    freeMatrix(temp,size);
    return result;
}

void swapRows(double** matrix,int row1,int row2,int size){
    double temp;
    int j;
    for(j=0; j<size; j++){
        temp = matrix[row1][j];
        matrix[row1][j] = matrix[row2][j];
        matrix[row2][j] = temp;
    }

}

void freeMatrix(double** matrix, int size){
    int i;
    for (i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

