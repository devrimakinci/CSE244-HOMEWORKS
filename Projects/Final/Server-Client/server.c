/**
* Yazan: Devrim AKINCI
* Numara: 141044052 
*
**/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>

#define LOGFILE "server.log"
#define BACKLOG 8
#define MAXTHREAD 4096
#define THREADSIZE 3
#define PERM (S_IRUSR | S_IWUSR)
#define READ -11
#define WRITE -22
#define SVD 0
#define QR 1
#define INVERSE 2


struct data{
    int column;
    int row;
    long threadID;
};

struct threadSvd{
    int flagSvd;
    int column;
    int row;
};

struct threadQr{
    int flagQr;
    int column;
    int row;
};

struct threadInv{
    int flagInv;
    int column;
    int row;
};

struct sockaddr_in theirAddr;
sem_t sem;
int count = 0;
int shmID,shmID2,shmID3;
double *copyMatrix;
double *copyMatrixB;
double *copyMatrixX;
pid_t child1, child2, child3;
pthread_t th[MAXTHREAD];
pthread_mutex_t mut;
struct data myData;
key_t key1 = 9876;
key_t key2 = 1234;
key_t key3 = 1925;

void* threadFunction(void *param);
void freeMatrix(double** matrix, int col);
double** generateMatrixA(int row,int col);
double* generateMatrixB(int row);
void writeToSocketMatrixA(double** matrix,int row,int col,int socketID);
void writeToSocketMatrixB(double* matrix,int row,int socketID);
void writeToSocketValue(double value,int socketID);
void handler(int signum);
void creatingLogFile(const char* logname, double** matrixA, double* matrixB, int row, int col,long threadID);
void writingLogFileMatrixX(const char* logname, double* matrixX,int col);
void matrixMulti(double* multiply, double* first, double* second,int row1, int col1,int row2,int col2);
void transpose(double* matrixTranspose,double* matrix ,int row, int col);

int main(int argc, char** argv){

    /*USAGE*/
    if(argc != 3){
        fprintf(stderr,"Usage: %s port poolSize\n",argv[0]);
        return -1;
    }

    int sfd,cfd;
    int error;
    struct sockaddr_in sa;
    struct sigaction sig;
    int sinSize;

    int port = atoi(argv[1]);
    int poolSize =atoi(argv[2]);

    bzero(&sig,sizeof(sig));

    sfd = socket(AF_INET,SOCK_STREAM,0);
    sig.sa_handler = &handler;

    if (sigaction(SIGINT, &sig, NULL) < 0) {
        perror ("Sigaction failed\n");
        return 1;
    }

    if(sfd < 0 ) {
        perror("Failed to socket!\n");
        return -1;
    }

    if (sem_init(&sem, 0, 1) == -1) {
        fprintf(stderr,"Failed to initialize semaphore");
        return 1;
    }

    bzero(&sa,sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sfd,(struct sockaddr *)&sa,sizeof sa) < 0){
        perror("Failed to bind!\n");
        return -1;
    }

    if(listen(sfd,BACKLOG) == -1){
        perror("Failed to listen!\n");
        return -1;
    }

    sinSize = sizeof(struct sockaddr_in);

    for(;;){
        sem_wait(&sem);
        cfd = accept(sfd,(struct sockaddr*)&theirAddr,(socklen_t*)&sinSize);
        sem_post(&sem);
        if(cfd == -1)
            return -1;
        error=pthread_create(&th[count],NULL,threadFunction,&cfd);
        if(error){
            fprintf(stderr,"Failed to create thread Server!\n");
            return -1;
        }
        count++;
    }
    return 0;
}

void handler(int signum) {
    if (signum == SIGINT) {
        shmctl(shmID,IPC_RMID,NULL);
        shmctl(shmID2,IPC_RMID,NULL);
        shmctl(shmID3,IPC_RMID,NULL);
        pthread_mutex_destroy(&mut);
        kill(child1,SIGTERM);
        kill(child2,SIGTERM);
        kill(child3,SIGTERM);
        exit(1);
    }
    return;
}

void* threadHandler(void *param){
    pthread_mutex_lock(&mut);
    struct threadSvd myThreadData1;
    myThreadData1 = *(struct threadSvd*)param;
    int i;
    double value;
    if(myThreadData1.flagSvd == SVD){
        srand(getpid());
        for(i=0; i<myThreadData1.column; i++){
            value = (rand() % 10) + 1;
            copyMatrixX[i] = value;
        }
    }
    else if(myThreadData1.flagSvd == QR){
        srand(getpid());
        for(i=0; i<myThreadData1.column; i++){
            value = (rand() % 10) + 1;
            copyMatrixX[i] = value;
        }
    }
    else if(myThreadData1.flagSvd == INVERSE){
        srand(getpid());
        for(i=0; i<myThreadData1.column; i++){
            value = (rand() % 10) + 1;
            copyMatrixX[i] = value;
        }
    }
    pthread_mutex_unlock(&mut);
    pthread_exit(NULL);
}

void* threadFunction(void *param) {
    int sock = *(int *) param;
    int n, i, j;
    pthread_t th[THREADSIZE];
    double **matrixA;
    double *matrixB;
    shmID = shmget(key1, sizeof(double), PERM | IPC_CREAT);
    shmID2 = shmget(key2, sizeof(double), PERM | IPC_CREAT);
    shmID3 = shmget(key3, sizeof(double), PERM | IPC_CREAT);
    if ((shmID == -1 || shmID2 == -1 || shmID3 == -1) && (errno != EEXIST)) {
        exit(1);
    }
    copyMatrix = (double *) shmat(shmID, NULL, 0);
    copyMatrixB = (double *) shmat(shmID2, NULL, 0);
    copyMatrixX = (double *) shmat(shmID3, NULL, 0);
    if (copyMatrix == (void *) -1 || copyMatrixB == (void *) -1 || copyMatrixX == (void *) -1)
        exit(1);
    while ((n = read(sock, &myData, sizeof(myData)) > 0)) {
        copyMatrix[myData.row * myData.column] = WRITE;
        copyMatrixX[myData.row * myData.column] = WRITE;
        child1 = fork();
        if (child1 == -1) {
            perror("Fork failed!\n");
            exit(1);
        } else if (child1 == 0) {
            while (copyMatrix[myData.row * myData.column] != WRITE) {}
            matrixA = generateMatrixA(myData.row, myData.column);
            matrixB = generateMatrixB(myData.row);
            for (i = 0; i < myData.row; i++) {
                for (j = 0; j < myData.column; j++) {
                    copyMatrix[(i * myData.column) + j] = matrixA[i][j];
                }
            }
            for (i = 0; i < myData.row; i++) {
                copyMatrixB[i] = matrixB[i];
            }
            writeToSocketMatrixA(matrixA, myData.row, myData.column, sock);
            writeToSocketMatrixB(matrixB, myData.row, sock);
            creatingLogFile(LOGFILE,matrixA,matrixB,myData.row,myData.column,myData.threadID);
            free(matrixB);
            freeMatrix(matrixA,myData.column);
            copyMatrix[myData.row * myData.column] = READ;
            exit(1);
        } else {
            child2 = fork();
            if (child2 == -1) {
                perror("Fork failed!\n");
                exit(1);
            } else if (child2 == 0) {
                struct threadSvd svd;
                struct threadQr qr;
                struct threadInv inv;
                while (copyMatrix[myData.row * myData.column] != READ) {
                }
                while (copyMatrixX[myData.row * myData.column] != WRITE) {
                }
                if (pthread_mutex_init(&mut, NULL) != 0) {
                    perror("mutex init failed\n");
                    exit(1);
                }
                svd.row = myData.row;
                svd.column = myData.column;
                qr.row = myData.row;
                qr.column = myData.column;
                inv.row = myData.row;
                inv.column = myData.column;
                svd.flagSvd = 0;
                qr.flagQr = 1;
                inv.flagInv = 2;
                pthread_create(&th[0], NULL, threadHandler, &svd);
                pthread_create(&th[1], NULL, threadHandler, &qr);
                pthread_create(&th[2], NULL, threadHandler, &inv);
                for (i = 0; i < THREADSIZE; i++) {
                    if (pthread_join(th[i], NULL)) {
                        perror("Failed to join!\n");
                        exit(1);
                    }
                }
                copyMatrix[myData.row * myData.column] = WRITE;
                copyMatrixX[myData.row * myData.column] = READ;
                exit(1);
            }
            else {
                child3 = fork();
                if (child3 == -1) {
                    perror("Fork failed!\n");
                    exit(1);
                }
                else if (child3 == 0) {
                    while (copyMatrixX[myData.row * myData.column] != READ) {
                    }
                    int i;
                    double* result;
                    double *transposeError;
                    double *error;
                    double e;
                    result = (double*)calloc(myData.row,sizeof(double));
                    transposeError = (double*)calloc(myData.row,sizeof(double));
                    error = calloc(1,sizeof(double));
                    writeToSocketMatrixB(copyMatrixX,myData.column,sock);
                    writingLogFileMatrixX(LOGFILE,copyMatrixX,myData.column);
                    matrixMulti(result,copyMatrix,copyMatrixX,myData.row,myData.column,myData.column,1);
                    for(i=0; i<myData.row; i++){
                        result[i] = result[i] - copyMatrixB[i];
                    }
                    transpose(transposeError,result,1,myData.row);
                    matrixMulti(error,transposeError,result,1,myData.row,myData.row,1);
                    e = sqrt(error[0]);
                    writeToSocketValue(e,sock);
                    free(result);
                    free(error);
                    free(transposeError);
                    copyMatrixX[myData.row * myData.column] = WRITE;
                    exit(1);
                }
            }
        }
    }
    while(wait(NULL) > 0){}
    close(sock);
    pthread_exit(NULL);
}

double** generateMatrixA(int row, int col){
    int number;
    double** matrix;
    int i,j;
    struct timespec this;
    if(clock_gettime(CLOCK_REALTIME,&this)){
        perror("Failed to get first time");
        exit(1);
    }
    matrix = (double **)calloc(row,sizeof(double*));
    for(i=0; i<row; i++){
        matrix[i] = (double*)calloc(col,sizeof(double));
    }
    srand(this.tv_nsec);
    for(i=0; i<row; i++){
        for(j=0; j<col; j++){
            number = (rand() % 10) + 1;
            matrix[i][j] = number;
        }
    }
    return matrix;
}

double* generateMatrixB(int row){
    int number;
    double* matrix;
    int i;
    matrix = (double *)calloc(row,sizeof(double));
    srand(getpid());
    for(i=0; i<row; i++){
        number = (rand() % 10) + 1;
        matrix[i] = number;
    }
    return matrix;
}

void freeMatrix(double** matrix, int col){
    int i;
    for (i = 0; i < col; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void writeToSocketMatrixA(double** matrix,int row,int col,int socketID){
    double value;
    int i,j;
    for(i=0; i<row; i++){
        for(j=0; j<col; j++){
            value = matrix[i][j];
            if(write(socketID,&value,sizeof(double))< 0){
                perror("Failed to write socket SERVER!\n");
                exit(1);
            }
        }
    }
}
void writeToSocketMatrixB(double* matrix,int row,int socketID){
    double value;
    int i;
    for(i=0; i<row; i++){
        value = matrix[i];
        if(write(socketID,&value,sizeof(double))< 0){
            perror("Failed to write socket SERVER!\n");
            exit(1);
        }
    }
}

void writeToSocketValue(double value,int socketID){
    if(write(socketID,&value,sizeof(double))< 0){
        perror("Failed to write socket SERVER!\n");
        exit(1);
    }
}

void writingLogFileMatrixX(const char* logname, double* matrixX,int col){
    FILE* lp;
    int i;
    lp = fopen(logname,"a");
    if (lp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    fprintf(lp,"MATRIX-X\n");
    fflush(lp);
    for(i=0; i<col; i++){
        fprintf(lp,"%.2lf\n",matrixX[i]);
        fflush(lp);
    }
    fclose(lp);
}

void creatingLogFile(const char* logname, double** matrixA, double* matrixB, int row, int col,long threadID){
    FILE* lp;
    int i,j;
    lp = fopen(logname,"a");
    if (lp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    fprintf(lp,"THREAD ID:%lu\n",threadID);
    fprintf(lp,"MATRIX-A\n");
    for(i=0; i<row; i++){
        for(j=0; j<col; j++){
            fprintf(lp,"%.2lf ",matrixA[i][j]);
            fflush(lp);
        }
        fprintf(lp,"\n");
        fflush(lp);
    }
    fprintf(lp,"MATRIX-B\n");
    for(i=0; i<row; i++){
        fprintf(lp,"%.2lf\n",matrixB[i]);
        fflush(lp);
    }
    fclose(lp);
}

void matrixMulti(double* multiply, double* first, double* second,int row1, int col1,int row2,int col2) {
    int i, j, k;
    for (i = 0; i < row1; i++) {
        for (j = 0; j < col2; j++) {
            for (k = 0; k < col1; k++) {
                multiply[(i * col2) + j] += first[(i * col1) + k] * second[(k * col2) + j];
            }
        }
    }
}

void transpose(double* matrixTranspose,double* matrix ,int row, int col) {
    int i, j;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++) {
            matrixTranspose[(i * col) + j] = matrix[(j * row) + i];
        }
    }
}
