/*
 * str_conv.h
 *
 *  Created on: 2013.11.18.
 *      Author: Rimóczi Péter
 */

#ifndef STR_CONV_H_
#define STR_CONV_H_

void strreverse(char* begin, char* end); 	// string reverse
void itoa(int value, char* str, int base);	// integer to ASCII
void ftoa(char *str, float f, char size);	// float to ascii

#endif /* STR_CONV_H_ */
