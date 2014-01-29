/*
 * debug.c
 *
 *  Created on: 2013.10.11.
 *      Author: Rim�czi P�ter
 *
 *  A debuggol�soz sz�ks�ges f�ggv�nyek �s v�ltoz�k.
 *
 */

#include <stm32f4xx_conf.h>
#include <string.h>
#include <stdarg.h>
#include <debug.h>

// DEBUG V�ltoz�k

volatile char 		DEBUG_bt_processed_args[DEBUG_MAX_COMMAND_ARGS][DEBUG_MAX_ARG_LENGTH];	// parancsargumentumok t�rol�s�ra haszn�lt v�ltoz�
volatile uint16_t 	DEBUG_bt_pa_p=0; 								// Parancsargumentumok argumentum pointere
volatile uint16_t 	DEBUG_bt_pap_p=0; 								// Parancsargumentum argumentum hossz pointere
volatile char 		DEBUG_bt_received[DEBUG_INCOMING_BUFFER_SIZE]; 	// USARTon kapott string deklar�l�sa
volatile uint16_t 	DEBUG_bt_r_p=0; 								// USARTon kapott string jelenlegi pointere
volatile uint16_t 	DEBUG_bt_rd_p=0; 								// USARTon kapott string beolvas�si pointere
volatile char 		DEBUG_bt_processed[DEBUG_PROCESS_BUFFER_SIZE]; 	// Feldolgoz�s alatt �ll� parancs buffere
volatile uint16_t 	DEBUG_bt_pd_p=0; 								// Feldolgoz�s alatt �ll� parancs bufferpointere
volatile char 		DEBUG_bbx_buffer[DEBUG_BLACKBOX_BUFFER_SIZE]; 	// Bels� blackbox t�rol�
volatile uint16_t	DEBUG_bbx_p=0; 									// Bels� blackbox t�rol� pointere
volatile uint8_t 	DEBUG_FLAG=0b00000000;							// ECHO m�d �llapotv�ltoz�ja USARTon
																	/*
																	 * DEBUG_FLAG v�ltoz�k:
																	 * 	bit0 - BE�LL�T�S - DEBUG aktiv�l�sa
																	 * 	bit1 - BE�LL�T�S - USART ECHO m�d
																	 * 	bit2 - BE�LL�T�S - BlackBox m�d
																	 * 	bit3 - BE�LL�T�S - Automata id�z�tett m�d
																	 * 	bit4 - N/O
																	 * 	bit5 - �ZEMI - Ismeretlen hiba�zenet jelz�
																	 * 	bit6 - �ZEMI - Parancshiba a feldolgoz�sn�l
																	 * 	bit7 - �ZEMI - Ismeretlen parancs jelz� bit
																	 */

/********************************************************/
/*  F�GGV�NYEK 											*/
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
		DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // el�tte az utols� karakterrel lez�rja a stringet
		USART3_puts(DEBUG_bbx_buffer); // ha nincs enged�lyezve a BBX akkor azonnali fizikai kik�ld�s
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
		GPIOD->BSRRL = 0x4000; // akkor felvillantjuk a PIROS ledet m�g feldolgozzuk
		DEBUG_bt_received[DEBUG_bt_r_p] = USART3->DR; // a be�rkez� bufferbe b�rjuk
		USART_ClearFlag(USART3,USART_IT_RXNE); // megszak�t�s megvan, fogadhatunk �jabb adatot
		DEBUG_bt_r_p++; // annak pointer�t n�velj�k
		if (DEBUG_bt_r_p>DEBUG_INCOMING_BUFFER_SIZE) DEBUG_bt_r_p=0; // ha t�ln�velt�k volna, akkor resetelj�k
		if (DEBUG_bt_r_p==DEBUG_bt_rd_p || (DEBUG_bt_r_p==DEBUG_INCOMING_BUFFER_SIZE && DEBUG_bt_rd_p==0)) Debug_Handle(); // EZ FONTOS! ha t�ll�pn�nk a feldolgoz�s alatt �ll� karakteren a be�rkez� bufferben, akkor el�tte azonnali feldolgoz�st k�r�nk
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
		char buffer[256]; // defini�lunk egy munkabuffert
		volatile char *ch;
		int i=0; // �s egy l�ptet�regisztert hozz�
		va_list arg; // v�ltoztathat� argumentumlist�val dolgozunk, teh�t listav�ltoz� csn�lunk hozz�
		va_start(arg, format); // kezdj�k az els� v�ltoz�val

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

		while (buffer[i]!='\0') // ha a bufferben t�rolt string v�g�re �r�nk, akkor nem t�bbet a BBXbe tenni
		{
			if (DEBUG_bbx_p==DEBUG_BLACKBOX_BUFFER_SIZE-1) // ha betelt a blackbox, akkor kik�ld mindent
			{
				DEBUG_bbx_p++;
				DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // el�tte az utols� karakterrel lez�rja a stringet
				USART3_puts(DEBUG_bbx_buffer); // fizikai kik�ld�s
				DEBUG_bbx_p=0; // �s resetelj�k a BBXot
			}
			DEBUG_bbx_buffer[DEBUG_bbx_p]=buffer[i]; // egy�bir�nt t�ltj�k tov�bb a buffert
			DEBUG_bbx_p++;
			i++;
		}
		if (!(DEBUG_FLAG & 0x04)) // amikor ki�runk, el�tte megn�zz�k hova megy az �zenet BBXbe, vagy k�zvetlen�l ki
		{
			DEBUG_bbx_buffer[DEBUG_bbx_p]='\0'; // el�tte az utols� karakterrel lez�rja a stringet
			USART3_puts(DEBUG_bbx_buffer); // ha nincs enged�lyezve a BBX akkor azonnali fizikai kik�ld�s
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
		// lehet�leg a kocsi-vissza karaktert ne fogadjuk be, mert linux �s windows kliens alatt nem ugyanaz
		if (DEBUG_bt_pd_p<DEBUG_PROCESS_BUFFER_SIZE) // t�lcsordul-e a process t�mb?
		{
			// ha nem, akkor �rjuk be az �j karakter �s t�r�lj�k a be�rkez� t�mb olvasott elem�t
			if (DEBUG_bt_received[DEBUG_bt_rd_p]!='\n')
			{
				DEBUG_bt_processed[DEBUG_bt_pd_p]=DEBUG_bt_received[DEBUG_bt_rd_p];
				DEBUG_bt_pd_p++;

				// ha \n-nel v�get �r egy parancs azt r�gt�n dolgozzuk is fel
				if (DEBUG_bt_processed[DEBUG_bt_pd_p-1]=='\r')
				{
					DEBUG_bt_processed[DEBUG_bt_pd_p]='\0';
					if (DEBUG_FLAG & 0x02) Debug_Print("cc","ECHO: ",DEBUG_bt_processed);
					DEBUG_bt_pd_p=0; // kezdj�k el�lr�l a process buffert, de most az argumentumokra bont�sra haszn�ljuk
					DEBUG_bt_pa_p=0; // �ll�tsuk be az argmuentumt�rol� v�ltoz� 2 dimenzi�s t�mbj�nek mutat�it
					DEBUG_bt_pap_p=0;
					while (DEBUG_bt_processed[DEBUG_bt_pd_p]!='\r' && !(DEBUG_FLAG & 0x40)) // am�g a parancs v�g�re nem �r�nk
					{
						if (DEBUG_bt_pa_p>DEBUG_MAX_COMMAND_ARGS) // t�l sok argument?
						{
							Debug_Error(3); // argumentum sz�m t�lcsordul�s
							DEBUG_FLAG|=0x40; // FLAGgel jelezz�k hogy sikertelen a parancsfeldolgoz�s, ne k�ldje el az argumentumokat sehova a program
						}

						if (DEBUG_bt_pap_p>DEBUG_MAX_ARG_LENGTH) // t�lcsordul az argumenthossz?
						{
							Debug_Error(4); // argumenthossz t�lcsordul�s
							DEBUG_FLAG|=0x40; // FLAGgel jelezz�k hogy sikertelen a parancsfeldolgoz�s, ne k�ldje el az argumentumokat sehova a program
						}

						if (!(DEBUG_FLAG & 0x40)) // ha m�r hib�s a parancs akkor ne �rjunk semmit sehov�, hiszen t�lcsordul�st �s hardfaultot okozhatunk
						{
							if (DEBUG_bt_processed[DEBUG_bt_pd_p]!=' ') // ha sz�k�z van akkor �j argumentum kell, egy�bir�nt �rjuk tov�bb az aktu�lis argumentumot
							{
								DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]=DEBUG_bt_processed[DEBUG_bt_pd_p];
								DEBUG_bt_pap_p++;
							}
							else // ha sz�k�z van akkor �jabb argumentum k�vetkezik, �s annak kezd�pointere resetel�dik
							{
								DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]='\0'; // el�z� argumentum v�g�t le kell z�rni!
								DEBUG_bt_pap_p=0;
								DEBUG_bt_pa_p++;
							}
						}

						// ha m�s nincs akkor j�n a k�vetkez� processben l�v� karakter
						DEBUG_bt_pd_p++;
					}

					DEBUG_bt_processed_args[DEBUG_bt_pa_p][DEBUG_bt_pap_p]='\0'; // az utols� argumentum lez�r�sa

					// megvannak az argumentumok, ideje feldolgozni �ket!
					if (!(DEBUG_FLAG & 0x40))
					{
						DEBUG_FLAG|=0x80; // kezdetnek �gy vessz�k, hogy ismeretlen parancsot kaptunk
						uint8_t i=0;
						for (i=0;i<DEBUG_COMMAND_NUM+1;i++) // keresni kezd�nk a megl�v� defini�lt parancsok k�z�tt
						{
							if (!strcmp(DEBUG_COMMANDS[i].name,DEBUG_bt_processed_args[0])) // megn�zz�k van e azonos nev� a az els� argumentummal
							{
								DEBUG_COMMANDS[i].func(DEBUG_bt_processed_args); // ha van, akkor futtatjuk a parancsot az �sszes argumentummal
								DEBUG_FLAG&=~0x80; // amennyiben ismert parancs volt, �gy kivessz�k a jelz�flaget
							}
						}
					}

					if (DEBUG_FLAG & 0x80) // ismeretlen parancsot kaptunk?
					{
						Debug_Error(2); // ismeretlen parancs hiba
					}

					DEBUG_bt_pd_p=0; // ha mindennel elk�sz�lt�nk akkor resetelj�k az �sszes pointert
					DEBUG_bt_pa_p=0;
					DEBUG_bt_pap_p=0;
					DEBUG_FLAG&=~0xC0; // parancshiba/ismeretlen parancs eset�n a k�t jelz� flaget resetelj�k
				}
			}

			DEBUG_bt_received[DEBUG_bt_rd_p]='\0';
			DEBUG_bt_rd_p++;
			// ha viszont k�rbe�rt�nk, akkor ugorjunk az elej�re
			if (DEBUG_bt_rd_p>DEBUG_INCOMING_BUFFER_SIZE) DEBUG_bt_rd_p=0;
		}
		else // ha t�lcsordul akkor hiba�zenet, �s pointer vissza az elej�re, elveszett parancsot kaptunk
		{
			DEBUG_bt_pd_p=0;
			Debug_Error(0); // 0-�s hiba�zenet, process buffer t�lcsordult
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
		uint8_t i=0; // keress�k a hiba�zenetet
		for (i=0;i<DEBUG_ERROR_NUM;i++)
		{
			if (DEBUG_ERRORS[i].errornum==ec)
			{
				USART3_puts(DEBUG_ERRORS[i].msg); // ha tal�ltunk akkor azonnal kik�ldj�k fizikai �zenetk�nt
			}
		}
	}
}
