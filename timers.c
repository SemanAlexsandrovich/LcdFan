
#include "main.h"

volatile extern uint8_t end_of_second_flag;
volatile extern uint8_t pulse_capture_flag;
volatile extern uint8_t interrupt_on_pcf_flag;
volatile extern uint16_t time_fixation;
volatile uint8_t count_begin_flag = 0;
volatile uint8_t interrupt_flag = 0;
volatile uint16_t tick_counter = 0;


void port_setup(void) {
	DDRD &= ~(1 << COUNT_PIN);
	PORTD |= (1 << COUNT_PIN); 
	DDRD |= (1 << PWM_PIN);
	DDRB |=  (1 << LED_PIN);
	DDRD &= ~(1 << INT_PIN);
	PORTD |= (1 << INT_PIN);
}

void int1_init(void) {
	//counter 43360.433604336045 Hz (16000000/((368+1)*1))
	OCR1A = 368;
	TCCR1B |= (1 << WGM12);
	TCCR1B |= (1 << CS10);
	TIMSK1 |= (1 << OCIE1A);	
	//tacho
	EIMSK |= (1 << INT1);
	//The rising edge of INT1 generates an interrupt request.
	// EICRA |= (1 << ISC10);
	// EICRA |= (1 << ISC11);	
	//Any logical change on INT1 generates an interrupt request.
	EICRA |= (1 << ISC10);
	EICRA &= ~(1 << ISC11);
	TCNT1 = 0;
}

void int0_init(void) {

	//interrupt_on_pcf
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC00);//falling front
	EICRA &= ~(1 << ISC01);
	//pwm
	TCCR0A |= (1 << WGM00) | (1 << COM0A1);
	TCCR0B |= (1 << WGM01) | (1 << CS00);
	TCNT0 = 0;

}

void setPwmDuty(uint8_t duty) {
	OCR0A = (uint16_t) (duty * TOP) / 100;
}

ISR(INT0_vect) {// button status changed
	cli();
	if (!interrupt_on_pcf_flag) {
		interrupt_on_pcf_flag = 1;
	}
	sei();
}

ISR(TIMER1_COMPA_vect) {//couunt ticks from pulse to pulse
	cli();
	if (count_begin_flag) {
		tick_counter++;		
	}
	static uint16_t count_to_the_second = 0;
	count_to_the_second++;
	if ((count_to_the_second == 43360) && (end_of_second_flag == 0)) {
		count_to_the_second = 0;
		end_of_second_flag = 1;
	}
	sei();
}

ISR(INT1_vect) {//pulse_capture
	cli();
	interrupt_flag = !interrupt_flag;
	if (interrupt_flag == 0) {
		count_begin_flag = 1;
	} else {
		if (count_begin_flag == 1) {
			static uint8_t few_pulses = 0;
			few_pulses++;
			if (few_pulses >= PULSE_PASS) {
				if (pulse_capture_flag == 0) {
					count_begin_flag = 0;
					time_fixation = tick_counter;
					tick_counter = 0;
					few_pulses = 0;
					pulse_capture_flag = 1;
				}
			} else {}
		}
	}
	sei();
}
