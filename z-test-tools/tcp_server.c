/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ptp_clock.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

// #include "common.h"

#define BUFSIZE 4096

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

extern bool running;// = true;
bool running = true;

static void sig_handler(int sig)
{
	running = false;
}

long time_index = 0;
struct timespec send_timestamp_arr[20000000];
struct timespec recv_timestamp_arr[20000000];
int sequence_ids[20000000];

int main(int argc, char **argv) {
  int sockfd,client_sock; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
//   clkid = get_nic_clock_id();
//   signal(SIGINT, sig_handler);
//   int m = 0;

  listen(sockfd, 5);
  printf("Listening...\n");

  while (1) {
    client_sock = accept(sockfd, (struct sockaddr*)&clientaddr, &clientlen);
    printf("[+]Client connected.\n");

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    recv(client_sock, buf, sizeof(buf), 0);
    printf("Client: %s\n", buf);

    close(client_sock);
    printf("[+]Client disconnected.\n\n");

    //recv time
    // sequence_ids[time_index] = atoi(buf);
    // recv_timestamp_arr[time_index] = get_nicclock();

    // printf("%d \n", sequence_ids[time_index]);
    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    // hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
	// 		  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    // if (hostp == NULL)
    //   error("ERROR on gethostbyaddr");
    // hostaddrp = inet_ntoa(clientaddr.sin_addr);
    // if (hostaddrp == NULL)
    //   error("ERROR on inet_ntoa\n");
    // printf("server received datagram from %s (%s)\n", 
	  //  hostp->h_name, hostaddrp);
    // printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
    
    /* 
     * sendto: echo the input back to the client 
     */

    //send time
    // send_timestamp_arr[time_index] = get_nicclock();

    // n = sendto(sockfd, buf, strlen(buf), 0, 
	//        (struct sockaddr *) &clientaddr, clientlen);
    // if (n < 0) 
    //   error("ERROR in sendto");

    // time_index++;
    // if (time_index > 1024) {
    //     break;
    // }
  }

    // int z = 0;
    // FILE *fpt;
    // fpt = fopen("./logs/mem_to_mem_recv_l2_fpga.csv", "w+");
    // fprintf(fpt,"seq_id,send_time_part_sec,send_time_part_nsec,recv_time_part_sec,recv_time_part_nsec\n");
    // for (z = 0; z < time_index; z++ ) {
    //     fprintf(fpt,"%d,%ld,%ld,%ld,%ld\n",sequence_ids[z],send_timestamp_arr[z].tv_sec,send_timestamp_arr[z].tv_nsec, recv_timestamp_arr[z].tv_sec,recv_timestamp_arr[z].tv_nsec);
    // }
    // fclose(fpt);

}