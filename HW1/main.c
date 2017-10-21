/* 
 * Dosya Ismi: main.c
 * Yazan: Devrim Akıncı
 * Numarasi: 141044052
 * Tarih: 27 Subat 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOUND 0 /* Dosyadaki aranan kelimenin bulundugunda dondurulecek deger*/
#define NOTFOUND -1 /* Dosyadaki aranan kelimenin bulunmadiginda dondurulecek deger*/

/**
 * int searchWordFile(char *filename, char *word);
 * 
 * Bu fonksiyon dosyadaki aranan kelime sayisini bulur.
 * 
 * Parametreleri
 * filename - Dosya ismi
 * word - Aranacak kelime
 * 
 */

int searchWordFile(char *filename, char *word);

/**
 * int isWord(FILE *fp, char *word);
 * 
 * Bu fonksiyon aranan kelimenin dosyada olup olmadigini kontrol eder.
 * 
 * Parametreleri
 * fptr - File pointer
 * word - Aranacak kelime
 * 
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

int main(int argc, char** argv) {
    int found = 0;
    if (argc != 3){
        fprintf (stderr, "Usage: %s word filename\n", argv[0]);
        return -1;
    }
    found = searchWordFile (argv[2],argv[1]);
    printf ("%d adet %s bulundu.\n",found,argv[1]);
    return 0;
}

int searchWordFile(char *filename,char* word){
    FILE *fp; /* File pointer */
    char ch; /* Okunacak karakter*/
    int colNum = 0; /* Sutun numarasi*/
    int rowNum = 1; /* Satir numarasi*/
    int firstCh=0; /* Bulunan ilk karakterin dosyadaki yeri*/
    int found = 0; /* Bulunan kelime sayisi*/
    /* Dosyanin acilmasi */
    fp = fopen(filename,"r");
    if (fp == NULL){/* Dosyanin acilmasinda hata varsa */
        fprintf(stderr,"Failed to open file %s",filename);
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
                   printf ("[%d, %d] konumunda ilk karakter bulundu.\n",rowNum,colNum);
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
    fclose(fp);
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
