
#ifndef MAIN_H_
#define MAIN_H_


#define F_CPU 16000000L
#define TICK_PWM 23//(1/F_PWM) = 1/43360 = 23/1 000 000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <inttypes.h>
#include <compat/twi.h>

#include "i2cmaster.h"
#include "timers.h"
#include "uart.h"
#include "twi.h"
#include "lcd.h"
#include "pcf8574.h"

#define PWM_PIN 6
#define COUNT_PIN 3
#define TOP 255
#define ONE_SEC 1000
#define INT_PIN 2
#define LED_PIN 5
#define PULSE_PASS 4

#endif /* MAIN_H_ */