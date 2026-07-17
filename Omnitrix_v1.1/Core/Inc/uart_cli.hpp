/*
 * uart_cli.h
 *
 *  Created on: May 31, 2025
 *      Author: Ashish Bansal
 */

#ifndef INC_UART_CLI_HPP_
#define INC_UART_CLI_HPP_

#define TOTAL_TASKS_TO_WATCH (20)
#define MAX_CMD_LEN (100)

//#define BACK_SPACE  0x7F
//#define ENTER       0x0D
//#define CAR_RET     '\r'

#define TOTAL_COMMANDS  ( sizeof(Command_Handlers) / sizeof(Command_Handlers[0]) )

typedef void (*cmd_handler)(const char*);

struct CliStruct
{
    const char* command;
    cmd_handler handler;
    const char* description;
};

struct ParamConfig
{
    uint8_t* data;
    size_t len;
};


extern "C" void Cli_Task(void *Arguments);
void cli_task(void *Arguments);

#endif /* INC_UART_CLI_HPP_ */
