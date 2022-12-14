#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define stop 10
#define ped 4

//flags

int termatismos=0;
int adcInterr=0;
int adcInterr2=0;
int timerInterr=0;


int main()
{
	// LET for straight movement

	PORTD.DIR |=  PIN1_bm ; // PIN1 is output
	PORTD.OUT |= PIN1_bm; // LED1 is off
	PORTD.OUTCLR |=  PIN1_bm ; //LED1 IS ON


	// LET for left turn
	PORTD.DIR |=  PIN2_bm ; // PIN2 is output
	PORTD.OUT |= PIN2_bm; // LED2 is off

	// LET for right turn

	PORTD.DIR |=  PIN3_bm ; // PIN3 is output
	PORTD.OUT |= PIN3_bm; // LED3 is off


	// Begin accepting interrupt signals
	sei();

	// Initialize the ADC for Free-Running mode

	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; // Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; // Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // The bit

	// Enable Debug Mode

	ADC0.DBGCTRL |= ADC_DBGRUN_bm;

	// Window Comparator Mode

	ADC0.WINLT |= stop; // Set threshold
	ADC0.WINHT |= stop; // Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; // Interrupt when RESULT < WINLT

	ADC0.COMMAND |= ADC_STCONV_bm; // Start Conversion


	// main while terminates when flag reaches the value 4

	while (termatismos!=4)
	{
		// see if you can turn left

		if (adcInterr){

			//turn left
			PORTD.OUT |= PIN1_bm; // LED1 is off
			PORTD.OUTCLR = PIN2_bm; // LED2 is on
			

			// 16-bit counter high and low
			TCA0.SINGLE.CNT = 0; // Clear counter
			TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
			TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
			TCA0.SINGLE.CTRLA |= 1; // Enable
			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)

			while (timerInterr == 0)
			{
				
			}

			PORTD.OUT |= PIN2_bm; // LED2 is off
			
			// Reset flags

			timerInterr = 0;
			adcInterr = 0;
			termatismos+=1;

		}
		// if not, see if you should turn right

		if (adcInterr2){
			//turn right
			PORTD.OUT |= PIN1_bm; // LED1 is off
			PORTD.OUTCLR = PIN3_bm; // LED3 is on
			
			// 16-bit counter high and low
			TCA0.SINGLE.CNT = 0; // Clear counter
			TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
			TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
			TCA0.SINGLE.CTRLA |= 1; // Enable
			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)
			while (timerInterr == 0)
			{
				
			}
			PORTD.OUT |= PIN3_bm; // LED3 is off
			termatismos-=1;
			// Reset flags
			timerInterr = 0;
			adcInterr2 = 0;
		}

		//keep going straight

		PORTD.OUTCLR |=  PIN1_bm ; //LED1 IS ON
		
	}






}

// Timer interrupt

ISR(TCA0_CMP0_vect){
	// Disable
	TCA0.SINGLE.CTRLA = 0;
	// Clear flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;

	// Set flag
	timerInterr = 1;
}

// ADC interrupt

ISR(ADC0_WCOMP_vect)
{
	// we see from which state the interrupt came to us

	if(ADC0.RES<stop)
	{
		
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;

		// Set flag
		adcInterr = 1;
	}
	else if(ADC0.RES>stop)
	{
		
		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;

		// Set flag
		adcInterr2 = 1;

	}


}
