/*

  HELPER.H
  ========
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net

  Interface to socket helper functions. 

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/


#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP


#include <unistd.h>             /*  for ssize_t data type  */

#include <sys/socket.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



#define LISTENQ        (1024)   /*  Backlog for listen()   */


/*  Function declarations  */

int convert(char* file_name, char* char_type, int* data_array, int count, int start);


/* 	Convert Function to convert and print Type 0 format types  */

void convertFirstTypes(FILE* fp, unsigned char type, int amount, int numbers[]) {

	/*  Writing type to file according to the client sent type  */

	if (type == 1) {
		//printf("Type0\n");
		fprintf(fp, "%c", type);
	} else {
		//printf("Type1\n");
		fprintf(fp, "%c", type+1);
	}
	
	char s;												// char declaration
	int k;												// looping variable declaration
	
	/*  print three byte amount to file in aciia  */

	for (k=2; k>=0; k--) {						
		s = amount/pow(10, k)+'0';						
		fprintf(fp, "%c", s);							// printing each byte to file
	}

	/*  directly printing number to a file as string  */

	for (k=0; k<amount-1; k++) {
		fprintf(fp, "%d,", numbers[k]);					// Printing number with ',' ascii
	}

	fprintf(fp, "%d", numbers[amount-1]);				// Printing end number without ','
	
}

/* 	Convert Function to convert and print Type 1 format types  */

void convertSecondTypes(FILE* fp, unsigned char type, int amount, int numbers[]) {	
	
	/*  Writing type to file according to the client sent type  */
	
	if (type == 0) {
		//printf("Type1\n");
		fprintf(fp, "%c", type);
	} else {
		//printf("Type0\n");
		fprintf(fp, "%c", type-1);
	}

	fwrite(&amount, 1, 1, fp);							// Using fwrite to write 1 byte of integer

	/*  Introducing flip_num array to address byte flip issue while writing to file  */

	int flip_num[amount];

	/*  Writing 2 bytes for each number after flipping their bytes  */

	for (int k=0; k<amount; k++) {
		flip_num[k] = (numbers[k]>>8) | (numbers[k]<<8);// Flipping bytes
		fwrite(&flip_num[k] , 2, 1, fp);				// Writing 2 bytes of integer
	}
	
}


/*  Convert function to convert types  */

int convert(char* file_name, char* char_type, int* data_array, int count, int start) {

	int main_type;									// client type declaration
	int i;											// position counter declaration
	int type;										// format type declaration
	int amount;										// amount declaration
	FILE *fp;										// File pointer

	main_type = atoi(char_type);					// taking integer of type
	i=start;										// Buffer start position
	type = 5;

	remove(file_name);								// Removing the writing fike										// Default format type

	fp = fopen(file_name, "wb");					// Opening the file in write binary


	/* 	Checking if the first character of file starts with type format  */

	if (data_array[i] != 0 && data_array[i] != 1) {
		return error_handler(fp, "First byte format is incorrect!", file_name);
	}


	/* 	Parsing and performing operation on buffer data from client  */

	while (i < count) {

		/*  Operation for no type  */
		
		if (type == 5) {							// if no type
			type = data_array[i];					// gets type from data array					
			i += 1;									// incrementing buffer position

		/*  Operations for type 0  */

		} else if (type == 0) {
			//printf("Type 0 ");
			amount = data_array[i];					// gets amount
			printf("%d ", amount);					// printing amount
			
			int numbers[amount];					// number array for storing 
													// numbers1 to numberN
			i += 1;									// incrementing buffer position

			/*  Getting N(amount) numbers in a loop  */

			for (int j=0; j<amount; j++) {
				// Concatinating two single byte data
				int result = (data_array[i] << 8) | data_array[i+1];
				// print number with ',' or without comma
				if (j == amount-1) {
					printf("%d", result);			// printing number
				} else {
					printf("%d,", result);			// printing number
				}
				
				numbers[j] = result;				// adding number to number array
				i += 2;								// incrementing buffer position
			}

			printf("\n");							// printing new line
			
			/**	
			 *
			 *	Passing amount and numbers in respective function  
			 *  	
			 *	If the client sends type 3 or 1, it converts the data to first type
			 *	else, it converts the data into second type
			 *
			 */

			if (main_type == 3 || main_type == 1) {	
				convertFirstTypes(fp, type, amount, numbers);
			} else {
				convertSecondTypes(fp, type, amount, numbers);
			}

			type = 5;								// setting type as default
		
		/*  Operations for type 1  */

		} else if (type == 1) {
			// printf("\nType 1 ");
			//char amount[3];

			/*  Getting the three bytes amount which are sent as character  */

			amount = 100*(data_array[i]-48) + 10*(data_array[i+1]-48) + (data_array[i+2]-48);

			printf("%d ", amount);					// print amount
			
			int numbers[amount];					// number arary for storing numbers
			i += 3;									// incremnting buffer position

			/*  Getting N(amount) numbers in a loop  */
			
			for (int j=0; j<amount; j++) {
				int result = 0;

				/*  
					Getting each character of ascii number one by one 
					until 0, 1 or null char found 	  					
				*/

				while (data_array[i] != 0 && data_array[i] != 1  && data_array[i] != '\0') {

					if (data_array[i] == 44) {		// If ascii comma is found
						i += 1;						// incrementing buffer position
						break;
					}

					// Calculating each number from ascii character
					result = result*10 + (data_array[i]-48); 
					i += 1;							// incrementing buffer position

					/*  Sending error if result exceed the min integer size  */

					if (result > 65535) {
						return error_handler(fp, "Invalid Number for type 1!", file_name);
					}

				} 

				/*  If N numbers are not read until next type or file terminates,
					Then there is format error for type 1  */

				if ((j<amount-1) && (data_array[i] == 0 
					|| data_array[i] == 1  || data_array[i] == '\0')) {

					return error_handler(fp, "Type 1 format error!", file_name);

				}

				// print number with ',' or without comma
				if (j == amount-1) {
					printf("%d", result);			// printing number
				} else {
					printf("%d,", result);			// printing number
				}
				
				numbers[j] = result;				// adding number to number array
			}

			printf("\n");							// printing new line

			/*  If ending character found before reading n characters  */

			/**	
			 *
			 *	Passing amount and numbers in respective function  
			 *  	
			 *	If the client sends type 3 or 2, it converts the data to second type
			 *	else, it converts the data into first type
			 *
			 */

			if (main_type == 3 || main_type == 2) {
				convertSecondTypes(fp, type, amount, numbers);
			} else {
				convertFirstTypes(fp, type, amount, numbers);
			}

			type = 5;								// setting type as default

		/*  If type format is wrong or type is not 0 or 1 or default */

		} else {

			return error_handler(fp, "Type error!", file_name);
		
		}

	}

	fclose(fp);										// Closing the file

	return 1;										// Return success code 1 if no error

}


/* 	Error handler function to print error in server  */

int error_handler(FILE *fp, char* message, char* file_name) {
	printf("%s\n", message);						// Printing the error is server
	fclose(fp);										// Closing the writing file
	remove(file_name);								// Removing the writing fike
	return 0;										// Returns error code
}







#endif  /*  PG_SOCK_HELP  */

