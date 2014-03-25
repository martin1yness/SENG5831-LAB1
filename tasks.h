/*******************************************
*
* Header file for Task Code
*
*******************************************/
#ifndef __TASK_H
#define __TASK_H

#include <inttypes.h> //gives us uintX_t
#include "led.h"

// number of empty for loops to eat up about 10 ms
#define FOR_COUNT_10MS 6000

volatile uint32_t __ii;

// 20 million machine instructions per second
// loop with non-optimized counter takes 10 * i + 1 machine instructions
// 20,000,000 i / s * 1 s / 1000 ms * 1 loops / 10 i = 2000 loops / 1 ms
// This version actually takes 27 i per iteration
// 20,000,000 i / s * 1 s / 1000 ms * 1 loops / 27 i ~ 741 loops / 1 ms
#define WAIT_10MS { for ( __ii=0; __ii < FOR_COUNT_10MS; __ii++ ); }


#endif

