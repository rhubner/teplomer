/* 
 * File:   main.c
 * Author: radek
 *
 * Created on 11. rijen 2010, 18:02
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "temp_linux.c"   //odkomentovanim toho radku

#define BUFF_LENGTH 16

//#define _POSIX_SOURCE 1 /* POSIX compliant source */


int main(int argc, char** argv) {

    int fd = 0; //deskriptor portu
    int res = 0;
    int i = 0;
    char buff[BUFF_LENGTH];

    memset(buff, 0, sizeof (char) * BUFF_LENGTH);

    if (argc != 3) {
        printf("\nSpatne parametry prikazove radky\nprogram <cesta k portu> <adresa cidla>\ntemp /dev/ttyS0 A\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(argv[2]) != 1) {
        printf("spatna delka adresy");
        exit(EXIT_FAILURE);
    }

    fd = temp_open_serial_line(argv[1]); //otevreme seriovy port a nechame nastavit vsechny hodnoty

    if (fd < 0) {
        perror("chyba pri otevirani portu");
        exit(EXIT_FAILURE);
    }

    buff[i++] = 'T';
    buff[i++] = *argv[2];
    buff[i++] = 'I';
    buff[i] = 0;
    printf("zapisuju : %s\n", buff);
    
    res = temp_zapis_data(fd, buff, i);
    printf("zapsano : %d\n", res);
    res = 0;

    res = temp_cti_data(fd, buff, BUFF_LENGTH - 1, 10, 0x0A);

    if (res <= 0) {
        temp_close_port(fd);
        exit(EXIT_FAILURE);
    } else {
        buff[res] = 0; //oznacime konec retezce;

        printf("%s\n", buff);
        //printf("%d", buff[9]);

        temp_close_port(fd);

        exit(EXIT_SUCCESS);
    }
}
