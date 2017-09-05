#include <stdlib.h>
#include <SoftwareSerial.h>
#include <OneWire.h> // Inclusion de la librairie OneWire
#include <Manchester.h>
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
 
#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 7 // Broche utilisée pour le bus 1-Wire
 
OneWire ds(BROCHE_ONEWIRE); // Création de l'objet OneWire ds
#define SSID "Livebox-ed10"
#define PASS "9D27A36134C3F99257CD1DE9EC"
#define IP "184.106.153.149" // thingspeak.com
String GET = "GET /update?key=H308Q7N8R7UGEPQN&field1=";
SoftwareSerial monitor(8,9); // RX, TX 

#define RX_PIN 6
uint8_t moo = 1;
#define BUFFER_SIZE 10
uint8_t buffer[BUFFER_SIZE];


 
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

void initWifi()
{
  monitor.begin(9600);
  //Reset du module
  monitor.println("AT+RST");
  delay(1000);
  
  //on attend qu'il soit pret
  if(monitor.find("ready"))
  {
    Serial.println("Communication with ESPOK");
  }
  
  monitor.println("AT");
  delay(1000);
  
  if(monitor.find("OK")){
    
    Serial.println("Communication with ESPOK");
    connectWiFi();
    
    /*monitor.println("AT+CIFSR");    
    
    delay(5000);
    
    while (monitor.available())
       {
         char c = monitor.read();
         Serial.write(c);
         if(c=='\r') Serial.print('\n');
       }*/
       
      monitor.println("AT+CWJAP?");  
      
        delay(5000);
    
    while (monitor.available())
       {
         char c = monitor.read();
         Serial.write(c);
         if(c=='\r') Serial.print('\n');
       }
   
    //AT+CWJAP?
    
  }
}

void setTemperature()
{
   
  float temp;
  String tmp="";
  char charVal[10];

  if(getTemperature(&temp)) {
     
    // Affiche la température
    Serial.print("Temperature : ");
    Serial.print(temp);
    Serial.write(176); // caractère °
    Serial.write('C');
    Serial.println();
    
    dtostrf(temp, 4, 2, charVal);
    
    //convert chararray to string
    for(int i=0;i<4;i++)
    {
      if(charVal[i]=='.')
        tmp+=".";
      else
        tmp+=charVal[i];
    }
    
    updateTemp(tmp);
    
  }
   
  delay(60000);  
  
}

void setup()
{
  
  Serial.begin(9600);
  
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceiveArray(BUFFER_SIZE, buffer);   
 
  
}

uint8_t id;
uint8_t data;

void loop(){
  
if (man.receiveComplete()) {
 Serial.println("test");
  //received something
uint16_t m = man.getMessage();
man.beginReceive(); //start listening for next message right after you retrieve the message
if (man.decodeMessage(m, id, data)) { //extract id and data from message, check if checksum is correct
//id now contains ID of sender (or receiver(or any other arbitrary 4 bits of data))
//data now contains one byte of data received from sender
//both id and data are BELIEVED (not guaranteed) to be transmitted correctly
moo = ++moo % 2;

}
}

}

void updateTemp(String tenmpF){
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  monitor.println(cmd);
  Serial.println(cmd);
  delay(2000);
  if(monitor.find("Error")){
    Serial.print("RECEIVED: Error");
    return;
  }
  cmd = GET;
  cmd += tenmpF;
  cmd += "\r\n";
  monitor.print("AT+CIPSEND=");
  monitor.println(cmd.length());
  if(monitor.find(">")){
    Serial.print(">");
    Serial.print(cmd);
    monitor.print(cmd);
  }else{
    //sendDebug("AT+CIPCLOSE");
    monitor.println("AT+CIPCLOSE");
  }
  if(monitor.find("OK")){
    Serial.println("RECEIVED: OK");
  }else{
    Serial.println("RECEIVED: Error");
  }
}
void sendDebug(String cmd){
  monitor.print("SEND: ");
  monitor.println(cmd);
  Serial.println(cmd);
} 
 
boolean connectWiFi(){
  monitor.println("AT+CWMODE=3");
  Serial.println("AT+CWMODE=3");
  delay(5000);
  
  if(monitor.find("OK")){
    Serial.println("RECEIVED Mode: OK");
   
  }else{
    Serial.println("RECEIVED Mode: Error");
    return false;
  }
  
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  monitor.println(cmd);
  Serial.println(cmd);
  //sendDebug(cmd);
  delay(5000);
  if(monitor.find("OK")){
    Serial.println("RECEIVED: OK");
    return true;
  }else{
    Serial.println("RECEIVED: Error");
    return false;
  }
}
