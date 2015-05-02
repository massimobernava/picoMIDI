/*
  USB-MIDI Button send MIDI NOTE ON
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
//               GND|10   11|PD6  (PIN 8)(BUTTON)
//                  +-------+

#include "picoMIDI.h"

int buttonState = 0;

void setup() {                
  pinMode(9, OUTPUT);
  pinMode(8, INPUT);
  
  digitalWrite(9, HIGH);
  delayMs(10);
  digitalWrite(9, LOW);

  UsbMidi.init();
}

void loop() {
  UsbMidi.update();
  buttonState = digitalRead(8);
  
    if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(9, HIGH);
    //UsbMidi.sendNoteOff(9, 37);
  }
  else {
    // turn LED off:
    digitalWrite(9, LOW);
    UsbMidi.sendNoteOn(9, 37, 100);
  }
}

void delayMs(unsigned int ms) {
  for( int i=0; i<ms; i++ ) {
    delayMicroseconds(1000);
  }
}
