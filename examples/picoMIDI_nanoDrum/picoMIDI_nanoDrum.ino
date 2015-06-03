/*
  USB-MIDI nanoDrum
  For picoMIDI (ATTiny4313)
*/

// ATTiny4313 Pins
//                  +-------+
//             RESET|1    20|VCC
//               RXD|2    19|PB7/SCK
//               TXD|3    18|PB6/MISO
//             XTAL2|4    17|PB5/MOSI
//             XTAL1|5    16|PB4  (PIN 13)
// PD2/INT0 (USB D+)|6    15|PB3  (PIN 12)
// PD3/INT1 (USB D-)|7    14|PB2  (PIN 11)
// RESET (PIN 6) PD4|8    13|PB1  (PIN 10)
//       (PIN 7) PD5|9    12|PB0  (PIN 9)
//               GND|10   11|PD6  (PIN 8)
//                  +-------+

#include "picoMIDI.h"

//RESET PIN 6
#define RESET_INIT() (DDRD |= _BV(4))
#define RESET_HIGH() (PORTD |= _BV(4))
#define RESET_LOW() (PORTD &= ~_BV(4))

// LED PIN 9
#define LED_INIT() (DDRB |= _BV(0))
#define LED_HIGH() (PORTB |= _BV(0))
#define LED_LOW() (PORTB &= ~_BV(0))
#define LED_FLIP() ( PORTB ^= _BV(0))

bool stk500_enabled=false;

void onSerialNoteOn(byte ch, byte note, byte vel){
	UsbMidi.sendNoteOn(ch,note,vel);
}

void onSerialSimpleSysex(byte cmd, byte data1, byte data2, byte data3){
    UsbMidi.sendSimpleSysex(cmd,data1,data2, data3);
}

void onUsbSimpleSysex(byte cmd, byte data1, byte data2, byte data3)
{
  if(cmd==0x50) //INIT
  {
    //Stop serial_midi
    stk500_enabled=true;
			
    //Start STK500
    stk500.init();
	
    RESET_HIGH();
	
    delayMicroseconds(100000);
    RESET_HIGH();
    delayMicroseconds(1000);
    RESET_LOW();
    delayMicroseconds(10000);

    return;
  }

	if(stk500_enabled)
	{ 
		if(cmd==0x51) //GETSYNC
		{
			stk500.getsync();
		}
		else if(cmd==0x52) //GETPARAM
		{
			byte param=data2 | ((data1 << 1) & 0x80);
			stk500.getparam(param);
		}
		else if(cmd==0x53) //ENTER_PROGMODE
		{
			//Not used - optiboot.c line 496
			//stk500.enter_progmode();
		}
		else if(cmd==0x54) //LOAD_ADDRESS
		{
			byte haddr=data2 | ((data1 << 1) & 0x80);
			byte lhaddr=data3 | ((data1 << 2) & 0x80);
		
    		stk500.loadaddr(haddr,lhaddr);
		}
		else if(cmd==0x55) //PROG_PAGE
		{
			byte state=data1 & 0x03; //0=start,1=fill,2=end
			//byte memtype=(data1 & 0x04)==0x00 ? 'E':'F'; // “E” – EEPROM, “F” – FLASH
		
			byte dataA=data2 | ((data1 << 1) & 0x80);
			byte dataB=data3 | ((data1 << 2) & 0x80);
		
    		stk500.prog_page(state,dataA, dataB);
		}
		else if(cmd==0x56) //LEAVE_PROGMODE
		{
			stk500.leave_progmode();

  			delayMicroseconds(10000);
  			RESET_HIGH();
  			delayMicroseconds(1000);
  			RESET_LOW();
		
			stk500.flush();
		
			SerialMidi.init();
		
			stk500_enabled=false;
		}
		else if(cmd==0x57) //READ_PAGE 
		{
			//byte length=data1 & 0x03; //1 ,2 o 3
			//byte memtype=(data1 & 0x04)==0x00 ? 'E':'F'; // “E” – EEPROM, “F” – FLASH
		
			stk500.read_page(2);
		}
		else if(cmd==0x58) //READ_SIGN
		{
			stk500.read_sign();
		}
	}
    else SerialMidi.sendSimpleSysex(cmd,data1,data2, data3);
}

void onSTK500Resp(byte data1,byte data2, byte data3,byte data4, byte data5)
{
	if(data1==Resp_STK_INSYNC)
	{
		UsbMidi.sendSimpleSysex(0x59,((data2 >> 1) & 0x40) + ((data3 >> 2) & 0x20),data2,data3);
		UsbMidi.sendSimpleSysex(0x5a,((data4 >> 1) & 0x40) + ((data5 >> 2) & 0x20),data4,data5);
	}
	else UsbMidi.sendSimpleSysex(0x5b,data1,data1>>7,data2);
}

void setup() {                

	RESET_INIT();

  UsbMidi.init();
  UsbMidi.setHdlSimpleSysex(onUsbSimpleSysex);
  
  SerialMidi.init();
  SerialMidi.setHdlSimpleSysex(onSerialSimpleSysex);
  SerialMidi.setHdlNoteOn(onSerialNoteOn);
  
  stk500.setHdlResponse(onSTK500Resp);
}

void loop() {
  	UsbMidi.update();
	if(stk500_enabled)
		stk500.update();
	else SerialMidi.update();
}

/*void delayMs(unsigned int ms) {
  for( int i=0; i<ms; i++ ) {
    delayMicroseconds(1000);
  }
}*/

