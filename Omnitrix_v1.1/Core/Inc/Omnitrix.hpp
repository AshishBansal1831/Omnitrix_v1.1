/*
 * Omnitrix.hpp
 *
 *  Created on: 17-Jun-2026
 *      Author: ashish
 */

#ifndef INC_OMNITRIX_HPP_
#define INC_OMNITRIX_HPP_


enum class States
{
	CHANGE_TO_NORMAL_SCREEN,
	IDLE, // Show the default Omnitrix Symbol
	ACTIVATING, // Display the animation where omnitrix synbol changes to the Alien scheoulette
	DISPLAY_ALIENS, // Display aliens based on the Selection from Left Right button simulating a rotation
	HANLDE_INPUT,
	HANDLE_ALIENS,  // Do alien specific task
	DISCHARGING,    // Discharging (Red and green animation)
	RECHARGING		// 10-20second delay after which the screen will turn green.
};

enum class UserInput
{
	NONE,
	LEFT,
	RIGHT,
	ACTION,
};


void handleOmniWatch();

#endif /* INC_OMNITRIX_HPP_ */
