/*-----------------------------------------------------------------------------------------------------------------------------------
- Software Name: 8-bit Snake for ATmega328P
- Version: 1.0
- Language: C
- Developer: Emilio Rivera Macías
- Date: 09/18/2022
- Contact:
-----------------------------------------------------------------------------------------------------------------------------------*/

//definition of the frequency that the CPU will have
#define F_CPU 1000000ul
// necessary includes 
// ATmega328P libraries
#include <xc.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
// C libraries
#include <stdlib.h>
#include <time.h>

// --------------------------------> MACRO declaration <--------------------------------
// 5x7 pixels display
// rows
#define PIXEL_HEIGHT 5
// columns
#define PIXEL_WIDTH 7
// maximum size of snake
#define MAX_SNAKE_SIZE PIXEL_HEIGHT*PIXEL_WIDTH
// byte declaration
#define BYTE unsigned char
// ----------------------------------------------------------------------------------------
// --------------------------------> Struct declaration <--------------------------------
// stores the height (y) and width (x) coordinates 
struct COORDINATES {
	BYTE height_coordinate;
	BYTE width_coordinate;
};
// ----------------------------------------------------------------------------------------
// --------------------------------> Variable declaration <--------------------------------
// 'u' = up / 'd' = down / 'l' = left / 'r' = right
char snake_direction = 'r';
// array for displaying the snake	
// 1 means a part of the body of the snake is in that space, 2 that the apple is there and 
// 0 meaning that it is empty
// the height goes from 0 (top) to PIXEL_HEIGHT - 1 (bottom)
// the width goes from 0 (left) to PIXEL_WIDTH - 1 (right)
BYTE snake_matrix[PIXEL_HEIGHT][PIXEL_WIDTH];
// array for tracking the snake body with COORDINATE struct
struct COORDINATES snake_tracker[MAX_SNAKE_SIZE];
// variable that stores the current sieze of the snake
BYTE snake_size = 1;
// ----------------------------------------------------------------------------------------

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: initializes the system by declaring which pins will be for I/O, enables interruptions and configures them
-----------------------------------------------------------------------------------------------------------------------------------*/
void _init_system() {
	//configuring the I/O pins (1 for output and 0 for input)
	DDRB = 0b01111111;
	DDRC = 0b00011111;
	DDRD = 0x00;
	//enabling INT0 and INT1 interruptions and configuring their sense control
	EIMSK = 0b00000011;
	EICRA = 0b00001010;
	//enabling PCINT23 and PCINT13 interruptions
	PCICR |= (1<<PCIE1) | (1<<PCIE2);
	PCMSK2 |= (1<<PCINT23);
	PCMSK1 |= (1<<PCINT13);
    //enabling interruptions
	sei();
	// function used to set the random number pattern
	srand(time(NULL));
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: spawn the apple in coordinates where there is no snake
-----------------------------------------------------------------------------------------------------------------------------------*/
void spawn_apple() {
	BYTE h_position, w_position;
	do
	{
		// generate a random position inside the board to spawn the apple
		h_position = rand() % PIXEL_HEIGHT;
		w_position = rand() % PIXEL_WIDTH;
		// check if the current position does not collide with the snake
	} while (snake_matrix[h_position][w_position] == 1);
	// exit the loop when the generated position does not collide with the snake
	// spawn the apple in the board
	snake_matrix[h_position][w_position] = 2;
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: first, sets all spaces in the board to 0 and then spawns the snake 
-----------------------------------------------------------------------------------------------------------------------------------*/
void set_board() {
	for (size_t height = 0; height < PIXEL_HEIGHT; height++) {
		for (size_t width = 0; width < PIXEL_WIDTH; width++) {
			snake_matrix[height][width] = 0;
		}
	}
	//spawn snake
	snake_matrix[2][1] = 1;
	//store the snake head coordinates in snake_tracker
	snake_tracker[0].height_coordinate = 2;
	snake_tracker[0].width_coordinate = 1;
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: initializes all the necessary elements for the game to start (either for first time or when it is restarted)
-----------------------------------------------------------------------------------------------------------------------------------*/
void set_snake() {
	//sets the snake size to 1 which will also set the snake_tracker size to 1
	snake_size = 1;
	//set to the initial board distribution
	set_board();
	spawn_apple();
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: returns 1 if the snake will eat the apple in its next move or 0 if not
-----------------------------------------------------------------------------------------------------------------------------------*/
int snake_eats() {
	switch (snake_direction)
	{
	case 'r':
		//checks if at the right of the snake's head is the apple
		if (snake_matrix[snake_tracker[0].height_coordinate][snake_tracker[0].width_coordinate + 1] == 2) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'l':
		//checks if at the left of the snake's head is the apple
		if (snake_matrix[snake_tracker[0].height_coordinate][snake_tracker[0].width_coordinate - 1] == 2) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'u':
		//checks if at the top of the snake's head is the apple
		if (snake_matrix[snake_tracker[0].height_coordinate - 1][snake_tracker[0].width_coordinate] == 2) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'd':
		//checks if under the snake's head is the apple
		if (snake_matrix[snake_tracker[0].height_coordinate + 1][snake_tracker[0].width_coordinate] == 2) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	}
    return 1;
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: returns 1 if the snake will collides either with its own body or with the end of the board
-----------------------------------------------------------------------------------------------------------------------------------*/
int snake_collides() {
	switch (snake_direction)
	{
	case 'r':
		//checks if at the right of the snake's head is either the end of the board or part of its body
		if ((snake_tracker[0].width_coordinate == (PIXEL_WIDTH - 1)) || (snake_matrix[snake_tracker[0].height_coordinate][snake_tracker[0].width_coordinate + 1] == 1)) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'l':
		//checks if at the left of the snake's head is either the end of the board or part of its body
		if ((snake_tracker[0].width_coordinate == 0) || (snake_matrix[snake_tracker[0].height_coordinate][snake_tracker[0].width_coordinate - 1] == 1)) {
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'u':
		//checks if at the top of the snake's head is either the end of the board or part of its body
		if ((snake_tracker[0].height_coordinate == 0) || (snake_matrix[snake_tracker[0].height_coordinate - 1][snake_tracker[0].width_coordinate] == 1)){
			return 1;
		}
		else {
			return 0;
		}
		break;
	case 'd':
		//checks if under the snake's head is either the end of the board or part of its body
		if ((snake_tracker[0].height_coordinate == (PIXEL_HEIGHT - 1)) || (snake_matrix[snake_tracker[0].height_coordinate + 1][snake_tracker[0].width_coordinate] == 1)){
			return 1;
		}
		else {
			return 0;
		}
		break;
	}
    return 1;
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: first, check if the snake does not collide to then move it through the snake_matrix and update the coordinates
	of its body parts in the snake_tracker, secondly if the snake eats it will spawn another apple and increase the snake size by 1.
	If the snake collides, then it resets the game to the initial/beginning point
-----------------------------------------------------------------------------------------------------------------------------------*/
void move_snake() {
	//last_position is used to store the position of the lastly moved part of the snake body in the current iteration. This will be used
		//for moving the whole snake body and for growing the snake if it eats
	//temp is just for temporarily storing coordinates
	struct COORDINATES last_position, temp;
	//stores if the snake eats to then increase its size after moving it
	BYTE eats = snake_eats();
	//if snake does not collide
	if (!snake_collides()) {
		//first the head is moved
		last_position = snake_tracker[0];
		//this is to avoid having a lost 1 inside the snake_matrix if the snake has a size of 1
		snake_matrix[last_position.height_coordinate][last_position.width_coordinate] = 0;
		//moving the snake's head
		switch (snake_direction)
		{
		case 'r':
			snake_tracker[0].width_coordinate++;
			break;
		case 'l':
			snake_tracker[0].width_coordinate--;
			break;
		case 'u':
			snake_tracker[0].height_coordinate--;
			break;
		case 'd':
			snake_tracker[0].height_coordinate++;
			break;
		}
		snake_matrix[snake_tracker[0].height_coordinate][snake_tracker[0].width_coordinate] = 1;
		//this loops only runs if the snake has a size greater to 1
		//drag each remaining part of the snake's body to their new positions
		for (size_t it = 1; it < snake_size; it++){
			//drag to new position
			snake_matrix[last_position.height_coordinate][last_position.width_coordinate] = 1;
			//change coordinates to new ones and store the previous coordinates
			temp = last_position;
			last_position = snake_tracker[it];
			snake_tracker[it] = temp;
			//avoiding having a lost 1 inside the snake_matrix if this iteration is from the last body part
			snake_matrix[last_position.height_coordinate][last_position.width_coordinate] = 0;
		}
		//if snake eats
		if (eats) {
			//adds the new part of the snake at last_position and increases the snake size
			snake_matrix[last_position.height_coordinate][last_position.width_coordinate] = 1;
			snake_size++;
			//stores the coordinates of the new snake body part
			snake_tracker[snake_size - 1] = last_position;
			//spawn a new apple
			spawn_apple();
			eats = 0;
		}
	}
	//if snake collides
	else {
		set_snake();
	}
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: send a combination of 1s and 0s to the output registers to display each row (one at a time) of 
	the matrix.
	*This function works for a display of 5x7 leds, if the display has different dimensions, just adapt the following loops to display
	 your snake_matrix correctly
-----------------------------------------------------------------------------------------------------------------------------------*/
void display_board() {
	/*this loop iterates through the display process for an arbitrary number of repetitions so it is visible for the human eye.
		In this case the display loop repeats for 200 times, this number and its display time mostly depends on the frequency of
		the system.
		*the greater the number of iterations is, the slower the snake will move */
	for(size_t c = 0; c <= 200; c++){
        //displaying each row (from top to bottom) of the snake_matrix
		for (size_t itv = 0; itv < PIXEL_HEIGHT; itv++){
            switch (itv)
            {
            //top row
			case 0:
				PORTC = 0b00000001;
                break;
            case 1:
                PORTC = 0b00000010;
                break;
            case 2:
                PORTC = 0b00000100;
                break;
            case 3:
                PORTC = 0b00001000;
                break;
            //botton row
			case 4:
                PORTC = 0b00010000;
                break;
            }
            //set the register to 0
			PORTB = 0x00;
			/*goes through the snake_matrix at the given row from left to right
			  and if there is either part of the snake or the apple, a 1 is set
			  at that specific coordinate to be displayed in the display*/
            for (size_t ith = 0; ith < PIXEL_WIDTH; ith++){
                //allways move the bits of the register to the left by 1 on each iteration
				PORTB = PORTB << 1;
                //if the snake or the apple is in the given coordinate
				if (snake_matrix[itv][ith] >= 1) {
                    PORTB = PORTB | 1;
                }
            }
            /*due to an specific problem, to display correctly the snake_matrix in the display, the
			  output register value is inverted*/
			PORTB = ~PORTB;
			//this delay is needed to make the displaying visible
			_delay_ms(1);
        }
    }
}

/*-----------------------------------------------------------------------------------------------------------------------------------
- Functionality: all the following ISR code blocks set the direction of the snake depending on the selected interruption (which is 
	triggered by a pressed button in the hardware by the user)
-----------------------------------------------------------------------------------------------------------------------------------*/
ISR(INT0_vect){
    snake_direction = 'u';
}
ISR(INT1_vect){
    snake_direction = 'd';
}
ISR(PCINT2_vect){

    snake_direction = 'r';
}
ISR(PCINT1_vect){
    snake_direction = 'l';
}

//main function
int main(void) {
    //initialize the system before beginning the game
	_init_system();
    //sets the snake_matrix and elements to their initial state
	set_snake();
    while(1){
        //display the board for a given time depending on the implementation of display_board
		display_board();  
		move_snake();
    }
    return 0;
}