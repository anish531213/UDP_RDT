/*

  ECHOCLNT.C
  ==========
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net
  
  Simple TCP/IP echo client.

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*  Global constants  */

#define MAX_LINE           (1000)


/*  Function declarations  */

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort, char **read_file, char** type, char** server_file, float* loss_probability, int* random_seed);

void readFromFile(char* buffer, char* read_file);

/*  Error handling function. */

void print_error_and_exit(char *msg)
{
    perror(msg);
    printf("\n");
    exit(0);
}


/*  main()  */

int main(int argc, char *argv[]) {

    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *szAddress;             /*  Holds remote IP address   */
    char     *szPort;                /*  Holds remote port         */
    char     *endptr;                /*  for strtol()              */

    char     *type;                  /*  Holds type variable       */
    char     *read_file;             /*  file to read by client    */
    char     *server_file;           /*  file name to be saved at server  */
    int      random_seed;            /*  random seed  */
    float    loss_probability;       /*  loss probability */
    

    int len_servaddr;
    int n;
 
    if (argc != 8) {
        print_error_and_exit("Invalid arguments!");
    }

    /*  Get command line arguments  */

    ParseCmdLine(argc, argv, &szAddress, &szPort, &read_file, &type, &server_file, &loss_probability, &random_seed);

    if (loss_probability>1 || loss_probability<=0) {
        print_error_and_exit("loss probability must be between 0 and 1");
    }

    /*  Set the remote port  */

    port = strtol(szPort, &endptr, 0);
    if ( *endptr ) {
	   print_error_and_exit("Invalid port.");
    }
	

    /*  Create the listening socket  */

    if ( (conn_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	   print_error_and_exit("Creating Socket");
    }

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(port);


    /*  Set the remote IP address  */

    if ( inet_aton(szAddress, &servaddr.sin_addr) <= 0 ) {
	   print_error_and_exit("Setting server IP address");
    }

    
    /*  connect() to the remote echo server  

    if ( connect(conn_s, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ) {
	printf("ECHOCLNT: Error calling connect()\n");
	exit(EXIT_FAILURE);
    }

    */


    readFromFile(buffer, read_file);

    /*  Get string to echo from user  */

    // printf("Enter the string to echo: ");
    // fgets(buffer, MAX_LINE, stdin);

    // // printf("%lu\n", strlen(buffer));

    // int len = strlen(buffer);

    // if ((n = sendto(conn_s, buffer, len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) 
    // {
    //     print_error_and_exit("sending");
    // }  

    /*  Send string to echo server, and retrieve response  */

    // Writeline(conn_s, buffer, strlen(buffer));
    // Readline(conn_s, buffer, MAX_LINE-1);


    /*  Output echoed string  */

    printf("Echo response: %s", buffer);

    return EXIT_SUCCESS;
}


void readFromFile(char* buffer, char* read_file) {

    FILE *ptr;
    unsigned char read_buffer[1];

    ptr=fopen(read_file,"rb");                      // Reading file in binary

    /*  If error in reading file  */

    if (!ptr) { 
        print_error_and_exit("Unable to open file!");     
    }

    /*  Checking the file length if it doesn't exceed 1000 bytes  */

    fseek(ptr, 0, SEEK_END);
    int lengthOfFile = ftell(ptr);
    rewind(ptr);

    int count = 0;

    while (fread(read_buffer,sizeof(read_buffer),1,ptr) != 0) {
        // Saving the file buffer into data array
        buffer[count] = read_buffer[0];
        count += 1;
    }

    //printf("%d\n", lengthOfFile);
}


int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort, char **read_file, char** type, char** server_file, float* loss_probability, int* random_seed) {

    int n = 1;

    // Setting address and port from command line argument
    *szAddress = argv[1];
    *szPort = argv[2];
    *read_file = argv[3];
    *type = argv[4];
    *server_file = argv[5];
    *loss_probability = atof(argv[6]);
    *random_seed = atoi(argv[7]);

    return 0;
}

