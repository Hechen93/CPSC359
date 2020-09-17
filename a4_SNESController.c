/*Harrison Chen - 10075185
Assignment 4 - CPSC 359 - W2020
Prepared for Dr. Manzara & Desmond Larsen-Rosner
*/

//Include necessary header files
#include "gpio.h"
#include "systimer.h"     //Don't forget this; systimer header brings in microsecond_delay function
#include "uart.h"
#include "framebuffer.h"

//***************************************************************************************Definitions******************************************************************************8*
#define CLOCK 11
#define LATCH 9
#define DATA 10

//**************************************************************************************Function Prototypes****************************************************************************
void init_LATCH_as_output();
void set_LATCH();
void clear_LATCH();
void init_CLOCK_as_output();
void riseedge_CLOCK();
void falledge_CLOCK();
void init_DATA_as_input();
unsigned int get_DATAPIN();
unsigned short get_CONTROLLERDATA();




//***************************************************************************************Main function**********************************************************************************
void startGame()
{

    unsigned short data, currentState = 0xFFFF;


    //Initialize controller input and outputs. Note that the controller will start with a clock rising edge.
    uart_init();
    init_CLOCK_as_output();
    init_LATCH_as_output();
    init_DATA_as_input();
    clear_LATCH();
    riseedge_CLOCK();


    //While running, the controller will continuously get data from the controller
    while(1){

        //Pull data from the controller and evaluate what the data is (which button was pressed)
        data = get_CONTROLLERDATA();

        // Compare the state of the controller to the state of the data being inputted (which is the button being pressed)
    	if (data != currentState) {

            // If the start button is pressed from the controller, signal is sent to the CPU/GPU to display player
            if (data == 3){

                displayPlayer();
            }

            // Record the state of the controller
    	    currentState = data;
    	}

        	// Delay 1/30th of a second
        	microsecond_delay(33333);


    }









}


//********************************************************************************Functions*******************************************************************************

void init_LATCH_as_output() //GPIO 9 is for the LATCH output
{
    register unsigned int r;


    // Get GPIO Function Select Register 0
    r = *GPFSEL0;


    // Clear bits for GPIO pin 9
    r &= ~(0x7 << 27);

    //Set GPIO pin 9 register to '1' to send outputs
    r |= (0x1 << 27);

    //Store back to GPIO Select Register 0
    *GPFSEL0 = r;


    // Disable pull-up/pull-down for pin 9.
    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Clear bits in fields 18 as these go back to PIN 9.
    r &= ~(0x3 << 18);

    // Store to register
    *GPPUPPDN0 = r;
}


void set_LATCH()
{
    register unsigned int r;

    // Set PIN field in GPIO pin 9 to a 1 using the GPSET0 register
    r = (0x1 << 9);
    *GPSET0 = r;
}


void clear_LATCH()
{
    register unsigned int r;

    // Place a 1 inside the GPCLR0 register
    r = (0x1 << 9);
    *GPCLR0 = r;
}


void init_CLOCK_as_output()    //GPIO 11 will be for the clock output. This is used to synchronize clock pulses
{
    register unsigned int r;


    // Get GPIO Function Select Register 1
    r = *GPFSEL1;

    //Clear the bits in field 3 which maps to pin 11.
    r &= ~(0x7 << 3);

    //Set GPIO pin 11 register to '1' to send outputs
    r |= (0x1 << 3);

    //Store back to GPIO Select Register 1
    *GPFSEL1 = r;


    // Disable pull-up/pull-down for pin 11.
    // Get the current bit pattern of the GPPUPPDN0 register (use GPPUPPDN0 since it covers pins 0-15)
    r = *GPPUPPDN0;

    // Clear bits in field 22 as these go back to PIN 11.
    r &= ~(0x3 << 22);

    //Store back to register
    *GPPUPPDN0 = r;
}

void riseedge_CLOCK()
{
    register unsigned int r;

    // Set PIN field in GPIO pin 11 to a 1 using the GPSET0 register
    r = (0x1 << 11);
    *GPSET0 = r;
}


void falledge_CLOCK()
{
    register unsigned int r;

    // Place a 1 inside the GPCLR0 register
    r = (0x1 << 11);
    *GPCLR0 = r;
}

void init_DATA_as_input()
{
    register unsigned int r;


    // Get GPIO Function Select Register 1
    r = *GPFSEL1;

    //Clear the bits in field 3 which maps to pin 10.
    r &= ~(0x7 << 0);

    // Save back to register memory
    *GPFSEL1 = r;


    // Disable pull-up/pull-downs for pin 10.

    // Get register contents
    r = *GPPUPPDN0;

    // Clear out bits 20-21 to disable PUPs and PDowns
    r &= ~(0x3 << 20);

    // Save to register memory
    *GPPUPPDN0 = r;
}

unsigned int get_DATAPIN()
{
    register unsigned int r;


    // Get contents of GPIO Pin Read Register 0
    r = *GPLEV0;

    // Read back value of Read Register in *GPLEV0 (a 0 if low, or a 1 if high)
    return ((r >> 10) & 0x1);
}

unsigned short get_CONTROLLERDATA() //The clock cycle must be on high for this to work (set in main function)
{
    int i;
    unsigned short data = 0; //Use short 16-bit integer as bits 0-11 will provide feedback from the controller. Return this value back to main function for button input
    unsigned int userInput; //Short 16-bit integer from SNES controller. 0-11 are valid inputs, 12-15 are not used.



    //First we hook up the controller so that we can read inputs coming in from data line
    //These inputs from the player will be read into data variable and returned back to the main function.
    //We then unhook the controller until this function is called again (disable the data line until it's called again).
    set_LATCH();
    microsecond_delay(12);
    clear_LATCH();

    // We will be checking for controller input on a 16-bit cycle, since SNES controller is using 16 bits. The clock pulse will need to be separated into 2 cycles;
    // One for the falling edge to read in the data from the data line, and one to create a rising edge to "stop" reading the data, so we can get data again on the
    // Next cycle.
    for (i = 0; i < 16; i++) {

    // Split the clock cycle into two.
	microsecond_delay(6);

	// Force a falling edge to read input from the controller
	falledge_CLOCK();

	// Read the value from data line
	userInput = get_DATAPIN();

	// Check to see if a button on the controller was pressed. If so, switch value to a '1' in data variable that corresponds to the controller button being pressed.
	if (userInput == 0) {
	    data |= (0x1 << i); //Shifting i spots corresponds to the button being pressed.
	}

	// Wait another half cycle to have clock on rising edge. The controller will get ready to send the next button pressed.
	microsecond_delay(6);

	riseedge_CLOCK();
    }

    // Return the button pressed from the controller back to the main function
    return data;
}
