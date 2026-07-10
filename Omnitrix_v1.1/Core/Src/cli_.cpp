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

void CmdLineInterface::print(const char *ptr)
{
	IO->write_buffer((uint8_t*)ptr, strlen(ptr));
}

void CmdLineInterface::printn(const char *ptr, const size_t len)
{
	IO->write_buffer((uint8_t*)ptr, len);
}

void CmdLineInterface::get_cmd_ip()
{
    char received_char = 0;
    size_t index = 0;

    print("\r>>>> ");

    do
    {
    	received_char = (char)IO->receive_byte();

        switch (received_char)
        {
            case BACK_SPACE:
            	IO->write_byte(BACK_SPACE);
            	ReceivedCmd[index--] = NULL_CHAR;
                break;

            case ENTER:
            	ReceivedCmd[index] = NULL_CHAR;
                print("\r\n");
                break;

            default:
            	IO->write_byte(received_char);
                ReceivedCmd[index++] = received_char;
                break;
        }

    } while (received_char != ENTER);

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
	    	const CliStruct *tempPtr = &Command_Handler[Iter];
	        const char *cmd = tempPtr->command;

	        if (strncasecmp(ReceivedCmd, cmd , CmdLen) == 0)
	        {
	            if (tempPtr->handler != NULL)
	            {
	                const char *Arguments = (ReceivedCmd[CmdLen] == NULL_CHAR) ? NULL : strchr(ReceivedCmd, ' ');
	                tempPtr->handler(Arguments);
	            }
	            else
	            {
	                print("Not handled\r\n");
	            }
	            return;
	        }
	    }

	    if (Iter == TotalCmds)
	    {
	        printf("%s cmd not found\r\n", ReceivedCmd);
	    }

	}
}

