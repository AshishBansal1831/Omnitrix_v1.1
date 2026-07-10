/*
 * cli_.h
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#ifndef INC_CLI__H_
#define INC_CLI__H_

#include <stdint.h>

#include "cpp_mutex.hpp"

class InterfaceIO;
class CmdLineInterface;
typedef void (*cmd_handler)(CmdLineInterface *cmd_interface, const char*);

constexpr uint8_t Max_Cmd_Len = 50;

enum SpecialChar_t
{
	BACK_SPACE  = 0x7F,
	ENTER       = 0x0D,
	CAR_RET     = '\r',
	NULL_CHAR	= '\0'
};

struct CliStruct
{
    const char* command;
    cmd_handler handler;
    const char* description;
};

class CmdLineInterface
{
	InterfaceIO *IO;
	const CliStruct *Command_Handler;
	const uint8_t TotalCmds;
	static FreeRtosMutex Mutex;

	char ReceivedCmd[Max_Cmd_Len];
	uint8_t CmdLen = 0;

	void get_cmd_ip();

public :

	CmdLineInterface (const CliStruct *Cmd, const uint8_t Total, InterfaceIO *paramIO)
		: IO(paramIO), Command_Handler(Cmd), TotalCmds(Total)  {}
	void init();
	void command_handler();
	void print(const char *ptr);
	void printn(const char *ptr, const size_t len);
	void printf(const char *format, ...);
};


#endif /* INC_CLI__H_ */
