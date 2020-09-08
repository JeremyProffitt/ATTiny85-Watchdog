#include <avr/power.h>    // Power management

#define RESET_MS 3000 //Time to hold reset line LOW
#define WATCHDOG_TIMEOUT_SEC 14 * 60 // 14 Minutes

/*
                      AT Tiny 85 Hookup Diagram
               +-----+
PB5 / RESET 1 -|     |- 8 VCC
        PB3 2 -|     |- 7 PB2 / SCK 
        PB4 3 -|     |- 6 PB1 / MOSI
        GND 4 -|     |- 5 PB0 / MISO
               +-----+
*/



//What Pins Do
const int resetPin    = PB4;  //Reset Pin for CPU
const int petPin      = PB1;  //Pin that gets hit to reset watchdog counter (Make sure to change interupt pin as well)
const int wasResetPin = PB0;  //This pin is set to High after reset, low after the first pet.  Tells us if a reset was done.

unsigned long lastPet = 0;  //Internal Timer
volatile bool petPinInterrupt = false; // Volatile as this flag is set in the Interrupt Service Routine

void setup() {
  // This saves power
  ADCSRA &= ~(1<<ADEN);
  
  //Setup Pin Modes
  pinMode(resetPin,INPUT_PULLUP);
  //pinMode(resetPin,OUTPUT);
  pinMode(petPin,INPUT);
  pinMode(wasResetPin, OUTPUT);  
  digitalWrite(wasResetPin, HIGH);  //On power on, wasReset should not be low
   
  PCMSK  |= bit (PCINT1);               // Pinchange interrupt on pin
  GIFR   |= bit (PCIF);                 // clear any outstanding interrupts
  GIMSK  |= bit (PCIE);                 // enable pin change interrupts
}

void loop() {

  //If the Pet Pin was cycled, then reset last Pet Timer
  if (petPinInterrupt) {           
    lastPet = millis();            
    petPinInterrupt = false;
  }
  
  //If it's been too long, issue that reset
  if (millis() - lastPet >= WATCHDOG_TIMEOUT_SEC * 1000UL) {
    //pinMode(resetPin, INPUT);
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, HIGH);
    delay(RESET_MS);    
    digitalWrite(resetPin, LOW);
    delay(RESET_MS); 
    digitalWrite(resetPin, HIGH);
    delay(RESET_MS);
    pinMode(resetPin, INPUT_PULLUP);
    digitalWrite(wasResetPin, LOW);
    lastPet = millis();
  }
}

ISR (PCINT0_vect) {                    
  petPinInterrupt = true;
  digitalWrite(wasResetPin, HIGH);  //If we get a pet, turn off wasReset
}
