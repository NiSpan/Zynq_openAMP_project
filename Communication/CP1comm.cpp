#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include "CP1comm.h"

static int fd =-1;
const char *rpmsg_dev="/dev/rpmsg0";
const int shutdown_msg = SHUTDOWN_MSG;
const int signal_start = SIGNAL_START;

void open_comm(){
	int size;
	fd = open(rpmsg_dev, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
  		perror("Failed to open rpmsg file /dev/rpmsg0.");
  	}
	printf("\r\n Query internal info .. \r\n");

	ioctl(fd, RPMSG_GET_KFIFO_SIZE, &size);

	printf(" rpmsg kernel fifo size = %u \r\n", size);

	ioctl(fd, RPMSG_GET_FREE_SPACE, &size);

	printf(" rpmsg kernel fifo free space = %u \r\n", size);
}

void write_to_cpu1(){
	int bytes_sent=0;
	double start_echo=-1;
	int bytes_rcvd=-1;
	bytes_sent = write(fd, &signal_start,sizeof(int));
        if (bytes_sent <= 0) {
        	printf("\r\n Error sending data");
		printf(" .. \r\n");
	  	return;
        }
	bytes_rcvd = read(fd, &start_echo,sizeof(double));
	while ((bytes_rcvd < 0) || (start_echo == 0)){
		usleep(1000);
		bytes_rcvd = read(fd, &start_echo,sizeof(double));
	}
  	//printf("CPU1 surf took : %4lf seconds\n ",start_echo);
	//printf("Start echo : %d\n",start_echo);

}
void read_from_cpu1(){
	int start_echo;
	int bytes_rcvd;
	
	bytes_rcvd = read(fd, &start_echo,sizeof(int));
	while (bytes_rcvd <= 0){
		usleep(1000);
		bytes_rcvd = read(fd, &start_echo,sizeof(int));
	}
	printf("Second read ok\n");
}

void close_comm(){
	int bytes_sent=0;
	bytes_sent = write(fd, &shutdown_msg, sizeof(int));
	if (bytes_sent <= 0) {
		printf("\r\n Error sending data");
		printf(" .. \r\n");
	}	
	close(fd);
}
