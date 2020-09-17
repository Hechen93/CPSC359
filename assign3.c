/*Harrison Chen (10075185)
Assignment #3 - CPSC 359
W2020 - Prepared for Dr. Leonard Manzara and Desmond Larsen-Rosner

Objective: The following C program creates a blinking LED pattern using GPIO pins from the Raspberry Pi 4.



*/


//////////////////////Header Files//////////////////////////////////////////////////
#include "gpio.h"
#include "uart.h"

////////////////////Function Prototypes////////////////////////////////////////////
void initOut_GPIO17();
void initOut_GPIO27();
void initOut_GPIO22();
void set_GPIO17();
void clr_GPIO17();
void set_GPIO27();
void clr_GPIO27();
void set_GPIO22();
void clr_GPIO22();
unsigned int get_GPIO23();
unsigned int get_GPIO24();

//////////////////////Main Function//////////////////////////////////////////////



void main()
{
    //Unsigned variable register declarations
    register unsigned int i;
    register unsigned int pullA;
    register unsigned int pullB;


    //Set up pins 17, 27, and 22 for LED output
    initOut_GPIO17();
    initOut_GPIO27();
    initOut_GPIO22();



    //Setting pattern 1 in a forever while loop
    while(1){

        //Get values for switches
        pullA = get_GPIO23();
        pullB = get_GPIO24();

        //Switch statement that determines the speed of the LED blink pattern
        switch(pullB){

            //Case 0 is the default pulled low, meaning regular LED speed
            case '0':

                //Maintains the same blink speed if Switch B isn't pressed
                asm volatile("nop");

            //Case 1 is when the Switch is pulled high and speed is doubled
            case '1':
                //Switch statement that determines the pattern for when Switch A
                //Is pulled low
                switch(pullA){

                    //Switch pattern when switch A is pulled high to low from 1 to 0
                    //This will cause the Pi to display the next pattern
                    case '0':
                        //set GPIOs 17 and 22 (LEDs 1 and 3)
                        set_GPIO17();
                        set_GPIO22();

                        //Reset GPIO 27 (Turn off LED 2)
                        clr_GPIO27();

                        //Busy loop delay activation between light activation
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Reset GPIOs 17 and 22 (LEDs 1 and 3)
                        clr_GPIO17();
                        clr_GPIO22();

                        //Turn on GPIO 27 (LED 2)
                        set_GPIO27();

                        //Busy loop delay activation between light activation
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                    //LED sequence pattern for when switch A is in the high pull state
                    //This is our default pattern
                    case '1':
                        //Turns on LED 1
                        set_GPIO17();

                        //Busy loop delay activation between LED 1 and LED 2
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Turns off LED 1
                        clr_GPIO17();

                        //Busy loop delay activation before turning on LED 2
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Turns on LED 2
                        set_GPIO27();

                        //Busy loop delay activation before turning off LED 2
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Turn off LED 2
                        clr_GPIO27();

                        //Busy loop delay before turning on LED 3
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Turn on LED 3
                        set_GPIO22();

                        //Busy loop delay before turning off LED 3
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                        }

                        //Turn off LED 3
                        clr_GPIO22();

                        //Busy loop delay before turning on LED 1
                        i = 0x003FFFFF;
                        while(i--){
                            asm volatile("nop");
                    }
                }




        }






    }







}

/////////////////////////////////////////////Functions///////////////////////////////////////////////////
void initOut_GPIO17()
{
    register unsigned int r;
    r = *GPFSEL1;

    //Clear bits and disable PU/PD for GPIO 17
    r &= ~(0x7 << 21);
    *GPFSEL1 = r;

    //Disabling the pullup/pulldown for GPIO 17
    r = *GPPUPPDN1;
    r &= ~(0x3 << 2);
    *GPPUPPDN1 = r;

}

void initOut_GPIO27()
{
    register unsigned int s;
    s = *GPFSEL2;

    //Clear bits and disable PU/PD for GPIO 27
    s &= ~(0x7 << 21);
    *GPFSEL2 = s;

    //Disabling the pullup/pullodwn for GPIO 27
    s = *GPPUPPDN1;
    s &= ~(0x3 << 22);
    *GPPUPPDN1 = s;
}

void initOut_GPIO22()
{
    register unsigned int t;
    t = *GPFSEL2;

    //Clear bits and disable PU/PD for GPIO 22
    t &= ~(0x7 << 6);
    *GPFSEL2 = t;

    //Disabling the pullup/pulldown for GPIO 22
    t = *GPPUPPDN1;
    t &= ~(0x3 << 12);
    *GPPUPPDN1 = t;


}

void set_GPIO17()
{
    register unsigned int r;

    //Place 1 in SET17 field to activate LED
    r = (0x1 << 17);
    *GPSET0 = r;


}

void clr_GPIO17()
{
    register unsigned int r;

    //Clear 1 in SET17 field to deactivate LED
    r = (0x1 << 17);
    *GPCLR0 = r;

}

void set_GPIO27()
{
    register unsigned int r;

    //Place 1 in SET27 field to activate LED
    r = (0x1 << 27);
    *GPSET0 = r;


}

void clr_GPIO27()
{
    register unsigned int r;

    //Clear 1 in SET27 field to deactivate LED
    r = (0x1 << 27);
    *GPCLR0 = r;

}

void set_GPIO22()
{
    register unsigned int r;

    //Place 1 in SET22 field to activate LED
    r = (0x1 << 22);
    *GPSET0 = r;


}

void clr_GPIO22()
{
    register unsigned int r;

    //Clear 1 in SET22 field to deactivate LED
    r = (0x1 << 22);
    *GPCLR0 = r;

}

unsigned int get_GPIO23()
{
    register unsigned int r;

    //Get current level of Pin 23
    r = *GPLEV0;

    //Get value on pin 23, and AND it to find if high or low
    return ((r >> 23) & 0x1);
}

unsigned int get_GPIO24()
{
    register unsigned int r;

    //Get current level of Pin 24
    r = *GPLEV0;

    //Get value on pin 24, and AND it to find if high or low
    return ((r >> 24) & 0x1);
}
