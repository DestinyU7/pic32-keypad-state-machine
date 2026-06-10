/*===================================CPEG222====================================
 * Program:		Project2_Template.c
 * Authors: 	Timothy M and Griffin T
 * Date: 		9/22/2023
 * Description: A template that includes examples for multiple topics:
 *                  - Using multiple buttons
 *                  - Using the SSD
 *                  - Using the ACL for random seed generation
 *                  - Using a switch statement for modes
 * Input: Buttons U & D
 * Output: LEDs and SSD
==============================================================================*/
/*------------------ Board system settings. PLEASE DO NOT MODIFY THIS PART ----------*/
#ifndef _SUPPRESS_PLIB_WARNING //suppress the plib warning during compiling
#define _SUPPRESS_PLIB_WARNING
#endif
#pragma config FPLLIDIV = DIV_2 // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_20 // PLL Multiplier (20x Multiplier)
#pragma config FPLLODIV = DIV_1 // System PLL Output Clock Divider (PLL Divide by 1)
#pragma config FNOSC = PRIPLL   // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF    // Secondary Oscillator Enable (Disabled)
#pragma config POSCMOD = XT     // Primary Oscillator Configuration (XT osc mode)
#pragma config FPBDIV = DIV_2   // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/8)
    
/*----------------------------------------------------------------------------*/

#include <xc.h> //Microchip XC processor header which links to the PIC32MX370512L header
#include <stdio.h>
#include "config.h" //Also need to include i2c.h, i2c.c, utils.h, utils.c in the project header files.
#include "lcd.h"
#include "ssd.h"
#include "acl.h"

/* --------------------------- Forward Declarations-------------------------- */
void initialize_ports();
void initialize_output_states();
void set_random_seed();
void toggle_LEDs();
void toggle_SSD();
void delay_ms(int ms);
void handle_raw_button_presses();
int correctAnswers = 0;

/* -------------------------- Definitions------------------------------------ */
#define SYS_FREQ (80000000L)
#define _80Mhz_ (80000000L)
#define LOOPS_NEEDED_TO_DELAY_ONE_MS_AT_80MHz 1426
#define LOOPS_NEEDED_TO_DELAY_ONE_MS (LOOPS_NEEDED_TO_DELAY_ONE_MS_AT_80MHz * (SYS_FREQ / _80Mhz_))
#define BtnC_RAW PORTFbits.RF0
#define BtnU_RAW PORTBbits.RB1
#define BtnD_RAW PORTAbits.RA15 
#define BtnL_RAW PORTBbits.RB0 
#define BtnR_RAW PORTBbits.RB8 

#define TRUE 1
#define FALSE 0
#define BUTTON_DEBOUNCE_DELAY_MS 20
#define SSD_EMPTY_DIGIT 31

int buttonsLocked = FALSE;

char ssdIsOn = 0;

char pressedUnlockedBtnC = FALSE;
char pressedUnlockedBtnU = FALSE;
char pressedUnlockedBtnD = FALSE; // Add a variable for button Down
char pressedUnlockedBtnL = FALSE; // Add a variable for button Left
char pressedUnlockedBtnR = FALSE; // Add a variable for button Right


enum Mode {
    MODE_1,
    MODE_2,
    MODE_3,
    MODE_4,
    MODE_5
};

enum Mode currentMode = MODE_1; // Initialize to Mode 1
int main(void) {
    initialize_ports();
    initialize_output_states();
    set_random_seed();
    
    int currentMode = MODE_1; // Initialize to Mode 1
    int iteration = 3;  // Initialize the iteration counter
    int directions[3];  // Array to store the directions

    while (1) {
        // Check for button presses
        handle_raw_button_presses();

        switch (currentMode) {
            case MODE_1:
                // Display the initial screen for Mode 1
                LCD_WriteStringAtPos("Ready", 0, 0);
                LCD_WriteStringAtPos("Press BtnC", 1, 0);
                LATA &= 0xFF00;
                
                // Clear the SSD
                SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
                
                if (pressedUnlockedBtnC) {
                    LCD_DisplayClear();
                    currentMode = MODE_2;
                }
                break;

            case MODE_2:
                // Mode 2 - Watch the Sequence
                // Clear display to stop collisions
                // Display "Watch the Sequence" on the LCD
                ;
                
                LCD_WriteStringAtPos("      Watch the", 0, 0);
                LCD_WriteStringAtPos("      Sequence", 1, 0);

                for (int i = 0; i < iteration; i++) {
                    // Call the function to display a random direction on the SSD and store it
                    directions[i] = displayRandomDirection();

                    // Delay for a certain amount of time (e.g., 250 ms or as needed)
                    delay_ms(250);
                }

                currentMode = MODE_3;
                ;
                break;

           case MODE_3:
{
    if (pressedUnlockedBtnC) {
        currentMode = MODE_2; // Reset to MODE_2
        pressedUnlockedBtnC = FALSE; // Reset BtnC state

        // Clear the LCD and SSD
        LCD_DisplayClear();
        SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);

        break;
    }

    // Rest of your MODE_3 code remains the same
    LCD_WriteStringAtPos("      repeat the", 0, 0);
    LCD_WriteStringAtPos("      Sequence", 1, 0);
    SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);//clear ssd
    static int remainingTries = 3; // Initialize remaining tries to 3 (static to persist between iterations)
    static int i = 0; // Initialize i to 0 to track the current direction
    int directions[3]; // Array to store the directions

    // Display remaining tries on the SSD
    SSD_WriteDigits(remainingTries, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
    
    // Handle button presses
    handle_raw_button_presses();

    int playerDirection = -1; // Initialize playerDirection to an invalid value

    if (pressedUnlockedBtnU) {
        playerDirection = 0; // Up
    } else if (pressedUnlockedBtnD) {
        playerDirection = 1; // Down
    } else if (pressedUnlockedBtnL) {
        playerDirection = 2; // Left
    } else if (pressedUnlockedBtnR) {
        playerDirection = 3; // Right
    }

    if (playerDirection != -1) {
        // Check if the player's input matches the stored direction
        if (playerDirection == directions[i]) {
            // Correct input
            i++; // Move to the next direction in the array

            if (i == 3) {
                LCD_DisplayClear();
                currentMode = MODE_4; // Transition to Mode 4 (Correct) after all directions are correct
            }
        } else {
            // Incorrect input
            remainingTries--; // Decrement remaining tries

            // Check if the player has no more tries
            if (remainingTries == 0) {
                
                currentMode = MODE_5; // Transition to Mode 5 (Failed)
            }
        }

        // Reset button presses
        pressedUnlockedBtnU = FALSE;
        pressedUnlockedBtnD = FALSE;
        pressedUnlockedBtnL = FALSE;
        pressedUnlockedBtnR = FALSE;
    }

    // Check if the player has no more tries
    if (remainingTries == 0) {
        // The player has no more tries, transition to Mode 5 (Failed)
        
        currentMode = MODE_5;
    }

    // If all directions have been displayed, generate new random directions
    if (i == 0) {
        directions[0] = displayRandomDirection();
        directions[1] = displayRandomDirection();
        directions[2] = displayRandomDirection();
    }

    break;
}




            case MODE_4:
                  
                LCD_WriteStringAtPos("     YES     ", 0, 0);
                SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
                break;

            case MODE_5:
                 
                LCD_DisplayClear();
                SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
                
                
                break;
        }
    }
}








void initialize_ports()
{
    DDPCONbits.JTAGEN = 0; // Statement is required to use Pin RA0 as IO
    TRISBbits.TRISB1 = 1;//bntu
    ANSELBbits.ANSB1 = 0;
    TRISBbits.TRISB0 = 1; //btnl
    TRISBbits.TRISB8 = 1;//btnr
    TRISFbits.TRISF0 = 1;
    TRISAbits.TRISA15 = 1;//btnd
    TRISFbits.TRISF4 = 1; //btnc
    ANSELBbits.ANSB0 = 0;
    ANSELBbits.ANSB8 = 0;
    
    TRISA &= 0xFF00;
    ACL_Init();
    SSD_Init();
    LCD_Init();
}

void initialize_output_states()
{
    LCD_WriteStringAtPos("", 0, 0);
    LCD_WriteStringAtPos("", 1, 0);
    LATA &= 0xFF00;
    // clear the SSD
    SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
}

void set_random_seed()
{
    float rgACLGVals[3];
    ACL_ReadGValues(rgACLGVals);
    int seed = rgACLGVals[0] * 10000;
    srand((unsigned)seed);
}

void toggle_LEDs()
{
    short topEigthLATABits = (LATA & 0xFF00);
    short invertertedBottomEightLATABits = (~LATA & 0x00FF);
    LATA = topEigthLATABits | invertertedBottomEightLATABits;
}
void toggle_SSD()
{
    if (ssdIsOn)
    {
        // Clear the SSD
        SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
    }
    else 
    {
    if (pressedUnlockedBtnC == TRUE)
    {SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0); 
    pressedUnlockedBtnC = FALSE;}
    
    if (pressedUnlockedBtnL == TRUE)
    {SSD_WriteDigits(29, 15, 15, 10, 0, 0, 0, 0);//Left
    pressedUnlockedBtnL = FALSE;}
    
    if (pressedUnlockedBtnR == TRUE)
    {SSD_WriteDigits(14, 26, 27, 28, 0, 0, 0, 0); //rite (right)
    pressedUnlockedBtnR = FALSE;}

    if (pressedUnlockedBtnD == TRUE)
    {SSD_WriteDigits(22, 21, 88, 13, 0, 0, 0, 0); //down
    pressedUnlockedBtnD = FALSE;}
    
    if (pressedUnlockedBtnU == TRUE)
    {SSD_WriteDigits(24, 23, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0); //UP
    pressedUnlockedBtnU = FALSE;}
    }


    // Toggle the state of the SSD
    ssdIsOn = !ssdIsOn;
}


void displayRandomDirection() {
    static int iteration = 0;  // A static variable to keep track of the number of iterations
    if (iteration < 3) {  // Limit the function to 3 iterations
        int direction = rand() % 4;  // Generate a random number between 0 and 3

        // Display segments based on the random direction
        switch (direction) {
            case 0:  // Up
                SSD_WriteDigits(24, 23, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
                break;
            case 1:  // Down
                SSD_WriteDigits(22, 21, 18, 13, 0, 0, 0, 0);
                break;
            case 2:  // Left
                SSD_WriteDigits(26, 15, 14, 29, 0, 0, 0, 0);
                break;
            case 3:  // Right
                SSD_WriteDigits(14, 26, 27, 28, 0, 0, 0, 0);
                break;
            default:
                // Handle unexpected case (optional)
                break;
        }

        iteration++;  // Increment the iteration counter

        // If we've reached 3 iterations, transition to MODE_3
        if (iteration >= 0) {
            currentMode = MODE_3;
        }
    }
}











void delay_ms(int ms)
{
    int i;
    for (i = 0; i < ms * LOOPS_NEEDED_TO_DELAY_ONE_MS; i++)
    {    }
}

void handle_raw_button_presses() {
    pressedUnlockedBtnC = FALSE;
    pressedUnlockedBtnU = FALSE;
    pressedUnlockedBtnD = FALSE;
    pressedUnlockedBtnL = FALSE;
    pressedUnlockedBtnR = FALSE;

    if (BtnC_RAW && !buttonsLocked) {
        delay_ms(BUTTON_DEBOUNCE_DELAY_MS); // Debounce
        buttonsLocked = TRUE;
        pressedUnlockedBtnC = TRUE;
    }

    if ((BtnC_RAW || BtnU_RAW || BtnD_RAW || BtnL_RAW || BtnR_RAW) && !buttonsLocked) {
        delay_ms(BUTTON_DEBOUNCE_DELAY_MS); // Debounce
        buttonsLocked = TRUE;
        pressedUnlockedBtnC = BtnC_RAW;
        pressedUnlockedBtnU = BtnU_RAW;
        pressedUnlockedBtnD = BtnD_RAW;
        pressedUnlockedBtnL = BtnL_RAW;
        pressedUnlockedBtnR = BtnR_RAW;
    }

    if (!(BtnC_RAW || BtnU_RAW || BtnD_RAW || BtnL_RAW || BtnR_RAW) && buttonsLocked) {
        buttonsLocked = FALSE;
        delay_ms(BUTTON_DEBOUNCE_DELAY_MS); // Debounce
    }
}


