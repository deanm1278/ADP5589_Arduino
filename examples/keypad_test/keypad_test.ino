
/* keypad_test.ino
 * April 10, 2016 Dean Miller
 * MIT license
 * 
 * use with ADP5589 keypad controller library: 
 * https://github.com/deanm1278/ADP5589_Arduino
 * 
 * ADP5589 Datasheet: 
 * http://www.analog.com/media/en/technical-documentation/data-sheets/ADP5589.pdf
 * 
 * plz note this is a 3.3V device
 * 
 * This example uses an ADP5589 keypad controller with switches
 * attached to (row , col)
 *              0   , 0
 *              0   , 1
 *              1   , 0
 *              1   , 1
 *              
 * There are LEDs attached to GPIOs 5,6,7,8 on the ADP5589.
 * 
 * A callback is attached to switch 0, 1 that blinks the LED
 * when a rising edge is detected.
 * 
 */

#include "ADP5589.h"

//declare how may leds we will use
#define NUMLEDS 4

//declare which gpios will be used as LEDs on the keypad
uint8_t ledPins[] = {5,6,7,8};

//declare the keypad
ADP5589 keypad = ADP5589();

//this will be used as a callback
int blinkLed(void);

void setup() {
  Serial.begin(9600);
  
  //set up the keypad
  keypad.begin();

  //create a 2x2 matrix using rows 0 and 1, and columns 0 and 1
  keypad.activateRow(0);
  keypad.activateRow(1);

  keypad.activateColumn(0);
  keypad.activateColumn(1);

  /* 
   * register a callback on button in row 0 col 1 for when a rising edge
   * is detected. Callbacks must be registered AFTER the active rows and columns
   * are declared.
   */
  keypad.registerCallback(0, 1, blinkLed, ADP5589_RISING);
  
  //set GPIO pins as outputs and do a thing to show we're on
  for(int i = 0; i<NUMLEDS; i++){
    keypad.gpioSetDirection(ledPins[i], ADP5589_OUTPUT);
    keypad.gpioWrite(ledPins[i], HIGH);
    delay(50);
  }
  for(int i=0; i<NUMLEDS; i++){
    keypad.gpioWrite(ledPins[i], LOW);
    delay(50);
  }
}

void loop() {
  //update the keypad. This will automatically call all triggered callbacks
  keypad.update();

  //delay in between updates
  delay(30);
}

//callback definition
int blinkLed(int evt){
  Serial.print("callback called! event number ");
  Serial.println(evt);

  //turn LED on
  keypad.gpioWrite(5, HIGH);
  delay(100);
  //turn LED off
  keypad.gpioWrite(5, LOW);

  return 0;
}
