#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/random.h>
#include <ctype.h>
#include <fcntl.h>

// MINIMAL ERROR HANDLING FOR EASE OF READING

int main(int argc, char *argv[])
{

  if(argc != 2){
	  printf("not the right amount of arguments!\n");
	  return -1;
  }
  int len_of_bytes = atoi(argv[1]);
  int num_chars = strlen(argv[1]);
  int  sockfd     = -1;
  int  bytes_read =  0;
  char recv_buff[1024];
  int randomfd = -1;
  int chars_sent = 0;
  int buff_sent = 0;
  int buff_size;
  int total_sent = 0;
  char buff_random [1024];//check if the size is correct
  struct sockaddr_in serv_addr;
  struct sockaddr_in my_addr;
  struct sockaddr_in peer_addr;
  socklen_t addrsize = sizeof(struct sockaddr_in );

  memset(recv_buff, '0',sizeof(recv_buff));
  if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Error : Could not create socket \n");
    return 1;
  }

  // print socket details
  getsockname(sockfd,
              (struct sockaddr*) &my_addr,
              &addrsize);
  printf("Client: socket created %s:%d\n",
         inet_ntoa((my_addr.sin_addr)),
         ntohs(my_addr.sin_port));

  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(2233); // Note: htons for endiannes
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // hardcoded...

  printf("Client: connecting...\n");
  // Note: what about the client port number?
  // connect socket to the target address
  if( connect(sockfd,
              (struct sockaddr*) &serv_addr,
              sizeof(serv_addr)) < 0)
  {
    printf("\n Error : Connect Failed. %s \n", strerror(errno));
    return 1;
  }

  // print socket details again
  getsockname(sockfd, (struct sockaddr*) &my_addr,   &addrsize);
  getpeername(sockfd, (struct sockaddr*) &peer_addr, &addrsize);
  printf("Client: Connected. \n"
         "\t\tSource IP: %s Source Port: %d\n"
         "\t\tTarget IP: %s Target Port: %d\n",
         inet_ntoa((my_addr.sin_addr)),    ntohs(my_addr.sin_port),
         inet_ntoa((peer_addr.sin_addr)),  ntohs(peer_addr.sin_port));

  // read data from server into recv_buff
  // block until there's something to read
  // print data to screen every time
  randomfd = open("/dev/urandom", O_RDONLY);
  if(randomfd<0){
	  printf("error reading from urandom\n");
	  ///close everything
  }
 // int nDigits = (int) (floor(log10(abs(len_of_bytes))) + 1);
  sprintf(buff_random, "%d*", len_of_bytes);
     buff_size = num_chars + 1;
     while (buff_sent < buff_size) {
         chars_sent = write(sockfd, buff_random + buff_sent, (size_t) (buff_size));
         if(chars_sent < 0){
        	 printf("error writing to socket\n");
        	 //close all
         }
         printf("Client: wrote %d bytes\n", chars_sent);
         buff_sent += chars_sent;
     }
  while (total_sent < len_of_bytes) {
	  if((len_of_bytes - total_sent) > 1024){
		  buff_size = 1024;
	  }
	  else{
		  buff_size = len_of_bytes - total_sent;
	  }
	  if(read(randomfd,buff_random,buff_size) < 0){
		  printf("error reading from urandom\n");
		  ///close all
	  }
	  buff_sent = 0;
      while (buff_sent < buff_size) {
             chars_sent = write(sockfd, buff_random + buff_sent, (size_t) buff_size);
             // check if error occured (client closed connection?)
             if (chars_sent < 0) {
                 printf("error writing to socket\n");
                 //close all
                 return 1;
             }
             printf("Client: wrote %d bytes\n", chars_sent);
             total_sent += chars_sent;
             buff_sent += chars_sent;
         }
     }



  close(sockfd); // is socket really done here?
  //printf("Write after close returns %d\n", write(sockfd, recvBuff, 1));
  return 0;
}



