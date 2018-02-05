/*
    Title: WhackAMoleGameLogic.c
    Author: Osman Arshad
    Email: osmanaarshad@gmail.com
    Description: WhackAMoleGameLogic.c implements the logic behind the game "Whack A Mole". 
    WhackAMoleLED.c works together with WhackAMoleGameLogic.c, by running on two separate 
    ATmega microcontrollers that communicate with each other. 
    This code follows a finite state machine design.
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.c"
#include "usart.h"

/*  ATmega microcontroller variables    */
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

/*  ATmega microcontroller functions    */
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}
unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}

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

//  The two keypads are placed on the board in different orientations 
//  resulting in the need to define two separate GetKeypadValue functions
unsigned char GetKeypadValue1()
{	
		PORTB = 0xEF;
		asm("nop");
		if (GetBit(PINB,0)==0) {return 0x01;} //1
		if (GetBit(PINB,1)==0) {return 0x05;} //4
		if (GetBit(PINB,2)==0) {return 0x09;} //7
		if (GetBit(PINB,3)==0) {return 0x0D;} //*

		PORTB = 0xDF;
		asm("nop");
		if (GetBit(PINB,0)==0) {return 0x02;} //2
		if (GetBit(PINB,1)==0) {return 0x06;} //5
		if (GetBit(PINB,2)==0) {return 0x0A;} //8
		if (GetBit(PINB,3)==0) {return 0xE;} // 0

		PORTB = 0xBF;
		asm("nop");
		if (GetBit(PINB,0)==0) {return 0x03;} //3
		if (GetBit(PINB,1)==0) {return 0x07;} //6
		if (GetBit(PINB,2)==0) {return 0x0B;} //9
		if (GetBit(PINB,3)==0) {return 0x0F;} //#
		
		PORTB = 0x7F;
		asm("nop");
		if (GetBit(PINB,0)==0) {return 0x04;} //A
		if (GetBit(PINB,1)==0) {return 0x08;} //B
		if (GetBit(PINB,2)==0) {return 0x0C;} //C
		if (GetBit(PINB,3)==0) {return 0x1F;} //D
		
		return(0xFF);
}
 
unsigned char GetKeypadValue2()
{	
		PORTC = 0xEF;
		asm("nop");
		if (GetBit(PINC,0)==0) {return 0x01;} //1
		if (GetBit(PINC,1)==0) {return 0x05;} //4
		if (GetBit(PINC,2)==0) {return 0x09;} //7
		if (GetBit(PINC,3)==0) {return 0x0D;} //*

		PORTC = 0xDF;
		asm("nop");
		if (GetBit(PINC,0)==0) {return 0x02;} //2
		if (GetBit(PINC,1)==0) {return 0x06;} //5
		if (GetBit(PINC,2)==0) {return 0x0A;} //8
		if (GetBit(PINC,3)==0) {return 0xE;} // 0

		PORTC = 0xBF;
		asm("nop");
		if (GetBit(PINC,0)==0) {return 0x03;} //3
		if (GetBit(PINC,1)==0) {return 0x07;} //6
		if (GetBit(PINC,2)==0) {return 0x0B;} //9
		if (GetBit(PINC,3)==0) {return 0x0F;} //#
		
		PORTC = 0x7F;
		asm("nop");
		if (GetBit(PINC,0)==0) {return 0x04;} //A
		if (GetBit(PINC,1)==0) {return 0x08;} //B
		if (GetBit(PINC,2)==0) {return 0x0C;} //C
		if (GetBit(PINC,3)==0) {return 0x1F;} //D
		
		return(0xFF);
}

/*  Global variables    */
static unsigned char recievedRandomLed;
static unsigned char winner = 0;
static unsigned char gameOn = 0;
static unsigned char menuKeypad;
static unsigned char quitCounter = 0;

static unsigned char playerOneKeypad;
static unsigned char playerOnePowerUpCounter = 0;
static unsigned char playerOneScore = 0;

static unsigned char playerTwoKeypad;
static unsigned char playerTwoScore = 0;
static unsigned char playerTwoPowerUpCounter = 0;

// Receives the location of the "mole" from the other microcontroller
void recieveRandomLed()
{
	if (USART_HasReceived(0))
	{
		recievedRandomLed = USART_Receive(0);
	}
}

// playerOneTick runs the game as experienced by a player
enum playerOneStates {playerOneStart, playerOneInput, playerOneScored, playerOneWait} playerOneState;
void playerOneTick()
{
	// The transitions taken between the states
    switch (playerOneState)
	{
        // Game starts
		case playerOneStart:
			playerOneState = playerOneInput;
			playerOnePowerUpCounter = 0;
		break;
		
        // Read in what player one entered on keypad, and send him to the appropriate state
		case playerOneInput:
            playerOneKeypad = GetKeypadValue1();
            
            if (playerOneKeypad == recievedRandomLed)
            {
                playerOneState = playerOneScored;
            }
            else if (playerOneKeypad != recievedRandomLed && playerOneKeypad == 0xFF)
            {
                playerOneState = playerOneInput;
            }
            else if (playerOneKeypad != recievedRandomLed)
            {
                playerOneState = playerOneStart;
            }
		break;
		
		case playerOneScored:
			playerOneState = playerOneWait;
		break;
		
        // Wait here until player one hits the keypad again
		case playerOneWait:
			playerOneKeypad = GetKeypadValue1();
			if (playerOneKeypad == 0xFF)
			{
				playerOneState = playerOneInput;
			}
			else
			{
				playerOneState = playerOneWait;
			}		
		break;
	}
	
    // The actions taken in each state
	switch (playerOneState)
	{
		case playerOneStart:
			playerOnePowerUpCounter = 0;
		break;
		
		case playerOneInput:
		break;
		
		case playerOneScored:
		{
            // playerOnePowerUpCounter rewards 3 consecutive hits with 3 points, instead of 1 
			playerOnePowerUpCounter++;
			if (playerOnePowerUpCounter >= 3)
			{
				playerOneScore = playerOneScore + 3;
			}
			else
			{
				playerOneScore++;
			}
			
			if (playerOneScore >= 20)
			{
				winner = 1;
				playerTwoScore = 0;
			}
			
			LCD_init();
			LCD_Cursor(1);
			LCD_WriteData(playerOneScore + '0');	
			LCD_Cursor(17);
			LCD_WriteData(playerTwoScore + '0');		
		}
		break;
		
		case playerOneWait:
		break;		
	}
}

// playerTwoTick is the same code as playerOneTick
enum playerTwoStates {playerTwoStart, playerTwoInput, playerTwoScored, playerTwoWait} playerTwoState;
void playerTwoTick()
{
    // The transitions taken between the states
	switch (playerTwoState)
	{
        // Game starts
		case playerTwoStart:
            playerTwoState = playerTwoInput;
            playerTwoPowerUpCounter = 0;
		break;
		
        // Read in what player one entered on keypad, and send him to the appropriate state
		case playerTwoInput:
            playerTwoKeypad = GetKeypadValue2();
            
            if (playerTwoKeypad == recievedRandomLed)
            {
                playerTwoState = playerTwoScored;
            }
            else if (playerTwoKeypad != recievedRandomLed && playerTwoKeypad == 0xFF)
            {
                playerTwoState = playerTwoInput;
            }
            else if (playerTwoKeypad != recievedRandomLed)
            {
                playerTwoState = playerTwoStart;
            }
		break;
		
		case playerTwoScored:
		    playerTwoState = playerTwoWait;
		break;
		
        // Wait here until player one hits the keypad again
		case playerTwoWait:
            playerTwoKeypad = GetKeypadValue2();
            if (playerTwoKeypad == 0xFF)
            {
                playerTwoState = playerTwoInput;
            }
            else
            {
                playerTwoState = playerTwoWait;
            }
		break;
	}
	
    // The actions taken in each state
	switch (playerTwoState)
	{
		case playerTwoStart:
		    playerTwoPowerUpCounter = 0;
		break;
		
		case playerTwoInput:
		break;
		
		case playerTwoScored:
		{
            // playerTwoPowerUpCounter rewards 3 consecutive hits with 3 points, instead of 1
			playerTwoPowerUpCounter++;
			if (playerTwoPowerUpCounter >= 3)
			{
				playerTwoScore = playerTwoScore + 3;
			}
			else
			{
				playerTwoScore++;
			}
			
			if (playerTwoScore >= 20)
			{
				winner = 2;
				playerOneScore = 0;
			}
			
			LCD_init();
			LCD_Cursor(1);
			LCD_WriteData(playerOneScore + '0');
			LCD_Cursor(17);
			LCD_WriteData(playerTwoScore + '0');
		}
		break;
		
		case playerTwoWait:
		break;
	}
}


int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xF0; PORTB = 0x0F;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	
	TimerSet(1);
	TimerOn();
	
	initUSART(0);
	USART_Flush(0);
	
	LCD_init();
	
	playerOneState = playerOneStart;
	playerTwoState = playerTwoStart;
	
	LCD_DisplayString(1, "Whack A Mole");
		
	while(1)
	{
		menuKeypad = GetKeypadValue2();
		if (menuKeypad == 0x0D)     // Quit game
		{
			quitCounter++;
			if (quitCounter > 500)
			{
				gameOn = 0;
				winner = 0;
				gameOn = 0;
				playerOneScore = 0;
				playerTwoScore = 0;
				quitCounter = 0;
			}
		}
		
		if (gameOn == 0)
		{
			menuKeypad = GetKeypadValue2();
			if (menuKeypad != 0xFF)    // Start game
			{
				gameOn = 1;
				LCD_init();
				LCD_Cursor(1);
				LCD_WriteData(playerOneScore + '0');
				LCD_Cursor(17);
				LCD_WriteData(playerTwoScore + '0');
			}
		}
		
        // Game is now running
		if (gameOn == 1)
		{
            // Run the state machines until winner is determined
			recieveRandomLed();
			playerOneTick();
			playerTwoTick();
			
            // Winner has his score displayed on the LCD screen
			if (winner == 1)
			{
				LCD_DisplayString(1,"P1 wins");
				menuKeypad = GetKeypadValue1();
				while(menuKeypad == 0xFF)
				{
					menuKeypad = GetKeypadValue2();
				}
				winner = 0;
				gameOn = 0;
				playerOneScore = 0;
				playerTwoScore = 0;
			}
			if (winner == 2)
			{
				LCD_DisplayString(1,"P2 wins");
				menuKeypad = GetKeypadValue2();
				while(menuKeypad != 0x0D)
				{
					menuKeypad = GetKeypadValue2();
				}
				winner = 0;
				gameOn = 0;
				playerOneScore = 0;
				playerTwoScore = 0;
			}
			
			while(!TimerFlag);
			TimerFlag = 0;		
		}

	}
}