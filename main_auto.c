/*************************************************************************************************************************************************/
/*!     \file main_auto.c
*
*       \brief File containing the example of Power Over Ethernet using the TPS23861 in automatic mode
*
*       \date January 2013
*
*       This file contains an example software that performs automatic detection and classification using the TPS23861
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
#include "I2C_Functions.h"
#include "TPS23861.h"
#include "usci_uart.h"
#include "system_init.h"

#define DETAILED_STATUS   1
#define NUM_OF_TPS23861   2


//uint8_t IntFlag = 0;
//uint8_t PrintPower = 0;
//uint8_t PerformDetection = 1;   // Initially perform a detection

// MSP430 Launchpad has P2.0 connected to nReset
void tpsReset (void)
{
    // Toggle nReset Pin
    P2OUT &= ~BIT0;
    __delay_cycles(1000);
    P2OUT |=  BIT0;
}

void Setup_Timer()
{
    /***************************************************
    * Timer Configuration
    ***************************************************/
    TA0CCR0 = 50000;                                    // 50 ms
    TA0CTL = ((TASSEL_2) | (ID_3) | (MC_1) | (TAIE));   // /8
}


void main(void)
{
//unsigned char i2cAddList[NUM_OF_TPS23861] = {0x20, 0x28};
//TPS238x_On_Off_t autoMode[NUM_OF_TPS23861] = {TPS_ON, TPS_ON};
TPS238X_Interrupt_Mask_Register_t intMask;
uint8_t intDelayTime = 0;
uint8_t sysPortNum, sysPortNum1, sysPortNum2, sysPortNum3, sysPortNum4;
uint8_t             sysPortNum5, sysPortNum6, sysPortNum7, sysPortNum8;
TPS238x_Ports_t powerEnablePortEvents, powerGoodPortEvents, detectionPortEvents, classificationPortEvents, icutPortEvents, disconnectPortEvents, inrushPortEvents, ilimPortEvents;
TPS238X_Interrupt_Register_t intStatus;
TPS238X_Supply_Event_Register_t supplyEvents;
volatile uint8_t rtn;
TPS238x_Detection_Status_t detectStatus;
TPS238x_Classification_Status_t classStatus;
//uint16_t current, voltage;
//unsigned long outNum;
uint8_t powerGood;
uint8_t powerEnable;
//uint8_t  temperature;
//unsigned char current_address;
//uint8_t  i;

TPS238x_Ports_t  inactivePorts[NUM_OF_TPS23861];
uint8_t  devNum;
//uint8_t  oldAutoBitSetting;
//uint8_t  addressChangeNeeded = FALSE;


#if (PRINT_STATUS == 1)
uint16_t current, voltage;
uint8_t  temperature;
unsigned long outNum;
#endif

#if (DETAILED_STATUS == 1)
TPS238x_Ports_t  powerEnablePorts;
TPS238x_Ports_t  powerGoodPorts;
#endif

    init_MSP430();           //initialize MSP430
    init_I2C_Functions();
    Setup_UART (PORT_1, BIT2, BIT1, SECONDARY_PERIPHERAL, SMCLK_8_MHZ, UART_9600_BAUD);
    Setup_Timer ();

    __enable_interrupt();
    __delay_cycles(184000);   //Wait TPS23861 I2C to be operational


    TPS23861_I2C_Address(NUM_OF_TPS23861,i2cAddList,autoMode);   //program TPS23861 I2C address

    sysPortNum1 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_1);
    sysPortNum2 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_2);
    sysPortNum3 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_3);
    sysPortNum4 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_4);

    sysPortNum5 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_1);
    sysPortNum6 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_2);
    sysPortNum7 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_3);
    sysPortNum8 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_4);

    rtn =  tps_GetDeviceInterruptStatus (i2cAddList[0], &intStatus);
    rtn =  tps_GetDeviceInterruptStatus (i2cAddList[1], &intStatus);

    // Read and clear any existing events
    rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[0], TPS_ON, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
                                            &classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
                                            &inrushPortEvents, &ilimPortEvents, &supplyEvents);

    rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[1], TPS_ON, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
                                            &classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
                                            &inrushPortEvents, &ilimPortEvents, &supplyEvents);

    intMask.CLMSK_Classificiation_Cycle_Unmask = 1;
    intMask.DEMSK_Detection_Cycle_Unmask = 0;
    intMask.DIMSK_Disconnect_Unmask = 0;
    intMask.PGMSK_Power_Good_Unmask = 0;
    intMask.PEMSK_Power_Enable_Unmask = 0;
    tps_SetDeviceInterruptMask (i2cAddList[0], intMask, intDelayTime);
    tps_SetDeviceInterruptMask (i2cAddList[1], intMask, intDelayTime);

    tps_SetDeviceOpMode (i2cAddList[0], OPERATING_MODE_AUTO, OPERATING_MODE_AUTO, OPERATING_MODE_AUTO, OPERATING_MODE_AUTO);
    tps_SetDeviceOpMode (i2cAddList[1], OPERATING_MODE_AUTO, OPERATING_MODE_AUTO, OPERATING_MODE_AUTO, OPERATING_MODE_AUTO);

    inactivePorts[0] = TPS238X_ALL_PORTS;
    inactivePorts[1] = TPS238X_ALL_PORTS;

    // Power off all ports in case we are re-running this application without physically shutting down ports from previous run
    tps_SetDevicePowerOff (i2cAddList[0], inactivePorts[0]);
    tps_SetDevicePowerOff (i2cAddList[1], inactivePorts[1]);

    // Set up all ports for Disconnect Enable at 7.5 mA
    tps_SetDeviceDisconnectEnable (i2cAddList[0], inactivePorts[0], DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP);
    tps_SetDeviceDisconnectEnable (i2cAddList[1], inactivePorts[1], DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP, DCTH_7_5_MILLIAMP);

    // Wait for input from user to start printing to the UART Terminal
    uart_puts ("\r\n");
    while ((UCA0RXBUF != 'S') && (UCA0RXBUF != 's'))
    {
        uart_puts ("\r POE 23861 - Hit 'S' to start");
    	__delay_cycles (60000);
    }

    uart_puts ("\r                                           \r");
    uart_puts ("Welcome to the POE 23861 - Auto Mode\r\n\n\n");

    IntFlag = 1;    // Set IntFlag to clear SUPF Fault at POR

    rtn = tps_SetDeviceDetectClassEnable (i2cAddList[0], inactivePorts[0], inactivePorts[0]);   // Start Auto Detection and Classification for all ports
    rtn = tps_SetDeviceDetectClassEnable (i2cAddList[1], inactivePorts[1], inactivePorts[1]);   // Start Auto Detection and Classification for all ports

    while (1)
    {

        if (!IntFlag)
        {
            __enable_interrupt();
            LPM0;
            __no_operation();
        }
        else
        {
        	IntFlag = 0;

            for (devNum = 0; devNum < NUM_OF_TPS23861; devNum++)
            {

				// Get the interrupt and event statuses. Clear the event statuses while reading
				rtn =  tps_GetDeviceInterruptStatus (i2cAddList[devNum], &intStatus);
				rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[devNum], TPS_ON, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
														&classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
														&inrushPortEvents, &ilimPortEvents, &supplyEvents);

				// Did we have a new Detection Event?
				if (intStatus.DETC_Detection_Cycle)
				{
					uint8_t target = detectionPortEvents;
					uint8_t i;

					for (i=TPS238X_PORT_1; i<=TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
							if (detectStatus == DETECT_RESIST_VALID)          // Valid
							{
								uart_puts ("\nDetection Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum)));
								uart_puts (" - Detect Status: ");
								uart_putLong ((unsigned long)(detectStatus));
								uart_puts ("\r\n");
							}
						}
						target >>= 1;

					}

				}

				// Did we have a new Classification Event?
				if (intStatus.CLASC_Classification_Cycle)
				{
					uint8_t target = classificationPortEvents;
					uint8_t i;

					for (i=TPS238X_PORT_1; i<=TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);

							uart_puts ("\nClassification Event Port ");
							uart_putLong ((unsigned long)(i + (4 * devNum)));
							uart_puts (" - Classification Status: ");
							uart_putLong ((unsigned long)(classStatus));
							uart_puts ("\r\n");

							if ((classStatus != CLASS_OVERCURRENT) &&
								(classStatus != CLASS_UNKNOWN) &&
								(classStatus != CLASS_MISMATCH))
							{
								uart_puts ("Classification Valid\r\n");
							}
						}
						target >>= 1;
					}
				}

				// Did we have a new Disconnection Event?
				if (intStatus.DISF_Disconnect_Event)
				{
					uint8_t target = disconnectPortEvents;
					uint8_t i;



					// Set all of the disconnected ports as inactive
					inactivePorts[devNum] |= target;

					for (i=TPS238X_PORT_1; i<=TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							// Re-enable the detection/classification for this port
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);
							tps_SetPortDetectClassEnable (sysPortNum, TPS_ON, TPS_ON);

							uart_puts ("\nDisconnection Event Port ");
							uart_putLong ((unsigned long)(i + (4 * devNum)));
							uart_puts ("\r\n");
						}
						target >>= 1;
					}

				}

				// Did we have a new Power Enable Event?
				if (intStatus.PEC_Power_Enable_Change)
				{
					uint8_t target = powerEnablePortEvents;
					uint8_t i;


					for (i=TPS238X_PORT_1; i<=TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							powerEnable = tps_GetPortPowerEnableStatus (sysPortNum);

							if (powerEnable == TPS_ON)          // Valid
							{
								// Log the port as Valid - Powered - Not expecting a detection
								inactivePorts[devNum] &= ~(CONVERT_PORT_NUM(i));
								uart_puts ("Power Enable Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum)));
								uart_puts ("\r\n");

							}
							else
							{
								uart_puts ("Power Disable Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum)));
								uart_puts ("\r\n");
							}
						}
						target >>= 1;

					}

				}

				// Did we have a new Power Good Event?
				if (intStatus.PGC_Power_Good_Change)
				{
					uint8_t target = powerGoodPortEvents;
					uint8_t i;

					for (i=TPS238X_PORT_1; i<=TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							powerGood = tps_GetPortPowerGoodStatus (sysPortNum);

							if (powerGood == TPS_ON)          // Valid
							{
								uart_puts ("Power Good Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum)));
								uart_puts ("\r\n");

							}
							else
							{
								uart_puts ("Power No Longer Good Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum)));
								uart_puts ("\r\n");

							}
						 }
						target >>= 1;
				    }

				}

				//Fault conditions
				if(intStatus.SUPF_Supply_Event_Fault)
				{
					uint8_t *temp = (uint8_t *)&supplyEvents;
					uint8_t target = *temp;

					if(target & 0x10)
					{
						uart_puts("VPWR undervlotage occurred");
						uart_puts("\r\n");
					}
					else if (target & 0x20)
					{
						uart_puts("VDD undervlotage occurred");
						uart_puts("\r\n");
					}

					else if (target & 0x80)
					{
						uart_puts("Thermal shutdown occurred");
						uart_puts("\r\n");
					}

				}

				if(intStatus.INRF_Inrush_Fault)
				{
					uint8_t target = inrushPortEvents;
					uint8_t i;

					// Set all of the start fault ports as inactive
				    *(uint8_t *)&inactivePorts[devNum] |= target;

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							uart_puts ("Start Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum)));
							uart_puts ("\r\n");
						}
						target >>= 1;
					}
				}

				if(intStatus.IFAULT_ICUT_ILIM_Fault)
				{
					uint8_t targetIcut = icutPortEvents;
					uint8_t targetIlim = ilimPortEvents;
					uint8_t i;

					// Set all of the ICUT fault fault ports as inactive
					*(uint8_t *)&inactivePorts[devNum] |= icutPortEvents;

					// Set all of the ICUT fault fault ports as inactive
					*(uint8_t *)&inactivePorts[devNum] |= ilimPortEvents;

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (targetIcut & 0x01)
						{
							uart_puts("ICUT Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum)));
							uart_puts ("\r\n");
						}

						if(targetIlim & 0x01)
						{
							uart_puts("ILIM Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum)));
							uart_puts ("\r\n");
						}

						targetIcut >>= 1;
						targetIlim >>= 1;

					}

				}

            }
        }  // end if (!IntFlag)

#if (PRINT_STATUS == 1)
        if (PrintPower)
        {

            PrintPower = 0;

            tps_GetDeviceInputVoltage (i2cAddList[0], &voltage);
            uart_puts ("Input Voltage: ");
            outNum = ((unsigned long)voltage * 3662) / 1000;
            uart_putLong (outNum);
            uart_puts ("mV \r\n");

            tps_GetDeviceTemperature (i2cAddList[0], &temperature);
            uart_puts ("Device Temperature: ");
            outNum = CONVERT_TEMP((unsigned long)temperature);
            uart_putLong (outNum);
            uart_puts (" degrees C\r\n\n");

            if (!(inactivePorts[0] & PORT_1_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum1, &voltage, &current);
                uart_puts ("Port 1 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[0] & PORT_2_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum2, &voltage, &current);
                uart_puts ("Port 2 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[0] & PORT_3_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum3, &voltage, &current);
                uart_puts ("Port 3 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[0] & PORT_4_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum4, &voltage, &current);
                uart_puts ("Port 4 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n\n");
            }

            if (!(inactivePorts[1] & PORT_1_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum5, &voltage, &current);
                uart_puts ("Port 5 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[1] & PORT_2_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum6, &voltage, &current);
                uart_puts ("Port 6 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[1] & PORT_3_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum7, &voltage, &current);
                uart_puts ("Port 7 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n");
            }

            if (!(inactivePorts[1] & PORT_4_VALUE))
            {
                tps_GetPortMeasurements (sysPortNum8, &voltage, &current);
                uart_puts ("Port 8 Voltage: ");
                outNum = ((unsigned long)voltage * 3662) / 1000;
                uart_putLong (outNum);
                uart_puts ("mV      Current: ");
                outNum = ((unsigned long)current * 62260) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n\n");
            }

#if (DETAILED_STATUS == 1)
            for (devNum=0; devNum < NUM_OF_TPS23861; devNum++)
            {

				// read current value of all event registers (Do not clear)
				rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[devNum], TPS_OFF, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
														&classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
														&inrushPortEvents, &ilimPortEvents, &supplyEvents);

				uart_puts ("\n---- Event Registers -----\r\n0x");
				uartPutHex ((powerGoodPortEvents<<4) | powerEnablePortEvents);
				uart_puts ("   0x");
				uartPutHex ((classificationPortEvents << 4) | detectionPortEvents);
				uart_puts ("   0x");
				uartPutHex ((disconnectPortEvents << 4) | icutPortEvents);
				uart_puts ("   0x");
				uartPutHex ((ilimPortEvents << 4) | inrushPortEvents);
				uart_puts ("   0x");
				uartPutHex ((*(unsigned char*)&supplyEvents << 4));
				uart_puts ("\r\n\n");

				uart_puts ("---- Port Status -----\r\n0x");
				sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], TPS238X_PORT_1);
				rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
				uartPutHex ((classStatus<<4) | detectStatus);
				uart_puts ("   0x");
				sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], TPS238X_PORT_2);
				rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
				uartPutHex ((classStatus<<4) | detectStatus);
				uart_puts ("   0x");
				sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], TPS238X_PORT_3);
				rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
				uartPutHex ((classStatus<<4) | detectStatus);
				uart_puts ("   0x");
				sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], TPS238X_PORT_4);
				rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
				uartPutHex ((classStatus<<4) | detectStatus);
				uart_puts ("\r\n\n");
				uart_puts ("---- Power Status -----\r\n0x");
				rtn = tps_GetDevicePowerStatus (i2cAddList[devNum], &powerEnablePorts, &powerGoodPorts);
				uartPutHex ((powerGoodPorts<<4) | powerEnablePorts);
				uart_puts ("\r\n\n");
#endif

				uart_puts ("------------------------------------------\r\n");
            }
        }
#endif

    }

}

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR (void)
{

    if (P2IFG & BIT3)
    {
        IntFlag = 1;
    }

    // Clear interrupt flags
    P2IFG = 0;

    LPM0_EXIT;
}


/**************************************************************************************************************************************************
* TIMER0_A1
**************************************************************************************************************************************************/
/*!
*  Blink the Heartbeat LED
*
* @brief Timer Interrupt Routine
*
* @return None
**************************************************************************************************************************************************/
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{
static unsigned long counter=0;

    TA0CTL &= ~TAIFG;

    if (++counter == 200)       // Every 10 seconds
    {
        counter = 0;
#if (PRINT_STATUS == 1)
        PrintPower = 1;
#endif
    }


    // Double check interrupt - Is P2.3 Low and P2.3 IFG not present?
    if ((!(P2IFG & BIT3)) && (!(P2IN & BIT3)))
    {
        IntFlag = 1;
    }

    LPM0_EXIT;
}



