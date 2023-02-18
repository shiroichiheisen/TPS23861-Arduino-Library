/*************************************************************************************************************************************************/
/*!     \file Common.h
*
*       \brief Header file containing standard definitions, error prototypes, and error codes used by the TPS23861 software API functions
*
*       \date January 2013
*
*       This file contains the standard definitions, error prototypes, and error codes used by the TPS23861 software API functions
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
*                                                                                                                                                 *
**************************************************************************************************************************************************/

#ifndef __COMMON_H_
#define __COMMON_H_


#define CLEAR  0
#define SET    1

#define TRUE   1
#define FALSE  0

#define NULL   0

#define ERROR_HIGH  1
#define ERROR_LOW   2


#define I2C_OPERATION_SUCCESSFUL              0x0
#define PMU_OPERATION_SUCCESSFUL              0x0

#define INVALID_PARAMETER_VALUE               0x10

#define I2C_OPERATION_FAILED                  0x21
#define PMU_OPERATION_FAILED                  0x22

#define WRITE_FAILED_ACCESS_MODE_INVALID      0x30

#define CLOCK_PERIOD   8    // 8MHz is 8 clocks per uSec
#define CLOCKS(a)     (a * CLOCK_PERIOD)


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;

typedef signed char int8_t;
typedef signed short int16_t;

extern void error_Trap (unsigned int errorCode, unsigned int errorLevel);



#endif /* __COMMON_H_ */
