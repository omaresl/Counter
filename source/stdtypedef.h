/*
 * stdtypedef.h
 *
 *  Created on: 07/02/2018
 *      Author: uidj2522
 */

#ifndef STDTYPEDEF_H_
#define STDTYPEDEF_H_

#define FALSE	0x00u
#define TRUE	0x01u

typedef unsigned char T_UBYTE;

typedef union
{
	struct
	{
		T_UBYTE	bit0	:1;
		T_UBYTE	bit1	:1;
		T_UBYTE	bit2	:1;
		T_UBYTE	bit3	:1;
		T_UBYTE	bit4	:1;
		T_UBYTE	bit5	:1;
		T_UBYTE	bit6	:1;
		T_UBYTE	bit7	:1;
	}bits;
	T_UBYTE byte;

}T_FIELD_8;

#endif /* STDTYPEDEF_H_ */
