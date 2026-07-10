/*
 * cpp_mutex.hpp
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#ifndef INC_CPP_MUTEX_HPP_
#define INC_CPP_MUTEX_HPP_

#include "FreeRTOS.h"
#include "semphr.h"
#include <mutex>

class FreeRtosMutex {
public:
    // Constructor is now completely empty and safe to call before main()!
    FreeRtosMutex() = default;

    ~FreeRtosMutex() {
        if (xMutex != nullptr) {
            vSemaphoreDelete(xMutex);
        }
    }

    // Explicit runtime initialization method called inside main()
    void init() {
        if (xMutex == nullptr) {
            xMutex = xSemaphoreCreateMutex();
        }
    }

    // Required for C++ Lockables
    void lock() {
        // Fallback safety check: initialize on first use if forgotten
        if (xMutex == nullptr) {
            init();
        }
        if (xMutex != nullptr) {
            xSemaphoreTake(xMutex, portMAX_DELAY); // Note: Fix typo here too (xSemaphoreTake, not xQueueSemaphoreTake)
        }
    }

    // Required for C++ Lockables
    void unlock() {
        if (xMutex != nullptr) {
            xSemaphoreGive(xMutex);
        }
    }

    FreeRtosMutex(const FreeRtosMutex&) = delete;
    FreeRtosMutex& operator=(const FreeRtosMutex&) = delete;

private:
    SemaphoreHandle_t xMutex = nullptr;
};

#endif /* INC_CPP_MUTEX_HPP_ */
