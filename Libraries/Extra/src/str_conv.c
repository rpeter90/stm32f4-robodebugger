/*
 * str_conv.c
 *
 *  Created on: 2013.11.18.
 *      Author: Rimóczi Péter
 *
 *      Basic conversions to ASCII characters
 */

#include <string.h>

// str reversing
void strreverse(char* begin, char* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}

// int to str
void itoa(int value, char* str, int base) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;
	if (base<2 || base>35){ *wstr='\0'; return; }
	if ((sign=value) < 0) value = -value;
	do *wstr++ = num[value%base]; while(value/=base);
	if(sign<0) *wstr++='-';
	*wstr='\0';
	strreverse(str,wstr-1);
}

// float to str
void ftoa(char *str, float f, char size)
{
       char pos;  // position in string
       char len;  // length of decimal part of result
       char* curr;  // temp holder for next digit
       int value;  // decimal digit(s) to convert
       pos = 0;  // initialize pos, just to be sure

       value = (int)f;  // truncate the floating point number
       itoa(value,str,10);  // this is kinda dangerous depending on the length of str
       // now str array has the digits before the decimal

       if (f < 0 )  // handle negative numbers
       {
               f *= -1;
               value *= -1;
       }

    len = strlen(str);  // find out how big the integer part was
       pos = len;  // position the pointer to the end of the integer part
       str[pos++] = '.';  // add decimal point to string

       while(pos < (size + len + 1) )  // process remaining digits
       {
               f = f - (float)value;  // hack off the whole part of the number
               f *= 10;  // move next digit over
               value = (int)f;  // get next digit
               itoa(value,curr,10); // convert digit to string
               str[pos++] = *curr; // add digit to result string and increment pointer
       }
}


