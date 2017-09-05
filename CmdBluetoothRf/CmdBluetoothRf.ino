#include <SoftwareSerial.h>
#include <OneWire.h> // Inclusion de la librairie OneWire
#include <HRCSwitch.h>
#include <RCSwitch.h>
#define IMEI_CMD_DIO 12325261

#define DS18B20 0x28     // Adresse 1-Wire du DS18B20
#define BROCHE_ONEWIRE 7 // Broche utilisée pour le bus 1-Wire

OneWire ds(BROCHE_ONEWIRE); // Création de l'objet OneWire ds
 
SoftwareSerial bluetoothRS(5, 6); // (RX, TX) (pin Rx BT, pin Tx BT)

HRCSwitch mySwitch = HRCSwitch();
RCSwitch mySwitchReceive = RCSwitch();
const int TXpinRF = 13;
const int RXpinRF = 0 ; //correspond à la broche 2
const int tempoInterOctetLS=30;
String readStringBt="";
int compteurCaractereBT=0;

bool switchLumOn;
int numberDevice;
String radioRXValue="";
String radioToSend="";

int bauds=9600;

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
    // Ouvre la voie série avec l'ordinateur
    Serial.begin(bauds);
    // Ouvre la voie série avec le module BT
    bluetoothRS.begin(bauds);
    
    //Pour émettre en radio
    mySwitch.enableTransmit(TXpinRF);
    
    //pour la reception
    mySwitchReceive.enableReceive(RXpinRF); // Receiver on inerrupt 0 => that is pin #2
    
    //
    Serial.println("Tranceiver Bluetooth/RF 433 by Jazys");
    
    switchLumOn=true;
    numberDevice=0;
}
 
void loop() // run over and over
{
  
  if (mySwitchReceive.available()) 
  {
    int value = mySwitchReceive.getReceivedValue();
    if (value == 0) 
    {
      Serial.print("Unknown encoding");
    } 
    else 
    {
      radioRXValue=String(mySwitchReceive.getReceivedValue());
      radioToSend="";
      
      Serial.print("Received ");
      Serial.print(  radioRXValue);
      Serial.print(" / ");
      Serial.print( mySwitchReceive.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitchReceive.getReceivedProtocol() );  

      
      //si c'est un paquet radio correcte
      if(radioRXValue[0]=='1' && radioRXValue[1]=='0')
      {

        //si c'est un paquet type sonnette
        if(radioRXValue[2]=='5' && radioRXValue[3]=='1')
        {
          radioToSend="AT+RNG:";
          radioToSend=radioToSend+radioRXValue[4];
          radioToSend=radioToSend+"\r\n";
          
          bluetoothRS.write(radioToSend.c_str());
        }
        //sinon c'est un type capteur de temperature
        else if(radioRXValue[2]=='7' && radioRXValue[3]=='2')
        {
          radioToSend="AT+TMP:";
          radioToSend=radioToSend+radioRXValue[4]+","+radioRXValue[5]+radioRXValue[6];
          radioToSend=radioToSend+"\r\n";
          
          bluetoothRS.write(radioToSend.c_str());
          
        }
        
      }
        
    }
    
    mySwitchReceive.resetAvailable();
  }
  
  //Si des données arrives en Bluetooth
  while (bluetoothRS.available()) 
  {
    delay(tempoInterOctetLS);  //small delay to allow input buffer to fill

    char c = bluetoothRS.read();  //lecture du caractere courant
    
    //Si c'est un A, c'est le debut d'une commande AT (il ne peut pas y avoir de A dans une commande)
    if(c=='A')
     readStringBt=""; //clears variable for new input
    
    //si c'est \r\n alors c'est une fin de commande
    if (c == '\n' && readStringBt[readStringBt.length()-1]=='\r') 
    {
       break;
    }  //on sort pour afficher quelque chose
    
    readStringBt += c; 
  }

 // Si la liaison RS Bluetooth contient des données
 if (readStringBt.length() >0) 
 {
   //Les commandes sont de la forme AT+CMD:
   //on recupère tout ce qu'il y a avant les ":"
   readStringBt.trim();
   String cmdAT= getValue(readStringBt, ':', 0);
   
   Serial.println(readStringBt); //prints string to serial port out
   
   //pour le keep alive, on reçoit AT, on envoit OK
   if(readStringBt.length()==2 && readStringBt[0]=='A' && readStringBt[1]=='T')
   {
     Serial.println("Keep Alive ");
     //acquitte le bluetooth
     bluetoothRS.write("OK\r\n");  
     
     readStringBt=""; //clears variable for new input
   }
     
   //Table de correspondance
   if(cmdAT=="AT+LUM")
   {
        Serial.println("Commande allumage : "+cmdAT);
     
       cmdAT= getValue(readStringBt, ':', 1);
       
       //recupère l'identifiant de la prise
       readStringBt=getValue(cmdAT, ',', 0);
       numberDevice=readStringBt.toInt();
       
       
       //pour savoir s'il faut l'éteindre ou l'allumer
       if(getValue(cmdAT, ',', 1)=="1")
         switchLumOn=true;
       else
         switchLumOn=false;
       
       //On envoie la commande  
       mySwitch.send(IMEI_CMD_DIO,numberDevice,switchLumOn);
       
       //Pour répéter la commande
       delay(10);
       mySwitch.send(IMEI_CMD_DIO,numberDevice,switchLumOn);
       
       
       //acquitte le bluetooth
       bluetoothRS.write("OK\r\n");         
       
       readStringBt=""; //clears variable for new input
       
   }

  
  }
  
       
   
 
}

/**
* Permet de splitter une chaine et de recupérer le contenu en fonction de l'index
*/
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


