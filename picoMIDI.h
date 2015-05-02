/*

based : 

- V-USB by Objective Development Software GmbH
	http://www.obdev.at/products/vusb/index.html

- vusb-for-arduino 
  https://code.google.com/p/vusb-for-arduino/downloads/list

- Attiny core for the Arduino IDE v.1.5.8
  http://www.leonardomiliani.com/en/2014/aggiornato-il-core-attiny-per-lide-1-5-8-di-arduino/
  
- Atmel Attiny45 / Attiny85 based USB MIDI controller by Thorsten
  http://electronicsodyssey.blogspot.jp/2011/10/atmel-attiny45-attiny85-based-usb-midi.html

- Arduino vusb MIDI Attiny45
  https://github.com/tadfmac/arduino-vusb-midi-attiny45
  
- MICO-MOCO USB-MIDI Converter
  http://morecatlab.akiba.coocan.jp/morecat_lab/MOCO-e.html
  
Licenses:

	Under the GNU License.
	See License.txt.

*/

#ifndef __picoMIDI_h__
#define __picoMIDI_h__

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "usbdrv.h"
#include "oddebug.h"
 
typedef uint8_t byte;

//Disable HardwareSerial
#define HardwareSerial_h

#define	TRUE			1
#define	FALSE			0

enum {
    SEND_ENCAPSULATED_COMMAND = 0,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK
};

// This deviceDescrMIDI[] is based on 
// http://www.usb.org/developers/devclass_docs/midi10.pdf
// Appendix B. Example: Simple MIDI Adapter (Informative)

// B.1 Device Descriptor
const static PROGMEM char deviceDescrMIDI[] = {	/* USB device descriptor */
  18,			/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
  USBDESCR_DEVICE,	/* descriptor type */
  0x10, 0x01,		/* USB version supported */
  0,			/* device class: defined at interface level */
  0,			/* subclass */
  0,			/* protocol */
  8,			/* max packet size */
  USB_CFG_VENDOR_ID,	/* 2 bytes */
  USB_CFG_DEVICE_ID,	/* 2 bytes */
  USB_CFG_DEVICE_VERSION,	/* 2 bytes */
  1,			/* manufacturer string index */
  2,			/* product string index */
  0,			/* serial number string index */
  1,			/* number of configurations */
};

// B.2 Configuration Descriptor
const static PROGMEM char configDescrMIDI[] = { /* USB configuration descriptor */
  9,	   /* sizeof(usbDescrConfig): length of descriptor in bytes */
  USBDESCR_CONFIG,		/* descriptor type */
  101, 0,			/* total length of data returned (including inlined descriptors) */
  2,		      /* number of interfaces in this configuration */
  1,				/* index of this configuration */
  0,				/* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    (1 << 7) | USBATTR_SELFPOWER,       /* attributes */
#else
    (1 << 7) |USBATTR_BUSPOWER,         /* attributes */
#endif
  USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */

// B.3 AudioControl Interface Descriptors
// The AudioControl interface describes the device structure (audio function topology) 
// and is used to manipulate the Audio Controls. This device has no audio function 
// incorporated. However, the AudioControl interface is mandatory and therefore both 
// the standard AC interface descriptor and the classspecific AC interface descriptor 
// must be present. The class-specific AC interface descriptor only contains the header 
// descriptor.

// B.3.1 Standard AC Interface Descriptor
// The AudioControl interface has no dedicated endpoints associated with it. It uses the 
// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl 
// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
  /* descriptor follows inline: */
  9,			/* sizeof(usbDescrInterface): length of descriptor in bytes */
  USBDESCR_INTERFACE,	/* descriptor type */
  0,			/* index of this interface */
  0,			/* alternate setting for this interface */
  0,			/* endpoints excl 0: number of endpoint descriptors to follow */
  1,			/* */
  1,			/* */
  0,			/* */
  0,			/* string index for interface */

// B.3.2 Class-specific AC Interface Descriptor
// The Class-specific AC interface descriptor is always headed by a Header descriptor 
// that contains general information about the AudioControl interface. It contains all 
// the pointers needed to describe the Audio Interface Collection, associated with the 
// described audio function. Only the Header descriptor is present in this device 
// because it does not contain any audio functionality as such.
  /* descriptor follows inline: */
  9,			/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
  36,			/* descriptor type */
  1,			/* header functional descriptor */
  0x0, 0x01,		/* bcdADC */
  9, 0,			/* wTotalLength */
  1,			/* */
  1,			/* */

// B.4 MIDIStreaming Interface Descriptors

// B.4.1 Standard MS Interface Descriptor
  /* descriptor follows inline: */
  9,			/* length of descriptor in bytes */
  USBDESCR_INTERFACE,	/* descriptor type */
  1,			/* index of this interface */
  0,			/* alternate setting for this interface */
  2,			/* endpoints excl 0: number of endpoint descriptors to follow */
  1,			/* AUDIO */
  3,			/* MS */
  0,			/* unused */
  0,			/* string index for interface */

// B.4.2 Class-specific MS Interface Descriptor
  /* descriptor follows inline: */
  7,			/* length of descriptor in bytes */
  36,			/* descriptor type */
  1,			/* header functional descriptor */
  0x0, 0x01,		/* bcdADC */
  65, 0,			/* wTotalLength */

// B.4.3 MIDI IN Jack Descriptor
  /* descriptor follows inline: */
  6,			/* bLength */
  36,			/* descriptor type */
  2,			/* MIDI_IN_JACK desc subtype */
  1,			/* EMBEDDED bJackType */
  1,			/* bJackID */
  0,			/* iJack */

  /* descriptor follows inline: */
  6,			/* bLength */
  36,			/* descriptor type */
  2,			/* MIDI_IN_JACK desc subtype */
  2,			/* EXTERNAL bJackType */
  2,			/* bJackID */
  0,			/* iJack */

//B.4.4 MIDI OUT Jack Descriptor
  /* descriptor follows inline: */
  9,			/* length of descriptor in bytes */
  36,			/* descriptor type */
  3,			/* MIDI_OUT_JACK descriptor */
  1,			/* EMBEDDED bJackType */
  3,			/* bJackID */
  1,			/* No of input pins */
  2,			/* BaSourceID */
  1,			/* BaSourcePin */
  0,			/* iJack */

  /* descriptor follows inline: */
  9,			/* bLength of descriptor in bytes */
  36,			/* bDescriptorType */
  3,			/* MIDI_OUT_JACK bDescriptorSubtype */
  2,			/* EXTERNAL bJackType */
  4,			/* bJackID */
  1,			/* bNrInputPins */
  1,			/* baSourceID (0) */
  1,			/* baSourcePin (0) */
  0,			/* iJack */


// B.5 Bulk OUT Endpoint Descriptors

//B.5.1 Standard Bulk OUT Endpoint Descriptor
  /* descriptor follows inline: */
  9,			/* bLenght */
  USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
  0x1,			/* bEndpointAddress OUT endpoint number 1 */
  3,			/* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
  8, 0,			/* wMaxPacketSize */
  10,			/* bIntervall in ms */
  0,			/* bRefresh */
  0,			/* bSyncAddress */

// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
  /* descriptor follows inline: */
  5,			/* bLength of descriptor in bytes */
  37,			/* bDescriptorType */
  1,			/* bDescriptorSubtype */
  1,			/* bNumEmbMIDIJack  */
  1,			/* baAssocJackID (0) */


//B.6 Bulk IN Endpoint Descriptors

//B.6.1 Standard Bulk IN Endpoint Descriptor
  /* descriptor follows inline: */
  9,			/* bLenght */
  USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
  0x81,			/* bEndpointAddress IN endpoint number 1 */
  3,			/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
  8, 0,			/* wMaxPacketSize */
  10,			/* bIntervall in ms */
  0,			/* bRefresh */
  0,			/* bSyncAddress */

// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
  /* descriptor follows inline: */
  5,			/* bLength of descriptor in bytes */
  37,			/* bDescriptorType */
  1,			/* bDescriptorSubtype */
  1,			/* bNumEmbMIDIJack (0) */
  3,			/* baAssocJackID (0) */
};


uchar usbFunctionDescriptor(usbRequest_t * rq) {
  if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
    usbMsgPtr = (uchar *) deviceDescrMIDI;
    return sizeof(deviceDescrMIDI);
  } else {		/* must be config descriptor */
    usbMsgPtr = (uchar *) configDescrMIDI;
    return sizeof(configDescrMIDI);
  }
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

static uchar    sendEmptyFrame;
// static uchar    modeBuffer[7];

uchar usbFunctionSetup(uchar data[8]) 
{
	usbRequest_t    *rq = (usbRequest_t *)((void *)data);

  if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
    
    /*  Prepare bulk-in endpoint to respond to early termination   */
    if ((rq->bmRequestType & USBRQ_DIR_MASK) ==
	USBRQ_DIR_HOST_TO_DEVICE)
      sendEmptyFrame = 1;
  }
  return 0xff;
}

/*---------------------------------------------------------------------------*/
/* usbFunctionRead                                                           */
/*---------------------------------------------------------------------------*/

uchar usbFunctionRead( uchar *data, uchar len ) {
	/*data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	data[6] = 0;*/
    return 7;
}

/*---------------------------------------------------------------------------*/
/* usbFunctionWrite                                                          */
/*---------------------------------------------------------------------------*/

uchar usbFunctionWrite(uchar * data, uchar len) {
  return 1;
}
/*---------------------------------------------------------------------------*/
/* usbFunctionWriteOut                                                       */
/*                                                                           */
/* this Function is called if a MIDI Out message (from PC) arrives.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/

#define NOTEOFF_KIND 		1
#define NOTEON_KIND 		2
#define CTLCHANGE_KIND 		3
#define SIMPLESYSEX_KIND 	4

void (*cbUsbNoteOff)(byte ch, byte note, byte vel);
void (*cbUsbNoteOn)(byte ch, byte note, byte vel);
void (*cbUsbCtlChange)(byte ch, byte num, byte value);
void (*cbUsbSimpleSysex)(byte cmd, byte data1, byte data2, byte data3);

void (*cbSerialNoteOff)(byte ch, byte note, byte vel);
void (*cbSerialNoteOn)(byte ch, byte note, byte vel);
void (*cbSerialSimpleSysex)(byte cmd, byte data1, byte data2, byte data3);

byte checkMidiMessage(byte *pMidi){
  if(((*(pMidi + 1) & 0xf0)== 0x90)&&(*(pMidi + 3) != 0)){
    return NOTEON_KIND;
  }else if(((*(pMidi + 1) & 0xf0)== 0x90)&&(*(pMidi + 3) == 0)){
    return NOTEOFF_KIND;
  }else if((*(pMidi + 1) & 0xf0)== 0x80){
    return NOTEOFF_KIND;
  }else if((*(pMidi + 1) & 0xf0)== 0xb0){
    return CTLCHANGE_KIND;
  }else if((*(pMidi + 1))== 0xf0){
    return SIMPLESYSEX_KIND;
  }else{
    return 0;
  }
}

void usbFunctionWriteOut(uchar *data, uchar len ) {

	byte cnt;
	byte kindmessage;
	byte interrupt;
	byte *pMidi;

	//cli();

	interrupt = len / 4;
	for(cnt = 0;cnt < interrupt;cnt++){
		pMidi = data + (cnt * 4);
		kindmessage = checkMidiMessage(pMidi);
		if(kindmessage == NOTEOFF_KIND){
			if(cbUsbNoteOff != NULL){
				(*cbUsbNoteOff)(*(pMidi+1)&0x0f,*(pMidi+2)&0x7f,*(pMidi+3)&0x7f);
			}
		}else if(kindmessage == NOTEON_KIND){
			if(cbUsbNoteOn != NULL){
				(*cbUsbNoteOn)(*(pMidi+1)&0x0f,*(pMidi+2)&0x7f,*(pMidi+3)&0x7f);
			}
		}else if(kindmessage == CTLCHANGE_KIND){
			if(cbUsbCtlChange != NULL){
				(*cbUsbCtlChange)(*(pMidi+1)&0x0f,*(pMidi+2)&0x7f,*(pMidi+3)&0x7f);
			}
		}else if(kindmessage == SIMPLESYSEX_KIND){
			if(cnt+1<interrupt)
			{
				cnt=cnt+1;
				if(cbUsbSimpleSysex != NULL){
					(*cbUsbSimpleSysex)(*(pMidi+3)&0x7f,*(pMidi+5)&0x7f,*(pMidi+6)&0x7f,*(pMidi+7)&0x7f);
				}
			}
		}
	}
	
	//sei();
}

//============================================================

#define PORTD_DDR 0x00

//============================================================
// HARDWARE_INIT
//============================================================

static void hardwareInit(void) {
  uchar i, j;

  /* activate pull-ups except on USB lines */
  USB_CFG_IOPORT =
    (uchar) ~ ((1 << USB_CFG_DMINUS_BIT) |
	       (1 << USB_CFG_DPLUS_BIT) | PORTD_DDR) ;
  /* all pins input except USB (-> USB reset) */
#ifdef USB_CFG_PULLUP_IOPORT
  /* use usbDeviceConnect()/usbDeviceDisconnect() if available */
  USBDDR = 0 | PORTD_DDR;	/* we do RESET by deactivating pullup */
  usbDeviceDisconnect();
#else
  USBDDR = (1 << USB_CFG_DMINUS_BIT) | (1 << USB_CFG_DPLUS_BIT) | PORTD_DDR ;
  /* for output (LED, TxD) */
#endif

  j = 0;
  while (--j) {		/* USB Reset by device only required on Watchdog Reset */
    i = 0;
    while (--i);	/* delay >10ms for USB reset */
  }
#ifdef USB_CFG_PULLUP_IOPORT
  usbDeviceConnect();
#else
  USBDDR = 0 | PORTD_DDR;		/*  remove USB reset condition */
#endif

}
//============================================================



//============================================================
// CLASS USB_MIDI_DEVICE
//============================================================

class UsbMidiDevice {

public:
	UsbMidiDevice () {
	}

	void init(){
	  
	  // disable timer 0 overflow interrupt (used for millis)
	  	TIMSK &= ~(1 << TOIE0);

  		wdt_enable(WDTO_1S);
  		//odDebugInit();
  		//hardwareInit();
  		usbDeviceDisconnect();
		_delay_ms(250);
		usbDeviceConnect();
  		usbInit();
		
  		sendEmptyFrame = 0;
		
  		sei();
  		//PORTB = 1;
	}
  
	void update() {
		wdt_reset();
    	usbPoll();

		if( usbInterruptIsReady() && buffer_size>0 ) {
      		usbSetInterrupt(buffer, buffer_size);
      		buffer_size = 0;
      		
      		if(buffer[0]==0x04 && buffer[4]==0x04)//SimpleSisex
      		{
      			buffer[0]=0x05;
      			buffer[1]=0xf7;
      			//buffer[2]=0x00;
      			//buffer[3]=0x00;
      			buffer_size = 2;
      		}
    	}
	}
  
	bool sendNoteOn(byte ch, byte note, byte vel){
		if(buffer_size>0) return FALSE;
		buffer[0] = 0x09;
		buffer[1] = 0x90 | ch;
		buffer[2] = 0x7f & note;
		buffer[3] = 0x7f & vel;
		//sendMidiMessage(buffer,4);
		buffer_size = 4;
		
		return TRUE;
	}

	void sendNoteOff(byte ch, byte note){
		buffer[0] = 0x08;
		buffer[1] = 0x80 | ch;
		buffer[2] = 0x7f & note;
		buffer[3] = 0;
		//sendMidiMessage(buffer,4);
		buffer_size = 4;
	}

	void sendCtlChange(byte ch, byte num, byte value){
		buffer[0] = 0x0b;
		buffer[1] = 0xb0 | ch;
		buffer[2] = 0x7f & num;
		buffer[3] = 0x7f & value;
		//sendMidiMessage(buffer,4);
		buffer_size = 4;
	}

	bool sendSimpleSysex(byte cmd, byte data1, byte data2,byte data3){
		if(buffer_size>0) return FALSE;
		buffer[0] = 0x04;
		buffer[1] = 0xf0;
		buffer[2] = 0x77;
		buffer[3] = 0x7f & cmd;
		buffer[4] = 0x04;
		buffer[5] = 0x7f & data1;
		buffer[6] = 0x7f & data2;
		buffer[7] = 0x7f & data3;

		buffer_size = 8;
	}

 	void setHdlNoteOff(void (*fptr)(byte ch, byte note, byte vel)){
 		cbUsbNoteOff = fptr;
 	}

	void setHdlNoteOn(void (*fptr)(byte ch, byte note, byte vel)){
 		cbUsbNoteOn = fptr;
	}

	void setHdlCtlChange(void (*fptr)(byte ch, byte num, byte value)){
 		cbUsbCtlChange = fptr;
	}

	void setHdlSimpleSysex(void (*fptr)(byte cmd, byte data1, byte data2, byte data3)){
 		cbUsbSimpleSysex = fptr;
	}
private:
	//TODO:CircularBuffer
	byte buffer[8];
	byte buffer_size = 0;

};
//============================================================

#define IN_BUFFER_SIZE 16
#define OUT_BUFFER_SIZE 16

class SerialMidiDevice {

public:
	SerialMidiDevice () {
	}

	void init(){
	  	// set baud rate 
  		UBRRL	= 31; //		// 312500Hz at 16MHz clock 

		UCSRB	= (1<<RXEN) | (1<<TXEN);
	}
  
	void update() {
	    	//    send to Serial MIDI line    
    	if( (UCSRA & (1<<UDRE)) && out_buffer_head!=out_buffer_tail ) {
      		UDR = out_buffer[out_buffer_tail];
      		out_buffer_tail=(out_buffer_tail+1)%OUT_BUFFER_SIZE;
    	}
    	
   		//    receive from Serial MIDI line    
    	if( UCSRA & (1<<RXC)) {      		
      		byte i=(in_buffer_head+1)%IN_BUFFER_SIZE;
      		if(i!=in_buffer_tail)
      		{
      			in_buffer[in_buffer_head]=UDR;
      			in_buffer_head=i;
      			if((in_buffer[in_buffer_tail]&0xf0)==0x90)//NOTE_ON
      			{
      				//TODO: available_data_in_buffer()
      				if( ((in_buffer_tail+1)%IN_BUFFER_SIZE)!=in_buffer_head && ((in_buffer_tail+2)%IN_BUFFER_SIZE)!=in_buffer_head )
      				{
      					(*cbSerialNoteOn)(in_buffer[in_buffer_tail]&0x0f,in_buffer[(in_buffer_tail+1)%IN_BUFFER_SIZE]&0x7f,in_buffer[(in_buffer_tail+2)%IN_BUFFER_SIZE]&0x7f);
      					in_buffer_tail=in_buffer_head;
      				}
      			}
      			else if(in_buffer[in_buffer_tail]==0xf0)//Sysex
      			{
      				//TODO: available_data_in_buffer()
      				if( ((in_buffer_tail+1)%IN_BUFFER_SIZE)!=in_buffer_head && 
      				    ((in_buffer_tail+2)%IN_BUFFER_SIZE)!=in_buffer_head &&
      				    ((in_buffer_tail+3)%IN_BUFFER_SIZE)!=in_buffer_head &&
      				    ((in_buffer_tail+4)%IN_BUFFER_SIZE)!=in_buffer_head &&
      				    ((in_buffer_tail+5)%IN_BUFFER_SIZE)!=in_buffer_head &&
      				    ((in_buffer_tail+6)%IN_BUFFER_SIZE)!=in_buffer_head
      				    )
      				{
      					(*cbSerialSimpleSysex)(in_buffer[in_buffer_tail+2]&0x7f,in_buffer[(in_buffer_tail+3)%IN_BUFFER_SIZE]&0x7f,in_buffer[(in_buffer_tail+4)%IN_BUFFER_SIZE]&0x7f,in_buffer[(in_buffer_tail+5)%IN_BUFFER_SIZE]&0x7f);
      					in_buffer_tail=in_buffer_head;
      				}
      			}
      			else
      			{
      				//Flush
      				in_buffer_tail=in_buffer_head;
      			}
      		}
    	}		
	}
  
	void sendNoteOn(byte ch, byte note, byte vel){
		out_buffer[out_buffer_head]=0x90|ch;
		
		//TODO: verifica available_space_out_buffer()
		out_buffer[(out_buffer_head+1)%OUT_BUFFER_SIZE]=note&0x7f;
		out_buffer[(out_buffer_head+2)%OUT_BUFFER_SIZE]=vel&0x7f;
		
		out_buffer_head=(out_buffer_head+3)%OUT_BUFFER_SIZE;
	}

	void sendSimpleSysex(byte cmd, byte data1, byte data2,byte data3){
	
		out_buffer[out_buffer_head]=0xf0;
		
		//TODO: verifica available_space_out_buffer()
		out_buffer[(out_buffer_head+1)%OUT_BUFFER_SIZE]=0x77;
		out_buffer[(out_buffer_head+2)%OUT_BUFFER_SIZE]=cmd&0x7f;
		out_buffer[(out_buffer_head+3)%OUT_BUFFER_SIZE]=data1&0x7f;
		out_buffer[(out_buffer_head+4)%OUT_BUFFER_SIZE]=data2&0x7f;
		out_buffer[(out_buffer_head+5)%OUT_BUFFER_SIZE]=data3&0x7f;
		out_buffer[(out_buffer_head+6)%OUT_BUFFER_SIZE]=0xf7;
		
		out_buffer_head=(out_buffer_head+8)%OUT_BUFFER_SIZE;
		
	}
	
	void setHdlNoteOn(void (*fptr)(byte ch, byte note, byte vel)){
 		cbSerialNoteOn = fptr;
	}
	void setHdlSimpleSysex(void (*fptr)(byte cmd, byte data1, byte data2, byte data3)){
 		cbSerialSimpleSysex = fptr;
	}
private:

	byte available_space_in_buffer()
	{
		return (in_buffer_tail>in_buffer_head)?in_buffer_tail-in_buffer_head:IN_BUFFER_SIZE-(in_buffer_head-in_buffer_tail);
	}
	byte available_data_in_buffer()
	{
		return (in_buffer_tail>in_buffer_head)?IN_BUFFER_SIZE-(in_buffer_tail-in_buffer_head):in_buffer_head-in_buffer_tail;
	}
	
	byte in_buffer[IN_BUFFER_SIZE];
	byte out_buffer[OUT_BUFFER_SIZE];
	byte in_buffer_head = 0;
	byte in_buffer_tail = 0;
	byte out_buffer_head = 0;
	byte out_buffer_tail = 0;

};
//============================================================

UsbMidiDevice UsbMidi = UsbMidiDevice();
SerialMidiDevice SerialMidi = SerialMidiDevice();

#endif // __picoMIDI_h__
