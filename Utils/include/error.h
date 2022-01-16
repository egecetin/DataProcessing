#pragma once

#include <inttypes.h>
#include <execinfo.h>

#define ALARM_INTERVAL 1

extern volatile uint64_t alarmCtr;
extern volatile uint8_t loopFlag;
extern volatile uint8_t sigReadyFlag;

/**
 * @brief           SIGALRM handler
 * 
 * @param signum
 */
void alarmFunc(int signum);

/**
 * @brief           Signal Segmentation handler
 * 
 * @param signum    Signal number
 */
void backtracer(int signum);

/**
 * @brief           Interrupt handler
 * 
 * @param signum    Signal number
 */
void interruptFunc(int signum);