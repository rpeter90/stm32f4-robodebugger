/*
 * debug.h
 *
 *  Created on: 2013.11.14.
 *      Author: Rimóczi Péter
 */
#include <stm32f4xx.h>

#ifndef DEBUG_H_
#define DEBUG_H_

#define DEBUG_COMMAND_NUM 1				// Commands number
#define DEBUG_ERROR_NUM 5				// Errors number
#define DEBUG_INCOMING_BUFFER_SIZE 41 	// Buffer for incoming messages
#define DEBUG_PROCESS_BUFFER_SIZE 105 	// Buffer for processing messages (must be larger than incoming buffer)
#define DEBUG_BLACKBOX_BUFFER_SIZE 5001 // Buffer for blackbox
#define DEBUG_MAX_COMMAND_ARGS 8 		// Maximum number of arguments per command
#define DEBUG_MAX_ARG_LENGTH 13 		// Maximum length of one argument
#define DEBUG_MAX_ERROR_LENGTH 41 		// Error message maximum length

// The required structure for the commands
typedef struct Dcom{
	char name[DEBUG_MAX_ARG_LENGTH];
	void (*func)();
} Dcom;

// The structure for the error messages
typedef struct Derr{
	char msg[DEBUG_MAX_ERROR_LENGTH];
	int errornum;
} Derr;

// Main function predeclaration
void Debug_Configure(uint8_t flag);							// Configure debug mode
void Debug_Bbx_Push(void);									// Push out Debug black box container
void USART3_IRQHandler(void);								// Handles incoming messages
void USART3_puts(volatile char *s);							// Sends out messages
void Debug_Handle(void); 									// Handles the incoming data
void Debug_Print(volatile char *format, ...); 				// Sending out messages like printf
void Debug_Error(int ec); 									// Error handling

// Command handler function predeclaration
void Debug_COM_Ping(char com_args[DEBUG_MAX_COMMAND_ARGS][DEBUG_MAX_ARG_LENGTH]);

// COMMAND LIST predeclaration
Dcom DEBUG_COMMANDS[DEBUG_COMMAND_NUM];

// ERROR LIST predeclaration
Derr DEBUG_ERRORS[DEBUG_ERROR_NUM];

#endif /* DEBUG_H_ */
