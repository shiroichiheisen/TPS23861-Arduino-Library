/*************************************************************************************************************************************************/
/*!     \file main_auto.c
*
*       \brief File containing the example of Power Over Ethernet using the TPS23861 in semi-automatic mode
*
*       \date January 2013
*
*       This file contains an example software that performs semi-automatic detection and classification using the TPS23861
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
#include "power_manage.h"
#include "system_init.h"






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

TPS238X_Interrupt_Register_t intStatus;
TPS238X_Interrupt_Mask_Register_t intMask;
uint8_t intDelayTime = 0;
uint8_t sysPortNum, sysPortNum1, sysPortNum2, sysPortNum3, sysPortNum4,sysPortNum5, sysPortNum6, sysPortNum7, sysPortNum8;
TPS238x_Ports_t powerEnablePortEvents, powerGoodPortEvents, detectionPortEvents, classificationPortEvents, icutPortEvents, disconnectPortEvents, inrushPortEvents, ilimPortEvents;
TPS238X_Supply_Event_Register_t supplyEvents;
volatile uint8_t rtn;
TPS238x_Detection_Status_t detectStatus;
TPS238x_Classification_Status_t classStatus;
uint8_t powerEnable;
TPS238x_Ports_t  inactivePorts[NUM_OF_TPS23861];
uint8_t  devNum;


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
    init_I2C_Functions();    //initialize I2C
    Setup_UART (PORT_1, BIT2, BIT1, SECONDARY_PERIPHERAL, SMCLK_8_MHZ, UART_9600_BAUD);  //initialize uart
    Setup_Timer ();

    __enable_interrupt();

    __delay_cycles(184000);   //Wait TPS23861 I2C to be operational


    TPS23861_I2C_Address(NUM_OF_TPS23861,i2cAddList,autoMode);   //program TPS23861 I2C address

    //Mapping system port for each port
    sysPortNum1 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_1);
    sysPortNum2 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_2);
    sysPortNum3 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_3);
    sysPortNum4 = tps_RegisterPort (i2cAddList[0], TPS238X_PORT_4);

    sysPortNum5 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_1);
    sysPortNum6 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_2);
    sysPortNum7 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_3);
    sysPortNum8 = tps_RegisterPort (i2cAddList[1], TPS238X_PORT_4);

    // Get the interrupt status
    rtn =  tps_GetDeviceInterruptStatus (i2cAddList[0], &intStatus);
    rtn =  tps_GetDeviceInterruptStatus (i2cAddList[1], &intStatus);

    // Read and Clear all Events
    rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[0], TPS_ON, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
                                            &classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
                                            &inrushPortEvents, &ilimPortEvents, &supplyEvents);

    rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[1], TPS_ON, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
                                            &classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
                                            &inrushPortEvents, &ilimPortEvents, &supplyEvents);

    intMask.CLMSK_Classificiation_Cycle_Unmask = 1;
    intMask.DEMSK_Detection_Cycle_Unmask = 0;
    intMask.DIMSK_Disconnect_Unmask = 1;
    intMask.PGMSK_Power_Good_Unmask = 1;
    intMask.PEMSK_Power_Enable_Unmask = 1;
    intMask.IFMSK_IFAULT_Unmask = 1;
    intMask.INMSK_Inrush_Fault_Unmask = 1;
    intMask.SUMSK_Supply_Event_Fault_Unmask = 1;

    tps_SetDeviceInterruptMask (i2cAddList[0], intMask, intDelayTime);
    tps_SetDeviceInterruptMask (i2cAddList[1], intMask, intDelayTime);

    tps_SetDeviceOpMode (i2cAddList[0], OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO);
    tps_SetDeviceOpMode (i2cAddList[1], OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO, OPERATING_MODE_SEMI_AUTO);

    //Set two event classification if a class 4 classification occurs
    tps_SetDeviceTwoEventEnable(i2cAddList[0],TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4);
    tps_SetDeviceTwoEventEnable(i2cAddList[1],TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4,TWO_EVENT_AFTER_CLASS_4);

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
    uart_puts ("Welcome to the POE 23861 - Semi-Auto Mode for Power Management Apllication\r\n\n\n");



    IntFlag = 1;    // Set IntFlag to clear SUPF Fault at POR

    rtn = tps_SetDeviceDetectClassEnable (i2cAddList[0], inactivePorts[0], inactivePorts[0]);   // Start Semi-Auto Detection and Classification for all ports
    rtn = tps_SetDeviceDetectClassEnable (i2cAddList[1], inactivePorts[1], inactivePorts[1]);   // Start Semi-Auto Detection and Classification for all ports

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

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);
							if (detectStatus == DETECT_RESIST_VALID)          // Valid
							{
                              // add any message to inform the host of the detection results
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

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							if((tps_GetPortPowerEnableStatus(sysPortNum) == TPS_OFF))        // Bypass second class interrupt to turn on the same port twice
							{
								rtn = tps_GetPortDetectClassStatus (sysPortNum, &detectStatus, &classStatus);

								uart_puts ("\r\n\n");
								uart_puts ("\nClassification Event Port ");
								uart_putLong ((unsigned long)(i + (4 * devNum) -1));
								uart_puts ("\r\n\n");
								uart_puts (" - Classification Status: ");
								uart_putLong ((unsigned long)(classStatus));
								uart_puts ("\r\n");

								if ((classStatus != CLASS_OVERCURRENT) &&
									(classStatus != CLASS_UNKNOWN) &&
									(classStatus != CLASS_MISMATCH))
								{
									if(PM_EN)//If power management is enabled, host will make the decision based on power and priority
									{
										PM_powerManagement(sysPortNum);
									}

									else//If If power management is disabled, turn on the port directly with correct settings of current protection
									{
										if(classStatus == CLASS_4)
										{
											tps_SetPortPoEP(sysPortNum, _2X_ILIM_FOLDBACK_CURVE, ICUT_686_MILLIAMP);
										}
										else
										{
											tps_SetPortPoEP(sysPortNum, _1X_ILIM_FOLDBACK_CURVE, ICUT_374_MILLIAMP);
										}


#if (NORMAL_OPERATING == 1)
										tps_SetPortPower(sysPortNum,TPS_ON);
#endif

#if (VMARK_WORKAROUND == 1)
										tps_SetPortOpMode(sysPortNum,OPERATING_MODE_AUTO);
#endif
									}
								}

							}
						}
						target >>= 1;
					}
				}

				// Did we have a new Disconnection Event?
				if (intStatus.DISF_Disconnect_Event)
				{
					uint8_t target = disconnectPortEvents;

					// Set all of the disconnected ports as inactive
					*(uint8_t *)&inactivePorts[devNum] |= target;

				}

				// Did we have a new Power Enable Event?
				if (intStatus.PEC_Power_Enable_Change)
				{
					uint8_t target = powerEnablePortEvents;
					uint8_t i;

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

							powerEnable = tps_GetPortPowerEnableStatus (sysPortNum);

							if (powerEnable == TPS_ON)          // Valid
							{
#if (VMARK_WORKAROUND == 1)
								tps_SetPortOpMode(sysPortNum,OPERATING_MODE_SEMI_AUTO);
#endif
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

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							sysPortNum = tps_GetSystemPortNumber (i2cAddList[devNum], (TPS238x_PortNum_t)i);

						}

						target >>= 1;
					}

				}


				// Fault condition
				if(intStatus.SUPF_Supply_Event_Fault)
				{
					uint8_t *temp = (uint8_t *)&supplyEvents;
					uint8_t target = *temp;

					if(target & 0x10)
					{
						uart_puts("VPWR undervlotage occurred");
						uart_puts("\r\n");
					}
					else if(target & 0x20)
					{
						uart_puts("VDD undervlotage occurred");
						uart_puts("\r\n");
					}

					else if(target & 0x80)
					{
						uart_puts("Thermal shutdown occurred");
						uart_puts("\r\n");
					}

				}

				if(intStatus.INRF_Inrush_Fault)
				{
					uint8_t target = inrushPortEvents;
					uint8_t i;

					*(uint8_t *)&inactivePorts[devNum] |= target;

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (target & 0x1)
						{
							uart_puts ("Start Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum) - 1));
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

					*(uint8_t *)&inactivePorts[devNum] |= targetIcut;
					*(uint8_t *)&inactivePorts[devNum] |= targetIlim;

					for (i = TPS238X_PORT_1; i <= TPS238X_PORT_4; i++)
					{
						if (targetIcut & 0x01)
						{
							uart_puts("ICUT Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum) - 1));
							uart_puts ("\r\n");
						}

						if(targetIlim & 0x01)
						{
							uart_puts("ILIM Fault Port");
							uart_putLong ((unsigned long)(i + (4 * devNum) - 1));
							uart_puts ("\r\n");
						}

						targetIcut >>= 1;
						targetIlim >>= 1;

					}

				}

            }
        }  // if !(IntFlag)



#if (PRINT_STATUS == 1)
        if (PrintPower)
        {
            PrintPower = 0;

        	uint32_t sysPower;
        	sysPower = PM_calSysPower();
        	uart_puts ("\r\n\n");
    		uart_puts ("\n System Power ");
    		uart_putLong ((unsigned long)(sysPower));
    		uart_puts ("mW \r\n");

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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
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
                outNum = ((unsigned long)current * 61039) / 1000000;
                uart_putLong (outNum);
                uart_puts ("mA \r\n\n");
            }


#if (DETAILED_STATUS == 1)
            for (devNum = 0; devNum < NUM_OF_TPS23861; devNum++)
            {
				// read current value of all event registers (Do not clear)
				rtn =  tps_GetDeviceAllInterruptEvents (i2cAddList[devNum], TPS_OFF, &powerEnablePortEvents, &powerGoodPortEvents, &detectionPortEvents,
														&classificationPortEvents, &icutPortEvents, &disconnectPortEvents,
														&inrushPortEvents, &ilimPortEvents, &supplyEvents);

				uart_puts ("\n---- Event Registers -----Dev : ");
				uartPutHex (devNum);
				uart_puts ("----- \r\n0x");
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


				uart_puts ("------------------------------------------\r\n");
            }
#endif
        }

#endif


        //Realtime monitor the system power
        if(PM_powerMonitor && PM_EN)
        {
        	PM_powerMonitor = 0;
        	PM_monitorSysPower();
        }


        if(PM_restartDectectionClassification)
        {
        	PM_restartDectectionClassification = 0;
        	PM_restartOffPortDetectionClassification();
        }


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

    // Monitor total power every 1s
    if (!(counter % PM_POWER_MONITOR_TIMER))
    {
       //PerformDetection = 1;
       PM_powerMonitor = 1;
    }

    if (!(counter % PM_DETECT_CLASS_RESTART_TIMER))
        {

          PM_restartDectectionClassification = 1;
        }


    // Double check interrupt - Is P2.3 Low and P2.3 IFG not present?
    if ((!(P2IFG & BIT3)) && (!(P2IN & BIT3)))
    {
        IntFlag = 1;
    }

    LPM0_EXIT;
}



