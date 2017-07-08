

#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "message_slot.h"

int main(int argc, const char *argv[]) {
	if(argc != 3){
		printf("not the right amount of arguments\n");
		exit(-1);
	}
    int fd;
    int num_of_channel = atoi(argv[1]);
    char *message_to_send = (char *) argv[2];


    fd = open("/dev/"DEVICE_FILE_NAME, O_WRONLY);
    if (fd < 0) {
        printf("couldn't open device\n");
        exit(-1);
    }

    if(ioctl(fd, IOCTL_SET_CHA, num_of_channel) < 0){
    	printf("ioctl error\n");
    	exit(-1);
    }


    if( (int) write(fd, message_to_send, strlen(message_to_send)) < 0){
    	printf("failed writing to message slot\n");
    	exit(-1);
    }
    printf("the message was sent successfully\n");

    close(fd);
    return 0;
}


