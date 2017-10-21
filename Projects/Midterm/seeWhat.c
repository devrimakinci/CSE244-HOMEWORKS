
/**
* Yazan: Devrim AKINCI
* Numara: 141044052 
*
**/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <wait.h>
#include <sys/time.h>
#include <string.h>

#define FILENAME "server.txt"
#define RESULTFILE "result.txt"
#define DIRECTORYNAME "log"
#define PIDNAME "pid.txt"
#define CLIENTFIFONAME "myclientfifo"
#define KERNELSIZE 3
#define MILI 1000
#define MICRO 1000000
#define BUFFSIZE 255
#define ORIGINALMATRIX 0
#define SHIFTEDMATRIX 1
#define CONVOLUTIONMATRIX 2
#define FAILMATRIX -1
#define KILL -1
#define SIGNAL 0

struct data{
    int pid;
    int size;
};


void readFromText(const char* filename,struct data *data1);
void writeToFifo(const char* fifoname, int pid);
void writeToPID(const char* filename);
void readFromFifoMatrix(const char* fifoname,double** matrix,int size);
int shiftedMatrix(double** matrix,double** shiftedInverse ,int size);
void shiftedConvolution(double** matrix,double** shiftedConvolution,int size);
void findingConvolutionMatrix(double** original,double**kernel,double** conv,int size);
void swapRows(double** matrix,int row1,int row2,int size);
double determinant(double **matrix, int size);
void cofactor(double** matrix, double** inverse,int size);
void transpose(double** matrix,double** fac,double** inverse,int size);
void freeMatrix(double** matrix, int size);
void fillZeroConv(double** conv,int size);
void creatingKernelMatrix(double** kernel, int size);
void operation(double** matrix,int size);
void creatingLogFile(const char* logname,double** matrix, int size, int chooseFlag);
double getdiff(struct timeval start, struct timeval end);
void readPidFile(const char* filename);
void handler(int signum);
int* split(char* string,char ch,int *size);
void writeErrorMessageLogFile(const char* logname, double time,int signum,int chooseFlag);

char logName[BUFFSIZE]={0};
char pidStr[BUFFSIZE]={0};
struct timeval start,end;
int count = 1;

int main(int argc, char* argv[]) {

    /*USAGE*/
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mainfifoname>\n", argv[0]);
        return 1;
    }

    gettimeofday(&start,NULL);

    struct sigaction sa;
    struct data data1;
    int i;
    double **matrix;
    writeToPID(PIDNAME);
    readFromText(FILENAME,&data1);
    matrix = (double**) calloc(data1.size,sizeof(double*));
    for(i=0; i<data1.size; i++){
        matrix[i] = (double*) calloc(data1.size,sizeof(double));
    }
    sa.sa_handler = &handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror ("Sigaction failed\n");
        return 1;
    }
    if (sigaction(SIGTERM,&sa,NULL) < 0){
        perror ("Sigaction failed\n");
        return 1;
    }
    for(;;) {
        if (kill(data1.pid, SIGUSR1) == -1) {
            fprintf(stderr, "Failed to send signal!\n");
            exit(1);
        }
        writeToFifo(argv[1], getpid());
        readFromFifoMatrix(CLIENTFIFONAME, matrix, data1.size);
        operation(matrix, data1.size);
    }
    freeMatrix(matrix,data1.size);
    return 0;
}

void handler(int signum){
    if (signum == SIGTERM){
        gettimeofday(&end,NULL);
        double time;
        time = getdiff(start,end);
        chdir(DIRECTORYNAME);
        writeErrorMessageLogFile(logName,time,signum,KILL);
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
        writeErrorMessageLogFile(logName,time,signum,SIGNAL);
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

void creatingLogFile(const char* logname,double** matrix, int size, int chooseFlag){
    int i,j;
    FILE *fp;
    fp = fopen(logname,"a");
    if (fp == NULL){
        fprintf(stderr,"Failed to open log file!\n");
        exit(1);
    }
    if(chooseFlag == ORIGINALMATRIX){
        fprintf(fp,"Original Matrix=[ ");
        for(i=0; i<size; i++){
            for(j=0; j<size; j++){
                fprintf(fp,"%.2f ",matrix[i][j]);
            }
            fprintf(fp,"\n");
        }
        fprintf(fp,"];\n");
    }
    else if (chooseFlag == FAILMATRIX){
        fprintf(fp, "Determinant of matrix:0.0 and this matrix(%dx%d) is not invertible!\n",size/2,size/2);
    }
    else if (chooseFlag == SHIFTEDMATRIX){
        fprintf(fp,"Shifted Inverse Matrix=[ ");
        for(i=0; i<size; i++){
            for(j=0; j<size; j++){
                fprintf(fp,"%.2f ",matrix[i][j]);
            }
            fprintf(fp,"\n");
        }
        fprintf(fp,"];\n");

    }
    else if (chooseFlag == CONVOLUTIONMATRIX){
        fprintf(fp,"Shifted 2D Convolution Matrix=[ ");
        for(i=0; i<size; i++){
            for(j=0; j<size; j++){
                fprintf(fp,"%.2f ",matrix[i][j]);
            }
            fprintf(fp,"\n");
        }
        fprintf(fp,"];\n");
    }
    fclose(fp);
}

void operation(double** matrix,int size){
    FILE *fp;
    int i,child;
    double det1,det2,result1,result2;
    double** shifted;
    double** conv;
    shifted = (double**) calloc(size,sizeof(double*));
    for(i=0; i<size; i++){
        shifted[i] = (double*) calloc(size,sizeof(double));
    }
    conv = (double**) calloc(size,sizeof(double*));
    for(i=0; i<size; i++){
        conv[i] = (double*) calloc(size,sizeof(double));
    }
    fp = fopen(RESULTFILE,"w");
    if(fp == NULL){
        fprintf(stderr,"Failed to open result file!\n");
        exit(1);
    }
    fprintf(fp,"%d ",getpid());
    sprintf(logName,"SW%d.log",count);
    count++;
    chdir(DIRECTORYNAME);
    creatingLogFile(logName,matrix,size,ORIGINALMATRIX);
    chdir("..");
    child = fork();
    if(child < 0){
        fprintf(stderr,"Fork failed!\n");
        exit(1);
    }
    if(child == 0){
        int stat;
        double time;
        struct timeval startTimeVal,endTimeVal;
        struct timezone startTimezone,endTimezone;
        stat = shiftedMatrix(matrix,shifted,size);
        if (stat == -1){
            chdir(DIRECTORYNAME);
            creatingLogFile(logName,shifted,size,FAILMATRIX);
            chdir("..");
            fprintf(fp,"%.2f %.5f ",0.00,0.00000);
            exit(1);
        }
        det1 = determinant(matrix,size);
        det2 = determinant(shifted,size);
        gettimeofday(&startTimeVal,&startTimezone);
        result1 = det1 - det2;
        gettimeofday(&endTimeVal,&endTimezone);
        time = getdiff(startTimeVal,endTimeVal);
        fprintf(fp,"%.2f %.5f ",result1,time);
        chdir(DIRECTORYNAME);
        creatingLogFile(logName,shifted,size,SHIFTEDMATRIX);
        chdir("..");
        exit(1);
    }
    else{
        wait(NULL);
        double time;
        struct timeval startTimeVal,endTimeVal;
        struct timezone startTimezone,endTimezone;
        shiftedConvolution(matrix,conv,size);
        det1 = determinant(matrix,size);
        det2 = determinant(conv,size);
        gettimeofday(&startTimeVal,&startTimezone);
        result2 = det1 - det2;
        gettimeofday(&endTimeVal,&endTimezone);
        time = getdiff(startTimeVal,endTimeVal);
        fprintf(fp,"%.2f %.5f ",result2,time);
        chdir(DIRECTORYNAME);
        creatingLogFile(logName,conv,size,CONVOLUTIONMATRIX);
        chdir("..");
    }
    freeMatrix(shifted,size);
    freeMatrix(conv,size);
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

double getdiff(struct timeval start, struct timeval end){
    double diff=0;
    diff=(end.tv_sec-start.tv_sec) * MICRO + (end.tv_usec-start.tv_usec);
    return diff / MILI;
}

void readFromText(const char* filename,struct data *data1){
    FILE *fp;
    fp = fopen(filename,"r");
    if(fp == NULL){
        fprintf(fp,"Failed to open text file for reading!\n");
        exit(1);
    }
    fscanf(fp,"%d %d",&data1->pid,&data1->size);
    fclose(fp);
}

void readFromFifoMatrix(const char* fifoname,double** matrix,int size){

    int fd,i,j;
    double value;
    fd = open(fifoname,O_RDONLY);
    if (fd < 0){
        fprintf(stderr,"Failed to open fifo for reading matrix!\n");
        exit(1);
    }
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            if (read(fd,&value,sizeof(double)) < 0){
                fprintf(stderr,"Failed to write!\n");
                exit(1);
            }
            matrix[i][j] = value;
        }
    }
    close(fd);
}

void writeToFifo(const char* fifoname, int pid){
    int fd;
    fd = open(fifoname,O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open fifo for writing!\n");
        exit(1);
    }
    if (write(fd,&pid,sizeof(int)) < 0) {
        fprintf(stderr, "Failed to write!\n");
        exit(1);
    }
    close(fd);
}

int shiftedMatrix(double** matrix,double** shiftedInverse ,int size){
    int i,j,x,y;
    double det;
    double **temp;
    double **inverse;
    x=0;
    y=0;
    inverse =(double**) calloc(size/2,sizeof(double *));
    for(i=0; i<size/2; i++)
        inverse[i] = (double*)calloc(size/2,sizeof(double*));
    temp =(double**) calloc(size/2,sizeof(double *));
    for(i=0; i<size/2; i++)
        temp[i] = (double*)calloc(size/2,sizeof(double*));
    for(i=0; i<size/2; i++){
        for(j=0; j<size/2; j++)
            temp[i][j] = matrix[i][j];
    }
    det = determinant(temp,size/2);
    if (det == 0.0) {
        fprintf(stderr, "Determinant of matrix:0.0 and this matrix(%dx%d) is not invertible!\n",size/2,size/2);
        return -1;
    }
    cofactor(temp,inverse,size/2);
    for(i=0; i<size/2; i++){
        for(j=0; j<size/2; j++){
            shiftedInverse[i][j] = inverse[i][j];
        }
    }
    for(i=0; i<size/2; i++){
        for(j=size/2; j<size; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    det = determinant(temp,size/2);
    if (det == 0.0) {
        fprintf(stderr, "Determinant of matrix:0.0 and this matrix(%dx%d) is not invertible!\n",size/2,size/2);
        return -1;
    }
    cofactor(temp,inverse,size/2);
    for(i=0; i<size/2; i++){
        for(j=size/2; j<size; j++){
            shiftedInverse[i][j] = inverse[x][y];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    for(i=size/2; i<size; i++){
        for(j=0; j<size/2; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    det = determinant(temp,size/2);
    if (det == 0.0) {
        fprintf(stderr, "Determinant of matrix:0.0 and this matrix(%dx%d) is not invertible!\n",size/2,size/2);
        return -1;
    }
    cofactor(temp,inverse,size/2);
    for(i=size/2; i<size; i++){
        for(j=0; j<size/2; j++){
            shiftedInverse[i][j] = inverse[x][y];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    for(i=size/2; i<size; i++){
        for(j=size/2; j<size; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    det = determinant(temp,size/2);
    if (det == 0.0) {
        fprintf(stderr, "Determinant of matrix:0.0 and this matrix(%dx%d) is not invertible!\n",size/2,size/2);
        return -1;
    }
    cofactor(temp,inverse,size/2);
    for(i=size/2; i<size; i++){
        for(j=size/2; j<size; j++){
            shiftedInverse[i][j] = inverse[x][y];
            y++;
        }
        y=0;
        x++;
    }
    return 0;
}

void shiftedConvolution(double** matrix,double** shiftedConvolution,int size){
    int i,j,x,y;
    double **temp;
    double **kernel;
    double **conv;
    x=0;
    y=0;
    kernel =(double**) calloc(KERNELSIZE,sizeof(double *));
    for(i=0; i<KERNELSIZE; i++)
        kernel[i] = (double*)calloc(KERNELSIZE,sizeof(double*));
    conv =(double**) calloc(size/2,sizeof(double *));
    for(i=0; i<size/2; i++)
        conv[i] = (double*)calloc(size/2,sizeof(double*));
    temp =(double**) calloc(size/2,sizeof(double *));
    for(i=0; i<size/2; i++)
        temp[i] = (double*)calloc(size/2,sizeof(double*));
    for(i=0; i<size/2; i++){
        for(j=0; j<size/2; j++)
            temp[i][j] = matrix[i][j];
    }
    creatingKernelMatrix(kernel,KERNELSIZE);
    findingConvolutionMatrix(temp,kernel,conv,size/2);
    for(i=0; i<size/2; i++){
        for(j=0; j<size/2; j++){
            shiftedConvolution[i][j] = conv[i][j];
        }
    }
    for(i=0; i<size/2; i++){
        for(j=size/2; j<size; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    fillZeroConv(conv,size/2);
    findingConvolutionMatrix(temp,kernel,conv,size/2);
    for(i=0; i<size/2; i++){
        for(j=size/2; j<size; j++){
            shiftedConvolution[i][j] = conv[x][y];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    for(i=size/2; i<size; i++){
        for(j=0; j<size/2; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    fillZeroConv(conv,size/2);
    findingConvolutionMatrix(temp,kernel,conv,size/2);
    for(i=size/2; i<size; i++){
        for(j=0; j<size/2; j++){
            shiftedConvolution[i][j] = conv[x][y];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    for(i=size/2; i<size; i++){
        for(j=size/2; j<size; j++) {
            temp[x][y] = matrix[i][j];
            y++;
        }
        y=0;
        x++;
    }
    x=0;
    y=0;
    fillZeroConv(conv,size/2);
    findingConvolutionMatrix(temp,kernel,conv,size/2);
    for(i=size/2; i<size; i++){
        for(j=size/2; j<size; j++){
            shiftedConvolution[i][j] = conv[x][y];
            y++;
        }
        y=0;
        x++;
    }
    freeMatrix(kernel,KERNELSIZE);
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

double determinant(double **matrix, int size){
    int i,j,k;
    if (size == 1){
        return matrix[0][0];
    }
    else if (size == 2){
        return (matrix[0][0] * matrix[1][1]) - (matrix[0][1] * matrix[1][0]);
    }
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

void cofactor(double** matrix, double** inverse,int size){
    int p,q,m,n,i,j;
    double** b;
    double** fac;
    b = (double**)calloc(size,sizeof(double *));
    for(i=0; i< size; i++)
        b[i] = (double*)calloc(size,sizeof(double));
    fac = (double**)calloc(size,sizeof(double *));
    for(i=0; i< size; i++)
        fac[i] = (double*)calloc(size,sizeof(double));
    for (q=0;q<size;q++)
    {
        for (p=0;p<size;p++)
        {
            m=0;
            n=0;
            for (i=0;i<size;i++)
            {
                for (j=0;j<size;j++)
                {
                    if (i != q && j != p)
                    {
                        b[m][n]=matrix[i][j];
                        if (n<(size-2))
                            n++;
                        else
                        {
                            n=0;
                            m++;
                        }
                    }
                }
            }
            fac[q][p]=pow(-1,q + p) * determinant(b,size-1);
        }
    }
    transpose(matrix,fac,inverse,size);
    freeMatrix(b,size);
    freeMatrix(fac,size);
}

void transpose(double** matrix,double** fac,double** inverse,int size) {
    int i,j;
    double det;
    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            inverse[i][j]=fac[j][i];
        }
    }
    det = determinant(matrix,size);
    for (i=0;i<size;i++) {
        for (j = 0; j < size; j++) {
            inverse[i][j] = inverse[i][j] / det;
        }
    }
}

void creatingKernelMatrix(double** kernel, int size){
    int i,j;
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            if (i == 1 && j == 1)
                kernel[1][1] = 1.0;
            else
                kernel[i][j] = 0.0;
        }
    }
}

void findingConvolutionMatrix(double** original,double**kernel,double** conv,int size){

    int i,j,m,n,nn,mm,ii,jj,kernelCenterX,kernelCenterY;

    kernelCenterX = KERNELSIZE / 2;
    kernelCenterY = KERNELSIZE / 2;

    for(i=0; i < size; ++i) {
        for(j=0; j < size; ++j) {
            for(m=0; m < KERNELSIZE; ++m) {
                mm = KERNELSIZE - 1 - m;
                for(n=0; n < KERNELSIZE; ++n) {
                    nn = KERNELSIZE - 1 - n;
                    ii = i + (m - kernelCenterY);
                    jj = j + (n - kernelCenterX);
                    if( ii >= 0 && ii < size && jj >= 0 && jj < size )
                        conv[i][j] += original[ii][jj] * kernel[mm][nn];
                }
            }
        }
    }
}

void freeMatrix(double** matrix, int size){
    int i;
    for (i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void fillZeroConv(double** conv,int size){
    int i,j;
    for(i=0; i<size; i++){
        for(j=0; j<size; j++){
            conv[i][j] = 0.0;
        }
    }
}

