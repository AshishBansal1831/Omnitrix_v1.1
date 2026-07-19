/*
 * uart_cli.c
 *
 *  Created on: May 30, 2025
 *      Author: Ashish Bansal
 */

/* -- Standard Library -- */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <settings_task.hpp>
#include <stdlib.h>

/* -- STM32 Library -- */
#include "usart.h"
#include "stm32f4xx_ll_usart.h"

/* -- FreeRTOS Library -- */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "SEGGER_RTT.h"

/* -- User Library -- */
#include "uart_cli.hpp"

#include "InterfaceIO.h"
#include "cli_.h"

/* -- Extern Variables -- */
extern UART_HandleTypeDef hUSART1;
volatile QueueHandle_t User_Uart_Queue;
volatile QueueHandle_t UsbCdcRxQueue;
extern volatile QueueHandle_t SettingsQueue;
extern TaskHandle_t CpuMonitorHandler;

/* -- Function Declarations -- */

/* Extracts the first integer found in `src_string`, returns it, and sets `length` to number of characters consumed. */
long extract_number_from_string(const char *src_string, size_t *length);

/* Print a list of all registered CLI commands and their descriptions. */
static inline void list_commands(InterfaceIO *Io, const char*);

/* Display CPU usage and free stack of all tasks, either once or continuously until ENTER is pressed. */
void cpu_monitor(InterfaceIO *Io, const char*);

/* Wifi Handler */
void wifi_cmd_handler(InterfaceIO *IO, const char* arguments);
void wifi_handler(InterfaceIO *IO, const char* arguments);

/* Mqtt Handler */
void mqtt_cmd_handler(InterfaceIO *IO, const char* arguments);
void mqtt_handler(InterfaceIO *IO, const char* arguments);

/* clear screen */
void clear_screen(InterfaceIO *IO, const char* arguments);

/* Handle UART Settings */
void uart_settings(InterfaceIO *Io, const char*);

/* -- Global Variables -- */

const static CliStruct_t Command_Handlers[] = {
    { .command = "list",           .handler = list_commands,  .description = "List all commands" },
    { .command = "uart",           .handler = uart_settings,  .description = "Do uart settings from here" },
    { .command = "cpu_monitor",    .handler = cpu_monitor,    .description = "Prints CPU Stats" },
	{ .command = "wifi" , 		   .handler = wifi_handler,	  .description = "Handler wifi settings" },
	{ .command = "mqtt" , 		   .handler = mqtt_handler,	  .description = "Handler mqtt settings" },
    { .command = "update",         .handler = NULL,           .description = "Should Put Device in update mode" },
	{ .command = "clear",          .handler = clear_screen,   .description = "clears screen" },
};

static const char* CPU_USAGE_Commands[] = { "once", "continue" };

/* -------------------------------------------------------------------------- */
/*                          Static Inline Function Group                      */
/* -------------------------------------------------------------------------- */


/* -- List All Registered Commands -- */
/* Iterates through `Command_Handlers` and prints each command name and description. */

static inline void list_commands(InterfaceIO *Io, const char*)
{
    for (uint8_t iter = 0; iter < TOTAL_COMMANDS ; ++iter)
    {
        Io->printf("%d. %-10s: %s %s\r\n",
			   iter + 1,
			   Command_Handlers[iter].command,
			   Command_Handlers[iter].description,
			   Command_Handlers[iter].handler == NULL ? "(Not Implemented)" : "");
    }
}

/* -------------------------------------------------------------------------- */
/*                              CLI Logic Functions                           */
/* -------------------------------------------------------------------------- */

/* -- CLI Task -- */
/* FreeRTOS task that waits for a full command via UART interrupts and then dispatches it to the command handler. */
void cli_task(void *Arguments)
{
    // The safe, modern C++ way to cast down from a void pointer
    InterfaceIO *Obj = static_cast<InterfaceIO *>(Arguments);
    // Quick defensive check to ensure you didn't pass a null pointer
    if (Obj == nullptr) {
        vTaskDelete(NULL);
    }

    CmdLineInterface cli(Command_Handlers, TOTAL_COMMANDS, Obj);

    cli.command_handler();
}

/* -- Extract Number from String -- */
/* Scans `src_string` for the first digit sequence, converts it to a long, and returns it.
 * Sets `length` to number of chars consumed. */
long extract_number_from_string(const char *src_string, size_t *length)
{
    long number = __LONG_MAX__;
    const char *follower = src_string;
    char *endpointer = NULL;

    while (*follower != 0)
    {
        follower++;
        if (isdigit((int)*follower))
        {
            number = strtol(follower, &endpointer, 10);
            *length = endpointer - src_string;
            break;
        }
    }

    return number;
}


/* -- CPU Monitor Command -- */
/* Retrieves runtime stats for each FreeRTOS task and prints CPU usage and free stack.
 * If "continue" is passed, updates in place until ENTER is pressed. */
void cpu_monitor(InterfaceIO *IO, const char* arguments)
{
    uint8_t command_type = 0;
    if (arguments == NULL)
    {
    	IO->print("Please pass arguments\r\n");
        return;
    }

    uint8_t index = 0;

    while(arguments[index] == ' ')
    {
        index++;
    }

    if (strcasecmp(CPU_USAGE_Commands[0], arguments + index) == 0)
    {
        command_type = 1;  // once
    }
    else if (strcasecmp(CPU_USAGE_Commands[1], arguments + index) == 0)
    {
    	IO->print("Press Enter to stop\r\n");
        command_type = 2;  // continue
    }
    else
    {
    	IO->print("Invalid argument. Use \"once\" or \"continue\"\r\n");
        return;
    }

    uint32_t total_run_time = 0;
    uint8_t  total_tasks = 0;
    BaseType_t TaskIter = 0;

    char received_char = 0;
    TaskStatus_t task_status[TOTAL_TASKS_TO_WATCH] = { 0 };

    IO->printf("%-10s | %-6s | %-17s |\r\n", "Task", "CPU%", "Free Stack (words)");
    IO->print("------------------------------------------------------\r\n");

    if (command_type == 2) // continuous
    {
    	IO->print("\033[?25l"); // Hide cursor
    }

    // Fixed refresh rate control (e.g., update every 500 ms)
	const TickType_t xRefreshPeriod = pdMS_TO_TICKS(500);
	TickType_t xLastWakeTime = xTaskGetTickCount();

	do
	{
		TickType_t xCurrentTime = xTaskGetTickCount();

		// ONLY calculate and print if the 500ms refresh period has elapsed
		if ((xCurrentTime - xLastWakeTime) >= xRefreshPeriod)
		{
			xLastWakeTime = xCurrentTime;

			total_tasks = uxTaskGetSystemState(task_status, TOTAL_TASKS_TO_WATCH, &total_run_time);

			for (TaskIter = 0; TaskIter < total_tasks; ++TaskIter)
			{
				if (total_run_time > 0)
				{
					const float cpu_percentage = (task_status[TaskIter].ulRunTimeCounter * 100.0f) / total_run_time;
					IO->printf("%-10s | %6.2f%% | %18d |\r\n",
									task_status[TaskIter].pcTaskName,
									cpu_percentage,
									task_status[TaskIter].usStackHighWaterMark);
				}
				else
				{
					IO->printf("%-10s | --.--%% | %18d |\r\n",
									task_status[TaskIter].pcTaskName,
									task_status[TaskIter].usStackHighWaterMark);
				}
			}

			if (command_type == 2) // continuous
			{
				// Move cursor up exactly by the number of tasks printed to prepare for overwrite
				IO->printf("\033[%dA", total_tasks);
			}
		}

		// Continuous Mode: Poll for exit key with a small, CPU-friendly sleep window
		if (command_type == 2)
		{
			// Blocks the task completely for 20ms waiting for input, letting the CPU rest.
			// 20ms gives instant UI responsiveness while dropping CPU utilization close to 0%.
			if (IO->receive_byte((uint8_t*)&received_char, pdMS_TO_TICKS(20)) == 1)
			{
				if (received_char == ENTER || received_char == CAR_RET)
				{
					break;
				}
			}
		}

	} while (command_type == 2);

	if (command_type == 2) // continuous
	{
		// Clean up the terminal space: move past the table block and restore cursor
		IO->printf("\033[%dB", total_tasks);
		IO->print("\033[?25h\r\n");
	}
}

void wifi_handler(InterfaceIO *IO, const char* arguments)
{

}

void mqtt_cmd_handler(InterfaceIO *IO, const char* arguments)
{

}
void mqtt_handler(InterfaceIO *IO, const char* arguments)
{

}

void clear_screen(InterfaceIO *IO, const char* arguments)
{
    (void)arguments;

    if (IO != nullptr)
    {
        // \033[2J  -> Clears the entire screen
        // \033[H   -> Moves the cursor to the home position (top-left corner)
        IO->print("\033[2J\033[H");
    }
}

static inline uint8_t is_baudrate_valid(const uint32_t BaudRate)
{
    uint8_t valid = 0;

    switch(BaudRate)
    {
        case 1200:
        case 4800:
        case 9600:
        case 19200:
        case 115200:
        case 460800:
        case 920600:
            valid = 1;
            break;
        default:
        	break;
    }

    return valid;
}

void uart_settings(InterfaceIO *IO, const char* Arguments)
{
    if(NULL == Arguments)
    {
        IO->print("Please provide BaudRate\r\n");
        return;
    }
    char* endptr = NULL;

    uint32_t NewBaudRate = strtol(Arguments, &endptr, 10);

    if(0 == NewBaudRate || is_baudrate_valid(NewBaudRate) != 1)
    {
    	IO->print("Please provide Valid BaudRate\r\n");
        return;
    }

    Settings uart_settings = { .config_id = Config_t::UART_CONFIG};
    memcpy(&uart_settings.Buffer, &NewBaudRate, sizeof(NewBaudRate ));

//    xQueueSend(SettingsQueue, &uart_settings, portMAX_DELAY);
}

/* -------------------------------------------------------------------------- */
/*                            UART IRQ Handler                                */
/* -------------------------------------------------------------------------- */

/* -- USART1 Interrupt Service Routine -- */
/* Called when a new byte arrives on USART1. Reads the byte and places it into the CLI queue. */
extern "C" void USART1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if RXNE flag is active and enabled
    if (LL_USART_IsActiveFlag_RXNE(USART1) && LL_USART_IsEnabledIT_RXNE(USART1))
    {
        // Check for hardware line errors (Overrun, Noise, Framing, Parity)
        if (LL_USART_IsActiveFlag_ORE(USART1) ||
            LL_USART_IsActiveFlag_FE(USART1)  ||
            LL_USART_IsActiveFlag_NE(USART1))
        {
            // Clear the error state by reading the data register and discarding the corrupt byte
            volatile uint8_t discard = LL_USART_ReceiveData8(USART1);
            (void)discard; // Silence unused variable warning
        }
        else
        {
            // Clean read of valid data
            uint8_t received = LL_USART_ReceiveData8(USART1);

            // Send to queue ONLY if the scheduler is running and queue is valid
            if (User_Uart_Queue != NULL)
            {
                xQueueSendFromISR(User_Uart_Queue, &received, &xHigherPriorityTaskWoken);
            }
        }
    }

    // Context switch if a higher priority task was awoken by the new byte
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
