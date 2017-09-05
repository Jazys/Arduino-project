
/****
Capteur de temperature avec un attiny, un ds18b20 et un chip 433

**/
#include <RCSwitch.h>
#include <OneWire.h> // Inclusion de la librairie OneWire
#include <avr/sleep.h>
#include <avr/wdt.h>
 
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 0 // Broche utilisée pour le bus 1-Wire

#ifndef WDTCR // Missing for attiny44, 84
#define WDTCR   _SFR_IO8(0x21)
#endif 

#define NBMINUTEACTION 10  // toutes les 10 on recup la temperature
#define VCC433 2
#define PIN_RF_DATA 3

#define HEADER_RAD 10
#define LENGTH_RAD 7
#define TYPE_RAD 2
#define ID_RAD 1

OneWire ds(BROCHE_ONEWIRE);

RCSwitch mySwitch = RCSwitch();

float temp;
volatile int f_wdt = 0;
int compteurAction;

// Fonction récupérant la température depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
boolean getTemperature(float *temp){
  byte data[9], addr[8];
  // data : Données lues depuis le scratchpad
  // addr : adresse du module 1-Wire détecté
 
  if (!ds.search(addr)) { // Recherche un module 1-Wire
    ds.reset_search();    // Réinitialise la recherche de module
    return false;         // Retourne une erreur
  }
   
  if (OneWire::crc8(addr, 7) != addr[7]) // Vérifie que l'adresse a été correctement reçue
    return false;                        // Si le message est corrompu on retourne une erreur
 
  if (addr[0] != DS18B20) // Vérifie qu'il s'agit bien d'un DS18B20
    return false;         // Si ce n'est pas le cas on retourne une erreur
 
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
   
  ds.write(0x44, 1);      // On lance une prise de mesure de température
  delay(800);             // Et on attend la fin de la mesure
   
  ds.reset();             // On reset le bus 1-Wire
  ds.select(addr);        // On sélectionne le DS18B20
  ds.write(0xBE);         // On envoie une demande de lecture du scratchpad
 
  for (byte i = 0; i < 9; i++) // On lit le scratchpad
    data[i] = ds.read();       // Et on stock les octets reçus
   
  // Calcul de la température en degré Celsius
  *temp = ((data[1] << 8) | data[0]) * 0.0625;
   
  // Pas d'erreur
  return true;
}


void setup()
{
  pinMode(VCC433,OUTPUT);
  
  //Pour indiquer que le module demarre bien
  digitalWrite(VCC433,HIGH);  // let led blink  
  delay(5000);
  digitalWrite(VCC433,LOW);  // let led blink  

  temp=0.0;
  compteurAction=0;
  mySwitch.enableTransmit(PIN_RF_DATA);
  setup_watchdog(9); // approximately 8 seconds sleep

}
 
void loop(){
// if (f_wdt>(NBMINUTEACTION*6)) {  // (56 s * NBMINUTEACTION)
  if(f_wdt>0){ // pour test
  f_wdt=0;       // reset flag
  
  digitalWrite(VCC433,HIGH);  // let led blink  
  delay(1000);
   
  if(getTemperature(&temp)) { 
 
    //On prend la partie entiere et on ajoute 40 (on va d'une plage de -40 à +60 degré)
    int temp_int=int(temp)+40;
        
    //construit le message    
    long message=(HEADER_RAD*pow(10,(LENGTH_RAD-2)))+(7*pow(10,(LENGTH_RAD-3)))+(TYPE_RAD*pow(10,(LENGTH_RAD-4)))+(ID_RAD*pow(10,(LENGTH_RAD-5)))+temp_int;
  
    mySwitch.send(message, 24);
    
    digitalWrite(VCC433,LOW); 
   delay(3000);   
    
  }   
  
   pinMode(VCC433,INPUT); // set all used port to intput to save power
   system_sleep();
   pinMode(VCC433,OUTPUT); // set all ports into state before sleep

  }
}
 
// set system into the sleep state
// system wakes up when wtchdog is timed out
void system_sleep() {
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
   
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
   
  sleep_mode();                        // System sleeps here
   
  sleep_disable();                     // System continues execution here when watchdog timed out
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
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
