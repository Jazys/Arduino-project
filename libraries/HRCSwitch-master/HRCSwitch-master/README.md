BEFORE USING THIS LIBRARY YOU SHOULD CHECK fuzzillogic library!
===
* It managed RCSwitch code (RemoteSwitch)
* It managed HomeEasy/Dio (NewRemoteSwitch)
* And even some weather stations (RemoteSensor)
There are even a system to manage interruptchain so you can use all theses protocols without issue and add more!
https://bitbucket.org/fuzzillogic/433mhzforarduino/wiki/Home

HRCSwitch
=========
* HRCSwitch is a fork of RCSwitch libraries that also support HomeEasy Protocol (only for transmitting)
* If you use send with 3 arguments the library will send a HE300 Protocol Code
* If you use send with 2 arguments the library will send RCSwitch Protocol 1

* mySwitch.send(Decimal Code,Length)
* mySwitch.send(Remote/Device code,Button/Recipient code,on/off)


The librairies have a test code inside example (senddemo)

`````
#include <HRCSwitch.h>
HRCSwitch mySwitch = HRCSwitch();

const int TXpin = 10;

void setup() {
  Serial.begin(9600);
  mySwitch.enableTransmit(TXpin);
  Serial.println("HRCSwitch ready");
}

void loop() {
  //Turn ON HE300 
  mySwitch.send(1234,0,true);
  delay(2000);
  //Turn ON RCSwitch
  mySwitch.send(1234,24);
  delay(2000);
  //Turn OFF HE300 
  mySwitch.send(1234,0,false);
  delay(2000);
  //Turn OFF RCSwitch
  mySwitch.send(1233,24);
  delay(2000);
}

````
