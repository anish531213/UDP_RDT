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
#include <math.h>
#include <netdb.h>
#include "sendlib.c"

/*  Global constants  */

#define MAX_LINE           (200000)
#define SEGMENT_SIZE       (20)
#define HEADER_SIZE        (6)
#define TIMEOUT            (50000)       //50 millisecond


/*  Function declarations  */

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort, char **read_file, char** type, char** server_file, float* loss_probability, int* random_seed);

int readFromFile(char* buffer, char* read_file);

char* make_pkt(char* seq, int* payload_size, char* pkt_serial, char* data);

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

    

    int     len_servaddr;            /*  servaddr size.            */
    int     n;                       
    int     len_of_file;             /*  length of file to send.   */
    int     no_of_udp_segemnts;      /*  total no. of segment.     */
    int     seq_num;                 /*  holds sequence number.    */
    char    state;                   /*  holds state.              */
    int     i;
    int     payload_size;            /*  payload size              */
    char    pkt_serial;              /*  holds packet serial.      */
    char    ack;                     /*  holds ack                 */
    int     len;                     /*  holds len                 */
    char*   payload;                 /*  holds payload             */
 

    /*  If all command line are not present. */

    if (argc != 8) {
        print_error_and_exit("Invalid arguments!");
    }

    /*  Get command line arguments  */

    ParseCmdLine(argc, argv, &szAddress, &szPort, &read_file, &type, &server_file, &loss_probability, &random_seed);

    /*  error if loss probability is not between 0 and 1. */

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


    /*  reading from the file to the buffer.  */

    len_of_file = readFromFile(buffer, read_file);

    /*  no. of segemnts to be constructed. */

    no_of_udp_segemnts = (int) ceil((1.0*len_of_file) / SEGMENT_SIZE);

    /*  Inital state. */

    state = '0';
    i = -1;

    /*  Setting timer. */

    struct timeval timer;
    
    /*  setting description set. */
       
    fd_set read_fds;
    FD_ZERO(&read_fds);

    FD_SET(conn_s, &read_fds);

    /*  while ack received all segemnts and arguments not sent. */

    while (i < no_of_udp_segemnts) {

        /*  setting up payload size and pkt serial for last packet. */

        if (i == no_of_udp_segemnts-1) {
            payload_size = len_of_file-i*SEGMENT_SIZE;
            pkt_serial = '1';
        }

        /*. setting up payload size and packet serial for other packets. */

        else {
            payload_size = SEGMENT_SIZE;
            pkt_serial = '0';
        }

        /*  creating payload for command line arguments. */

        if (i < 0) {
            int len_serv_file = strlen(server_file);

            pkt_serial = 'i';

            payload_size = len_serv_file+5;
            payload = (char*) malloc(payload_size);
            memcpy(payload, type, 1);
            memcpy(payload+1, &len_serv_file, 4);
            memcpy(payload+5, server_file, len_serv_file);
        }

        /*  creating payload for file data. */

        else {
            payload = (char*) malloc(payload_size);
            memcpy(payload, buffer+i*SEGMENT_SIZE, payload_size);
        }

        /*  creating packets  */
        
        char* sndpkt = make_pkt(&state, &payload_size, &pkt_serial, payload);

        /*  packet size. */

        len = payload_size+HEADER_SIZE;

        printf("Sending Packet with seq %c\n", state);
        
        /*  Unreliable send using lossy sendto. */

        if ((n = lossy_sendto(loss_probability, random_seed, conn_s, sndpkt, len, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0) 
        {
            print_error_and_exit("sending");
        } 

        /*   
            TIMER.  

        */

        timer.tv_sec = 0;
        timer.tv_usec = TIMEOUT;

        int status;
        status = select(conn_s+1, &read_fds, NULL, NULL, &timer);
        if (status == -1){
            print_error_and_exit("select ");

        /* Timeout : if ack not received for the packet */

        } else if(status == 0){
            printf("Timeout! Retrasmitting...\n");
            FD_SET(conn_s, &read_fds);
            free(payload);
            free(sndpkt);
            continue;   /* If timeout continue again. */
        }

        /*  ------------------  */

        /*  recvfrom for receiving ack. */ 

        if ((n = recvfrom(conn_s, &ack, 1, 0, 0, 0)) < 0) 
        {
            print_error_and_exit("error receiving ACK");
        } 

        /*  If the status packet get succes or failure  */

        // printf("%c\n", ack);
        if (ack == 's' || ack == 'f') {
            if (ack == 's')
                printf("Format Success\n");
            else 
                printf("Format Error\n");
            i++;
        }

        /*  If the same ack is back for the packet sent  */

        if (ack == state) {
            printf("ACK%c received \n", ack);
            if (state == '0')
                state = '1';
            else
                state = '0';
            i++;   // Continue to next packet
        }   
         
        free(payload);  /* free payload. */
        free(sndpkt);   /* free packet. */
        
    }

    return EXIT_SUCCESS;
}

/*  Make packet function for making packet from header info and payload. */

char* make_pkt(char* seq, int* payload_size, char* pkt_serial, char* data) {
    char* packet = (char*) malloc(SEGMENT_SIZE+HEADER_SIZE);
    memcpy(packet, seq, 1);
    memcpy(packet+1, payload_size, 4);
    memcpy(packet+5, pkt_serial, 1);
    memcpy(packet+6, data, *payload_size);

    return packet;
}

/*  FUnction to read file into a buffer. */

int readFromFile(char* buffer, char* read_file) {

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

    return lengthOfFile;
}


/*  function to parse command line arguments. */

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

