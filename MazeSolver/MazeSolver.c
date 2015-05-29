#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>

/* IN1, IN2 - first motor directions
 * IN3, IN4 - second motor directions
 * 1, 0 -> forward
 * 0, 1 -> backward
 * 1, 1 / 0, 0 -> fast stop
 */
#define IN_PORT		PORTB
#define IN_DDR		DDRB
#define IN1			PB0
#define IN2			PB1
#define IN3			PB2
#define IN4			PB3
#define EN_PORT		PORTD
#define EN_DDR		DDRD
#define EN1			PD4 //PB3
#define EN2			PD5 //PB4
#define LED_PORT	PORTD
#define LED_DDR		DDRD
#define LED			PD7
#define CM_SEVEN	614
#define CM_TEN 		470
#define CM_FIFTEEN	338
#define CM_TWENTY	266
#define MOTOR_R		OCR1A
#define MOTOR_L		OCR1B
#define boolean		uint8_t
#define TRUE		1
#define FALSE		0
#define T_NINETYD	700


void pwm_init() {
	/* Use timer 1, channel A for motor 1 and channel B for motor 2 */
	
	/* Set outputs */
	EN_DDR |= _BV(EN1) | _BV(EN2);
	
	//timer 1 , 8 bit mode , fast pwm non inverting
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (3 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	
	/* 100% */
	OCR1A = 1023; //right motor
	OCR1B = 1023; //left motor
}

void ADC_init(void)
{
	#ifndef ADC_USE_IRQ

	// enable ADC with:
	// *reference AVCC with external capacitor at AREF pin
	// *without left adjust of conversion result
	// *no auto-trigger
	// *no interrupt
	// *prescaler at 128
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (7 << ADPS0);
	#else

	// enable ADC with interrupt
	ADMUX = (1 << REFS0) | (1 << ADC_INIT_CHANNEL) | (1 << 7);
	ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (7 << ADPS0);
	#endif
}

int ADC_get(uint8_t channel)
{
	#ifndef ADC_USE_IRQ

	// start ADC conversion on "channel"
	// wait for completion
	// return the result
	ADMUX = (ADMUX & ~(0x1f << MUX0)) | channel;

	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));

	return ADC;
	#else

	return ADC_value[channel];
	#endif

	(void)channel;
	return 0;
}

int get_duty_cycle(unsigned int procent) {
	//don't be stupid
	if(procent > 100)
		return 0;
		
	return procent * 1023 / 100;
}

void senzor_test(void) 
{
	int val, val2;
	
	while(1) {
		val = ADC_get(0);
		val2 = ADC_get(7);
		
		//check for objects within 10 to 20 cm
		if (val2 > 266 && val2 < 470) {
			OCR1B = 512;
		}
		else {
			OCR1B = 1023;
		}
		
		if (val > 266 && val < 470) {
			OCR1A = 512;
		}
		else {
			OCR1A = 1023;
		}
		
		if((val > 266 && val < 470) || (val2 > 266 && val2 < 470))
			LED_PORT |= _BV(LED);
		else
			LED_PORT &= ~_BV(LED);
		_delay_ms(25);
	}

}

void move_front(void)
{
	MOTOR_R = 300 + 31;
	MOTOR_L = 300;
}

void full_stop(void)
{
	MOTOR_R = 0;
	MOTOR_L = 0;
}

//turns 90 degrees
void turn_right()
{
	//set direction
	IN_PORT &= ~_BV(IN3);
	IN_PORT |= _BV(IN4);
	
	MOTOR_L = 400;
	MOTOR_R = 400 + 31;
	_delay_ms(T_NINETYD);
	
	//set direction back to normal
	IN_PORT &= ~_BV(IN4);
	IN_PORT |= _BV(IN3);
	
	
}

//turns 90 degrees
void turn_left()
{
	//set direction
	IN_PORT &= ~_BV(IN2);
	IN_PORT |= _BV(IN1);
	
	MOTOR_L = 400;
	MOTOR_R = 400 + 31;
	_delay_ms(T_NINETYD);
	
	//set direction back to normal
	IN_PORT &= ~_BV(IN1);
	IN_PORT |= _BV(IN2);
	
	
}

//first logic attempt
void run_logic() {
	int frontVal, rightVal;
	boolean sweetspot = FALSE, hitFront = FALSE, hitRight = FALSE;
	
	while(1) {
		//read sensors
		rightVal = ADC_get(0);
		frontVal = ADC_get(7);
		
		//check distances
		if(frontVal > CM_TEN) //checks if distance smaller than 10cm
			hitFront = TRUE;
		if(rightVal > CM_TEN)
			hitRight = TRUE;
		
		//check sweetspot
		if(frontVal < CM_TEN && rightVal < CM_TEN) {//check without walls for now // && frontVal > TWENTY_CM) { //check if it's between 10 to 20cm
			sweetspot = TRUE;
			hitRight = FALSE;
			hitFront = FALSE;
		}
		
		/*
		//debug each sensor with 2 leds
		if(hitFront == TRUE)
			LED_PORT |= _BV(LED);
		else
			LED_PORT &= ~_BV(LED);
		
		if(hitRight == TRUE)
			PORTC |= _BV(PC0);
		else
			PORTC &= ~_BV(PC0);
		*/
		
		if(sweetspot == TRUE)
			move_front();
		else if(hitFront == TRUE && hitRight == FALSE)
			turn_right();
		else if(hitFront == TRUE && hitRight == TRUE)
			turn_left();		
		else
			full_stop();
			
		//if either sensor sees smth open led
		if(frontVal > CM_TEN || rightVal > CM_TEN)
			LED_PORT |= _BV(LED);
		else
			LED_PORT &= ~_BV(LED);
		
		
		
		
		_delay_ms(25);
		
		sweetspot = FALSE;
		hitRight = FALSE;
		hitFront = FALSE;
		
	}
	
}

void turn_left_small(void)
{
	MOTOR_L = 0;
}

void turn_right_small(void)
{
	MOTOR_R = 0;
}

void turn_right_arc(void)
{
	MOTOR_L = 200;
	MOTOR_R = 150 + 31;
}

void turn_left_arc(void)
{
	MOTOR_L = 150;
	MOTOR_R = 200 + 31;
}

void turn_right_soft()
{
	//set direction
	IN_PORT &= ~_BV(IN3);
	IN_PORT |= _BV(IN4);
	
	MOTOR_L = 400;
	MOTOR_R = 400 + 31;
	_delay_ms(200);
	
	//set direction back to normal
	IN_PORT &= ~_BV(IN4);
	IN_PORT |= _BV(IN3);
	
	
}

void turn_left_soft()
{
	//set direction
	IN_PORT &= ~_BV(IN2);
	IN_PORT |= _BV(IN1);
	
	MOTOR_L = 400;
	MOTOR_R = 400 + 31;
	_delay_ms(200);
	
	//set direction back to normal
	IN_PORT &= ~_BV(IN1);
	IN_PORT |= _BV(IN2);
	
	
}

//final algorithm version, tested within maze walls
void run_logic_walls() {
	
	int frontVal, rightVal;
	boolean sweetspot = FALSE, hitFront = FALSE, hitRight = FALSE, farRight = FALSE;

	
	while(1) {
		//read sensors
		rightVal = ADC_get(0);
		frontVal = ADC_get(7);
		
		//check distances
		if(frontVal > CM_FIFTEEN) //checks if distance smaller than 10cm
			hitFront = TRUE;
		if(rightVal > CM_TEN)
			hitRight = TRUE;
		else if(rightVal < CM_FIFTEEN)
			farRight = TRUE;
			
		//check if it's between 10 to 15cm
		if(hitRight == FALSE && farRight == FALSE) { 
			sweetspot = TRUE;

		}
		
			
		
		if(hitFront == TRUE && sweetspot == TRUE)
			turn_left();
		else if(hitFront == TRUE) {
			turn_right();
			move_front();
			_delay_ms(500);
		}
		else if(hitRight == TRUE) {
			//turn_left_arc();
			turn_left_soft();
			move_front();
			_delay_ms(200);
			
		}
		else if(farRight == TRUE) {
			//turn_right_arc();
			turn_right_soft();
			move_front();
			_delay_ms(200);
		}
		else
			move_front();
		
		
		//if either sensor sees smth open led
		if(frontVal > CM_FIFTEEN || rightVal > CM_FIFTEEN)
			LED_PORT |= _BV(LED);
		else
			LED_PORT &= ~_BV(LED);	
		
		_delay_ms(25);
		
		//reset flags
		sweetspot = FALSE;
		hitRight = FALSE;
		hitFront = FALSE;
		farRight = FALSE;
	}
	
}


//hardware test
void full_test(void)
{
	int frontVal, rightVal;
	
	while(1) {
		rightVal = ADC_get(0);
		frontVal = ADC_get(7);
		
		
		//check for objects within 10 to 20 cm
		/*
		if (val2 > 266 || val > 266) {
			//full stop
			OCR1B = 0;
			OCR1A = 0;
		}*/
		if(frontVal > CM_TEN && rightVal < CM_TWENTY) {
			turn_right();
		}
		else if(frontVal > CM_TEN && rightVal > CM_TEN) {
			turn_left();
		}
		else {
			MOTOR_L = 400;//get_duty_cycle(20);
			MOTOR_R = 400 + 31;//get_duty_cycle(20);
		}
		
		//if either sensor sees smth open led
		if(frontVal > CM_TWENTY || rightVal > CM_TWENTY)
			LED_PORT |= _BV(LED);
		else
			LED_PORT &= ~_BV(LED);
			
		
		_delay_ms(25);
	}

}

void led_test(void)
{
	//led test
	while(1)
	{
		LED_PORT |= _BV(LED);
		_delay_ms(1000);
		LED_PORT &= ~_BV(LED);
		_delay_ms(1000);
	}
}

int main(void)
{
	
	
	pwm_init();
	ADC_init();
	
	//second led for debug only (added extra, not native)
	//DDRC |= _BV(PC0);
	//PORTC |= _BV(PC0);
	
	//set input
	IN_DDR  |= _BV(IN1) | _BV(IN2) | _BV(IN3) | _BV(IN4);
	//set led
	LED_DDR |= _BV(LED);	
	//set direction
	IN_PORT |= _BV(IN2) | _BV(IN3);

	
	//senzor_test();
	//led_test();
 	//full_test();
	//run_logic();
	run_logic_walls();
	
	return 0;
}