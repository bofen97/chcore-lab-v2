/*
   MIT License

   Copyright (c) 2018 Sergey Matyukevich

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
 * ChCore refers to
 * https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md
 * for the min-uart init process.
 */

#include <io/uart.h>

#include "uart.h"

void early_uart_init(void)
{
        unsigned int ra;

        ra = early_get32(GPFSEL1);
        ra &= ~(7 << 12); 
        ra |= 2 << 12;
        ra &= ~(7 << 15);
        ra |= 2 << 15;
        /*
        GPIO Function Select 1  
        ra bin code :  00 010 010 000 000 000 000
        FSEL 14 : GPIO Pin 14 takes alternate function 5
        FSEL 15 : GPIO Pin 15 takes alternate function 5

        GIIO No. | Pull  | ALT0 | ALT1 | ALT2 | ALT3 | ALT4 | ALT5 |
        GPIO 14  | Low   | TXD0 | SD6  | ---- | ---- | ---- | TXD1 |
        GPIO 15  | Low   | RXD0 | SD7  | ---- | ---- | ---- | RXD1 |

        -- TXD1 UART 1 Transmit Data
        -- RXD1 UART 1 Receive Data 
        */         
        early_put32(GPFSEL1, ra); 
        

        //Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither 
        //to remove the current Pull-up/down)
        early_put32(GPPUD, 0); 
        //Wait 150 cycles – this provides the required set-up time for the control signal 
        delay(150);
        //Write  to  GPPUDCLK0/1  to  clock  the  control  signal into  the  GPIO  pads  you  wish  to 
        //modify 
        early_put32(GPPUDCLK0, (1 << 14) | (1 << 15)); //Assert Clock on line , pin 14 and 15
        //Wait 150 cycles 
        delay(150);
        //Write to GPPUD to remove the control signal
        early_put32(GPPUD, 0); 
        // Write to GPPUDCLK0/1 to remove the clock
        early_put32(GPPUDCLK0, 0);


        early_put32(AUX_ENABLES, 1);
        early_put32(AUX_MU_IER_REG, 0);
        early_put32(AUX_MU_CNTL_REG, 0);
        early_put32(AUX_MU_IER_REG, 0);
        early_put32(AUX_MU_LCR_REG, 3); // set bit  11 : the UART works in 8-bit mode
        early_put32(AUX_MU_MCR_REG, 0);
        early_put32(AUX_MU_BAUD_REG, 270);

        early_put32(AUX_MU_CNTL_REG, 3);
}

static unsigned int early_uart_lsr(void)
{
        return early_get32(AUX_MU_LSR_REG);
}

static void early_uart_send(unsigned int c)
{
        while (1) {
                if (early_uart_lsr() & 0x20) /* This bit is set if the transmit FIFO is empty and the 
transmitter is idle. (Finished shifting out the last bit) */
                        break;
        }
        early_put32(AUX_MU_IO_REG, c); /*If the DLAB bit in the line control register is set this register gives access to the LS 8 bits 
of the baud rate.

        */
}

void uart_send_string(char *str)
{
        /* LAB 1 TODO 3 BEGIN */
        for(const char *ch = str ; *ch!='\0' ;ch++){
                early_uart_send(*ch);
        }
        /* LAB 1 TODO 3 END */
}
