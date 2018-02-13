/*
 * app_Timer.c
 *
 *  Created on: 13/02/2018
 *      Author: uidj2522
 */


/* Interfaces/Drivers */
#include "stdtypedef.h"
#include "app_Timer.h"
#include "fsl_pit.h"
#include "fsl_clock.h"

/* Functions */

/************************************************
 * Name: app_Timer_Init
 * Description: TBD
 * Parameters: None
 * Return: None
 ************************************************/
void app_Timer_Init(void)
{
	pit_config_t ls_PitConfig;

	/* Get Default Configuration */
	PIT_GetDefaultConfig(&ls_PitConfig);

	/* PIT Init */
	PIT_Init(PIT, &ls_PitConfig);

	/* Set Period */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(1000000U, CLOCK_GetFreq(kCLOCK_BusClk)));

	/* Start PIT */
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}

/************************************************
 * Name: app_Timer_IsExpired
 * Description: TBD
 * Parameters: None
 * Return: None
 ************************************************/
T_UBYTE app_Timer_IsExpired(void)
{
	T_UBYTE lub_Value;

	/* Get Status */
	lub_Value = PIT_GetStatusFlags(PIT, kPIT_Chnl_0);

	if(TRUE == lub_Value)
	{
		/* Clear Status Flag */
		PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, PIT_TFLG_TIF_MASK);
	}
	else
	{
		/* Do nothing */
	}

	/* Get PIT Status */
	return lub_Value;
}
