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

void CmdLineInterface::get_cmd_ip()
{
    char received_char = 0;
    size_t index = 0;

    IO->print("\r>>>> ");

    do
    {
    	received_char = (char)IO->receive_byte();

        switch (received_char)
        {
            case BACK_SPACE:
            	IO->write_byte(' ');
            	IO->write_byte(BACK_SPACE);
            	IO->write_byte(BACK_SPACE);
            	ReceivedCmd[index--] = NULL_CHAR;
                break;

            case CAR_RET:
            case ENTER:
            	ReceivedCmd[index] = NULL_CHAR;
            	IO->print("\r\n");
                break;

            default:
            	IO->write_byte(received_char);
                ReceivedCmd[index++] = received_char;
                break;
        }

    } while (received_char != ENTER && received_char != CAR_RET);

    CmdLen = index;
}

void CmdLineInterface::command_handler()
{
	while(1)
	{
		get_cmd_ip();

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

