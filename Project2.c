#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define ped 20
#define stop 10

//flags

int stopped =0;
int aristerhStrofh = 0;
int deksiaStrofh = 0;
int kanonikhTimh = 10;
int mishTimh = 5;
int counterportf5 = 0;
int counterportf6 = 0;
int tcCounter1 = 0;
int tcCounter2 = 0;
int bit5=0;
int bit6=0;
int mask1 = 0;
int mask5=0;
int mask6=0;
int pressportf5 = 0;
int pressportf6 = 0;
int tc = 0;
int decideled1 = 0;
int decideled2 = 0;


int main()
{
	
	//right wheel

	PORTD.DIR |=  PIN0_bm ; // PIN0 is output
	PORTD.OUT |= PIN0_bm; // LED0 is off
	

	//left wheel

	PORTD.DIR |=  PIN1_bm ; // PIN1 is output
	PORTD.OUT |= PIN1_bm; // LED1 is off
	

	// pin to stop going straight

	PORTD.DIR |=  PIN2_bm ; // PIN2 is output
	PORTD.OUT |= PIN2_bm; // LED2 is off


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


	// Pull up enable and Interrupt enabled with sense on both edges

	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // deksia strofh
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // aristerh strofh

	// we can get interrupts

	sei();


	while (1)
	{

		// straight movement

		TCA0.SPLIT.CTRLD  = TCA_SPLIT_SPLITM_bm;    		// Enable split mode

		// pwm gia deksio troxo

		TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV64_gc   
		TCA0.SPLIT.HPER = ped; // megisth timh counter
		TCA0.SPLIT.HCMP0 = kanonikhTimh; // duty cicle palmou

		// select SPLIT-Slope

		TCA0.SPLIT.CTRLA |= TCA_SPLIT_WGMODE_SPLITSLOPE_gc;

		// enable interrupt overflow

		TCA0.SPLIT.INTCTRL = TCA_SPLIT_OVF_bm;
		
		// enable interrupt HCMP0

		TCA0.SPLIT.INTCTRL |= TCA_SPLIT_HCMP0_bm;
		TCA0.SPLIT.CTRLA |= TCA_SPLIT_ENABLE_bm;


		// pwm for left wheel

		TCB0.SPLIT.CTRLB=TCA_SPLIT_CLKSEL_DIV1024_gc;
		TCB0.SPLIT.LPER = ped; // megisth timh counter
		TCB0.SPLIT.LCMP0 = kanonikhTimh; // duty cicle palmou

		// select SPLIT-Slope

		TCB0.SPLIT.CTRLB |= TCA_SPLIT_WGMODE_SPLITSLOPE_gc;

		// enable interrupt overflow

		TCB0.SPLIT.INTCTRL = TCA_SPLIT_OVF_bm;
		
		// enable interrupt LCMP0
		TCB0.SPLIT.INTCTRL |= TCA_SPLIT_LCMP0_bm;
		TCB0.SPLIT.CTRLB |= TCA_SPLIT_ENABLE_bm;



		if (stopped){

			// open led so we can turn

			PORTD.OUTCLR |=  PIN2_bm ;

		// stop right wheel

			// stop pwm for right wheel
			TCA0.SPLIT.HPER = 0;
			TCA0.SPLIT.HCMP0 =0;
			PORTD.OUT |= PIN0_bm; // LED0 is off

		// stop left wheel

			// stop pwm for left wheel
			TCB0.SPLIT.LPER = 0;
			TCB0.SPLIT.LCMP0 =0;
			PORTD.OUT |= PIN1_bm; // LED1 is off

			if (deksiaStrofh)
			{
				// we open leds for right turn and we turn on pwm

				// pwd for led0 will run for standard tempo
				PORTD.OUTCLR |=  PIN0_bm ; //LED0 IS ON
				TCA0.SPLIT.HPER = kanonikhTimh;
				TCA0.SPLIT.HCMP0 =kanonikhTimh;

				// pwm for led 1 will run with half pace of led0
				PORTD.OUTCLR |=  PIN1_bm ; //LED1 IS ON
				TCB0.SPLIT.LPER = mishTimh;
				TCB0.SPLIT.LCMP0 =mishTimh;



			}
			else if (aristerhStrofh)
			{


				// we open leds for left turn we turn on pwm

				PORTD.OUTCLR |=  PIN0_bm ; //LED0 IS ON
				TCA0.SPLIT.HPER = mishTimh;
				TCA0.SPLIT.HCMP0 =mishTimh;

				// pwm for led1 will run at standart tempo
				PORTD.OUTCLR |=  PIN1_bm ; //LED1 IS ON
				TCB0.SPLIT.LPER = kanonikhTimh;
				TCB0.SPLIT.LCMP0 =kanonikhTimh;

			}
			stopped=0;

		}

	// kleinoyme ledaki gia stripsimo

		PORTD.OUT |= PIN2_bm; // LED2 is off
	}


	cli();

}

	// interrupt for the stimalation of the wheel with leds



ISR(TCA0_SPLIT_HUNF_vect)
{
	// we make a mask with left shift for stimulation of bit LUNF

	mask1 = (1 << 1);

	// compare the maska with current timh 
	 
	 tc = TCA0.SPLIT.CNT & mask1;

	// check if we are at high edge

	if(TCA0.SPLIT.CNT==TCA0.SPLIT.CCMPL) // an CNT==CCMPL output goes high
	{
	// we have hight edge here so we blink leds 
		
		decideled1 = tcCounter1 % 2 ;

	// if decideled1 is even number then we open led if not we close led

		//even number
		if (decideled1==0)
		{

			PORTD.OUTCLR |=  PIN0_bm ; //LED0 IS ON


		}
		else 					//odd number
		{
			PORTD.OUT |= PIN0_bm; // LED0 is off
		}
		tcCounter1++			//increase counter for right wheel
	}

	// case LUNF has high edge

		 // ama tc!=0 then it's high edge my second pulse so I go to flash LED1 tha is my left wheel


		if (tc!=0) 
		{
			
			// if decideled2 is even we open led if not we close led

			decideled2 = tcCounter2 % 2 ;

			// even number
		if (decideled2==0)
		{

			PORTD.OUTCLR |=  PIN1_bm ; //LED1 IS ON


		}
		// odd number
		else
		{
			PORTD.OUT |= PIN1_bm; // LED1 is off
		}


		tcCounter2++		// increase counter for left wheel
		}



}




ISR(ADC0_WCOMP_vect)
{
	if (ADC0.RES<stop){

		int intflags = ADC0.INTFLAGS;
		ADC0.INTFLAGS = intflags;
		
		// set flags
		stopped = 1;

	}


}


ISR(PORTF_PORT_vect)
{
	// make mask for port5 to isolate 5th bit
	mask5=(1 << 5);

	// if bit5 is !0 then we enabled portf5

	bit5 = PORTF.INTFLAGS & mask5;
	
	
	// make mask for port6 to isolate 6th bit

	mask6=(1 << 6);

	// if this is !0 then we enabled portf6

	bit6 = PORTF.INTFLAGS & mask6;

	// decide if its the firts time the button is clicked or the second

	pressportf5= counterportf5 % 2;
	pressportf6= counterportf6 % 2;

	// if port5 was pressed and it wasnt an odd number then we start right turn

	if( bit5!= 0 && pressportf5==0)
	{
		//set flags
		deksiaStrofh=1;
		

	}

	
	if (bit5!=0){

		// inform counter that portf5 was pressed
		counterportf5++;
		
	}

	// if port6 was pressed and it wasnt an odd number then we start left turn

	if (bit6!= 0 && pressportf6==0)
	{

		//set flags
		aristerhStrofh = 1;
	}

	if (bit6!= 0){

		//inform counter that portf6 was pressed
		counterportf6++;

	}

	

}
