/*
 * system_init.h
 *
 *  Created on: Sep 23, 2016
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







#ifndef SYSTEM_INIT_H_
#define SYSTEM_INIT_H_


#include "msp430.h"
#include "TPS23861.h"
#include "common.h"
#include "usci_uart.h"
#include "I2C_Functions.h"


//user configurable prameter
#define NUM_OF_TPS23861                  2           // device number in system
#define VMARK_WORKAROUND                 1           // set 1 if TPS23861 has the mark voltage drop
#define NORMAL_OPERATING                 0           // set 1 if TPS23861 doesn't have the mark voltage drop
#define PRINT_STATUS                     0          // set 1 if wanting to print port status through uart
#define DETAILED_STATUS                  0           // set 1 if wanting to print detailed status through uart
#define PM_POWER_MONITOR_TIMER           20          //20 = 1s
#define PM_DETECT_CLASS_RESTART_TIMER    60          //20 = 1s

#define PM_NUM_OF_PORT                   4           // port count in one PSE

extern uint8_t i2cAddList[NUM_OF_TPS23861];
extern uint8_t IntFlag;
extern TPS238x_On_Off_t autoMode[NUM_OF_TPS23861];
extern uint8_t PM_powerMonitor;
extern uint8_t PM_restartDectectionClassification;



#if (PRINT_STATUS == 1)
extern uint8_t PrintPower;
#endif


void init_MSP430(void);
void TPS23861_I2C_Address(uint8_t deviceNumber,uint8_t i2cAddList[],TPS238x_On_Off_t autoMode[]);



#endif /* SYSTEM_INIT_H_ */
