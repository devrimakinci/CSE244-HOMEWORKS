/*
* Dosya Ismi: main.c
* Yazan: Devrim Akıncı
* Numarasi: 141044052
* Tarih: 07 Mayis 2017
*/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/msg.h>

#define MICRO 1000000 /* Mikrosaniye birimine cevirmek icin */
#define MILI 1000 /* Milisaniye birimine cevirmek icin */
#define FOUND 0 /* Dosyadaki aranan kelimenin bulundugunda dondurulecek deger */
#define NOTFOUND -1 /* Dosyadaki aranan kelimenin bulunmadiginda dondurulecek deger */
#define PATHMAX 256 /* Maksimum path uzunlugu */
#define LOGFILENAME "log.txt" /* Log dosyasinin adi */
#define FILENAME "temp.txt"
#define TXT 4 /*.txt uzantili dosyayi anlamak icin tanimladigim deger */
#define PERM (S_IRUSR | S_IWUSR)
#define MAXTHREAD 256 /*Maksimum thread miiktari*/
#define MAXSIZE 256 /*Maksimum klasor sayisi*/
#define ASCIIZERO 48 /*Sifirin ASCII kodu*/

/* Parametre bilgisini tutan struct */
struct parameter{
    char* filename; /* Dosya ismi */
    char* word; /* Aranacak kelime*/
    FILE* logptr; /*Log dosyasinin file pointeri */
};

/* Verilerin bilgisini tutan struct */
struct data{

    int totalfound; /* Toplam bulunan kelime sayisi */
    int maxThread; /* Ayni anda bu islem sirasinda maksimum thread sayisi */
    int numberThread; /* Her directory olusturulan thread sayisi */
    int creatingThread; /* Arama sirasinda toplam kac tane thread olusturuldu */
    int numberFiles; /* Toplam aranan dosya sayisi */
    int numberDirectory; /* Toplam aranan klasor sayisi */
    int numberLines; /* Toplam aranan satir sayisi */
    double time; /* Programin calisma suresi */
};

struct myMessage{
    long messageType; /*Mesaj tipi*/
    struct data message; /*Gonderilecek mesaj*/
};

/**
 * void handler(int signum);
 *
 * Bu fonksiyon verilen sinyalin numarasina gore o sinyali handle eder.
 *
 */


void handler(int signum);

/**
 * int isWord(FILE *fp, char *word);
 *
 * Bu fonksiyon aranan kelimenin dosyada olup olmadigini kontrol eder.
 *
 * Parametreleri
 * fptr - File pointer
 * word - Aranacak kelime
 */

int isWord(FILE *fptr, char *word);

/**
 * char caseSensitivity (char ch);
 *
 * Bu fonksiyon verilen buyuk karakteri kucuk karaktere cevirir.
 * Eger karakter kucukse ayni sekilde return eder.
 *
 * Parametreleri
 * ch - Verilen karakter
 */

char caseSensitivity (char ch);

/**
 * int isTextFile(char* filename);
 *
 * Bu fonksiyon bulunan klasorun icindeki dosyalarin .txt uzantili olup
 * olmadigina bakar.
 *
 * Parametreleri
 * filename - Dosya adi
 */

int isTextFile(char* filename);

/**
 * int searchDir (char* dirName, char* word,FILE *logptr);
 *
 * Bu fonksiyon verilen klasor icerisinde verilen kelimeyi dosya gordugu
 * anda fork fonksiyonun cagirip yeni process olusturarak arar.
 *
 * Parametreleri
 * dirName - Klasor ismi
 * word - Aranacak kelime
 * logptr - Log dosyasinin pointeri
 */

int searchDir (char* dirName, char* word,FILE *logptr);

/**
 * void creatingLogFile (char* filename,char* filename2,char* word);
 *
 * Bu fonksiyon Log dosyasi olusturur ve klasor icerisinde bulunan kelimelerin
 * hangi dosyada hangi satirda ve sutunda bulundugunun bilgisini yazar. Son
 * olarak toplam bulunan kelime sayisini yazar.
 *
 * Parametreleri
 * logFilename - Log dosyasinin adi
 * dirName - Aranacak klasorun adi
 * word - Aranacak kelime
 */

void creatingLogFile (char* logFilename,char* dirName,char* word);

/**
 * !!!Bu fonksiyonu dersin anlatildigi slayttan aldim!!!
 *
 * int isDirectory(char *dirName);
 *
 * Bu fonksiyon verilen stringin klasor olup olmadigini anlar.
 *
 * Parametreleri
 * dirName - Verilen string
 */

int isDirectory(char *dirName);

/**
 * void* threadFunction(void *param);
 *
 * Bu fonksiyonu thread calistirir.
 * Fonksiyon verilen dosya icerisinde arancak kellimenin sayisini bulur.
 *
 * Parametreleri
 * param - Parametre bilgisini tutan degisken
 */

void* threadFunction(void *param);

/**
 * int findingFileSize (const char *filename);
 *
 * Bu fonksiyon verilen dosyanin icerisinde kac satir oldugunu bulur.
 *
 * Parametreleri
 * filename - Dosya ismi
 */

int findingFileSize (const char *filename);

/**
 * double getdiff(struct timeval start, struct timeval end);
 *
 * Bu fonksiyon milisaniye cinsinden zamani bulur.
 *
 * Parametreleri
 * start - struct timeval tipindeki degisken
 * end - struct timeval tipindeki degisken
 */

double getdiff(struct timeval start, struct timeval end);

/**
 * int findMaxThread(int threadArr[], int size);
 *
 * Bu fonksiyon verilen array'deki maksimum thread sayisini bulur.
 *
 * Parametreleri
 * threadArr - Her bir klasorun olusturdugu thread sayisini tutan array
 * size - Array'in boyutu
 */

int findMaxThread(int threadArr[], int size);

/**
 * void printScreen();
 *
 * Bu fonksiyon struct data icindeki degiskenlerin degerlerini ekrana basar.
 */

void printScreen();

/**
 * int creatingSharedMemory(struct data** structdata);
 *
 * Bu fonksiyon shared memory olusturur.
 *
 * Parametreleri
 * structdata - Memory den bu tipte(struct data) yer ayiracak olan parametre
 */

int creatingSharedMemory(struct data** structdata);

/**
 * int creatingMessageQueue();
 *
 * Bu fonksiyon message queue olusturur.
 */

int creatingMessageQueue();

/**
 * struct myMessage preparingMessage(struct myMessage mes);
 *
 * Bu fonksiyon gonderilecek mesaji hazirlar.
 *
 * Parametreleri
 *
 * mes - Gonderilecek mesaj
 */

struct myMessage preparingMessage(struct myMessage mes);

/**
 * void readingCascade(const char* filename,int* arr);
 *
 * Bu fonksiyon verilen bir dosyadan her bir klasorun altında olusan thread sayisini okur
 * filename - Dosya ismi
 * arr - Thread sayilarini tutacak array
 */

void readingCascade(const char* filename,int* arr);

/* Global Variables */
sem_t sem; /* Semafor */
FILE *fp;
struct data *myData;
struct timeval startTimeVal,endTimeVal;
static volatile sig_atomic_t doneflag = 0;
int indeks = -1;
int shmID; /* Shared Memory ID */
int mainPID = 0;
pthread_t th[MAXTHREAD]; /* Thread Array */
int cascadeArr[MAXSIZE];

int main(int argc, char *argv[]) {
    gettimeofday(&startTimeVal,NULL);
    double time;
    /* USAGE */
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    if (argc != 3) {
        fprintf(stderr, "Usage: %s word directory name\n", argv[0]);
        return 1;
    }
    sa.sa_handler = &handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        fprintf(stderr,"Failed to sigaction");
        return 1;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        fprintf(stderr,"Failed to sigaction!\n");
        return 1;
    }
    fp = fopen(FILENAME,"a+");
    mainPID = getpid();
    creatingLogFile(LOGFILENAME,argv[2],argv[1]);
    chdir("..");
    readingCascade(FILENAME,cascadeArr);
    myData->maxThread = findMaxThread(cascadeArr,myData->numberDirectory);
    gettimeofday(&endTimeVal,NULL);
    time = getdiff(startTimeVal,endTimeVal);
    myData->time = time;
    printScreen();
    printf("Exit condition: Normal\n");
    unlink(FILENAME);
    return 0;
}

void handler(int signum){
    if(signum == SIGINT){ /*CTRL+C nin handle edilmesi*/
        int pid;
        doneflag = 1;
        pid = getpid();
        if(mainPID != pid) {
            kill(getpid(), SIGKILL); /*Diger processler'e SIGKILL sinyalinin gonderilmesi*/
        }
    }
    else if(signum == SIGTERM){ /*SIGTERM sinyalinin hanle edilmesi*/
        int pid;
        doneflag = 2;
        pid = getpid();
        if(mainPID != pid) {
            kill(getpid(), SIGKILL); /*Diger processler'e SIGKILL sinyalinin gonderilmesi*/
        }
    }
    if (doneflag){
        double time;
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        if(doneflag == 1)
            printf("Exit condition: Signal No: %d\n",SIGINT);
        else if (doneflag == 2)
            printf("Exit condition: Signal No: %d\n",SIGTERM);
        chdir("..");
        unlink(FILENAME);
        exit(1);
    }
}

void readingCascade(const char* filename,int *arr){
    double time;
    char ch;
    int i=0;
    FILE *filePtr;
    filePtr = fopen(filename,"r");
    if(filePtr == NULL){
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf ("Exit condition: Due to ERROR NO:%d Failed to open file!\n",errno);
        exit(1);
    }
    ch = fgetc(filePtr);
    while(ch != EOF){
        arr[i] = ch - ASCIIZERO;
        ch = fgetc(filePtr);
        i++;
    }
    fclose(filePtr);
}

void printScreen(){
    int i;
    printf("Total number of strings found : %d\n",myData->totalfound);
    printf("Number of directories searched: %d\n",myData->numberDirectory);
    printf("Number of files searched: %d\n",myData->numberFiles);
    printf("Number of lines searched: %d\n",myData->numberLines);
    printf("Number of cascade threads created: ");
    for(i=0; i<myData->numberDirectory; i++){
        printf("%d ",cascadeArr[i]);
    }
    printf("\n");
    printf("Number of search threads created: %d\n",myData->creatingThread);
    printf("Max # of threads running concurrently: %d\n",myData->maxThread);
    printf("Number of created shared memory: %d\n",myData->numberDirectory);
    for(i=0; i<myData->numberDirectory; i++){
        printf("%d-Shared Memory Size:%d\n",i+1,(int)sizeof(struct data));
    }
    printf("Total run time in milliseconds: %.3lf\n",myData->time);
}

double getdiff(struct timeval start, struct timeval end){
    double diff=0;
    diff=(end.tv_sec-start.tv_sec) * MICRO + (end.tv_usec-start.tv_usec);
    return diff / MILI;
}

int isWord(FILE *fptr, char *word){
    int i=1;
    char ch; /* Okunacak karakter */
    if (i == strlen(word))/* Aranan kelime bir harfli ise*/
        return FOUND;
    do{
        ch = fgetc(fptr);/* Dosyadan karakter okuma islemi */
        ch = caseSensitivity(ch);/* Dosyadaki karakteri kucuge cevirme */
        /* Okunan karakter kelimenin ilk karakteri degilse */
        if (caseSensitivity(word[i]) == ch)
            i++;
            /* Okunan karakterin bosluk, tab veya new line karakteri oldugu durumda */
        else if ((ch == ' ' || ch == '\t') || (ch == '\n'))
            ; /* Program hicbir sey yapmayacak */
            /* Okunan karakter kelimenin ilk karakteri ise */
        else if (caseSensitivity(word[0]) == ch)
            return NOTFOUND;
            /*Okunan karakter kelimenin icerisinde yoksa */
        else if (caseSensitivity(word[i]) != ch)
            return NOTFOUND;
        if (i == strlen(word))
            return FOUND;
    }while(ch != EOF);/* Dosyanin sonuna kadar okuma islemi */
    return NOTFOUND;
}

char caseSensitivity (char ch){
    if (ch >= 'A' && ch <= 'Z')
        ch += 'a' - 'A';
    return ch;
}

int isTextFile(char* filename){
    if (strcmp(filename, ".txt")== 0)/*Dosyanin uzantisi .txt ise*/
        return 0;
    return -1;
}

int creatingSharedMemory(struct data** structdata){
    int shmid;
    shmid = shmget(IPC_PRIVATE,sizeof(struct data),PERM | IPC_CREAT);
    if((shmid == -1) && (errno != EEXIST)) {
        return -1;
    }
    if(shmid == -1){
        if(((shmid = shmget(IPC_PRIVATE,sizeof(struct data), PERM)) == -1) || ((*structdata = (struct data*)shmat(shmid,NULL,0)) == (void *) -1)) {
            return -1;
        }
    }
    else {
        *structdata = (struct data *) shmat(shmid, NULL, 0);
        if (*structdata == (void *) -1)
            return -1;
        (*structdata)->totalfound = 0;
        (*structdata)->creatingThread = 0;
        (*structdata)->maxThread = 0;
        (*structdata)->numberThread = 0;
        (*structdata)->numberDirectory = 1;
        (*structdata)->numberFiles = 0;
        (*structdata)->numberLines = 0;
    }
    return shmid;
}

int creatingMessageQueue(){
    int messageID;
    messageID = msgget(IPC_PRIVATE,PERM | IPC_CREAT);
    if(messageID == -1)
        return -1;
    return messageID;
}

struct myMessage preparingMessage(struct myMessage mes){
    mes.message.totalfound = myData->totalfound;
    mes.message.creatingThread = myData->creatingThread;
    mes.message.maxThread = myData->maxThread;
    mes.message.numberDirectory = myData->numberDirectory;
    mes.message.numberFiles = myData->numberFiles;
    mes.message.numberThread = myData->numberThread;
    mes.message.numberLines = myData->numberLines;
    return mes;
}

int searchDir (char* dirName, char* word,FILE* logptr){
    struct dirent *direntp; /* Dirent struct'i*/
    DIR *dirp; /* Directory pointer */
    pid_t child = 0; /*Process PID*/
    char mycwd[PATHMAX]; /*Su anda hangi klasorde calistigimi tutacak 256'lik string*/
    int filesize = 0;
    int indexThread = 0;
    int totalFound = 0;
    int error,mesID,j;
    double time;
    struct parameter myParam;
    struct myMessage *sendingMessage;
    indeks++;
    if ((dirp = opendir(dirName)) == NULL) {/* Klasorun acilmasi */
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf ("Exit condition: Due to ERROR NO:%d Failed to open directory!\n",errno);
        exit(1);
    }
    /* Su anda calistigim klasorun dogru alinip alinmadiginin kontrolu */
    if (getcwd(mycwd,PATHMAX) == NULL){
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf("Exit condition: Due to ERROR NO:%d Failed to get current working directory!\n",errno);
        exit(1);
    }
    chdir(dirName); /* Gosterilen klasore gitme */
    if (sem_init(&sem, 0, 1) == -1) {
        fprintf(stderr,"Failed to initialize semaphore");
        return 1;
    }
    shmID = creatingSharedMemory(&myData); /* Shared Memory nin olusturulmasi */
    if(shmID == -1){
        fprintf(stderr,"failed to creating shared memory!\n");
        exit(1);
    }
    mesID = creatingMessageQueue(); /* Message Queue nun olusturulmasi */
    if(mesID == -1){
        fprintf(stderr,"failed to creating message queue!\n");
        exit(1);
    }
    sendingMessage = (struct myMessage*) malloc(sizeof(struct myMessage));
    sendingMessage->messageType = 1;
    while ((direntp = readdir(dirp)) != NULL){/* Klasorun icini okuma islemi */
        if (strcmp(direntp->d_name,".") !=0 && strcmp(direntp->d_name,"..")!=0){
/*----------------------------Eger klasor okunduysa---------------------------*/
            if (isDirectory(direntp->d_name)){
                child = fork();/* Fork fonksiyonu ile yeni process olusturma*/
                if (child < 0){ /* Eger fork yapilamadiysa */
                    gettimeofday(&endTimeVal,NULL);
                    time = getdiff(startTimeVal,endTimeVal);
                    myData->time = time;
                    printScreen();
                    printf("Exit condition: Due to ERROR NO:%d Fork failed!\n",errno);
                    exit(1);
                }
                if (child == 0){ /* Child process */
                    /* Kelimeyi recursive olarak arar. */
                    searchDir(direntp->d_name,word,logptr);
                    chdir(".."); /* Bir ust klasore gitme */
                    (*sendingMessage) = preparingMessage((*sendingMessage));
                    if (msgsnd(mesID,sendingMessage,sizeof(struct myMessage),0) == -1)
                        fprintf(stderr,"Failed to sending message!\n");
                    exit(1); /* Child processin olmesi */
                }
                else{ /* Parent process */
                    struct myMessage *rcvMessage;
                    rcvMessage = (struct myMessage*)malloc(sizeof(struct myMessage));
                    msgrcv(mesID,rcvMessage,sizeof(struct myMessage),0,0);
                    myData->totalfound += rcvMessage->message.totalfound;
                    myData->maxThread += rcvMessage->message.maxThread;
                    myData->numberLines += rcvMessage->message.numberLines;
                    myData->creatingThread += rcvMessage->message.creatingThread;
                    myData->numberDirectory += rcvMessage->message.numberDirectory;
                    myData->numberFiles += rcvMessage->message.numberFiles;
                    myData->numberThread += rcvMessage->message.numberThread;
                    free(rcvMessage);
                }
            }
/*-------------------------Okunan dosya .txt uzantili ise---------------------*/
            else if (isTextFile((direntp->d_name + strlen(direntp->d_name)-TXT)) == 0){
                while (sem_wait(&sem) == -1)
                    if (errno != EINTR) {
                        gettimeofday(&endTimeVal,NULL);
                        time = getdiff(startTimeVal,endTimeVal);
                        myData->time = time;
                        printScreen();
                        printf("Exit condition: Due to ERROR NO:%d Thread failed to lock semaphore\n",errno);
                        exit(1);
                    }
                myParam.filename = direntp->d_name;
                myParam.word = word;
                myParam.logptr = logptr;
                /*Thread olusturulmasi*/
                error = pthread_create(&th[indexThread],NULL,threadFunction,&myParam);
                indexThread++;
                if (error) {
                    gettimeofday(&endTimeVal,NULL);
                    time = getdiff(startTimeVal,endTimeVal);
                    myData->time = time;
                    printScreen();
                    printf("Exit condition: Due to ERROR NO:%d Failed to create thread: %s!\n",errno,strerror(error));
                    exit(1);
                }
                myData->creatingThread++;
                myData->numberFiles = myData->creatingThread;
                filesize++;
            }
/*Eger program klasor veya .txt uzantili bir dosya okumadiysa hicbir sey yapmayacak*/
            else
                ;
        }
    }
    fprintf(fp,"%d",filesize);
    while (wait(NULL) > 0){} /* Butun cocuklar beklenir */
    for(j=0; j<indexThread; j++){
        /*Threadlerin beklemesi*/
        if (pthread_join(th[j],NULL)) {
            gettimeofday(&endTimeVal, NULL);
            time = getdiff(startTimeVal, endTimeVal);
            myData->time = time;
            printScreen();
            printf("Exit condition: Due to ERROR NO:%d Failed to join thread!\n", errno);
            return -1;
        }
    }
    free(sendingMessage);
    closedir(dirp);/* Klasorun kapanmasi */
    totalFound = myData->totalfound;
    shmctl(shmID,IPC_RMID,NULL);
    msgctl(mesID,IPC_RMID,NULL);
    return totalFound;
}

void creatingLogFile (char* logFilename,char *dirName,char* word){
    FILE* fptr; /* File pointer*/
    int totalFound; /* Toplam bulunan kelime sayisi*/
    double time;
    fptr = fopen(logFilename,"a");/* Log dosyasinin append modunda acilmasi*/
    if (fptr == NULL){ /* Dosyanin hatali acilmasi durumunda*/
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf("Exit condition: Due to ERROR NO:%d Failed to open file %s!\n",errno,logFilename);
        exit(1);
    }
    totalFound = searchDir(dirName,word,fptr);
    fprintf (fptr,"\n%d %s were found in total\n",totalFound,word);
    fclose(fptr);/* Dosyanin kapanmasi */
    fclose(fp);
}

int isDirectory(char *dirName) {
    struct stat statbuf;
    if (stat(dirName, &statbuf) == -1)
        return 0;
    else
        return S_ISDIR(statbuf.st_mode);
}

void* threadFunction(void *param){
    struct parameter myParam;
    myParam = *((struct parameter *) param);
    FILE *fp;
    double time;
    char ch; /* Okunacak karakter*/
    int colNum = 0; /* Sutun numarasi*/
    int rowNum = 1; /* Satir numarasi*/
    int firstCh = 0; /* Bulunan ilk karakterin dosyadaki yeri*/
    int filesize = 0;
    filesize = findingFileSize(myParam.filename);
    myData->numberLines = myData->numberLines + filesize;
    /* Dosyanin acilmasi */
    fp = fopen(myParam.filename,"r");
    if (fp == NULL) {/* Dosyanin acilmasinda hata varsa */
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf("Exit condition: Due to ERROR NO:%d Failed to open file %s!\n",errno,myParam.filename);
        exit(1);
    }
    do{
        colNum++;
        ch = fgetc(fp);/* Dosyadan karakter okuma islemi */
        ch = caseSensitivity(ch); /* Dosyadaki karakteri kucuge cevirme */
        /* Dosyanin icinde bosluk veya karakter oldugunda */
        if (ch == ' ' || ch == '\t')
            ; /* Program hicbir sey yapmayacak*/
            /* Dosyanin icinde new line karakteri varsa yapilacak islemler*/
        else if (ch == '\n'){
            colNum = 0;
            rowNum++;
        }
            /* Aranacak kelimenin ilk karakterini kucuge cevirme */
/* Dosyadan okunan karakterin aranacak kelimenin ilk karakteri esit oldugunda */
        else if (ch == caseSensitivity(myParam.word[0])){
/* Dosya icerisindeki aranan kelimenin ilk karakterinin yerini tutan degisken */
            firstCh = ftell(fp);
            if (isWord(fp,myParam.word) == 0){/* Aranan kelime bulunduysa*/
                myData->totalfound++;
                fprintf (myParam.logptr,"%d - %lu %s: [%d,%d] %s first character is found\n",
                         getpid(),pthread_self(),myParam.filename,rowNum,colNum,myParam.word);
                fflush(myParam.logptr);
/*File pointer'in yerini dosyadaki aranan kelimenin ilk karakterinin bulundugu yere goturme islemi*/
                fseek(fp,firstCh,SEEK_SET);
            }
            else{/* Aranan kelime bulunamadiysa */
/*File pointer'in yerini dosyadaki aranan kelimenin ilk karakterinin bulundugu yere goturme islemi*/
                fseek(fp,firstCh,SEEK_SET);
            }
        }
    }while (ch != EOF);/* Dosyanin sonuna kadar okuma islemi */
    fclose(fp);/* Dosyanin kapatilmasi */
    if (sem_post(&sem) == -1) {
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf("Exit condition: Due to ERROR NO:%d Thread failed to unlock semaphore\n",errno);
        exit(1);
    }
    pthread_exit(NULL); /*Threadlerin oldurulmesi*/
    return NULL;
}

int findingFileSize (const char *filename){
    FILE *fp;
    fp = fopen(filename,"r");
    char ch;
    int fileSize = 0;
    double time;
    if(fp == NULL){
        gettimeofday(&endTimeVal,NULL);
        time = getdiff(startTimeVal,endTimeVal);
        myData->time = time;
        printScreen();
        printf("Exit condition: Due to ERROR NO:%d Failed to open file!\n",errno);
        exit(1);
    }
    do{
        ch = fgetc(fp);
        if (ch == '\n')
            fileSize++;
    }while(ch != EOF);
    fclose(fp);
    return fileSize;
}

int findMaxThread(int threadArr[], int size){
    int i;
    int max;
    max = threadArr[0];
    for(i=1; i<size; i++){
        if(threadArr[i] > max){
            max = threadArr[i];
        }
    }
    return max;
}
