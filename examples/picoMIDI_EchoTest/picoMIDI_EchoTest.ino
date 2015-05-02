/*
  USB-MIDI Echo
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
//       (PIN 6) PD4|8    13|PB1  (PIN 10)
//       (PIN 7) PD5|9    12|PB0  (PIN 9)(LED)
//               GND|10   11|PD6  (PIN 8)
//                  +-------+

#include "picoMIDI.h"

// LED 
#define LED_INIT() (DDRB |= _BV(0))
#define LED_HIGH() (PORTB |= _BV(0))
#define LED_LOW() (PORTB &= ~_BV(0))
#define LED_FLIP() ( PORTB ^= _BV(0))


void onNoteOn(byte ch, byte note, byte vel){
	UsbMidi.sendNoteOn(ch,note,vel);
	LED_FLIP();
}

void setup() {                
  LED_INIT();
  LED_HIGH();
  delayMs(10);
  LED_LOW();

  UsbMidi.init();
  UsbMidi.setHdlNoteOn(onNoteOn);
}

void loop() {

  UsbMidi.update();
}

void delayMs(unsigned int ms) {
  for( int i=0; i<ms; i++ ) {
    delayMicroseconds(1000);
  }
}
