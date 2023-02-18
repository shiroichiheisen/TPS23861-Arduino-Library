/*************************************************************************************************************************************************/
/*!     \file Common.c
*
*       \brief File containing a placeholder for shared error handling / notification funtions
*
*       \date January 2013
*
*       This file contains the simple error handler used as an example in the software
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

#include "msp430.h"
#include "common.h"

unsigned int ErrorHigh[10], ErrorLow[10];
unsigned int ErrorHighIdx = 0, ErrorLowIdx = 0;

void error_Trap (unsigned int errorCode, unsigned int errorLevel)
{
    if (errorLevel == ERROR_HIGH)
    {
    	ErrorHigh[ErrorHighIdx++] = errorCode;
    	while(1);
    }
    else if (errorLevel == ERROR_LOW)
    {
    	ErrorHigh[ErrorLowIdx++] = errorCode;
    	if (ErrorLowIdx >= 10)
    		ErrorLowIdx = 0;
    }

}


