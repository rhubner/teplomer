/* 
 * File:   temp.h
 * Author: radek
 *
 * Created on 3. únor 2011, 17:41
 */

#ifndef TEMP_H
#define	TEMP_H


int temp_open_serial_line(char* port);

int temp_cti_data(const int fd, const char* buff, const int pocet, const int timeout, const char konec);

int temp_zapis_data(int fd, char* buff, int length);

void temp_vyprazdni_io_buffer(const int fd);

void temp_close_port(const int fd);



#endif	/* TEMP_H */

