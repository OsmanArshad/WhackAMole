/*
    Title: WhackAMoleLed.c
    Author: Osman Arshad
    Email: osmanaarshad@gmail.com
    Description: WhackAMoleLed.c implements the logic necessary to display the game "Whack A Mole " 
    on a 6x6 LED matrix board. WhackAMoleLED.c works together with WhackAMoleGameLogic.c, by running
    on two separate ATmega microcontrollers that communicate with each other. 
    This code follows a finite state machine design.
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

/*  Global variables */
// ATmega microcontroller variables
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

static unsigned short randomLedCounter = 0; // Used to control the speed at which the LED lights are illuminated
static unsigned int randomLed;              // Used to hold the randomly generated location of the mole on the LED matrix
static unsigned char randomLedTransmit;     // Used to hold the data that is transmitted to the other ATmega microcontroller

/* ATmega microcontroller functions */
void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0X02;
	TCNT1= 0;
	_avr_timer_cntcurr= _avr_timer_M;
	SREG |= 0X80;
}

void TimerOff()
{
	TCCR1B= 0x00;
}

void TimerISR()
{
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0 )
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr= _avr_timer_M;
}

// Generates a pseudo random number for the time length a "mole" will appear on the LED
unsigned int randIntervalGenerate(unsigned int min, unsigned int max)
{
	int r;
	const unsigned int range = 1 + max - min;
	const unsigned int buckets = RAND_MAX / range;
	const unsigned int limit = buckets * range;
	do
	{
		r = rand();
	} while (r >= limit);

	return min + (r / buckets);
}

/*  Finite State Machine   */
//  Function LedMatrixTick is responsible for the display of the game on the 6x6 LED board
//  The outside pattern states create a display around the 6x6 board, leaving the inside 4x4 of the LED blank
//  The pickRandomLed state picks a random spot to illuminate on that remaining 4x4 LED board
enum LedMatrixStates {outsidePatternTopAndBottom, outsidePatternSides, pickRandomLed} LedMatrixState;
void LedMatrixTick()
{
	static unsigned char outsidePatternColumnValue;
	static unsigned char outsidePatternColumn;
	static unsigned char randomLedColumnValue;
	static unsigned char randomLedColumn;

	randomLedCounter++;
	
    // The actions taken in each state
	switch (LedMatrixState)
	{
		case outsidePatternTopAndBottom:
            outsidePatternColumnValue = 0xC3;
            outsidePatternColumn = 0x00;
		break;
		
		case outsidePatternSides:
            outsidePatternColumnValue = 0x3C;
            outsidePatternColumn = 0x3C;
		break;
		
		case pickRandomLed:
            if (randomLedCounter < 5)
            {
                randomLed = rand_interval(1,16);
                if (randomLed == 1)
                {
                    randomLedColumnValue = 0x04;
                    randomLedColumn = 0xFB;
                    randomLedTransmit = 0x01;
                }
                if (randomLed == 2)
                {
                    randomLedColumnValue = 0x04;
                    randomLedColumn = 0xF7;
                    randomLedTransmit = 0x02;
                }
                if (randomLed == 3)
                {
                    randomLedColumnValue = 0x04;
                    randomLedColumn = 0xEF;
                    randomLedTransmit = 0x03;
                }
                if (randomLed == 4)
                {
                    randomLedColumnValue = 0x04;
                    randomLedColumn = 0xDF;
                    randomLedTransmit = 0x04;
                }
                if (randomLed == 5)
                {
                    randomLedColumnValue = 0x08;
                    randomLedColumn = 0xFB;
                    randomLedTransmit = 0x05;
                }
                if (randomLed == 6)
                {
                    randomLedColumnValue = 0x08;
                    randomLedColumn = 0xF7;
                    randomLedTransmit = 0x06;
                }
                if (randomLed == 7)
                {
                    randomLedColumnValue = 0x08;
                    randomLedColumn = 0xEF;
                    randomLedTransmit = 0x07;
                }
                if (randomLed == 8)
                {
                    randomLedColumnValue = 0x08;
                    randomLedColumn = 0xDF;
                    randomLedTransmit = 0x08;
                }
                if (randomLed == 9)
                {
                    randomLedColumnValue = 0x10;
                    randomLedColumn = 0xFB;
                    randomLedTransmit = 0x09;
                }
                if (randomLed == 10)
                {
                    randomLedColumnValue = 0x10;
                    randomLedColumn = 0xF7;
                    randomLedTransmit = 0x0A;
                }
                if (randomLed == 11)
                {
                    randomLedColumnValue = 0x10;
                    randomLedColumn = 0xEF;
                    randomLedTransmit = 0x0B;
                }
                if (randomLed == 12)
                {
                    randomLedColumnValue = 0x10;
                    randomLedColumn = 0xDF;
                    randomLedTransmit = 0x0C;
                }
                if (randomLed == 13)
                {
                    randomLedColumnValue = 0x20;
                    randomLedColumn = 0xFB;
                    randomLedTransmit = 0x0D;
                }
                if (randomLed == 14)
                {
                    randomLedColumnValue = 0x20;
                    randomLedColumn = 0xF7;
                    randomLedTransmit = 0x0E;
                }
                if (randomLed == 15)
                {
                    randomLedColumnValue = 0x20;
                    randomLedColumn = 0xEF;
                    randomLedTransmit = 0x0F;
                }
                if (randomLed == 16)
                {
                    randomLedColumnValue = 0x20;
                    randomLedColumn = 0xDF;
                    randomLedTransmit = 0x1F;
                }
                
                // Sends to the other ATmega microcontroller the location of the "mole"
                if (USART_IsSendReady())
                {
                    USART_Send(randomLedTransmit);
                }
            }
		break;
		
		default:
		break;
	}

    // The transitions taken between the states
	switch (LedMatrixState)
	{
		case outsidePatternTopAndBottom:
            PORTB = outsidePatternColumnValue;
            PORTA = outsidePatternColumn;
            LedMatrixState = outsidePatternSides;
		break;
		
		case outsidePatternSides:
            PORTB = outsidePatternColumnValue;
            PORTA = outsidePatternColumn;
            LedMatrixState = pickRandomLed;
		break;

		case pickRandomLed:
		    LedMatrixState = outsidePatternTopAndBottom;
            if (randomLedCounter >= 500)
            {
                PORTB = randomLedColumnValue;
                PORTA = randomLedColumn;
            }
            if (randomLedCounter >= 3000)
            {
                randomLedCounter = 0;
            }
		break;
		
		default:
		break;
	}
};

int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	TimerSet(1);
	TimerOn();
	LedMatrixState = outsidePatternTopAndBottom;
	
	while(1)
	{
		LedMatrixTick();
		while(!TimerFlag);
		TimerFlag = 0;
	}
}