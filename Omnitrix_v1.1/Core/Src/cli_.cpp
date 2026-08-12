/*
 * cli_.cpp
 *
 *  Created on: 11-Jul-2026
 *      Author: ashish
 */
#include "cstring"
#include "InterfaceIO.h"
#include "cli_.h"

void CmdLineInterface::init()
{
	Mutex.init();
}

void CmdLineInterface::command_handler()
{
	while(1)
	{
		CmdLen = IO->get_string_input(ReceivedCmd, sizeof(ReceivedCmd));

	    if(CmdLen == 0) // <! User just pressed enter without any input
	    {
	        continue;
	    }

	    uint8_t Iter = 0;

	    for (Iter = 0; Iter < TotalCmds; ++Iter)
	    {
	    	const CliStruct_t *tempPtr = &Command_Handler[Iter];
	        const char *cmd = tempPtr->command;
	        const size_t command_len = strlen(cmd);

	        if (strncasecmp(ReceivedCmd, cmd , command_len) == 0)
	        {
	            if (tempPtr->handler != NULL)
	            {
	                const char *Arguments =
	                		(ReceivedCmd[command_len] == NULL_CHAR) ?
	                				NULL : strchr(ReceivedCmd, ' ');
	                tempPtr->handler(IO, Arguments);
	            }
	            else
	            {
	            	IO->print("Not handled\r\n");
	            }
	            break;
	        }
	    }

	    if (Iter == TotalCmds)
	    {
	    	IO->printf("'%s' cmd not found\r\n", ReceivedCmd);
	    }

	}
}

