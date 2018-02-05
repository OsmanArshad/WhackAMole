# WhackAMole

## Description
This is my implementation of the game "Whack A Mole" on a breadboard using ATmega microcontrollers that are programmed using finite state machine design with C. 

The game works by having an LED matrix illuminate a random light for a variable time within a 4x4 space on the matrix to represent the "mole". Then 2 players each using their own 4x4 keypad, hit the button on the keypad that corresponds to that light, and the player who hits the right button first, gets the point. If a player gets three consecutive correct hits, he/she is awarded 2 additional points for each hit after that. An LCD screen serves as the menu and keeps track of each player's points. The game ends when a player hits 20 points, and a corresponding message is displayed on the LCD Screen.

## Video
Here is a quick video I made displaying the game:
https://www.youtube.com/watch?v=1SFMhlHLtAY
