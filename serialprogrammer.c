/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2011,2013 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <inttypes.h>

struct baudentry {
	int flag;
	unsigned int baud;
};
#define BAUDENTRY(baud) { B##baud, baud },

static const struct baudentry sp_baudtable[] = {
	BAUDENTRY(9600) /* unconditional default */
#if defined(B19200)
	BAUDENTRY(19200)
#endif
#if defined(B38400)
	BAUDENTRY(38400)
#endif
#if defined(B57600) 
	BAUDENTRY(57600)
#endif
#if defined(B115200)
	BAUDENTRY(115200)
#endif
#if defined(B230400)
	BAUDENTRY(230400)
#endif
#if defined(B460800)
	BAUDENTRY(460800)
#endif
#if defined(B500000)
	BAUDENTRY(500000)
#endif
#if defined(B576000)
	BAUDENTRY(576000)
#endif
#if defined(B921600) 
	BAUDENTRY(921600)
#endif
#if defined(B1000000)
	BAUDENTRY(1000000)
#endif
#if defined(B1152000)
	BAUDENTRY(1152000)
#endif
#if defined(B1500000) 
	BAUDENTRY(1500000)
#endif
#if defined(B2000000)
	BAUDENTRY(2000000)
#endif
#if defined(B2500000)
	BAUDENTRY(2500000)
#endif
#if defined(B3000000)
	BAUDENTRY(3000000)
#endif
#if defined(B3500000)
	BAUDENTRY(3500000)
#endif
#if defined(B4000000)
	BAUDENTRY(4000000)
#endif
	{0, 0}			/* Terminator */
};

void set_baudrate(int fd, unsigned int baud) {
	int i;
	struct termios options;
	int bro = -1;
	for (i=0;sp_baudtable[i].baud;i++) {
		if (sp_baudtable[i].baud == baud) {
			bro = i;
			break;
		}
	}
	if (bro==-1) {
		printf("Cannot set baud rate %d\n",baud);
		exit(10);
	}
	tcgetattr(fd,&options);
	cfsetispeed(&options, sp_baudtable[bro].flag);
	cfsetospeed(&options, sp_baudtable[bro].flag);
	tcsetattr(fd, TCSANOW, &options);
}

/* command generation */
#define TAG_CHAR 0x55
#define INIT_CMD 0xAA
#define EXIT_CMD 0xFF

#define MAX_SIZE 65536
static uint8_t buffer[MAX_SIZE];

int open_devfd(char * fn) {
	struct termios options;
	int fd;
	fd = open(fn,O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd == -1) {
		printf("Serial open fail\n");
		exit(2);
		}
	fcntl(fd, F_SETFL, 0);
	tcgetattr(fd,&options);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	options.c_oflag &= ~OPOST;
	options.c_cc[VTIME] = 0;
	
	tcsetattr(fd, TCSANOW, &options);
	return fd;
	}

void dev_read(int fd, void*buf, int count) {
	int i;
	i = read(fd,buf,count);
	if (i != count) {
		printf("Read failed %d vs %d\n",i,count);
		exit(6);
	}
}

void dev_write(int fd, const void*buf, int count) {
	int i;
	i = write(fd,buf,count);
	if (i != count) {
		printf("Write failed\n");
		exit(5);
	}
}


void dev_reset(int devfd) {
	unsigned char c = EXIT_CMD;
	dev_write(devfd,&c,1); /* dev reset, bb */
}

int main(int argc, char * argv[]) {
	int argvbase=1;
	unsigned int bljump_baud = 0;
	uint8_t c;
	int devfd,flashfd,i,datasz,pagesz;
	if (argc < 3) { 
		printf("Usage: %s flash_file serial_device\n",argv[0]);
		exit(1);
	}
	if (strncmp(argv[argvbase],"--bljump=",9)==0) {
		bljump_baud = atoi(argv[argvbase]+9);
		argvbase++;
	}
	devfd = open_devfd(argv[argvbase+1]);
	printf("Connected.\n");
	if (bljump_baud) {
		set_baudrate(devfd,bljump_baud);
		printf("Doing BLJUMP (baud=%d).\n",bljump_baud);
		const char* cmd = "                         BLJUMP\r\n";
		dev_write(devfd,cmd,strlen(cmd));
		tcdrain(devfd);
		set_baudrate(devfd,115200);
	}
	sleep(1);
	tcflush(devfd,TCIFLUSH);
	flashfd = open(argv[argvbase],O_RDWR);
	if (flashfd == -1) {
		printf("Flash file open fail\n");
		exit(3);
	}
	memset(buffer,0xFF,MAX_SIZE);
	datasz = read(flashfd,buffer,MAX_SIZE);
	if (datasz < 1) {
			printf("Flash file read fail\n");
			exit(4);
	}
	close(flashfd);
	c = INIT_CMD;
	dev_write(devfd,&c,1);
	dev_read(devfd,&c,1);
	if (c != TAG_CHAR) {
			printf("Read invalid - 0x%02X\n",(int)c);
			exit(7);
	}
	dev_read(devfd,&c,1);
	pagesz = c;
	if (pagesz == 0) pagesz = 256;
	if (datasz&(pagesz-1)) datasz = (datasz&(~(pagesz-1)))+pagesz;
	printf("Entered programming mode.\nPagesize = %d. We have %d pages to write.\n",pagesz,datasz/pagesz);

	for (i=0;i<datasz;i+=pagesz) {
		c = i/pagesz;
		dev_write(devfd,&c,1);
		dev_write(devfd,&(buffer[i]),pagesz);
		dev_read(devfd,&c,1);
		if (c != TAG_CHAR) {
			printf("Read invalid - 0x%02X\n",(int)c);
			dev_reset(devfd);
			exit(8);
		}
		printf("%03d/%03d\r",(i/pagesz)+1,datasz/pagesz);
		fflush(stdout);
	}
	dev_reset(devfd);
	close(devfd);
	printf("\nEverything is ok\n");
	return 0;
}
