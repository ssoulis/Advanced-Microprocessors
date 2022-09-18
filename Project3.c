#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ped 20
#define stop 10

// flags

int bit5=0;
int bit6=0;
int enableCounter=0;
int enableAlarm=0;
int openedTimer=0;
int openedAlarm=0;
int closedAlarm=0;
int secondTimer=0;
int sirina=0;
int lathos=0;
int topvalue=0;
int dutycycle=0;
int decideled=0;
int tcCounter=0;
int mask5=0;
int mask6=0;
int kwdikos=1;

int main()
{

	// led for alarm
	PORTD.DIR |=  PIN0_bm ; // PIN0 is output
	PORTD.OUT |= PIN0_bm; // LED0 is off

	// Pull up enable and Interrupt enabled with sense on both edges

	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // pin5
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // pin6

	sei();


	while (1)
	{
		while (kwdikos){
			
			// give code
			
			
		}
		if(enableAlarm){

			// erase flag enableCounter so we can count from the start for the next counter
			enableCounter=0;

			// set flag openedTimer so we can know if this was the first timer that was set
			openedTimer=1;


			// 16-bit counter high and low
			TCA0.SINGLE.CNT = 0; // Clear counter
			TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
			TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
			TCA0.SINGLE.CTRLA |= 1; // Enable
			TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)

			if (openedAlarm==1){

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

				if(secondTimer==1){

					// erase flag enableCounter  so we can count from the start for the next counter
					enableCounter=0;

					// 16-bit counter high and low
					TCA0.SINGLE.CNT = 0; // Clear counter
					TCA0.SINGLE.CTRLB = 0; // Normal Mode (TCA_SINGLE_WGMODE_NORMAL_gc)
					TCA0.SINGLE.CMP0 = ped; // When reaches this value -> interrupt CLOCK FREQUENCY/1024
					TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // (= 0x7<<1)
					TCA0.SINGLE.CTRLA |= 1; // Enable
					TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable (=0x10)

					if(sirina==1) {

						// erase flag enableCounter  so we can count from the start for the next counter
						enableCounter=0;

						// pwm enable

						topvalue=100;
						dutycycle=2;
						TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc;
						TCA0.SINGLE.PER = topvalue;
						TCA0.SINGLE.CMP0 = dutycycle;
						TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
						TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm; // ENABLE INTERRUPT OVEREFLOW
						TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm; // ENABLE INTERRUPT CMPO
						TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // ENABLE
						

					}

				}

			}

		}

		// erase flags
		enableAlarm=0;
		openedAlarm=0;
		enableCounter=0;
		openedTimer=0;
		secondTimer=0;
		sirina=0;
		kwdikos=1;

	}

	cli();

}



ISR(PORTF_PORT_vect)
{

	// make mask for port5 so we can isolate 5th bit
	
	mask5=(1 << 5);

	// make mask for port6 so we can isolate 6th bit
	mask6=(1 << 6);

	// if this is =32 then we enable bit5

	bit5= PORTF.INTFLAGS & mask5;

	// if this is =64 then we enable bit6

	bit6 = PORTF.INTFLAGS & mask6;

	// 3 wrong enters of the code and we open the alarm
	
	if(lathos==3)
	{
		sirina=1;

	}


	// we recognize the sequence of switches SW6>SW5>SW5>SW6

	
	if (bit6==64 && enableCounter==0)
	{

		enableCounter++;



	}
	else if (bit5==32 && enableCounter==1)
	{

		enableCounter++;



	}
	else if (bit5==32 && enableCounter==2)
	{

		enableCounter++;


	}
	else if (bit6==64 && enableCounter==3)
	{
		if(sirina==1){

			// disable alarm stop pwd
			topvalue=0;
			dutycycle=0;
			PORTD.OUT |=  PIN0_bm ; // led0 IS OFF


		}
		else {
			
			kwdikos=0;
			enableAlarm=1;
		}

	}
	else                // if the wrong button is pressed for the sequence then we reset the enableCounter and we start all over again
	{
		
		// I need to understand here which time is the one asking for a password and if it's the one I care about with the wrong entries
		// and how many times the code has been entered so that on the third wrong time the siren sounds
		// so when we know the alarm is in the openAlarm state then we count wrong code entries
		if(openedAlarm==1 && sirina==0){

			lathos++;
			enableCounter=0;
		}
		else {
			enableCounter=0;

		}
	}


}

ISR(TCA0_CMP0_vect){

	// we see from which state the interrupt came if openTimer==1 & openAlarm==0 then its the first timer

	// if openedTimer==1 & openedAlarm==1 then its the second timer
	// we also have to know if we ae in the pws mode and if yes we should close leds

	if( openedTimer==1  && openedAlarm==0)
	{
		
		openedAlarm=1;


	}
	else if (openedTimer==1 && openedAlarm==1)
	{

		sirina=1;



	}
	else if (sirina==1){
		
		int intflags = TCA0_SINGLE_INTFLAGS;
		TCA0_SINGLE_INTFLAGS = intflags;
		PORTD.OUT |= PIN0_bm; // LED0 is off
		
	}



}


ISR(ADC0_WCOMP_vect)
{
	// open led0

	PORTD.OUTCLR |=  PIN0_bm ; //LED0 IS ON

	// set next timer

	secondTimer = 1;




}

ISR(TCA0_OVF_vect)
{
	// we already have high edge so led on
	int intflags = TCA0_SINGLE_INTFLAGS;
	TCA0_SINGLE_INTFLAGS = intflags;
	PORTD.OUTCLR |= PIN0_bm; // LED0 is on

}
