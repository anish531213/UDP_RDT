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

/*  Global constants  */

#define ECHO_PORT          (2002)
#define MAX_LINE           (1000)
#define SEGMENT_SIZE       (20)
#define HEADER_SIZE        (6)


/*  Error handling function. */

void writeToFile(char* buffer, int length);

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

    char status = '0';
    
    int payload_size;

    char* rcv_data = (char*) malloc(10);
    char* rcv_ptr = rcv_data;
    int i = 0;
    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( status != '1' ) {

	/*  Wait for a connection, then accept() it  

	if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
	    fprintf(stderr, "ECHOSERV: Error calling accept()\n");
	    exit(EXIT_FAILURE);
	}

    */

	/*  Retrieve an input line from the connected socket
	    then simply write it back to the same socket.     */
    
    

	if ((n = recvfrom(list_s, rcv_buffer, SEGMENT_SIZE+HEADER_SIZE, 0, (struct sockaddr *)&servaddr, &len_servaddr) > 0)) {
        memcpy(&payload_size, rcv_buffer+1, 4);
        printf("Payload size: %d,  i: %d\n", payload_size, i);
        rcv_data = (char*) realloc(rcv_data, i+payload_size);

        rcv_ptr = rcv_data+i;

        memcpy(rcv_ptr, rcv_buffer+6, payload_size);

        memcpy(&status, rcv_buffer+5, 1);

        printf("Status: %c\n", status);

        i += payload_size;
    }


    

	/*  Close the connected socket  */

	// if ( close(conn_s) < 0 ) {
	//     print_error_and_exit("Closing connection");
	// }
    }

    printf("Total data size: %d\n", i);

    writeToFile(rcv_data, i);

    // for (int j=0; j<i; j++) {
    //     printf("%c\n", rcv_data[j]);
    // }

    if ( close(conn_s) < 0 ) {
        print_error_and_exit("Closing connection");
    }
}



void writeToFile(char* buffer, int length) {

    FILE *ptr;
    // unsigned char write_buffer[1];

    ptr=fopen("target","wb");                      // Reading file in binary

    /*  If error in reading file  */

    if (!ptr) { 
        print_error_and_exit("Unable to open file!");     
    }

    /*  Checking the file length if it doesn't exceed 1000 bytes  */

    // fseek(ptr, 0, SEEK_END);
    // int lengthOfFile = ftell(ptr);
    // rewind(ptr);

    fwrite(buffer, length, 1, ptr);

    // int count = 0;

    // while (fread(read_buffer,sizeof(read_buffer),1,ptr) != 0) {
    //     // Saving the file buffer into data array
    //     buffer[count] = read_buffer[0];
    //     count += 1;
    // }

}
