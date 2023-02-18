/*
 * system_init.c
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




#include "system_init.h"
#include "common.h"
#include "power_manage.h"

//User configurable variables
uint8_t i2cAddList[NUM_OF_TPS23861] = {0x20, 0x28};
TPS238x_On_Off_t autoMode[NUM_OF_TPS23861] = {TPS_OFF, TPS_OFF};
uint8_t PM_setPriority[NUM_OF_TPS23861 * PM_NUM_OF_PORT] = {High, High, Normal, Normal, Normal, Low, Low, Low};
uint8_t PM_restartDectectionClassification = 0;



uint8_t PM_powerMonitor = 1;
uint8_t IntFlag = 0;

#if (PRINT_STATUS == 1)
uint8_t PrintPower = 0;
#endif


/*************************************************************************************************************************************************
*  init_MSP430
**************************************************************************************************************************************************/
/*!
* @brief Initialize MSP430
*
* This function will initialize clock,watchdog, I/O of MSP430
*
* @param[in]   none
*
* @return  none
*
**************************************************************************************************************************************************/


void init_MSP430(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Configure processor and SMCLK for 8MHz Clock.
    BCSCTL1 = CALBC1_8MHZ;              // BCSCTL1 Calibration Data for 8MHz
    DCOCTL = CALDCO_8MHZ;               // DCOCTL Calibration Data for 8MHz
    BCSCTL2 &= ~SELS;                   // Select DCOCLK as SMCLK source

    // Set the nReset pin P2.0 High to keep TPS out of reset.
    P2OUT |=  BIT0;
    P2DIR |=  BIT0;

    // Set the nShutdown pin P2.2 High to prevent shutdown
    P2OUT |=  BIT2;
    P2DIR |=  BIT2;

    // Set the A3 Pin P2.1 Low to use Low I2C addresses for bit 3 (0x8 value)
    P2OUT &= ~BIT1;
    P2DIR |=  BIT1;

    // TODO: P2.3 is the nINT input pin - Configure as interrupt pin. It is INPUT at default.
    P2DIR &= ~BIT3;
    P2IFG &= ~BIT3;
    P2IES |=  BIT3;     // Falling edge
    P2IE  |=  BIT3;
}



/*************************************************************************************************************************************************
*  TPS23861_I2C_Address
**************************************************************************************************************************************************/
/*!
* @brief Program I2C address of every TPS23861
*
* This function will program TPS23861's I2C address one by one
*
* @param[in]   deviceNumber  number of TPS23861 in the system
* @param[in]   i2cAddList[]  addresses for each TPS23861
* @param[in]   autoMode[]    AUTO bi status for each TPS23861
*
* @return  none
*
**************************************************************************************************************************************************/



void TPS23861_I2C_Address(uint8_t deviceNumber,uint8_t i2cAddList[],TPS238x_On_Off_t autoMode[])
{
	uint8_t i,oldAutoBitSetting,addressChangeNeeded,current_address,rtn;

	for (i=0; i < NUM_OF_TPS23861; i++)
	    {
	    	rtn = tps_ReadI2CReg (i2cAddList[i], TPS238X_I2C_SLAVE_ADDRESS_COMMAND, &current_address);
	    	oldAutoBitSetting = (current_address & AUTO_BIT);  //save the old AUTO bit setting

	        uart_puts ("\r\ncurrent_address: ");
	        uart_putLong ((unsigned long)(current_address));

	    	current_address &= 0x7F;

	    	if (autoMode[i] == TPS_ON)
	    	{
	    		current_address |= AUTO_BIT;
	    	}

	        uart_puts ("   -new_address: ");
	        uart_putLong ((unsigned long)(current_address));
	        uart_puts ("   - i2cAddList[i]: ");
	        uart_putLong ((unsigned long)(i2cAddList[i]));
	        uart_puts ("   - rtn: ");
	        uart_putLong ((unsigned long)(rtn));
	        uart_puts ("\r\n");

	        if ((current_address != (oldAutoBitSetting | i2cAddList[i])) || (rtn != I2C_SUCCESSFUL))
	        {
	        	addressChangeNeeded = TRUE;
	        }
	    }

	        // If the address does not match standard, OR a NACK is received,
	        //  reprogram the address(es) of the TPS23861's
	    if(addressChangeNeeded == TRUE)
	    {
	    	tps_SetI2CAddresses (0x14, NUM_OF_TPS23861, i2cAddList, autoMode);
	    }



}




