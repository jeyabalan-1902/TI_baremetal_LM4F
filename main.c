

/**
 * main.c
 */

//SW1 - PF4
//SW2 - PF0
//RED LED - PF1
//BLUE LED - PF2
//GREEN LED - PF3
//UART0 RX - PA0
//UART0 TX - PA1

#include "TM4C1230H6PM.h"
#include <string.h>

uint8_t button1_state;
uint8_t button2_state;
char Rx_Buffer[10];  // Buffer to store received string
int index = 0;

void UART0_ReceiveString()
{
    char received_char;

    while ((UART0->FR & (1 << 6)) != 0)  // Check if RX buffer is NOT empty
    {
        received_char = UART0->DR;  // Read the received character

        if (received_char == '\r' || received_char == '\n') // End of message
        {
            Rx_Buffer[index] = '\0'; // Null-terminate the string
            index = 0;  // Reset for next message
            break;
        }
        else if (index < sizeof(Rx_Buffer) - 1)
        {
            Rx_Buffer[index++] = received_char; // Store character in buffer
        }
    }
}

void delay(long t)
{
    while(t--);
}

void printf(const char *str)
{
    while(*str)
    {
        while((UART0->FR & (1 << 5)) != 0);
        UART0->DR = *str;
        str++;
    }
}

int main(void)
{
    SYSCTL->RCGCGPIO = SYSCTL->RCGCGPIO | (1 << 5);             // clock enabling for GPIOF
    GPIOF->LOCK = 0x4C4F434B;
    GPIOF->CR = GPIOF->CR | (1 << 0);                           //PF0 unlock
    GPIOF->DIR = GPIOF->DIR & (~(1 << 4)) & (~(1 <<0));         // setting PF4 & PF0 is input
    GPIOF->PUR = GPIOF->PUR | (1 << 4) | (1 << 0);              // enabling internal pullup for PF0
    GPIOF->DIR = GPIOF->DIR |(1 << 3) | (1 << 1);                          // setting PF3 is output
    GPIOF->DEN = GPIOF->DEN | (1 << 3) | (1 << 4) | (1 << 0) | (1 << 1);   //digitalizing PF4, PF0 and PF3

   //UART - GPIO configuration

    SYSCTL->RCGCUART |= (1 << 0);            //Enabling clock for UART 0
    SYSCTL->RCGCGPIO |= (1 << 0);            //Enabling clock for PORT A
    GPIOA->AFSEL |= (1 << 0) | (1 << 1);     // Set Alternate function for UART pins high
    GPIOA->PCTL |= (1 << 0) | (1 << 4);      //set the mode in PORT control register
    GPIOA->DEN |= (1 << 0) | (1 << 1);       // Digitalizing PA0 and PA1

    //UART communication configuration
    /*
     * IBRD = UARTSysClk / (ClkDiv * Baud Rate)  =>  16000000 / (16 * 9600)  = 104.1666666666667  it doesnot take feactional part so = >  104.
     * FBRD = (BRDF * 64 + 0.5)    =>  (0.1666666666667 * 64 + 0.5) => 11
     *
     */

    UART0->CTL &= (~(1 << 0)) & (~(1 << 8)) & (~(1 << 9));    // disabling the UART EN , TXEN & RXEN for configuration
    UART0->IBRD = 8;
    UART0->FBRD = 44;                                         // set baud rate for the uart communication  is 9600 by using IBRD & FBRD
    UART0->LCRH |= (0x3 << 5);                                // set word length as 8 bits
    UART0->CC = 0x5;                                          // select the clock source in CC register
    UART0->CTL |= (1 << 0) | (1 << 8) | (1 << 9);             //turn on UART EN , TXEN & RXEN bits from uart ctl



	while(1)
	{
	    button1_state = ((GPIOF->DATA & (1 << 4))>>4);
	    button2_state = ((GPIOF->DATA & (1 << 0))>>0);

	    UART0_ReceiveString();  // Get full message from UART

        if (strcmp(Rx_Buffer, "ON") == 0)  // Compare received string with "ON"
        {
            GPIOF->DATA |= (1 << 1);       // Turn ON the LED
            printf("RED LED Turned ON\r\n");
        }
        else if (strcmp(Rx_Buffer, "OFF") == 0)
        {
            GPIOF->DATA &= ~(1 << 1);      // Turn OFF the LED
            printf("RED LED Turned OFF\r\n");
        }


	    if(button1_state == 0)
	    {
	        printf("Button1 Pressed ----->");
            GPIOF->DATA =  GPIOF->DATA | (1 << 3);
            printf(" Green LED Turned ON\r\n");
            delay(1000000);
	    }
        if(button2_state == 0)
        {
            printf("Button2 Pressed ----->");
            GPIOF->DATA =  GPIOF->DATA & (~(1 << 3));
            printf(" Green LED Turned OFF\r\n");
            delay(1000000);
        }
	}
}
