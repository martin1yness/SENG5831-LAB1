#define ECHO2LCD

#include <pololu/orangutan.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// System tasks
#include "menu.h"
#include "tasks.h"

/*
 * http://www.pololu.com/docs/0J20
 * http://www.pololu.com
 * http://forum.pololu.com
 */

// tick count for scheduler, yellow task, green task
volatile uint32_t G_timer3Ticks = 0;
volatile uint32_t G_timer0Ticks = 0;
volatile uint32_t G_timer1Ticks = 0;
volatile uint32_t G_redToggles = 0;
volatile uint32_t G_yellowToggles = 0;
volatile uint32_t G_greenToggles = 0;

int main()
{
	ConfigurePulseWithModulationClocks();
	
	int i;

	// Used to print to serial comm window
	char tempBuffer[32];
	int length = 0;

	// Turn all LEDs on for a second or two then turn off to confirm working properly
	// Send a message to LCD to confirm proper start. ( interrupts might need to be on for this ?? )
	// Send a message through serial comm to confirm working properly.

	// Initialize All Tasks

	clear();	// clear the LCD

	//enable interrupts
	sei();

	while (1) {
		/* BEGIN with a simple toggle using for-loops. No interrupt timers */

		// --------- blink LED by using a busy-wait delay implemented with an empty for-loop		
		//red_led(1);
		
		//length = sprintf( tempBuffer, "R toggles %d\r\n", G_redToggles );
		//print_usb( tempBuffer, length );

		#ifdef ECHO2LCD
		lcd_goto_xy(0,0);
		char* redToggles = malloc(32);
		itoa(G_redToggles, redToggles, 10);
		char* yellowToggles = malloc(32);
		itoa(G_yellowToggles, yellowToggles, 10);
		char* greenToggles = malloc(32);
		itoa(G_greenToggles, greenToggles, 10);
		char* milliSeconds = malloc(32);
		itoa(G_timer3Ticks, milliSeconds, 10);
		char* all = malloc(128);
		strcpy(all, redToggles);
		strcat(all, "r, ");
		strcat(all, yellowToggles);
		strcat(all, "y, ");
		strcat(all, greenToggles);
		strcat(all, "g");
		print(all);
		lcd_goto_xy(0, 1);
		print(milliSeconds);
		free(redToggles);
		free(yellowToggles);
		free(greenToggles);
		free(milliSeconds);
		free(all);
		#endif
		//printf("R:%d ",G_redToggles);

		// ----------- COMMENT OUT above implementation of toggle and replace with this...
		// ------------------- Have scheduler release tasks using user-specified period



		// --------------- RED task

		// --------------- MENU task
		///*
		#ifndef ECHO2LCD
		serial_check();
		check_for_new_bytes_received();
		#endif		
		//*/

	} //end while loop
}

inline void toggleRed() {
	LED_TOGGLE(RED);
	++G_redToggles;
}

inline void toggleYellow() {
	LED_TOGGLE(YELLOW);
	++G_yellowToggles;
}

inline void toggleGreen() {
	LED_TOGGLE(GREEN);
	++G_greenToggles;
}

/*
 * PWM Pulse-width Modulation - Used to control voltage over time a duty time
 *
 * Device Details
 * 1. Timer/Counter 0 8bit
 *    - Interrupts: TOV0, OCF0A, OCF0B
 * 2. Timer/Counter 1 16bit
 *    - Interrupts: TOV1 OCF1A, OCF1B, ICF1
 */
inline void ConfigurePulseWithModulationClocks() {
	// Red 2 Green 5  00100100
	// Disabled because board resets when enabled, assuming power draw problem
	//DDRA |= 0x24; // 00100100
	//DDRA |= 0x05; // 0000_0101
	DDRD |= (1 << DDD5); // PORTD PIN5 output
	
	//
	// Set 8-Bit Time/Counter 2 for 1ms resolution
	//
	//   20mil * 1/prescaler * 1/CompareMatchRegister = 1000 Hz (1ms)
	//   20mil / 8 / 250 = 10000 Hz (.1ms)
	//
	// setting of WGM01 to get CTC mode
	TCCR0A = 0x02; // 0000_0010 
	//  00 (n/a) 00 (read-only) 0 (Fast PWM) 010 (8 prescaler)
	TCCR0B = 0x02; // 0000_0101
	//  00000 (reserved) 0 (compare b interrupt) 1 (compare a interrupt) 0 (overflow interrupt)
	TIMSK0 = 0x02; // 0000_0010
	// Output Compare Registers 0 A
	OCR0A = 250;
	 
	//
	// Set 16-bit Timer/Counter 1 for 1ms resolution
	//
	//   Clock 20Mil * 1/prescaler * 1/CompareMatchRegister = 10 Hz (100ms)
	//   20M / 64 / 31250 = 10
	
	// Timer/Counter Control Registers 3 A - xxxxxx00 bit
	TCCR3A &= ~(1 << WGM31);
	TCCR3A &= ~(1 << WGM30);
	// Timer/Counter Control Register 3 B - xxx01010
	TCCR3B &= ~(1 << WGM33); // xxx0xxxx
	TCCR3B |= (1 << WGM32); // xxxx1xxx
	// 64 prescaler
	TCCR3B &= ~(1 << CS32);  // xxxxx0xx
	TCCR3B |= (1 << CS31);  // xxxxxx1x
	TCCR3B |= (1 << CS30);  // xxxxxxx1
	// Timer Interrupt Mask Register - xxxxxx1x
	TIMSK3 |= (1 << OCIE3A); // xxxxxx1x
	// Output Compare Registers 3 A
	OCR3A = 31250;
	
	
	// 
	// Set Timer 1 (16-bit) PWD
	//
	// 10 (Channel A Toggle High/Low Output - Because PWD) 00 (Channel B Output) 00 (RESERVED) 11 (PWM Fast OCRnA top)
	TCCR1A = 0x83; // 1000_0011
	// Default 64bit prescaler
	TCCR1B |= 0x03; // xxxx_x011
	TCCR1B |= (1 << WGM13) | (1 << WGM12); // xxx1_1xxx
	TIMSK1 = 0x02; // Interrupt to count ticks
	OCR1A = 31250;
}

// Interrupt Service Routine every 100 ms using 16 bit timer/counter 3
ISR(TIMER3_COMPA_vect) {
	if(++G_timer3Ticks % 20 == 0) {
		toggleRed();	
	}
}

//
// 10000Hz (.1ms)
//
ISR(TIMER0_COMPA_vect) {
	for(int i=0; i<11; i++) {
		WAIT_10MS;
	}
	if(++G_timer0Ticks % 20000 ==0) {
		toggleYellow();			
	}
}

//
// User Defined Frequency
//
ISR(TIMER1_COMPA_vect) {
	//for(int i=0; i<11; i++) {
	//	WAIT_10MS;
	//}
	if(++G_timer1Ticks % 20 == 0) {	
		toggleGreen();
	}
}