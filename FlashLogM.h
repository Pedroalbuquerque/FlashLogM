/* Arduino FlashLog Library
* Copyright (C) 2016 by Pedro Albuquerque
*
*
* This file is part of the Arduino PATinySPI Library. This library is for
* Winbond NOR flash memory modules. In its current form it enables reading
* and writing individual data variables, structs and arrays from and to various locations;
* reading and writing pages; continuous read functions; sector, block and chip erase;
* suspending and resuming programming/erase and powering down for low power operation.
*
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This Library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License v3.0
* along with the Arduino PATinySPI Library.  If not, see
* <http://www.gnu.org/licenses/>.
*/
//      #### Flash memory ####

/////////////////////////////////////////////////////////////////////////////
// flash(SPI_CS, MANUFACTURER_ID)
// SPI_CS          - CS pin attached to SPI flash chip (8 in case of Moteino)
// MANUFACTURER_ID - OPTIONAL, 0x1F44 for adesto(ex atmel) 4mbit flash
//                             0xEF30 for windbond 4mbit flash
//                             0xEF40 for windbond 16/64mbit flash
/////////////////////////////////////////////////////////////////////////////

/*
 *	FlashLog V2.1	update 2016.05.19 by Pedro Albuquerque
 *
 *	1- Adjusted definition to allow MEGA boards that use diferent pin for LED and FLASH
 *
 */


/*
 *	FlashLog V2.0	update 2016.02.28 by Pedro Albuquerque
*
*	1- changed write to allow circular writing
*	2- change initialize function to find nextWrite and nextRead on circular writing
*	NOTE:  Structure data must start by a 2 character string "/*" Header to allow for struture star detection by the initialize function
*
*/

/*
*	FlashLog V1.1	update 2016.02.11 by Pedro Albuquerque
	*
	*	1- introduced initialize(anyVar)	to determine how many records are in memory assuming ther are of type anyVar
	*		anyVar can be any data type including a structure;
	*	2- initialize will set numRecords to the number of records saved,
			nextWrite to the fisrt available adress to write to
			and nextRead to 1
	*
*/

#ifndef FLASHLOGM_H
#define FLASHLOGM_H


#include <SPIFlash.h>


// initiate flash memory
#ifdef __AVR_ATmega1284P__
    #define FLASH_SS      23 // and FLASH SS on D23
#else
    #define FLASH_SS      8 // and FLASH SS on D8
#endif

// SPI flash = 4Mb = 512KB = 524288 B
#define FLASH_MAXADR  524287L
#define FLASH_SIZE 524288L  //(FLASH_MAXADR + 1)
#define BLOCKSIZE 4096


SPIFlash flash(FLASH_SS, 0xEF30); //EF40 for 16mbit windbond chip


class FlashLogM {
public:
	unsigned long nextWrite, nextRead, numRecords, recSize, maxRecords;
	boolean memWrap; // log has wrapped back to beginning

	template <class T> byte saveData(const T& datast);
	template <class T> byte readData(T& datast);
	void eraseData();
	void eraseNext4K(unsigned long addr);
	template < class T> void initialize( T& datast);
	//void status(unsigned long i, unsigned long numEmp);

};

template <class T> byte FlashLogM::saveData( const T& datast)
{
	const byte* _pByte ;  // byte pointer to save one byte at a time
	unsigned long _numBytes ; // how many byte the str has
	unsigned long _address;  //mem address to write to
	unsigned long _eraseAddr;

	_numBytes = recSize;
	_pByte = (const  byte *)( const void *) &datast;
	_address = nextWrite;


	while (_numBytes > 0)
	{

		flash.writeByte( _address , *_pByte);

		if (_address % BLOCKSIZE == 0) // if writig on first byte of a block erase 2 block ahead
		{
			_eraseAddr = ((_address + BLOCKSIZE) % FLASH_SIZE) ;
			eraseNext4K(_eraseAddr);
		}

		_numBytes--;
		_pByte++;
		_address++;

		if (_address > FLASH_MAXADR) // if address to write exceed flash size start on 1
		{
			_address = 0;
		}


	}
	nextWrite =  _address;
	if (numRecords < maxRecords)
	{
		numRecords++; 			// increment number of records saved
	}


	return 1;

}
template <class T> byte FlashLogM::readData(T& datast)
{
	byte *p = (byte* )(void*)&datast;
	for (unsigned int  i = 0; i < recSize;i++)
	{
		*p = flash.readByte((unsigned long)p);

		if (p >(void *) FLASH_MAXADR) p = 0; // if end of memory reached start from beginning

	}

	nextRead = (nextRead + recSize) % FLASH_SIZE;
	return 1;
}

void FlashLogM::eraseNext4K(unsigned long addr)
{
	if (flash.readByte(addr) != 255 || flash.readByte(addr + BLOCKSIZE - 1) != 255)
	{
		flash.blockErase4K(addr);
		Serial.print("erased:"); Serial.println(addr);
	}

	if (memWrap)
	{
		unsigned long j = addr + BLOCKSIZE;
		Serial.print("looking for str start:"); Serial.println(j);
		while (flash.readByte(j) != '/' || flash.readByte(j + 1) != '*') //look for start of structure
		{
			j++;
			if (j > FLASH_MAXADR) j = 0; // if end reached start from beginning

		}
		nextRead = j;

	}
	return ;
}
void FlashLogM::eraseData()
{

	flash.chipErase();
	nextRead = 0;
	nextWrite = 0;
	numRecords = 0;
  memWrap = false;

	return ;
}

template < class T> void FlashLogM::initialize(T& datast)
{
	byte val;
	unsigned long  i,j, numEmpty =0;
	char str[200];
	boolean SignturaFound = false;
	i = FLASH_MAXADR;
	recSize = sizeof(datast);
	maxRecords = (long)((FLASH_MAXADR - 2*BLOCKSIZE)  / recSize );
	memWrap = false;
	nextWrite = 0;
	nextRead = 0;


	// count last free space if any
	if ((flash.readByte(i) == 255 && flash.readByte(i - BLOCKSIZE + 1) != 255))
	{

		while(flash.readByte(i) == 255)
		{
			numEmpty++;
			i--;
		}
		memWrap = true;
		nextWrite = i + 1;
		i = FLASH_MAXADR - BLOCKSIZE;
	}


	// find the end of first block empty
	while ((flash.readByte(i) != 255 || flash.readByte(i-BLOCKSIZE+1) != 255) && i >= BLOCKSIZE )
	{
		i-= BLOCKSIZE;
	} ;

	// find start of struture
	if (i != FLASH_MAXADR)
	{
		memWrap = true;

		// search start of a structure "/*"to set nextRead
		j = i;
		while (flash.readByte(j) != '/' || flash.readByte(j+1) !='*' ) //look for start of structure
		{
			j++;
			if (j > FLASH_MAXADR) j = 0; // if end reached start from beginning

		}
		nextRead = j ;
	}

	// find start of first empty (225) area
	while (flash.readByte(i) == 255 && i > 0)
	{
		numEmpty ++;
		i--;
	} ;

	if (i == 0 && flash.readByte(i)== 255)
	{
		if( ! memWrap) nextWrite = 0;
		numEmpty++;
	}
	else
		nextWrite = i+1;


	numRecords = (FLASH_SIZE-numEmpty) / recSize;

}
/*
void FlashLogM::status(unsigned long i, unsigned long numEmp)
{

	Serial.print("i:"); Serial.println(i);
	Serial.print("NextWrite:"); Serial.println(nextWrite);
	Serial.print("NextRead:"); Serial.println(nextRead);
	Serial.print("memWrap:"); Serial.println(memWrap);
	Serial.print("numEmpty:"); Serial.println(numEmp);
	Serial.print("numRecords:"); Serial.println(numRecords);

}
*/
#endif
