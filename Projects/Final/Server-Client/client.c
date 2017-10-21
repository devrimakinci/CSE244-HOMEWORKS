/**
* Yazan: Devrim AKINCI
* Numara: 141044052
*
**/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define LOGFILE "clients.log"

struct data{
    int column;
    int row;
    long threadID;
};

void* threadFunction(void *param);
void freeMatrix(double** matrix, int col);
void readFromSocketMatrixA(double **matrix,int row,int col,int socketID);
void readFromSocketMatrixB(double *matrix,int row,int socketID);
double readFromSocketValue(int socketID);
void creatingLogFile(const char* logname, double** matrixA, double* matrixB, double* matrixX,double error,int row, int col);
void handler(int signum);

sem_t sem;
int count = 1;
int port;

int main(int argc, char** argv) {

    /*USAGE*/
    if(argc != 5){
        fprintf(stderr,"Usage: %s port row col threadSize\n",argv[0]);
        return -1;
    }

    port = atoi(argv[1]);
    int col = atoi(argv[2]);
    int row = atoi(argv[3]);
    int threadSize = atoi(argv[4]);

    int i=0,error;
    struct data myData;
    pthread_t th[threadSize];

    signal(SIGINT,handler);

    if (sem_init(&sem, 0, 1) == -1) {
        fprintf(stderr,"Failed to initialize semaphore");
        return 1;
    }
    myData.column = col;
    myData.row = row;

    for (i=0; i<threadSize; i++) {
        error = pthread_create(&th[i], NULL, threadFunction, &myData);
        if (error) {
            fprintf(stderr, "Failed to create thread!\n");
            return -1;
        }
    }
    for (i = 0; i < threadSize; i++) {
        if (pthread_join(th[i], NULL)) {
            fprintf(stderr, "Failed to join thread client!\n");
            return -1;
        }
    }
    return 0;
}
void handler(int signum){
    if(signum == SIGINT){
        sem_destroy(&sem);
        exit(1);
    }
}

void* threadFunction(void *param){
    int i,ret;
    struct data myData;
    myData = *(struct data*) param;
    struct sockaddr_in sa;
    int sfd;
    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0) {
        perror("Failed to socket!\n");
    }

    bzero(&sa,sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    sem_wait(&sem);

    ret = connect(sfd,(struct sockaddr *)&sa,sizeof sa);

    sem_post(&sem);


    if(ret < 0){
        perror("failed to connect client!\n");
    }
    myData.threadID = pthread_self();
    while(1) {
        double** matrixA;
        double* matrixB;
        double* matrixX;
        double error;
        matrixA = (double **)calloc(myData.row,sizeof(double*));
        for (i=0; i<myData.row; i++){
            matrixA[i] = (double*) calloc(myData.column,sizeof(double));
        }
        matrixB = (double *)calloc(myData.row,sizeof(double));
        matrixX = (double *)calloc(myData.column,sizeof(double));
        write(sfd,&myData,sizeof(myData));
        readFromSocketMatrixA(matrixA,myData.row,myData.column,sfd);
        readFromSocketMatrixB(matrixB,myData.row,sfd);
        readFromSocketMatrixB(matrixX,myData.column,sfd);
        error = readFromSocketValue(sfd);
        creatingLogFile(LOGFILE,matrixA,matrixB,matrixX,error,myData.row,myData.column);
        freeMatrix(matrixA,myData.column);
        free(matrixB);
        free(matrixX);
    }
    return 0;
}

void creatingLogFile(const char* logname, double** matrixA, double* matrixB, double* matrixX,double error,int row, int col){
    FILE* lp;
    int i,j;
    lp = fopen(logname,"a");
    if (lp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    fprintf(lp,"THREAD ID:%lu\n",pthread_self());
    fflush(lp);
    fprintf(lp,"MATRIX-A\n");
    fflush(lp);
    for(i=0; i<row; i++){
        for(j=0; j<col; j++){
            fprintf(lp,"%.2lf ",matrixA[i][j]);
            fflush(lp);
        }
        fprintf(lp,"\n");
        fflush(lp);
    }
    fprintf(lp,"MATRIX-B\n");
    fflush(lp);
    for(i=0; i<row; i++){
        fprintf(lp,"%.2lf\n",matrixB[i]);
        fflush(lp);
    }
    fprintf(lp,"MATRIX-X\n");
    fflush(lp);
    for(i=0; i<col; i++){
        fprintf(lp,"%.2lf\n",matrixX[i]);
        fflush(lp);
    }
    fprintf(lp,"ERROR:%.4lf\n",error);
    fflush(lp);
    fclose(lp);
}

void freeMatrix(double** matrix, int col){
    int i;
    for (i = 0; i < col; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void readFromSocketMatrixA(double **matrix,int row,int col,int socketID){
    int i,j;
    double value;
    for(i=0; i<row; i++){
        for(j=0; j<col; j++){
            if(read(socketID,&value,sizeof(double)) < 0 ){
                perror("Failed to read from socket CLIENT!\n");
                exit(1);
            }
            matrix[i][j] = value;
        }
    }
}
void readFromSocketMatrixB(double *matrix,int row,int socketID){
    int i;
    double value;
    for(i=0; i<row; i++){
        if(read(socketID,&value,sizeof(double)) < 0 ){
            perror("Failed to read from socket CLIENT!\n");
            exit(1);
        }
        matrix[i] = value;
    }
}
double readFromSocketValue(int socketID){
    double value;
    if(read(socketID,&value,sizeof(double)) < 0 ){
        perror("Failed to read from socket CLIENT!\n");
        exit(1);
    }
    return value;
}
