
#include <RCSwitch.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
 
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifndef WDTCR // Missing for attiny44, 84
#define WDTCR   _SFR_IO8(0x21)
#endif

#define HEADER_RAD 10
#define LENGTH_RAD  5
#define TYPE_RAD 1
#define ID_RAD 1

#define PIN_RF_VCC 2
#define PIN_RF_DATA 0

RCSwitch mySwitch = RCSwitch();

volatile int f_wdt = 0;

void setup()
{
  //pour activer le composant 433
  pinMode(PIN_RF_VCC,OUTPUT);
  mySwitch.enableTransmit(PIN_RF_DATA);

  //pour la gestion de l'attiny base conso
  sbi(GIMSK,PCIE); // Turn on Pin Change interrupt
  sbi(PCMSK,PCINT1); // Which pins are affected by the interrupt
  
  //Reveil sur timer
  //setup_watchdog(8); // approximately 4 seconds sleep
}
 
void loop(){
  
  int message=(HEADER_RAD*pow(10,(LENGTH_RAD-2)))+(LENGTH_RAD*pow(10,(LENGTH_RAD-3)))+(TYPE_RAD*pow(10,(LENGTH_RAD-4)))+ID_RAD;
  
  //active composant RF +COM
  digitalWrite(PIN_RF_VCC,HIGH);  
  mySwitch.send(message, 24);
  delay(1000);
  
  //repasse en mode inactif
  digitalWrite(PIN_RF_VCC,LOW);
  pinMode(PIN_RF_VCC,INPUT); // set all used port to intput to save power
  system_sleep();
  pinMode(PIN_RF_VCC,OUTPUT); // set all ports into state before sleep
 
}
 

void system_sleep() {
  cbi(ADCSRA,ADEN); // Switch Analog to Digital converter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Set sleep mode
  sleep_mode(); // System sleeps here
  sbi(ADCSRA,ADEN);  // Switch Analog to Digital converter ON
}

ISR(PCINT0_vect) {
}
 
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {
 
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
   
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
 
// Watchdog Interrupt Service / is executed when watchdog timed out
ISR(WDT_vect) {
 f_wdt=f_wdt+1;  // set global flag
}
