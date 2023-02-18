/**********************************************************************************************************************************************/
/*! 
* \mainpage Power over Ethernet TPS23861 Software Application Programming Interface (API)
*
* \authors Texas Instruments Software Developers
*
* \section intro Introduction
*
* This package contains library software, including code templates for use by TPS23861 software developers.
* 
* The documentation is maintained as doxygen based comments throughout the source code.
* 
* The software is broken up into separate functions. There are public functions that are used by standard software developers
* to command and control the TPS23861. 
*
* Additionally, there are a set of abstraction level functions that are collected in the TPS23861_Glue.c file. The TPS23871 resides
* on the I2C bus, and standard I2C commands are used to communicate with it. As different hardware processors and operating systems
* use different functions to perform I2C communications, the software here uses standard function calls which can be changed
* by the end user into the specific function calls for their specific applciation. This small set of hardware abstraction functions
* are all collected into one file for ease of porting.
*
* For a number of functions there are two different functions that may perform the same configuration. One function exists to set up
* all four ports on the device at the same time. A seaparate function exists to configure one of the four ports. An application that
* has fixed connections may wish to configure everything at once. An application that sees unknown devices inserted and removed may
* wish to configure each port individually as they are discovered. In general, the funciotns that deal with individual ports will have the 
* word Port in the function name. For example, tps_SetDetectClassEnable() enables the detection and classification for all four ports.
* However, the tps_SetPortDetectClassEnable() function only enables one port's detection and classification capaibilities 
*
* \section license Document License         
*
* This work is licensed under the Creative Commons Attribution-Share Alike 3.0 United States License. To view a copy of this license, 
* visit http://creativecommons.org/licenses/by-sa/3.0/us/  or send a letter to Creative Commons, 171 Second Street, Suite 300, 
* San Francisco, California, 94105, USA.
*
* Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
*
***********************************************************************************************************************************************/
