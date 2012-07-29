
#ifndef TEMP_LINUX_H
#define	TEMP_LINUX_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>


#define BAUDRATE B9600
#define FALSE 0
#define TRUE  1

int temp_zapis_data(int fd, char* buff, int length) {
    return write(fd, buff, length);
}

void temp_vyprazdni_io_buffer(const int fd) {
    tcflush(fd, TCIOFLUSH);
}

void temp_close_port(const int fd) {
    close(fd);
}

int temp_cti_data(const int fd, const char* buff, const int pocet, const int timeout, const char konec) {
    int precteno = 0;
    int celkove_precteno = 0;
    int pokusu = 0;
    while (TRUE) {
        precteno = read(fd, (void*) (buff + celkove_precteno), pocet - celkove_precteno);
        if (precteno > 0) {
            celkove_precteno += precteno;
            if (celkove_precteno == pocet) {
                break; //precten celej buffer
            }
            if (konec != 0x00 && buff[celkove_precteno - 1] == konec) {
                break; //precten konec radky
            }
        } else { //nic se neprecetlo. Zaznamename jden pokus
            pokusu++;
            if (timeout != 0 && pokusu > timeout) {
                break;
            }
        }
    }
    return celkove_precteno;

}

int temp_open_serial_line(char* port) {
    int fd = 0;
    int dtr_status = 0;
    int ioctrl_success = 0;
    struct termios newtio;

    memset(&newtio, 0, sizeof (newtio));


    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(fd, F_SETFL, 0);
    //fcntl(fd, F_SETFL, FNDELAY);

    if (fd < 0) {
        return -1;
    }

    temp_vyprazdni_io_buffer(fd);

    tcgetattr(fd, &newtio);

    cfsetispeed(&newtio, BAUDRATE);
    cfsetospeed(&newtio, BAUDRATE);

    newtio.c_cflag = CLOCAL | CREAD;
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;
    newtio.c_cflag &= ~CRTSCTS; //vypneme HW rizeni toku

    //local options
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    newtio.c_iflag &= ~(INPCK);
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

    newtio.c_oflag &= ~OPOST;



    newtio.c_cc[VMIN] = 0;
    newtio.c_cc[VTIME] = 1;

    tcsetattr(fd, TCSANOW, &newtio);

    //nastaveni DTR do 1

    ioctrl_success = ioctl(fd, TIOCMGET, &dtr_status);
    if (ioctrl_success < 0) {
        return -2;
    }
    dtr_status |= TIOCM_DTR; //tohle je spravne, takle se nahazuje DTR
    //dtr_status &= ~TIOCM_DTR;       //tohle ne! tohle DTR schazuje. pro ukazku dobry

    ioctrl_success = ioctl(fd, TIOCMSET, &dtr_status);
    if (ioctrl_success < 0) {
        return -3;
    }

    temp_vyprazdni_io_buffer(fd);

    return fd;

}

#endif