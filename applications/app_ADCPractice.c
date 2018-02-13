/*
 * app_ADCPractice.c
 *
 *  Created on: 12/02/2018
 *      Author: uidj2522
 */

/* Interfaces/Drivers */
#include <applications/app_ADCPractice.h>
#include "stdtypedef.h"
#include "stdio.h"
#include "fsl_port.h"
#include "fsl_adc16.h"

/* Private Variables */
static T_UBYTE rub_ADCConversionInProgress = FALSE;
static T_UWORD ruw_LastConversion = 0u;

/* Functions */

/************************************************
 * Name: app_CounterPractice_Task
 * Description: This function inits ADC Module Configuration.
 * Parameters: None
 * Return: None
 ************************************************/
void app_ADCPractice_Init(void)
{
	CLOCK_EnableClock(kCLOCK_PortE);                           /* Port E Clock Gate Control: Clock enabled */

	PORT_SetPinMux(PORTE, 20u, kPORT_PinDisabledOrAnalog); 		/* PORTE20 (pin 13) is configured as ADC0_DP0 */

	adc16_config_t adc16ConfigStruct;

	ADC16_GetDefaultConfig(&adc16ConfigStruct);

	ADC16_Init(ADC0, &adc16ConfigStruct);
	ADC16_EnableHardwareTrigger(ADC0, false); /* Make sure the software trigger is used. */

	(void)ADC16_DoAutoCalibration(ADC0);
}

/************************************************
 * Name: app_CounterPractice_Task
 * Description: TBD
 * Parameters: None
 * Return: None
 ************************************************/
void app_ADCPractice_Task(void)
{
	/* Check if a conversion is not in progress  */
	if(FALSE == rub_ADCConversionInProgress)
	{
		adc16_channel_config_t adc16ChannelConfigStruct;

		adc16ChannelConfigStruct.channelNumber = 0u;
		adc16ChannelConfigStruct.enableInterruptOnConversionCompleted = false;
		adc16ChannelConfigStruct.enableDifferentialConversion = false;

		ADC16_SetChannelConfig(ADC0, 0u, &adc16ChannelConfigStruct);

		rub_ADCConversionInProgress = TRUE;
	}
	else
	{
		/* In progress */
	}

	/* Wait for ADC Measure Ready */
	if(0U == (kADC16_ChannelConversionDoneFlag & ADC16_GetChannelStatusFlags(ADC0, 0u)))
	{
		/* Do Nothing */
	}
	else
	{
		ruw_LastConversion = ADC16_GetChannelConversionValue(ADC0, 0u) >> 6u;

		rub_ADCConversionInProgress = FALSE;
	}
}

/************************************************
 * Name: app_ADCPractice_GetConversion
 * Description: TBD
 * Parameters: None
 * Return: None
 ************************************************/
T_UBYTE app_ADCPractice_GetConversion(void)
{
	/* Return last converted value */
	return ruw_LastConversion;
}
