#include <arduino.h>
#include <FlashLogM.h>
#include <RH_RF95.h>  //get it here http://lowpowerlab.com/RadioHead_LowPowerLab.zip


/*
 * 4Mb = 512KB
 * 1 page = 255Bytes
 *
 * 512KB = 2048 pages
 *
 * Byte (8bit) = 0 - 255
 * int (16bit) = 0 - 32767
 * u_int (16bit) = 0 - 65535 (64K)
 * long (32bit) = 0 - 4Giga
 */


const long ADDR_NUMREC=1;


FlashLogM  mylog;

 typedef struct  {
	char HEADER[3] = "/*";
	char apelido[10];
	char nome[10];
  uint8_t idade;
  uint16_t altura;
  float peso;
  double velocidade;
}Data;

Data data, Rdata;  // declare data variable of type Data(struct)



char str[100];
unsigned long i;

void setup() {

	long timer1 = millis();
  // initialize USB Serial for Debug messages
  Serial.begin(115200);//Initialize the Serial port at the specified baud rate
  delay(10);//Wait for the Serial port to settle down


  //numLogs();
  Serial.print("Initializing......");
  mylog.initialize(data);
  Serial.print("done!  took :"); Serial.println((millis() - timer1) / 1000);
  delay(2000);


  //  USB serial port  Splash Start message  for debug
  Serial.println("Flash Test");

  Serial.print("sizeof data:"); Serial.println(sizeof(data));

  Serial.println("0 - W & R Byte(0)");
  Serial.println("S - status");
  Serial.println("E - Erase Block");
  Serial.println("e - Erase all");
  Serial.println("d - dump(0)");
  Serial.println("D - dump(10000)");
  Serial.println("n - Erase 4K from writepoint");
  Serial.println("M - Erase 4K from writepoint-600");
  Serial.println("w - Write 1 struct from Write Point");
  Serial.println("W - Fill flash with struct from 0");
  Serial.println("R - read all available data");
  Serial.println("t - Read Byte(0-1)");


  }


void loop() {

	int c;
	int num;
	static unsigned long nextBlockErase = 1;

	//num = Serial.available();
	//Serial.print("num:"); Serial.println(num);

	 if (Serial.available() > 0)
	{
		c = Serial.read();
		Serial.print("c:"); Serial.println(c);
		switch( c)
		{
      case 't': //status


        Serial.print(" 0:"); Serial.println(flash.readByte(0),HEX);
        Serial.print(" 1:"); Serial.println(flash.readByte(1),HEX);


        Serial.println(" ttt          Read              ttt");

        break;
			case '0': //status

				flash.writeByte(0, 125);

				Serial.print(" 0:"); Serial.println(flash.readByte(0));


				Serial.println(" 000          Write/Read              000");

				break;
			case 'S': //status
					Serial.print("NextWrite:"); Serial.println(mylog.nextWrite);
					Serial.print("recSize:"); Serial.println(mylog.recSize);
					Serial.print("NextRead:"); Serial.println(mylog.nextRead);
					Serial.print("memWrap:"); Serial.println(mylog.memWrap);
					Serial.print("nextBlockErase:"); Serial.println(nextBlockErase);

					Serial.println(" SSS          Status              SSS");

					break;
			case 'E': //erase Block
				flash.blockErase4K(nextBlockErase);
				nextBlockErase += BLOCKSIZE;

				Serial.println(" EEE          erase Block 4K               EEE");

				break;
			case 'e': //erase all
      				mylog.eraseData();
					Serial.println(" eee          erase ALL               eee");

					break;
			case 'd': // dump till 256 found
					  //mylog.eraseData();
  				dump(0);

				break;
			case 'D': // dump till 256 found
				//mylog.eraseData();
				dump(10000);

			break;
			case 'n': // erase next 4K
				mylog.eraseNext4K(mylog.nextWrite);
				Serial.println(" nnn          erase                nnn");
					break;
			case 'm': // erase next 4K
				mylog.eraseNext4K(mylog.nextWrite-600);
				Serial.println(" mmm          erase -10               mmm");
					break;
			case 'w': // write a data struct to next Write addr

        uint16_t nR ;
        nR = mylog.nextWrite;
        char strtmp[100];

        sprintf(strtmp, "%p",data);

        Serial.print("data adrr:");Serial.println((unsigned int)&data.HEADER, HEX);
        sprintf(strtmp, "%p",Rdata);
        Serial.print("Rdata adrr:");Serial.println((unsigned int)&Rdata.HEADER, HEX);

				Serial.print("NextWrite:"); Serial.println(mylog.nextWrite);
				Serial.print("recSize:"); Serial.println(mylog.recSize);

				sprintf(data.nome, "João");
				sprintf(data.apelido, "Afonso");
        data.idade = 10;
        data.altura = 1820;
        data.peso = 67.8;
        data.velocidade = 5.4320;
        Serial.print("Nome:");Serial.println(data.nome);
        Serial.print("Apelido:");Serial.println(data.apelido);
        Serial.print("Idade:");Serial.println(data.idade);
        Serial.print("altura:");Serial.println(data.altura);
        Serial.print("peso:");Serial.println(data.peso);
        Serial.print("Velocidade:");Serial.println(data.velocidade);

				mylog.saveData(data);

        mylog.nextRead = nR;
        Serial.print("\nnextRead:");Serial.println(mylog.nextRead);

        mylog.readData(Rdata);
        Serial.print("Nome:");Serial.println(Rdata.nome);
        Serial.print("Apelido:");Serial.println(Rdata.apelido);
        Serial.print("Idade:");Serial.println(Rdata.idade);
        Serial.print("altura:");Serial.println(Rdata.altura);
        Serial.print("peso:");Serial.println(Rdata.peso);
        Serial.print("Velocidade:");Serial.println(Rdata.velocidade);

				Serial.println(" WWWW          save One               WWWW");
				break;

			case 'W': // fill memory with data
				sprintf(data.nome, "Pedro");
				sprintf(data.apelido, "Miguel");
        data.idade = 10;
        data.altura = 1820;
        data.peso = 67.8;
        data.velocidade = 5.4320;

				for (i = 1; i < FLASH_MAXADR / mylog.recSize; i++)
				{
					if (i % 10 == 0)
					{
						Serial.print("saving @:"); Serial.println(mylog.nextWrite);
					}
					mylog.saveData(data);
				}
				Serial.println(" WWW          Fill mem with data               WWW");

				break;
			case 'R': // read all available data from memory

				for (i = 1; i < FLASH_MAXADR / mylog.recSize + 10 ; i++)
				{
					Serial.print("i:"); Serial.print(i);
					Serial.print("\t addr:"); Serial.print(mylog.nextRead);
					mylog.readData(data);
					Serial.print("\t Header:"); Serial.print(data.HEADER);
					Serial.print("\t nome:"); Serial.print(data.nome);
					Serial.print("\t apelido:"); Serial.println(data.apelido);
          Serial.print("\tIdade:");Serial.println(data.idade);
          Serial.print("\taltura:");Serial.println(data.altura);
          Serial.print("\tpeso:");Serial.println(data.peso);
          Serial.print("\tVelocidade:");Serial.println(data.velocidade);
				}

				Serial.println(" RRR          Read all               RRR");

				break;

			default:
				Serial.print("other :"); Serial.println(c);
				break;

		}
   delay(1000);
	}


}



//  ********       Funnctions *****************


void dump(int len)
{
	byte c;

	if (len > 0)
	{
		for (i = 0; i < len + 5; i++)
		{
			c = flash.readByte(i);
			//sprintf(str, "adr:%d val:%d \n", i, c);
			//Serial.print(str);
			Serial.print("addr:"); Serial.print(i); Serial.print(" val:"); Serial.println(c);
		}
	}
	else
	{
		i = 0;
		do
		{
			c = flash.readByte(i);
			//sprintf(str, "adr:%d val:%d \n", i, c);
			//Serial.print(str);
			Serial.print("addr:"); Serial.print(i); Serial.print(" val:"); Serial.println(c);
			i++;
		}
		while (c  != 255);
	}
	return ;
}


/*

unsigned long numLogs()
{
	unsigned long i = 1;
	byte c = 0;
	mylog.nextRead = 1;
	do
	{
		mylog.readData(c);
		Serial.println(c);
		i++;
	} while (c != 255 && i < 262143);
	Serial.print("numLog:"); Serial.println(i / 30);
	return i / 30;
};
*/
