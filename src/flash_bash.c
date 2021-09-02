/*
 * flash_bash.c
 *
 * Author: Cristian
 */

#include <stdio.h> //getchar,printf
#include <stdlib.h>
#include <unistd.h>
#include <string.h> //strlen
#include <errno.h>	//strerror

#include <wiringPi.h>     //wiringPi
#include <wiringSerial.h> //serialOpen

#define PIN7 7       //pin for glitching
#define PIN11 0      //pin for glitching

#define FLAT_LINE 1  //number definition
#define MORSE_CODE 2 //number definition
#define TIMED 1      //number definition
#define SERIAL 2     //number definition

#define SHOW_OUTPUT  //undefine to not show serial data on stdout


int main(void)
{
	int fd;					 //file descriptor
	char start_trigger[100]; //glitch start trigger
	char stop_trigger[100];	 //glitch stop trigger
	char buff[100];			 //buffer for incoming serial
	char device[100];		 //device name from /dev
	int i, n, n2, in;		 //for loops, for stringlength, for storing incoming serial
	int choice;				 //for choosing glitch style
	int baud;				 //stores baud rate
	int val = 1;			 //value from string comparison
	int count = 0;			 //simple count for how many times buff has been changed
	int attack_type = 0;	 //attack style
	int start_time = 0;		 //time for startup before glitching
	int stop_time = 0;		 //time for tool to stop glitching

	// Welcome
	printf("Welcome to Flash BASH!\n\n");
	printf("****CONFIGURATION*****\n\n");
	
	// Initialize WiringPi
	if (wiringPiSetup() == -1)
	{
		fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
		return 1;
	}

	// Get attack style selection from user
	printf("What type of attack? TIMED [1] or SERIAL [2]: ");
	scanf("%d", &choice);
	if ((choice != 1) && (choice != 2))
	{
		printf("Invalid selection, expect 1 or 2.\n");
		return 1;
	}
	else if (choice == 1)
	{
		attack_type = TIMED;
		printf("Attack style: TIMED\n\n");
	}
	else
	{
		attack_type = SERIAL;
		printf("Attack style: SERIAL\n\n");
	}

	// Go through set up for serial attack
	if (attack_type == SERIAL)
	{
		// Initialize wiringPi
		pinMode(PIN7, OUTPUT);    //sets pin mode to output
		digitalWrite(PIN7, HIGH); //initially sets it to high

		// Get Baud rate from user
		printf("What BAUD (9600, 115200, 38400, etc): ");
		scanf("%d", &baud);
		if ((baud <= 0) && (baud >= 250001))
		{
			printf("Invalid baud rate entry (expects 1 to 250000 baud).\n");
			return 1;
		}
		else
		{
			printf("Baud = %d\n\n", baud);
		}

		// Get serial device from user
		printf("Enter serial device descriptor:\n");
		scanf("%99s", device);
		printf("Device descriptor = `%s`\n\n", device);

		// Initialize serial communication
		if ((fd = serialOpen(device, baud)) == -1)
		{
			fprintf(stderr, "Unable to open serial devices: %s\n", strerror(errno));
			return 1;
		}

		// Obtain glitch trigger 1 from user
		printf("What phrase would you like to start glitching on? (no longer than 99 characters):\n");
		scanf("%99s", start_trigger);
		n = strlen(start_trigger);
		printf("Trigger string length = %d\n", n);
		printf("Glitch string = %s\n\n", start_trigger);

		// Obtain glitch trigger 2 from user
		printf("What phrase would you like to stop glitching on? (no longer than 99 characters):\n");
		scanf("%99s", stop_trigger);
		n2 = strlen(stop_trigger);
		printf("Stirng length = %d\n", n2);
		printf("Glitch string = %s\n", stop_trigger);

		// This buffer is filled by the serial output from target device.
		// The buffer is compared to start_trigger and stop_trigger. All 3 are of size 100.
		// Once buffer is equal to a trigger it either grounds the pin or releases

		// Fill in buffer once from serial
		for (i = 0; i < n; i++)
		{
			in = serialGetchar(fd);
			serialPutchar(fd, in);
			buff[i] = in;
		}
		buff[i] = '\0';
#ifdef SHOW_OUTPUT
		printf("%d: buff = %s\n", count, buff);
#endif
		val = strcmp(buff, start_trigger); //compare the buffer to the trigger

		// Start loop to compare strings
		// NOTE: This code could be made faster for efficiency and accuracy of
		//       triggering, however in basic glitching scenarios this is good enough.
		while (val != 0)
		{ //run loop until buff and trigger are the same
			in = serialGetchar(fd);
			fflush(stdout);
			// This loop 'shifts' the buffer down to make space for the incoming
			// byte, as we do a rolling comparison byte-by-byte.
			for (i = 0; i < n - 1; i++)
			{
				buff[i] = buff[i + 1];
			}
			buff[i] = in;
			buff[i + 1] = '\0';
			count++;
#ifdef SHOW_OUTPUT
			printf("%d: buff = %s\n", count, buff);
#endif
			// If start_trigger == buff, break out of this loop:
			val = strcmp(buff, start_trigger);
		}

		// Print success statement and trigger glitching
		printf("\n\n\n\n\nGLITCHING INITIATED!\n\n\n\n\n"); //buff and trigger ==
		digitalWrite(PIN7, LOW);  //tigger MUX
		val = 1;    //reset loop breaker
		count = 0;  //reset buffer count

		// Fill in buffer once from serial
		for (i = 0; i < n2; i++)
		{
			in = serialGetchar(fd);
			serialPutchar(fd, in);
			buff[i] = in;
		}
		buff[i] = '\0';
#ifdef SHOW_OUTPUT
		printf("%d: buff = %s\n", count, buff);
#endif
		val = strcmp(buff, stop_trigger);  //compare the buffer to the trigger

		// Start loop to compare strings
		while (val != 0)
		{
			//run loop until buff and trigger are the same
			in = serialGetchar(fd);
			fflush(stdout);
			for (i = 0; i < n2 - 1; i++)
			{
				buff[i] = buff[i + 1];
			}
			buff[i] = in;
			buff[i + 1] = '\0';
			count++;
#ifdef SHOW_OUTPUT
			printf("%d: buff = %s\n", count, buff);
#endif
			val = strcmp(buff, stop_trigger);
		}

		// Print success statement and trigger glitching
		printf("\n\n\n\nGLITCHED\n\n\n\n"); 	//buff and trigger ==
		digitalWrite(PIN7, HIGH);				//tigger MUX
		val = 1;
		count = 0;

		// Close serial port
		serialClose(fd);
	}
	else
	{
		// Set up timed glitch attack
		pinMode(PIN11, INPUT); //sets pin mode to INPUT

		// Get time variable from user
		printf("How long after boot to start glitching (1 - 300 secs): ");
		scanf("%d", &start_time);
		if ((start_time <= 0) && (start_time >= 301))
		{
			printf("Invalid time entry (expects 1 to 300 secs).\n");
			return 1;
		}
		else
		{
			printf("start_time = %d\n\n", start_time);
		}
		
		printf("How long after glitching has started to stop (1 - 300 secs): ");
		scanf("%d", &stop_time);
		if ((stop_time <= 0) && (stop_time >= 301))
		{
			printf("Invalid time entry (expects 1 to 300 secs).\n");
			return 1;
		}
		else
		{
			printf("stop_time = %d\n\n", stop_time);
		}

		printf("Please turn on target device now!\n\n");

		while (digitalRead(PIN11) != 1)
		{
			// Wait for device to turn on, as PIN11 is sensing device VCC.
		}

		// NOTE: Percision of this is not optimal as OS may not give us accurate timing,
		//		 users who need more precision should update this.		
		sleep(start_time);
		
		digitalWrite(PIN7, LOW); //tigger MUX
		printf("\n\n\n\n\nGLITCHING INITIATED!\n\n\n\n\n"); //buff and trigger ==

		// NOTE: See above (lack of) precision note.
		sleep(stop_time);

		digitalWrite(PIN7, HIGH);  //tigger MUX
		printf("\n\n\n\nGLITCHING CEASED\n\n\n\n");  //time elapsed
	}

	return 0;
}
