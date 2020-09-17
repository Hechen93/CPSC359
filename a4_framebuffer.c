/*Harrison Chen - 10075185
Assignment 4 - CPSC 359 - W2020
Prepared for Dr. Manzara & Desmond Larsen-Rosner
*/

//*****************************************************************Headers**************************************************************************
#include "uart.h"
#include "mailbox.h"
#include "SNESController.h"

//*****************************************************************Definitions*********************************************************************
#define BLACK     0x00000000
#define WHITE     0x00FFFFFF
#define RED       0x00FF0000
#define GREEN     0x00008000
#define START_BUTTON    0x00000008
#define RIGHT_BUTTON    0x00000080
#define LEFT_BUTTON     0x00000040
#define DOWN_BUTTON     0x00000020
#define UP_BUTTON       0x00000010

//******************************************************************Constants********************************************************************
#define FRAMEBUFFER_WIDTH      1024  // in pixels
#define FRAMEBUFFER_HEIGHT     768   // in pixels
#define FRAMEBUFFER_DEPTH      32    // bits per pixel (4 bytes per pixel)
#define FRAMEBUFFER_ALIGNMENT  4     // framebuffer address preferred alignment
#define VIRTUAL_X_OFFSET       0
#define VIRTUAL_Y_OFFSET       0
#define PIXEL_ORDER_BGR        0     // needed for the above color codes

// Frame buffer global variables
unsigned int fbWidth, fbHeight, fbPitch;
unsigned int fbDepth, fbPOrder, fbBSize;
unsigned int *frameBuffer;

//  Global pointer to reach maze array address
unsigned int *mazeptr;

void startFrameBuffer()
{
    //SET tags get processed before GET tags (put these ones first)
    //TEST tags are not processed with SET/GET tags
    //MUST SPECIFY A BUFFER ALLOCATE TAG (40001)
    //https://jsandler18.github.io/extra/framebuffer.html
    mailbox_buffer[0] = 35 * 4;
    mailbox_buffer[1] = MAILBOX_REQUEST;                    //Need this one

    mailbox_buffer[2] = TAG_SET_PHYSICAL_WIDTH_HEIGHT;      //Need this one
    mailbox_buffer[3] = 8;
    mailbox_buffer[4] = 0;                                  //Zero indicates that we must make a request (request code)
    mailbox_buffer[5] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[6] = FRAMEBUFFER_HEIGHT;

    mailbox_buffer[7] = TAG_SET_VIRTUAL_WIDTH_HEIGHT;       //Need this one
    mailbox_buffer[8] = 8;
    mailbox_buffer[9] = 0;
    mailbox_buffer[10] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[11] = FRAMEBUFFER_HEIGHT;

    mailbox_buffer[12] = TAG_SET_VIRTUAL_OFFSET;            //Need this one - sets buffer portion of display device, not actual buffer
    mailbox_buffer[13] = 8;
    mailbox_buffer[14] = 0;
    mailbox_buffer[15] = VIRTUAL_X_OFFSET;
    mailbox_buffer[16] = VIRTUAL_Y_OFFSET;

    mailbox_buffer[17] = TAG_SET_DEPTH;                     //Need this one
    mailbox_buffer[18] = 4;
    mailbox_buffer[19] = 0;
    mailbox_buffer[20] = FRAMEBUFFER_DEPTH;

    mailbox_buffer[21] = TAG_SET_PIXEL_ORDER;               //Need this one - sets the order of RGB values
    mailbox_buffer[22] = 4;
    mailbox_buffer[23] = 0;
    mailbox_buffer[24] = PIXEL_ORDER_BGR;

    mailbox_buffer[25] = TAG_ALLOCATE_BUFFER;
    mailbox_buffer[26] = 8;
    mailbox_buffer[27] = 0;

    mailbox_buffer[28] = FRAMEBUFFER_ALIGNMENT;
    mailbox_buffer[29] = 0;

    mailbox_buffer[30] = TAG_GET_PITCH;
    mailbox_buffer[31] = 4;
    mailbox_buffer[32] = 0;
    mailbox_buffer[33] = 0;

    mailbox_buffer[34] = TAG_LAST;


    // Make a mailbox request using the above mailbox data structure
    if (mailbox_query(CHANNEL_PROPERTY_TAGS_ARMTOVC)) {
	// If here, the query succeeded, and we can check the response

	// Get the returned frame buffer address, masking out 2 upper bits
        mailbox_buffer[28] &= 0x3FFFFFFF;
        frameBuffer = (void *)((unsigned long)mailbox_buffer[28]);

	// Read the frame buffer settings from the mailbox buffer -------This part is also important since it initializes the global variables
        fbWidth = mailbox_buffer[5];
        fbHeight = mailbox_buffer[6];
        fbPitch = mailbox_buffer[33];
	fbDepth = mailbox_buffer[20];
	fbPOrder = mailbox_buffer[24];
	fbBSize = mailbox_buffer[29];
    }
}


void drawSquare(int rowStart, int columnStart, int squareSize, unsigned int color)
{
    int row, column, rowEnd, columnEnd;
    unsigned int *pixel = frameBuffer;


    // Calculate where the row and columns end
    rowEnd = rowStart + squareSize;
    columnEnd = columnStart + squareSize;

    // Draw the square row by row, from the top down
    for (row = rowStart; row < rowEnd; row++) {
	// Draw each pixel in the row from left to right
        for (column = columnStart; column < columnEnd; column++) {
	    //Insert color value into the FB array address
            pixel[(row * fbWidth) + column] = color;
        }
    }
}

void drawMaze(int numberOfRows, int numberOfColumns, int squareSize,
		       unsigned int color1, unsigned int color2)
{
    int i, j;

    int maze[numberOfRows][numberOfColumns] = {

        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 3},
        {1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    }

    // Draw the rows from the top down
    for (i = 0; i < numberOfRows; i++) {

        for (j = 0; j < numberOfColumns; j++){

            switch(maze[i][j]){

                //Draws the white squares for the maze paths
                case '0':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);

                //Draws the black squares for the maze walls
                case '1':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color2);

                //Draws the white square for the start space for the player
                case '2':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);

                //Draws the white square for the finish space to leave the maze
                case '3':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);
            }
        }
    }
}


void displayFrameBuffer()
{
    int squareSize, numberOfRows, numberOfColumns;


    //This is the pixel size of the square
    squareSize = 64;

    // Calculate the number of rows and columns
    numberOfRows = fbHeight / squareSize;
    numberOfColumns = fbWidth / squareSize;

    // Draw maze
    drawMaze(numberOfRows, numberOfColumns, squareSize, WHITE, BLACK);
}


//(arr+i*n) + j)
//Function that initializes the RED PLAYER at the beginning of the maze
void displayPlayer(unsigned char input)
{

    //Init maze pointer to get address of maze in memory, init an int variable to store memory offset value
    int *mazeptr;
    int displayPlayerOffset;

    //Validate that the input is a start button
    if(input == START_BUTTON){

        *mazeptr = &maze;       //Get maze address from memory

        displayPlayerOffset = (*mazeptr + 1 * fbWidth) + 0;     //Calculate the address of the maze start (hard coded)

        *mazeptr += displayPlayerOffset;        //Find the memory address of the maze

        *mazeptr = RED;     //Color the space RED

    }

}


//Function that moves player through the maze depending on the different inputs
void movePlayer(unsigned char input)
{

    int *mazeptr;           //Initializing pointer to store maze memory
    int playerOffset;       //Init. offset for pixel calculation
    int *storeplayer;

    int squareSize = 64;    //Defines the pixel square size to pass to other functions



    //Offset of frame
    int row, column, newRowEnd, newColumnEnd;
    unsigned int *pixel = frameBuffer;

    int maze[mazeRow][mazeColumn] = {

        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 3},
        {1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    };


    switch(input){

        //The SNES controller is sending an UP signal
        case 'UP_BUTTON':




            newRowEnd = newRowStart - squareSize;
            newColumnEnd = newColumnStart + squareSize;

            playerOffset = (*mazeptr + newRowEnd * fbWidth) + newColumnEnd;

            *mazeptr = &maze[newRowEnd][newColumnEnd];

            //If the square up from the character is a wall, then the character cannot go through the wall
            if(*mazeptr == BLACK){

                asm volatile("nop");

            }

            else *mazeptr += playerOffset;  //Moves player to the next square UP????
            *mazeptr = RED;

            *storeplayer = &mazeptr;
            }

            deactivateSquare(newRowStart, newColumnStart);
            updateMaze(newRowEnd, newColumnEnd, squareSize, WHITE, BLACK);

            //The SNES controller is sending an DOWN signal
        case 'DOWN_BUTTON':
            newRowEnd = newRowStart + squareSize + squareSize;
            newColumnEnd = newColumnStart + squareSize;

            playerOffset = (*mazeptr + newRowEnd * fbWidth) + newColumnEnd;

            //If the square up from the character is a wall, then the character cannot go through the wall
            if(*mazeptr == BLACK){

                asm volatile("nop");

            }
            else *mazeptr += playerOffset;  //Moves player to the next square DOWN????
            *mazeptr = RED;

            *storeplayer = &mazeptr;
            }

            deactivateSquare(newRowStart, newColumnStart);
            updateMaze(newRowEnd, newColumnEnd, squareSize, WHITE, BLACK);

                //The SNES controller is sending a LEFT signal
        case 'LEFT_BUTTON':
            newRowEnd = newRowStart + squareSize;
            newColumnEnd = newColumnStart - squareSize;

            playerOffset = (*mazeptr + newRowEnd * fbWidth) + newColumnEnd;

            //If the square up from the character is a wall, then the character cannot go through the wall
            if(*mazeptr == BLACK){

                asm volatile("nop");

            }

            else *mazeptr += playerOffset;
            *mazeptr = RED;

            *storeplayer = &mazeptr;
            }

            deactivateSquare(newRowStart, newColumnStart);
            updateMaze(newRowEnd, newColumnEnd, squareSize, WHITE, BLACK);

            //The SNES controller is sending a RIGHT signal
        case 'RIGHT_BUTTON':
            newRowEnd = newRowStart - squareSize;
            newColumnEnd = newColumnStart + squareSize + squareSize;

            playerOffset = (*mazeptr + newRowEnd * fbWidth) + newColumnEnd;

            //If the square up from the character is a wall, then the character cannot go through the wall
            if(*mazeptr == BLACK){

                asm volatile("nop");

            }
            //Color the square to the right of the character
            else *mazeptr += playerOffset;
            *mazeptr = RED;

            *storeplayer = &mazeptr;
            }

            deactivateSquare(newRowStart, newColumnStart);
            updateMaze(newRowEnd, newColumnEnd, squareSize, WHITE, BLACK);
            finishGame();
    }


}

void deactivateSquare(int rowStart, int columnStart) //This turns off the previous square's pixels once the player has moved and recolors the pathway
{
    int row, column, rowEnd, columnEnd;
    unsigned int *pixel = frameBuffer;

    squareSize = 64;


    // Calculate where the row and columns end
    rowEnd = rowStart + squareSize;
    columnEnd = columnStart + squareSize;

    // Draw the square row by row, from the top down
    for (row = rowStart; row < rowEnd; row++) {
	// Draw each pixel in the row from left to right
        for (column = columnStart; column < columnEnd; column++) {
	    //Insert color value into the FB array address
            pixel[(row * fbWidth) + column] = WHITE;
        }
    }


}

void updateMaze(int numberOfRows, int numberOfColumns, int squareSize,
		       unsigned int color1, unsigned int color2)                                   //Redraws maze with updated positioning of the player
{

    int *playerCoord;
    int playerOffset;

    *playerCoord = &mazeptr;                    //Gets the address of the player coordinates in the maze

    playerOffset = (*mazeptr + newRowEnd * fbWidth) + newColumnEnd;         //Calculates the maze offset location for the player


    // Redraws the whole map. Once the map is redrawn, then we will update the maze with the new player coordinates
    int maze[numberOfRows][numberOfColumns] = {

        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {2, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1},
        {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
        {1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1},
        {1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 3},
        {1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
        {1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    }


    for (i = 0; i < numberOfRows; i++) {

        for (j = 0; j < numberOfColumns; j++){

            switch(maze[i][j]){

                //Draws the white squares for the maze paths
                case '0':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);

                //Draws the black squares for the maze walls
                case '1':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color2);

                //Draws the WHITE square for the start space for the player
                case '2':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);

                //Draws the white square for the finish space to leave the maze
                case '3':

                        drawSquare(i * squareSize, j * squareSize, squareSize, color1);
            }
        }

        *playerCoord += playerOffset;               //Calculates the new player coordinate
        *playerCoord = RED;                         //Colors new player coordinate RED

}



void finishGame()           //Checks to see if player has reached the end of the maze. If not, the function does nothing
{
    int *playerAddress;

    *playerAddress = *mazeptr;

    if(&playerAddress == 3){

        *playerAddress = GREEN;             //Initializes the player to green

        deactivateSquare();                 //Clear color of previous square to WHITE

        uart_puts("Press 'Start' to replay");   //Outputs restart message to screen
    }
}
