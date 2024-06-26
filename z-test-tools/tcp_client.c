/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <time.h>
#include <errno.h>

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


// #include "common.h"

#define BUFSIZE 8192

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

struct thread_data
{
  int  thread_id;
  int sockid;
};

int serverlen;
struct sockaddr_in serveraddr;
char readbuf[BUFSIZE];
int sockfd;

// int msleep(long tms)
// {
//     struct timespec ts;
//     int ret;

//     if (tms < 0)
//     {
//         errno = EINVAL;
//         return -1;
//     }

//     ts.tv_sec = tms / 1000;
//     ts.tv_nsec = (tms % 1000) * 1000000;

//     do {
//         ret = nanosleep(&ts, &ts);
//     } while (ret && errno == EINTR);

//     return ret;
// }

long time_index = 0;
struct timespec send_timestamp_arr[20000000];
struct timespec recv_timestamp_arr[20000000];
int sequence_ids[20000000];

void make_array(FILE* file, char* array, int size) {
   // read entire file into array
   fread(array, size, 1, file);
}

int main(int argc, char **argv) {
    int portno, n;
    
    // struct hostent *server;
    // char *src_hostip;
    char *dst_hostip;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <dst_hostip> <port>\n", argv[0]);
       exit(0);
    }
    // src_hostip = argv[1];
    dst_hostip = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(dst_hostip); 
    
    serveraddr.sin_port = htons(portno);

    serverlen = sizeof(serveraddr);

    // clkid = get_nic_clock_id();

    connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    printf("Connected to the server.\n");

    int m = 0;
    FILE* file = fopen("sample_file.txt", "r");
    while(m < 32000)
    {
        /* get a message from the user */
        bzero(buf, BUFSIZE);
        // char snum[5];
        // sprintf(snum, "%d", m);
        // strcat(buf, snum);
        // strcpy(buf, "HELLO, THIS IS CLIENT.");
        
        fseek(file, 0, SEEK_END);
        int fs = ftell(file);
        fseek(file, 0, SEEK_SET);
        make_array(file, buf, BUFSIZE);
        
        
        sequence_ids[time_index] = m;
        m++;

        /* send the message to the server */
        // sleep(0.001);

        //send time
		// struct timespec client_send_time = get_nicclock();
        // send_timestamp_arr[time_index] = get_nicclock();

        // n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
        send(sockfd, buf, strlen(buf), 0);
        // if (n < 0) 
        //     error("ERROR in sendto");
        
        /* print the server's reply */
        // n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
        // if (n < 0) 
        //     error("ERROR in recvfrom");
        // printf("Echo from server: %s \n", buf);

        //recv time
        // recv_timestamp_arr[time_index] = get_nicclock();

        // time_index++;
    }
    fclose(file);
    
    close(sockfd);
    printf("Disconnected from the server.\n");

    // int z = 0;
	// FILE *fpt;
	// fpt = fopen("./logs/mem_to_mem_send_l1_fpga.csv", "w+");
	// fprintf(fpt,"seq_id,send_time_part_sec,send_time_part_nsec,recv_time_part_sec,recv_time_part_nsec\n");
	// for (z = 0; z < time_index; z++ ) {
	// 	fprintf(fpt,"%d,%ld,%ld,%ld,%ld\n",sequence_ids[z],send_timestamp_arr[z].tv_sec,send_timestamp_arr[z].tv_nsec, recv_timestamp_arr[z].tv_sec,recv_timestamp_arr[z].tv_nsec);
	// }
	// fclose(fpt);

    return 0;
}