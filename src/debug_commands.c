/*
 * debug_commands.c
 *
 *	Debug command line command functions
 *
 *  Created on: 2013.11.14.
 *      Author: Rimóczi Péter
 */
#include <debug.h>
#include <stm32f4xx_conf.h>

// COMMAND LIST
Dcom DEBUG_COMMANDS[]={
		{
			"PING",
			&Debug_COM_Ping
		}
};

// ERROR LIST
Derr DEBUG_ERRORS[]={
		{
			"ERROR 0 - P BUFFER OVERLOADED\n\r",
			0
		},
		{
			"ERROR 1 - I BUFFER OVERLOADED\n\r",
			1
		},
		{
			"ERROR 2 - UNKNOWN COMMAND\n\r",
			2
		},
		{
			"ERROR 3 - TOO MANY ARGUMENTS\n\r",
			3
		},
		{
			"ERROR 4 - ARGUMENT TOO LONG\n\r",
			4
		}
};


/*
 *  --------- COMMAND HANDLER FUNCTIONS ---------
 */


/**
  * @brief  Returns a PONG if PING was received.
  * @param  n/a: n/a.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_COM_Ping(char com_args[DEBUG_MAX_COMMAND_ARGS][DEBUG_MAX_ARG_LENGTH])
{
	Debug_Print("c","PONG\n\r");
}

