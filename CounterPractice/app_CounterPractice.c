/*
 * app_CounterPractice.c
 *
 *  Created on: 07/02/2018
 *      Author: uidj2522
 */

/* Interfaces/Drivers */
#include "stdtypedef.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "app_CounterPractice.h"

/* Local Typedefs */
typedef enum
{
	DIGIT0,
	DIGIT1,
	DIGIT2,
	N_DIGITS
}T_DIGIT;

/* Local Macros */

#define APP_COUNTERPRACTICE_INCREASE_PIN_NUMBER		12u
#define APP_COUNTERPRACTICE_DECREASE_PIN_NUMBER		13u
#define APP_COUNTERPRACTICE_RESET_PIN_NUMBER		16u

#define APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK		0u
#define APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK		1u
#define APP_COUNTERPRACTICE_RESET_SHIFT_MASK		2u

#define APP_COUNTERPRACTICE_INCREASE_MASK			(1u << APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK)
#define APP_COUNTERPRACTICE_DECREASE_MASK			(1u << APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK)
#define APP_COUNTERPRACTICE_RESET_MASK				(1u << APP_COUNTERPRACTICE_RESET_SHIFT_MASK)

#define APP_COUNTERPRACTICE_COUNTER_MIN_VALUE		0u
#define APP_COUNTERPRACTICE_COUNTER_MAX_VALUE		100u
#define APP_COUNTERPRACTICE_COUNTER_RESET_VALUE		APP_COUNTERPRACTICE_COUNTER_MIN_VALUE

//#define COMMON_CATODE
#define APP_COUNTERPRACTICE_7SEG_A_PIN_NUMBER		3u
#define APP_COUNTERPRACTICE_7SEG_B_PIN_NUMBER		7u
#define APP_COUNTERPRACTICE_7SEG_C_PIN_NUMBER		6u
#define APP_COUNTERPRACTICE_7SEG_D_PIN_NUMBER		4u
#define APP_COUNTERPRACTICE_7SEG_E_PIN_NUMBER		10u
#define APP_COUNTERPRACTICE_7SEG_F_PIN_NUMBER		0u
#define APP_COUNTERPRACTICE_7SEG_G_PIN_NUMBER		11u
#define APP_COUNTERPRACTICE_7SEG_DOT_PIN_NUMBER		5u

#define APP_COUNTERPRACTICE_7SEG_DIGIT0_PIN_NUMBER	30u
#define APP_COUNTERPRACTICE_7SEG_DIGIT1_PIN_NUMBER	29u
#define APP_COUNTERPRACTICE_7SEG_DIGIT2_PIN_NUMBER	23u

#define APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE	1000u
#define APP_COUNTERPRACTICE_DEBOUNCE_LONG_PRESS_VALUE	3000u

/* Extern Variables */

/* Private Variables */
static T_UBYTE rub_Counter		= 0u;
static T_UBYTE rub_InputValue 	= 0u;
static T_UBYTE rub_BCDUnits		= 0u;
static T_UBYTE rub_BCDTens		= 0u;
static T_UBYTE rub_BCDHundreds	= 0u;

static T_UBYTE rub_IncreasePressedFlag 	= FALSE;
static T_UBYTE rub_DecreasePressedFlag 	= FALSE;
static T_UBYTE rub_ResetPressedFlag 	= FALSE;

static T_UBYTE rub_LongIncreasePressedFlag 	= FALSE;
static T_UBYTE rub_LongDecreasePressedFlag 	= FALSE;

static T_UWORD raub_DebounceCounter[N_DIGITS] = {0,0,0};

static T_DIGIT re_DigitToShow = DIGIT0;
static const T_UBYTE caub_7SegSymbols[0x10] =
{
		/*abcdefg.*/
		0b11111100u,	//0
		0b01100000u,	//1
		0b11011010u,	//2
		0b11110010u,	//3
		0b01100110u,	//4
		0b10110110u,	//5
		0b10111110u,	//6
		0b11100000u,	//7
		0b11111110u,	//8
		0b11110110u,	//9
		0b11101110u,	//A
		0b00111110u,	//b
		0b10011100u,	//C
		0b01111010u,	//d
		0b10011110u,	//E
		0b10001110u		//F
};

static const T_UBYTE caub_DigitPinNum[N_DIGITS] = {
		APP_COUNTERPRACTICE_7SEG_DIGIT0_PIN_NUMBER,
		APP_COUNTERPRACTICE_7SEG_DIGIT1_PIN_NUMBER,
		APP_COUNTERPRACTICE_7SEG_DIGIT2_PIN_NUMBER
};

/* Prototypes */
static T_UBYTE app_CounterPractice_CheckInputs(void);
static T_UBYTE app_CounterPractice_ResetButtonIsPressed(void);
static T_UBYTE app_CounterPractice_IncreaseButtonIsPressed(void);
static T_UBYTE app_CounterPractice_DecreaseButtonIsPressed(void);
static void app_CounterPractice_ClearCounter(void);
static void app_CounterPractice_IncreaseCounter(void);
static void app_CounterPractice_DecreaseCounter(void);
static void app_CounterPractice_Byte2BCD(T_UBYTE* lpub_Data, T_UBYTE* lpub_Uni, T_UBYTE* lpub_Ten, T_UBYTE* lpub_Hun);
static T_UBYTE app_CounterPractice_BCD27Seg(T_UBYTE lub_BCDData);
static void app_CounterPractice_Set7SegOutput(T_UBYTE lub_7SegData);
static void app_CounterPractice_DigitManager(void);

/* Functions */

/************************************************
 * Name: app_CounterPractice_Task
 * Description: This function contains all task for this practicein sequence.
 * 				This function must be called in a periodic task (like an infinite loop or scheduled task)
 * Parameters: None
 * Return: None
 ************************************************/
void app_CounterPractice_Task(void)
{
	/* Check and store the input values */
	rub_InputValue = app_CounterPractice_CheckInputs();

	/* Check if there is a button pressed */
	/* Check for Reset Button */
	if(app_CounterPractice_ResetButtonIsPressed() == TRUE)
	{
		app_CounterPractice_ClearCounter();
	}
	/* Check for Increase Button */
	else if(app_CounterPractice_IncreaseButtonIsPressed() == TRUE)
	{
		app_CounterPractice_IncreaseCounter();
	}
	/* Check for Decrease Button */
	else if (app_CounterPractice_DecreaseButtonIsPressed() == TRUE)
	{
		app_CounterPractice_DecreaseCounter();
	}
	/* Any button pressed */
	else
	{
		/* Do Nothing */
	}

	/* Convert Byte to BCD */
	app_CounterPractice_Byte2BCD(&rub_Counter, &rub_BCDUnits, &rub_BCDTens, &rub_BCDHundreds);

	/* Multiplexing task */
	app_CounterPractice_DigitManager();
}

/************************************************
 * Name: app_CounterPractice_Init
 * Description: This function initializes the modules needed for the practice
 * Parameters: None
 * Return: None
 ************************************************/
void app_CounterPractice_Init(void)
{
	/* Enable the clock for the modules needed */
	CLOCK_EnableClock(kCLOCK_PortC);
	CLOCK_EnableClock(kCLOCK_PortE);

	/* PORT Module Configuration */

	port_pin_config_t ls_PinConfig;

	ls_PinConfig.mux = kPORT_MuxAsGpio;
	ls_PinConfig.pullSelect = kPORT_PullUp;

	port_pin_config_t *lps_PinConfig;

	lps_PinConfig = &ls_PinConfig;

	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_A_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_B_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_C_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_D_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_E_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_F_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_G_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_7SEG_DOT_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_INCREASE_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_DECREASE_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTC, APP_COUNTERPRACTICE_RESET_PIN_NUMBER, lps_PinConfig);

	/* DIGIT Pins */
	PORT_SetPinConfig(PORTE, APP_COUNTERPRACTICE_7SEG_DIGIT0_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTE, APP_COUNTERPRACTICE_7SEG_DIGIT1_PIN_NUMBER, lps_PinConfig);
	PORT_SetPinConfig(PORTE, APP_COUNTERPRACTICE_7SEG_DIGIT2_PIN_NUMBER, lps_PinConfig);

	/* GPIO Configuration for Inputs */

	gpio_pin_config_t ls_GPIOPinConfig;

	/* GPIO Configuration for Outputs */
	ls_GPIOPinConfig.pinDirection = kGPIO_DigitalOutput;
#ifdef COMMON_CATODE
	ls_GPIOPinConfig.outputLogic = TRUE;
#else
	ls_GPIOPinConfig.outputLogic = FALSE;
#endif


	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_A_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_B_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_C_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_D_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_E_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_F_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_G_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, APP_COUNTERPRACTICE_7SEG_DOT_PIN_NUMBER, &ls_GPIOPinConfig);

	ls_GPIOPinConfig.outputLogic = TRUE;

	GPIO_PinInit(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT0_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT1_PIN_NUMBER, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT2_PIN_NUMBER, &ls_GPIOPinConfig);

	/* GPIO Configuration Inputs */
	ls_GPIOPinConfig.pinDirection = kGPIO_DigitalInput;

	GPIO_PinInit(GPIOC, 12U, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, 13U, &ls_GPIOPinConfig);
	GPIO_PinInit(GPIOC, 16U, &ls_GPIOPinConfig);
}

/************************************************
 * Name: app_CounterPractice_CheckInputs
 * Description: This function returns the input value in available GPIO pins
 * Parameters: None
 * Return: Inputs Masked
 ************************************************/
static T_UBYTE app_CounterPractice_CheckInputs(void)
{
	T_UBYTE lub_Value;

	/* Clear Local Variable */
	lub_Value = 0;

	/* Read and store the input values */
	lub_Value |= (GPIO_ReadPinInput(GPIOC, APP_COUNTERPRACTICE_INCREASE_PIN_NUMBER)? FALSE:TRUE) << APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK;
	lub_Value |= (GPIO_ReadPinInput(GPIOC, APP_COUNTERPRACTICE_DECREASE_PIN_NUMBER)? FALSE:TRUE) << APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK;
	lub_Value |= (GPIO_ReadPinInput(GPIOC, APP_COUNTERPRACTICE_RESET_PIN_NUMBER)? FALSE:TRUE) << APP_COUNTERPRACTICE_RESET_SHIFT_MASK;

	return lub_Value;
}

/************************************************
 * Name: app_CounterPractice_ResetButtonIsPressed
 * Description: This function returns TRUE if the
 * 				assigned pin Reset Button was pressed
 * Parameters: None
 * Return: TRUE/FALSE (Pressed/Not Pressed)
 ************************************************/
static T_UBYTE app_CounterPractice_ResetButtonIsPressed(void)
{
	T_UBYTE lub_Value;

	/* Clear Local Variable */
	lub_Value = 0;

	/* Check if Increase button is pressed by first time */
	if(((rub_InputValue & APP_COUNTERPRACTICE_RESET_MASK) == APP_COUNTERPRACTICE_RESET_MASK) &&
			(rub_ResetPressedFlag == FALSE))
	{
		//Debounce Strategy
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Set pressed flag */
			rub_ResetPressedFlag = TRUE;
			/* Clear Debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK] = 0u;
		}
		else
		{
			raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK]++;
		}
	}
	/* Check if a valid press has been performed */
	else if(((rub_InputValue & APP_COUNTERPRACTICE_RESET_MASK) != APP_COUNTERPRACTICE_RESET_MASK) &&
			(rub_ResetPressedFlag == TRUE))
	{
		/* Debounce Strategy */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Clear pressed flag */
			rub_ResetPressedFlag = FALSE;

			/* Clear Debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK] = 0u;

			/* Valid Press */
			lub_Value = TRUE;
		}
		else
		{
			/* Increase Debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK]++;
		}

	}
	else
	{
		/* Clear Debounce Counter */
		raub_DebounceCounter[APP_COUNTERPRACTICE_RESET_SHIFT_MASK] = 0u;
	}

	return lub_Value;
}

/************************************************
 * Name: app_CounterPractice_IncreaseButtonIsPressed
 * Description: This function returns TRUE if the
 * 				assigned pin Reset Button was pressed
 * Parameters: None
 * Return: TRUE/FALSE (Pressed/Not Pressed)
 ************************************************/
static T_UBYTE app_CounterPractice_IncreaseButtonIsPressed(void)
{
	T_UBYTE lub_Value;

	/* Clear Local Variable */
	lub_Value = FALSE;

	/* Check if Increase button is pressed by first time */
	if(((rub_InputValue & APP_COUNTERPRACTICE_INCREASE_MASK) == APP_COUNTERPRACTICE_INCREASE_MASK) &&
			(rub_IncreasePressedFlag == FALSE))
	{
		/* Debounce Strategy */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Set pressed flag */
			rub_IncreasePressedFlag = TRUE;

			/* Clear Debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] = 0u;
		}
		else
		{
			/* Increase Debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK]++;
		}
	}
	/* Check if a valid press has been performed */
	else if(((rub_InputValue & APP_COUNTERPRACTICE_INCREASE_MASK) != APP_COUNTERPRACTICE_INCREASE_MASK) &&
			(rub_IncreasePressedFlag == TRUE))
	{
		/* Reset counter and Clear Long Press flag if was set already */
		if(rub_LongIncreasePressedFlag == TRUE)
		{
			/* Clear Long press validation flag */
			rub_LongIncreasePressedFlag = FALSE;

			/* Reset Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_MASK] = 0u;
		}
		else
		{
			/* Do Nothing */
		}

		/* Debounce Strategy */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Clear pressed flag */
			rub_IncreasePressedFlag = FALSE;

			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] = 0u;

			/* Valid Press */
			lub_Value = TRUE;
		}
		else
		{
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK]++;
		}

	}
	/* Check for long press */
	else if((rub_InputValue & APP_COUNTERPRACTICE_INCREASE_MASK) == APP_COUNTERPRACTICE_INCREASE_MASK &&
			rub_IncreasePressedFlag == TRUE)
	{//Button pressed and debounce counter validated

		/* Set Long Press "In Validation" Flag */
		rub_LongIncreasePressedFlag = TRUE;

		/* Check if a long press has been performed */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] >= APP_COUNTERPRACTICE_DEBOUNCE_LONG_PRESS_VALUE)
		{
			/* Clear Long Press "In Validation" Flag */
			rub_IncreasePressedFlag = FALSE;

			/* Clear Long Press "In Validation" Flag */
			rub_LongIncreasePressedFlag = FALSE;

			/* Clear debounce counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] = 0u;

			/* Valid Press */
			lub_Value = TRUE;
		}
		/* Long press not reached yet */
		else
		{
			raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK]++;
		}
	}
	else
	{
		/* Clear Long Press "In Validation" Flag */
		rub_LongIncreasePressedFlag = FALSE;

		/* Clear debounce Counter */
		raub_DebounceCounter[APP_COUNTERPRACTICE_INCREASE_SHIFT_MASK] = 0u;
	}

	return lub_Value;
}

/************************************************
 * Name: app_CounterPractice_DecreaseButtonIsPressed
 * Description: This function returns TRUE if the
 * 				assigned pin Decrease Button was pressed
 * Parameters: None
 * Return: TRUE/FALSE (Pressed/Not Pressed)
 ************************************************/
static T_UBYTE app_CounterPractice_DecreaseButtonIsPressed(void)
{
	T_UBYTE lub_Value;

	/* Clear Local Variable */
	lub_Value = 0;

	/* Check if Increase button is pressed by first time */
	if(((rub_InputValue & APP_COUNTERPRACTICE_DECREASE_MASK) == APP_COUNTERPRACTICE_DECREASE_MASK) &&
			(rub_DecreasePressedFlag == FALSE))
	{
		/* Debounce Strategy */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Set pressed flag */
			rub_DecreasePressedFlag = TRUE;

			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] = 0u;
		}
		else
		{
			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK]++;
		}
	}
	/* Check if a valid press has been performed */
	else if(((rub_InputValue & APP_COUNTERPRACTICE_DECREASE_MASK) != APP_COUNTERPRACTICE_DECREASE_MASK) &&
			(rub_DecreasePressedFlag == TRUE))
	{
		/* Reset counter and Clear Long Press flag if was set already */
		if(rub_LongDecreasePressedFlag == TRUE)
		{
			/* Clear Long press validation flag */
			rub_LongDecreasePressedFlag = FALSE;

			/* Reset Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_MASK] = 0u;
		}
		else
		{
			/* Do Nothing */
		}

		/*Debounce Strategy*/
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] > APP_COUNTERPRACTICE_DEBOUNCE_SHORT_PRESS_VALUE )
		{
			/* Clear pressed flag */
			rub_DecreasePressedFlag = FALSE;

			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] = 0u;

			/* Valid Press */
			lub_Value = TRUE;
		}
		else
		{
			/* Clear Long Press "In Validation" Flag */
			rub_LongDecreasePressedFlag = FALSE;

			/* Clear debounce Counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK]++;
		}
	}
	/* Check for long press */
	else if((rub_InputValue & APP_COUNTERPRACTICE_DECREASE_MASK) == APP_COUNTERPRACTICE_DECREASE_MASK &&
			rub_DecreasePressedFlag == TRUE)
	{//Button pressed and debounce counter validated

		/* Set Long Press "In Validation" Flag */
		rub_LongDecreasePressedFlag = TRUE;

		/* Check if a long press has been performed */
		if(raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] >= APP_COUNTERPRACTICE_DEBOUNCE_LONG_PRESS_VALUE)
		{
			/* Clear Long Press "In Validation" Flag */
			rub_DecreasePressedFlag = FALSE;

			/* Clear Long Press "In Validation" Flag */
			rub_LongDecreasePressedFlag = FALSE;

			/* Clear debounce counter */
			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] = 0u;

			/* Valid Press */
			lub_Value = TRUE;
		}
		/* Long press not reached yet */
		else
		{
			raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK]++;
		}
	}
	else
	{
		raub_DebounceCounter[APP_COUNTERPRACTICE_DECREASE_SHIFT_MASK] = 0u;
	}

	return lub_Value;
}

/************************************************
 * Name: app_CounterPractice_ClearCounter
 * Description: This function clears the counter
 * Parameters: None
 * Return: None
 ************************************************/
static void app_CounterPractice_ClearCounter(void)
{
	rub_Counter = APP_COUNTERPRACTICE_COUNTER_RESET_VALUE;
}

/************************************************
 * Name: app_CounterPractice_IncreaseCounter
 * Description: This function increases the counter
 * Parameters: None
 * Return: None
 ************************************************/
static void app_CounterPractice_IncreaseCounter(void)
{
	/* Check if counter has reached the MAX value */
	if(rub_Counter >= APP_COUNTERPRACTICE_COUNTER_MAX_VALUE)
	{/* Counter has reached his MAX value */

		/* Set Counter at his MIN value*/
		rub_Counter = APP_COUNTERPRACTICE_COUNTER_MIN_VALUE;
	}
	else
	{/* Counter has not reached his MAX value */

		/* Increase counter */
		rub_Counter++;

	}
}

/************************************************
 * Name: app_CounterPractice_DecreaseCounter
 * Description: This function decreases the counter
 * Parameters: None
 * Return: None
 ************************************************/
static void app_CounterPractice_DecreaseCounter(void)
{
	/* Check if counter has reached the MIN value */
	if(rub_Counter > APP_COUNTERPRACTICE_COUNTER_MIN_VALUE)
	{/* Counter has not reached his MIN value */

		/* Decrease counter */
		rub_Counter--;
	}
	else
	{/* Counter has reached his MIN value */

		/* Set counter at his MAX value */
		rub_Counter = APP_COUNTERPRACTICE_COUNTER_MAX_VALUE;

	}
}

/************************************************
 * Name: app_CounterPractice_Byte2BCD
 * Description: This function converts one byte to BCD format
 * Parameters: 	lpub_Data	-> Pointer to Data to be converted
 * 				lpub_Uni 	-> Pointer to variable to store units
 * 				lpub_Ten	-> Pointer to variable to store tens
 * 				lpub_Hun	-> Pointer to variable to store hundreds
 * Return: None
 ************************************************/
static void app_CounterPractice_Byte2BCD(T_UBYTE* lpub_Data, T_UBYTE* lpub_Uni, T_UBYTE* lpub_Ten, T_UBYTE* lpub_Hun)
{
	T_UBYTE lub_Data;

	/* Get data to be converted */
	lub_Data = *lpub_Data;

	/* Get Hundreds */
	*lpub_Hun = lub_Data / 100u;

	/* Get Tens */
	lub_Data -= (*lpub_Hun * 100u);
	*lpub_Ten = lub_Data / 10u;

	/* Get Units */
	lub_Data -= (*lpub_Ten * 10u);
	*lpub_Uni = lub_Data;
}

/************************************************
 * Name: app_CounterPractice_BCD27Seg
 * Description: This function converts one BCD byte to 7 seg format
 * Parameters: 	lub_BCDData	-> Pointer to Data to be converted
 * Return: 7 Seg const value
 ************************************************/
static T_UBYTE app_CounterPractice_BCD27Seg(T_UBYTE lub_BCDData)
{
#ifdef COMMON_CATODE
	/* Return 7seg symbol using Data as index */
	return caub_7SegSymbols[lub_BCDData];
#else //COMMON_ANODE
	return (T_UBYTE)(~caub_7SegSymbols[lub_BCDData]);
#endif
}

/************************************************
 * Name: app_CounterPractice_Set7SegOutput
 * Description: This function set the GPIO pins according to the input variable
 * Parameters: 	lub_7SegData	-> Pointer to 7seg Data
 * Return: None
 ************************************************/
static void app_CounterPractice_Set7SegOutput(T_UBYTE lub_7SegData)
{
	T_FIELD_8 ls_PinValues;

	/* Store the value to show in a structure */
	ls_PinValues.byte = lub_7SegData;

	/* Set A segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_A_PIN_NUMBER, ls_PinValues.bits.bit7);
	/* Set B segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_B_PIN_NUMBER, ls_PinValues.bits.bit6);
	/* Set C segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_C_PIN_NUMBER, ls_PinValues.bits.bit5);
	/* Set D segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_D_PIN_NUMBER, ls_PinValues.bits.bit4);
	/* Set E segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_E_PIN_NUMBER, ls_PinValues.bits.bit3);
	/* Set F segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_F_PIN_NUMBER, ls_PinValues.bits.bit2);
	/* Set G segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_G_PIN_NUMBER, ls_PinValues.bits.bit1);
	/* Set DOT segment */
	GPIO_WritePinOutput(GPIOC, APP_COUNTERPRACTICE_7SEG_DOT_PIN_NUMBER, ls_PinValues.bits.bit0);
}

/************************************************
 * Name: app_CounterPractice_DigitManager
 * Description: This function manages each digit per cycle
 * Parameters: 	none
 * Return: None
 ************************************************/
static void app_CounterPractice_DigitManager(void)
{
	/* Pwr Off all digits */
	GPIO_WritePinOutput(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT0_PIN_NUMBER, FALSE);
	GPIO_WritePinOutput(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT1_PIN_NUMBER, FALSE);
	GPIO_WritePinOutput(GPIOE, APP_COUNTERPRACTICE_7SEG_DIGIT2_PIN_NUMBER, FALSE);

	switch(re_DigitToShow)
	{
	case DIGIT0:
	default:
	{
		/* Show the units */
		/* Convert BCD to 7 Seg. Then, set the 7 seg outputs*/
		app_CounterPractice_Set7SegOutput( app_CounterPractice_BCD27Seg( rub_BCDUnits ) );

		/* Prepare Next Digit state */
		re_DigitToShow = DIGIT1;
	}break;
	case DIGIT1:
	{
		/* Show the tens */
		/* Convert BCD to 7 Seg. Then, set the 7 seg outputs*/
		app_CounterPractice_Set7SegOutput( app_CounterPractice_BCD27Seg( rub_BCDTens ) );
		/* Prepare Next Digit state */
		re_DigitToShow = DIGIT2;
	}break;
	case DIGIT2:
	{
		/* Show the Hundreds */
		/* Convert BCD to 7 Seg. Then, set the 7 seg outputs*/
		app_CounterPractice_Set7SegOutput( app_CounterPractice_BCD27Seg( rub_BCDHundreds ) );
		/* Prepare Next Digit state */
		re_DigitToShow = DIGIT0;
	}break;
	}

	/* Digit PwrOn */
	GPIO_WritePinOutput(GPIOE, caub_DigitPinNum[(T_UBYTE)re_DigitToShow], TRUE);
}
