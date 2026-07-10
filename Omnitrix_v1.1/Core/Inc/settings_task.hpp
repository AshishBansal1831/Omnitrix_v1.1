/*
 * settings_task.h
 *
 *  Created on: Jun 1, 2025
 *      Author: Ashish Bansal
 */

#ifndef _SETTINGS_TASK_H_
#define _SETTINGS_TASK_H_

#include <stdint.h>
#include <limits.h>

enum class Config_t : uint8_t
{
	UART_CONFIG,
	WIFI_SETTINGS,
	MQTT_SETTINGS,
} ;

enum class WifiConfig_t : uint8_t
{
	ADD_WIFI,
	REMOVE_WIFI,
	CHANGE_PASSWORD,
	MANUAL_CONN
};

enum class MqttConfig_t : uint8_t
{
	ADD_BROKER,
	REMOVE_BROKER,
	MANUAL_CONN
};

struct Settings
{
	Config_t config_id;
    uint8_t Buffer[UINT8_MAX]; // this will contain the sub configuration such as WifiConfig or Mqtt Config
    uint8_t CmdSrc;
} ;

//extern "C" void setting_task(void*);

#endif /* _SETTINGS_TASK_H_ */
