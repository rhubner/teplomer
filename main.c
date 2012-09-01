/* 
 * File:   main.c
 * Author: radek
 *
 * Created on 11. rijen 2010, 18:02
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "temp_linux.c"   //odkomentovanim toho radku

#define BUFF_LENGTH 16

#define FORMAT_ALL 0
#define FORMAT_READ 1
#define TEMP_ERR -1000

//#define _POSIX_SOURCE 1 /* POSIX compliant source */


int cti_drak(const int fd, char adresa, char vstup);

int cti_teplotu(const int fd, const char adresa, int format);

int is_valid_char(const char c);

int main(int argc, char** argv) {

    FILE *vystup;

    const char *short_options = "p:t:d:";

    const struct option long_options[] = {
	{"port", 1, NULL, 'p'},
	{"temp", 1, NULL, 't'},
	{"drak", 1, NULL, 'd'},
	{NULL,   0, NULL,  0}
    };

    int fd = 0; //deskriptor portu
    int res = 0;
    int i = 0;
    char buff[BUFF_LENGTH];

    memset(buff, 0, sizeof (char) * BUFF_LENGTH);

    if (argc != 2) {
        printf("\nSpatne parametry prikazove radky\nprogram <cesta k portu> \ntemp /dev/ttyS0 \n");
        exit(EXIT_FAILURE);
    }

    fd = temp_open_serial_line(argv[1]); //otevreme seriovy port a nechame nastavit vsechny hodnoty

    if (fd < 0) {
        perror("chyba pri otevirani portu");
        exit(EXIT_FAILURE);
    }


    int hodnota = cti_drak(fd, '1', '3');
  
    if(hodnota >= 0) {
      vystup = fopen("/tmp/tlak_now.txt", "w");
      fprintf(vystup, "%d", hodnota);
      fclose(vystup);
    }
 
    temp_close_port(fd);

    exit(EXIT_SUCCESS);
}



int cti_teplotu(const int fd, const char adresa, const int format) {

    char buff[BUFF_LENGTH];
    char temp[BUFF_LENGTH];
    int res = 0;
    int i = 0;
    int j = 0;
    int value;

    buff[i++] = 'T';
    buff[i++] = adresa;
    buff[i++] = (format == FORMAT_READ) ? 'R' : 'I' ;
    buff[i] = 0;
    printf("zapisuju : %s\n", buff);
    
    res = temp_zapis_data(fd, buff, i);
    printf("zapsano : %d\n", res);
    res = 0;

    res = temp_cti_data(fd, buff, BUFF_LENGTH - 1, 10, 0x0D);	//TQS by taky mel pouzivat 0x0D

//    sprintf(buff, "*H-025.9C");		//testovani zaporne hodnoty
//    buff[9] = 0x0D;
//    res = 10;

    if(res <= 8)
      return TEMP_ERR;

    buff[res] = 0;
    printf("prislo z cidla : %s\n", buff);

    memset(temp, 0, BUFF_LENGTH);
    
    for(j = 0, i = 0 ; buff[j] != 0 && j < BUFF_LENGTH; j++) {
      if(is_valid_char(buff[j]))
        temp[i++] = buff[j];
    }
    printf("zkopirovano %s\n", temp);

    sscanf(temp, "%d", &value);
    
    printf("hodnota : %d", value);    
    
    return value;
}


int cti_drak(const int fd, const char adresa, const char vstup) {
    char buff[BUFF_LENGTH];
    char temp[BUFF_LENGTH];
    int res = 0;
    int i = 0;
    int value;
    int crc;
    int crc2;

    memset(buff, 0, sizeof (char) * BUFF_LENGTH);

    buff[i++] = '*';
    buff[i++] = adresa;
    buff[i++] = 'M';
    buff[i++] = vstup;
    buff[i] = 0;
    printf("zapisuju : %s\n", buff);

    temp_vyprazdni_io_buffer(fd);	//vycistime buffer
 
    res = temp_zapis_data(fd, buff,i);	//zapiseme dotaz na hodnotu
 
    res = temp_cti_data(fd, buff, BUFF_LENGTH -1, 14, 0x0D);	// drak ukoncuje znakem 0x0D (CR)
    if(res != 8)
      return -1;
    buff[res] = 0; //konec retezce
    printf("delka dat prislo z draka %s\n", buff);
    
    memcpy(temp, buff, 5);
    temp[5] = 0;
    printf("zkopirovano %s\n", temp);
    sscanf(temp, "%d", &value);
    
    memcpy(temp, buff+5, 2);
    temp[2] = 0;
    printf("zkopirovano %s\n", temp);
    
    sscanf(temp, "%x", &crc);

    printf("hodnota : %d, crc : %d\n", value, crc);

   //overeni CRC 
    crc2 = buff[0] + buff[1] + buff[2] + buff[3] + buff[4];
    if(crc2 % 256 != crc) {
        printf("nesouhlasi crc %d %d\n", crc2 % 256, crc);
        return -2;
    }

    return value;
}

int is_valid_char(const char c) {
    if( (c >= '0' && c <= '9') || c == '+' || c == '-') {
      return 1;
    } else {
      return 0;
    }
}
