#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>


pthread_mutex_t lock;
int stats[95] = {0};
int total_bytes_read = 0;
int listenfd  = -1;
int count_threads = 0;
int connfd    = -1;

void* count() {
	//int sockfd = (int) fd;
    int curr_stats[95] = {0};
    char buffer[1024];
    char len_of_bytes[1024];
    int chars_read = 0;
    int printable = 0;
    int i;
    int len = -1;
    int flag = 0;
    int sent_bytes_to_client = 0;
    int curr;
    int total_sent = 0;
    size_t num_read_chars ;
    while(len == -1 || chars_read < len ){
    	num_read_chars = read(connfd,buffer,sizeof(buffer));
    	if(num_read_chars < 0){
    		printf("error reading from socket\n");
    		///close all
    	}
    	for(i = 0;i<num_read_chars;i++){
    		if(flag == 0){
    			if(buffer[i] == '*'){
    				len = atoi(len_of_bytes);
    				flag = 1;
    			}
    			else{
    				len_of_bytes[i] = buffer[i];
    			}
    		}
    		if(buffer[i] >= 32 && buffer[i] <= 126){
    			curr_stats[buffer[i] - 32] ++;
    			printable++;
    		}

    	}
    	chars_read += num_read_chars;
		total_bytes_read += num_read_chars;
    }
    char back_to_client[1024];
    sprintf(back_to_client, "%d", printable);
    while( printable != 0){
    	printable /= 10;
        total_sent  ++;
    }
    while (sent_bytes_to_client < total_sent) {
    	curr = write(connfd, back_to_client, total_sent);
        if( curr < 0){
        	printf("error writing to client\n");
        	///close all
        }
        sent_bytes_to_client += curr;
    }
    if(pthread_mutex_lock(&lock) != 0){
    	printf("error locking mutex\n");
    	close(connfd);
    	pthread_exit(NULL);
    }

    for (i = 0; i < 95; i++){
        stats[i] += curr_stats[i];
    }
    if(pthread_mutex_unlock(&lock) != 0){
    	printf("error unlocking mutex\n");
    	close(connfd);
    	pthread_exit(NULL);

    }
    count_threads--;
    /* close socket  */
    close(connfd);
    pthread_exit(NULL);
}

void my_signal_handler( int signum,siginfo_t* info, void* ptr){
	close(listenfd);
	while(count_threads != 0){
		sleep(1);
	}
	 if(pthread_mutex_lock(&lock) != 0){
	    	printf("error locking mutex\n");
	    	exit(0);
	    }
	 printf("total bytes read is:%d\n", total_bytes_read);
	 printf("we saw ");
	 int i;
	 for(i=0;i<95;i++){
		 printf("%d '%c's, ",stats[i], (i+32));
	 }
	 printf("\n");
	  if(pthread_mutex_unlock(&lock) != 0){
	    	printf("error unlocking mutex\n");
	    	exit(0);
	    }
	  if (pthread_mutex_destroy(&lock) != 0) {
	          printf("error destroying mutex\n");
	          exit(0);
	      }
	  exit(0);

}


int main(int argc, char *argv[])
{
  //int connfd    = -1;
  int totalsent = -1;
  int nsent     = -1;
  int len       = -1;
  int n         =  0;

  pthread_t thread;
  struct sockaddr_in serv_addr;
  struct sockaddr_in my_addr;
  struct sockaddr_in peer_addr;
  char data_buff[1025];

  struct sigaction new_action;
  memset(&new_action, 0, sizeof(new_action));
	// Assign pointer to our handler function
  new_action.sa_sigaction = my_signal_handler;
	// Setup the flags
  new_action.sa_flags = SA_SIGINFO;
	// Register the handler
  if( 0 != sigaction(SIGINT, &new_action, NULL) ){
	printf("Signal handle registration "
	"failed. %s\n", strerror(errno));
	return -1;
	}

  pthread_mutex_init(&lock, NULL);

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset( &serv_addr, '0', sizeof(serv_addr));
  memset( data_buff, '0', sizeof(data_buff));

  serv_addr.sin_family = AF_INET;
  // INADDR_ANY = any local machine address
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(2233);

  if( 0 != bind( listenfd,
                 (struct sockaddr*) &serv_addr,
                 sizeof(serv_addr)))
  {
    printf("\n Error : Bind Failed. %s \n", strerror(errno));
    return 1;
  }

  if( 0 != listen(listenfd, 10) )
  {
    printf("\n Error : Listen Failed. %s \n", strerror(errno));
    return 1;
  }

  while(1)
  {
    // Prepare for a new connection
    socklen_t addrsize = sizeof(struct sockaddr_in );

    // Accept a connection.
    // Can use NULL in 2nd and 3rd arguments
    // but we want to print the client socket details
    connfd = accept( listenfd,
                     (struct sockaddr*) &peer_addr,
                     &addrsize);

    if( connfd < 0 )
    {
      printf("\n Error : Accept Failed. %s \n", strerror(errno));
      return 1;
    }

    getsockname(connfd, (struct sockaddr*) &my_addr,   &addrsize);
    getpeername(connfd, (struct sockaddr*) &peer_addr, &addrsize);
    printf("Server: Client connected.\n"
           "\t\tClient IP: %s Client Port: %d\n"
           "\t\tServer IP: %s Server Port: %d\n",
           inet_ntoa( peer_addr.sin_addr ),
           ntohs(     peer_addr.sin_port ),
           inet_ntoa( my_addr.sin_addr   ),
           ntohs(     my_addr.sin_port   ) );



    if ((pthread_create(&thread, NULL, count, NULL)) < 0){
          printf("error creating a thread\n");
          continue;
      }
    count_threads++;
  }
}





