#include <RCSwitch.h>
#include <HRCSwitch.h>

#define RXRFPIN 13
#define TXRFPIN 0

RCSwitch mySwitch = RCSwitch();
HRCSwitch mySwitchTransmitter = HRCSwitch();

void setup() {
  Serial.begin(9600);
  
  //Pour émettre en radio
  mySwitchTransmitter.enableTransmit(RXRFPIN);
  
  mySwitch.enableReceive(TXRFPIN);  // Receiver on inerrupt 0 => that is pin #2
}

void loop() {
  
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() ); 
      delay(500);   
      mySwitchTransmitter.send(mySwitch.getReceivedValue(),mySwitch.getReceivedBitlength());
      //mySwitchTransmitter.send(12325261,1,false);          
      
      delay(500);
    }

    mySwitch.resetAvailable();
  }
}
