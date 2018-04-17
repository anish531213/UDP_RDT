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


/*  Error handling function. */

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


    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ) {

	/*  Wait for a connection, then accept() it  

	if ( (conn_s = accept(list_s, NULL, NULL) ) < 0 ) {
	    fprintf(stderr, "ECHOSERV: Error calling accept()\n");
	    exit(EXIT_FAILURE);
	}

    */

	/*  Retrieve an input line from the connected socket
	    then simply write it back to the same socket.     */

	if ((n = recvfrom(list_s, buffer, MAX_LINE, 0, (struct sockaddr *)&servaddr, &len_servaddr) > 0)) {
        printf("%s", buffer);
    }



	/*  Close the connected socket  */

	// if ( close(conn_s) < 0 ) {
	//     print_error_and_exit("Closing connection");
	// }
    }
}