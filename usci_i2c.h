#ifndef __USCI_I2C_H
#define __USCI_I2C_H
/*************************************************************************************************************************************************/
/*!     usci_i2c.h
*
*       Header file for USCI Library I2C software
*
*       April 2013
*
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
#include "usci_digital_io.h"


//======================================================================================================
#define ISSUE_I2C_STOP                 0
#define NO_I2C_STOP_FOR_RESTART        1

#define I2C_ACTION_ONGOING             0
#define I2C_ACTION_COMPLETE            1

#define I2C_RX_INT                     1
#define I2C_TX_INT                     2

#define I2C_COMMAND_STARTED            0
#define I2C_FAIL_IN_USE                1
#define I2C_FAIL_NACK                  2
       
typedef enum {
    I2C_100_KHZ        = 0,
    I2C_400_KHZ        = 1
} i2cClkRate_t;

/*************************************************************************************************************/
/*                              PROTOTYPES                                                                   */
/*************************************************************************************************************/

#ifdef __CPLUSPLUS
extern "C" {
#endif

void Setup_I2C (Port_t i2cPort, unsigned char sda_pin, unsigned char scl_pin, primSecPeripheral_t primary_secondary, smclkSpeed_t smclkSpeed, i2cClkRate_t i2cClkRate);
unsigned char Transmit_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes, unsigned char rptFlag);
unsigned char Receive_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes, unsigned char rptFlag);
unsigned char Read_I2C(unsigned int slaveAddress, unsigned char commandByte, unsigned char *readBuffer, unsigned char numBytes);
unsigned char Read_I2C_With_Stop(unsigned int slaveAddress, unsigned char commandByte, unsigned char *readBuffer, unsigned char numBytes);
unsigned char Write_Read_I2C(unsigned int slaveAddress, unsigned char *writeBuffer, unsigned char numWriteBytes,
                             unsigned char *readBuffer, unsigned char numReadBytes);
unsigned char Write_Read_I2C_With_Stop(unsigned int slaveAddress, unsigned char *writeBuffer, unsigned char numWriteBytes,
                                       unsigned char *readBuffer, unsigned char numReadBytes);

unsigned char Write_I2C(unsigned int slaveAddress, unsigned char *i2cArray, unsigned char numBytes);
unsigned char i2c_ISR (void);
unsigned char i2c_stat_isr (void);

#ifdef __CPLUSPLUS
}
#endif

#endif // #ifndef __USCI_I2C_H

