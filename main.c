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
volatile uint32_t G_msTicks = 0;
volatile uint32_t G_100msTicks = 0;
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

		//#ifdef ECHO2LCD
		lcd_goto_xy(0,0);
		char* redToggles = malloc(32);
		itoa(G_redToggles, redToggles, 10);
		char* yellowToggles = malloc(32);
		itoa(G_yellowToggles, yellowToggles, 10);
		char* greenToggles = malloc(32);
		itoa(G_greenToggles, greenToggles, 10);
		char* milliSeconds = malloc(32);
		itoa(G_msTicks, milliSeconds, 10);
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
		//printf("R:%d ",G_redToggles);
		//#endif

		// ----------- COMMENT OUT above implementation of toggle and replace with this...
		// ------------------- Have scheduler release tasks using user-specified period



		// --------------- RED task

		// --------------- MENU task
		/*
		serial_check();
		check_for_new_bytes_received();
		*/

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
	//
	// Set 16-bit Timer/Counter 1 for 1ms resolution
	//
	//   Clock 20Mil * 1/prescaler * CompareMatchRegister = 1000 Hz
	//   2500000 / CMR = 1000 Hz
	//   CMR = 2500000 / 1000
	//   CMR = 2500
	
	// Timer/Counter Control Registers 3 A - xxxxxx00 bit
	TCCR3A &= ~(1 << WGM31);
	TCCR3A &= ~(1 << WGM30);
	// Timer/Counter Control Register 3 B - xxx01010
	TCCR3B &= ~(1 << WGM33); // xxx0xxxx
	TCCR3B |= (1 << WGM32); // xxxx1xxx
	TCCR3B &= ~(1 << CS02);  // xxxxx0xx
	TCCR3B |= (1 << CS01);  // xxxxxx1x
	TCCR3B &= ~(1 << CS00);  // xxxxxxx0
	// Timer Interrupt Mask Register - xxxxxx1x
	TIMSK3 |= (1 << OCIE3A); // xxxxxx1x
	// Output Compare Registers 3 A
	OCR3A = 2500;
	
	//
	// Set 8-bit (0-255) Timer/Counter 0 for 100ms resolution (10Hz)
	//  20,000,000 * 1 / y * 1 / x * 1 / 1000 = 1 / 100ms
	//  2,000,000 / y = x
	// Timer/Counter Control Registers A/B
	//  10 (non-inverted a) 10 (non-inverted b) 00 (read-only) 11 (Fast PWM)
	TCCR0A = 0xA3; // 10100011
	//  00 (n/a) 00 (read-only) 0 (Fast PWM) 101 (1024 prescaler)
	TCCR0B = 0x05; // 00000101
	// Timer Interrupt Mask Register
	//  00000 (reserved) 0 (compare b interrupt) 1 (compare a interrupt) 0 (overflow interrupt)
	TIMSK0 = 0x02; // 00000010
	// Output Compare Registers 0 A
	OCR0A = 195;
	
	// 
	// Set Timer 1 (16-bit) PWD
	//
	TCCR1A = 0xA3;
	TCCR1B = 0x02;
	TIMSK1 = 0x02;
	OCR1A = 2500;
}

// Interrupt Service Routine every 1 ms using 16 bit timer/counter 3
ISR(TIMER3_COMPA_vect) {
	if(++G_msTicks % 1000 == 0) {
		toggleRed();	
	}
}

//
// 100.01602564Hz (~10ms)
//
volatile uint16_t helperCount = 0;
ISR(TIMER0_COMPA_vect) {
	helperCount = (helperCount + 1) % 8;
	if(helperCount == 0) {
		if(++G_100msTicks % 10 ==0) {
			toggleYellow();	
		}
	}
}

//
// User Defined Frequency
//
ISR(TIMER1_COMPA_vect) {
	toggleGreen();
}