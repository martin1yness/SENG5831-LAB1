#ifndef __LEDs_H
#define __LEDs_H

#include <avr/io.h>         //gives us names for registers
#include <avr/interrupt.h>

#include <inttypes.h> //gives us uintX_t

#include <string.h>
#include "led.h"

volatile uint32_t G_redToggles;
volatile uint32_t G_yellowToggles;
volatile uint32_t G_greenToggles;

#endif