/*

  SERVER.C
  ==========
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net
  
  Simple TCP/IP echo server.

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sendlib.c"
#include "helper.h"

/*  Global constants  */

#define ECHO_PORT          (2002)
#define MAX_LINE           (1000)
#define SEGMENT_SIZE       (20)
#define HEADER_SIZE        (6)


/*  Error handling function. */

void convertAndWriteToFile(unsigned char* data, int length, char* filename, char type);

void print_error_and_exit(char *msg)
{
    perror(msg);
    printf("\n");
    exit(0);
}


int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */

    int n;
    float loss_probability;
    int random_seed;
    char rcv_buffer[SEGMENT_SIZE+HEADER_SIZE];
    int serv_name_size;
    char type;


    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */


    if (argc == 4) {
        port = strtol(argv[1], &endptr, 0);
        if ( *endptr ) {
           print_error_and_exit("Invalid port number.");
        }
        loss_probability = atof(argv[2]);

        if (loss_probability>1 || loss_probability<=0) {
            print_error_and_exit("loss probability must be between 0 and 1");
        }

        random_seed = atoi(argv[3]);

    } else {
        print_error_and_exit("argumnet (<server> <port> <loss probability> <random seed>) error");
    }

	
    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	   print_error_and_exit("Creating Socket");
    }


    unsigned int len_servaddr = sizeof(servaddr);

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);


    /*  Bind our socket addresss to the 
	listening socket, and call listen()  */

    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
	   print_error_and_exit("Binding");
    }

 //    if ( listen(list_s, LISTENQ) < 0 ) {
	// fprintf(stderr, "ECHOSERV: Error calling listen()\n");
	// exit(EXIT_FAILURE);
 //    }

    char seq_num;
    char pkt_serial = '0';
    
    int payload_size;
    char* serv_name;

    unsigned char* rcv_data = (unsigned char*) malloc(10);
    unsigned char* rcv_ptr = rcv_data;
    int i = 0;

    char curr_seq = '0';
    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( pkt_serial != '1' ) {

	/*  Wait for a connection, then accept() it  

	if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
	    fprintf(stderr, "ECHOSERV: Error calling accept()\n");
	    exit(EXIT_FAILURE);
	}

    */

	/*  Retrieve an input line from the connected socket
	    then simply write it back to the same socket.     */
    
    

	if ((n = recvfrom(list_s, rcv_buffer, SEGMENT_SIZE+HEADER_SIZE, 0, (struct sockaddr *)&servaddr, &len_servaddr) > 0)) {
       
        memcpy(&seq_num, rcv_buffer, 1);

        printf("Packet %c received \n", seq_num);

        if (curr_seq == seq_num) {
            memcpy(&pkt_serial, rcv_buffer+5, 1);

            if (pkt_serial == 'i') {
                memcpy(&type, rcv_buffer+6, 1);
                memcpy(&serv_name_size, rcv_buffer+7, 4);

                serv_name = (char*) malloc (serv_name_size);
                memcpy(serv_name, rcv_buffer+11, serv_name_size);

                // printf("Type %c\n", type);
                // printf("Server file name %s\n", serv_name);
            } else {
                memcpy(&payload_size, rcv_buffer+1, 4);
            // printf("Payload size: %d,  i: %d\n", payload_size, i);
                rcv_data = (unsigned char*) realloc(rcv_data, i+payload_size);
                rcv_ptr = rcv_data+i;
                
                memcpy(rcv_ptr, rcv_buffer+6, payload_size);
            }

            i += payload_size;

            if (curr_seq == '0')
                curr_seq = '1';
            else
                curr_seq = '0';

        } 


        // printf("Payload size is %d\n", payload_size);
        
        if ((n = lossy_sendto(loss_probability, random_seed, list_s, &seq_num, 1, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) 
        {
            print_error_and_exit("Error sending ACK");
        } 
        //printf("ACK %c sent\n", seq_num);

       
    }


    

	/*  Close the connected socket  */

	// if ( close(conn_s) < 0 ) {
	//     print_error_and_exit("Closing connection");
	// }
    }

    printf("Total data size: %d\n", i);

    convertAndWriteToFile(rcv_data, i, serv_name, type);

    // for (int j=0; j<i; j++) {
    //     printf("%c\n", rcv_data[j]);
    // }

    if ( close(conn_s) < 0 ) {
        print_error_and_exit("Closing connection");
    }
}



void convertAndWriteToFile(unsigned char* data, int length, char* filename, char type) {

    FILE *ptr;
    // unsigned char write_buffer[1];

    //printf("Length of file %d\n", length);

    int buffer[length];

    for (int i=0; i<length; i++) {
        buffer[i] = data[i];
        printf("%u ", buffer[i]);
    }

    printf("\n");

    int status = convert(filename, &type, buffer, length, 0);
    printf("Status %d\n", status);
    // ptr=fopen("target","wb");                      // Reading file in binary

}
