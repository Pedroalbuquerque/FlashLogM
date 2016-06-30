Welcome to the FlashLogM wiki!

#FlashLogM

This library implements a class to make data Log handling easy, providing a set of functionalities that abstract code from the details on reading and writing to a Flash memory, on Moteino boards.

The standard flash memory (on Moteino) is typically a 4Mbit memory (256KB) .

The data to store in the Log can be of any type, and all development was based on a struct data type.

## Base concepts

A Log will be a sequence of data stored in the flash memory;

A pointer is always set to the next available write address and updated on every write;

A pointer is always set to the next read address and updated on every read so that you can do a sequential read of the Log, starting on the first data stored and independently of the write position;

Once memory is full, it will wrap around and will start writing again from position 0. 
Once this happens, a switch is set (memWrap) with no influence on Log usage. 
The wrap around feature does not change the way you use the saveData and readData methods;

So that this wrap around mechanism can work, new Data must start with a signature header of "/*";

Example:  
struct mytype {

	char HD[3] = "/*"; //New data signature header
	uint8_t hour;
	uint8_t minute;
	uint8_t seconds;
	uint16_t miliseconds;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	float groundspeed;
}


## Base functions

saveData(&yourDataVar) - Write data to the next Log position;

readData(&yourDataVar) - Read data from the next Log position;

eraseData() - Clear the flash memory;

eraseNext4K(address) - Erase a 4KByte block starting on address;

initialize - This method will search for the first store data in the Log to set the next read pointer and the last one to set the next write pointer. It will also determine the number of stored Log records;
