/*************************************************************************************************************************************************/
/*!     \file I2C_Functions.c   
*
*       \brief Functions that perform standard read and write to slave devices on the I2C bus
*                                                                                                                                                 
*
*                                                                                                                                                 
*       \date April 2010                                                                                                          
*
*       The I2C_Functions perform a standard read and write of multiple bytes of data over the I2C bus. 
*       The functions will reconfigure the bus prior to use and run through the entire I2C states until the 
*       block of data is moved. The functions will poll in between states for the hardware to complete the next
*       step. 
*
*       \note Prototypes for functions in this file are located in the I2C_Functions.h header file located in the 
*        mavrk_embedded\\Modular_EVM_Libraries\\Peripherals directory
*
*       \sa  For Support: http://ti.com/mavrkquestions                                                       
*/                                                                                                                                                 
/**************************************************************************************************************************************************
*       Copyright © 2009-2016 Texas Instruments Incorporated - http://www.ti.com/                                                                 *
***************************************************************************************************************************************************
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*                                                                                                                                                 *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 Included Headers                                                                                                *
**************************************************************************************************************************************************/
#include <msp430.h>
#include "usci_i2c.h"
#include "usci_digital_io.h"


/**************************************************************************************************************************************************
*                                 Definitions                                                                                                     *
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*                                 Prototypes                                                                                                      *
**************************************************************************************************************************************************/


void init_I2C_Functions (void)
{

    Setup_I2C (PORT_1, BIT7, BIT6, SECONDARY_PERIPHERAL, SMCLK_8_MHZ, I2C_100_KHZ);
    
}


/**************************************************************************************************************************************************
*                                 I2C Write Routines                                                                                              *
**************************************************************************************************************************************************/
unsigned char tps_WriteI2CReg (unsigned int i2cAddress, unsigned char registerAddress, unsigned char writeValue)
{
unsigned char rtn;
unsigned char value[2];

    value[0] = registerAddress;
    value[1] = writeValue;

    rtn = Write_I2C (i2cAddress, value, 2);

    return (rtn);
}

unsigned char tps_WriteI2CMultiple (unsigned int i2cAddress, unsigned char registerAddress, unsigned char *writeValues, unsigned char numWriteBytes)
{
unsigned char rtn;
unsigned char i;
unsigned char value[32];

    value[0] = registerAddress;
    for (i=0; i<numWriteBytes; i++)
        value[i] = writeValues[i];

    rtn = Write_I2C (i2cAddress, value, numWriteBytes);

    return (rtn);
}

/**************************************************************************************************************************************************
*                                 I2C Read Routines                                                                                               *
**************************************************************************************************************************************************/
unsigned char tps_ReadI2CReg (unsigned int i2cAddress, unsigned char registerAddress, unsigned char *readValue)
{
    unsigned char rtn;

    rtn = Read_I2C(i2cAddress, registerAddress, readValue, 1);

    return (rtn);
}

unsigned char tps_ReadI2CMultiple (unsigned int i2cAddress, unsigned char registerAddress, unsigned char *readValue, unsigned char numReadBytes)
{
unsigned char rtn;

    rtn = Write_Read_I2C(i2cAddress, &registerAddress, 1, readValue, numReadBytes);

    return (rtn);

}


#pragma vector = USCIAB0TX_VECTOR       //I2C - UCB0RXIFG and UCB0TXIFG interrupts; UART/SPI - TX Interrupt
__interrupt void USCI0TX_ISR(void)
{
    unsigned char wakeCpu = DONT_WAKE_CPU;

    // Is this an I2C Interrupt???
    if (((IE2 & UCB0RXIE) && (IFG2 & UCB0RXIFG)) ||
        ((IE2 & UCB0TXIE) && (IFG2 & UCB0TXIFG)))
    {
        wakeCpu = i2c_ISR();
    }

    if (wakeCpu)
    {
        __bic_SR_register_on_exit(CPUOFF + GIE);      // Exit LPM0
    }

}

#pragma vector = USCIAB0RX_VECTOR       //I2C - Status interrupts; SPI - RX Interrupt
__interrupt void USCI0RX_ISR(void)
{
    unsigned char wakeCpu = DONT_WAKE_CPU;

    // Is this an I2C NACK Interrupt???
    if ((UCB0I2CIE & UCNACKIE) && (UCB0STAT & UCNACKIFG))
    {
        wakeCpu = i2c_stat_isr ();
    }

    if (wakeCpu)
    {
        __bic_SR_register_on_exit(CPUOFF + GIE);      // Exit LPM0
    }

}
