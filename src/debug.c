/*
 * debug.c
 *
 *  Created on: 2013.10.11.
 *      Author: Rimóczi Péter
 *
 *  A debuggolásoz szükséges függvények és változók.
 *
 */

#include <stm32f4xx_conf.h>
#include <string.h>
#include <stdarg.h>
#include <debug.h>

// DEBUG Változók

volatile char 		DEBUG_bt_processed_args[DEBUG_MAX_COMMAND_ARGS][DEBUG_MAX_ARG_LENGTH];	// parancsargumentumok tárolására használt változó
volatile uint16_t 	DEBUG_bt_pa_p=0; 								// Parancsargumentumok argumentum pointere
volatile uint16_t 	DEBUG_bt_pap_p=0; 								// Parancsargumentum argumentum hossz pointere
volatile char 		DEBUG_bt_received[DEBUG_INCOMING_BUFFER_SIZE]; 	// USARTon kapott string deklarálása
volatile uint16_t 	DEBUG_bt_r_p=0; 								// USARTon kapott string jelenlegi pointere
volatile uint16_t 	DEBUG_bt_rd_p=0; 								// USARTon kapott string beolvasási pointere
volatile char 		DEBUG_bt_processed[DEBUG_PROCESS_BUFFER_SIZE]; 	// Feldolgozás alatt álló parancs buffere
volatile uint16_t 	DEBUG_bt_pd_p=0; 								// Feldolgozás alatt álló parancs bufferpointere
volatile char 		DEBUG_bbx_buffer[DEBUG_BLACKBOX_BUFFER_SIZE]; 	// Belsõ blackbox tároló
volatile uint16_t	DEBUG_bbx_p=0; 									// Belsõ blackbox tároló pointere
volatile uint8_t 	DEBUG_FLAG=0b00000000;							// ECHO mód állapotváltozója USARTon
																	/*
																	 * DEBUG_FLAG változók:
																	 * 	bit0 - BEÁLLÍTÁS - DEBUG aktiválása
																	 * 	bit1 - BEÁLLÍTÁS - USART ECHO mód
																	 * 	bit2 - BEÁLLÍTÁS - BlackBox mód
																	 * 	bit3 - BEÁLLÍTÁS - Automata idõzített mód
																	 * 	bit4 - N/O
																	 * 	bit5 - ÜZEMI - Ismeretlen hibaüzenet jelzõ
																	 * 	bit6 - ÜZEMI - Parancshiba a feldolgozásnál
																	 * 	bit7 - ÜZEMI - Ismeretlen parancs jelzõ bit
																	 */

/********************************************************/
/*  FÜGGVÉNYEK 											*/
/********************************************************/

/**
  * @brief  Configure Debugger parameters.
  * @param  flag: configuration flagset.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_Configure(uint8_t flag)
{
	DEBUG_FLAG=flag;
}

/**
  * @brief  Get the configuration parameters for the Debugger.
  * @param  n/a: n/a.
  *            @arg n/a: n/a.
  * @retval The DEBUG_FLAG 8bit flagset.
  */
uint8_t Debug_Get_Config(void)
{
	return DEBUG_FLAG;
}

/**
  * @brief  Push out the existing data and clear the Debugger Blackbox.
  * @param  n/a: n/a.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_Bbx_Push(void)
{
	if (DEBUG_FLAG & 0x01)
	{
		DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // elõtte az utolsó karakterrel lezárja a stringet
		USART3_puts(DEBUG_bbx_buffer); // ha nincs engedélyezve a BBX akkor azonnali fizikai kiküldés
		DEBUG_bbx_p=0;
	}
}

/**
  * @brief  Handle incoming data from USART3 for Debugger.
  * @param  n/a: n/a.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void USART3_IRQHandler(void)
{
	if ( USART_GetITStatus(USART3, USART_IT_RXNE) && (DEBUG_FLAG & 0x01) ) // ha kapunk valamit USARTon
	{
		GPIOD->BSRRL = 0x4000; // akkor felvillantjuk a PIROS ledet míg feldolgozzuk
		DEBUG_bt_received[DEBUG_bt_r_p] = USART3->DR; // a beérkezõ bufferbe bírjuk
		USART_ClearFlag(USART3,USART_IT_RXNE); // megszakítás megvan, fogadhatunk újabb adatot
		DEBUG_bt_r_p++; // annak pointerét növeljük
		if (DEBUG_bt_r_p>DEBUG_INCOMING_BUFFER_SIZE) DEBUG_bt_r_p=0; // ha túlnöveltük volna, akkor reseteljük
		if (DEBUG_bt_r_p==DEBUG_bt_rd_p || (DEBUG_bt_r_p==DEBUG_INCOMING_BUFFER_SIZE && DEBUG_bt_rd_p==0)) Debug_Handle(); // EZ FONTOS! ha túllépnénk a feldolgozás alatt álló karakteren a beérkezõ bufferben, akkor elõtte azonnali feldolgozást kérünk
		GPIOD->BSRRH = 0x4000; // led kikapcs
	}
}

/**
  * @brief  Send out data trough usart3.
  * @param  *s: string containing the message to be sent out.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void USART3_puts(volatile char *s) {
	while (*s) {
		while (!(USART3->SR & 0x00000040))
			;
		USART_SendData(USART3, *s);
		*s++;
	}
}



/**
  * @brief  Uniqe printf variant for Debugger.
  * @param  *format: the string containing the format characters.
  * 	(Example: "ccfdi" means that the actual argument list consists of a string, string, float, double, integer type data)
  *            @arg n/a: n/a.
  * @param  ...: the rest of the arguments.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_Print(volatile char *format, ...)
{
	if (DEBUG_FLAG & 0x01)
	{
		char buffer[256]; // definiálunk egy munkabuffert
		volatile char *ch;
		int i=0; // és egy léptetõregisztert hozzá
		va_list arg; // változtatható argumentumlistával dolgozunk, tehát listaváltozó csnálunk hozzá
		va_start(arg, format); // kezdjük az elsõ változóval

		while (*format)
		{
			if (*format=='c')
			{
				ch=va_arg(arg,char *);
				while (*ch)
				{
					buffer[i]=*ch;
					i++;
					*ch++;
				}
			}
			/* TODO: NOT YET PROPERLY IMPLEMENTED
			if (*format=='d' || *format=='f')
			{
				int j=0;
				double d;
				d=va_arg(arg,double);
				volatile char dbuf[sizeof(d)+1];
				*(double*)dbuf = d;
				dbuf[sizeof(d)+1]='\0';
				while (dbuf[j])
				{
					buffer[i]=dbuf[j];
					i++;
					j++;
				}
			}
			if (*format=='i')
			{
				int j=0;
				int in;
				in=va_arg(arg,int);
				volatile char inbuf[sizeof(in)+1];
				*(int*)inbuf = in;
				inbuf[sizeof(in)+1]='\0';
				while (inbuf[j])
				{
					buffer[i]=inbuf[j];
					i++;
					j++;
				}
			}*/
			*format++;
			buffer[i]='\0';
		}

		i=0;

		while (buffer[i]!='\0') // ha a bufferben tárolt string végére érünk, akkor nem többet a BBXbe tenni
		{
			if (DEBUG_bbx_p==DEBUG_BLACKBOX_BUFFER_SIZE-1) // ha betelt a blackbox, akkor kiküld mindent
			{
				DEBUG_bbx_p++;
				DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // elõtte az utolsó karakterrel lezárja a stringet
				USART3_puts(DEBUG_bbx_buffer); // fizikai kiküldés
				DEBUG_bbx_p=0; // és reseteljük a BBXot
			}
			DEBUG_bbx_buffer[DEBUG_bbx_p]=buffer[i]; // egyébiránt töltjük tovább a buffert
			DEBUG_bbx_p++;
			i++;
		}
		if (!(DEBUG_FLAG & 0x04)) // amikor kiírunk, elõtte megnézzük hova megy az üzenet BBXbe, vagy közvetlenül ki
		{
			DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // elõtte az utolsó karakterrel lezárja a stringet
			USART3_puts(DEBUG_bbx_buffer); // ha nincs engedélyezve a BBX akkor azonnali fizikai kiküldés
			DEBUG_bbx_p=0;
		}
		va_end(arg);
	}
}


/**
  * @brief  The actual handler script for the communication. This should be in the main program, with high priority settings.
  * @param  n/a: n/a.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_Handle(void)
{
	if ((DEBUG_bt_rd_p!=DEBUG_bt_r_p || DEBUG_bt_received[DEBUG_bt_rd_p]!='\0') && (DEBUG_FLAG & 0x01))
	{
		// lehetõleg a kocsi-vissza karaktert ne fogadjuk be, mert linux és windows kliens alatt nem ugyanaz
		if (DEBUG_bt_pd_p<DEBUG_PROCESS_BUFFER_SIZE) // túlcsordul-e a process tömb?
		{
			// ha nem, akkor írjuk be az új karakter és töröljük a beérkezõ tömb olvasott elemét
			if (DEBUG_bt_received[DEBUG_bt_rd_p]!='\n')
			{
				DEBUG_bt_processed[DEBUG_bt_pd_p]=DEBUG_bt_received[DEBUG_bt_rd_p];
				DEBUG_bt_pd_p++;

				// ha \n-nel véget ér egy parancs azt rögtön dolgozzuk is fel
				if (DEBUG_bt_processed[DEBUG_bt_pd_p-1]=='\r')
				{
					DEBUG_bt_processed[DEBUG_bt_pd_p]='\0';
					if (DEBUG_FLAG & 0x02) Debug_Print("cc","ECHO: ",DEBUG_bt_processed);
					DEBUG_bt_pd_p=0; // kezdjük elölrõl a process buffert, de most az argumentumokra bontásra használjuk
					DEBUG_bt_pa_p=0; // állítsuk be az argmuentumtároló változó 2 dimenziós tömbjének mutatóit
					DEBUG_bt_pap_p=0;
					while (DEBUG_bt_processed[DEBUG_bt_pd_p]!='\r' && !(DEBUG_FLAG & 0x40)) // amíg a parancs végére nem érünk
					{
						if (DEBUG_bt_pa_p>DEBUG_MAX_COMMAND_ARGS) // túl sok argument?
						{
							Debug_Error(3); // argumentum szám túlcsordulás
							DEBUG_FLAG|=0x40; // FLAGgel jelezzük hogy sikertelen a parancsfeldolgozás, ne küldje el az argumentumokat sehova a program
						}

						if (DEBUG_bt_pap_p>DEBUG_MAX_ARG_LENGTH) // túlcsordul az argumenthossz?
						{
							Debug_Error(4); // argumenthossz túlcsordulás
							DEBUG_FLAG|=0x40; // FLAGgel jelezzük hogy sikertelen a parancsfeldolgozás, ne küldje el az argumentumokat sehova a program
						}

						if (!(DEBUG_FLAG & 0x40)) // ha már hibás a parancs akkor ne írjunk semmit sehová, hiszen túlcsordulást és hardfaultot okozhatunk
						{
							if (DEBUG_bt_processed[DEBUG_bt_pd_p]!=' ') // ha szóköz van akkor új argumentum kell, egyébiránt írjuk tovább az aktuális argumentumot
							{
								DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]=DEBUG_bt_processed[DEBUG_bt_pd_p];
								DEBUG_bt_pap_p++;
							}
							else // ha szóköz van akkor újabb argumentum következik, és annak kezdõpointere resetelõdik
							{
								DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]='\0'; // elõzõ argumentum végét le kell zárni!
								DEBUG_bt_pap_p=0;
								DEBUG_bt_pa_p++;
							}
						}

						// ha más nincs akkor jön a következõ processben lévõ karakter
						DEBUG_bt_pd_p++;
					}

					DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]='\0'; // az utolsó argumentum lezárása

					// megvannak az argumentumok, ideje feldolgozni õket!
					if (!(DEBUG_FLAG & 0x40))
					{
						DEBUG_FLAG|=0x80; // kezdetnek úgy vesszük, hogy ismeretlen parancsot kaptunk
						uint8_t i=0;
						for (i=0;i<DEBUG_COMMAND_NUM+1;i++) // keresni kezdünk a meglévõ definiált parancsok között
						{
							if (!strcmp(DEBUG_COMMANDS[i].name,DEBUG_bt_processed_args[0])) // megnézzük van e azonos nevû a az elsõ argumentummal
							{
								DEBUG_COMMANDS[i].func(DEBUG_bt_processed_args); // ha van, akkor futtatjuk a parancsot az összes argumentummal
								DEBUG_FLAG&=~0x80; // amennyiben ismert parancs volt, úgy kivesszük a jelzõflaget
							}
						}
					}

					if (DEBUG_FLAG & 0x80) // ismeretlen parancsot kaptunk?
					{
						Debug_Error(2); // ismeretlen parancs hiba
					}

					DEBUG_bt_pd_p=0; // ha mindennel elkészültünk akkor reseteljük az összes pointert
					DEBUG_bt_pa_p=0;
					DEBUG_bt_pap_p=0;
					DEBUG_FLAG&=~0xC0; // parancshiba/ismeretlen parancs esetén a két jelzõ flaget reseteljük
				}
			}

			DEBUG_bt_received[DEBUG_bt_rd_p]='\0';
			DEBUG_bt_rd_p++;
			// ha viszont körbeértünk, akkor ugorjunk az elejére
			if (DEBUG_bt_rd_p>DEBUG_INCOMING_BUFFER_SIZE) DEBUG_bt_rd_p=0;
		}
		else // ha túlcsordul akkor hibaüzenet, és pointer vissza az elejére, elveszett parancsot kaptunk
		{
			DEBUG_bt_pd_p=0;
			Debug_Error(0); // 0-ás hibaüzenet, process buffer túlcsordult
		}

	}
}


/**
  * @brief  Sends out error messages trough usart3.
  * @param  ec: the error code based on the error message definitions.
  *            @arg n/a: n/a.
  * @retval n/a.
  */
void Debug_Error(int ec)
{
	if (DEBUG_FLAG & 0x01)
	{
		uint8_t i=0; // keressük a hibaüzenetet
		for (i=0;i<DEBUG_ERROR_NUM;i++)
		{
			if (DEBUG_ERRORS[i].errornum==ec)
			{
				USART3_puts(DEBUG_ERRORS[i].msg); // ha találtunk akkor azonnal kiküldjük fizikai üzenetként
			}
		}
	}
}
