Welcome to the FlashLogM wiki!

#FlashLogM

This library implement a class to easy a log handling, providing a set of functionality that abstract code from the details on reading and writing to a Flash memory as provided by Moteino boards.

The flash memory (on Moteino) are typically a 4Mbit memory ( 256KB) .

The info o store in the log can be of any type, and all development was based on a struct data type.

## Base concepts

Log will be a sequence of data stored in the flash memory.

A pointer is always set to the next available write address and updated on every write

A pointer is always set to the next read address and updated on every read so you can do a sequential read of the log starting on first data stored and independent from write position.

Once memory is full, it will wrap, meaning it will start writing again from position 0. once this happens a switch is set (memWrap) with no influence on log use. your code should keep using saveData and readData methods the same way.

So that this wrap mechanism can work, the Data must start with a signature "/*"

Example:  
struct mytype {

	char HD[3] = "/*";

	uint8_t hour;

	uint8_t minute;

	uint8_t seconds;

	uint16_t miliseconds;

	uint8_t day;

	uint8_t month;

	uint8_t year;

	float groundspeed; // In knots

}


## Base functions

saveData(&yourDataVar) - write data to the next log position

readData(&yourDataVar) - Read data from the next log position

eraseData() - clear the flash memory

eraseNext4K(address) - erase a 4KByte block starting on address

initialize - this method will search for first Data to set the next read Pinter and the last one to set the next write pointer. It will also determine the number of Log records stored.
