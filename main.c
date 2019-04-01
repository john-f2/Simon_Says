  /*
 * Project5.c
 *
 * Created: 3/15/2019 3:39:23 PM
 * Author : John Fu
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "avr.h"
#include "lcd.h"

/* Struct used to represent the values of simon says*/
//struct simonvalue
//{
	//int pin;
	//sound to play for the value
	//int frequency;
	//int duration;
	//
//};

enum simon_states {simon_start, simon_play, simon_player, simon_check, simon_add, simon_gameover} simon_state;



//array used to store the values for the game
int simon_array[100]; 	
//pointer to add the next value to in the array 
int t = 0; 

//Used to change the seed for srand()
unsigned int SEED = 0;	

//player's array
int player_array[100];
int pt = 0;

/*check variable used to see if arrays are equal*/
int CHECK;

/*gloabl variable to keep track of the rounds played*/
int ROUND = 1;



/* is pressed function is to determine if the given row, col is pressed
return 1 if the button is pressed, else 0 */
int is_pressed(int row, int col)
{
	unsigned char rowCheck[4] = {0x10, 0x20,0x40,0x80};
	/*Makes DDRC an input, and makes the other ports a N/C */
	int p;
	for(p = 0; p<8; p++)
	{
		CLR_BIT(DDRC, p);
		CLR_BIT(PORTC, p);
	}
	
	//set pin 4 to weak 1 by setting the bit to 1
	SET_BIT(PORTC, row+4);
	
	//set the other pin to output a strong 0
	SET_BIT(DDRC, col);
	CLR_BIT(PORTC,col);
	
	//gets the output for the given row, if the row is "weak 1" then we know it is not being pressed
	if(GET_BIT(PINC, row+4) == rowCheck[row] )
	{
		return 0;
	}
	else
	{
		return 1;
	}
	
	
}


/*Plays the note for the given duration*/
void playNote(int freq)
{
	SET_BIT(DDRB, 4);
	
	//play the notes through the for loop using the SET_BIT, CLR_BIT and wait
	
	volatile int d;
	for(d=0; d<50;d++)
	{
		
		SET_BIT(PORTB, 4);
		avr_wait(4+freq);
		CLR_BIT(PORTB,4);
		avr_wait(4+freq);
		
	}
	
	
}

/* Get key function, used get keypad input
returns the key that is pressed */
int get_key()
{
	int r,c;
	for(r=0; r<4; r++)
	{
		for(c=0;c<4;c++)
		{
			//if is pressed is returned, then we know that the button is pressed
			if(is_pressed(r,c) == 1)
			{
				return 1 + (c*4) + r;
			}
		}
	}
	
	return 0;
}


/*prints value to lcd */
void print_to_lcd(int key)
{

	
	char top_buffer[17];
	sprintf(top_buffer, "%d", key);
	
	lcd_pos(0,0);
	lcd_puts2(top_buffer);
	
	char bottom_buffer[17];
	sprintf(bottom_buffer, " sffjsa");
	lcd_pos(1,0);
	lcd_puts2(bottom_buffer);
	
	
	
}

/*print game over on the lcd, called in the simon_gameover state */
void print_game_over()
{
	char top_buffer[17];
	sprintf(top_buffer, "Game Over");
		
	lcd_pos(0,0);
	lcd_puts2(top_buffer);
	
	char bottom_buffer[17];
	sprintf(bottom_buffer, "%d rounds played", ROUND);
	lcd_pos(1,0);
	lcd_puts2(bottom_buffer);
	
}

void print_start_screen()
{
	char top_buffer[17];
	sprintf(top_buffer, "Simon Says");
		
	lcd_pos(0,0);
	lcd_puts2(top_buffer);
		
	char bottom_buffer[17];
	sprintf(bottom_buffer, "D to start Game ");
	lcd_pos(1,0);
	lcd_puts2(bottom_buffer);
	
}

void print_yourturn()
{
	char bottom_buffer[17];
	sprintf(bottom_buffer, "Your Turn");
	lcd_pos(1,0);
	lcd_puts2(bottom_buffer);
}

void print_round()
{
	char top_buffer[17];
	sprintf(top_buffer, "Round: %d", ROUND);
	
	lcd_pos(0,0);
	lcd_puts2(top_buffer);
	
}

/*clears lcd */
void clear_lcd()
{
	char buffer[17] = "                ";
	lcd_pos(0,0);
	lcd_puts2(buffer);
	lcd_pos(1,0);
	lcd_puts2(buffer);

}

/*initializes DDRA to output for leds*/
void init_led_ddr()
{
	SET_BIT(DDRA, 0);
	SET_BIT(DDRA, 1);
	SET_BIT(DDRA, 2);
	SET_BIT(DDRA, 3);
	
}

/*picks a random simon says option and adds it to the simon_array*/
void pick_and_add_next_option(int nextValue)
{
	//adds the next value to the simon_array, increment the counter 
	simon_array[t] = nextValue;
	t++;
	
	
}

void play_option(int v)
{
	SET_BIT(PORTA, v);
	avr_wait(5000);
	CLR_BIT(PORTA, v);
	avr_wait(5000);
	
	
	
}

/*plays the simon array. use for the simon_play state */
void play_simon_arry()
{
	int i;
	for(i =0; i< t; i++)
	{
		playNote(simon_array[i]);
		play_option(simon_array[i]);	
		
	}
	
}

/*checks the two arrays to see if they are equal*/
int check_answers()
{
	int i;
	for(i=0; i<t; i++)
	{
		
		if(player_array[i] != simon_array[i])
		{
			CHECK = 0;
			return 0;
		}
	}
	CHECK = 1;
	return 1;
}

/*player turn function, used in simon_player state, used to add user move to their array*/
void player_turn(int key)
{
	
	//adds the player's move to the player's array
	player_array[pt] = abs(4-key);
	pt+=1;
	playNote(abs(4-key));
	play_option(abs(4-key));
}





void simon_says()
{
	int key = get_key();
	if(key!=0)
	{
		avr_wait(1800);
	}
	//transitions state
	switch(simon_state)
	{
		case(simon_start):
			print_start_screen();
			//if key 'A' is pressed, change state to simon_play
			if(key == 16)
			{
				//sets the Seed for random generation
				srand(SEED);
				//adds the first value to the simon_array
				pick_and_add_next_option(((rand() % (3 - 0 + 1)) + 0));
				simon_state = simon_play;
				SEED = 0;
				clear_lcd();
			
			}
			else
			{
				SEED +=1;
				
			}

			break;
		case(simon_play):
			clear_lcd();
			print_round();			
			play_simon_arry();
			simon_state = simon_player;
			break;
		case(simon_player):
			print_yourturn();
			if(pt == t)
			{
				simon_state = simon_check;
				pt = 0;
				break;
			}
			if(key == 1 || key == 2 || key == 3 || key == 4)
			{
				
				player_turn(key);
				avr_wait(1000);		
					
			}
			break;
		case(simon_add):
			pick_and_add_next_option(((rand() % (3 - 0 + 1)) + 0));
			simon_state = simon_play;
			break;
		case(simon_check):
			if(check_answers() == 1)
			{
				
				simon_state = simon_add;
				ROUND++;
			}
			else
			{
				simon_state = simon_gameover;
			}
			avr_wait(5000);
			//print_to_lcd(CHECK);
			break;
		case(simon_gameover):
			clear_lcd(); 
			print_game_over();
			avr_wait(220000);
			
			simon_state = simon_start;
			pt = 0; 
			t = 0;
			ROUND = 1;
			break;
		default:
			break;
			
	}
	
	
}




int main(void)
{
	//Initializes the LCD
	lcd_init();
	
	//generates a random seed for rand()
	srand(time(NULL));
	
	
	//set DDRA pins to output
	init_led_ddr();
    
	//initial state is simon_start 
	simon_state = simon_start;
    while (1) 
    {
		
		simon_says();	
    }
}

