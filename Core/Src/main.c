/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "liquidcrystal_i2c.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DHT22_PORT GPIOA
#define DHT22_PIN GPIO_PIN_8 			// si crea una maschera per GPIO_PIN_8 cosicchè ora si possa scrivere DHT22_PIN

#define DHT22_PIN_EXT GPIO_PIN_9 		// si crea una maschera per GPIO_PIN_8(PA9) per il sensore di temperatura esterna


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */


				/********* variabili per misurare temperatura ed umidità tramite sensore ***********/

// per sensore INTERNO

uint8_t RH1, RH2, TC1, TC2, SUM, CHECK;
uint32_t pMillis, cMillis;
float tCelsius = 0;
float tFahrenheit = 0;
float RH = 0;

// per sensore ESTERNO

uint8_t RH1_EXT, RH2_EXT, TC1_EXT, TC2_EXT, SUM_EXT, CHECK_EXT;
float tCelsius_EXT = 0;
float tFahrenheit_EXT = 0;
float RH_EXT = 0;



				/**************** variabili per la lettura dei valori lungo l'asse x ed y del joystick ****************/

uint16_t readValueX; 		//lettura valori asse x joystick
uint16_t readValueY; 		//lettura valori asse y joystick

int mode=0;					//variabile che identifica la modalità operativa scelta

float T_Cold = 22;			// variabile temperatura MINIMA
float T_Hot= 30;			// variabile temperatura MASSIMA
float tExt;					//variabile che indica la temperatura esterna

char strc[20] = {0};		//stringa per stampare la temperatura minima su display
char strh[20] = {0};		//stringa per stampare la temperatura massima su display
char strm1[20] = {0};		//stringa per stampare la temperatura della modalità 1 su display
char strm2[20] = {0};		//stringa per stampare la temperatura della modalità 2 su display
char strm3[20] = {0};		//stringa per stampare la temperatura della modalità 3 su display
char strm4[20] = {0};		//stringa per stampare la temperatura della modalità 4 su display

int fire = 0; // variabile per l'uscita di emergenza

								/************** VARIABILI MENÙ ****************/

int comando;
int scelta;
int scenario;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */





							/******************* FUNZIONE DELAY *******************/

// delay in microsecondi per il sensore DHT22

void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim1, 0);
  while (__HAL_TIM_GET_COUNTER(&htim1) < delay);
}
							/******************* END FUNZIONE DELAY *******************/






								/******************* SENSORE DHT22 *******************/

/*
  Le seguenti due funzioni sono utili per inizializzare e per aggiornare il sensore DHT22.
  Tutte le informazioni sono rimandate al link https://www.micropeta.com/video48
 */


uint8_t DHT22_Start (uint32_t Pin,	GPIO_TypeDef *Port)
{
  uint8_t Response = 0;
  GPIO_InitTypeDef GPIO_InitStructPrivate = {0};
  GPIO_InitStructPrivate.Pin = Pin;
  GPIO_InitStructPrivate.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructPrivate.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructPrivate.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Port, &GPIO_InitStructPrivate); 			// set the pin as output
  HAL_GPIO_WritePin (Port, Pin, 0);   				// pull the pin low
  microDelay (1300);   // wait for 1300us
  HAL_GPIO_WritePin (Port, Pin, 1);   				// pull the pin high
  microDelay (30);   // wait for 30us
  GPIO_InitStructPrivate.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructPrivate.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Port, &GPIO_InitStructPrivate); 			// set the pin as input
  microDelay (40);
  if (!(HAL_GPIO_ReadPin (Port, Pin)))
  {
    microDelay (80);
    if ((HAL_GPIO_ReadPin (Port, Pin))) Response = 1;
  }
  pMillis = HAL_GetTick();
  cMillis = HAL_GetTick();
  while ((HAL_GPIO_ReadPin (Port, Pin)) && pMillis + 2 > cMillis)
  {
    cMillis = HAL_GetTick();
  }
  return Response;
}

uint8_t DHT22_Read (uint32_t Pin,	GPIO_TypeDef *Port)
{
  uint8_t a,b;
  for (a=0;a<8;a++)
  {
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while (!(HAL_GPIO_ReadPin (Port, Pin)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go high
      cMillis = HAL_GetTick();
    }
    microDelay (40);   // wait for 40 us
    if (!(HAL_GPIO_ReadPin (Port, Pin)))   // if the pin is low
      b&= ~(1<<(7-a));
    else
      b|= (1<<(7-a));
    pMillis = HAL_GetTick();
    cMillis = HAL_GetTick();
    while ((HAL_GPIO_ReadPin (Port, Pin)) && pMillis + 2 > cMillis)
    {  // wait for the pin to go low
      cMillis = HAL_GetTick();
    }
  }
  return b;
}


									/******************* END SENSORE DHT22 *******************/





									/******************* DISPLAY LCD *******************/

// Questa funzione permette di mostrare sul display LCD i valori di temperatura e umidità

void Display_Temp (float Temp, float Rh)
{
	char str[20] = {0};
	char str1[20] = {0};

	char str_esp[20] = {0};

		HD44780_Init(2);
	    HD44780_Clear();




	    HD44780_SetCursor(0,0);
	    HAL_Delay(20);
	    sprintf (str, "TempINT: %.1f gradi", Temp); 			//stampa temperatura
	    HD44780_PrintStr(str);

	    HD44780_SetCursor(0,1);
	    HAL_Delay(20);
	    sprintf (str1, "Umidita'INT: %.1f ", Rh); 				//stampa umidità
	    HD44780_PrintStr(str1);
	    HD44780_PrintStr("%");

	    sprintf (str_esp,"%.1f", Temp);
	    HAL_UART_Transmit(&huart3, str_esp, sizeof(str_esp), 100);		// trasmissione alla ESP


}

void Display_Temp_EXT (float Temp_EXT, float Rh_EXT)
{
	char str[20] = {0};
	char str1[20] = {0};


	HD44780_Init(2);
	HD44780_Clear();




	HD44780_SetCursor(0,0);
	HAL_Delay(20);
	sprintf (str, "TempEXT:%.1f gradi", Temp_EXT); 				//stampa temperatura
	HD44780_PrintStr(str);

	HD44780_SetCursor(0,1);
	HAL_Delay(20);
	sprintf (str1, "Umidita'EXT: %.1f ", Rh_EXT); 				//stampa umidità
	HD44780_PrintStr(str1);
	HD44780_PrintStr("%");


}



									/******************* END DISPLAY LCD *******************/




									/*****************	EMERGENZA	*******************/
/*
 * La Funzione FireExit() serve a definire quali eventi possono far scattare una procedura di emergenza, che permetta di poter gestire
 * il possibile problema senza interrompere il corretto funzionamento della Camera. I due eventi sono il doppio click del joystick e
 * la rilevazione dell'apertura della porta durante il ciclo di lavoro.
 */

void FireExit(){


	if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET) // primo click
	{
		HAL_Delay(2000);
		if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET) // secondo click
		{
			fire = 1;

			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10); 	// PELTIER SPENTA
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1000); 	// PELTIER HOT FAN  ACCESA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 1000); 	// PELTIER COLD FAN ACCESA

			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 1000); 	// PTC FAN ACCESA
			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); 	// PTC SPENTA

			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,1);
			HAL_Delay(20);
			HD44780_PrintStr("INTERRUZIONE! ");
			HAL_Delay(500);

			scelta = 0;								//imposto scelta = 0 per non uscire dal menù
			HAL_Delay(300);
			mode = 0;								//imposto mode = 0 per uscire dalla modalità  in esecuzione
			HAL_Delay(300);


		}
	}

	if(HAL_GPIO_ReadPin(STATO_PORTA_GPIO_Port, STATO_PORTA_Pin)==GPIO_PIN_SET) //Porta Aperta
	{
		fire = 1;

		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10); 	// PELTIER SPENTA
		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1000); 	// PELTIER HOT FAN  ACCESA
		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 1000); 	// PELTIER COLD FAN ACCESA

		__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 1000); 	// PTC FAN	ACCESA
		__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); 	// PTC	SPENTA

		HAL_Delay(200);
		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,1);
		HAL_Delay(20);
		HD44780_PrintStr("INTERRUZIONE! ");
		HAL_Delay(500);

		scelta = 0;							//imposto scelta = 0 per non uscire dal menù
		HAL_Delay(300);
		mode = 0;							//imposto mode = 0 per uscire dalla modalità in esecuzione
		HAL_Delay(300);

	}
}



										/**************** END EMERGENZA *****************/



										/******************* MODALITÀ *******************/

void Modalita (int mode,float tCelsius, float RH){

	tExt = tCelsius_EXT;

	while(HAL_GPIO_ReadPin(STATO_PORTA_GPIO_Port, STATO_PORTA_Pin)==GPIO_PIN_SET) //Porta Aperta
	{
		HAL_Delay(200);
		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		HD44780_PrintStr("Chiudere la porta! ");
		HAL_Delay(500);

	}

	while(mode==1)			 //modalità raggiungimento temperatura minima
	{
		while(tCelsius>T_Cold) //si compara la temperatura interna della camera con quella da raggiungere. Se la prima è maggiore --> bisogna raffreddare --> Peltier accese
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 1000); // PELTIER ACCESA, valore del compare alto (1000)
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1000); // PELTIER HOT FAN  ACCESA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 1000); // PELTIER COLD FAN ACCESA

			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 10); // PTC FAN	SPENTA
			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC	SPENTA


			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("Peltier Accesa ");
			HAL_Delay(500);


			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);				// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);			// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);


					/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/


			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				fire = 0; 			// ripristino il flag
				break; 				// torno nel menu
			}


					/******************* END USCITA CON DOPPIO CLICK***************/

			while(tCelsius>tExt)	// finché la temperatura interna è maggiore di quella esterna mantengo gli sportellini aperti per velocizzare il raffreddamento
			{

				/****************** COMANDI APERTURA SERVO *********************************/


				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 			//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);			// SERVO 2 BASSO


				/****************** END COMANDI PER I SERVO *********************************/




				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; 			// torno nel menu
				}



						/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("Peltier Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);		// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);

			}

			HAL_Delay(500);
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 		//SERVO 1 ALTO
			HAL_Delay(500);
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);		// SERVO 2 BASSO
		}

		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//Peltier spenta ma ventole accese

		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10);	//PELTIER SPENTA: valore compare basso = 10 (NON ZERO PERCHÈ VIENE ELABORATO COME ALTRO ESTREMO E QUINDI RIMANGONO ACCESE)


		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strc, "%.1f gradi raggiunti", T_Cold); 			//stampa temperatura
		HD44780_PrintStr(strc);

		/********************************* LETTURA TEMPERATURA ***************************/

		if(DHT22_Start(DHT22_PIN, DHT22_PORT))
		{
			RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
			RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
			TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
			TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
			SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
			CHECK = RH1 + RH2 + TC1 + TC2;
			if (CHECK == SUM)
			{
				if (TC1>127) // If TC1=10000000, negative temperature
				{
					tCelsius = (float)TC2/10*(-1);
				}
				else
				{
					tCelsius = (float)((TC1<<8)|TC2)/10;
				}
				tFahrenheit = tCelsius * 9/5 + 32;
				RH = (float) ((RH1<<8)|RH2)/10;
			}
		}


		/********************************* END LETTURA TEMPERATURA ***************************/


		HAL_Delay(200);
		Display_Temp(tCelsius, RH);		// stampo temperatura e umidità interne su LCD
		HAL_Delay(1000);
		Display_Temp_EXT(tExt, RH_EXT);	// stampo temperatura e umidità esterne su LCD
		HAL_Delay(1000);


		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,1);
		HAL_Delay(20);
		HD44780_PrintStr("Peltier Spenta ");
		HAL_Delay(500);

		mode = 0; 						//si esce dal while: terminata la modalità 1
	}

	while(mode==2) 					//modalità raggiungimento temperatura massima
	{
		while(tCelsius<T_Hot) //si compara la temperatura interna della camera con quella da raggiungere
		{
			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 1000); // PTC ACCESA: bisogna riscaldare l'interno della camera
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 1000); // PTC FAN ACCESA

			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10); // PELTIER SPENTA
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 10); // PELTIER HOT FAN  SPENTA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 10); // PELTIER COLD FAN SPENTA


			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("PTC Accesa ");
			HAL_Delay(500);

			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);


			/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				fire = 0; 			//ripristino il flag
				break;				 //torno nel menu
			}

			/******************* END USCITA CON DOPPIO CLICK ***************/


			while(tCelsius<tExt) //se la temperatura interna è minore di quella esterna apro gli sportellini per velocizzare il riscaldamento
			{

				/****************** COMANDI APERTURA SERVO *********************************/

				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 		//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);		// SERVO 2 BASSO

				/****************** END COMANDI SERVO *********************************/



				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; 		//torno nel menu
				}

				/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("PTC Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);		// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);	// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);


			}

				//bisogna chiudere gli sportelli: si è raggiunta la temperatura ambiente
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 		//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);		// SERVO 2 BASSO
		}


		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//PTC spenta ma ventola accesa

		__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC SPENTA

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strh, "%.1f gradi raggiunti", T_Hot); 			//stampa temperatura
		HD44780_PrintStr(strh);
		HAL_Delay(500);



		/********************************* LETTURA TEMPERATURA ***************************/
		if(DHT22_Start(DHT22_PIN, DHT22_PORT))
		{
			RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
			RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
			TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
			TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
			SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
			CHECK = RH1 + RH2 + TC1 + TC2;
			if (CHECK == SUM)
			{
				if (TC1>127) // If TC1=10000000, negative temperature
				{
					tCelsius = (float)TC2/10*(-1);
				}
				else
				{
					tCelsius = (float)((TC1<<8)|TC2)/10;
				}
				tFahrenheit = tCelsius * 9/5 + 32;
				RH = (float) ((RH1<<8)|RH2)/10;
			}
		}


		/********************************* END LETTURA TEMPERATURA ***************************/


		HAL_Delay(200);
		Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
		HAL_Delay(1000);
		Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
		HAL_Delay(1000);

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,1);
		HAL_Delay(20);
		HD44780_PrintStr("PTC Spenta ");
		HAL_Delay(500);

		mode = 0; 				//si esce dal while: modalità 2 terminata
	}



	while (mode == 3) 		// SALITA: da minima a massima
	{
		while(tCelsius>T_Cold) //si compara la temperatura interna della camera con quella da raggiungere. Se la prima è maggiore --> bisogna raffreddare --> si accendono le Peltier
		{
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 1000); // PELTIER ACCESA, valore del compare alto (1000)
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1000); // PELTIER HOT FAN  ACCESA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 1000); // PELTIER COLD FAN ACCESA

			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 10); // PTC FAN	SPENTA
			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC	SPENTA

			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("Peltier Accesa ");
			HAL_Delay(500);

			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);

			/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				break; 				//torno nel menu
			}


			/******************* END USCITA CON DOPPIO CLICK ***************/



			while(tCelsius>tExt)	// finchè la temperatura interna è maggiore di quella esterna tengo gli sportellini aperti per velocizzare il raffreddamento
			{

				/****************** COMANDI APERTURA SERVO *********************************/

				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 	// SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);	// SERVO 2 BASSO

				/****************** END COMANDI PER I SERVO *********************************/



				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		// Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; 		// torno nel menu
				}

				/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("Peltier Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);


			}

				//si chiudono gli sportelli: si è raggiunta la temperatura ambiente
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 	//	SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);	// SERVO 2 BASSO
		}
		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//a questo punto si spengono le peltier lasciando accese solo le ventole

		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10);	//PELTIER SPENTE: valore compare basso = 10 (NON ZERO PERCHÈ VIENE ELABORATO COME ALTRO ESTREMO E QUINDI RIMANGONO ACCESE)


		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		HD44780_PrintStr("Peltier Spenta ");
		HAL_Delay(500);

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strc, "%.1f gradi raggiunti", T_Cold); 			//stampa temperatura
		HD44780_PrintStr(strc);
		HAL_Delay(500);




		while(tCelsius<T_Hot) // si compara la temperatura interna della camera con quella da raggiungere
		{
			if(fire==1) 	// se è gia stata attivata prima esci
			{
				fire = 0; 		// ripristino il flag
				break; 			// torno nel menu
			}

			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 1000); // PTC ACCESA: bisogna riscaldare l'interno della camera
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 1000); // PTC FAN ACCESA

			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("PTC Accesa ");
			HAL_Delay(500);

			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);


			/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				break; // torno nel menu
			}


			/******************* END USCITA CON DOPPIO CLICK ***************/


			while(tCelsius<tExt) //se la temperatura interna è minore di quella esterna apro gli sportellini per velocizzare il riscaldamento
			{
				/****************** COMANDI APERTURA SERVO *********************************/

				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 	//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);	// SERVO 2 BASSO

				/****************** END COMANDI PER I SERVO *********************************/



				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; //torno nel menu
				}


				/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("PTC Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);

			}

				// si chiudono gli sportelli quando si raggiunge la temperatura ambiente
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 		//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);		// SERVO 2 BASSO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 10);	 // PELTIER HOT FAN SPENTA
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 10); 	// PELTIER COLD FAN SPENTA

		}
		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//viene spenta la PTC lasciondo accesa la ventola

		__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC SPENTA

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		HD44780_PrintStr("PTC Spenta ");
		HAL_Delay(500);

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strh, "%.1f gradi raggiunti", T_Hot); 			//stampa temperatura
		HD44780_PrintStr(strh);
		HAL_Delay(500);

		Display_Temp(tCelsius, RH);
		HAL_Delay(5000);

		mode = 0; //si esce dal while: termina la modalità 3
	}



	while (mode == 4) // DISCESA: dalla temperatura massima a quella minima
	{

		while(tCelsius<T_Hot) //si compara la temperatura interna della camera con quella da raggiungere
		{
			__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 1000); // PTC ACCESA: bisogna riscaldare l'interno della camera
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 1000); // PTC FAN ACCESA

			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10);	//PELTIER SPENTA
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 10); // PELTIER HOT FAN SPENTA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 10); // PELTIER COLD FAN SPENTA

			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("PTC Accesa ");
			HAL_Delay(500);

			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);


			/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				break; 		// torno nel menu
			}


			/******************* END USCITA CON DOPPIO CLICK ***************/



			while(tCelsius<tExt) //finchè la temperatura interna è minore di quella esterna apro gli sportellini per velocizzare il riscaldamento
			{
				/****************** COMANDI APERTURA SERVO *********************************/

				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 		//SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);		// SERVO 2 BASSO

				/****************** END COMANDI PER I SERVO *********************************/



				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; 		// torno nel menu
				}


				/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("PTC Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);


			}

				//si chiudono gli sportelli: si è raggiunto la temperatura ambiente
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 		// SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);		// SERVO 2 BASSO

		}
		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//si spegne la PTC e viene mantenuta accesa la ventola

		__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC SPENTA

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		HD44780_PrintStr("PTC Spenta ");
		HAL_Delay(500);

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strh, "%.1f gradi raggiunti", T_Hot); 			//stampa temperatura
		HD44780_PrintStr(strh);
		HAL_Delay(500);



		while(tCelsius>T_Cold) //si compara la temperatura interna della camera con quella da raggiungere. Se la prima è maggiore --> bisogna raffreddare --> Peltier accese
		{

			if(fire==1)
			{
				fire = 0; //ripristino il flag
				break; // torno nel menu
			}
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 1000); // PELTIER ACCESA, valore del compare alto (1000)
			__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 1000); // PELTIER HOT FAN  ACCESA
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 1000); // PELTIER COLD FAN ACCESA

			HD44780_Init(2);
			HD44780_Clear();
			HD44780_SetCursor(0,0);
			HAL_Delay(20);
			HD44780_PrintStr("Peltier Accesa ");
			HAL_Delay(500);

			/********************************* LETTURA TEMPERATURA ***************************/

			if(DHT22_Start(DHT22_PIN, DHT22_PORT))
			{
				RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
				RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
				TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
				TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
				SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
				CHECK = RH1 + RH2 + TC1 + TC2;
				if (CHECK == SUM)
				{
					if (TC1>127) // If TC1=10000000, negative temperature
					{
						tCelsius = (float)TC2/10*(-1);
					}
					else
					{
						tCelsius = (float)((TC1<<8)|TC2)/10;
					}
					tFahrenheit = tCelsius * 9/5 + 32;
					RH = (float) ((RH1<<8)|RH2)/10;
				}
			}


			/********************************* END LETTURA TEMPERATURA ***************************/


			HAL_Delay(200);
			Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
			HAL_Delay(1000);
			Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
			HAL_Delay(1000);


			/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

			FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


			if(fire==1)
			{
				break; // torno nel menu
			}

			/******************* END USCITA CON DOPPIO CLICK ***************/


			while(tCelsius>tExt)	// se la temperatura interna è maggiore di quella esterna tengo gli sportellini aperti per velocizzare il raffreddamento
			{

				/****************** COMANDI PER I SERVO *********************************/

				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 230); 	//	SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 230);	// 	SERVO 2 BASSO


				/****************** END COMANDI PER I SERVO *********************************/



				/******************* USCITA CON DOPPIO CLICK ( O TENENEDO PREMUTO) ***************/

				FireExit();		//Se la funzione FireExit() ha fatto scattare l'interruzione


				if(fire==1)
				{
					break; // torno nel menu
				}


				/******************* END USCITA CON DOPPIO CLICK ***************/

				HD44780_Init(2);
				HD44780_Clear();
				HD44780_SetCursor(0,0);
				HAL_Delay(20);
				HD44780_PrintStr("Peltier Accesa ");
				HAL_Delay(500);

				/********************************* LETTURA TEMPERATURA ***************************/

				if(DHT22_Start(DHT22_PIN, DHT22_PORT))
				{
					RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
					RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
					TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
					TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
					SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
					CHECK = RH1 + RH2 + TC1 + TC2;
					if (CHECK == SUM)
					{
						if (TC1>127) // If TC1=10000000, negative temperature
						{
							tCelsius = (float)TC2/10*(-1);
						}
						else
						{
							tCelsius = (float)((TC1<<8)|TC2)/10;
						}
						tFahrenheit = tCelsius * 9/5 + 32;
						RH = (float) ((RH1<<8)|RH2)/10;
					}
				}


				/********************************* END LETTURA TEMPERATURA ***************************/


				HAL_Delay(200);
				Display_Temp(tCelsius, RH);			// stampo temperatura e umidità interne su LCD
				HAL_Delay(1000);
				Display_Temp_EXT(tExt, RH_EXT);		// stampo temperatura e umidità esterne su LCD
				HAL_Delay(1000);


			}

				//si chiudono gli sportelli: si è raggiunta la temperatura ambiente
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1, 700); 	// SERVO 1 ALTO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_2, 700);	// SERVO 2 BASSO
				HAL_Delay(500);
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 10); 	// PTC FAN SPENTA

		}
		//si esce dall'IF non appena la temperatura interna sarà uguale alla temperatura da raggiungere
		//a questo punto vengono spente le celle di peltier lasciando accese solo le ventole

		__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10);	//PELTIER SPENTA: valore compare basso = 10 (NON ZERO PERCHÈ VIENE ELABORATO COME ALTRO ESTREMO E QUINDI RIMANGONO ACCESE)

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		HD44780_PrintStr("Peltier Spenta ");
		HAL_Delay(500);

		HD44780_Init(2);
		HD44780_Clear();
		HD44780_SetCursor(0,0);
		HAL_Delay(20);
		sprintf (strc, "%.1f gradi raggiunti", T_Cold); 			//stampa temperatura
		HD44780_PrintStr(strc);
		HAL_Delay(500);

		Display_Temp(tCelsius, RH);
		HAL_Delay(5000);

		mode = 0; //si esce dal while: termina la modalità 4
	}

}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */




  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc2);
  HAL_TIM_Base_Start(&htim1);


  	/********************************* LETTURA TEMPERATURA ***************************/

  if(DHT22_Start(DHT22_PIN, DHT22_PORT))
  {
	  RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
	  RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
	  TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
	  TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
	  SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
	  CHECK = RH1 + RH2 + TC1 + TC2;
	  if (CHECK == SUM)
	  {
		  if (TC1>127) // If TC1=10000000, negative temperature
		  {
			  tCelsius = (float)TC2/10*(-1);
		  }
		  else
		  {
			  tCelsius = (float)((TC1<<8)|TC2)/10;
		  }
		  tFahrenheit = tCelsius * 9/5 + 32;
		  RH = (float) ((RH1<<8)|RH2)/10;
	  }
  }

  if(DHT22_Start(DHT22_PIN_EXT, DHT22_PORT))
  {
	  RH1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of humidity
	  RH2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Relative humidity
	  TC1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of Celsius
	  TC2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Celsius
	  SUM_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Check sum
	  CHECK_EXT = RH1_EXT + RH2_EXT + TC1_EXT + TC2_EXT;
	  if (CHECK_EXT == SUM_EXT)
	  {
		  if (TC1_EXT>127) // If TC1=10000000, negative temperature
		  {
			  tCelsius_EXT = (float)TC2_EXT/10*(-1);
		  }
		  else
		  {
			  tCelsius_EXT = (float)((TC1_EXT<<8)|TC2_EXT)/10;
		  }
		  tFahrenheit_EXT = tCelsius_EXT * 9/5 + 32;
		  RH_EXT = (float) ((RH1_EXT<<8)|RH2_EXT)/10;
	  }
  }


  	/********************************* END LETTURA TEMPERATURA ***************************/



   	/******************** START INIZIALIZZAZIONE PWM ****************************/


    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);	// PELTIER
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);	// PELTIER HOT FAN
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);	// PELTIER COLD FAN
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);	// PTC FAN
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);	// PTC
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);	// SERVO 1
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);	// SERVO 2

    HAL_Delay(200);
    HAL_GPIO_WritePin (VENTOLINA_GPIO_Port, VENTOLINA_Pin ,SET); // VENTOLINA ACCESA
    HAL_Delay(200);

    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_1, 10); // PELTIER SPENTA
    __HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, 10); // PELTIER HOT FAN  SPENTA
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, 10); // PELTIER COLD FAN SPENTA

    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, 10); // PTC FAN	SPENTA
    __HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1, 10); // PTC	SPENTA


    /******************** END INIZIALIZZAZIONE PWM ****************************/


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


	  	  	  	  	  	  	  	  	  	  /******************** REED SWITCH *******************/

	  if(HAL_GPIO_ReadPin(STATO_PORTA_GPIO_Port, STATO_PORTA_Pin)==GPIO_PIN_RESET) //Tensione bassa (RESET) perchè quando si avvicina il magnete allo switch si chiude il circuito è la ddp=0. CIÓ ACCADE QUANDO LA PORTA È CHIUSA
	  {

		  HD44780_Init(2);
		  HD44780_Clear();
		  HD44780_SetCursor(0,0);
		  HAL_Delay(20);
		  HD44780_PrintStr("Porta Chiusa");

	  }

	  else //Tensione bassa (SET) perchè quando si allontana il magnete dallo switch si apre il circuito è la ddp=3V. CIÓ ACCADE QUANDO LA PORTA È APERTA
	  {

		  HD44780_Init(2);
		  HD44780_Clear();
		  HD44780_SetCursor(0,0);
		  HAL_Delay(20);
		  HD44780_PrintStr("Porta Aperta");
	  }

	  	  	  	  	  	  	  	  	  	 /******************** END REED SWITCH *******************/



	  	  	  	  	  	  	  	  	  	  /*******************  JOYSTICK  *******************/

	  /*
	   * Questa è uno script che permette di acquisire e quindi aggiornare i valori dei joystick. Necessario da richiamare ogni volta che si vuole
	   * riacquisire la posizione dell'analogico
	   */


	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_Start(&hadc2);

	  HAL_ADC_PollForConversion(&hadc1,1000);
	  readValueX = HAL_ADC_GetValue(&hadc1);
	  HAL_ADC_PollForConversion(&hadc2,1000);
	  readValueY = HAL_ADC_GetValue(&hadc2);

	  	  	  	  	  	  	  	  	  	  /*******************  END JOYSTICK  *******************/



	  	  	  	  	  	  	  	  	  	  /******************* SENSORE DHT22 *******************/

	  /*
	   * Questa è una condizione if che è necessario richiamare ogni qual volta si fa uso del sensore, in modo che i valori acquisiti dal sensore
	   * vengano aggiornati, man mano
	   */

	  if(DHT22_Start(DHT22_PIN, DHT22_PORT))
	  {
		  RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
		  RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
		  TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
		  TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
		  SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
		  CHECK = RH1 + RH2 + TC1 + TC2;
		  if (CHECK == SUM)
		  {
			  if (TC1>127) // If TC1=10000000, negative temperature
			  {
				  tCelsius = (float)TC2/10*(-1);
			  }
			  else
			  {
				  tCelsius = (float)((TC1<<8)|TC2)/10;
			  }
			  tFahrenheit = tCelsius * 9/5 + 32;
			  RH = (float) ((RH1<<8)|RH2)/10;
		  }
	  }

	  if(DHT22_Start(DHT22_PIN_EXT, DHT22_PORT))
	  {
		  RH1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of humidity
		  RH2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Relative humidity
		  TC1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of Celsius
		  TC2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Celsius
		  SUM_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Check sum
		  CHECK_EXT = RH1_EXT + RH2_EXT + TC1_EXT + TC2_EXT;
		  if (CHECK_EXT == SUM_EXT)
		  {
			  if (TC1_EXT>127) // If TC1=10000000, negative temperature
			  {
				  tCelsius_EXT = (float)TC2_EXT/10*(-1);
			  }
			  else
			  {
				  tCelsius_EXT = (float)((TC1_EXT<<8)|TC2_EXT)/10;
			  }
			  tFahrenheit_EXT = tCelsius_EXT * 9/5 + 32;
			  RH_EXT = (float) ((RH1_EXT<<8)|RH2_EXT)/10;
		  }
	  }



	  	  	  	  	  	  	  	  	  	  	  /******************* END SENSORE DHT22 *******************/



	  	  	  	  	  	  	  	  	  	  /************************** MENU SUL DISPLAY ********************************/


	  scelta = 0; 			//variabile che determina l'uscita dal menù

	  HD44780_Init(2);
	  HD44780_Clear();
	  HD44780_SetCursor(0,0);
	  HAL_Delay(20);
	  HD44780_PrintStr("Menu");
	  HAL_Delay(2000);



	  comando = 1; 			//variabile che determina quale voce visualizzare nel menù




	  while(scelta == 0)	//se scelta = 0 allora si resta all'interno del menù. Se scelta = 1 si esce dal menù
	  {
		  while(comando == 1)	//finchè comando = 1 si visualizza la modalità 1 sul display
		  {
			  HD44780_Init(2);
			  HD44780_Clear();
			  HD44780_SetCursor(0,0);
			  HD44780_PrintStr("Seleziona modalita: ");
			  HAL_Delay(20);
			  sprintf (strm1, "1: Tset %.1f gradi ", T_Cold); 			//stampa temperatura
			  HD44780_PrintStr(strm1);


			  scenario = 1; 	//EQUIVALENTE DI "mode": PARAMETRO CHE VIENE PASSATO ALLA FUNZIONE MODALITÀ


			  	  	  	  	  	  	  	  /************* REINIZIALIZZAZIONE JOYSTICK ********************/

			  HAL_ADC_Start(&hadc1);
			  HAL_ADC_Start(&hadc2);
			  HAL_ADC_PollForConversion(&hadc1,1000);
			  readValueX = HAL_ADC_GetValue(&hadc1);
			  HAL_ADC_PollForConversion(&hadc2,1000);
			  readValueY = HAL_ADC_GetValue(&hadc2);

			  	  	  	  	  	  	  	  /****************** END REINIZIALIZZAZIONE JOYSTICK ***************/


			  HAL_Delay(20);
			  if (readValueY < 1000)
			  {
				  comando = 2;	// muovendo il joystick verso il basso si visualizza la modalità 2 sul menù
			  }
			  else if(readValueY > 3700)
			  {
				  comando = 4;	// muovendo il joystick verso l'alto si visualizza la modalità 3 sul menù
			  }

			  if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET) // premendo il joystick allora si seleziona la modalità 1
			  {

				  HAL_Delay(200);
				  HD44780_Init(2);
				  HD44780_Clear();
				  HD44780_SetCursor(0,0);
				  HAL_Delay(20);
				  HD44780_PrintStr("Scenario 1 selezionato! ");
				  HAL_Delay(500);

				  Modalita(scenario, tCelsius, RH); //--> verrà eseguita la modalità 1
			  }
		  }


		  while(comando == 2)
		  {
			  HD44780_Init(2);
			  HD44780_Clear();
			  HD44780_SetCursor(0,0);
			  HD44780_PrintStr("Seleziona modalita: ");
			  HAL_Delay(20);
			  sprintf (strm2, "2: Tset %.1f gradi ", T_Hot); 			//stampa temperatura
			  HD44780_PrintStr(strm2);

			  scenario = 2;


			  	  	  	  	  	  	  /******************  REINIZIALIZZAZIONE JOYSTICK ***************/


			  HAL_ADC_Start(&hadc1);
			  HAL_ADC_Start(&hadc2);
			  HAL_ADC_PollForConversion(&hadc1,1000);
			  readValueX = HAL_ADC_GetValue(&hadc1);
			  HAL_ADC_PollForConversion(&hadc2,1000);
			  readValueY = HAL_ADC_GetValue(&hadc2);

			  	  	  	  	  	  	  /****************** END REINIZIALIZZAZIONE JOYSTICK ***************/

			  if (readValueY < 1000)
			  {
				  comando = 3;
			  }
			  else if(readValueY > 3700)
			  {
				  comando = 1;
			  }
			  if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET)
			  {

				  HAL_Delay(200);
				  HD44780_Init(2);
				  HD44780_Clear();
				  HD44780_SetCursor(0,0);
				  HAL_Delay(20);
				  HD44780_PrintStr("Scenario 2 selezionato! ");
				  HAL_Delay(500);

				  Modalita(scenario, tCelsius, RH);
			  }
		  }



		  while(comando == 3)
		  {
			  HD44780_Init(2);
			  HD44780_Clear();
			  HD44780_SetCursor(0,0);
			  HD44780_PrintStr("Seleziona modalita: ");
			  HAL_Delay(20);
			  sprintf (strm3, "3: Tset %.1f -> %.1f ", T_Cold, T_Hot); 			//stampa temperatura
			  HD44780_PrintStr(strm3);

			  scenario = 3;

			  	  	  	  	  	  /****************** REINIZIALIZZAZIONE JOYSTICK ***************/

			  HAL_ADC_Start(&hadc1);
			  HAL_ADC_Start(&hadc2);
			  HAL_ADC_PollForConversion(&hadc1,1000);
			  readValueX = HAL_ADC_GetValue(&hadc1);
			  HAL_ADC_PollForConversion(&hadc2,1000);
			  readValueY = HAL_ADC_GetValue(&hadc2);

			  	  	  	  	  	  /****************** END REINIZIALIZZAZIONE JOYSTICK ***************/

			  if (readValueY < 1000)
			  {
				  comando = 4;
			  }
			  else if(readValueY > 3700)
			  {
				  comando = 2;
			  }

			  if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET)
			  {

				  HAL_Delay(200);
				  HD44780_Init(2);
				  HD44780_Clear();
				  HD44780_SetCursor(0,0);
				  HAL_Delay(20);
				  HD44780_PrintStr("Scenario 3 selezionato! ");
				  HAL_Delay(500);

				  Modalita(scenario, tCelsius, RH);
			  }
		  }


		  while(comando == 4)
		  {
			  HD44780_Init(2);
			  HD44780_Clear();
			  HD44780_SetCursor(0,0);
			  HD44780_PrintStr("Seleziona modalita: ");
			  HAL_Delay(20);
			  sprintf (strm4, "4: Tset %.1f -> %.1f ", T_Hot, T_Cold); 			//stampa temperatura
			  HD44780_PrintStr(strm4);


			  HAL_Delay(20);
			  scenario = 4;

			  	  	  	  	  	  	  /****************** REINIZIALIZZAZIONE JOYSTICK ***************/

			  HAL_ADC_Start(&hadc1);
			  HAL_ADC_Start(&hadc2);
			  HAL_ADC_PollForConversion(&hadc1,1000);
			  readValueX = HAL_ADC_GetValue(&hadc1);
			  HAL_ADC_PollForConversion(&hadc2,1000);
			  readValueY = HAL_ADC_GetValue(&hadc2);

			  	  	  	  	  	  	  /****************** END REINIZIALIZZAZIONE JOYSTICK ***************/

			  if (readValueY < 1000)
			  {
				  comando = 1;
			  }
			  else if(readValueY > 3700)
			  {
				  comando = 3;
			  }

			  if(HAL_GPIO_ReadPin(TASTO_JOYSTICK_GPIO_Port, TASTO_JOYSTICK_Pin)==GPIO_PIN_RESET)
			  {

				  HAL_Delay(200);
				  HD44780_Init(2);
				  HD44780_Clear();
				  HD44780_SetCursor(0,0);
				  HAL_Delay(20);
				  HD44780_PrintStr("Scenario 4 selezionato! ");
				  HAL_Delay(500);

				  Modalita(scenario, tCelsius, RH);
			  }
		  }
	  }


	  	  	  	  	  /************************** END MENU SUL DISPLAY ****************************/




	  	  	  	  	  /********************************* LETTURA TEMPERATURA ***************************/

	  if(DHT22_Start(DHT22_PIN, DHT22_PORT))
	  {
		  RH1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of humidity
		  RH2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Relative humidity
		  TC1 = DHT22_Read(DHT22_PIN, DHT22_PORT); // First 8bits of Celsius
		  TC2 = DHT22_Read(DHT22_PIN, DHT22_PORT); // Second 8bits of Celsius
		  SUM = DHT22_Read(DHT22_PIN, DHT22_PORT); // Check sum
		  CHECK = RH1 + RH2 + TC1 + TC2;
		  if (CHECK == SUM)
		  {
			  if (TC1>127) // If TC1=10000000, negative temperature
			  {
				  tCelsius = (float)TC2/10*(-1);
			  }
			  else
			  {
				  tCelsius = (float)((TC1<<8)|TC2)/10;
			  }
			  tFahrenheit = tCelsius * 9/5 + 32;
			  RH = (float) ((RH1<<8)|RH2)/10;
		  }
	  }

	  if(DHT22_Start(DHT22_PIN_EXT, DHT22_PORT))
	  {
		  RH1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of humidity
		  RH2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Relative humidity
		  TC1_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // First 8bits of Celsius
		  TC2_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Second 8bits of Celsius
		  SUM_EXT = DHT22_Read(DHT22_PIN_EXT, DHT22_PORT); // Check sum
		  CHECK_EXT = RH1_EXT + RH2_EXT + TC1_EXT + TC2_EXT;
		  if (CHECK_EXT == SUM_EXT)
		  {
			  if (TC1_EXT>127) // If TC1=10000000, negative temperature
			  {
				  tCelsius_EXT = (float)TC2_EXT/10*(-1);
			  }
			  else
			  {
				  tCelsius_EXT = (float)((TC1_EXT<<8)|TC2_EXT)/10;
			  }
			  tFahrenheit_EXT = tCelsius_EXT * 9/5 + 32;
			  RH_EXT = (float) ((RH1_EXT<<8)|RH2_EXT)/10;
		  }
	  }


	  	  	  	  	  	  /********************************* END LETTURA TEMPERATURA ***************************/


	  HAL_Delay(200);
	  Display_Temp(tCelsius, RH);	//stampa temperatura e umidità
	  HAL_Delay(1000);





    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 144-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 9999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 72-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 999;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  HAL_TIM_MspPostInit(&htim5);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, VENTOLINA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SENSORE_TEMP_INT_Pin|SENSORE_TEMP_EXT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TASTO_JOYSTICK_Pin */
  GPIO_InitStruct.Pin = TASTO_JOYSTICK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TASTO_JOYSTICK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : VENTOLINA_Pin SENSORE_TEMP_3_Pin SENSORE_TEMP_4_Pin */
  GPIO_InitStruct.Pin = VENTOLINA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SENSORE_TEMP_INT_Pin SENSORE_TEMP_EXT_Pin */
  GPIO_InitStruct.Pin = SENSORE_TEMP_INT_Pin|SENSORE_TEMP_EXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : STATO_PORTA_Pin */
  GPIO_InitStruct.Pin = STATO_PORTA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(STATO_PORTA_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
