/*
 * cli_.h
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#ifndef INC_CLI__H_
#define INC_CLI__H_

#include <stdint.h>

#include "InterfaceIO.h"
#include "cpp_mutex.hpp"

class CmdLineInterface;
typedef void (*cmd_handler_t)(InterfaceIO *, const char*);

constexpr uint8_t Max_Cmd_Len = 50;

struct CliStruct_t
{
    const char* command;
    cmd_handler_t handler;
    const char* description;
};

class CmdLineInterface
{
	InterfaceIO *IO;
	const CliStruct_t *Command_Handler;
	static FreeRtosMutex Mutex;

	char ReceivedCmd[Max_Cmd_Len];
	uint8_t CmdLen = 0;
	const uint8_t TotalCmds;

public :

	CmdLineInterface (const CliStruct_t *Cmd, const uint8_t Total, InterfaceIO *paramIO)
		: IO(paramIO), Command_Handler(Cmd), TotalCmds(Total)  {}
	void init();
	void command_handler();
};


#endif /* INC_CLI__H_ */
