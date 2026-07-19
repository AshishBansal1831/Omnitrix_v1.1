/*
 * fifo.h
 *
 *  Created on: 19-Jul-2026
 *      Author: ashish
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#include <stdint.h>
#include <cstring>

#define CIRCULAR_FIFO_BUFFER_SIZE (64)

class FIFO
{
		volatile uint8_t Buffer[CIRCULAR_FIFO_BUFFER_SIZE];
		volatile uint16_t InIdx;
		volatile uint16_t OutIndx;

	public:

		inline uint8_t add(const uint8_t data)
		{
			uint16_t next = (InIdx + 1U) % CIRCULAR_FIFO_BUFFER_SIZE;

			if (next == OutIndx)
			{
				return 0;
			}

			Buffer[InIdx] = data;
			InIdx = next;

			return 1;
		}

		inline uint8_t read(FIFO *fifo, uint8_t *data)
		{
			if (InIdx == OutIndx)
			{
				return 0;
			}

			*data = Buffer[OutIndx];
			OutIndx = (OutIndx + 1) % CIRCULAR_FIFO_BUFFER_SIZE;
			return 1;
		}

		inline uint8_t add(const uint8_t* data, uint16_t length)
		{
			if (data == nullptr || length == 0)
			{
				return 0;
			}

			// 1. Calculate current available free space in the FIFO
			int16_t freeSpace = OutIndx - InIdx - 1;
			if (freeSpace < 0)
			{
				freeSpace += CIRCULAR_FIFO_BUFFER_SIZE;
			}

			// If the incoming packet size exceeds the space left, reject it entirely
			if (length > (uint16_t)freeSpace)
			{
				return 0;
			}

			// 2. Calculate contiguous space available from InIdx to the literal end of the array
			uint16_t spaceToEnd = CIRCULAR_FIFO_BUFFER_SIZE - InIdx;

			if (length <= spaceToEnd)
			{
				// Case A: The whole packet fits cleanly in a single copy without wrapping
				memcpy((void*)&Buffer[InIdx], data, length);
				InIdx = (InIdx + length) % CIRCULAR_FIFO_BUFFER_SIZE;
			}
			else
			{
				// Case B: Data wraps around. Split it into two distinct memcpy blocks
				uint16_t firstPart = spaceToEnd;
				uint16_t secondPart = length - firstPart;

				// Fill up to the absolute end of the array
				memcpy((void*)&Buffer[InIdx], data, firstPart);

				// Wrap back around and drop the remainder at index 0
				memcpy((void*)&Buffer[0], data + firstPart, secondPart);

				// The new InIdx becomes exactly the size of the wrapped overflow
				InIdx = secondPart;
			}

			return 1;
		}

		void init()
		{
			memset((void*) Buffer, 0, sizeof(Buffer));
		}
};

#endif /* INC_FIFO_H_ */
