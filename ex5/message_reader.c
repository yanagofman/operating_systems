
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "message_slot.h"

int main(int argc, const char *argv[]) {
	if(argc != 2){
		printf("not the right amount of arguments\n");
		exit(-1);
	}
    int fd;
    int num_of_channel = atoi(argv[1]);
    char buff[BUF_LEN];


    fd = open("/dev/"DEVICE_FILE_NAME, O_RDONLY);
    if (fd < 0) {
        printf("failed to open device\n");
        exit(-1);
    }

    if(ioctl(fd, IOCTL_SET_CHA, num_of_channel) < 0){
    	printf("ioctl error\n");
    	exit(-1);
    }


    if(read(fd, buff, BUF_LEN) < 0){
    	printf("failed reading from message slot\n");
    	exit(-1);
    }


    printf("the message: %s was read successfully\n", buff);


    close(fd);
    return 0;
}

