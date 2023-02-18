/*************************************************************************************************************************************************/
/*!     USCI I2C Code
*
*       This code is designed for I2C blocks on the MSP430 using an USCI hardware module.
* 
*       The code was desigmed on MSP430 that do not have the UCB0IV, Interrupt Vector, registers for the I2C module. It also was designed for
*       USCI blocks that have a single I2c Own Address register. Newer MSP430s exist will multiple I2C Own Addresses with individual interrupt
*       vectors for each.
* 
*       The code is specifically written to use UCB0 I2C functions the user can indicate the location of the I2C pins in Setup_I2C()
*       The code only supports Master communications (Reads and Writes). There is no support currently present in this software for Slave or
*       Multi-Master systems.
* 
*       Standard functions that will be used by implementors are:
*          Setup_I2C() - This function is called once to configure the I2C hardware
*          Write_I2C() - This function will perform an I2C write to the I2c address indicated, sending the bytes provided
*          Read_I2C()  - This function will perform a dummy I2C write of 1 byte, followed by a RESTART and
*                                     then read the number of bytes indicated into the buffer provided
*          Read_I2C_With_Stop() - This function is the same as Read_I2C, except that a STOP/START occurs between the dummy write and read,
*                                     rather than a RESTART
* 
*       Note that the Read_I2C() is specifically written to support a 1 byte dummy write. This is intended for a register offset. For I2C devices
*       that use a 2 byte register offset (or any other different dummy write), changes would be needed to the code to support it.
* 
*       The software is written to place the CPU into CPUOFF mode when executing the I2C functionality. All processing occurs in I2C interrupts.
*       At completion of an I2C functionality, the processor will exit CPUOFF mode to continue processing.
* 
*       The i2c_ISR() function is included which must be tied to the USCIAB0TX_VECTOR interrupt vector. Since this interrupt can be shared with
*       other (UART) USCI blocks, the implementation is left to a higher level program (often main.c). The i2c_ISR() function will return an
*       indicator that the processor should exit low power mode upon exit of the interrupt code.
* 
*               if (i2c_ISR())
*                   __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
* 
*       Error recovery processing is not implemented in this software. The Write_I2C(), and Read_I2C*() functions will return an error if the I2C
*       bus is already in use or a NAK is received. (I2C_FAIL_NACK)
* 
*       April 2013
*
*       \note that the functions in this file are not re-entrant. It is the user's responsibility to assure that these functions
*       are not called until the previous function has completed.
*/
/**************************************************************************************************************************************************
*       Copyright © 2013-2016 Texas Instruments Incorporated - http://www.ti.com/                                                                      *
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
 *
**************************************************************************************************************************************************/
#include <msp430.h>
#include "usci_digital_io.h"
#include "usci_i2c.h"


static unsigned char *PRxData;                     // Pointer to RX data
static unsigned int   RXByteCtr;
static unsigned char *PTxData;                     // Pointer to TX data
static unsigned char  TXByteCtr;
static unsigned char  RPT_Flag;                    // Indicates whether to perform a STOP command at the end of the block (Used to get a RESTART)

static unsigned char  I2C_In_Use_Flag;
static unsigned char  NackFlag;

static unsigned char I2C_ClockRates[10][2] =
{
        0x0a, 0x03,                        // 1 MHz
        0x14, 0x05,                        // 2 MHz
        0x28, 0x0a,                        // 4 MHz
        0x50, 0x14,                        // 8 MHz
        0x78, 0x1e,                        // 12 MHz
        0xa0, 0x28,                        // 16 MHz
};

/*************************************************************************************************************************************************
*  Setup_I2C                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Initialize the software structures used in I2C communications. Uses USCI B0 and pins P1.6/P1.7 for I2C
*
* The function is called once to configure the I2C hardware settings for I2C Master Communications.
*
* @param[in]   i2cPort             Indicates the MSP430 port number containing the I2c pins
* @param[in]   sda_pin             Pin number (BIT0 - BIT7) of the SDA pin
* @param[in]   scl_pin             Pin number (BIT0 - BIT7) of the SCL pin
* @param[in]   primary_secondary   Indicates if this is a primary peripheral or secondary (PxSEL2 set) on the MSP430
* @param[in]   smclk_speed         Speed of the MSCLK bus (used to determine bit rate divisors
* @param[in]   i2cClkRate          Desired I2C Clock speed (I2C_100_KHZ or I2C_400_KHZ)
* @return  None
*
* @note This function must be called prior to any I2C_Write or I2C_Read functions
**************************************************************************************************************************************************/
void Setup_I2C (Port_t i2cPort, unsigned char sda_pin, unsigned char scl_pin, primSecPeripheral_t primary_secondary, smclkSpeed_t smclkSpeed, i2cClkRate_t i2cClkRate)
{
#if defined __MSP430_HAS_PORT1_R__
   if (i2cPort == PORT_1)
   {
       P1SEL  |= sda_pin + scl_pin;           // Assign I2C pins to Primary Peripheral of USCI_B0
#if IF_PSxSEL2_PRESENT
       if (primary_secondary == SECONDARY_PERIPHERAL)
       {
           P1SEL2  |= sda_pin + scl_pin;      // Assign I2C pins to Secondary Peripheral of USCI_B0
       }
#endif
   }
#endif
#if defined __MSP430_HAS_PORT2_R__
   if (i2cPort == PORT_2)
   {
       P2SEL  |= sda_pin + scl_pin;           // Assign I2C pins to Primary Peripheral of USCI_B0
#if IF_PSxSEL2_PRESENT
       if (primary_secondary == SECONDARY_PERIPHERAL)
       {
           P2SEL2  |= sda_pin + scl_pin;      // Assign I2C pins to Secondary Peripheral of USCI_B0
       }
#endif
   }
#endif
#if defined __MSP430_HAS_PORT3_R__
   if (i2cPort == PORT_3)
   {
       P3SEL  |= sda_pin + scl_pin;           // Assign I2C pins to Primary Peripheral of USCI_B0
#if IF_PSxSEL2_PRESENT
       if (primary_secondary == SECONDARY_PERIPHERAL)
       {
           P3SEL2  |= sda_pin + scl_pin;      // Assign I2C pins to Secondary Peripheral of USCI_B0
       }
#endif
   }
#endif
#if defined __MSP430_HAS_PORT4_R__
   if (i2cPort == PORT_4)
   {
       P4SEL  |= sda_pin + scl_pin;           // Assign I2C pins to Primary Peripheral of USCI_B0
#if IF_PSxSEL2_PRESENT
       if (primary_secondary == SECONDARY_PERIPHERAL)
       {
           P4SEL2  |= sda_pin + scl_pin;      // Assign I2C pins to Secondary Peripheral of USCI_B0
       }
#endif
   }
#endif

    UCB0CTL1  |= UCSWRST;                     // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = I2C_ClockRates[smclkSpeed][i2cClkRate];
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation

    I2C_In_Use_Flag = 0;
    NackFlag = 0;
}
       
/*************************************************************************************************************************************************
*  Read_I2C                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs a standard I2C Dummy Write followed by I2C Read Sequence to get data from I2C device
*
* The function sends a dummy write to an I2C Slave Device containing a single byte (most often a command or register pointer). An I2C RESTART is
* issued and then an I2C Read command for the indicated number of bytes is executed. The resulting data is placed into the buffer provided by the
* calling function
* 
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]   commandByte       Byte to place in dummy write. Generally a command or register offset
* param[in]  *readBuffer        Location to place the byte returned from the slave device
* param[in]   numBytes          Number of bytes to read
* 
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Read_I2C(unsigned int slaveAddress, unsigned char commandByte, unsigned char *readBuffer, unsigned char numBytes)
{
    unsigned char rtn;

    rtn = Transmit_I2C(slaveAddress, &commandByte, 1, NO_I2C_STOP_FOR_RESTART);

    if (rtn != I2C_COMMAND_STARTED)
        return (rtn);

    // Assure previous command has completed. This is here in case user application
    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    //Receive process
    rtn = Receive_I2C(slaveAddress, readBuffer, numBytes, ISSUE_I2C_STOP);

    return (rtn);

}

/*************************************************************************************************************************************************
*  Read_I2C_With_Stop                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs a standard I2C Dummy Write followed by I2C Read Sequence to get data from I2C device
*
* The function sends a dummy write to an I2C Slave Device containing a single byte (most often a command or register pointer). An I2C STOP followed
* by a START is issued and then an I2C Read command for the indicated number of bytes is executed. The resulting data is placed into the buffer
* provided by the calling function
* 
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]   commandByte       Byte to place in dummy write. Generally a command or register offset
* param[in]  *readBuffer        Location to place the byte returned from the slave device
* param[in]   numBytes          Number of bytes to read
* 
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Read_I2C_With_Stop(unsigned int slaveAddress, unsigned char commandByte, unsigned char *readBuffer, unsigned char numBytes)
{
    unsigned char rtn;

    rtn = Transmit_I2C(slaveAddress, &commandByte, 1, ISSUE_I2C_STOP);

    if (rtn != I2C_COMMAND_STARTED)
        return (rtn);

    // Assure previous command has completed
    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    //Receive process
    rtn = Receive_I2C(slaveAddress, readBuffer, numBytes, ISSUE_I2C_STOP);

    return (rtn);
}

/*************************************************************************************************************************************************
*  Write_Read_I2C
**************************************************************************************************************************************************/
/*!
* @brief Performs a standard I2C Write of multiple bytes followed by I2C Read Sequence to get data from I2C device
*
* The function sends a dummy write to an I2C Slave Device containing a number of bytes.  An I2C RESTART is issued and then an I2C Read command
* for the indicated number of bytes is executed. The resulting data is placed into the buffer provided by the calling function
*
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]  *writeBuffer       Bytes to write.
* param[in]   numWriteBytes     Number of bytes to write
* param[in]  *readBuffer        Location to place the byte returned from the slave device
* param[in]   numReadBytes      Number of bytes to read
*
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Write_Read_I2C(unsigned int slaveAddress, unsigned char *writeBuffer, unsigned char numWriteBytes,
                             unsigned char *readBuffer, unsigned char numReadBytes)
{
    unsigned char rtn;

    rtn = Transmit_I2C(slaveAddress, writeBuffer, numWriteBytes, NO_I2C_STOP_FOR_RESTART);

    if (rtn != I2C_COMMAND_STARTED)
        return (rtn);

    // Assure previous command has completed. This is here in case user application
    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    //Receive process
    rtn = Receive_I2C(slaveAddress, readBuffer, numReadBytes, ISSUE_I2C_STOP);

    return (rtn);

}

/*************************************************************************************************************************************************
*  Write_Read_I2C_With_Stop
**************************************************************************************************************************************************/
/*!
* @brief Performs a standard I2C Write of multiple bytes followed by I2C Read Sequence to get data from I2C device
*
* The function sends a dummy write to an I2C Slave Device containing a number of bytes.  An I2C STOP is issued and then an I2C Read command
* for the indicated number of bytes is executed. The resulting data is placed into the buffer provided by the calling function
*
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]  *writeBuffer       Bytes to write.
* param[in]   numWriteBytes     Number of bytes to write
* param[in]  *readBuffer        Location to place the byte returned from the slave device
* param[in]   numReadBytes      Number of bytes to read
*
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Write_Read_I2C_With_Stop(unsigned int slaveAddress, unsigned char *writeBuffer, unsigned char numWriteBytes,
                             unsigned char *readBuffer, unsigned char numReadBytes)
{
    unsigned char rtn;

    rtn = Transmit_I2C(slaveAddress, writeBuffer, numWriteBytes, ISSUE_I2C_STOP);

    if (rtn != I2C_COMMAND_STARTED)
        return (rtn);

    // Assure previous command has completed. This is here in case user application
    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    //Receive process
    rtn = Receive_I2C(slaveAddress, readBuffer, numReadBytes, ISSUE_I2C_STOP);

    return (rtn);

}

/*************************************************************************************************************************************************
*  Write_I2C                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs a standard I2C Write to the indicated I2C device
*
* The function sends a I2C write to an I2C Slave Device. The calling function will provide a pointer to the data to write, and the length.
* 
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]  *i2cArray          Location of the data to write
* param[in]   numBytes          Number of bytes to write
* 
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Write_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes)
{
    unsigned char rtn;

    rtn = Transmit_I2C(slaveAddress, i2cArray,numBytes, ISSUE_I2C_STOP);

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition completed

    return (rtn);
}

/*************************************************************************************************************************************************
*  Transmit_I2C                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs the low level configuration of the USCI to transmit I2C data
*
* The function performs low level configuration of USCI to begin the I2C write to an I2C Slave Device.
* 
* This logic will hold the processor in CPUOFF mode until the I2C logic completes. If a main program wishes to perform other non-isr processing
* during I2C communications, the  - while (I2C_In_Use_Flag) - Line of code can be removed. It will then be the job of the application to monitor
* all I2C function calls to assure that the previous I2C communication has completed prior to beginning a new one. The start of a new communication
* will fail with I2C_FAIL_IN_USE if the previous communication has not completed.
*
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]  *i2cArray          Location of the data to write
* param[in]   numBytes          Number of bytes to write
* param[in]   rptFlag           Indicates whether to end command with STOP (used to generate a RESTART [ISSUE_I2C_STOP or NO_I2C_STOP_FOR_RESTART]
* 
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Transmit_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes, unsigned char rptFlag)
{

    if (I2C_In_Use_Flag == 1)
    {
        return (I2C_FAIL_IN_USE);
    }

    I2C_In_Use_Flag = 1;
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition from previous command has completed

    UCB0I2CSA = slaveAddress;               // Slave Address of the device on the I2C bus
    RPT_Flag = rptFlag;

    PTxData = i2cArray;
    TXByteCtr = numBytes;                   // Load TX byte counter
    __disable_interrupt();                  // Hold interrupts
    IE2 |= UCB0TXIE;                        // Enable TX interrupt
    UCB0I2CIE |= UCNACKIE;                  // Enable NACK interrupt
    UCB0CTL1  |= UCTR + UCTXSTT;            // I2C TX, start condition
    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    if (NackFlag)
    {
        NackFlag = 0;
        return (I2C_FAIL_NACK);
    }

    return (I2C_COMMAND_STARTED);
}


/*************************************************************************************************************************************************
*  Receive_I2C                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs the low level configuration of the USCI to receive I2C data
*
* The function performs low level configuration of USCI to begin the I2C read from an I2C Slave Device.
* 
* This logic will hold the processor in CPUOFF mode until the I2C logic completes. If a main program wishes to perform other non-isr processing
* during I2C communications, the  - while (I2C_In_Use_Flag) - Line of code can be removed. It will then be the job of the application to monitor
* all I2C function calls to assure that the previous I2C communication has completed prior to beginning a new one. The start of a new communication
* will fail with I2C_FAIL_IN_USE if the previous communication has not completed.
*
* param[in]   slaveAddress      I2C Address of the slave device. (7 bit address without R/~W bit)
* param[in]  *i2cArray          Location to place the received data 
* param[in]   numBytes          Number of bytes to read
* param[in]   rptFlag           Indicates whether to end command with STOP (used to generate a RESTART [ISSUE_I2C_STOP or NO_I2C_STOP_FOR_RESTART]
* 
* @return  I2C_COMMAND_STARTED, I2C_FAIL_NACK or I2C_FAIL_IN_USE
**************************************************************************************************************************************************/
unsigned char Receive_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes, unsigned char rptFlag)
{
    if (I2C_In_Use_Flag == 1)
    {
        return (I2C_FAIL_IN_USE);
    }

    I2C_In_Use_Flag = 1;

    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition (if there was one) from previous command has completed
    IE2 |= UCB0RXIE;                        // Enable RX

    UCB0I2CSA = slaveAddress;               // Slave Address of the device on the I2C bus
    RPT_Flag = rptFlag;

    PRxData = i2cArray;                     // Start of RX buffer
    RXByteCtr = numBytes;                   // Load RX byte counter
    UCB0CTL1 &= ~UCTR;                      // Configure I2C for RX 
    __disable_interrupt();                  // Hold interrupts

    if ((numBytes == 1) && (rptFlag == ISSUE_I2C_STOP))
    {
        UCB0CTL1 |= UCTXSTT;                // I2C Start
        while ((UCB0CTL1 & UCTXSTT) && (!(UCB0STAT & UCNACKIFG)));         // Poll for START to complete or NACK to occur
        if (UCB0STAT & UCNACKIFG)
        {
            NackFlag = 0;
            I2C_In_Use_Flag = 0;
            return (I2C_FAIL_NACK);
        }
        else
            UCB0CTL1 |= UCTXSTP;                // I2C Stop condition - Only one byte received
    }
    else
        UCB0CTL1 |= UCTXSTT;                // I2C RX, start condition

    while (I2C_In_Use_Flag)                 // Make sure we aren't woken up prematurely
        __bis_SR_register(CPUOFF + GIE);    // Enter LPM0 w/ interrupts; First byte received will begin interrupt based processing

    if (NackFlag)
    {
        NackFlag = 0;
        return (I2C_FAIL_NACK);
    }

    return (I2C_COMMAND_STARTED);

}


/*************************************************************************************************************************************************
*  i2c_ISR                                                                                                                         
**************************************************************************************************************************************************/
/*!
* @brief Performs the interrupt level processing for I2C interrupts.
*
*  The i2c_ISR() function is included which must be tied to the USCIAB0TX_VECTOR interrupt vector. Since this interrupt can be shared with
*  other (UART) USCI blocks, the implementation is left to a higher level program (often main.c). The i2c_ISR() function will return an
*  indicator that the processor should exit low power mode upon exit of the interrupt code.
* 
* CODE EXAMPLE FOR CALLING ROUTINE
* #pragma vector = USCIAB0TX_VECTOR       //I2C - UCB0RXIFG and UCB0TXIFG interrupts; UART/SPI - TX Interrupt
* __interrupt void USCI0TX_ISR(void)
* {
*    unsigned char wakeCpu;
*
*    // Is this an I2C Interrupt???
*    if (((IE2 & UCB0RXIE) && (IFG2 & UCB0RXIFG)) ||
*        ((IE2 & UCB0TXIE) && (IFG2 & UCB0TXIFG)))
*    {
*        wakeCpu = i2c_stat_isr ();
*    }
*
*    if (wakeCpu)
*    {
*        __bic_SR_register_on_exit(CPUOFF + GIE);      // Exit LPM0
*    }
* }
*
* @return  unsigned char      Indicates whether calling interrupt function should exit CPUOFF, waking up the processor [WAKE_CPU or DONT_WAKE_CPU]
**************************************************************************************************************************************************/
unsigned char i2c_ISR (void)
{
    if ((IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE))
    {
        IFG2 &= ~UCB0RXIFG;                     // Clear USCI_B0 RX int flag
        *PRxData++ = UCB0RXBUF;                 // Move RX Data Buffer to PRxData
        RXByteCtr--;						  	// Decrement the counter
        if(RXByteCtr == 1)
        {
            if (RPT_Flag == ISSUE_I2C_STOP)
            {
                UCB0CTL1 |= UCTXSTP;            // No Repeated Start: stop condition
            }
        }
        else if (RXByteCtr == 0)
        {
            IE2 &= ~UCB0RXIE;                   //Disable RX interrupt
            I2C_In_Use_Flag = 0;
            return (WAKE_CPU);

        }
    }

    if ((IFG2 & UCB0TXIFG) && (IE2 & UCB0TXIE))
    {

        IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        if(TXByteCtr == 0)
        {
            if(RPT_Flag == ISSUE_I2C_STOP)
            {
                // Wait ISR and restart processing. Issue a STOP command
                UCB0CTL1 |= UCTXSTP;                    // Issue I2C stop condition

            }
            IE2 &= ~UCB0TXIE;                       // Disable TX interrupt
            UCB0I2CIE &= ~UCNACKIE;                 // Disable NACK interrupt
            I2C_In_Use_Flag = 0;
            return (WAKE_CPU);

        }
        else     // TXByteCtr > 0
        {
            UCB0TXBUF = *PTxData++;                 // Load TX buffer
            TXByteCtr--;                            // Decrement TX byte counter
        }
    }
    return (DONT_WAKE_CPU);
}

/*************************************************************************************************************************************************
*  i2c_stat_isr
**************************************************************************************************************************************************/
/*!
* @brief Performs the interrupt level processing for I2C Status interrupts.
*
*  The i2c_ISR() function is included which must be tied to the USCIAB0RX_VECTOR interrupt vector. Since this interrupt can be shared with
*  other (UART) USCI blocks, the implementation is left to a higher level program (often main.c). The i2c_stat_isr() function will return an
*  indicator that the processor should exit low power mode upon exit of the interrupt code.
*
* CODE EXAMPLE FOR CALLING ROUTINE
* #pragma vector = USCIAB0RX_VECTOR       //I2C - Status interrupts; SPI - RX Interrupt
* __interrupt void USCI0RX_ISR(void)
* {
*    unsigned char wakeCpu;
*
*    // Is this an I2C NACK Interrupt???
*    if ((UCB0I2CIE & UCNACKIE) && (UCB0STAT & UCNACKIFG))
*    {
*        wakeCpu = i2c_stat_isr ();
*    }
*
*    if (wakeCpu)
*    {
*        __bic_SR_register_on_exit(CPUOFF + GIE);      // Exit LPM0
*    }
* }
*
* @return  unsigned char      Indicates whether calling interrupt function should exit CPUOFF, waking up the processor [WAKE_CPU or DONT_WAKE_CPU]
**************************************************************************************************************************************************/
unsigned char i2c_stat_isr (void)
{
    if (UCB0STAT & UCNACKIFG)
    {
        UCB0CTL1 |= UCTXSTP;                    // Issue I2C stop condition
        IE2 &= ~UCB0TXIE;                       // Disable TX interrupt
        I2C_In_Use_Flag = 0;
        NackFlag = 1;
        UCB0STAT  &= ~UCNACKIFG;
        UCB0I2CIE &= ~UCNACKIE;
        return (WAKE_CPU);
    }

    return (DONT_WAKE_CPU);
}
