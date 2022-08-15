
#include "main.h"

char buff_rx[buff_rx_SIZE] = {0};
char rpm_buff_twi[SIZEOF_SENDBUF];
char power_buff_twi[SIZEOF_SENDBUF];
char buff_to_send[SIZEOF_SENDBUF];

char *buff_rx_begin = buff_rx;

volatile uint8_t pos = 0;
volatile uint8_t pwm = 0;
volatile uint8_t current_pwm = 0;
volatile uint16_t time_fixation = 0;
volatile uint8_t end_of_second_flag = 0;
volatile uint16_t analys_increment = 0;
volatile uint8_t pulse_capture_flag = 0;
volatile uint8_t interrupt_on_pcf_flag = 0;


volatile uint32_t max_ticks_between_pulses = 0;
volatile uint32_t min_ticks_between_pulses = 65535;
volatile uint32_t mid_ticks_between_pulses;
volatile uint16_t rev_per_min = 0;
volatile uint16_t rev_per_sec = 0;
volatile uint32_t real_time_between_adjacent_pulses;
volatile uint32_t real_rev_time;

const char command_on[] = "On\r";
const char command_off[] = "Off\r";
const char command_pwm[] = "Power";

volatile char flag_recive = 0;


uint8_t change_pwm(uint8_t copy_pwm) {
	clearlcd();
	if (copy_pwm > 100 && copy_pwm < 121) {
		copy_pwm = 100;
	}
	if (copy_pwm >= 121) {
		copy_pwm = 0;
	}
	sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", copy_pwm);
	DebagUart(buff_to_send);
	UCSR0B |= (1 << UDRIE0);
	return copy_pwm;
}

void button_handler(void) {
	uint8_t pcf_pins_value = 0;
	pcf_pins_value = pcf8574_getinput(0);
	switch (pcf_pins_value) {
		case 0b00000001://pwm = 0
			pwm = 0;
			pwm = change_pwm(pwm);
			break;
		case 0b00000010://pwm = 100
			pwm = 100;
			pwm = change_pwm(pwm);
			break;
		case 0b00000100://vallue pwm down 20
			pwm -= 20;
			pwm = change_pwm(pwm);
			break;
		case 0b00001000://vallue pwm up 20
			pwm += 20;
			pwm = change_pwm(pwm);
			break;
		case 0b00010000://vallue pwm down 5
			pwm -= 5;
			pwm = change_pwm(pwm);
			break;
		case 0b00100000://vallue pwm up 5
			pwm += 5;
			pwm = change_pwm(pwm);
			break;
		case 0b01000000://vallue pwm down 1
			pwm -= 1;
			pwm = change_pwm(pwm);
			break;
		case 0b10000000://vallue pwm up 1
			pwm += 1;
			pwm = change_pwm(pwm);
			break;
		default:
			break;
	}
}

int main(void) {
	port_setup();
	int0_init();
	int1_init();
	UartInit();
	pcf8574_init();
	pcf8574_setoutput(0, 0xFF);
	I2C_Init();
	LCD_ini();
	clearlcd();
	sei();
	//setPwmDuty(pwm);
	/*When you connect via UART, the fan accelerates and the pwm becomes 0*/
	sprintf((char*)buff_to_send, "mode:OFF [power: %d%%]\r\n", pwm);
	DebagUart(buff_to_send);
	UCSR0B |= (1 << UDRIE0);

	while (1) {
		if (flag_recive) {
			const char space[] = " ";
			char *until_space;
			char *after_space;
			until_space = strtok (buff_rx_begin, space);
			after_space = strtok (NULL, space);
			if ( after_space != NULL){
				*(after_space + 3) = '\0';
				current_pwm = strtol(after_space,0, 10);
				if (errno == ERANGE) {
					current_pwm = 100;//overflow
					errno = 0;
				} else {
					if (current_pwm > 100) {
						current_pwm = 100;
					} else {
						if (current_pwm <= 0) {
							current_pwm = 0;
						}
					}
				}
			}
			else {//str havnt spaces
				until_space = buff_rx_begin;
			}
			if (!strcmp((char *)until_space, command_off)){
				pwm = 0;
				clearlcd();
				setPwmDuty(pwm);
				sprintf((char*)buff_to_send, "mode:OFF [power: %d%%]\r\n", pwm);
			} else {
				if (!strcmp((char *)until_space, command_on)){
					setPwmDuty(pwm);
					sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", pwm);
				} else {
					if ((!strcmp((char *)until_space, command_pwm))){
						pwm = current_pwm;
						if (pwm != 0) {
							setPwmDuty(pwm);
							sprintf((char*)buff_to_send, "mode:ON   [power: %d%%]\r\n", pwm);
						} else {
							setPwmDuty(pwm);
							sprintf((char*)buff_to_send, "mode:OFF [power: %d%%]\r\n", pwm);
						}
					} else {
						sprintf((char*)buff_to_send, "Error\r\n");
					}
				}
			}
			DebagUart(buff_to_send);
			UCSR0B |= (1 << UDRIE0);
			flag_recive = 0;
		} 
		if (pulse_capture_flag) {
			if (time_fixation > max_ticks_between_pulses) {
				max_ticks_between_pulses = time_fixation;
			}
			if (time_fixation < min_ticks_between_pulses) {
				min_ticks_between_pulses = time_fixation;
			}
			// sprintf((char*)buff_to_send, "time: %d\r\n", time_fixation);
			// //sprintf((char*)buff_to_send, "min: %ld max: %ld\r\n", min_ticks_between_pulses , max_ticks_between_pulses);
			// DebagUart(buff_to_send);
			// UCSR0B |= (1 << UDRIE0);

			pulse_capture_flag = 0;
		}
		if (end_of_second_flag) {

			if (max_ticks_between_pulses >= min_ticks_between_pulses) {
				mid_ticks_between_pulses = 5*((max_ticks_between_pulses + min_ticks_between_pulses) >> 1);
				real_time_between_adjacent_pulses = (mid_ticks_between_pulses * TICK_PWM)/PULSE_PASS;//real_time_between_pulses * 1 000 000
				real_rev_time = 2 * real_time_between_adjacent_pulses;//for one complete revolution, the fan gives 2 pulses

				rev_per_sec = 1000000/real_rev_time;//RPS = 1sec/real_rev_time	
			} else {
				rev_per_sec = 0;
			}
			rev_per_min = rev_per_sec * 60;//RPM = RPS * 60
			//Uncomment to output RPM to UART
			
			//sprintf((char*)buff_to_send, "rev: %d RPM\r\n", rev_per_min);
			//sprintf((char*)buff_to_send, "min: %ld max: %ld mid: %ld\r\n", min_ticks_between_pulses , max_ticks_between_pulses, mid_ticks_between_pulses);
			//sprintf((char*)buff_to_send, "mid: %ld\r\n",mid_ticks_between_pulses);
			//DebagUart(buff_to_send);
			//UCSR0B |= (1 << UDRIE0);
			
			//clearlcd();
			sprintf((char*)rpm_buff_twi, "RPM: %d        ",rev_per_min);
			setpos(0,0);
			str_lcd(rpm_buff_twi);

			max_ticks_between_pulses = 0;
			min_ticks_between_pulses = 65535;
			end_of_second_flag = 0;
		}

		sprintf((char*)power_buff_twi, "Power: %d%%", pwm);
		setpos(0,1);
		str_lcd(power_buff_twi);

		if (interrupt_on_pcf_flag) {
			button_handler();
			setPwmDuty(pwm);
			interrupt_on_pcf_flag = 0;
		}
	}
}


