/* 
 * Dosya Ismi: main.c
 * Yazan: Devrim Akıncı
 * Numarasi: 141044052
 * Tarih: 15 Mart 2017
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


#define FOUND 0 /* Dosyadaki aranan kelimenin bulundugunda dondurulecek deger */
#define NOTFOUND -1 /* Dosyadaki aranan kelimenin bulunmadiginda dondurulecek deger */
#define PATHMAX 256 /* Maksimum path uzunlugu */
#define LOGFILENAME "log.log" /* Log dosyasinin adi */
#define FIFONAME "myfifo" /* Fifo dosyasinin ismi */
#define TXT 4 /*.txt uzantili dosyayi anlamak icin tanimladigim deger */
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /*Fifo icin izinler */

/**
 * int searchWordFile(char *filename, char *word,FILE* logptr);
 *
 * Bu fonksiyon dosyadaki aranan kelime sayisini bulur.
 *
 * Parametreleri
 * filename - Dosya ismi
 * word - Aranacak kelime
 * logptr - Log dosyasinin pointeri
 */

int searchWordFile(char *filename, char *word,FILE* logptr);

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
 * int readFromPipe(int fileDes);
 *
 * Bu fonksiyon Pipe'dan veri okur ve o veriyi return eder.
 *
 * Parametreleri
 * fileDes - Okuma des.
 */

int readFromPipe(int fileDes);

/**
 * void writeToPipe(int fileDes, int value);
 *
 * Bu fonksiyon Pipe'a verilen degeri yazar.
 *
 * Parametreleri
 * fileDes - Yazma des.
 * value - Yazilacak deger
 */

void writeToPipe(int fileDes, int value);

/**
 * int readFromFifo(const char* fifoname);
 *
 * Bu fonksiyon fifo'dan veriyi okur ve onu return eder.
 *
 * Parametreleri
 * fifoname - Fifo dosyasinin adi
 */

int readFromFifo(const char* fifoname);

/**
 * void writeToFifo(const char* fifoname, int value);
 *
 * Bu fonksiyon fifo'ya verilen degeri yazar.
 *
 * Parametreleri
 * fifoname - Fifo dosyasinin adi
 * value - Yazilacak deger
 */

void writeToFifo(const char* fifoname, int value);

int main(int argc, char *argv[]) {
    /* USAGE */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s word directory name\n", argv[0]);
        return 1;
    }
    creatingLogFile(LOGFILENAME,argv[2],argv[1]);
    return 0;
}

int searchWordFile(char *filename,char* word,FILE* logptr){
    FILE *fp; /* File pointer */
    char ch; /* Okunacak karakter*/
    int colNum = 0; /* Sutun numarasi*/
    int rowNum = 1; /* Satir numarasi*/
    int firstCh=0; /* Bulunan ilk karakterin dosyadaki yeri*/
    int found = 0; /* Bulunan kelime sayisi*/
    /* Dosyanin acilmasi */
    fp = fopen(filename,"r");
    if (fp == NULL){/* Dosyanin acilmasinda hata varsa */
        fprintf(stderr,"Failed to open file %s!\n",filename);
        exit(1);
    }
    else{
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
            else if (ch == caseSensitivity(word[0])){
/* Dosya icerisindeki aranan kelimenin ilk karakterinin yerini tutan degisken */
                firstCh = ftell(fp);
                if (isWord(fp,word) == 0){/* Aranan kelime bulunduysa*/
                    found++;
                    fprintf (logptr,"%s: [%d,%d] %s first character is found\n",filename,rowNum,colNum,word);
/*File pointer'in yerini dosyadaki aranan kelimenin ilk karakterinin bulundugu yere goturme islemi*/
                    fseek(fp,firstCh,SEEK_SET);
                }
                else{/* Aranan kelime bulunamadiysa */
/*File pointer'in yerini dosyadaki aranan kelimenin ilk karakterinin bulundugu yere goturme islemi*/
                    fseek(fp,firstCh,SEEK_SET);
                }
            }
        }while (ch != EOF);/* Dosyanin sonuna kadar okuma islemi */
    }
    fclose(fp);/* Dosyanin kapatilmasi */
    return found;
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

int searchDir (char* dirName, char* word,FILE* logptr){
    struct dirent *direntp; /* Dirent struct'i*/
    DIR *dirp; /* Directory pointer */
    pid_t child = 0; /*Process PID*/
    char mycwd[PATHMAX]; /*Su anda hangi klasorde calistigimi tutacak 256'lik string*/
    int totalfound=0; /* Toplam bulunan kelime sayisi*/
    int found = 0; /* Bir dosyanin icinde aranan kelimenin sayisi*/
    int result = 0;
    int fileDes[2];
    if ((dirp = opendir(dirName)) == NULL) {/* Klasorun acilmasi */
        fprintf (stderr,"Failed to open directory!\n");
        return -1;
    }
    /* Su anda calistigim klasorun dogru alinip alinmadiginin kontrolu */
    if (getcwd(mycwd,PATHMAX) == NULL){
        fprintf(stderr,"Failed to get current working directory!\n");
        return -1;
    }
    chdir(dirName); /* Gosterilen klasore gitme */
    if (mkfifo(FIFONAME,FIFO_PERMS) < 0){ /* Fifo dosyasinin olusturulmasi*/
        fprintf (stderr,"Failed fifo!\n");
        return -1;
    }
    while ((direntp = readdir(dirp)) != NULL){/* Klasorun icini okuma islemi */
        if (strcmp(direntp->d_name,".") !=0 && strcmp(direntp->d_name,"..")!=0){
/*----------------------------Eger klasor okunduysa---------------------------*/
            if (isDirectory(direntp->d_name)){
                child = fork();/* Fork fonksiyonu ile yeni process olusturma*/
                if (child < 0){ /* Eger fork yapilamadiysa */
                    fprintf(stderr,"Fork failed!\n");
                    return -1;
                }
                if (child == 0){ /* Child process */
                    /* Kelimeyi recursive olarak arar. */
                    totalfound = totalfound + searchDir(direntp->d_name,word,logptr);
                    chdir(".."); /* Bir ust klasore gitme */
                    /* Recursive olarak bulunan kelime sayisinin fifo'ya yazilmasi */
                    writeToFifo(FIFONAME,totalfound);
                    exit(1); /* Child processin olmesi */
                }
                else{ /* Parent process */
                    /* Parent processin fifo'dan veriyi okumasi*/
                    result = readFromFifo(FIFONAME) + result;
                }
            }
/*-------------------------Okunan dosya .txt uzantili ise---------------------*/
            else if (isTextFile((direntp->d_name + strlen(direntp->d_name)-TXT)) == 0){
                if (pipe(fileDes) < 0){ /* Pipe olusturulmasi */
                    fprintf(stderr,"Failed pipe!\n");
                    return -1;
                }
                child = fork(); /* Fork fonksiyonu ile yeni process olusturma*/
                if (child < 0){ /* Eger fork yapilamadiysa */
                    fprintf(stderr,"Fork failed!\n");
                    return -1;
                }
                else if (child == 0){ /* Child process */
                    close(fileDes[0]);/* Okuma des. kapatilmasi */
                    /* Kelimenin dosyada aranmasi*/
                    found = searchWordFile(direntp->d_name,word,logptr);
                    /* Dosyadaki aranan kelimenin sayisini pipe'a yazar */
                    writeToPipe(fileDes[1],found);
                    close(fileDes[1]); /* Yazma des. kapatilmasi*/
                    exit(1);/* Child processin olmesi */
                }
                else{ /* Parent process */
                    close(fileDes[1]); /* Yazma des. kapatilmasi */
                    /* Pipe'dan yazilan veriyi okur */
                    result = readFromPipe(fileDes[0]) + result;
                    close(fileDes[0]); /* Okuma des. kapatilmasi */
                }
            }
/*Eger program klasor veya .txt uzantili bir dosya okumadiysa hicbir sey yapmayacak*/
            else
                ;
        }
    }
    while (wait(NULL) > 0){} /* Butun cocuklar beklenir */
    unlink(FIFONAME); /* Olusturulan fifo dosyanin silinmesi */
    closedir(dirp);/* Klasorun kapanmasi */
    return result;
}
void creatingLogFile (char* logFilename,char *dirName,char* word){
    FILE* fptr; /* File pointer*/
    int totalFound; /* Toplam bulunan kelime sayisi*/
    fptr = fopen(logFilename,"a");/* Log dosyasinin append modunda acilmasi*/
    if (fptr == NULL){ /* Dosyanin hatali acilmasi durumunda*/
        fprintf(stderr,"Failed to open file %s!\n",logFilename);
        exit(1);
    }
    totalFound = searchDir(dirName,word,fptr);
    fprintf (fptr,"\n%d %s were found in total\n",totalFound,word);
    fclose(fptr);/* Dosyanin kapanmasi */
}

int isDirectory(char *dirName) {
    struct stat statbuf;
    if (stat(dirName, &statbuf) == -1)
        return 0;
    else
        return S_ISDIR(statbuf.st_mode);
}

int readFromPipe(int fileDes){
    int status = 0;
    int result;
    /* Pipe'dan okuma yapilmasi */
    status = read(fileDes,&result,sizeof(int));
    if (status < 0)
        fprintf (stderr,"Failed to read from pipe!\n");
    return result;
}

void writeToPipe(int fileDes, int value){
    /* Pipe'a verinin yazilmasi */
    if (write(fileDes,&value,sizeof(int)) < 0)
        fprintf (stderr,"Failed to write to pipe!\n");
}

int readFromFifo(const char* fifoname){
    int result;
    FILE *fp;
    /* Fifo dosyasinin read mode'da acilmasi */
    fp = fopen(fifoname,"r");
    if (fp == NULL){
        fprintf(stderr,"Failed to open fifo for Open!\n");
    }
    fscanf(fp,"%d",&result); /* Fifo'dan verinin okunmasi */
    fclose(fp); /* Dosyanin kapatilmasi */
    return result;
}

void writeToFifo(const char* fifoname,int value){
    FILE* fp;
    /* Fifo dosyasinin write moddda acilmasi */
    fp = fopen(fifoname,"w+");
    if (fp == NULL){
        fprintf(stderr,"Failed to open fifo for Write!\n");
    }
    fprintf(fp,"%d",value); /*Fifo'ya verinin yazilmasi */
    fclose(fp); /* Dosyanin kapatilmasi */
}
