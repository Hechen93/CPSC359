/*Harrison Chen - 10075185
Assignment 4 - CPSC 359 - W2020
Prepared for Dr. Manzara & Desmond Larsen-Rosner
*/

// This program demonstrates how to initialize a frame buffer for a
// 1024 x 768 display, and how to draw on it using a simple checker board
// pattern.

// Included header files
#include "uart.h"
#include "framebuffer.h"
#include "SNESController.h"


void main()
{
    char c;

    // Initialize the UART terminal
    uart_init();

    // Initialize the frame buffer
    initFrameBuffer();

    // Draw on the frame buffer and display it
    displayFrameBuffer();

    //Start SNES Controller using function from SNESController.h
    startGame();


    // Loop forever, echoing characters received from the console
    // on a separate line with : : around the character
    while (1) {
        // Wait for a character input from the console
	c = uart_getc();


    movePlayer();       //Continuously calls movePlayer function based on controller polling. Everything will be updated from movePlayer function.
    
        // Output the character
        uart_puts(":");
        uart_putc(c);
        uart_puts(":\n");
    }
}
