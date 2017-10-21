/* 
 * Dosya Ismi: main.c
 * Yazan: Devrim Akıncı
 * Numarasi: 141044052
 * Tarih: 06 Mart 2017
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


#define FOUND 0 /* Dosyadaki aranan kelimenin bulundugunda dondurulecek deger*/
#define NOTFOUND -1 /* Dosyadaki aranan kelimenin bulunmadiginda dondurulecek deger*/
#define PATHMAX 256 /* Maksimum path uzunlugu*/
#define LOGFILENAME "log.log" /* Log dosyasinin adi*/
#define TXT 4 /*.txt uzantili dosyayi anlamak icin tanimladigim deger */

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
                    found = totalfound;
                    chdir(".."); /* Bir ust klasore gitme*/
                    exit(found); /* Child processin cikis kodu*/
                }
                else{ /* Parent process */
                    wait(&found); /* Child processin cikis kodunu bekler*/
                    totalfound = WEXITSTATUS(found);
                }
            }
/*-------------------------Okunan dosya .txt uzantili ise---------------------*/            
            else if (isTextFile((direntp->d_name + strlen(direntp->d_name)-TXT)) == 0){
                child = fork(); /* Fork fonksiyonu ile yeni process olusturma*/
                if (child < 0){ /* Eger fork yapilamadiysa */
                    fprintf(stderr,"Fork failed!\n");
                    return -1;
                }
                else if (child == 0){ /* Child process */
                    /* Kelimenin dosyada aranmasi*/
                    found = searchWordFile(direntp->d_name,word,logptr);
                    exit(found);/* Child process cikis kodu */
                }
                else{ /* Parent process */
                    wait(&found); /* Child processin cikis kodunu bekler */
                    totalfound = WEXITSTATUS(found) + totalfound;
                }
            }
/*Eger program klasor veya .txt uzantili bir dosya okumadiysa hicbir sey yapmayacak*/            
            else
                ;
        }
    }
    closedir(dirp);/* Klasorun kapanmasi */
    return totalfound;
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
